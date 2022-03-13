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

//-----
// View Rectangle.
//-----
rect_t     scr_vrect;      // position of render window on screen

// Color table containing actual color codes that match their string names.
const uint32_t colorTable[8] = {
    U32_BLACK, U32_RED, U32_GREEN, U32_YELLOW,
    U32_ORANGE, U32_CYAN, U32_MAGENTA, U32_WHITE
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
    //size_t len = strlen(s);

    //if (len > maxlen) {
    //    len = maxlen;
    //}

    //if ((flags & UI_CENTER) == UI_CENTER) {
    //    x -= len * CHAR_WIDTH / 2;
    //}
    //else if (flags & UI_RIGHT) {
    //    x -= len * CHAR_WIDTH;
    //}

    //return clgi.R_DrawString(x, y, flags, maxlen, s, font);
    return 0;
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
    //char* p;
    //size_t  len;

    //while (*s) {
    //    p = (char*)strchr(s, '\n'); // CPP: Cast
    //    if (!p) {
    //        SCR_DrawStringEx(x, y, flags, maxlen, s, font);
    //        break;
    //    }

    //    len = p - s;
    //    if (len > maxlen) {
    //        len = maxlen;
    //    }
    //    SCR_DrawStringEx(x, y, flags, len, s, font);

    //    y += CHAR_HEIGHT;
    //    s = p + 1;
    //}
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
// Before adding color orange.
//const char* const colorNames[10] = {
//    "black", "red", "green", "yellow",
//    "blue", "cyan", "magenta", "white",
//    "alt", "none"
//};
const char* const colorNames[10] = {
    "black", "red", "green", "yellow",
    "orange", "cyan", "magenta", "white",
    "alt", "none"
};

enum color_index_t {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_ORANGE, // Used to be: COLOR_BLUE
    COLOR_CYAN,
    COLOR_MAGENTA, // COLOR_...
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
// string color names: black, red, green, yellow, orange, cyan, magenta, white,
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
            c[i] = PH_CharHex(s[i]);
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
    //const char* s;

    //scr_centertime_start = clgi.GetRealTime();
    //if (!strcmp(scr_centerstring, str)) {
    //    return;
    //}

    //Q_strlcpy(scr_centerstring, str, sizeof(scr_centerstring));

    //// count the number of lines for centering
    //scr_center_lines = 1;
    //s = str;
    //while (*s) {
    //    if (*s == '\n')
    //        scr_center_lines++;
    //    s++;
    //}

    //// echo it to the console
    //Com_LPrintf(PRINT_ALL, "%s\n", scr_centerstring);

    //// PH: We call into the client so it can clear the previous notify times.
    //// (these are set at realtime).
    //clgi.Con_ClearNotify();
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
    //int y;
    //float alpha;

    //clgi.Cvar_ClampValue(scr_centertime, 0.3f, 10.0f);

    //alpha = SCR_FadeAlpha(scr_centertime_start, scr_centertime->value * 1000, 300);
    //if (!alpha) {
    //    return;
    //}

    //clgi.R_SetAlpha(alpha * scr_alpha->value);

    //y = scr.hud_height / 4 - scr_center_lines * 8 / 2;

    //SCR_DrawStringMulti(scr.hud_width / 2, y, UI_CENTER,
    //    MAX_STRING_CHARS, scr_centerstring, scr.font_pic);

    //clgi.R_SetAlpha(scr_alpha->value);
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
    //memset(scr_chatlines, 0, sizeof(scr_chatlines));
    //scr_chathead = 0;
}

void SCR_AddToChatHUD(const char* text)
{
    //chatline_t* line;
    //char* p;

    //line = &scr_chatlines[scr_chathead++ & CHAT_LINE_MASK];
    //Q_strlcpy(line->text, text, sizeof(line->text));
    //line->time = clgi.GetRealTime();

    //p = strrchr(line->text, '\n');
    //if (p)
    //    *p = 0;
}

void SCR_DrawChatHUD(void)
{
    //int x, y, flags, step;
    //unsigned i, lines, time;
    //float alpha;
    //chatline_t* line;

    //if (scr_chathud->integer == 0)
    //    return;

    //x = scr_chathud_x->integer;
    //y = scr_chathud_y->integer;

    //if (scr_chathud->integer == 2)
    //    flags = UI_ALTCOLOR;
    //else
    //    flags = 0;

    //if (x < 0) {
    //    x += scr.hud_width + 1;
    //    flags |= UI_RIGHT;
    //}
    //else {
    //    flags |= UI_LEFT;
    //}

    //if (y < 0) {
    //    y += scr.hud_height - CHAR_HEIGHT + 1;
    //    step = -CHAR_HEIGHT;
    //}
    //else {
    //    step = CHAR_HEIGHT;
    //}

    //lines = scr_chathud_lines->integer;
    //if (lines > scr_chathead)
    //    lines = scr_chathead;

    //time = scr_chathud_time->value * 1000;

    //for (i = 0; i < lines; i++) {
    //    line = &scr_chatlines[(scr_chathead - i - 1) & CHAT_LINE_MASK];

    //    if (time) {
    //        alpha = SCR_FadeAlpha(line->time, time, 1000);
    //        if (!alpha)
    //            break;

    //        clgi.R_SetAlpha(alpha * scr_alpha->value);
    //        SCR_DrawString(x, y, flags, line->text);
    //        clgi.R_SetAlpha(scr_alpha->value);
    //    }
    //    else {
    //        SCR_DrawString(x, y, flags, line->text);
    //    }

    //    y += step;
    //}
}