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

// WID: TODO: Place elsewhere ofc. 
void CLG_ParseInventory(void);
void CLG_ParseLayout(void);
void CLG_ParseTempEntitiesPacket(void);
void CLG_ParseMuzzleFlashPacket(int32_t mask);
void CLG_ParsePrint(void);
void CLG_ParseCenterPrint(void);

#endif // __CLGAME_PARSE_H__