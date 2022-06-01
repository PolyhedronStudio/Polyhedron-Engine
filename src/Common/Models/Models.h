/***
*
*	License here.
*
*	@file
*
*	Experimental file, goal: Allow for the server to load models, and in case of a
*	combined client/server build, let it load them once for both: client, and server,
*	if need be.
* 
***/
#pragma once

//
// models.h -- common models manager
//
#include "Shared/Refresh.h"
#include "System/Hunk.h"
#include "Common/Error.h"

//#define MOD_Malloc(size)    Hunk_Alloc(&model->hunk, size)

#define MAX_ALIAS_SKINS     32
#define MAX_ALIAS_VERTS     4096

// WATISDEZE: In case you came here for some refresh work, updating etc, these have been moved to Shared.
// needs to know about these.
// typedef struct mspriteframe_s {
// } mspriteframe_t;

// typedef enum
// {
// } model_class_t;

// typedef struct model_s {
// } model_t;
extern model_t      r_models[];
extern int          r_numModels;

extern int registration_sequence;

// these are implemented in r_models.c
model_t *MOD_Alloc(void);
model_t *MOD_Find(const char *name);
model_t *MOD_ForHandle(qhandle_t h);

void MOD_FreeUnused(void);
void MOD_FreeAll(void);
void MOD_Init(void);
void MOD_Shutdown(void);

struct dmd2header_s;

qerror_t MOD_LoadSP2(model_t* model, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name);
qerror_t MOD_ValidateMD2(struct dmd2header_s *header, size_t length);
qerror_t MOD_LoadIQM_Base(model_t* mod, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name);
qboolean R_ComputeIQMTransforms(const iqm_model_t* model, const r_entity_t* entity, float* pose_matrices);

// these are implemented in [gl,sw]_models.c
typedef qerror_t(*mod_load_t)(model_t*, ModelMemoryAllocateCallback modelAllocate, const void*, size_t, const char*);
extern qerror_t(*MOD_LoadMD2)(model_t* model, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name);
#if USE_MD3
extern qerror_t(*MOD_LoadMD3)(model_t* model, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name);
#endif
extern qerror_t(*MOD_LoadIQM)(model_t* model, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name);
extern void (*MOD_Reference)(model_t *model);