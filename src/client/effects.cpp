/*
Copyright (C) 1997-2001 Id Software, Inc.
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
// effects.c -- leftover code from q2rtx before we did the CLGame move over.

#include "client.h"

// Leftover from the q2rtx code, this variable maintains the sound effect parsing parameters...
snd_params_t    snd;

// These speak for theirselves.
cvar_t* cvar_pt_particle_emissive = NULL;
cvar_t* cl_particle_num_factor = NULL;

void FX_Init(void)
{
    cvar_pt_particle_emissive = Cvar_Get("pt_particle_emissive", "10.0", 0);
	cl_particle_num_factor = Cvar_Get("cl_particle_num_factor", "1", 0);
}
