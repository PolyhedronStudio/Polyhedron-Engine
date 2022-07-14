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
//#include "Common/Models/Models.h"
#include "Client/Models.h"
#include "System/Hunk.h"
#include "Shared/Formats/Md2.h"
#if USE_MD3
#include "Shared/Formats/Md3.h"
#endif
#include "Shared/Formats/Sp2.h"
#include "Shared/Formats/Iqm.h"



//================================================================================
// TODO: Move elsewhere, also for MD2 and MD3.
qerror_t MOD_LoadIQM_Base(model_t* model, const void* rawdata, size_t length, const char* mod_name);
//================================================================================

//! Server Side Model Registration Sequence:
//! Each time a map change occurs it increments the sequence.
//! All models that have an unmatching sequence afterwards are freed.
int32_t sv_registration_sequence = 1;

//! Server Side Model maximum.
static constexpr int32_t MAX_SVMODELS = 512;
//! Server Side Model Data.
model_t			sv_models[MAX_SVMODELS];
//! Number of total loaded server side model data.
int32_t			sv_numModels = 0;

//! Client Side storage of our internal Skeletal Model Data.
//! Indexed by the same handle as the r_models model it belongs to.
SkeletalModelData sv_skeletalModels[MAX_SVMODELS];



/**
*
*	Registration Sequence.
*
**/
/**
*	@brief	Callback for allocating Server Side Memory when loading model data.
**/
void *SV_Model_MemoryAllocate(memhunk_t *hunk, size_t size) {
	return Hunk_Alloc(hunk, size);// Z_TagMalloc(size, TAG_SERVER);
}

/**
*	@brief	Begins a new registration sequence for the server model cache.
**/
void SV_Model_BeginRegistrationSequence() {
	// Increase registration sequence.
	sv_registration_sequence++;
}

/**
*	@brief	Ends the registration sequence, and frees all unused(non referenced) model cache data.
**/
void SV_Model_EndRegistrationSequence() {
	// Free Unused data.
	SV_Model_FreeUnused();
}

/**
*	@brief	Finds the first free slot in line and returns a pointer to its
*			model data object.
*	@return	A pointer to a model_t. A (nullptr) on failure.
**/
model_t *SV_Model_Alloc(void)
{
	model_t *model = nullptr;
	int32_t i = 0;

	// Find the first free slot.
	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		// Found it, our pointer is set, so break out of our loop.
		if (!model->type) {
			break;
		}
	}

	// If there was no earlier on freed slot.
	if (i == sv_numModels) {
		// Ensure we aren't exceeding cache size.
		if (sv_numModels == MAX_SVMODELS) {
			return nullptr;
		}
		// Good to go, increase the number of models we got.
		sv_numModels++;
	}

	// And, if we got to here, return the pointer.
	return model;
}

/**
*	@brief	Scans through all models to see if their names match, returning a pointer to it.
*	@return	Pointer to the model in the cache which has a matching name. (nullptr) otherwise.
*/
model_t* SV_Model_Find(const char* name) {
	model_t *model = nullptr;
	int32_t i = 0;

	for (i = 0, model = sv_models; i < sv_numModels; i++, model++) {
		// If it isn't free, skip on to the next index.
		if (!model->type) {
			continue;
		}
		// Names match, return our pointer.
		if (!FS_pathcmp(model->name, name)) {
			return model;
		}
	}

	// If we got here, we didn 't find it.
	return nullptr;
}

/**
*	@brief	List all cached Server Model Data.
**/
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
	Com_Printf("Total server models: %d (out of %d slots)\n", count, sv_numModels);
	Com_Printf("Total server resident: %" PRIz "\n", bytes); // CPP: String fix.
}

/**
*	@brief	Free all unused model cache data.
**/
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

/**
*	@brief	Free all Server Model Data.
**/
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

/**
*	@brief	References a model so that it won't get unloaded after the registration sequence.
*			(This is called by PF_PrecacheSkeletalModelData)
**/
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

/**
*	@brief	Finds a model that has a matching handle.
*	@return A pointer to the model. (nullptr) on failure.
*/
model_t *SV_Model_ForHandle(qhandle_t handle) {
	// Need a valid handle.
	if (!handle) {
		return NULL;
	}

	// Ensure handle is within boundaries.
	if (handle < 0 || handle > sv_numModels) {
		Com_Error(ErrorType::Drop, "%s: %d out of range", __func__, handle);
	}

	// Ensure it's not a free slot handle.
	model_t *model = &sv_models[handle - 1];
	if (!model->type) {
		return nullptr;
	}

	// Got it.
	return model;
}

/**
*	@brief	Initializes server side model caching system.
**/
void SV_Model_Init() {
	if (sv_numModels) {
		Com_Error(ErrorType::Fatal, "%s: %d models not freed", __func__, sv_numModels);
	}

	Cmd_AddCommand("sv_modellist", SV_Model_List_f);
}

/**
*	@brief	Shutsdown server side model caching, clears all memory.
**/
void SV_Model_Shutdown() {
	SV_Model_FreeAll();
	Cmd_RemoveCommand("sv_modellist");
}

qhandle_t SV_Model_PrecacheSkeletalModelData(const char* name) {
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
		load = MOD_LoadIQM_Base;
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

	// Client uses Hunk_Alloc (Used to be a define CL_Model_Alloc).
	ret = load(model, SV_Model_MemoryAllocate, rawdata, filelen, name);
	FS_FreeFile(rawdata);

	if (ret != Q_ERR_SUCCESS) {
		Hunk_Free(&model->hunk);
		memset(model, 0, sizeof(*model));
		goto fail1;
	}


// TODO: Check for special model type stuff here.
//#if USE_CLIENT && !USE_SERVER
//	model->model_class = get_model_class(model->name);
//#endif

done:
	// Calculate the index.
	index = (model - sv_models) + 1;

	// Assign the skeletal model data struct as a pointer to this model_t
	model->skeletalModelData = &sv_skeletalModels[index - 1];

	// Generate Skeletal Model Data.
	SKM_GenerateModelData(model);

	return index;
fail2:
	FS_FreeFile(rawdata);
fail1:
	Com_EPrintf("Couldn't load %s: %s\n", normalized, Q_ErrorString(ret));
	return 0;
}
