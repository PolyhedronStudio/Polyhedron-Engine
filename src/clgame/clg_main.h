/*
// LICENSE HERE.

//
// clgame/clg_main.h
//
*/

#ifndef __CLGAME_MAIN_H__
#define __CLGAME_MAIN_H__

// Externs.
// Contains the function pointers being passed in from the engine.
extern ClientGameImport clgi;
// Static export variable, lives as long as the client game dll lives.
extern IClientGameExports* clge;

void CLG_Shutdown(void);

void CLG_ClientDeltaFrame(void);
void CLG_ClientFrame(void);
void CLG_ClientBegin(void);
void CLG_ClientDisconnect(void);
void CLG_ClearState(void);
void CLG_DemoSeek(void);

void Com_Print(const char* fmt, ...);
void Com_DPrint(const char* fmt, ...);
void Com_WPrint(const char* fmt, ...);
void Com_EPrint(const char* fmt, ...);
void Com_Error(ErrorType code, const char* fmt, ...);
void Com_LPrintf(PrintType type, const char* fmt, ...);

#endif // __CLGAME_MAIN_H__