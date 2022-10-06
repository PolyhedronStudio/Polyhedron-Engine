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




//


/***
*
*
*   View Projections: First and Third -person are supported.
*
* 
***/
/**
*   @brief  Sets up a firstperson view mode.
**/
void ViewCamera::SetupFirstpersonViewProjection() {
	// If kickangles are enabled, lerp them and add to view angles.
    if (cl_kickangles->integer) {
        const PlayerState *playerState = &cl->frame.playerState;
        const PlayerState *oldPlayerState = &cl->oldframe.playerState;

        // Lerp first.
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



/***
*
*
*   Weapon Viewmodel.
*
* 
***/
/**
*	@brief	Calculate client view bobmove.
**/
void ViewCamera::CalculateBobMoveCycle( PlayerState *previousPlayerState, PlayerState *currentPlayerState ) {
    // Get our client game entity.
	ClientGameWorld *gameWorld = GetGameWorld();
	GameEntity *geClient = gameWorld->GetClientGameEntity();

	// Get frametime.
	const double frameTime = clgi.GetFrameTime();

	// Get velocity and calculate XYSpeed with that..
	const vec3_t velocity = currentPlayerState->pmove.velocity;//geClient->GetVelocity();
	// Without * FRAMETIME_S = XYSpeed = sqrtf(playerVelocity[0] * playerVelocity[0] + playerVelocity[1] * playerVelocity[1]);
    bobMoveCycle.XYSpeed = sqrtf( velocity.x * velocity.x + velocity.y * velocity.y );

	const bool isValidGroundEntity = ( cl->predictedState.groundEntityNumber >= 0 ? gameWorld->GetPODEntityByIndex( cl->predictedState.groundEntityNumber ) : false );
	// Determine if the ground entity is actually valid.
	//const bool isValidGroundEntity = ClientGameWorld::ValidateEntity( geClient->GetGroundEntityHandle() );

	if ( bobMoveCycle.XYSpeed < 5 ) {
        // Special handling for when not on ground.
        bobMoveCycle.move = 0;

        // Start at beginning of cycle again (See the else if statement.)
        bobTime = 0;
    } else if ( isValidGroundEntity || geClient->GetWaterLevel() == 2 ) {
        // So bobbing only cycles when on ground.
        if ( bobMoveCycle.XYSpeed > 450 ) {
            bobMoveCycle.move = 0.25;
		} else if ( bobMoveCycle.XYSpeed > 210 ) {
            bobMoveCycle.move = 0.125;
		} else if ( !isValidGroundEntity&& geClient->GetWaterLevel() == 2 && bobMoveCycle.XYSpeed > 100 ) {
            bobMoveCycle.move = 0.225;
		} else if ( bobMoveCycle.XYSpeed > 100 ) {
            bobMoveCycle.move = 0.0825;
		} else if ( !isValidGroundEntity && geClient->GetWaterLevel() == 2 ) {
            bobMoveCycle.move = 0.1625;
		} else {
            bobMoveCycle.move = 0.03125;
		}
    }

    // Calculate bob time, cycle, and sin fraction.
    bobMoveCycle.move /= 3.5;

	// Local frame bobtime set to total bobtime after adding the move of our frame.
    float _bobTime = ( bobTime += bobMoveCycle.move );

	// Multipl by 1.5 in case of crouching.
    if ( currentPlayerState->pmove.flags & PMF_DUCKED ) {
        _bobTime *= 1.5;
	}

	// Calculate cycle and frac sin.
    bobMoveCycle.cycle = static_cast<int64_t>( _bobTime );
    bobMoveCycle.fracSin = fabs( sin( _bobTime * M_PI ) );
}

/**
*	@brief	Calculate's view offset based on bobmove.
**/
void ViewCamera::CalculateViewOffset( PlayerState *previousPlayerState, PlayerState *currentPlayerState ) {
	//
	// I really wanted to get this right, but last time I ended up puking all my guts out so... not today.
	//
	//static constexpr float run_pitch = 0.002f;
	//static constexpr float run_roll = 0.005f;

	//static constexpr float bob_up = 0.005f;
	//static constexpr float bob_pitch = 0.002f;
	//static constexpr float bob_roll = 0.002f;
	//	// Get frametime.
	//const double frameTime = clgi.GetFrameTime();
	//// The view bob value itself.
 //   float viewBob = 0.f;
	//// Ratio of view bob.
 //   float bopRatio = 0.f;
	//// Delta between view bobs.
 //   float bobDelta = 0.f;

 //   // Get our client game entity.
	//ClientGameWorld *gameWorld = GetGameWorld();
	//GameEntity *geClient = gameWorld->GetClientGameEntity();
	//
	//// Get velocity and calculate XYSpeed with that..
	//const vec3_t velocity = currentPlayerState->pmove.velocity; //geClient->GetVelocity();

	//// Bob time? C
	////cl->moveCommand.timeSent
	//
	//vec3_t newKickAngles = vec3_zero(); //currentPlayerState->kickAngles;
	//// Add angles based on velocity
	//bobDelta = vec3_dot( velocity, viewForward );
	//newKickAngles[ vec3_t::Pitch ] += bobDelta * run_pitch;

	//// Right roll.
	//bobDelta = vec3_dot( velocity, viewRight );
	//newKickAngles[ vec3_t::Roll ] += bobDelta * run_roll;

	//// Add angles based on bob
	//bobDelta = bobMoveCycle.fracSin * bob_pitch * bobMoveCycle.XYSpeed;
	//// Adjust for crouching.
	//if ( currentPlayerState->pmove.flags & PMF_DUCKED ) {
	//	bobDelta *= 6; 
	//}
	//newKickAngles[ vec3_t::Pitch ] += bobDelta;
	//bobDelta = bobMoveCycle.fracSin * bob_roll * bobMoveCycle.XYSpeed;

	//// Adjust for crouching.
	//if ( currentPlayerState->pmove.flags & PMF_DUCKED ) {
	//	bobDelta *= 6;
	//}
	//if ( bobMoveCycle.cycle & 1 ) {
	//	bobDelta = -bobDelta;
	//}
	//newKickAngles[ vec3_t::Roll ] += bobDelta;

	//currentPlayerState->kickAngles += newKickAngles;
}

/**
*	@brief	Applies a certain view model drag effect to make it look more realistic in turns.
**/
void ViewCamera::CalculateViewWeaponDrag( ) {
	// Last facing direction.
	static vec3_t lastFacingAngles = vec3_zero();
	// Actual lag we allow to stay behind.
	static constexpr float maxViewModelLag = 1.35f;
	
	//Calculate the difference between current and last facing forward angles.
	vec3_t difference = viewForward - lastFacingAngles;

	// Actual speed to move at.
	constexpr float constSpeed = 6.15;
	float speed = constSpeed;

	// When the yaw is rotating too fast, it'll get choppy and "lag" behind. Interpolate to neatly catch up,
	// gives a more realism effect to go along with it.
	const float distance = vec3_length( difference );
	if ( distance > maxViewModelLag ) {
		speed *= distance / maxViewModelLag;
	}

	lastFacingAngles = vec3_fmaf( lastFacingAngles, speed * clgi.GetFrameTime(), difference );
	VectorNormalize( lastFacingAngles );

	difference = vec3_negate( difference );
	rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, constSpeed, difference );

	// Calculate Pitch.
	float pitch = AngleMod( rEntWeaponViewModel.angles[vec3_t::PYR::Pitch] );
	if ( pitch > 180.0f ) {
		pitch -= 360.0f;
	}
	else if ( pitch < -180.0f ) {
		pitch += 360.0f;
	}

	// Now apply to our origin.
	rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, -pitch * 0.035f, viewForward );
	rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, -pitch * 0.03f, viewRight );
	rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, -pitch * 0.02f, viewUp );
}

/**
*	Applies the Viewbob to weapon.
**/
void ViewCamera::CalculateWeaponViewBob( PlayerState *previousPlayerState, PlayerState *currentPlayerState ) {
    // Calculate gun angles based on view bob.
    vec3_t viewGunAngles = vec3_zero();
	const double frameTime = clgi.GetFrameTime();

	viewGunAngles[vec3_t::Roll] = bobMoveCycle.XYSpeed * bobMoveCycle.fracSin * 0.005f;
    viewGunAngles[vec3_t::Yaw]  = bobMoveCycle.XYSpeed * bobMoveCycle.fracSin * 0.01f;

    // Negate roll and yaw based on which bob cycle step we're in.
    if (bobMoveCycle.cycle & 1) {
        viewGunAngles[vec3_t::Roll] = -viewGunAngles[vec3_t::Roll];
        viewGunAngles[vec3_t::Yaw]  = -viewGunAngles[vec3_t::Yaw];
    }

    // Calculate pitch.
    viewGunAngles[vec3_t::Pitch] = bobMoveCycle.XYSpeed * bobMoveCycle.fracSin * 0.005f;

    // Calculate gun angles from delta view movement.
    for (int32_t i = 0 ; i < 3 ; i++) {
        // Calculate delta value.
        float delta = previousPlayerState->pmove.viewAngles[i] - currentPlayerState->pmove.viewAngles[i];

        // Keep delta within sane bounds.
        if (delta > 180) {
            delta -= 360;
        }
        if (delta < -180) {
            delta += 360;
        }
        if (delta > 45) {
            delta = 45;
        }
        if (delta < -45) {
            delta = -45;
        }

        // Special handling for gun angle Yaw.
        if (i == vec3_t::Yaw) {
            viewGunAngles[vec3_t::Roll] += 0.1f * delta;
        }

        // Apply delta to gun angles.
        viewGunAngles[i] += 0.2f * delta;
    }

	// Apply the new view gun angles to our player state.
	currentPlayerState->gunAngles = viewGunAngles;

    // Gun height
    currentPlayerState->gunOffset = vec3_zero();

	// Apply developer vweapon offsets (Easy for debugging.)
    for (int32_t i = 0 ; i < 3 ; i++) {
        currentPlayerState->gunOffset[i] += bobMoveCycle.forward[i] * (cl_vwep_y->value);
        currentPlayerState->gunOffset[i] += bobMoveCycle.right[i] * cl_vwep_x->value;
        currentPlayerState->gunOffset[i] += bobMoveCycle.up[i] * (-cl_vwep_z->value);
    }
}

/**
*	@brief	Calculates the weapon viewmodel offset (tracing it against other objects and adjusting its position to that.)
**/
void ViewCamera::TraceViewWeaponOffset() {
	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

	// Actual offsets from origin.
    constexpr float gunLengthOffset = 56.f;
    constexpr float gunRightOffset = 10.f;
    constexpr float gunUpOffset = -5.f;

	// Gun tip bounding box.
    const vec3_t gunMins = { -4, -2, -12 };
	const vec3_t gunMaxs = { 4, 8, 12 };

	// Get client entity, in order to skip it for tracing.
	GameEntity *geClient = gameWorld->GetClientGameEntity();

	// Calculate gun origin.
    vec3_t gunOrigin = vec3_fmaf( rEntWeaponViewModel.origin, gunRightOffset, viewRight );
    gunOrigin = vec3_fmaf( gunOrigin, gunUpOffset, viewUp );

	// Calculate gun tip origin.
    const vec3_t gunTipOrigin = vec3_fmaf(gunOrigin, gunLengthOffset, viewForward );

    // Perform gun tip trace.
    CLGTraceResult trace = CLG_Trace( gunOrigin, gunMins, gunMaxs, gunTipOrigin, geClient, BrushContentsMask::PlayerSolid );//clgi.Trace(gunOrigin, gunMins, gunMaxs, gunTipOrigin, geClient, BrushContentsMask::PlayerSolid); 

	// In case the trace hit anything, adjust our view model position so it doesn't stick in a wall.
    if (trace.fraction != 1.0f || trace.podEntity != nullptr) {
        rEntWeaponViewModel.origin = vec3_fmaf( trace.endPosition, -gunLengthOffset, viewForward );
        rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, -gunRightOffset, viewRight );
        rEntWeaponViewModel.origin = vec3_fmaf( rEntWeaponViewModel.origin, -gunUpOffset, viewUp );
    }
}

static uint32_t animStart = 0;
/**
*	@brief	Changes the view weapon model if the gun index has changed, and applies current animation state.
**/
void ViewCamera::UpdateViewWeaponModel( PlayerState *previousPlayerState, PlayerState *currentPlayerState ) {
    // Gun ViewModel.
    uint32_t lastModel = rEntWeaponViewModel.model;
    rEntWeaponViewModel.model = (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0);//(gun_model ? gun_model : (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0));

    // This is very ugly right now, but it'll prevent the wrong frame from popping in-screen...
    if (previousPlayerState->gunAnimationStartTime != currentPlayerState->gunAnimationStartTime) {
        animStart = cl->time;

		rEntWeaponViewModel.frame = currentPlayerState->gunAnimationStartFrame;
        rEntWeaponViewModel.oldframe = previousPlayerState->gunAnimationEndFrame;
    }
	
    // Setup the proper lerp and model frame to render this pass.
    // Moved into the if statement's else case up above.
    rEntWeaponViewModel.oldframe = rEntWeaponViewModel.frame;
    rEntWeaponViewModel.backlerp = 1.0 - SG_FrameForTime(&rEntWeaponViewModel.frame,
        GameTime(cl->time), // Current Time.
        GameTime(animStart),  // Animation Start time.
        currentPlayerState->gunAnimationFrametime,  // Current frame time.
        currentPlayerState->gunAnimationStartFrame, // Start frame.
        currentPlayerState->gunAnimationEndFrame,   // End frame.
        currentPlayerState->gunAnimationLoopCount,  // Loop count.
        currentPlayerState->gunAnimationForceLoop
    );

    // Don't allow it to go below 0, instead set it to old frame.
    if (rEntWeaponViewModel.frame < 0) {
        rEntWeaponViewModel.frame = rEntWeaponViewModel.oldframe;
    }
}

/**
*	@brief	Calculates the weapon viewmodel's origin and angles and adds it for rendering.
**/
void ViewCamera::AddWeaponViewModel() {
	int32_t shellFlags = 0;

    // No need to render the rEntWeaponViewModel in this case.
    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED) {
        return;
    }

    // Neither in this case.
    if (info_hand->integer == 2) {
        return;
    }

	/**
	*	Prepare the basic render refresh entity:
	*		- Set handedness.
	*		- Set initial origin and angles. Apply interpolated gunangles to vieworigin.
	*		- Adjust origin to high FOV if needed.
	**/
	// Set its specific rEntWeaponViewModel ID.
    rEntWeaponViewModel.id = RESERVED_LOCAL_ENTITIY_ID_GUN;


    // Setup basic render entity flags for our view weapon.
    rEntWeaponViewModel.flags = RenderEffects::MinimalLight | RenderEffects::DepthHack | RenderEffects::WeaponModel;
    if (info_hand->integer == 1) {
        rEntWeaponViewModel.flags |= RF_LEFTHAND;
    }

    // Add vieworigin and add interpolated playerstate gunoffsets.
    PlayerState *currentPlayerState = &cl->frame.playerState;
    PlayerState *oldPlayerState= &cl->oldframe.playerState;

	// Calculate the weapon view bob.
	CalculateWeaponViewBob( oldPlayerState, currentPlayerState );

	// Calculate the player's view offset.
	//CalculateViewOffset( oldPlayerState, currentPlayerState );

	// Add interpolated(between player states) gunoffset.
	rEntWeaponViewModel.origin = viewOrigin + vec3_mix( oldPlayerState->gunOffset, currentPlayerState->gunOffset, cl->lerpFraction );
    // Adjust for high fov.
    if (currentPlayerState->fov > 90) {
        vec_t ofs = (90 - currentPlayerState->fov) * 0.2f;
        rEntWeaponViewModel.origin = vec3_fmaf(rEntWeaponViewModel.origin, ofs, GetForwardViewVector());
    }
	// Add interpolated(between player states) gunAngles..
	rEntWeaponViewModel.angles = viewAngles + vec3_mix_euler( oldPlayerState->gunAngles, currentPlayerState->gunAngles, cl->lerpFraction );
//	rEntWeaponViewModel.angles = viewAngles + currentPlayerState->gunAngles;

	// Trace the weapon offset against other entities.
	TraceViewWeaponOffset();

	// Calculate the weapon view drag in case of too fast yaw changing, preventing jumping origins and
	// making it more realistic like in general.
	CalculateViewWeaponDrag();
	
	// Update the actual view model(did we change weapons? did we start another animation event?)
	UpdateViewWeaponModel( oldPlayerState, currentPlayerState );
	
    // If there is no model to render, there is no need to continue.
    if ( !rEntWeaponViewModel.model ) {
        return;
    }

	// Never lerp the origin, instant set oldorigin to current.
    rEntWeaponViewModel.oldorigin = rEntWeaponViewModel.origin;

	// In RTX mode we can apply these immediately: TODO: We still got no new OpenGL renderer so... This might be ditched some day.
    if (vid_rtx->integer) {
        rEntWeaponViewModel.flags |= shellFlags;
    }
	
    // Add the gun render entity to the current render frame.
    clge->view->AddRenderEntity( rEntWeaponViewModel );

    //// OpenGL: Yeah... 
    //if (shellFlags && !vid_rtx->integer) {
    //    rEntWeaponViewModel.alpha = 0.30f;
    //    rEntWeaponViewModel.flags |= shellFlags | RenderEffects::Translucent;
    //    clge->view->AddRenderEntity( rEntWeaponViewModel );
    //}
}



/***
*
*
*   View Vectors: Updated every time after we've set the view camera in a game's frame.
*
* 
***/
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