// LICENSE HERE.

//
// clg_view.c
//
//
// View handling on a per frame basis.
//
#include "clg_local.h"

//
//=============================================================================
//
// CLIENT MODULE VIEW FUNCTIONS.
//
//=============================================================================

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