// LICENSE HERE.

//
// clg_view.c
//
//
// View handling on a per frame basis.
//
#include "clg_local.h"

#define USE_DLIGHTS 0

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
// [Opted for adding V_AddParticle to CG Module for customizability reasons.]
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
// [Opted for adding V_AddLight to CG Module for customizability reasons.]
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
// [Opted for adding V_AddLightStyle to CG Module for customizability reasons.]
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
// V_Init
// 
// Called by CLG_MediaInit, initializes the view related things.
//===============
//
void V_Init(void)
{
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
// Adds all the CG Module entities to tthe current frame scene.
//===============
//
static void V_AddEntities (void) {
    // Add entities here.
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