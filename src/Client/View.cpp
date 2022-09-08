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
// cl_view.c -- player rendering positioning

#include "Client.h"
#include "Client/GameModule.h"

//=============
//
// development tools for weapons
//
int         gun_frame;
qhandle_t   gun_model;

//=============

static cvar_t   *cl_add_particles;

static cvar_t   *cl_add_lights;
static cvar_t   *cl_show_lights;

static cvar_t   *cl_add_entities;
static cvar_t   *cl_add_blend;

static cvar_t   *cl_adjustfov;

//============================================================================

// gun frame debugging functions
static void V_Gun_Next_f(void)
{
    gun_frame++;
    Com_Printf("frame %i\n", gun_frame);
}

static void V_Gun_Prev_f(void)
{
    gun_frame--;
    if (gun_frame < 0)
        gun_frame = 0;
    Com_Printf("frame %i\n", gun_frame);
}

static void V_Gun_Model_f(void)
{
    char    name[MAX_QPATH];

    if (Cmd_Argc() != 2) {
        gun_model = 0;
        return;
    }
    Q_concat(name, sizeof(name), "models/", Cmd_Argv(1), "/tris.md2", NULL);
    gun_model = R_RegisterModel(name);
}

//============================================================================

static int entitycmpfnc(const void *_a, const void *_b)
{
    const r_entity_t *a = (const r_entity_t *)_a;
    const r_entity_t *b = (const r_entity_t *)_b;

    // all other models are sorted by model then skin
    if (a->model == b->model)
        return a->skin - b->skin;
    else
        return a->model - b->model;
}

/**
*	@brief	Calculates the client's local PVS, used for culling out local entities. 
**/
static void V_CalculateViewPVS( const vec3_t &viewOrigin ) {
    
	// Get leaf, area, cluster for our view origin.
	cl.clientPVS.leaf = BSP_PointLeaf( cl.cm.cache->nodes, viewOrigin );
    cl.clientPVS.leafArea	 = cl.clientPVS.leaf->area;
    cl.clientPVS.leafCluster = cl.clientPVS.leaf->cluster;

	// Copy area bytes from frame.  ////------------ Write our area bytes.
	//cl.clientPVS.areaBytes = cl.frame.areaBytes;//CM_WriteAreaBits( &cl.cm, cl.clientPVS.areaBits, cl.clientPVS.leafArea	);
	//memcpy( &cl.clientPVS.areaBytes, cl.frame.areaBits, sizeof(byte) * 32 );
	cl.clientPVS.areaBytes = CM_WriteAreaBits( &cl.cm, cl.clientPVS.areaBits, cl.clientPVS.leafArea );

    if ( cl.clientPVS.leafCluster >= 0 ) {
        CM_FatPVS( &cl.cm, cl.clientPVS.pvs, cl.refdef.vieworg, DVIS_PVS2 );
		// Store old valid cluster.
        cl.clientPVS.lastValidCluster = cl.clientPVS.leafCluster;
    } else {
        BSP_ClusterVis( cl.cm.cache, cl.clientPVS.pvs, cl.clientPVS.leafCluster, DVIS_PVS2 );
    }

	//Com_LPrintf(PrintType::Developer, fmt::format(
	//	"V_CalculateViewPVS(leafArea({}), leafCluster({})): {}, {}, {}\n",
	// cl.clientPVS.leafArea, cl.clientPVS.leafCluster, viewOrigin.x, viewOrigin.y, viewOrigin.z ).c_str());
}
//static void V_CalculateViewPVS( const vec3_t &viewOrigin ) {
//    
//	// Get leaf, area, cluster for our view origin.
//	cl.clientPVS.leaf = BSP_PointLeaf( cl.bsp->nodes, viewOrigin );
//    cl.clientPVS.leafArea	 = cl.clientPVS.leaf->area;
//    cl.clientPVS.leafCluster = cl.clientPVS.leaf->cluster;
//
//	// Copy area bytes from frame.  ////------------ Write our area bytes.
//	cl.clientPVS.areaBytes = cl.frame.areaBytes;//CM_WriteAreaBits( &cl.cm, cl.clientPVS.areaBits, cl.clientPVS.leafArea	);
//	memcpy( &cl.clientPVS.areaBytes, cl.frame.areaBits, sizeof(byte) * 32 );
//
//	cm_t bspCm = { .cache = cl.bsp };
//
//    if ( cl.clientPVS.leafCluster >= 0 ) {
//        CM_FatPVS( &bspCm, cl.clientPVS.pvs, cl.refdef.vieworg, DVIS_PVS2 );
//		// Store old valid cluster.
//        cl.clientPVS.lastValidCluster = cl.clientPVS.leafCluster;
//    } else {
//        BSP_ClusterVis( cl.bsp, cl.clientPVS.pvs, cl.clientPVS.leafCluster, DVIS_PVS2 );
//    }
//
//	Com_LPrintf(PrintType::Developer, fmt::format(
//		"V_CalculateViewPVS(leafArea({}), leafCluster({})): {}, {}, {}\n",
//	 cl.clientPVS.leafArea, cl.clientPVS.leafCluster, viewOrigin.x, viewOrigin.y, viewOrigin.z ).c_str());
//}

/**
*	@brief	This is where all entities that are rendered per frame are added to the
*			scene.
*
*			Calls into the CG Module so it can add it can run its own scene.
**/
void V_RenderView(void) {

    // an invalid frame will just use the exact previous refdef
    // we can't use the old frame if the video mode has changed, though...
    if (cl.frame.valid) {
        // Clear the scene before calling into the CG Module.
        CL_GM_ClearScene();

        // PreRender CG Module View.
        CL_GM_PreRenderView();

		// Calculate the new client's PVS.
		V_CalculateViewPVS( cl.refdef.vieworg );

        // Add CG Module entities.
        CL_GM_RenderView();

		// Old Original Comment:
        // never let it sit exactly on a node line, because a water plane can
        // dissapear when viewed with the eye exactly on it.
        // the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
		// New:
		// Our offsets are 0.015625, so 1/64 pixel at max in case of 4096x4096
        cl.refdef.vieworg[0] += 1.0 / 16;
        cl.refdef.vieworg[1] += 1.0 / 16;
        cl.refdef.vieworg[2] += 1.0 / 16;

        // Setup refresh X, Y, Width and Height.
        cl.refdef.x = scr_vrect.x;
        cl.refdef.y = scr_vrect.y;
        cl.refdef.width = scr_vrect.width;
        cl.refdef.height = scr_vrect.height;

        // Adjust for non-4/3 screens
        if (cl_adjustfov->integer) {
            cl.refdef.fov_y = cl.fov_y;
            cl.refdef.fov_x = CL_GM_CalcFOV(cl.refdef.fov_y, cl.refdef.height, cl.refdef.width);
        } else {
            cl.refdef.fov_x = cl.fov_x;
            cl.refdef.fov_y = CL_GM_CalcFOV(cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);
        }

        // Increase refresh time.
        cl.refdef.time = cl.time * 0.001f;

        // Set areabytes if we got any.
        if (cl.frame.areaBytes) {
            cl.refdef.areaBits = cl.frame.areaBits;
        } else {
            cl.refdef.areaBits = NULL;
        }

        // In case we don't want blends, zero out the vec4.
        if (!cl_add_blend->integer) {
            cl.refdef.blend = vec4_zero();
        }

        cl.refdef.rdflags = cl.frame.playerState.rdflags;

        // sort entities for better cache locality
        qsort(cl.refdef.entities, cl.refdef.num_entities, sizeof(cl.refdef.entities[0]), entitycmpfnc);
	} else {

		// Calculate the new client's PVS.
		V_CalculateViewPVS( cl.refdef.vieworg );
	}

    // Pass the actual scene render definiiton over to the Renderer for rendering.  
    R_RenderFrame(&cl.refdef);

    // Let the ClientGame module handle post renderview logic.
    CL_GM_PostRenderView();
}

static const cmdreg_t v_cmds[] = {
    { "gun_next", V_Gun_Next_f },
    { "gun_prev", V_Gun_Prev_f },
    { "gun_model", V_Gun_Model_f },
    { NULL }
};

/*
=============
V_Init
=============
*/
void V_Init(void)
{
    Cmd_Register(v_cmds);

    cl_add_lights = Cvar_Get("cl_lights", "1", 0);
    cl_show_lights = Cvar_Get("cl_show_lights", "0", 0);

    cl_add_particles = Cvar_Get("cl_particles", "1", 0);
    cl_add_entities = Cvar_Get("cl_entities", "1", 0);
    cl_add_blend = Cvar_Get("cl_blend", "1", 0);

    cl_adjustfov = Cvar_Get("cl_adjustfov", "1", 0);
}

void V_Shutdown(void)
{
    Cmd_Unregister(v_cmds);
}



