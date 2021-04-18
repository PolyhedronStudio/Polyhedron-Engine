/*
// LICENSE HERE.

//
// clgame/clg_screen.h
//
*/

#ifndef __CLGAME_SCREEN_H__
#define __CLGAME_SCREEN_H__

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

#endif // __CLGAME_SCREEN_H__