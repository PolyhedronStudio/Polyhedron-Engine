/***
*
*	License here.
*
*	@file
* 
*   Client Game Temp Entities.
*
***/
#pragma once

void CLG_ParseTempEntity(void);
void CLG_SmokeAndFlash(vec3_t origin);

void CLG_RegisterTempEntityModels(void);
void CLG_RegisterTempEntitySounds(void);

void CLG_ClearTempEntities(void);
void CLG_AddTempEntities(void);
void CLG_InitTempEntities(void);

/**
*	Game - Specific to the game itself.
**/
// Stores parameters parsed from a temporary entity message.s
extern tent_params_t   teParameters;
// Stores parameters parsed from a muzzleflash message.
extern mz_params_t     mzParameters;