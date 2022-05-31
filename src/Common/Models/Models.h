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

#define MOD_Malloc(size)    Hunk_Alloc(&model->hunk, size)

#define MAX_ALIAS_SKINS     32
#define MAX_ALIAS_VERTS     4096

// WATISDEZE: Moved over to Shared/refresh.h since client game dll 
// needs to know about these.
// typedef struct mspriteframe_s {
//     int             width, height;
//     int             origin_x, origin_y;
//     struct image_s  *image;
// } mspriteframe_t;

// typedef enum
// {
// 	MCLASS_REGULAR,
// 	MCLASS_EXPLOSION,
// 	MCLASS_SMOKE,
//     MCLASS_STATIC_LIGHT,
//     MCLASS_FLARE
// } model_class_t;

// typedef struct model_s {
//     enum {
//         MOD_FREE,
//         MOD_ALIAS,
//         MOD_SPRITE,
//         MOD_EMPTY
//     } type;
//     char name[MAX_QPATH];
//     int registration_sequence;
//     memhunk_t hunk;

//     // alias models
//     int numframes;
//     struct maliasframe_s *frames;
// #if USE_REF == REF_GL || USE_REF == REF_VKPT
//     int nummeshes;
//     struct maliasmesh_s *meshes;
// 	model_class_t model_class;
// #else
//     int numskins;
//     struct image_s *skins[MAX_ALIAS_SKINS];
//     int numtris;
//     struct maliastri_s *tris;
//     int numsts;
//     struct maliasst_s *sts;
//     int numverts;
//     int skinwidth;
//     int skinheight;
// #endif

//     // sprite models
//     struct mspriteframe_s *spriteframes;
// 	qboolean sprite_vertical;
// 	qboolean sprite_fxup;
// 	qboolean sprite_fxft;
// 	qboolean sprite_fxlt;
	   
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

qerror_t MOD_LoadSP2(model_t* model, const void* rawdata, size_t length, const char* mod_name);
qerror_t MOD_ValidateMD2(struct dmd2header_s *header, size_t length);
qerror_t MOD_LoadIQM_Base(model_t* mod, const void* rawdata, size_t length, const char* mod_name);
qboolean R_ComputeIQMTransforms(const iqm_model_t* model, const r_entity_t* entity, float* pose_matrices);

// these are implemented in [gl,sw]_models.c
typedef qerror_t(*mod_load_t)(model_t*, const void*, size_t, const char*);
extern qerror_t(*MOD_LoadMD2)(model_t* model, const void* rawdata, size_t length, const char* mod_name);
#if USE_MD3
extern qerror_t(*MOD_LoadMD3)(model_t* model, const void* rawdata, size_t length, const char* mod_name);
#endif
extern qerror_t(*MOD_LoadIQM)(model_t* model, const void* rawdata, size_t length, const char* mod_name);
extern void (*MOD_Reference)(model_t *model);