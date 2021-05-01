/*
// LICENSE HERE.

//
// clgame/clg_media.h
//
*/

#ifndef __CLGAME_MEDIA_H__
#define __CLGAME_MEDIA_H__

void CLG_RegisterVWepModels();
void CLG_LoadClientInfo(ClientInfo* ci, const char* str);
void CLG_SetSky(void);

void CLG_InitMedia(void);
char* CLG_GetMediaLoadStateName(LoadState state);
void CLG_LoadScreenMedia(void);
void CLG_LoadWorldMedia(void);
void CLG_ShutdownMedia(void);

#endif // __CLGAME_MEDIA_H__