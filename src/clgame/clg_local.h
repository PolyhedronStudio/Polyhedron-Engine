/*
// LICENSE HERE.

//
// clg_local.h
//
//
// Contains the definitions of the import, and export API structs.
//
*/

#ifndef __CLGAME_LOCAL_H__
#define __CLGAME_LOCAL_H__

// Define CGAME_INCLUDE so that files such as:
// common/cmodel.h
// common/cmd.h
//
#define CGAME_INCLUDE

// Shared.
#include "shared/shared.h"
#include "shared/list.h"
#include "shared/refresh.h"

// // Common.
// #include "common/cmodel.h"
// #include "common/cmd.h"
// #include "common/math.h"
#include "common/msg.h"
#include "common/pmove.h"
#include "common/protocol.h"

// Shared Client Game Header.
#include "shared/cl_types.h"
#include "shared/cl_game.h"


//
//=============================================================================
//
//	Client Game structures and definitions.
//
//=============================================================================
//
extern clg_import_t clgi;
extern client_state_t *cl;

//
//=============================================================================
//
//	Client Game Function declarations.
//
//=============================================================================
//

//
// clg_main.c
//
void CLG_Init (void);
void CLG_Shutdown (void);

void CLG_StartServerMessage (void);
qboolean CLG_ParseServerMessage (int serverCommand);
void CLG_EndServerMessage (int realTime);

void Com_Print(char *fmt, ...);
void Com_DPrint(char *fmt, ...);
void Com_WPrint(char *fmt, ...);
void Com_EPrint(char *fmt, ...);
void Com_Error (error_type_t code, char *fmt, ...);

//
// clg_media.c
//
void CLG_InitMedia (void);
void CLG_RegisterMedia (void);
void CLG_ShutdownMedia (void);

//
// clg_tests.c
//
void CLG_ExecuteTests (void);

//
// clg_view.c
//
void CLG_PreRenderView (void);
void CLG_RenderView (void);
void CLG_PostRenderView (void);

#endif // __CLGAME_LOCAL_H__