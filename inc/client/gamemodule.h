/*
// LICENSE HERE.

//
// gamemodule.h
//
// Contains declarations of all the functions concerning interaction with
// the client game module.
//
*/
#ifndef __CLIENT_CLGMODULE_H__
#define __CLIENT_CLGMODULE_H__

#include "shared/shared.h"
#include "sharedgame/pmove.h" // PMOVE: Remove once the game modules init pmove themselves using CLG_ParseServerData.
//
// cgmodule.c
//
extern "C" void CL_ShutdownGameProgs(void);    // N&C: Place elsewhere?
extern "C" void CL_InitGameProgs(void);        // N&C: Place elsewhere? // CPP: Extern "C"

//
// Core
//
void        CL_GM_Init();
void        CL_GM_Shutdown(void);

float		CL_GM_CalcFOV(float fov_x, float width, float height);
void		CL_GM_ClientUpdateOrigin(void);
void		CL_GM_ClientBegin(void);
void		CL_GM_ClientDeltaFrame(void);
void		CL_GM_ClientFrame(void);
void		CL_GM_ClientDisconnect(void);
void		CL_GM_ClientClearState(void);
void		CL_GM_DemoSeek(void);
void		CL_GM_ClientUpdateUserInfo(cvar_t* var, from_t from);

//
// Entities.
//
void		CL_GM_EntityEvent(int32_t number);

//
// Media
//
void        CL_GM_InitMedia(void);
const char	*CL_GM_GetMediaLoadStateName(LoadState state);
void        CL_GM_LoadScreenMedia(void);
void        CL_GM_LoadWorldMedia(void);
void        CL_GM_ShutdownMedia(void);

//
// Movement.
// 
void CL_GM_BuildFrameMoveCommand(int32_t msec);
void CL_GM_FinalizeFrameMoveCommand(void);

//
// Predict
//
void		CL_GM_CheckPredictionError(ClientMoveCommand* moveCommand);
void		CL_GM_PredictAngles(void);
void		CL_GM_PredictMovement(uint32_t acknowledgedCommandIndex, uint32_t currentCommandIndex);

//
// Parse
//
void		CL_GM_ParsePlayerSkin(char* name, char* model, char* skin, const char* s);
qboolean	CL_GM_UpdateConfigString(int index, const char* str);

void		CL_GM_StartServerMessage(void);
qboolean	CL_GM_ParseServerMessage(int32_t serverCommand);
qboolean    CL_GM_SeekDemoMessage(int32_t demoCommand);
void		CL_GM_EndServerMessage(void);

//
// Screen
//
void		CL_GM_RenderScreen(void);
void		CL_GM_ScreenModeChanged(void);

void		CL_GM_DrawLoadScreen(void);
void		CL_GM_DrawPauseScreen(void);

//
// View
//
void		CL_GM_ClearScene(void);

void		CL_GM_PreRenderView(void);
void		CL_GM_RenderView(void);
void		CL_GM_PostRenderView(void);

#endif //  __CLIENT_CGMODULE_H__