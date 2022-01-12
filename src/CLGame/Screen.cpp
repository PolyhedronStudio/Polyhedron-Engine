// LICENSE HERE.

//
// clg_screen.c
//
//
// Client Screen Implementation.
//
#include "ClientGameLocal.h"

#include "Main.h"
#include "Media.h"
#include "Screen.h"

RenderScreenData scr;

//-----
// CVars.
//-----
cvar_t* scr_viewsize;           // Scale of the view size.

static cvar_t* scr_fps;         // Show FPS count, or not.
static cvar_t* scr_showitemname;// Show item name, or not.

static cvar_t* scr_draw2d;      // To draw 2D elements or not.
cvar_t* scr_alpha;              // WID: TODO: Move elsewhere... // Alpha value for elements.
static cvar_t* scr_font;        // The quake fontset to use for rendering.
cvar_t* scr_scale;       // WID: TODO: Move elsewhere... // Determines the scale we render our screen elements at.

static cvar_t* scr_centertime;  // The amount of time the center print is on-screen.

static cvar_t* scr_crosshair;   // Whether to draw the crosshair or not.

static cvar_t* scr_chathud;
static cvar_t* scr_chathud_lines;
static cvar_t* scr_chathud_time;
static cvar_t* scr_chathud_x;
static cvar_t* scr_chathud_y;

static cvar_t* ch_scale;        // Crosshair scale.
static cvar_t* ch_x;            // Crosshair Y
static cvar_t* ch_y;            // Crosshair X

static cvar_t* ch_health;       // Wether to color the crosshair based on current health or not.
static cvar_t* ch_red;          // Crosshair R color value.
static cvar_t* ch_green;        // Crosshair G color value.
static cvar_t* ch_blue;         // Crosshair B color value.
static cvar_t* ch_alpha;        // Crosshair A color value.

//-----
// View Rectangle.
//-----
rect_t     scr_vrect;      // position of render window on screen

// HUD image filenames, matching their corresponding number based on array access. (Exception for the -..)
static const char* const sb_nums[2][STAT_PICS] = {
    {
        "num_0", "num_1", "num_2", "num_3", "num_4", "num_5",
        "num_6", "num_7", "num_8", "num_9", "num_minus"
    },
    {
        "anum_0", "anum_1", "anum_2", "anum_3", "anum_4", "anum_5",
        "anum_6", "anum_7", "anum_8", "anum_9", "anum_minus"
    }
};

// Color table containing actual color codes that match their string names.
const uint32_t colorTable[8] = {
    U32_BLACK, U32_RED, U32_GREEN, U32_YELLOW,
    U32_BLUE, U32_CYAN, U32_MAGENTA, U32_WHITE
};

//
//=============================================================================
//
// SCREEN UTILITY FUNCTIONS
//
//=============================================================================
// 

//
//===============
// SCR_DrawString
// 
// A simplified version of SCR_DrawStringEx. It fills in certain defaults
// so you don't have to worry about those.
// 
// (Uses the default font, and the default string limit.)
//===============
//
#define SCR_DrawString(x, y, flags, string) \
    SCR_DrawStringEx(x, y, flags, MAX_STRING_CHARS, string, scr.font_pic)

//
//===============
// SCR_DrawStringEx
// 
// Draws a string with the given font, maximum length, and flags, at the
// desired destination. Does NOT support multiline string buffers (\n).
//===============
//
int SCR_DrawStringEx(int x, int y, int flags, size_t maxlen,
    const char* s, qhandle_t font)
{
    size_t len = strlen(s);

    if (len > maxlen) {
        len = maxlen;
    }

    if ((flags & UI_CENTER) == UI_CENTER) {
        x -= len * CHAR_WIDTH / 2;
    }
    else if (flags & UI_RIGHT) {
        x -= len * CHAR_WIDTH;
    }

    return clgi.R_DrawString(x, y, flags, maxlen, s, font);
}


//
//===============
// SCR_DrawStringMulti
// 
// Similar to SCR_DrawStringEx, with the addition of supporting strings
// that contain the multiline \n symbol.
//===============
//
void SCR_DrawStringMulti(int x, int y, int flags, size_t maxlen,
    const char* s, qhandle_t font)
{
    char* p;
    size_t  len;

    while (*s) {
        p = (char*)strchr(s, '\n'); // CPP: Cast
        if (!p) {
            SCR_DrawStringEx(x, y, flags, maxlen, s, font);
            break;
        }

        len = p - s;
        if (len > maxlen) {
            len = maxlen;
        }
        SCR_DrawStringEx(x, y, flags, len, s, font);

        y += CHAR_HEIGHT;
        s = p + 1;
    }
}


//
//===============
// SCR_FadeAlpha
// 
// Calculates the alpha value fade.
//
// startTime is meant to be a constant.
// visTime is the amount of time it has to be on-screen in miliseconds.
// fadeTime is the speed at which to fade in miliseconds.
//
// WatIsDeze: Excuse me if I documented this incorrectly, there's a lot 
// of more work to be done :)
//===============
//
float SCR_FadeAlpha(unsigned startTime, unsigned visTime, unsigned fadeTime)
{
    float alpha;
    unsigned timeLeft, delta = clgi.GetRealTime() - startTime;

    if (delta >= visTime) {
        return 0;
    }

    if (fadeTime > visTime) {
        fadeTime = visTime;
    }

    alpha = 1;
    timeLeft = visTime - delta;
    if (timeLeft < fadeTime) {
        alpha = (float)timeLeft / fadeTime;
    }

    return alpha;
}

//
//=============================================================================
//
// COLOR PARSING
//
//=============================================================================
// 
const char* const colorNames[10] = {
    "black", "red", "green", "yellow",
    "blue", "cyan", "magenta", "white",
    "alt", "none"
};

enum color_index_t {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_WHITE,

    COLOR_ALT,
    COLOR_NONE
};

//
//===============
// SCR_ParseColorIndex
// 
// Parses color name or index up to the maximum allowed index.
// Returns COLOR_NONE in case of error.
//
// TODO: Let the client be able to use this for the console. Why not? :)
// ================
//
color_index_t SCR_ParseColorIndex(const char* s, color_index_t last)
{
    color_index_t i;

    if (COM_IsUint(s)) {
        i = (color_index_t)strtoul(s, NULL, 10); // CPP: Cast
        return i > last ? COLOR_NONE : i;
    }

    for (i = (color_index_t)0; i <= last; i = (color_index_t)(i + 1)) { // CPP: Cast for loop
        if (!strcmp(colorNames[i], s)) {
            return i;
        }
    }

    return COLOR_NONE;
}

//
//===============
// SCR_ParseColor
// 
// Accepts as input a 12 bit hexadecimal color value, or one of the following
// string color names: black, red, green, yellow, blue, cyan, magenta, white,
// alt, none.
//
// If the color string is invalid, it returns false. If it is valid, it will
// return true and assign the color value to the designated color pointer.
//===============
//
qboolean SCR_ParseColor(const char* s, color_t* color)
{
    int i;
    int c[8];

    // parse generic color
    if (*s == '#') {
        s++;
        for (i = 0; s[i]; i++) {
            if (i == 8) {
                return false;
            }
            c[i] = Q_charhex(s[i]);
            if (c[i] == -1) {
                return false;
            }
        }

        switch (i) {
        case 3:
            color->u8[0] = c[0] | (c[0] << 4);
            color->u8[1] = c[1] | (c[1] << 4);
            color->u8[2] = c[2] | (c[2] << 4);
            color->u8[3] = 255;
            break;
        case 6:
            color->u8[0] = c[1] | (c[0] << 4);
            color->u8[1] = c[3] | (c[2] << 4);
            color->u8[2] = c[5] | (c[4] << 4);
            color->u8[3] = 255;
            break;
        case 8:
            color->u8[0] = c[1] | (c[0] << 4);
            color->u8[1] = c[3] | (c[2] << 4);
            color->u8[2] = c[5] | (c[4] << 4);
            color->u8[3] = c[7] | (c[6] << 4);
            break;
        default:
            return false;
        }

        return true;
    }

    // parse name or index
    i = SCR_ParseColorIndex(s, COLOR_WHITE);
    if (i == COLOR_NONE) {
        return false;
    }

    color->u32 = colorTable[i];
    return true;
}

//
//=============================================================================
//
// CENTER PRINTING
//
//=============================================================================
// 
static char     scr_centerstring[MAX_STRING_CHARS];
static unsigned scr_centertime_start;   // for slow victory printing
static int      scr_center_lines;

//
//===============
// SCR_CenterPrint
// 
// Places the given string in the centerprint buffer which will be rendered
// to screen.
//===============
//
void SCR_CenterPrint(const char* str)
{
    const char* s;

    scr_centertime_start = clgi.GetRealTime();
    if (!strcmp(scr_centerstring, str)) {
        return;
    }

    Q_strlcpy(scr_centerstring, str, sizeof(scr_centerstring));

    // count the number of lines for centering
    scr_center_lines = 1;
    s = str;
    while (*s) {
        if (*s == '\n')
            scr_center_lines++;
        s++;
    }

    // echo it to the console
    Com_LPrintf(PRINT_ALL, "%s\n", scr_centerstring);

    // N&C: We call into the client so it can clear the previous notify times.
    // (these are set at realtime).
    clgi.Con_ClearNotify();
}

//
//===============
// SCR_DrawCenterString
// 
// Takes care of actually drawing the center print string.
//===============
//
void SCR_DrawCenterString(void)
{
    int y;
    float alpha;

    clgi.Cvar_ClampValue(scr_centertime, 0.3f, 10.0f);

    alpha = SCR_FadeAlpha(scr_centertime_start, scr_centertime->value * 1000, 300);
    if (!alpha) {
        return;
    }

    clgi.R_SetAlpha(alpha * scr_alpha->value);

    y = scr.hud_height / 4 - scr_center_lines * 8 / 2;

    SCR_DrawStringMulti(scr.hud_width / 2, y, UI_CENTER,
        MAX_STRING_CHARS, scr_centerstring, scr.font_pic);

    clgi.R_SetAlpha(scr_alpha->value);
}

//
//=============================================================================
//
// CROSSHAIR
//
//=============================================================================
// 

//
//===============
// scr_crosshair_changed
// 
// Called when the crosshair cvar changes. It'll reload the crosshair
// based on the value of scr_crosshair
//===============
//
static void scr_crosshair_changed(cvar_t* self)
{
    char buffer[16];
    int w, h;
    float scale;

    if (scr_crosshair->integer > 0) {
        Q_snprintf(buffer, sizeof(buffer), "ch%i", scr_crosshair->integer);
        scr.crosshair_pic = clgi.R_RegisterPic(buffer);
        clgi.R_GetPicSize(&w, &h, scr.crosshair_pic);

        // prescale
        scale = clgi.Cvar_ClampValue(ch_scale, 0.1f, 9.0f);
        scr.crosshair_width = w * scale;
        scr.crosshair_height = h * scale;
        if (scr.crosshair_width < 1)
            scr.crosshair_width = 1;
        if (scr.crosshair_height < 1)
            scr.crosshair_height = 1;

        if (ch_health->integer) {
            SCR_SetCrosshairColor();
        }
        else {
            scr.crosshair_color.u8[0] = (byte)(ch_red->value * 255);
            scr.crosshair_color.u8[1] = (byte)(ch_green->value * 255);
            scr.crosshair_color.u8[2] = (byte)(ch_blue->value * 255);
        }
        scr.crosshair_color.u8[3] = (byte)(ch_alpha->value * 255);
    }
    else {
        scr.crosshair_pic = 0;
    }
}

//
//===============
// SCR_SetCrosshairColor
// 
// Called each time by DeltaFrame
//===============
//
void SCR_SetCrosshairColor(void)
{
    int health;

    if (!ch_health->integer) {
        return;
    }

    health = cl->frame.playerState.stats[STAT_HEALTH];
    if (health <= 0) {
        VectorSet(scr.crosshair_color.u8, 0, 0, 0);
        return;
    }

    // red
    scr.crosshair_color.u8[0] = 255;

    // green
    if (health >= 66) {
        scr.crosshair_color.u8[1] = 255;
    }
    else if (health < 33) {
        scr.crosshair_color.u8[1] = 0;
    }
    else {
        scr.crosshair_color.u8[1] = (255 * (health - 33)) / 33;
    }

    // blue
    if (health >= 99) {
        scr.crosshair_color.u8[2] = 255;
    }
    else if (health < 66) {
        scr.crosshair_color.u8[2] = 0;
    }
    else {
        scr.crosshair_color.u8[2] = (255 * (health - 66)) / 33;
    }
}

/*
===============================================================================

DRAW OBJECTS

===============================================================================
*/

typedef struct {
    list_t          entry;
    int             x, y;
    cvar_t* cvar;
    cmd_macro_t* macro;
    int             flags;
    color_t         color;
} drawobj_t;

#define FOR_EACH_DRAWOBJ(obj) \
    LIST_FOR_EACH(drawobj_t, obj, &scr_objects, entry)
#define FOR_EACH_DRAWOBJ_SAFE(obj, next) \
    LIST_FOR_EACH_SAFE(drawobj_t, obj, next, &scr_objects, entry)

static LIST_DECL(scr_objects);

static void SCR_Color_g(genctx_t* ctx)
{
    int color;

    for (color = 0; color < 10; color++) {
        if (!clgi.Prompt_AddMatch(ctx, colorNames[color])) {
            break;
        }
    }
}

static void SCR_Draw_c(genctx_t* ctx, int argnum)
{
    if (argnum == 1) {
        clgi.Cvar_Variable_g(ctx);
        clgi.Cmd_Macro_g(ctx);
    }
    else if (argnum == 4) {
        SCR_Color_g(ctx);
    }
}

// draw cl_fps -1 80
static void SCR_Draw_f(void)
{
    int x, y;
    const char* s, * c;
    drawobj_t* obj;
    cmd_macro_t* macro;
    cvar_t* cvar;
    color_t color;
    int flags;
    int argc = clgi.Cmd_Argc();

    if (argc == 1) {
        if (LIST_EMPTY(&scr_objects)) {
            Com_Print("No draw strings registered.\n");
            return;
        }
        Com_Print("Name               X    Y\n"
            "--------------- ---- ----\n");
        FOR_EACH_DRAWOBJ(obj) {
            s = obj->macro ? obj->macro->name : obj->cvar->name;
            Com_Print("%-15s %4d %4d\n", s, obj->x, obj->y);
        }
        return;
    }

    if (argc < 4) {
        Com_Print("Usage: %s <name> <x> <y> [color]\n", clgi.Cmd_Argv(0));
        return;
    }

    color.u32 = U32_BLACK;
    flags = UI_IGNORECOLOR;

    s = clgi.Cmd_Argv(1);
    x = atoi(clgi.Cmd_Argv(2));
    y = atoi(clgi.Cmd_Argv(3));

    if (x < 0) {
        flags |= UI_RIGHT;
    }

    if (argc > 4) {
        c = clgi.Cmd_Argv(4);
        if (!strcmp(c, "alt")) {
            flags |= UI_ALTCOLOR;
        }
        else if (strcmp(c, "none")) {
            if (!SCR_ParseColor(c, &color)) {
                Com_Print("Unknown color '%s'\n", c);
                return;
            }
            flags &= ~UI_IGNORECOLOR;
        }
    }

    cvar = NULL;
    macro = clgi.Cmd_FindMacro(s);
    if (!macro) {
        cvar = clgi.Cvar_WeakGet(s);
    }

    FOR_EACH_DRAWOBJ(obj) {
        if (obj->macro == macro && obj->cvar == cvar) {
            obj->x = x;
            obj->y = y;
            obj->flags = flags;
            obj->color.u32 = color.u32;
            return;
        }
    }

    obj = (drawobj_t*)clgi.Z_TagMalloc(sizeof(*obj), TAG_GENERAL); // CPP: Cast
    obj->x = x;
    obj->y = y;
    obj->cvar = cvar;
    obj->macro = macro;
    obj->flags = flags;
    obj->color.u32 = color.u32;

    List_Append(&scr_objects, &obj->entry);
}

static void SCR_Draw_g(genctx_t* ctx)
{
    drawobj_t* obj;
    const char* s;

    if (LIST_EMPTY(&scr_objects)) {
        return;
    }

    clgi.Prompt_AddMatch(ctx, "all");

    FOR_EACH_DRAWOBJ(obj) {
        s = obj->macro ? obj->macro->name : obj->cvar->name;
        if (!clgi.Prompt_AddMatch(ctx, s)) {
            break;
        }
    }
}

static void SCR_UnDraw_c(genctx_t* ctx, int argnum)
{
    if (argnum == 1) {
        SCR_Draw_g(ctx);
    }
}

static void SCR_UnDraw_f(void)
{
    const char* s; // C++20: STRING: Added const to char*
    drawobj_t* obj, * next;
    cmd_macro_t* macro;
    cvar_t* cvar;

    if (clgi.Cmd_Argc() != 2) {
        Com_Print("Usage: %s <name>\n", clgi.Cmd_Argv(0));
        return;
    }

    if (LIST_EMPTY(&scr_objects)) {
        Com_Print("No draw strings registered.\n");
        return;
    }

    s = clgi.Cmd_Argv(1);
    if (!strcmp(s, "all")) {
        FOR_EACH_DRAWOBJ_SAFE(obj, next) {
            clgi.Z_Free(obj);
        }
        List_Init(&scr_objects);
        Com_Print("Deleted all draw strings.\n");
        return;
    }

    cvar = NULL;
    macro = clgi.Cmd_FindMacro(s);
    if (!macro) {
        cvar = clgi.Cvar_WeakGet(s);
    }

    FOR_EACH_DRAWOBJ_SAFE(obj, next) {
        if (obj->macro == macro && obj->cvar == cvar) {
            List_Remove(&obj->entry);
            clgi.Z_Free(obj);
            return;
        }
    }

    Com_Print("Draw string '%s' not found.\n", s);
}

void SCR_DrawObjects(void)
{
    char buffer[MAX_QPATH];
    int x, y;
    drawobj_t* obj;

    FOR_EACH_DRAWOBJ(obj) {
        x = obj->x;
        y = obj->y;
        if (x < 0) {
            x += scr.hud_width + 1;
        }
        if (y < 0) {
            y += scr.hud_height - CHAR_HEIGHT + 1;
        }
        if (!(obj->flags & UI_IGNORECOLOR)) {
            clgi.R_SetColor(obj->color.u32);
        }
        if (obj->macro) {
            obj->macro->function(buffer, sizeof(buffer));
            SCR_DrawString(x, y, obj->flags, buffer);
        }
        else {
            SCR_DrawString(x, y, obj->flags, obj->cvar->string);
        }
        if (!(obj->flags & UI_IGNORECOLOR)) {
            clgi.R_ClearColor();
            clgi.R_SetAlpha(scr_alpha->value);
        }
    }
}

void SCR_DrawFPS(void)
{
    if (scr_fps->integer == 0)
        return;

    int fps = clgi.GetFramesPerSecond();
    int scale = clgi.GetResolutionScale();

    char buffer[MAX_QPATH];
    if (scr_fps->integer == 2 && vid_rtx->integer)
        Q_snprintf(buffer, MAX_QPATH, "%d FPS at %3d%%", fps, scale);
    else
        Q_snprintf(buffer, MAX_QPATH, "%d FPS", fps);

    int x = scr.hud_width - 2;
    int y = 1;

    clgi.R_SetColor(~0u);
    SCR_DrawString(x, y, UI_RIGHT, buffer);
}

//
//=============================================================================
//
// CHAT HUD.
//
//=============================================================================
// 

#define MAX_CHAT_TEXT       150
#define MAX_CHAT_LINES      32
#define CHAT_LINE_MASK      (MAX_CHAT_LINES - 1)

typedef struct {
    char        text[MAX_CHAT_TEXT];
    unsigned    time;
} chatline_t;

static chatline_t   scr_chatlines[MAX_CHAT_LINES];
static unsigned     scr_chathead;

void SCR_ClearChatHUD_f(void)
{
    memset(scr_chatlines, 0, sizeof(scr_chatlines));
    scr_chathead = 0;
}

void SCR_AddToChatHUD(const char* text)
{
    chatline_t* line;
    char* p;

    line = &scr_chatlines[scr_chathead++ & CHAT_LINE_MASK];
    Q_strlcpy(line->text, text, sizeof(line->text));
    line->time = clgi.GetRealTime();

    p = strrchr(line->text, '\n');
    if (p)
        *p = 0;
}

void SCR_DrawChatHUD(void)
{
    int x, y, flags, step;
    unsigned i, lines, time;
    float alpha;
    chatline_t* line;

    if (scr_chathud->integer == 0)
        return;

    x = scr_chathud_x->integer;
    y = scr_chathud_y->integer;

    if (scr_chathud->integer == 2)
        flags = UI_ALTCOLOR;
    else
        flags = 0;

    if (x < 0) {
        x += scr.hud_width + 1;
        flags |= UI_RIGHT;
    }
    else {
        flags |= UI_LEFT;
    }

    if (y < 0) {
        y += scr.hud_height - CHAR_HEIGHT + 1;
        step = -CHAR_HEIGHT;
    }
    else {
        step = CHAR_HEIGHT;
    }

    lines = scr_chathud_lines->integer;
    if (lines > scr_chathead)
        lines = scr_chathead;

    time = scr_chathud_time->value * 1000;

    for (i = 0; i < lines; i++) {
        line = &scr_chatlines[(scr_chathead - i - 1) & CHAT_LINE_MASK];

        if (time) {
            alpha = SCR_FadeAlpha(line->time, time, 1000);
            if (!alpha)
                break;

            clgi.R_SetAlpha(alpha * scr_alpha->value);
            SCR_DrawString(x, y, flags, line->text);
            clgi.R_SetAlpha(scr_alpha->value);
        }
        else {
            SCR_DrawString(x, y, flags, line->text);
        }

        y += step;
    }
}


//
//=============================================================================
//
// MEDIA REGISTRATION.
//
//=============================================================================
// 
void SCR_RegisterMedia(void)
{
    int     i, j;

    // N&C: We register these here, once again. The client has its own
    // internally. However, we want to give the CG Module the option to
    // register their own, and for that... We have this piece of code around :)
    for (i = 0; i < 2; i++)
        for (j = 0; j < STAT_PICS; j++)
            scr.sb_pics[i][j] = clgi.R_RegisterPic(sb_nums[i][j]);

    // Register inventory and field pictures.
    scr.inven_pic = clgi.R_RegisterPic("inventory");
    scr.field_pic = clgi.R_RegisterPic("field_3");

    // Register pause screen picture and fetch its size info.
    scr.pause_pic = clgi.R_RegisterPic("pause");
    clgi.R_GetPicSize(&scr.pause_width, &scr.pause_height, scr.pause_pic);

    // Register load screen picture and fetch its size info.
    scr.loading_pic = clgi.R_RegisterPic("loading");
    clgi.R_GetPicSize(&scr.loading_width, &scr.loading_height, scr.loading_pic);

    // Register font pic. (Has unless modified, already been registered by the client.)
    scr.font_pic = clgi.R_RegisterFont(scr_font->string);

    // Crosshair chan
    scr_crosshair_changed(scr_crosshair);
}


//
//=============================================================================
//
// STAT PROGRAMS.
//
//=============================================================================
// 

#define ICON_WIDTH  24
#define ICON_HEIGHT 24
#define DIGIT_WIDTH 16
#define ICON_SPACE  8

#define HUD_DrawString(x, y, string) \
    clgi.R_DrawString(x, y, 0, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawAltString(x, y, string) \
    clgi.R_DrawString(x, y, UI_XORCOLOR, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawCenterString(x, y, string) \
    SCR_DrawStringMulti(x, y, UI_CENTER, MAX_STRING_CHARS, string, scr.font_pic)

#define HUD_DrawAltCenterString(x, y, string) \
    SCR_DrawStringMulti(x, y, UI_CENTER | UI_XORCOLOR, MAX_STRING_CHARS, string, scr.font_pic)

static void HUD_DrawNumber(int x, int y, int color, int width, int value)
{
    char    num[16], * ptr;
    int     l;
    int     frame;

    if (width < 1)
        return;

    // draw number string
    if (width > 5)
        width = 5;

    color &= 1;

    l = Q_scnprintf(num, sizeof(num), "%i", value);
    if (l > width)
        l = width;
    x += 2 + DIGIT_WIDTH * (width - l);

    ptr = num;
    while (*ptr && l) {
        if (*ptr == '-')
            frame = STAT_MINUS;
        else
            frame = *ptr - '0';

        clgi.R_DrawPic(x, y, scr.sb_pics[color][frame]);
        x += DIGIT_WIDTH;
        ptr++;
        l--;
    }
}

#define DISPLAY_ITEMS   17

void SCR_DrawInventory(void)
{
    int     i;
    int     num, selected_num, item;
    int     index[MAX_ITEMS];
    char    string[MAX_STRING_CHARS];
    int     x, y;
    const char* bind; // C++20: STRING: Added const to char*
    int     selected;
    int     top;

    if (!(cl->frame.playerState.stats[STAT_LAYOUTS] & 2))
        return;

    selected = cl->frame.playerState.stats[STAT_SELECTED_ITEM];

    num = 0;
    selected_num = 0;
    for (i = 0; i < MAX_ITEMS; i++) {
        if (i == selected) {
            selected_num = num;
        }
        if (cl->inventory[i]) {
            index[num++] = i;
        }
    }

    // determine scroll point
    top = selected_num - DISPLAY_ITEMS / 2;
    if (top > num - DISPLAY_ITEMS) {
        top = num - DISPLAY_ITEMS;
    }
    if (top < 0) {
        top = 0;
    }

    x = (scr.hud_width - 256) / 2;
    y = (scr.hud_height - 240) / 2;

    clgi.R_DrawPic(x, y + 8, scr.inven_pic);
    y += 24;
    x += 24;

    HUD_DrawString(x, y, "hotkey ### item");
    y += CHAR_HEIGHT;

    HUD_DrawString(x, y, "------ --- ----");
    y += CHAR_HEIGHT;

    for (i = top; i < num && i < top + DISPLAY_ITEMS; i++) {
        item = index[i];
        // search for a binding
        Q_concat(string, sizeof(string),
            "use ", cl->configstrings[ConfigStrings::Items+ item], NULL);
        bind = clgi.Key_GetBinding(string);

        Q_snprintf(string, sizeof(string), "%6s %3i %s",
            bind, cl->inventory[item], cl->configstrings[ConfigStrings::Items+ item]);

        if (item != selected) {
            HUD_DrawAltString(x, y, string);
        }
        else {    // draw a blinky cursor by the selected item
            HUD_DrawString(x, y, string);
            if ((clgi.GetRealTime() >> 8) & 1) {
                clgi.R_DrawChar(x - CHAR_WIDTH, y, 0, 15, scr.font_pic);
            }
        }

        y += CHAR_HEIGHT;
    }
}

static void SCR_DrawSelectedItemName(int x, int y, int item)
{
    static int display_item = -1;
    static int display_start_time = 0;

    float duration = 0.f;
    if (display_item != item)
    {
        display_start_time = clgi.Sys_Milliseconds();
        display_item = item;
    }
    else
    {
        duration = (float)(clgi.Sys_Milliseconds() - display_start_time) * 0.001f;
    }

    float alpha;
    if (scr_showitemname->integer < 2)
        alpha = max(0.f, min(1.f, 5.f - 4.f * duration)); // show and hide
    else
        alpha = 1; // always show

    if (alpha > 0.f)
    {
        clgi.R_SetAlpha(alpha * scr_alpha->value);

        int index = ConfigStrings::Items+ item;
        HUD_DrawString(x, y, cl->configstrings[index]);

        clgi.R_SetAlpha(scr_alpha->value);
    }
}

static void SCR_ExecuteLayoutString(const char* s)
{
    char    buffer[MAX_QPATH];
    int     x, y;
    int     value;
    char* token;
    int     width;
    int     index;
    ClientInfo* ci;

    if (!s[0])
        return;

    x = 0;
    y = 0;

    while (s) {
        token = COM_Parse(&s);
        if (token[2] == 0) {
            if (token[0] == 'x') {
                if (token[1] == 'l') {
                    token = COM_Parse(&s);
                    x = atoi(token);
                    continue;
                }

                if (token[1] == 'r') {
                    token = COM_Parse(&s);
                    x = scr.hud_width + atoi(token);
                    continue;
                }

                if (token[1] == 'v') {
                    token = COM_Parse(&s);
                    x = scr.hud_width / 2 - 160 + atoi(token);
                    continue;
                }
            }

            if (token[0] == 'y') {
                if (token[1] == 't') {
                    token = COM_Parse(&s);
                    y = atoi(token);
                    continue;
                }

                if (token[1] == 'b') {
                    token = COM_Parse(&s);
                    y = scr.hud_height + atoi(token);
                    continue;
                }

                if (token[1] == 'v') {
                    token = COM_Parse(&s);
                    y = scr.hud_height / 2 - 120 + atoi(token);
                    continue;
                }
            }
        }

        if (!strcmp(token, "pic")) {
            // draw a pic from a stat number
            token = COM_Parse(&s);
            value = atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            index = cl->frame.playerState.stats[value];
            if (index < 0 || index >= MAX_IMAGES) {
                Com_Error(ERR_DROP, "%s: invalid pic index", __func__);
            }
            token = cl->configstrings[ConfigStrings::Images+ index];
            if (token[0] && cl->precaches.images[index]) {
                clgi.R_DrawPic(x, y, cl->precaches.images[index]);
            }

            if (value == STAT_SELECTED_ICON && scr_showitemname->integer)
            {
                SCR_DrawSelectedItemName(x + 32, y + 8, cl->frame.playerState.stats[STAT_SELECTED_ITEM]);
            }
            continue;
        }

        if (!strcmp(token, "client")) {
            // draw a deathmatch client block
            int     score, ping, time;

            token = COM_Parse(&s);
            x = scr.hud_width / 2 - 160 + atoi(token);
            token = COM_Parse(&s);
            y = scr.hud_height / 2 - 120 + atoi(token);

            token = COM_Parse(&s);
            value = atoi(token);
            if (value < 0 || value >= MAX_CLIENTS) {
                Com_Error(ERR_DROP, "%s: invalid client index", __func__);
            }
            ci = &cl->clientInfo[value];

            token = COM_Parse(&s);
            score = atoi(token);

            token = COM_Parse(&s);
            ping = atoi(token);

            token = COM_Parse(&s);
            time = atoi(token);

            HUD_DrawAltString(x + 32, y, ci->name);
            HUD_DrawString(x + 32, y + CHAR_HEIGHT, "Score: ");
            Q_snprintf(buffer, sizeof(buffer), "%i", score);
            HUD_DrawAltString(x + 32 + 7 * CHAR_WIDTH, y + CHAR_HEIGHT, buffer);
            Q_snprintf(buffer, sizeof(buffer), "Ping:  %i", ping);
            HUD_DrawString(x + 32, y + 2 * CHAR_HEIGHT, buffer);
            Q_snprintf(buffer, sizeof(buffer), "Time:  %i", time);
            HUD_DrawString(x + 32, y + 3 * CHAR_HEIGHT, buffer);

            if (!ci->icon) {
                ci = &cl->baseClientInfo;
            }
            clgi.R_DrawPic(x, y, ci->icon);
            continue;
        }

        if (!strcmp(token, "ctf")) {
            // draw a ctf client block
            int     score, ping;

            token = COM_Parse(&s);
            x = scr.hud_width / 2 - 160 + atoi(token);
            token = COM_Parse(&s);
            y = scr.hud_height / 2 - 120 + atoi(token);

            token = COM_Parse(&s);
            value = atoi(token);
            if (value < 0 || value >= MAX_CLIENTS) {
                Com_Error(ERR_DROP, "%s: invalid client index", __func__);
            }
            ci = &cl->clientInfo[value];

            token = COM_Parse(&s);
            score = atoi(token);

            token = COM_Parse(&s);
            ping = atoi(token);
            if (ping > 999)
                ping = 999;

            Q_snprintf(buffer, sizeof(buffer), "%3d %3d %-12.12s",
                score, ping, ci->name);
            if (value == cl->frame.clientNumber) {
                HUD_DrawAltString(x, y, buffer);
            }
            else {
                HUD_DrawString(x, y, buffer);
            }
            continue;
        }

        if (!strcmp(token, "picn")) {
            // draw a pic from a name
            token = COM_Parse(&s);
            clgi.R_DrawPic(x, y, clgi.R_RegisterPic2(token));
            continue;
        }

        if (!strcmp(token, "num")) {
            // draw a number
            token = COM_Parse(&s);
            width = atoi(token);
            token = COM_Parse(&s);
            value = atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            value = cl->frame.playerState.stats[value];
            HUD_DrawNumber(x, y, 0, width, value);
            continue;
        }

        if (!strcmp(token, "hnum")) {
            // health number
            int     color;

            width = 3;
            value = cl->frame.playerState.stats[STAT_HEALTH];
            if (value > 25)
                color = 0;  // green
            else if (value > 0)
                color = ((cl->frame.number / CLG_FRAMEDIV) >> 2) & 1;     // flash
            else
                color = 1;

            if (cl->frame.playerState.stats[STAT_FLASHES] & 1)
                clgi.R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strcmp(token, "anum")) {
            // ammo number
            int     color;

            width = 3;
            value = cl->frame.playerState.stats[STAT_AMMO];
            if (value > 5)
                color = 0;  // green
            else if (value >= 0)
                color = ((cl->frame.number / CLG_FRAMEDIV) >> 2) & 1;     // flash
            else
                continue;   // negative number = don't show

            if (cl->frame.playerState.stats[STAT_FLASHES] & 4)
                clgi.R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strcmp(token, "rnum")) {
            // armor number
            int     color;

            width = 3;
            value = cl->frame.playerState.stats[STAT_ARMOR];
            if (value < 1)
                continue;

            color = 0;  // green

            if (cl->frame.playerState.stats[STAT_FLASHES] & 2)
                clgi.R_DrawPic(x, y, scr.field_pic);

            HUD_DrawNumber(x, y, color, width, value);
            continue;
        }

        if (!strcmp(token, "stat_string")) {
            token = COM_Parse(&s);
            index = atoi(token);
            if (index < 0 || index >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            index = cl->frame.playerState.stats[index];
            if (index < 0 || index >= ConfigStrings::MaxConfigStrings) {
                Com_Error(ERR_DROP, "%s: invalid string index", __func__);
            }
            HUD_DrawString(x, y, cl->configstrings[index]);
            continue;
        }

        if (!strcmp(token, "cstring")) {
            token = COM_Parse(&s);
            HUD_DrawCenterString(x + 320 / 2, y, token);
            continue;
        }

        if (!strcmp(token, "cstring2")) {
            token = COM_Parse(&s);
            HUD_DrawAltCenterString(x + 320 / 2, y, token);
            continue;
        }

        if (!strcmp(token, "string")) {
            token = COM_Parse(&s);
            HUD_DrawString(x, y, token);
            continue;
        }

        if (!strcmp(token, "string2")) {
            token = COM_Parse(&s);
            HUD_DrawAltString(x, y, token);
            continue;
        }

        if (!strcmp(token, "if")) {
            token = COM_Parse(&s);
            value = atoi(token);
            if (value < 0 || value >= MAX_STATS) {
                Com_Error(ERR_DROP, "%s: invalid stat index", __func__);
            }
            value = cl->frame.playerState.stats[value];
            if (!value) {   // skip to endif
                while (strcmp(token, "endif")) {
                    token = COM_Parse(&s);
                    if (!s) {
                        break;
                    }
                }
            }
            continue;
        }

        if (!strcmp(token, "color")) {
            color_t     color;

            token = COM_Parse(&s);
            if (SCR_ParseColor(token, &color)) {
                color.u8[3] *= scr_alpha->value;
                clgi.R_SetColor(color.u32);
            }
            continue;
        }
    }

    clgi.R_ClearColor();
    clgi.R_SetAlpha(scr_alpha->value);
}


//=============================================================================


void SCR_DrawCrosshair(void)
{
    int x, y;

    if (!scr_crosshair->integer)
        return;

    x = (scr.hud_width - scr.crosshair_width) / 2;
    y = (scr.hud_height - scr.crosshair_height) / 2;

    clgi.R_SetColor(scr.crosshair_color.u32);

    clgi.R_DrawStretchPic(x + ch_x->integer,
        y + ch_y->integer,
        scr.crosshair_width,
        scr.crosshair_height,
        scr.crosshair_pic);
}

// The status bar is a small layout program that is based on the stats array
void SCR_DrawStats(void)
{
    if (scr_draw2d->integer <= 1)
        return;

    SCR_ExecuteLayoutString(cl->configstrings[ConfigStrings::StatusBar]);
}

void SCR_DrawLayout(void)
{
    if (scr_draw2d->integer == 3 && !clgi.Key_IsDown(K_F1))
        return;     // turn off for GTV

    if (clgi.IsDemoPlayback() && clgi.Key_IsDown(K_F1))
        goto draw;

    if (!(cl->frame.playerState.stats[STAT_LAYOUTS] & 1))
        return;

draw:
    SCR_ExecuteLayoutString(cl->layout);
}

//
//=============================================================================
//
// CLIENT MODULE SCREEN COMMAND FUNCTIONS.
//
//=============================================================================
// 
/*
=================
SCR_Sky_f

Set a specific sky and rotation speed. If empty sky name is provided, falls
back to server defaults.
=================
*/
static void SCR_Sky_f(void)
{
    const char* name; // C++20: STRING: Added const to char*
    float   rotate;
    vec3_t  axis;
    int     argc = clgi.Cmd_Argc();

    if (argc < 2) { 
        Com_Print("Usage: sky <basename> [rotate] [axis x y z]\n");
        return;
    }

    if (clgi.GetClienState() != ClientConnectionState::Active) {
        Com_Print("No map loaded.\n");
        return;
    }

    name = clgi.Cmd_Argv(1);
    if (!*name) {
        CLG_SetSky();
        return;
    }

    if (argc > 2)
        rotate = atof(clgi.Cmd_Argv(2));
    else
        rotate = 0;

    if (argc == 6) {
        axis[0] = atof(clgi.Cmd_Argv(3));
        axis[1] = atof(clgi.Cmd_Argv(4));
        axis[2] = atof(clgi.Cmd_Argv(5));
    }
    else
        VectorSet(axis, 0, 0, 1);

    clgi.R_SetSky(name, rotate, axis);
}

void SCR_CalcVRect(void) {
    scr_vrect.width = scr.hud_width;
    scr_vrect.height = scr.hud_height;
    scr_vrect.x = 0;
    scr_vrect.y = 0;
}


//
//=============================================================================
//
// CLIENT MODULE SCREEN ENTRY FUNCTIONS.
//
//=============================================================================
//
static const cmdreg_t scr_cmds[] = {
    //{ "timerefresh", SCR_TimeRefresh_f },
    //{ "sizeup", SCR_SizeUp_f },
    //{ "sizedown", SCR_SizeDown_f },
    { "sky", SCR_Sky_f },
    { "draw", SCR_Draw_f, SCR_Draw_c },
    { "undraw", SCR_UnDraw_f, SCR_UnDraw_c },
    { "clearchathud", SCR_ClearChatHUD_f },
    { NULL }
};

static void scr_scale_changed(cvar_t* self)
{
    scr.hud_scale = clgi.R_ClampScale(self);
}

//
//===============
// SCR_Init
// 
// 
//================
//
void SCR_Init(void)
{
    // Fetch CVars from client.
    scr_viewsize    = clgi.Cvar_Get("viewsize", NULL, 0);
    scr_draw2d      = clgi.Cvar_Get("scr_draw2d", NULL, 0);
    scr_alpha       = clgi.Cvar_Get("scr_alpha", NULL, 0);
    scr_font        = clgi.Cvar_Get("scr_font", NULL, 0);
    scr_fps         = clgi.Cvar_Get("scr_fps", NULL, 0);

    // Create CVars.
    scr_scale               = clgi.Cvar_Get("scr_scale", "2", 0);
    scr_scale->changed      = scr_scale_changed;

    scr_showitemname        = clgi.Cvar_Get("scr_showitemname", "1", CVAR_ARCHIVE);

    scr_centertime          = clgi.Cvar_Get("scr_centertime", "2.5", 0);

    scr_crosshair           = clgi.Cvar_Get("crosshair", "0", CVAR_ARCHIVE);
    scr_crosshair->changed  = scr_crosshair_changed;

    ch_scale            = clgi.Cvar_Get("ch_scale", "1", 0);
    ch_scale->changed   = scr_crosshair_changed;
    ch_x                = clgi.Cvar_Get("ch_x", "0", 0);
    ch_y                = clgi.Cvar_Get("ch_y", "0", 0);

    scr_chathud         = clgi.Cvar_Get("scr_chathud", "0", 0);
    scr_chathud_lines   = clgi.Cvar_Get("scr_chathud_lines", "4", 0);
    scr_chathud_time    = clgi.Cvar_Get("scr_chathud_time", "0", 0);
    scr_chathud_x       = clgi.Cvar_Get("scr_chathud_x", "8", 0);
    scr_chathud_y       = clgi.Cvar_Get("scr_chathud_y", "-64", 0);

    ch_health           = clgi.Cvar_Get("ch_health", "0", 0);
    ch_health->changed  = scr_crosshair_changed;
    ch_red              = clgi.Cvar_Get("ch_red", "1", 0);
    ch_red->changed     = scr_crosshair_changed;
    ch_green            = clgi.Cvar_Get("ch_green", "1", 0);
    ch_green->changed   = scr_crosshair_changed;
    ch_blue             = clgi.Cvar_Get("ch_blue", "1", 0);
    ch_blue->changed    = scr_crosshair_changed;
    ch_alpha            = clgi.Cvar_Get("ch_alpha", "1", 0);
    ch_alpha->changed   = scr_crosshair_changed;

    // Register commands.
    clgi.Cmd_Register(scr_cmds);

    // Scale init.
    scr_scale_changed(scr_scale);

    // We've initialized the screen.
    scr.initialized = true;
}

//
//===============
// CLG_ScreenModeChanged
//
// 
//===============
//
void CLG_ScreenModeChanged(void) {
    if (scr.initialized)
        scr.hud_scale = clgi.R_ClampScale(scr_scale);

    scr.hud_alpha = 1.f;
}

//
//===============
// CLG_RenderScreen
//
// 
//===============
//
void CLG_RenderScreen(void) {
    // start with full screen HUD
    scr.hud_width = cl->refdef.width;
    scr.hud_height = cl->refdef.height;

    // Calculate view rectangle.
    SCR_CalcVRect();

    clgi.R_SetAlphaScale(scr.hud_alpha);

    clgi.R_SetScale(scr.hud_scale);

    scr.hud_height *= scr.hud_scale;
    scr.hud_width *= scr.hud_scale;

    // Render the crosshair.
    SCR_DrawCrosshair();

    // the rest of 2D elements share common alpha
    clgi.R_ClearColor();
    clgi.R_SetAlpha(clgi.Cvar_ClampValue(scr_alpha, 0, 1));

    // Draw status.
    SCR_DrawStats();

    // Draw layout string.
    SCR_DrawLayout();

    // Draw inventory.
    SCR_DrawInventory();

    // Draw center screen print
    SCR_DrawCenterString();

    // Draw objects.
    SCR_DrawObjects();

    // Draw FPS.
    SCR_DrawFPS();

    // Draw Chat Hud.
    SCR_DrawChatHUD();
}

//
//===============
// CLG_DrawLoadScreen
// 
// 
//================
//
void CLG_DrawLoadScreen(void) {
    int x, y;

    clgi.R_SetScale(scr.hud_scale);

    x = (cl->refdef.width * scr.hud_scale - scr.loading_width) / 2; //x = (r_config.width * scr.hud_scale - scr.loading_width) / 2;
    y = (cl->refdef.height * scr.hud_scale - scr.loading_height) / 2; //y = (r_config.height * scr.hud_scale - scr.loading_height) / 2;

    clgi.R_DrawPic(x, y, scr.loading_pic);
    clgi.R_SetScale(1.0f);
}

//
//===============
// CLG_DrawPauseScreen
// 
// 
//================
//
void CLG_DrawPauseScreen(void) {
    int x, y;

    x = (scr.hud_width - scr.pause_width) / 2;
    y = (scr.hud_height - scr.pause_height) / 2;

    clgi.R_DrawPic(x, y, scr.pause_pic);
}

//
//===============
// SCR_Shutdown
// 
// 
//================
//
void SCR_Shutdown(void)
{
    clgi.Cmd_Unregister(scr_cmds);
    scr.initialized = false;
}
