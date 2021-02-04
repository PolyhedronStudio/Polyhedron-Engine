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

// Shared Client Game Header. 
#include "shared/clgame.h"

//
//=============================================================================
//
//	Client Game structures and definitions.
//
//=============================================================================
//
extern clg_import_t clgi;

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
// clg_tests.c
//
void CLG_ExecuteTests (void);

#endif // __CLGAME_LOCAL_H__