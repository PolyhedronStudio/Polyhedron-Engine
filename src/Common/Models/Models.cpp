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

#include "Shared/Shared.h"
#include "Shared/List.h"
#include "Common/Common.h"
#include "Common/Files.h"
#include "System/Hunk.h"
#include "Shared/Formats/Md2.h"
#if USE_MD3
#include "Shared/Formats/Md3.h"
#endif
#include "Shared/Formats/Sp2.h"
#include "Shared/Formats/Iqm.h"
#include "Refresh/Images.h"
#include "Refresh/Models.h"

// during registration it is possible to have more models than could actually
// be referenced during gameplay, because we don't want to free anything until
// we are sure we won't need it.
#define MAX_RMODELS     (MAX_MODELS * 2)

model_t      r_models[MAX_RMODELS];
int          r_numModels;

#if USE_CLIENT
extern cvar_t *vid_rtx;
extern cvar_t *gl_use_hd_assets;
#endif

model_t *MOD_Alloc(void)
{
	model_t *model;
	int i;

	for (i = 0, model = r_models; i < r_numModels; i++, model++) {
		if (!model->type) {
			break;
		}
	}

	if (i == r_numModels) {
		if (r_numModels == MAX_RMODELS) {
			return NULL;
		}
		r_numModels++;
	}

	return model;
}

model_t *MOD_Find(const char *name)
{
	model_t *model;
	int i;

	for (i = 0, model = r_models; i < r_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		if (!FS_pathcmp(model->name, name)) {
			return model;
		}
	}

	return NULL;
}

void MOD_List_f(void)
{
	static const char types[] = "FASE"; // CPP: Cast - was types[4] = "FASE";
	int     i, count;
	model_t *model;
	size_t  bytes;

	Com_Printf("------------------\n");
	bytes = count = 0;

	for (i = 0, model = r_models; i < r_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		Com_Printf("%c %8" PRIz " : %s\n", types[model->type], // CPP: String fix.
			model->hunk.mapped, model->name);
		bytes += model->hunk.mapped;
		count++;
	}
	Com_Printf("Total models: %d (out of %d slots)\n", count, r_numModels);
	Com_Printf("Total resident: %" PRIz "\n", bytes); // CPP: String fix.
}

void MOD_FreeUnused(void)
{
	model_t *model;
	int i;

	for (i = 0, model = r_models; i < r_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}
		if (model->registration_sequence == registration_sequence) {
			// make sure it is paged in
			Com_PageInMemory(model->hunk.base, model->hunk.currentSize);
		}
		else {
			// don't need this model
			Hunk_Free(&model->hunk);
			memset(model, 0, sizeof(*model));
		}
	}
}

void MOD_FreeAll(void)
{
	model_t *model;
	int i;

	for (i = 0, model = r_models; i < r_numModels; i++, model++) {
		if (!model->type) {
			continue;
		}

		Hunk_Free(&model->hunk);
		memset(model, 0, sizeof(*model));
	}

	r_numModels = 0;
}

qerror_t MOD_ValidateMD2(dmd2header_t *header, size_t length)
{
	size_t end;

	// check ident and version
	if (header->ident != MD2_IDENT)
		return Q_ERR_UNKNOWN_FORMAT;
	if (header->version != MD2_VERSION)
		return Q_ERR_UNKNOWN_FORMAT;

	// check triangles
	if (header->num_tris < 1)
		return Q_ERR_TOO_FEW;
	if (header->num_tris > MD2_MAX_TRIANGLES)
		return Q_ERR_TOO_MANY;

	end = header->ofs_tris + sizeof(dmd2triangle_t) * header->num_tris;
	if (header->ofs_tris < sizeof(*header) || end < header->ofs_tris || end > length)
		return Q_ERR_BAD_EXTENT;

	// check st
	if (header->num_st < 3)
		return Q_ERR_TOO_FEW;
	if (header->num_st > MAX_ALIAS_VERTS)
		return Q_ERR_TOO_MANY;

	end = header->ofs_st + sizeof(dmd2stvert_t) * header->num_st;
	if (header->ofs_st < sizeof(*header) || end < header->ofs_st || end > length)
		return Q_ERR_BAD_EXTENT;

	// check xyz and frames
	if (header->num_xyz < 3)
		return Q_ERR_TOO_FEW;
	if (header->num_xyz > MAX_ALIAS_VERTS)
		return Q_ERR_TOO_MANY;
	if (header->num_frames < 1)
		return Q_ERR_TOO_FEW;
	if (header->num_frames > MD2_MAX_FRAMES)
		return Q_ERR_TOO_MANY;

	end = sizeof(dmd2frame_t) + (header->num_xyz - 1) * sizeof(dmd2trivertx_t);
	if (header->framesize < end || header->framesize > MD2_MAX_FRAMESIZE)
		return Q_ERR_BAD_EXTENT;

	end = header->ofs_frames + (size_t)header->framesize * header->num_frames;
	if (header->ofs_frames < sizeof(*header) || end < header->ofs_frames || end > length)
		return Q_ERR_BAD_EXTENT;

	// check skins
	if (header->num_skins) {
		if (header->num_skins > MAX_ALIAS_SKINS)
			return Q_ERR_TOO_MANY;

		end = header->ofs_skins + (size_t)MD2_MAX_SKINNAME * header->num_skins;
		if (header->ofs_skins < sizeof(*header) || end < header->ofs_skins || end > length)
			return Q_ERR_BAD_EXTENT;
	}

	if (header->skinwidth < 1 || header->skinwidth > MD2_MAX_SKINWIDTH)
		return Q_ERR_INVALID_FORMAT;
	if (header->skinheight < 1 || header->skinheight > MD2_MAX_SKINHEIGHT)
		return Q_ERR_INVALID_FORMAT;

	return Q_ERR_SUCCESS;
}

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
#if USE_CLIENT
qerror_t MOD_LoadSP2(model_t* model, ModelMemoryAllocateCallback modelAllocate, const void* rawdata, size_t length, const char* mod_name)
{
	dsp2header_t header;
	dsp2frame_t *src_frame;
	mspriteframe_t *dst_frame;
	unsigned w, h, x, y;
	char buffer[SP2_MAX_FRAMENAME];
	int i;

	if (length < sizeof(header))
		return Q_ERR_FILE_TOO_SMALL;

	// byte swap the header
	header = *(dsp2header_t *)rawdata;
	for (i = 0; i < sizeof(header) / 4; i++) {
		((uint32_t *)&header)[i] = LittleLong(((uint32_t *)&header)[i]);
	}

	if (header.ident != SP2_IDENT)
		return Q_ERR_UNKNOWN_FORMAT;
	if (header.version != SP2_VERSION)
		return Q_ERR_UNKNOWN_FORMAT;
	if (header.numframes < 1) {
		// empty models draw nothing
		model->type = model_s::MOD_EMPTY; // CPP: Enum
		return Q_ERR_SUCCESS;
	}
	if (header.numframes > SP2_MAX_FRAMES)
		return Q_ERR_TOO_MANY;
	if (sizeof(dsp2header_t) + sizeof(dsp2frame_t) * header.numframes > length)
		return Q_ERR_BAD_EXTENT;

	Hunk_Begin(&model->hunk, 0x10000);
	model->type = model_s::MOD_SPRITE; // CPP: Enum

	model->spriteframes = (mspriteframe_s*)modelAllocate(&model->hunk, sizeof(mspriteframe_t) * header.numframes); // CPP: Cast
	model->numframes = header.numframes;

	src_frame = (dsp2frame_t *)((byte *)rawdata + sizeof(dsp2header_t));
	dst_frame = model->spriteframes;
	for (i = 0; i < header.numframes; i++) {
		w = LittleLong(src_frame->width);
		h = LittleLong(src_frame->height);
		if (w < 1 || h < 1 || w > MAX_TEXTURE_SIZE || h > MAX_TEXTURE_SIZE) {
			Com_WPrintf("%s has bad frame dimensions\n", model->name);
			w = 1;
			h = 1;
		}
		dst_frame->width = w;
		dst_frame->height = h;

		// FIXME: are these signed?
		x = LittleLong(src_frame->origin_x);
		y = LittleLong(src_frame->origin_y);
		if (x > 8192 || y > 8192) {
			Com_WPrintf("%s has bad frame origin\n", model->name);
			x = y = 0;
		}
		dst_frame->origin_x = x;
		dst_frame->origin_y = y;

		if (!Q_memccpy(buffer, src_frame->name, 0, sizeof(buffer))) {
			Com_WPrintf("%s has bad frame name\n", model->name);
			dst_frame->image = R_NOTEXTURE;
		}
		else {
			FS_NormalizePath(buffer, buffer);
			dst_frame->image = IMG_Find(buffer, IT_SPRITE, IF_SRGB);
		}

		src_frame++;
		dst_frame++;
	}

	// WID: TODO: This might need some improvements since we first assign a char* to a std::string
	std::string modelName = model->name;
	std::string subModelPart = modelName.substr(0, 4);

	if (subModelPart == "vrty_") {
		model->sprite_vertical = true;
	} else if (subModelPart == "fxup_") {
		model->sprite_fxup = true;
	} else if (subModelPart == "fxft_") {
		model->sprite_fxft = true;
	} else if (subModelPart == "fxlt_") {
		model->sprite_fxlt = true;
	}

	// WID: This is the old Omega way which will really be relentless if for whichever reason a file contains these same
	// parts.
	//if (strstr(model->name, "vrty"))
	//	model->sprite_vertical = true;
	//else if (strstr(model->name, "fxup"))
	//	model->sprite_fxup = true;
	//else if (strstr(model->name, "fxft"))
	//	model->sprite_fxft = true;
	//else if (strstr(model->name, "fxlt")) 
	//	model->sprite_fxlt = true;

	Hunk_End(&model->hunk);

	return Q_ERR_SUCCESS;
}
#endif
model_t *MOD_ForHandle(qhandle_t h)
{
	model_t *model;

	if (!h) {
		return NULL;
	}

	if (h < 0 || h > r_numModels) {
		Com_Error(ErrorType::Drop, "%s: %d out of range", __func__, h);
	}

	model = &r_models[h - 1];
	if (!model->type) {
		return NULL;
	}

	return model;
}

void MOD_Init(void)
{
	if (r_numModels) {
		Com_Error(ErrorType::Fatal, "%s: %d models not freed", __func__, r_numModels);
	}

	Cmd_AddCommand("modellist", MOD_List_f);
}

void MOD_Shutdown(void)
{
	MOD_FreeAll();
	Cmd_RemoveCommand("modellist");
}

