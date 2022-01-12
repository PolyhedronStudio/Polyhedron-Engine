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
#if USE_DLIGHTS
static cvar_t* cl_add_lights;
static cvar_t* cl_show_lights;
#endif
static cvar_t* cl_add_entities;
static cvar_t* cl_add_blend;

#ifdef _DEBUG
static cvar_t* cl_testparticles;
static cvar_t* cl_testentities;
#if USE_DLIGHTS
static cvar_t* cl_testlights;
#endif
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

#if USE_DLIGHTS
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
#endif

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
#if USE_DLIGHTS
    cl_add_lights       = clgi.Cvar_Get("cl_lights", "1", 0);
    cl_show_lights      = clgi.Cvar_Get("cl_show_lights", "0", 0);
#endif
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
//===============
// V_SetLightLevel
// 
// Set the lightLevel of the client.
//===============
//
void V_SetLightLevel(void)
{
    vec3_t shadelight;

    // save off light value for server to look at (BIG HACK!)
    clgi.R_LightPoint(cl->refdef.vieworg, shadelight);

    // pick the greatest component, which should be the same
    // as the mono value returned by software
    if (shadelight[0] > shadelight[1]) {
        if (shadelight[0] > shadelight[2]) {
            cl->lightLevel = 150.0f * shadelight[0];
        } else {
            cl->lightLevel = 150.0f * shadelight[2];
        }
    } else {
        if (shadelight[1] > shadelight[2]) {
            cl->lightLevel = 150.0f * shadelight[1];
        } else {
            cl->lightLevel = 150.0f * shadelight[2];
        }
    }
}

//
//===============
// CLG_SetupFirstPersonView
// 
// First Person Camera implementation.
//===============
//
static void CLG_SetupFirstPersonView(void)
{
    vec3_t kickangles;

    // add kick angles
    if (cl_kickangles->integer) {
        PlayerState *playerState = &cl->frame.playerState;
        PlayerState *oldPlayerState = &cl->oldframe.playerState;

        double lerp = cl->lerpFraction;

        vec3_t kickAngles = vec3_mix_euler(oldPlayerState->kickAngles, playerState->kickAngles, cl->lerpFraction);
        cl->refdef.viewAngles += kickAngles;
        //VectorAdd(cl->refdef.viewAngles, kickangles, cl->refdef.viewAngles);
    }

    // add the weapon
    CLG_AddViewWeapon();

    cl->thirdPersonView = false;
}

//
//===============
// CLG_SetupThirdPersionView
// 
// Third Person Camera implementation.
//===============
//
static void CLG_SetupThirdPersionView(void)
{
    vec3_t focus;
    float fscale, rscale;
    float dist, angle, range;
    trace_t trace;
    static vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

    // if dead, set a nice view angle
    if (cl->frame.playerState.stats[STAT_HEALTH] <= 0) {
        cl->refdef.viewAngles[vec3_t::Roll] = 0;
        cl->refdef.viewAngles[vec3_t::Pitch] = 10;
    }

    VectorMA(cl->refdef.vieworg, 512, cl->v_forward, focus);

    cl->refdef.vieworg[2] += 8;

    cl->refdef.viewAngles[vec3_t::Pitch] *= 0.5f;
    AngleVectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    angle = Radians(cl_thirdperson_angle->value);
    range = cl_thirdperson_range->value;
    fscale = std::cosf(angle);
    rscale = std::sinf(angle);
    VectorMA(cl->refdef.vieworg, -range * fscale, cl->v_forward, cl->refdef.vieworg);
    VectorMA(cl->refdef.vieworg, -range * rscale, cl->v_right, cl->refdef.vieworg);

    clgi.CM_BoxTrace(&trace, cl->playerEntityOrigin, cl->refdef.vieworg,
        mins, maxs, cl->bsp->nodes, CONTENTS_MASK_SOLID);
    if (trace.fraction != 1.0f) {
        VectorCopy(trace.endPosition, cl->refdef.vieworg);
    }

    VectorSubtract(focus, cl->refdef.vieworg, focus);
    dist = std::sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    cl->refdef.viewAngles[vec3_t::Pitch] = -180.f / M_PI * std::atan2f(focus[2], dist);
    cl->refdef.viewAngles[vec3_t::Yaw] -= cl_thirdperson_angle->value;

    cl->thirdPersonView = true;
}

//
//===============
// CLG_FinishViewValues
// 
// Finish the view values, calculate first/third -person view.
//===============
//
void CLG_FinishViewValues(void)
{
    cl_entity_t* ent;

    if (cl_player_model->integer != CL_PLAYER_MODEL_THIRD_PERSON)
        goto first;

    if (cl->frame.clientNumber == CLIENTNUM_NONE)
        goto first;
    
    ent = &cs->entities[cl->frame.clientNumber + 1];
    if (ent->serverFrame != cl->frame.number)
        goto first;

    if (!ent->current.modelIndex)
        goto first;

    CLG_SetupThirdPersionView();
    return;

first:
    CLG_SetupFirstPersonView();
}

//
//===============
// CLG_AddEntities
// 
// Adds all the CG Module entities to tthe current frame scene.
//===============
//
static void CLG_AddEntities (void) {
    // Calculate client view values.
    CLG_UpdateOrigin();

    // Finish calculating view values.
    CLG_FinishViewValues();

    // Add entities here.
    CLG_AddPacketEntities();
    CLG_AddTempEntities();
    CLG_AddParticles();

#if USE_DLIGHTS
    CLG_AddDLights();
#endif
#if USE_LIGHTSTYLES
    CLG_AddLightStyles();
#endif
    //LOC_AddLocationsToScene();
}

//
//=============================================================================
//
// CLIENT MODULE VIEW ENTRY FUNCTIONS.
//
//=============================================================================
//

//
//===============
// CLG_CalculateFOV
// 
// Calculates the Field Of View.
//===============
//
float CLG_CalculateFOV(float fov_x, float width, float height)
{
    float    a;
    float    x;

    if (fov_x < 1.f || fov_x > 179.f)
        Com_Error(ERR_DROP, "%s: bad fov: %f", __func__, fov_x);

    x = width / tan(fov_x / 360.f * M_PI);

    a = atan(height / x);
    a = a * 360.f / M_PI;

    return a;
}

//
//===============
// CLG_PreRenderView
// 
// Called right after the engine clears the scene, and begins a new one.
//===============
//
void CLG_PreRenderView (void) {

}

//
//===============
// CLG_ClearScene
// 
// Called when the engine wants to clear the frame.
// It also specifies the model that will be used as the world.
//===============
//
void CLG_ClearScene(void)
{
#if USE_DLIGHTS
    view.num_dlights = 0;
#endif
    view.num_entities = 0;
    view.num_particles = 0;
}

//
//===============
// CLG_RenderView
// 
// Called whenever the engine wants to render a newly parsed valid frame.
// Fill the scene with entities you want rendered to the client here.
//===============
//
void CLG_RenderView (void) {
    // Add our view entities.
    CLG_AddEntities();

    // Last but not least, pass our array over to the client.
    cl->refdef.num_entities     = view.num_entities;
    cl->refdef.entities         = view.entities;
    cl->refdef.num_particles    = view.num_particles;
    cl->refdef.particles        = view.particles;
#if USE_DLIGHTS
    cl->refdef.num_dlights      = view.num_dlights;
    cl->refdef.dlights          = view.dlights;
#endif
#if USE_LIGHTSTYLES
    cl->refdef.lightstyles      = view.lightstyles;
#endif
}


//
//===============
// CLG_PostRenderView
// 
// Called right after the engine renders the scene, and prepares to
// finish up its current frame loop iteration.
//===============
//
void CLG_PostRenderView (void) {
    V_SetLightLevel();
}