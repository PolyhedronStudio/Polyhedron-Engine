/*
// LICENSE HERE.

//
// clgame/clg_tents.h
//
*/

#ifndef __CLGAME_TENTS_H__
#define __CLGAME_TENTS_H__

void CLG_ParseTempEntity(void);
void CLG_SmokeAndFlash(vec3_t origin);

void CLG_RegisterTempEntityModels(void);
void CLG_RegisterTempEntitySounds(void);

void CLG_ClearTempEntities(void);
void CLG_AddTempEntities(void);
void CLG_InitTempEntities(void);

#endif // __CLGAME_TENTS_H__

