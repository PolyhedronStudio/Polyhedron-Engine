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

#include "client.h"
#include "client/gamemodule.h"

//=============
//
// development tools for weapons
//
int         gun_frame;
qhandle_t   gun_model;

//=============

static cvar_t   *cl_add_particles;
#if USE_DLIGHTS
static cvar_t   *cl_add_lights;
static cvar_t   *cl_show_lights;
#endif
static cvar_t   *cl_add_entities;
static cvar_t   *cl_add_blend;

#ifdef _DEBUG
static cvar_t   *cl_testparticles;
static cvar_t   *cl_testentities;
#if USE_DLIGHTS
static cvar_t   *cl_testlights;
#endif
static cvar_t   *cl_testblend;

static cvar_t   *cl_stats;
#endif

static cvar_t   *cl_adjustfov;

#if USE_DLIGHTS
int         r_numdlights;
rdlight_t    r_dlights[MAX_DLIGHTS];
#endif

int         r_numentities;
r_entity_t    r_entities[MAX_ENTITIES];

int         r_numparticles;
rparticle_t  r_particles[MAX_PARTICLES];

#if USE_LIGHTSTYLES
lightstyle_t    r_lightstyles[MAX_LIGHTSTYLES];
#endif


/*
====================
V_ClearScene

Specifies the model that will be used as the world
====================
*/
static void V_ClearScene(void)
{
#if USE_DLIGHTS
    r_numdlights = 0;
#endif
    r_numentities = 0;
    r_numparticles = 0;
}

#ifdef _DEBUG
/*
================
V_TestParticles

If cl_testparticles is set, create 4096 particles in the view
================
*/
static void V_TestParticles(void)
{
    rparticle_t  *p;
    int         i, j;
    float       d, r, u;

    r_numparticles = MAX_PARTICLES;
    for (i = 0; i < r_numparticles; i++) {
        d = i * 0.25;
        r = 4 * ((i & 7) - 3.5);
        u = 4 * (((i >> 3) & 7) - 3.5);
        p = &r_particles[i];

        for (j = 0; j < 3; j++)
            p->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * d +
                           cl.v_right[j] * r + cl.v_up[j] * u;

        p->color = 8;
        p->alpha = cl_testparticles->value;
    }
}

/*
================
V_TestEntities

If cl_testentities is set, create 32 player models
================
*/
static void V_TestEntities(void)
{
    int         i, j;
    float       f, r;
    r_entity_t    *ent;

    r_numentities = 32;
    memset(r_entities, 0, sizeof(r_entities));

    for (i = 0; i < r_numentities; i++) {
        ent = &r_entities[i];

        r = 64 * ((i % 4) - 1.5);
        f = 64 * (i / 4) + 128;

        for (j = 0; j < 3; j++)
            ent->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * f +
                             cl.v_right[j] * r;

        ent->model = cl.baseClientInfo.model;
        ent->skin = cl.baseClientInfo.skin;
    }
}

#if USE_DLIGHTS
/*
================
V_TestLights

If cl_testlights is set, create 32 lights models
================
*/
static void V_TestLights(void)
{
    int         i, j;
    float       f, r;
    rdlight_t    *dl;

    if (cl_testlights->integer != 1) {
        dl = &r_dlights[0];
        r_numdlights = 1;

        VectorMA(cl.refdef.vieworg, 256, cl.v_forward, dl->origin);
        if (cl_testlights->integer == -1)
            VectorSet(dl->color, -1, -1, -1);
        else
            VectorSet(dl->color, 1, 1, 1);
        dl->intensity = 256;
        return;
    }

    r_numdlights = 32;
    memset(r_dlights, 0, sizeof(r_dlights));

    for (i = 0; i < r_numdlights; i++) {
        dl = &r_dlights[i];

        r = 64 * ((i % 4) - 1.5);
        f = 64 * (i / 4) + 128;

        for (j = 0; j < 3; j++)
            dl->origin[j] = cl.refdef.vieworg[j] + cl.v_forward[j] * f +
                            cl.v_right[j] * r;
        dl->color[0] = ((i % 6) + 1) & 1;
        dl->color[1] = (((i % 6) + 1) & 2) >> 1;
        dl->color[2] = (((i % 6) + 1) & 4) >> 2;
        dl->intensity = 200;
    }
}
#endif

#endif

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


//
//===============
// V_RenderView
// 
// This is where all entities that are rendered per frame are added to the
// scene.
//
// Calls into the CG Module so it can add it can run its own scene.
//===============
//
void V_RenderView(void)
{

    // an invalid frame will just use the exact previous refdef
    // we can't use the old frame if the video mode has changed, though...
    if (cl.frame.valid) {
        // Clear the scene before calling into the CG Module.
        CL_GM_ClearScene();

        // PreRender CG Module View.
        CL_GM_PreRenderView();

        // Add CG Module entities.
        CL_GM_RenderView();

        // never let it sit exactly on a node line, because a water plane can
        // dissapear when viewed with the eye exactly on it.
        // the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
        cl.refdef.vieworg[0] += 1.0 / 32;
        cl.refdef.vieworg[1] += 1.0 / 32;
        cl.refdef.vieworg[2] += 1.0 / 32;

        cl.refdef.x = scr_vrect.x;
        cl.refdef.y = scr_vrect.y;
        cl.refdef.width = scr_vrect.width;
        cl.refdef.height = scr_vrect.height;

        // adjust for non-4/3 screens
        if (cl_adjustfov->integer) {
            cl.refdef.fov_y = cl.fov_y;
            cl.refdef.fov_x = CL_GM_CalcFOV(cl.refdef.fov_y, cl.refdef.height, cl.refdef.width);
        } else {
            cl.refdef.fov_x = cl.fov_x;
            cl.refdef.fov_y = CL_GM_CalcFOV(cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);
        }

        cl.refdef.time = cl.time * 0.001f;

        if (cl.frame.areaBytes) {
            cl.refdef.areaBits = cl.frame.areaBits;
        } else {
            cl.refdef.areaBits = NULL;
        }

        if (!cl_add_entities->integer)
            r_numentities = 0;
        if (!cl_add_particles->integer)
            r_numparticles = 0;
#if USE_DLIGHTS
        if (!cl_add_lights->integer)
            r_numdlights = 0;
#endif
        if (!cl_add_blend->integer)
            Vector4Clear(cl.refdef.blend);

        cl.refdef.rdflags = cl.frame.playerState.rdflags;

        // sort entities for better cache locality
        qsort(cl.refdef.entities, cl.refdef.num_entities, sizeof(cl.refdef.entities[0]), entitycmpfnc);
    }

    // Pass the actual scene render definiiton over to the Renderer for rendering.  
    R_RenderFrame(&cl.refdef);
#ifdef _DEBUG
    if (cl_stats->integer)
#if USE_DLIGHTS
        Com_Printf("ent:%i  lt:%i  part:%i\n", r_numentities, r_numdlights, r_numparticles);
#else
        Com_Printf("ent:%i  part:%i\n", r_numentities, r_numparticles);
#endif
#endif

    // 
    CL_GM_PostRenderView();
}

/*
=============
V_Viewpos_f
=============
*/
static void V_Viewpos_f(void)
{
    Com_Printf("(%i %i %i) : %i\n", (int)cl.refdef.vieworg[0],
               (int)cl.refdef.vieworg[1], (int)cl.refdef.vieworg[2],
               (int)cl.refdef.viewAngles[vec3_t::Yaw]);
}

static const cmdreg_t v_cmds[] = {
    { "gun_next", V_Gun_Next_f },
    { "gun_prev", V_Gun_Prev_f },
    { "gun_model", V_Gun_Model_f },
    { "viewpos", V_Viewpos_f },
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

#ifdef _DEBUG
    cl_testblend = Cvar_Get("cl_testblend", "0", 0);
    cl_testparticles = Cvar_Get("cl_testparticles", "0", 0);
    cl_testentities = Cvar_Get("cl_testentities", "0", 0);
#if USE_DLIGHTS
    cl_testlights = Cvar_Get("cl_testlights", "0", CVAR_CHEAT);
#endif

    cl_stats = Cvar_Get("cl_stats", "0", 0);
#endif

#if USE_DLIGHTS
    cl_add_lights = Cvar_Get("cl_lights", "1", 0);
	cl_show_lights = Cvar_Get("cl_show_lights", "0", 0);
#endif
    cl_add_particles = Cvar_Get("cl_particles", "1", 0);
    cl_add_entities = Cvar_Get("cl_entities", "1", 0);
    cl_add_blend = Cvar_Get("cl_blend", "1", 0);

    cl_adjustfov = Cvar_Get("cl_adjustfov", "1", 0);
}

void V_Shutdown(void)
{
    Cmd_Unregister(v_cmds);
}



