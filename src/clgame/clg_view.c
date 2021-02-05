// LICENSE HERE.

//
// clg_view.c
//
//
// View handling on a per frame basis.
//
#include "clg_local.h"

#define USE_DLIGHTS 0

// Development tools for weapons.
int         gun_frame;
qhandle_t   gun_model;

// CVars.
static cvar_t   *cl_add_particles;
#if USE_DLIGHTS
static cvar_t   *cl_add_lights;
static cvar_t   *cl_show_lights;
#endif
static cvar_t   *cl_add_entities;
static cvar_t   *cl_add_blend;

static cvar_t   *cl_testparticles;
static cvar_t   *cl_testentities;
#if USE_DLIGHTS
static cvar_t   *cl_testlights;
#endif
static cvar_t   *cl_testblend;

static cvar_t   *cl_stats;

static cvar_t   *cl_adjustfov;


void CL_UpdateBlendSetting(void)
{
    if (clgi.Com_GetClientState() < ca_connected) {
        return;
    }
    // TODO: IMPLEMENT PROTOCOL CHECK.
    // if (cls.serverProtocol < PROTOCOL_VERSION_R1Q2) {
    //     return;
    // }

    clgi.MSG_WriteByte(clc_setting);
    clgi.MSG_WriteShort(CLS_NOBLEND);
    clgi.MSG_WriteShort(!cl_add_blend->integer);
    // TODO: Implement.
    //clgi.MSG_FlushTo(&cls.netchan->message);
}


static void cl_add_blend_changed(cvar_t *self)
{
    //CL_UpdateBlendSetting();
}

//
//=============================================================================
//
// CLIENT MODULE VIEW COMMAND FUNCTIONS.
//
//=============================================================================
//
// gun frame debugging functions
static void V_Gun_Next_f(void)
{
    gun_frame++;
    Com_DPrint("frame %i\n", gun_frame);
}

static void V_Gun_Prev_f(void)
{
    gun_frame--;
    if (gun_frame < 0)
        gun_frame = 0;
    Com_DPrint("frame %i\n", gun_frame);
}

static void V_Gun_Model_f(void)
{
    char    name[MAX_QPATH];

    if (Cmd_Argc() != 2) {
        gun_model = 0;
        return;
    }
    Q_concat(name, sizeof(name), "models/", Cmd_Argv(1), "/tris.md2", NULL);
    gun_model = clgi.R_RegisterModel(name);
}

static void V_Viewpos_f(void)
{
    Com_Printf("(%i %i %i) : %i\n", (int)cl->refdef.vieworg[0],
               (int)cl->refdef.vieworg[1], (int)cl->refdef.vieworg[2],
               (int)cl->refdef.viewangles[YAW]);
}

// static const cmdreg_t v_cmds[] = {
//     { "gun_next", V_Gun_Next_f },
//     { "gun_prev", V_Gun_Prev_f },
//     { "gun_model", V_Gun_Model_f },
//     { "viewpos", V_Viewpos_f },
//     { NULL }
// };


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
// Add the entity to the current scene frame.
//===============
//
void V_AddEntity(entity_t *ent)
{
    // Ensure we aren't exceeding boundary limits.
    if (cl->view.num_entities >= MAX_ENTITIES)
        return;

    // Copy entity over into the current scene frame list.
    cl->view.entities[*cl->view.num_entities++] = *ent;
}


//
//===============
// V_AddParticle
// 
// Add the particle effect to the current scene frame.
//===============
//
void V_AddParticle(particle_t *p)
{
    // Ensure we aren't exceeding boundary limits.
    if (*cl->view.num_particles >= MAX_PARTICLES)
        return;

    // Copy particle over into the current scene frame list.
    cl->view.particles[*cl->view.num_particles++] = *p;
}

#if USE_DLIGHTS
//
//===============
// V_AddLight
// 
// Add the light of given properties to the current scene frame.
//===============
//
void V_AddLightEx(vec3_t org, float intensity, float r, float g, float b, float radius)
{
    dlight_t    *dl;

    if (*cl->view.num_dlights >= MAX_DLIGHTS)
        return;
    dl = &cl->view.dlights[*cl->view.num_dlights++];
    VectorCopy(org, dl->origin);
    dl->intensity = intensity;
    dl->color[0] = r;
    dl->color[1] = g;
    dl->color[2] = b;
	dl->radius = radius;

	if (cl_show_lights->integer && *cl->view.num_particles < MAX_PARTICLES)
	{
		particle_t* part = &cl->view.particles[*cl->view.num_particles++];

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

void V_AddLight(vec3_t org, float intensity, float r, float g, float b)
{
	V_AddLightEx(org, intensity, r, g, b, 10.f);
}
#endif

#if USE_LIGHTSTYLES
//
//===============
// V_AddLightStyle
// 
// Add the current lightstyle to the scene.
//===============
//
void V_AddLightStyle(int style, vec4_t value)
{
    lightstyle_t    *ls;

    if (style < 0 || style >= MAX_LIGHTSTYLES)
        Com_Error(ERR_DROP, "Bad light style %i", style);
    ls = &cl->view.lightstyles[style];

    //ls->white = r+g+b;
    ls->rgb[0] = value[0];
    ls->rgb[1] = value[1];
    ls->rgb[2] = value[2];
    ls->white = value[3];
}
#endif

//
//===============
// V_TestParticles
// 
// If cl_testparticles is set, create 4096 particles in the view
//===============
//
static void V_TestParticles(void)
{
    particle_t  *p;
    int         i, j;
    float       d, r, u;

    if (!cl->view.num_particles || !cl->view.particles) {
        Com_DPrint("%s - %s\n", __func__, "cl->view.particles | cl->view.num_particles is empty");
        return;
    }

    *cl->view.num_particles = MAX_PARTICLES;
    for (i = 0; i < *cl->view.num_particles; i++) {
        d = i * 0.25;
        r = 4 * ((i & 7) - 3.5);
        u = 4 * (((i >> 3) & 7) - 3.5);
        p = &cl->view.particles[i];

        for (j = 0; j < 3; j++)
            p->origin[j] = cl->refdef.vieworg[j] + cl->v_forward[j] * d +
                           cl->v_right[j] * r + cl->v_up[j] * u;

        p->color = 8;
        p->alpha = cl_testparticles->value;
    }
}

//
//===============
// V_TestEntities
// 
// If cl_testentities is set, create 32 player models
//===============
//
static void V_TestEntities(void)
{
    int         i, j;
    float       f, r;
    entity_t    *ent;

    if (!cl->view.num_entities || !cl->view.entities) {
        Com_DPrint("%s - %s\n", __func__, "cl->view.entities | cl->view.num_entities is empty");
        return;
    }

    *cl->view.num_entities = 32;
    memset(cl->view.entities, 0, sizeof(entity_t) * MAX_ENTITIES);

    for (i = 0; i < *cl->view.num_entities; i++) {
        ent = &cl->view.entities[i];

        r = 64 * ((i % 4) - 1.5);
        f = 64 * (i / 4) + 32;

        for (j = 0; j < 3; j++)
            ent->origin[j] = cl->refdef.vieworg[j] + cl->v_forward[j] * f +
                             cl->v_right[j] * r;

        ent->model = cl->baseclientinfo.model;
        ent->skin = cl->baseclientinfo.skin;
    }
}

#if USE_DLIGHTS
//
//===============
// V_TestLights
// 
// If cl_testlights is set, create 32 lights models
//===============
//
static void V_TestLights(void)
{
    int         i, j;
    float       f, r;
    dlight_t    *dl;

    if (cl_testlights->integer != 1) {
        dl = &cl->view.dlights[0];
        *cl->view.num_dlights = 1;

        VectorMA(cl->refdef.vieworg, 256, cl->v_forward, dl->origin);
        if (cl_testlights->integer == -1)
            VectorSet(dl->color, -1, -1, -1);
        else
            VectorSet(dl->color, 1, 1, 1);
        dl->intensity = 256;
        return;
    }

    *cl->view.num_dlights = 32;
    memset(cl->view.dlights, 0, sizeof(cl->view.dlights));

    for (i = 0; i < *cl->view.num_dlights; i++) {
        dl = &cl->view.dlights[i];

        r = 64 * ((i % 4) - 1.5);
        f = 64 * (i / 4) + 128;

        for (j = 0; j < 3; j++)
            dl->origin[j] = cl->refdef.vieworg[j] + cl->v_forward[j] * f +
                            cl->v_right[j] * r;
        dl->color[0] = ((i % 6) + 1) & 1;
        dl->color[1] = (((i % 6) + 1) & 2) >> 1;
        dl->color[2] = (((i % 6) + 1) & 4) >> 2;
        dl->intensity = 200;
    }
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
    //Cmd_Register(v_cmds);

    cl_testblend = clgi.Cvar_Get("cl_testblend", "0", 0);
    cl_testparticles = clgi.Cvar_Get("cl_testparticles", "0", 0);
    cl_testentities = clgi.Cvar_Get("cl_testentities", "0", 0);
#if USE_DLIGHTS
    cl_testlights = clgi.Cvar_Get("cl_testlights", "0", CVAR_CHEAT);
#endif

//     cl_stats = clgi.Cvar_Get("cl_stats", "0", 0);

// #if USE_DLIGHTS
//     cl_add_lights = clgi.Cvar_Get("cl_lights", "1", 0);
// 	cl_show_lights = clgi.Cvar_Get("cl_show_lights", "0", 0);
// #endif
//     cl_add_particles = clgi.Cvar_Get("cl_particles", "1", 0);
//     cl_add_entities = clgi.Cvar_Get("cl_entities", "1", 0);
//     cl_add_blend = clgi.Cvar_Get("cl_blend", "1", 0);
//     cl_add_blend->changed = cl_add_blend_changed;

//     cl_adjustfov = clgi.Cvar_Get("cl_adjustfov", "1", 0);
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
    //Cmd_Deregister(v_cmds);
}


//
//===============
// V_SetLightLevel
// 
// Set the lightlevel of the client.
//===============
//
static void V_SetLightLevel(void)
{
    vec3_t shadelight;

    // save off light value for server to look at (BIG HACK!)
    if (!clgi.R_LightPoint)
        Com_DPrint("WTF WTF WTF WTF WTF\n\n WTF WTF \n");
    clgi.R_LightPoint(cl->refdef.vieworg, shadelight);

    // pick the greatest component, which should be the same
    // as the mono value returned by software
    if (shadelight[0] > shadelight[1]) {
        if (shadelight[0] > shadelight[2]) {
            cl->lightlevel = 150.0f * shadelight[0];
        } else {
            cl->lightlevel = 150.0f * shadelight[2];
        }
    } else {
        if (shadelight[1] > shadelight[2]) {
            cl->lightlevel = 150.0f * shadelight[1];
        } else {
            cl->lightlevel = 150.0f * shadelight[2];
        }
    }
}

//
//===============
// V_AddEntities
// 
// Adds all the entity types to the scene.
//===============
//
static void V_AddEntities (void) {
        if (cl_testparticles->integer)
            V_TestParticles();
        if (cl_testentities->integer)
            V_TestEntities();
#if USE_DLIGHTS
        if (cl_testlights->integer)
            V_TestLights();
#endif
        // if (cl_testblend->integer) {
        //     cl.refdef.blend[0] = 1;
        //     cl.refdef.blend[1] = 0.5;
        //     cl.refdef.blend[2] = 0.25;
        //     cl.refdef.blend[3] = 0.5;
        // }
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
// CLG_PreRenderView
// 
// Called right after the engine clears the scene, and begins a new one.
//===============
//
void CLG_PreRenderView (void) {

}


//
//===============
// CLG_RenderView
// 
// Called whenever the engine wants to render a valid frame.
// Fill the scene with entities you want rendered to the client here.
//===============
//
void CLG_RenderView (void) {
    // Add our view entities.
    V_AddEntities();
// #ifdef _DEBUG
//         if (cl_testparticles->integer)
//             V_TestParticles();
//         if (cl_testentities->integer)
//             V_TestEntities();
// #if USE_DLIGHTS
//         if (cl_testlights->integer)
//             V_TestLights();
// #endif
//         if (cl_testblend->integer) {
//             cl.refdef.blend[0] = 1;
//             cl.refdef.blend[1] = 0.5;
//             cl.refdef.blend[2] = 0.25;
//             cl.refdef.blend[3] = 0.5;
//         }
// #endif
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