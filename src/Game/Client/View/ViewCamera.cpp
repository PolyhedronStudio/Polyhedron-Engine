/***
*
*	License here.
*
*	@file
*
*	Client Game Key Binding Object.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
// Exports classes.
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/View.h"

#include "Game/Client/View/ViewCamera.h"



/**
*   @brief  Sets up a firstperson view mode.
**/
void ViewCamera::SetupFirstpersonViewProjection() {
    // If kickangles are enabled, lerp them and add to view angles.
    if (cl_kickangles->integer) {
        const PlayerState *playerState = &cl->frame.playerState;
        const PlayerState *oldPlayerState = &cl->oldframe.playerState;

        // Lerp first.
        const double lerp = cl->lerpFraction;
        const vec3_t kickAngles = vec3_mix_euler(oldPlayerState->kickAngles, playerState->kickAngles, cl->lerpFraction);

        // Add afterwards.
        viewAngles += kickAngles;
    }

    // Add the first person view entities.
    clge->entities->AddViewEntities();

    // Let the client state be known we aren't in thirdperson mode.
    cl->thirdPersonView = false;
}

/**
*   @brief  Sets up a thirdperson view mode.
**/
void ViewCamera::SetupThirdpersonViewProjection() {
    // Const vec3_t mins and maxs for box tracing the camera with.
    static const vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

    // In case of death, set a specific view angle that looks nice.
    if (cl->frame.playerState.stats[PlayerStats::Health] <= 0) {
        viewAngles[vec3_t::Roll] = 0;
        viewAngles[vec3_t::Pitch] = 10;
    }

    // Calculate focus point.
    vec3_t focus = vec3_fmaf(viewOrigin, 512, viewForward);

    // Add an additional unit to the z value.
    viewOrigin.z += 8.f;
    viewAngles[vec3_t::Pitch] *= 0.5f;
    AngleVectors(viewAngles, &viewForward, &viewRight, &viewUp);

    // Calculate view origin to use based on thirdperson range and angle.
    const float angle = Radians(cl_thirdperson_angle->value);
    const float range = cl_thirdperson_range->value;
    const float fscale = cosf(angle);
    const float rscale = sinf(angle);
    viewOrigin = vec3_fmaf(viewOrigin, -range * fscale, viewForward);
    viewOrigin = vec3_fmaf(viewOrigin, -range * rscale, viewRight);

    // TODO: Uncomment when I get back to work on thirdperson camera.
    // This is the start of having a camera that is nice third person wise.
    // 
    // Likely needs a sphere instead of box collide in order to work properly though.
    // Experimenting with a side third person view.
    //cl->refdef.vieworg = vec3_fmaf(cl->refdef.vieworg, 24, cl->v_right);
    
    // Execute a line trace to see if we collide with the world.
    TraceResult trace = clgi.Trace(cl->playerEntityOrigin, vec3_zero(), vec3_zero(), viewOrigin , nullptr, BrushContentsMask::PlayerSolid);

    if (trace.fraction != 1.0f) {
        // We've collided with the world, let's adjust our view origin.
        viewOrigin = trace.endPosition;
    }
    
    // Subtract view origin from focus point.
    focus -= viewOrigin;

    // Calculate the new distance to use.
    float dist = sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    // Set our view angles.
    viewAngles[vec3_t::Pitch] = -180.f / M_PI * atan2f(focus[2], dist);
    viewAngles[vec3_t::Yaw] -= cl_thirdperson_angle->value;

    // Last but not least, let it be known we are in thirdperson view.
    cl->thirdPersonView = true;
}

/**
*   @brief  Calculates the new forward, up, and right vectors based on
*           the camera's current viewAngles.
**/
void ViewCamera::UpdateViewVectors() {
    // Calculate new client forward, right, and up vectors.
    vec3_vectors(viewAngles, &viewForward, &viewRight, &viewUp);
}

/**
*   @brief  Calculates the new forward, up, and right vectors of
*           the view camera based on the vec3_t argument.
**/
void ViewCamera::UpdateViewVectors(const vec3_t& fromAngles) {
    // Calculate new client forward, right, and up vectors.
    vec3_vectors(fromAngles, &viewForward, &viewRight, &viewUp);
}