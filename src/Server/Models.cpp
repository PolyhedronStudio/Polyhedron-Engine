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
#include "Server.h"
#include "Models.h"
#include "Common/Models/Models.h"
#include "System/Hunk.h"
#include "Shared/Formats/Md2.h"
#if USE_MD3
#include "Shared/Formats/Md3.h"
#endif
#include "Shared/Formats/Sp2.h"
#include "Shared/Formats/Iqm.h"

// TODO: Move elsewhere, also for MD2 and MD3.
qerror_t MOD_LoadIQM_Base(model_t* model, const void* rawdata, size_t length, const char* mod_name);

/**
*	The following is compile only with dedicated server builds.
*
*	For a client build, it has them defined elsewhere.
**/
//#if USE_SERVER && !USE_CLIENT
////model_t      r_models[];
////int          r_numModels;

//#endif
#define MAX_SVMODELS 512
model_t			sv_models[MAX_SVMODELS];
int32_t			sv_numModels = 0;

#if USE_SERVER && !USE_CLIENT
int registration_sequence = 0;
#endif
int32_t sv_registration_sequence = 1;


/**
*	Empty place-holders for when building WITH client.
**/
//#if USE_SERVER && USE_CLIENT
void SV_Model_BeginRegistrationSequence() {
	sv_registration_sequence++;
}
void SV_Model_EndRegistrationSequence() {
	SV_Model_FreeUnused();
}

void *SV_Model_MemoryAllocate(memhunk_t *hunk, size_t size) {
	return Hunk_Alloc(hunk, size);// Z_TagMalloc(size, TAG_SERVER);
}

model_t *SV_Model_Alloc(void)
{
	model_t *model;
	int i;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		if (!model->type) {
			break;
		}
	}

	if (i == sv_numModels) {
		if (sv_numModels == MAX_SVMODELS) {
			return NULL;
		}
		sv_numModels++;
	}

	return model;
}

model_t* SV_Model_Find(const char* name) {
	model_t *model;
	int i;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		if (!FS_pathcmp(model->name, name)) {
			return model;
		}
	}

	return NULL;
}

void SV_Model_List_f(void)
{
	static const char types[] = "FASE"; // CPP: Cast - was types[4] = "FASE";
	int     i, count;
	model_t *model;
	size_t  bytes;

	Com_Printf("------------------\n");
	bytes = count = 0;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		Com_Printf("%c %8" PRIz " : %s\n", types[model->type], // CPP: String fix.
			model->hunk.mapped, model->name);
		bytes += model->hunk.mapped;
		count++;
	}
	Com_Printf("Total server models: %d (out of %d slots)\n", count, r_numModels);
	Com_Printf("Total server resident: %" PRIz "\n", bytes); // CPP: String fix.
}

void SV_Model_FreeUnused() {
	model_t *model;
	int i;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		if (model->registration_sequence == sv_registration_sequence) {
			// make sure it is paged in
			Com_PageInMemory(model->hunk.base, model->hunk.currentSize);
		}
		else {
			// don't need this model
			Hunk_Free(&model->hunk);
			memset(model, 0, sizeof(*model));
			//Z_Free(model);
		}
	}
}
void SV_Model_FreeAll() {
	model_t *model;
	int i;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}

		Hunk_Free(&model->hunk);
		//Z_Free(model);
		memset(model, 0, sizeof(*model));
	}

	sv_numModels = 0;
}

void SV_Model_Reference(model_t* model) {
	int mesh_idx, skin_idx, frame_idx;

	// TODO: Register materials used by models here?
	switch (model->type) {
	case model_t::MOD_ALIAS:

		//for (mesh_idx = 0; mesh_idx < model->nummeshes; mesh_idx++) {
		//	maliasmesh_t *mesh = &model->meshes[mesh_idx];
		//	for (skin_idx = 0; skin_idx < mesh->numskins; skin_idx++) {
		//		MAT_UpdateRegistration(mesh->materials[skin_idx]);
		//	}
		//}
		break;
	case model_t::MOD_SPRITE:
		//for (frame_idx = 0; frame_idx < model->numframes; frame_idx++) {
		//	model->spriteframes[frame_idx].image->registration_sequence = registration_sequence;
		//}
		break;
	case model_t::MOD_EMPTY:
		break;
	default:
		Com_Error(ErrorType::Fatal, "%s: bad model type", __func__);
	}

	model->registration_sequence = sv_registration_sequence;
}

model_t *SV_Model_ForHandle(qhandle_t handle) {
	model_t *model;

	if (!handle) {
		return NULL;
	}

	if (handle < 0 || handle > sv_numModels) {
		int x = 10;

		Com_Error(ErrorType::Drop, "%s: %d out of range", __func__, handle);
	}

	model = &sv_models[handle - 1];
	if (!model->type) {
		return NULL;
	}

	return model;
}

void SV_Model_Init() {
	if (sv_numModels) {
		Com_Error(ErrorType::Fatal, "%s: %d models not freed", __func__, r_numModels);
	}

	Cmd_AddCommand("servermodellist", SV_Model_List_f);
}

void SV_Model_Shutdown() {
	SV_Model_FreeAll();
	Cmd_RemoveCommand("servermodellist");
}

qhandle_t SV_Model_PrecacheModel(const char* name) {
	char normalized[MAX_QPATH];
	qhandle_t index;
	size_t namelen;
	ssize_t filelen;
	model_t *model;
	byte *rawdata = NULL;
	uint32_t ident;
	mod_load_t load;
	qerror_t ret;

	// Empty names are legal, silently ignore them.
	if (!*name) {
		return 0;
	}

	// Can't do inline BSP models.
	if (*name == '*') {
		return 0;
	}

	// Normalize the path.
	namelen = FS_NormalizePathBuffer(normalized, name, MAX_QPATH);

	// this should never happen
	if (namelen >= MAX_QPATH) {
		Com_Error(ErrorType::Drop, "%s: oversize name", __func__);
	}

	// Normalized to empty name?
	if (namelen == 0) {
		Com_DPrintf("%s: empty name\n", __func__);
		return 0;
	}

	// See if it's already loaded.
	char* extension = normalized + namelen - 4; // C++20: Moved up in case we need to.
	
	// Only register IQM models.
	if (namelen > 4 && (strcmp(extension, ".iqm") != 0) ){
		return 0;
	}

	model = SV_Model_Find(normalized);
	if (model) {
//#if USE_SERVER && !USE_CLIENT
		SV_Model_Reference(model);
//#else
		
//#endif
		goto done;
	}

#if USE_CLIENT && !USE_SERVER
	if (namelen > 4 && (strcmp(extension, ".md2") == 0) && (vid_rtx && gl_use_hd_assets) && (vid_rtx->integer || gl_use_hd_assets->integer))
#else
	if (namelen > 4 && (strcmp(extension, ".md2") == 0))
#endif
	{
		memcpy(extension, ".md3", 4);

		filelen = FS_LoadFile(normalized, (void **)&rawdata);

		memcpy(extension, ".md2", 4);
	}

	if (!rawdata)
	{
		filelen = FS_LoadFile(normalized, (void **)&rawdata);
		if (!rawdata) {
			// don't spam about missing models
			if (filelen == Q_ERR_NOENT) {
				return 0;
			}

			ret = filelen;
			goto fail1;
		}
	}

	if (filelen < 4) {
		ret = Q_ERR_FILE_TOO_SMALL;
		goto fail2;
	}

	// check ident
	ident = LittleLong(*(uint32_t *)rawdata);
	switch (ident) {
	case MD2_IDENT:
		load = 0; //MOD_LoadMD2;
		break;
#if USE_MD3
	case MD3_IDENT:
		load = 0; //MOD_LoadMD3;
		break;
#endif
	case SP2_IDENT:
		load = 0; //MOD_LoadSP2;
		break;
	case IQM_IDENT:
//#if USE_SERVER && !USE_CLIENT
		load = MOD_LoadIQM_Base;
//#else
//		load = MOD_LoadIQM;
//#endif
		break;
	default:
		ret = Q_ERR_UNKNOWN_FORMAT;
		goto fail2;
	}

	model = SV_Model_Alloc();
	if (!model) {
		ret = Q_ERR_OUT_OF_SLOTS;
		goto fail2;
	}

	memcpy(model->name, normalized, namelen + 1);
	model->registration_sequence = sv_registration_sequence;

	Hunk_Begin(&model->hunk, 0x4000000);
	model->type = model_t::MOD_ALIAS;

	//qerror_t res = MOD_LoadIQM_Base(model, modelAlloc, rawdata, length, mod_name);

	// Client uses Hunk_Alloc (Used to be a define MOD_Alloc).
	ret = load(model, SV_Model_MemoryAllocate, rawdata, filelen, name);

	//if (res != Q_ERR_SUCCESS) 	{
	//	
	//	return res;
	//}
	FS_FreeFile(rawdata);

	if (ret != Q_ERR_SUCCESS) {
		Hunk_Free(&model->hunk);
		memset(model, 0, sizeof(*model));
		goto fail1;
	}

//#if USE_CLIENT && !USE_SERVER
//	model->model_class = get_model_class(model->name);
//#endif

done:
	index = (model - sv_models) + 1;
	return index;
//#if USE_CLIENT
//#if USE_REF == REF_VKPT
//	int register_model_dirty;
//	register_model_dirty = 1;
//#endif
//#endif


fail2:
	FS_FreeFile(rawdata);
fail1:
	Com_EPrintf("Couldn't load %s: %s\n", normalized, Q_ErrorString(ret));
	return 0;
}

///**
//*	Empty place-holders for when building WITH client.
//**/
//#if USE_SERVER && USE_CLIENT
//void SV_Model_Init() {}
//void SV_Model_Shutdown() {}
//model_t* SV_Model_Find(const char* name) {
//	return MOD_Find(name);
//}
//void SV_Model_FreeUnused() {}
//void SV_Model_Reference(model_t* model) {
//	MOD_Reference(model);
//}
//qhandle_t SV_Model_PrecacheModel(const char* name) {
//		char normalized[MAX_QPATH];
//	qhandle_t index;
//	size_t namelen;
//	// Empty names are legal, silently ignore them.
//	if (!*name) {
//		return 0;
//	}
//
//	// Can't do inline BSP models.
//	if (*name == '*') {
//		return 0;
//	}
//
//	// Normalize the path.
//	namelen = FS_NormalizePathBuffer(normalized, name, MAX_QPATH);
//
//	// this should never happen
//	if (namelen >= MAX_QPATH) {
//		Com_Error(ErrorType::Drop, "%s: oversize name", __func__);
//	}
//
//	// Normalized to empty name?
//	if (namelen == 0) {
//		Com_DPrintf("%s: empty name\n", __func__);
//		return 0;
//	}
//
//	// See if it's already loaded.
//	char* extension = normalized + namelen - 4; // C++20: Moved up in case we need to.
//	
//	// Only register IQM models.
//	if (namelen > 4 && (strcmp(extension, ".iqm") != 0) ) {
//		return 0;
//	}
//
//	return R_RegisterModel(name);
//}
//#endif
//
///**
//*	Actual implementation for initializing model system in our server.
//**/
//#if USE_SERVER && !USE_CLIENT
//void SV_Model_Init() {
//	MOD_Init();
//}
//void SV_Model_Shutdown() {
//	MOD_Shutdown();
//}
//void SV_Model_Reference(model_t* model) {
//	// TODO: Implement?
//	//MOD_Reference(model);
//}
//
//model_t* SV_Model_Find(const char* name) {
//	char normalized[MAX_QPATH];
//	qhandle_t index;
//	size_t namelen;
//	ssize_t filelen;
//	model_t *model;
//	byte *rawdata = NULL;
//	uint32_t ident;
//	mod_load_t load;
//	qerror_t ret;
//
//	// Empty names are legal, silently ignore them.
//	if (!*name) {
//		return 0;
//	}
//
//	// Can't do inline BSP models.
//	if (*name == '*') {
//		return 0;
//	}
//
//	// Normalize the path.
//	namelen = FS_NormalizePathBuffer(normalized, name, MAX_QPATH);
//
//	// this should never happen
//	if (namelen >= MAX_QPATH) {
//		Com_Error(ErrorType::Drop, "%s: oversize name", __func__);
//	}
//
//	// Normalized to empty name?
//	if (namelen == 0) {
//		Com_DPrintf("%s: empty name\n", __func__);
//		return 0;
//	}
//
//	//char* extension = normalized + namelen - 4; // C++20: Moved up in case we need to.
//	
//	return MOD_Find(normalized);
//}
//void SV_Model_FreeUnused() {
//	MOD_FreeUnused();
//}
//
//qhandle_t SV_Model_PrecacheModel(const char* name) {
//	char normalized[MAX_QPATH];
//	qhandle_t index;
//	size_t namelen;
//	ssize_t filelen;
//	model_t *model;
//	byte *rawdata = NULL;
//	uint32_t ident;
//	mod_load_t load;
//	qerror_t ret;
//
//	// Empty names are legal, silently ignore them.
//	if (!*name) {
//		return 0;
//	}
//
//	// Can't do inline BSP models.
//	if (*name == '*') {
//		return 0;
//	}
//
//	// Normalize the path.
//	namelen = FS_NormalizePathBuffer(normalized, name, MAX_QPATH);
//
//	// this should never happen
//	if (namelen >= MAX_QPATH) {
//		Com_Error(ErrorType::Drop, "%s: oversize name", __func__);
//	}
//
//	// Normalized to empty name?
//	if (namelen == 0) {
//		Com_DPrintf("%s: empty name\n", __func__);
//		return 0;
//	}
//
//	// See if it's already loaded.
//	char* extension = normalized + namelen - 4; // C++20: Moved up in case we need to.
//	
//	// Only register IQM models.
//	if (namelen > 4 && (strcmp(extension, ".iqm") != 0) ){
//		return 0;
//	}
//
//	model = MOD_Find(normalized);
//	if (model) {
////#if USE_SERVER && !USE_CLIENT
//		SV_Model_Reference(model);
////#else
//		
////#endif
//		goto done;
//	}
//
//#if USE_CLIENT && !USE_SERVER
//	if (namelen > 4 && (strcmp(extension, ".md2") == 0) && (vid_rtx && gl_use_hd_assets) && (vid_rtx->integer || gl_use_hd_assets->integer))
//#else
//	if (namelen > 4 && (strcmp(extension, ".md2") == 0))
//#endif
//	{
//		memcpy(extension, ".md3", 4);
//
//		filelen = FS_LoadFile(normalized, (void **)&rawdata);
//
//		memcpy(extension, ".md2", 4);
//	}
//
//	if (!rawdata)
//	{
//		filelen = FS_LoadFile(normalized, (void **)&rawdata);
//		if (!rawdata) {
//			// don't spam about missing models
//			if (filelen == Q_ERR_NOENT) {
//				return 0;
//			}
//
//			ret = filelen;
//			goto fail1;
//		}
//	}
//
//	if (filelen < 4) {
//		ret = Q_ERR_FILE_TOO_SMALL;
//		goto fail2;
//	}
//
//	// check ident
//	ident = LittleLong(*(uint32_t *)rawdata);
//	switch (ident) {
//	case MD2_IDENT:
//		load = 0; //MOD_LoadMD2;
//		break;
//#if USE_MD3
//	case MD3_IDENT:
//		load = 0; //MOD_LoadMD3;
//		break;
//#endif
//	case SP2_IDENT:
//		load = 0; //MOD_LoadSP2;
//		break;
//	case IQM_IDENT:
//#if USE_SERVER && !USE_CLIENT
//		load = MOD_LoadIQM_Base;
//#else
//		load = MOD_LoadIQM;
//#endif
//		break;
//	default:
//		ret = Q_ERR_UNKNOWN_FORMAT;
//		goto fail2;
//	}
//
//	model = MOD_Alloc();
//	if (!model) {
//		ret = Q_ERR_OUT_OF_SLOTS;
//		goto fail2;
//	}
//
//	memcpy(model->name, normalized, namelen + 1);
//	model->registration_sequence = registration_sequence;
//
//	ret = load(model, rawdata, filelen, name);
//
//	FS_FreeFile(rawdata);
//
//	if (ret) {
//		memset(model, 0, sizeof(*model));
//		goto fail1;
//	}
//
//#if USE_CLIENT && !USE_SERVER
//	model->model_class = get_model_class(model->name);
//#endif
//
//done:
//	index = (model - r_models) + 1;
//#if USE_CLIENT
//#if USE_REF == REF_VKPT
//	int register_model_dirty;
//	register_model_dirty = 1;
//#endif
//#endif
//	return index;
//
//fail2:
//	FS_FreeFile(rawdata);
//fail1:
//	Com_EPrintf("Couldn't load %s: %s\n", normalized, Q_ErrorString(ret));
//	return 0;
//}
//
//#endif