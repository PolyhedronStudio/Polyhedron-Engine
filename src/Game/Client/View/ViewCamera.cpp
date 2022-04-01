/***
*
*	License here.
*
*	@file
*
*	ClientGame ViewCamera Implementation.
* 
***/
// ClientGame Locals.
#include "../ClientGameLocals.h"

// Exports interfaces.
#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"

// Exports classes.
#include "../Exports/Entities.h"
#include "../Exports/View.h"

#include "ViewCamera.h"

/**
*   @brief  Sets up a firstperson view mode.
**/
void ViewCamera::SetupFirstpersonViewProjection() {
    // If kickangles are enabled, lerp them and add to view angles.
    if (cl_kickangles->integer) {
        PlayerState *playerState = &cl->frame.playerState;
        PlayerState *oldPlayerState = &cl->oldframe.playerState;

        // Lerp first.
        double lerp = cl->lerpFraction;
        kickAngles = vec3_mix_euler(oldPlayerState->kickAngles, playerState->kickAngles, cl->lerpFraction);

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
    vec3_t focus = vec3_fmaf(viewOrigin, 512, cl->v_forward);

    // Add an additional unit to the z value.
    viewOrigin.z += 8.f;
    viewAngles[vec3_t::Pitch] *= 0.5f;
    AngleVectors(viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    // Calculate view origin to use based on thirdperson range and angle.
    const float angle = Radians(cl_thirdperson_angle->value);
    const float range = cl_thirdperson_range->value;
    const float fscale = std::cosf(angle);
    const float rscale = std::sinf(angle);
    viewOrigin = vec3_fmaf(viewOrigin, -range * fscale, cl->v_forward);
    viewOrigin = vec3_fmaf(viewOrigin, -range * rscale, cl->v_right);

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
    float dist = std::sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    // Set our view angles.
    viewAngles[vec3_t::Pitch] = -180.f / M_PI * std::atan2f(focus[2], dist);
    viewAngles[vec3_t::Yaw] -= cl_thirdperson_angle->value;

    // Last but not least, let it be known we are in thirdperson view.
    cl->thirdPersonView = true;
}

/**
*   @brief  Calculates the new forward, up, and right vectors of
*           the view camera.
**/
void ViewCamera::CalculateViewVectors() {
    // Calculate new client forward, right, and up vectors.
    vec3_vectors(viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);
}

/**
*   @brief  Periodically calculates the player's horizontal speed, and interpolates it
*           over a small interval to smooth out rapid changes in velocity.
**/
float ViewCamera::CalculateBobSpeedModulus(const PlayerState *playerState) {
	//static float oldSpeed = 0.f, newSpeed = 0.f;
	//static GameTime bobModTime = GameTime::zero();

	//if (cl->time < bobModTime.count()) {
	//	bobModTime = GameTime::zero();
	//	oldSpeed = 0;
 //       newSpeed = 0;
	//}

 //   // Speed value.
	//float speed = 0;

 //   // Delta value.
	//const GameTime deltaTime = GameTime(clgi.GetRealTime()) - bobModTime;

 //   // In case of a low delta, slow down instead of increasing bob speed.
	//if (deltaTime.count() < 200) {
	//	const float lerp = (float)deltaTime.count() / (float)200.f;
	//	speed = oldSpeed + lerp * (newSpeed - oldSpeed);
	//} else {
	//	const bool isDucked = playerState->pmove.flags & PMF_DUCKED;
	//	const float maxSpeed = isDucked ? PM_SPEED_DUCKED : PM_SPEED_AIR;

 //       // Generate X/Y velocity.
 //       vec3_t velocityXY = { playerState->pmove.velocity.x, playerState->pmove.velocity.y, 0.f };

 //       // Update oldSpeed with the previous frame's "newSpeed".
	//	oldSpeed = newSpeed;

 //       // Calculate new speed value.
	//	newSpeed = vec3_length(velocityXY) / maxSpeed;
	//	newSpeed = Clampf(newSpeed , 0.f, 1.f);
	//	speed = oldSpeed;

	//	bobModTime = GameTime(clgi.GetRealTime());
	//}

	//return 0.66f + speed;
    return 0.0f;
}

/**
*   @brief  Calculate the view bob. This is done using a running time counter and a
*           simple sin function. The player's speed, as well as whether or not they
*           are on the ground, determine the bob frequency and amplitude.
**/
void ViewCamera::UpdateViewBob() {
	//if (!cl_bob->value) {
	//	return;
	//}

	//if (cgi.client->third_person) {
	//	return;
	//}

 //   PlayerState *playerState = &cl->frame.playerState;
	//if (playerState->pmove.type >= PlayerMoveType::Spectator) {
 //       // if we're frozen and not chasing, don't bob
	//	if (!playerState->stats[PlayerStats::ChaseClientID]) {
	//		return;
	//	}
	//}

	////if (cg_bob->modified) {
	////	cgi.SetCvarValue(cg_bob->name, Clampf(cg_bob->value, 0.f, 2.f));
	////	cg_bob->modified = false;
	////}

	////if (cgi.client->unclamped_time < time) {
 //   if (cl->time < bobTime.count()) {
	//	bobValue = 0.0;
 //       bobTime = GameTime::zero();
	//}

 //   // Calculate bob speed/modulus.
	//const float bobSpeedModulus = CalculateBobSpeedModulus(playerState);

	//// then calculate how much bob to add this frame
	//float frameBob = Clampf((GameTime(clgi.GetRealTime()) - bobTime).count(), 1u, 1000u) * bobSpeedModulus;

 //   // If not on-ground, increase bob strength.
	//if (!(playerState->pmove.flags & PMF_ON_GROUND)) {
	//	frameBob *= 0.25f;
	//}

 //   
 //   // Set new bob time.
	//bobTime = GameTime(clgi.GetRealTime());

 //   // Calculate new bobValue.
	//bobValue += frameBob;
	//bobValue = sinf(0.0066f * bobValue) * bobSpeedModulus * bobSpeedModulus;
	//bobValue *= 0.3;//cg_bob->value; // scale via cvar too

 //   // Apply generated bob value to view Origin.
	//viewOrigin  = vec3_fmaf(viewOrigin, -bobValue, cl->v_forward);
	//viewOrigin  = vec3_fmaf(viewOrigin,  bobValue, cl->v_right);
	//viewOrigin  = vec3_fmaf(viewOrigin,  bobValue, cl->v_up);
}