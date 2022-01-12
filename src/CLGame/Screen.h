/*
// LICENSE HERE.

//
// clgame/clg_screen.h
//
*/

#ifndef __CLGAME_SCREEN_H__
#define __CLGAME_SCREEN_H__

// Number of "sb_pics"
static constexpr int32_t STAT_PICS = 11;              // Number of "sb_pics"
static constexpr int32_t STAT_MINUS = STAT_PICS - 1; // Index into the sb_pics array pointing at the pic name for the '-'

// Our client game screen container struct.
struct RenderScreenData {
    qboolean    initialized;        // ready to draw

    qhandle_t   font_pic;

    qhandle_t   pause_pic;
    int         pause_width, pause_height;

    qhandle_t   loading_pic;
    int         loading_width, loading_height;

    qhandle_t   crosshair_pic;
    int         crosshair_width, crosshair_height;
    color_t     crosshair_color;

    qhandle_t   sb_pics[2][STAT_PICS];
    qhandle_t   inven_pic;
    qhandle_t   field_pic;

    int         hud_width, hud_height;
    float       hud_scale;
    float       hud_alpha;
};

extern RenderScreenData scr;

void SCR_SetCrosshairColor(void);
void SCR_AddToChatHUD(const char* text);
void SCR_CenterPrint(const char* str);
void SCR_ClearChatHUD_f(void);

void SCR_RegisterMedia(void);

void SCR_Init(void);
void SCR_Shutdown(void);

void CLG_RenderScreen(void);
void CLG_ScreenModeChanged(void);
void CLG_DrawLoadScreen(void);
void CLG_DrawPauseScreen(void);

// WID: TODO: ...
void SCR_CalcVRect(void);
void SCR_DrawCrosshair(void);
void SCR_DrawStats(void);
void SCR_DrawLayout(void);
void SCR_DrawInventory(void);
void SCR_DrawCenterString(void);
void SCR_DrawObjects(void);
void SCR_DrawFPS(void);
void SCR_DrawChatHUD(void);

#endif // __CLGAME_SCREEN_H__