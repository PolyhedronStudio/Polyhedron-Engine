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
// VIEW.
//
//=============================================================================
//

//
//===============
// CLG_RenderView
// 
// Called when the engine wants to render a view.
//===============
//
void CLG_RenderView (void) {
    // an invalid frame will just use the exact previous refdef
    // we can't use the old frame if the video mode has changed, though...
    if (cl.frame.valid) {
        V_ClearScene();

        // build a refresh entity list and calc cl.sim*
        // this also calls CL_CalcViewValues which loads
        // v_forward, etc.
        CL_AddEntities();

#ifdef _DEBUG
        if (cl_testparticles->integer)
            V_TestParticles();
        if (cl_testentities->integer)
            V_TestEntities();
#if USE_DLIGHTS
        if (cl_testlights->integer)
            V_TestLights();
#endif
        if (cl_testblend->integer) {
            cl.refdef.blend[0] = 1;
            cl.refdef.blend[1] = 0.5;
            cl.refdef.blend[2] = 0.25;
            cl.refdef.blend[3] = 0.5;
        }
#endif

        // never let it sit exactly on a node line, because a water plane can
        // dissapear when viewed with the eye exactly on it.
        // the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
        cl.refdef.vieworg[0] += 1.0 / 16;
        cl.refdef.vieworg[1] += 1.0 / 16;
        cl.refdef.vieworg[2] += 1.0 / 16;

        cl.refdef.x = scr_vrect.x;
        cl.refdef.y = scr_vrect.y;
        cl.refdef.width = scr_vrect.width;
        cl.refdef.height = scr_vrect.height;

        // adjust for non-4/3 screens
        if (cl_adjustfov->integer) {
            cl.refdef.fov_y = cl.fov_y;
            cl.refdef.fov_x = V_CalcFov(cl.refdef.fov_y, cl.refdef.height, cl.refdef.width);
        } else {
            cl.refdef.fov_x = cl.fov_x;
            cl.refdef.fov_y = V_CalcFov(cl.refdef.fov_x, cl.refdef.width, cl.refdef.height);
        }

        cl.refdef.time = cl.time * 0.001;

        if (cl.frame.areabytes) {
            cl.refdef.areabits = cl.frame.areabits;
        } else {
            cl.refdef.areabits = NULL;
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

        cl.refdef.num_entities = r_numentities;
        cl.refdef.entities = r_entities;
        cl.refdef.num_particles = r_numparticles;
        cl.refdef.particles = r_particles;
#if USE_DLIGHTS
        cl.refdef.num_dlights = r_numdlights;
        cl.refdef.dlights = r_dlights;
#endif
#if USE_LIGHTSTYLES
        cl.refdef.lightstyles = r_lightstyles;
#endif

        cl.refdef.rdflags = cl.frame.ps.rdflags;

        // sort entities for better cache locality
        qsort(cl.refdef.entities, cl.refdef.num_entities, sizeof(cl.refdef.entities[0]), entitycmpfnc);
    }

    R_RenderFrame(&cl.refdef);
#ifdef _DEBUG
    if (cl_stats->integer)
#if USE_DLIGHTS
        Com_Printf("ent:%i  lt:%i  part:%i\n", r_numentities, r_numdlights, r_numparticles);
#else
        Com_Printf("ent:%i  part:%i\n", r_numentities, r_numparticles);
#endif
#endif

    V_SetLightLevel();
}