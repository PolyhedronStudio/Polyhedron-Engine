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

#ifndef MODELS_H
#define MODELS_H


//! Maximum allowed Alias Model Skins.
static constexpr int32_t MAX_ALIAS_SKINS = 32;
//! Maximum allowed Alias Model Verts.
static constexpr int32_t MAX_ALIAS_VERTS = 4096;

//
// models.h -- common models manager
//
#include "Shared/Refresh.h"
#include "System/Hunk.h"
#include "Common/Error.h"


qhandle_t R_RegisterModel(const char *name);

#endif // MODELS_H
