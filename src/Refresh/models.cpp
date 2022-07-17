/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2008 Andrey Nazarov
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "../Shared/Shared.h"
#include "../Shared/List.h"
#include "Common/Common.h"
#include "Common/Files.h"
#include "Client/Models.h"
//#include "Common/Models/Models.h"
#include "System/Hunk.h"
#include "Shared/Formats/Md2.h"
#if USE_MD3
#include "Shared/Formats/Md3.h"
#endif
#include "Shared/Formats/Sp2.h"
#include "Shared/Formats/Iqm.h"
#include "Refresh/Images.h"
#include "Refresh/Models.h"

extern cvar_t *vid_rtx;
extern cvar_t *gl_use_hd_assets;
extern SkeletalModelData r_skeletalModels[];

static model_class_t
get_model_class(const char *name)
{
	if (!strcmp(name, "models/objects/explode/tris.md2"))
		return MCLASS_EXPLOSION;
	else if (!strcmp(name, "models/objects/r_explode/tris.md2"))
		return MCLASS_EXPLOSION;
	else if (!strcmp(name, "models/objects/flash/tris.md2"))
		return MCLASS_SMOKE;
	else if (!strcmp(name, "models/objects/smoke/tris.md2"))
		return MCLASS_SMOKE;
	//else if (!strcmp(name, "models/objects/minelite/light2/tris.md2"))
	//	return MCLASS_STATIC_LIGHT;
	else if (!strcmp(name, "models/logo/poly_logo.iqm"))
		return MCLASS_STATIC_LIGHT;
	else if (!strcmp(name, "models/objects/flare/tris.md2"))
		return MCLASS_FLARE;
	else
		return MCLASS_REGULAR;
}

/**
*	@brief Replaces the old CL_Model_Alloc macro.
**/
void* R_MOD_Alloc(memhunk_t *hunk, size_t size) {
	return Hunk_Alloc(hunk, size);
}

qhandle_t R_RegisterModel(const char *name) {
	char normalized[MAX_QPATH];
	qhandle_t index;
	size_t namelen;
	ssize_t filelen;
	model_t *model;
	byte *rawdata = NULL;
	uint32_t ident = 0;
	mod_load_t load;
	qerror_t ret;

	// empty names are legal, silently ignore them
	if (!*name)
		return 0;

	if (*name == '*') {
		// inline bsp model
		index = atoi(name + 1);
		return ~index;
	}

	// normalize the path
	namelen = FS_NormalizePathBuffer(normalized, name, MAX_QPATH);

	// this should never happen
	if (namelen >= MAX_QPATH)
		Com_Error(ErrorType::Drop, "%s: oversize name", __func__);

	// normalized to empty name?
	if (namelen == 0) {
		Com_DPrintf("%s: empty name\n", __func__);
		return 0;
	}

	// see if it's already loaded
	char* extension = normalized + namelen - 4; // C++20: Moved up in case we need to.
	model = CL_Model_Find(normalized);
	if (model) {
		MOD_Reference(model);
		goto done;
	}

	if (namelen > 4 && (strcmp(extension, ".md2") == 0) && (vid_rtx && gl_use_hd_assets) && (vid_rtx->integer || gl_use_hd_assets->integer))
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
		load = MOD_LoadMD2;
		break;
#if USE_MD3
	case MD3_IDENT:
		load = MOD_LoadMD3;
		break;
#endif
	case SP2_IDENT:
		load = MOD_LoadSP2;
		break;
	case IQM_IDENT:
		load = MOD_LoadIQM;
		break;
	default:
		ret = Q_ERR_UNKNOWN_FORMAT;
		goto fail2;
	}

	model = CL_Model_Alloc();
	if (!model) {
		ret = Q_ERR_OUT_OF_SLOTS;
		goto fail2;
	}

	memcpy(model->name, normalized, namelen + 1);
	model->registration_sequence = registration_sequence;

	ret = load(model, R_MOD_Alloc, rawdata, filelen, name);

	// If we had no return values(no issues), identified model as IQM, and it has not
	// had any SKM data yet, generate it.
	if (!ret && ident == IQM_IDENT && !model->skeletalModelData) {
		model->skeletalModelData = &r_skeletalModels[(model - r_models) + 1];

		// Generate Skeletal Model Data.
		SKM_GenerateModelData(model);

		// This function needs rewriting but who am I... got 2 hands, so little time, right?
		memcpy(extension, ".skc", 4);

		// Now, load up our SKM config file.
		const bool result = SKM_LoadAndParseConfiguration( normalized );
		if (result) {
			Com_DPrintf("Loaded up SKM Config file: %s\n", normalized );

		} else {
			Com_DPrintf("Couldn't find/load SKM Config file: %s\n", normalized );
		}

		// Stuff back in iqm for sake.
		memcpy(extension, ".iqm", 4);
	}

	FS_FreeFile(rawdata);

	if (ret) {
		memset(model, 0, sizeof(*model));
		goto fail1;
	}

	model->model_class = get_model_class(model->name);

done:
	index = (model - r_models) + 1;
#if USE_REF == REF_VKPT
	int register_model_dirty;
	register_model_dirty = 1;
#endif



	return index;

fail2:
	FS_FreeFile(rawdata);
fail1:
	Com_EPrintf("Couldn't load %s: %s\n", normalized, Q_ErrorString(ret));
	return 0;
}

//model_t *CL_Model_GetModelByHandle(qhandle_t h)
//{
//	model_t *model;
//
//	if (!h) {
//		return NULL;
//	}
//
//	if (h < 0 || h > r_numModels) {
//		Com_Error(ErrorType::Drop, "%s: %d out of range", __func__, h);
//	}
//
//	model = &r_models[h - 1];
//	if (!model->type) {
//		return NULL;
//	}
//
//	return model;
//}

//void CL_Model_Init(void)
//{
//	if (r_numModels) {
//		Com_Error(ErrorType::Fatal, "%s: %d models not freed", __func__, r_numModels);
//	}
//
//	Cmd_AddCommand("modellist", CL_Model_List_f);
//}
//
//void CL_Model_Shutdown(void)
//{
//	CL_Model_FreeAll();
//	Cmd_RemoveCommand("modellist");
//}

