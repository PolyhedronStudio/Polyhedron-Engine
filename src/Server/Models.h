/***
*
*	License here.
*
*	@file
*
*	Brings Model loading to the server, allowing it to pass a model pointer
*	to the SharedGame Skeletal Animatio Data loading system.
* 
***/
#pragma once

void SV_Model_BeginRegistrationSequence();
void SV_Model_EndRegistrationSequence();
void SV_Model_Init();
void SV_Model_Shutdown();
model_t *SV_Model_Alloc(void);
void SV_Model_List_f(void);
void SV_Model_FreeUnused();
void SV_Model_FreeAll();

void SV_Model_Reference(model_t* model);

model_t* SV_Model_Find(const char* name);
model_t *SV_Model_ForHandle(qhandle_t handle);
qhandle_t SV_Model_PrecacheModel(const char* name);