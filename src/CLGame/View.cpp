// LICENSE HERE.

//
// clg_view.c
//
//
// View handling on a per frame basis.
//
#include "ClientGameLocal.h"

#include "Effects.h"
#include "Entities.h"
#include "Main.h"
#include "TemporaryEntities.h"
#include "View.h"

clg_view_t  view;
//=============
//
// development tools for weapons
//
int         gun_frame;
qhandle_t   gun_model;

//=============

static cvar_t* cl_add_particles;

static cvar_t* cl_add_lights;
static cvar_t* cl_show_lights;

static cvar_t* cl_add_entities;
static cvar_t* cl_add_blend;

#ifdef _DEBUG
static cvar_t* cl_testparticles;
static cvar_t* cl_testentities;

static cvar_t* cl_testlights;

static cvar_t* cl_testblend;

static cvar_t* cl_stats;
#endif

static cvar_t* cl_adjustfov;


//
//=============================================================================
//
// CLIENT MODULE VIEW COMMAND FUNCTIONS.
//
//=============================================================================
// 

//
//===============
// V_AddEntity
// 
// [Opted for adding V_AddEntity to CG Module for customizability reasons.]
// Add the entity to the current scene frame.
//===============
//
void V_AddEntity(r_entity_t *ent)
{
    // Ensure we aren't exceeding boundary limits.
    if (view.num_entities >= MAX_ENTITIES)
        return;

    // Copy entity over into the current scene frame list.
    view.entities[view.num_entities++] = *ent;
}

//
//===============
// V_AddParticle
// 
// [Opted for adding V_AddParticle to CG Module for customizability reasons.]
// Add the particle effect to the current scene frame.
//===============
//
void V_AddParticle(rparticle_t *p)
{
    // Ensure we aren't exceeding boundary limits.
    if (view.num_particles >= MAX_PARTICLES)
        return;

    // Copy particle over into the current scene frame list.
    view.particles[view.num_particles++] = *p;
}


//
//===============
// V_AddLight
// 
// [Opted for adding V_AddLight to CG Module for customizability reasons.]
// Add the light of given properties to the current scene frame.
//===============
//
void V_AddLightEx(const vec3_t& org, float intensity, float r, float g, float b, float radius)
{
    rdlight_t    *dl;

    if (view.num_dlights >= MAX_DLIGHTS)
        return;
    dl = &view.dlights[view.num_dlights++];
    VectorCopy(org, dl->origin);
    dl->intensity = intensity;
    dl->color[0] = r;
    dl->color[1] = g;
    dl->color[2] = b;
	dl->radius = radius;

	if (cl_show_lights->integer && view.num_particles < MAX_PARTICLES)
	{
		rparticle_t* part = &view.particles[view.num_particles++];

		VectorCopy(dl->origin, part->origin);
		part->radius = radius;
		part->brightness = max(r, max(g, b));
		part->color = -1;
		part->rgba.u8[0] = (uint8_t)max(0.f, min(255.f, r / part->brightness * 255.f));
		part->rgba.u8[1] = (uint8_t)max(0.f, min(255.f, g / part->brightness * 255.f));
		part->rgba.u8[2] = (uint8_t)max(0.f, min(255.f, b / part->brightness * 255.f));
		part->rgba.u8[3] = 255;
		part->alpha = 1.f;
	}
}

void V_AddLight(const vec3_t& org, float intensity, float r, float g, float b)
{
	V_AddLightEx(org, intensity, r, g, b, 10.f);
}

#if USE_LIGHTSTYLES
//
//===============
// V_AddLightStyle
// 
// [Opted for adding V_AddLightStyle to CG Module for customizability reasons.]
// Add the current lightstyle to the scene.
//===============
//
void V_AddLightStyle(int style, const vec4_t &value)
{
    lightstyle_t    *ls;

    if (style < 0 || style >= MAX_LIGHTSTYLES)
        Com_Error(ERR_DROP, "Bad light style %i", style);
    ls = &view.lightstyles[style];

    //ls->white = r+g+b;
    ls->rgb[0] = value[0];
    ls->rgb[1] = value[1];
    ls->rgb[2] = value[2];
    ls->white = value[3];
}
#endif

//
//===============
// V_Init
// 
// Called by CLG_MediaInit, initializes the view related things.
//===============
//
void V_Init(void)
{

    cl_add_lights       = clgi.Cvar_Get("cl_lights", "1", 0);
    cl_show_lights      = clgi.Cvar_Get("cl_show_lights", "0", 0);

    cl_add_particles    = clgi.Cvar_Get("cl_particles", "1", 0);
    cl_add_entities     = clgi.Cvar_Get("cl_entities", "1", 0);
    cl_add_blend        = clgi.Cvar_Get("cl_blend", "1", 0);
    //cl_add_blend->changed = cl_add_blend_changed;

    cl_adjustfov        = clgi.Cvar_Get("cl_adjustfov", "1", 0);

    // Register possible view related commands and cvars here.
    // ...
}

//
//===============
// V_Shutdown
// 
// Called by CLG_MediaShutdown.
//===============
//
void V_Shutdown(void)
{

    // Unregister cmd's here.
    // ...
}

//
//=============================================================================
//
// CLIENT MODULE VIEW ENTRY FUNCTIONS.
//
//=============================================================================
//
