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

#include "../Shared/Shared.h"
//#include "Game/Shared/PlayerPMove.h" // PMOVE: Remove once the game modules init pmove themselves using CLG_ParseServerData.

//
// cgmodule.c
//
extern "C" void CL_ShutdownGameProgs(void);    // WID: Place elsewhere?
extern "C" void CL_InitGameProgs(void);        // WID: Place elsewhere? // CPP: Extern "C"

//
// Core
//
void        CL_GM_Init();
void        CL_GM_Shutdown(void);

float		CL_GM_CalcFOV(float fov_x, float width, float height);
void		CL_GM_ClientUpdateOrigin(void);
/**
*	@brief	Let the Client Game Module know we connected. This gives it a chance to create
*			objects that are relevant to the client game right before all settles and the
*			client game begins.
**/
void		CL_GM_ClientConnect();
void		CL_GM_ClientBegin(void);
void		CL_GM_ClientPacketEntityDeltaFrame(void);
/**
*   @brief  Runs the local entity logic for a single client game frame.
*	@return	The current actual hashed classname of the PODEntity's game object. 
*			If it has no object, 0 will be returned instead.
**/
uint32_t	CL_GM_GetHashedGameEntityClassname(PODEntity *podEntity);
void		CL_GM_ClientLocalEntitiesFrame(void);
void		CL_GM_ClientFrame(void);
void		CL_GM_ClientDisconnect(void);
void		CL_GM_ClientClearState(void);
void		CL_GM_DemoSeek(void);
void		CL_GM_CheckEntityPresent(int32_t entityNumber, const std::string& what);
void		CL_GM_ClientUpdateUserInfo(cvar_t* var, from_t from);

//
// Entities.
//
qboolean	CL_GM_SpawnEntitiesFromBSPString(const char *mapName, const char *entities);
qboolean	CL_GM_CreateFromNewState(PODEntity *clEntity, const EntityState *state);
void		CL_GM_PacketEntityEvent(int32_t number);
void		CL_GM_LocalEntityEvent(int32_t number);

//
// Media
//
void        CL_GM_InitMedia(void);
const char	*CL_GM_GetMediaLoadStateName(int32_t loadState);
void        CL_GM_RegisterScreenMedia(void);
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
