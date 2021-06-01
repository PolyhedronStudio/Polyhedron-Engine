/*
// LICENSE HERE.

//
// clgame/clg_parse.h
//
*/

#ifndef __CLGAME_PARSE_H__
#define __CLGAME_PARSE_H__

qboolean CLG_UpdateConfigString(int index, const char* str);
void CLG_StartServerMessage(void);
qboolean CLG_ParseServerMessage(int serverCommand);
qboolean CLG_SeekDemoMessage(int demoCommand);
void CLG_EndServerMessage(int realTime);

#endif // __CLGAME_PARSE_H__