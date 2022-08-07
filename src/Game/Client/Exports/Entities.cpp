/***
*
*	License here.
*
*	@file
*
*	Client Game Entities Interface Implementation.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"


// Base Client Game Functionality.
#include "Game/Client/Debug.h"
#include "Game/Client/TemporaryEntities.h"

// Export classes.
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/View.h"

// Effects.
#include "Game/Client/Effects/ParticleEffects.h"

// Entitiess.
#include "Game/Client/Entities/Base/CLGBasePacketEntity.h"
#include "Game/Client/Entities/Base/CLGBaseLocalEntity.h"

// World.
#include "Game/Client/World/ClientGameWorld.h"


// WID: TODO: Gotta fix this one too.
extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;



/**
*   @brief  Parses and spawns the local class entities in the BSP Entity String.
* 
*   @details    When a class isn't locally registered, it'll automatically spawn
*               a CLGBasePacketEntity instead which has all the default behaviors that
*               you'd expect for it to be functional.
* 
*   @return True on success.
**/
qboolean ClientGameEntities::PrepareBSPEntities(const char *mapName, const char* bspString) {
	ClientGameWorld *gameWorld = GetGameWorld();
	if (gameWorld) {
		return gameWorld->PrepareBSPEntities(mapName, bspString, nullptr);
	} else {
		return false;
	}
}

/**
*   @brief  When the client receives packet entity state updates it calls into this function 
*			to update the game entity belonging to the server side entity. (defined by state.number).
*
*			When the state belongs to a new PODEntity, however it happens to have a similar matching
*			hashed classname as the previous PODEntity, we only update its pointer.
*
*           If however, the hashed classname differs, we allocate a new one matching the new hashed 
*			classname instead. 
*
*   @return True on success, false in case of trouble. (Should never happen, and if it does,
*           well... file an issue lmao.)
**/
qboolean ClientGameEntities::UpdateGameEntityFromState(PODEntity *clEntity, const EntityState* state) {
    // Sanity check. Even though it shouldn't have reached this point of execution if the entity was nullptr.
    if (!clEntity) {
        // Developer warning.
        CLG_Print( PrintType::DeveloperWarning, "Warning: ClientGameEntities::UpdateFromState called with a nullptr(clEntity)!!\n");

        return false;
    }
    // Sanity check. Even though it shouldn't have reached this point of execution if the entity was nullptr.
    if (!state) {
        // Developer warning.
        CLG_Print( PrintType::DeveloperWarning, "Warning: ClientGameEntities::UpdateFromState called with a nullptr(state)!!\n");

        return false;
    }

	// Acquire gameworld.
	ClientGameWorld *gameWorld = GetGameWorld();
    
    // Depending on the state it will either return a pointer to a new classentity type, or to an already existing in place one.
    IClientGameEntity *clgEntity = gameWorld->UpdateGameEntityFromState(state, clEntity);

	// Debug.
	if (!clgEntity) {
		CLG_Print( PrintType::DeveloperWarning, "Warning: ClientGameEntities::UpdateFromState had a nullptr returned from ClientGameWorld::UpdateGameEntityFromState\n");
		return false;
	} else {
		//// Precache and spawn entity.
		//clgEntity->Precache();
  //      clgEntity->Spawn();

	    // Do a debug print.
#ifdef _DEBUG
        PODEntity *podEntity = clgEntity->GetPODEntity();

	    const char *mapClass = clgEntity->GetTypeInfo()->mapClass; // typeinfo->classname = C++ classname.
	    uint32_t hashedMapClass = clgEntity->GetTypeInfo()->hashedMapClass; // hashed mapClass.

        if (podEntity) {
    	    CLG_Print( PrintType::Warning, fmt::format( "CLG UpdateFromState: clEntNumber={}, svEntNumber={}, mapClass={}, hashedMapClass={}\n", podEntity->clientEntityNumber, state->number, mapClass, hashedMapClass) );
        } else {
    	    CLG_Print( PrintType::Warning, fmt::format( "CLG UpdateFromState: clEntity=nullptr, svEntNumber={}, mapClass={}, hashedMapClass={}\n", state->number, mapClass, hashedMapClass ) );
        }
#endif
		return true;
	}

	return false;
}

//===============
// SVG_RunThink
//
// Runs entity thinking code for this frame if necessary
//===============
qboolean CLG_RunThink(IClientGameEntity *ent) {
    if (!ent) {
	    //SVG_PhysicsEntityWPrint(__func__, "[start of]", "nullptr entity!\n");
        return false;
    }

    // Fetch think time.
    GameTime nextThinkTime = ent->GetNextThinkTime();

    // Should we think at all? 
    // Condition A: Below 0, aka -(1+) means no thinking.
    // Condition B: > level.time, means we're still waiting before we can think.
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
		return true;
    }

    // Reset think time before thinking.
    ent->SetNextThinkTime(GameTime::zero());

#if _DEBUG
    if ( !ent->HasThinkCallback() ) {
        // Write the index, programmers may look at that thing first
        std::string errorString = "";
        if (ent->GetPODEntity()) {
            errorString += "entity (index " + std::to_string(ent->GetNumber());
        } else {
            errorString += "entity has no ServerEntity ";
        }

        // Write the targetname as well, if it exists
        if ( !ent->GetTargetName().empty() ) {
            errorString += ", name '" + ent->GetTargetName() + "'";
        }

        // Write down the C++ class name too
        errorString += ", class '";
        errorString += ent->GetTypeInfo()->classname;
        errorString += "'";

        // Close it off and state what's actually going on
        errorString += ") has a nullptr think callback \n";
        
        CLG_Print( PrintType::Warning, errorString.c_str() );

        // Return true.
        return true;
    }
#endif

    // Last but not least, let the entity execute its think behavior callback.
    ent->Think();

    return false;
}

/**
*   @brief  Runs the client game module's entity logic for a single frame.
**/
void CLG_RunServerEntity(SGEntityHandle &entityHandle);
void CLG_RunLocalClientEntity(SGEntityHandle &entityHandle);
#include "../../Shared/Physics/Physics.h"
/**
*   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
**/
void ClientGameEntities::RunPacketEntitiesDeltaFrame() {
	// Unlike for the server game, the level's framenumber, time and timeStamp
    // have already been calculated before reaching this point.

	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

	// This needs to be set of course.
	if (!gameWorld) {
		return;
	}

    // Iterate up till the amount of entities active in the current frame.
    for (int32_t entityNumber = 0; entityNumber < cl->frame.numEntities; entityNumber++) {
		// Fetch the entity index.
        const int32_t entityIndex = (cl->frame.firstEntity + entityNumber) & PARSE_ENTITIES_MASK;

		// Get POD Entity, and validate it to get our Game Entity.
		PODEntity *podEntity = gameWorld->GetPODEntityByIndex(entityIndex);
		GameEntity *gameEntity = ClientGameWorld::ValidateEntity(podEntity);

        // If invalid for whichever reason, warn and continue to next iteration.
        if (!podEntity || !gameEntity || !podEntity->inUse) {
            //Com_DPrint("ClientGameEntites::RunFrame: Entity #%i is nullptr\n", entityNumber);
            continue;
        }

		// Let the world know about the current entity we're running.
		level.currentEntity = gameEntity;
		
        // Store previous(old) origin.
        gameEntity->SetOldOrigin(gameEntity->GetOrigin());

        // If the ground entity moved, make sure we are still on it
		if (!gameEntity->GetClient()) {
			GameEntity *geGroundEntity = ClientGameWorld::ValidateEntity(gameEntity->GetGroundEntityHandle());
			if (geGroundEntity && (geGroundEntity->GetLinkCount() != gameEntity->GetGroundEntityLinkCount())) {
				// Reset ground entity.
				//gameEntity->SetGroundEntity( SGEntityHandle() );

				// Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
				//if (!(gameEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (gameEntity->GetServerFlags() & EntityServerFlags::Monster)) {
					// Check for a new ground entity that resides below this entity.
					SG_CheckGround(gameEntity); //SVG_StepMove_CheckGround(gameEntity);
				//}
			}
		}

        // Run it for a frame.
		SGEntityHandle handle = podEntity;
		SG_RunEntity(handle);
    }
}

/**
*   @brief  Gives Local Entities a chance to think. Called synchroniously to the server frames.
**/
void ClientGameEntities::RunLocalEntitiesFrame() {
	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

	// This needs to be set of course.
	if (!gameWorld) {
		return;
	}

	// Iterate through our local client side entities.
    for (int32_t localEntityNumber = MAX_WIRED_POD_ENTITIES; localEntityNumber < MAX_CLIENT_POD_ENTITIES; localEntityNumber++) {
		const int32_t entityIndex = localEntityNumber;
		PODEntity *podEntity = gameWorld->GetPODEntityByIndex(entityIndex);
		GameEntity *gameEntity = ClientGameWorld::ValidateEntity(podEntity);
		
		// If invalid for whichever reason, warn and continue to next iteration.
        if (!podEntity || !gameEntity || !podEntity->inUse) {
            //Com_DPrint("ClientGameEntites::RunFrame: Entity #%i is nullptr\n", entityNumber);
            continue;
        }

        // Admer: entity was marked for removal at the previous tick
        if (podEntity && gameEntity && (gameEntity->GetClientFlags() & EntityServerFlags::Remove)) {
            // Free server entity.
            game.world->FreePODEntity(podEntity);

            // Be sure to unset the server entity on this SVGBaseEntity for the current frame.
            // 
            // Other entities may wish to point at this entity for the current tick. By unsetting
            // the server entity we can prevent malicious situations from happening.
            //gameEntity->SetPODEntity(nullptr);

            // Skip further processing of this entity, it's removed.
            continue;
        }

		// Let the world know about the current entity we're running.
		level.currentEntity = gameEntity;

        // Store previous(old) origin.
        gameEntity->SetOldOrigin(gameEntity->GetOrigin());

        // If the ground entity moved, make sure we are still on it
		if (!gameEntity->GetClient()) {
			GameEntity *geGroundEntity = ClientGameWorld::ValidateEntity(gameEntity->GetGroundEntityHandle());
			if (geGroundEntity && (geGroundEntity->GetLinkCount() != gameEntity->GetGroundEntityLinkCount())) {
				// Reset ground entity.
				//gameEntity->SetGroundEntity( SGEntityHandle() );

				// Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
				//if (!(gameEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (gameEntity->GetServerFlags() & EntityServerFlags::Monster)) {
					// Check for a new ground entity that resides below this entity.
					SG_CheckGround(gameEntity); //SVG_StepMove_CheckGround(gameEntity);
				//}
			}
		}

		// Run it for a frame.
		SGEntityHandle handle = gameEntity;
		SG_RunEntity(handle);
    }
}

/**
*	@return	The GameEntity's hashed classname value, 0 if it has no GameEntity.
**/
uint32_t ClientGameEntities::GetHashedGameEntityClassname(PODEntity* podEntity) {
	if (podEntity) {
		if (podEntity->gameEntity) {
			// Keep it up to date with whatever the game entities type info 
			return podEntity->gameEntity->GetTypeInfo()->hashedMapClass;
		} else {
			return 0;
		}
	}

	return 0; // This should never happen of.
}

/**
*   @brief  Called for each prediction frame, so all entities can try and predict like the player does.
**/
void ClientGameEntities::RunPackEntitiesPredictionFrame() {

}

/**
*   @brief Executed whenever a server frame entity event is receieved.
**/
void ClientGameEntities::PacketEntityEvent(int32_t number) {
    // Ensure entity number is in bounds.
    if (number < 0 || number > MAX_ENTITIES) {
        CLG_Print( PrintType::DeveloperWarning, fmt::format( "ClientGameEntities::Event caught an OOB Entity(#{})\n", number ) );
        return;
	}

	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

    // Get the Game Entity.
	GameEntity *geEventTarget = gameWorld->GetGameEntityByIndex(number);

	// Get the POD Entity.
	// TODO: Add support for events that aren't just GameEntity specific, but can instead also
	// rely on just a plain old PODEntity. Do so by using the last bit, if set, it's a POD Event.
	PODEntity *podEventTarget = gameWorld->GetPODEntityByIndex(number);

	// Only proceed if both type of entities are existent. Warn otherwise.
	if (!podEventTarget) {
		CLG_Print( PrintType::DeveloperWarning, fmt::format( "({}): podEventTarget is (nullptr)!\n", __func__) );
		return;
	}

	// If valid, move on, if not, then prepare for horrible feelings of doom of course.
	if (!geEventTarget) {
		CLG_Print( PrintType::DeveloperWarning, fmt::format( "({}): geEventTarget for number(#{}) is (nullptr)!\n", __func__, number ) );
		return;
	}

	// With a valid entity we can notify it about its new received event.
	if (podEventTarget->currentState.eventID != 0) {
		//Com_DPrint("(%s): eventID != 0 for PODEntity(#%i)! podEntityTarget=(%s), geEntityTarget=(%s)\n", __func__, number, (podEventTarget ? podEventTarget->clientEntityNumber : -1), (geEventTarget ? geEventTarget->GetNumber() : -1));
	} else {
		//Com_DPrint("(%s): PODEntity(#%i): eventID(#%i) origin(%s), oldOrigin(%s)\n", __func__, number, podEventTarget->currentState.eventID, Vec3ToString(podEventTarget->currentState.origin), Vec3ToString(podEventTarget->currentState.oldOrigin));
	}

	PODEntity* clientEntity = &cs->entities[number];
	geEventTarget->OnEventID(clientEntity->currentState.eventID);

	//// Fetch the client entity.
    //PODEntity* clientEntity = &cs->entities[number];

	//// TODO: Add support for events that aren't just GameEntity specific, but can instead also
	//// rely on just a plain old PODEntity. Do so by using the last bit, if set, it's a POD Event.

    //// EF_TELEPORTER acts like an event, but is not cleared each frame
    //if ((clientEntity->currentState.effects & EntityEffectType::Teleporter)) {
    //    ParticleEffects::Teleporter(clientEntity->currentState.origin);
    //}

   // // Switch to specific execution based on a unique Event ID.
   // switch (clientEntity->currentState.eventID) {
   //     case EntityEvent::ItemRespawn:
   //         clgi.S_StartSound(NULL, number, SoundChannel::Weapon, clgi.S_RegisterSound("items/respawn1.wav"), 1, Attenuation::Idle, 0);
   //         ParticleEffects::ItemRespawn(clientEntity->currentState.origin);
   //         break;
   //     case EntityEvent::PlayerTeleport:
   //         clgi.S_StartSound(NULL, number, SoundChannel::Weapon, clgi.S_RegisterSound("misc/tele1.wav"), 1, Attenuation::Idle, 0);
   //         ParticleEffects::Teleporter(clientEntity->currentState.origin);
   //         break;
   //     case EntityEvent::Footstep:
   //         if (cl_footsteps->integer) {
			//	extern qhandle_t   cl_sfx_footsteps[4];
			//	clgi.S_StartSound(NULL, number, SoundChannel::Body, cl_sfx_footsteps[rand() & 3], 1, Attenuation::Normal, 0);
			//}
   //         break;
   //     case EntityEvent::FallShort:
   //         clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("player/land1.wav"), 1, Attenuation::Normal, 0);
   //         break;
   //     case EntityEvent::Fall:
   //         clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("*fall2.wav"), 1, Attenuation::Normal, 0);
   //         break;
   //     case EntityEvent::FallFar:
   //         clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("*fall1.wav"), 1, Attenuation::Normal, 0);
   //         break;
   // }
}

/**
*   @brief  Executed whenever a local client entity event is set.
**/
void ClientGameEntities::LocalEntityEvent(int32_t number) {

}

/**
*   @brief  Prepares all parsed server entities, as well as local entities for rendering
*			of the current frame.
**/
void ClientGameEntities::PrepareRefreshEntities() {
	// Get Gameworld, we're about to iterate.
	ClientGameWorld *gameWorld = GetGameWorld();

    // Iterate from 0 till the amount of entities present in the current frame.
    for (int32_t pointerNumber = 0; pointerNumber < cl->frame.numEntities; pointerNumber++) {
        // Get the entity state index.
        const int32_t entityIndex = (cl->frame.firstEntity + pointerNumber) & PARSE_ENTITIES_MASK;
        // Get a pointer the state of the given entity index.
        EntityState *currentEntityState = &cl->entityStates[entityIndex];
		// Get the actual entity number.
		const int32_t entityNumber = currentEntityState->number;
		// Get the actual entity to process based on the entity's state index number.
        PODEntity *clientEntity = &cs->entities[entityNumber];
		// Get the game entity to inquire.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(clientEntity->clientEntityNumber);
		// Get a const reference to the previous entity state.
		EntityState *previousEntityState = &clientEntity->previousState;
		// Setup the render entity ID for the renderer.
        const int32_t refreshEntityID = clientEntity->clientEntityNumber;

		if (!gameEntity) {
			// Ouche..?
			
			continue;
		}

		// Prepare this game entity's refresh entity.
		gameEntity->PrepareRefreshEntity(refreshEntityID, currentEntityState, previousEntityState, cl->lerpFraction);
    }

	// Iterate from 0 till the amount of entities present in the current frame.
    for (int32_t localEntityNumber = LOCAL_ENTITIES_START_INDEX; localEntityNumber < MAX_CLIENT_POD_ENTITIES; localEntityNumber++) {
        PODEntity *clientEntity = gameWorld->GetPODEntityByIndex(localEntityNumber);
		// Get the current state of the given entity index.
		EntityState *entityState = &clientEntity->currentState;
		// Get the previous state of the given entity index.
		EntityState *previousEntityState = &clientEntity->previousState;

		// Get the game entity belonging to this entity.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(clientEntity->clientEntityNumber);
		
		// Setup the render entity ID for the renderer.
        const int32_t refreshEntityID = clientEntity->clientEntityNumber;
		
		if (!gameEntity) {
			// Ouche..?

			continue;
		}

		// Go on.
		gameEntity->PrepareRefreshEntity(refreshEntityID, entityState, previousEntityState, cl->lerpFraction);
    }
}

/**
* Add the view weapon render entity to the screen. Can also be used for
* other scenarios where a depth hack is required.
**/
static uint32_t animStart = 0;
r_entity_t gunRenderEntity;
void ClientGameEntities::AddViewEntities() {
    int32_t  shellFlags = 0;

    // Hidden in bsp menu mode.
    if (info_in_bspmenu->integer) {
        return;
    }

    // No need to render the gunRenderEntity in this case.
    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED) {
        return;
    }

    // Neither in this case.
    if (info_hand->integer == 2) {
        return;
    }

    // Acquire access to View Camera.
    ViewCamera *viewCamera = clge->view->GetViewCamera();

    // View Origin and Angles.
    const vec3_t viewOrigin = viewCamera->GetViewOrigin();
    const vec3_t viewAngles = viewCamera->GetViewAngles();

    // Find states to between frames to interpolate between.
    PlayerState *currentPlayerState = &cl->frame.playerState;
    PlayerState *oldPlayerState= &cl->oldframe.playerState;

    // Gun ViewModel.
    //if (gunRenderEntity.model != cl->drawModels[currentPlayerState->gunIndex]) {
    //    gunRenderEntity.frame = 1;
    //    gunRenderEntity.oldframe = 0;
    //}
    uint32_t lastModel = gunRenderEntity.model;
    gunRenderEntity.model = (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0);//(gun_model ? gun_model : (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0));


    // This is very ugly right now, but it'll prevent the wrong frame from popping in-screen...
    if (oldPlayerState->gunAnimationStartTime != currentPlayerState->gunAnimationStartTime) {
        animStart = cl->time;
                gunRenderEntity.frame = currentPlayerState->gunAnimationStartFrame;
        gunRenderEntity.oldframe = currentPlayerState->gunAnimationEndFrame;
    }
    //if (lastModel != gunRenderEntity.model) {
    //    gunRenderEntity.frame = currentPlayerState->gunAnimationStartFrame;
    //    gunRenderEntity.oldframe = currentPlayerState->gunAnimationEndFrame;
    //}

    gunRenderEntity.id = RESERVED_LOCAL_ENTITIY_ID_GUN;

    // If there is no model to render, there is no need to continue.
    if (!gunRenderEntity.model) {
        return;
    }

    // Set up gunRenderEntity position
    for (int32_t i = 0; i < 3; i++) {
        gunRenderEntity.origin[i] = viewOrigin[i] + oldPlayerState->gunOffset[i] +
            cl->lerpFraction * (currentPlayerState->gunOffset[i] - oldPlayerState->gunOffset[i]);
        gunRenderEntity.angles = viewAngles + vec3_mix_euler(oldPlayerState->gunAngles,
                                                            currentPlayerState->gunAngles, cl->lerpFraction);
    }

    // Adjust for high fov.
    if (currentPlayerState->fov > 90) {
        vec_t ofs = (90 - currentPlayerState->fov) * 0.2f;
        gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, ofs, viewCamera->GetForwardViewVector());
    }

    // Adjust the gunRenderEntity origin so that the gunRenderEntity doesn't intersect with walls
    {
        vec3_t view_dir, right_dir, up_dir;
        vec3_t gun_real_pos, gun_tip;
        constexpr float gun_length = 56.f;
        constexpr float gun_right = 10.f;
        constexpr float gun_up = -5.f;
        static vec3_t mins = { -4, -2, -12 }, maxs = { 4, 8, 12 };

        vec3_vectors(viewAngles, &view_dir, &right_dir, &up_dir);
        gun_real_pos = vec3_fmaf(gunRenderEntity.origin, gun_right, right_dir);
        gun_real_pos = vec3_fmaf(gun_real_pos, gun_up, up_dir);
        gun_tip = vec3_fmaf(gun_real_pos, gun_length, view_dir);

        // Execute the trace for the view model weapon.

        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        TraceResult trace = clgi.Trace(gun_real_pos, mins, maxs, gun_tip, nullptr, BrushContentsMask::PlayerSolid); 

        // In case the trace hit anything, adjust our view model position so it doesn't stick in a wall.
        if (trace.fraction != 1.0f || trace.ent != nullptr)
        {
            gunRenderEntity.origin = vec3_fmaf(trace.endPosition, -gun_length, view_dir);
            gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, -gun_right, right_dir);
            gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, -gun_up, up_dir);
        }
    }

    // Do not lerp the origin at all.
    gunRenderEntity.oldorigin = gunRenderEntity.origin;

    //if (gun_frame) {
    //    gunRenderEntity.frame = gun_frame;      // Development tool
    //    gunRenderEntity.oldframe = gun_frame;   // Development tool
    //} else {
        // Setup the proper lerp and model frame to render this pass.
        // Moved into the if statement's else case up above.
        gunRenderEntity.oldframe = gunRenderEntity.frame;
        gunRenderEntity.backlerp = 1.0 - SG_FrameForTime(&gunRenderEntity.frame,
            GameTime(cl->time), // Current Time.
            GameTime(animStart),  // Animation Start time.
            currentPlayerState->gunAnimationFrametime,  // Current frame time.
            currentPlayerState->gunAnimationStartFrame, // Start frame.
            currentPlayerState->gunAnimationEndFrame,   // End frame.
            currentPlayerState->gunAnimationLoopCount,  // Loop count.
            currentPlayerState->gunAnimationForceLoop
        );

        // Don't allow it to go below 0, instead set it to old frame.
        if (gunRenderEntity.frame < 0) {
            gunRenderEntity.frame = gunRenderEntity.oldframe;
        }
        //gunRenderEntity.frame = tionFrame = renderEntity.frame;
        //gunRenderEntity.frame = 0;//currentPlayerState->gunAnimationFrame;
        //if (gunRenderEntity.frame == 0) {
        //    gunRenderEntity.oldframe = 0;   // just changed weapons, don't lerp from old
        //} else {
        //    gunRenderEntity.oldframe = 0;//oldPlayerState->gunAnimationFrame;
        //    gunRenderEntity.backlerp = 1.0f - cl->lerpFraction;
        //}
    //}

    // Setup basic render entity flags for our view weapon.
    gunRenderEntity.flags =  RenderEffects::DepthHack | RenderEffects::WeaponModel | RenderEffects::MinimalLight;
    if (info_hand->integer == 1) {
        gunRenderEntity.flags |= RF_LEFTHAND;
    }

    // Apply translucency render effect to the render entity and clamp its alpha value if nescessary.
    if (cl_gunalpha->value != 1) {
        gunRenderEntity.alpha = clgi.Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
        gunRenderEntity.flags |= RenderEffects::Translucent;
    }

    // Apply shell effects to the same entity in rtx mode.
    if (vid_rtx->integer) {
        gunRenderEntity.flags |= shellFlags;
    }

    // Add the gun render entity to the current render frame.
    clge->view->AddRenderEntity(gunRenderEntity);

    // Render a separate shell entity in non-rtx mode.
    if (shellFlags && !vid_rtx->integer) {
        gunRenderEntity.alpha = 0.30f * cl_gunalpha->value;
        gunRenderEntity.flags |= shellFlags | RenderEffects::Translucent;
        clge->view->AddRenderEntity(gunRenderEntity);
    }
}

//---------------
// ClientGameEntities::ApplyRenderEffects
//
//---------------
int32_t ClientGameEntities::ApplyRenderEffects(int32_t renderEffects) {

    //
    // NOTE: The commented code below is left here as an example
    // of what to do with this function.
    //

    //    // all of the solo colors are fine.  we need to catch any of the combinations that look bad
    //    // (double & half) and turn them into the appropriate color, and make double/quad something special
    //    if (renderEffects & RenderEffects::HalfDamShell) {
    //        // ditch the half damage shell if any of red, blue, or double are on
    //        if (renderEffects & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::DoubleShell))
    //            renderEffects &= ~RenderEffects::HalfDamShell;
    //    }
    //    if (renderEffects & RenderEffects::DoubleShell) {
    //        // lose the yellow shell if we have a red, blue, or green shell
    //        if (renderEffects & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::GreenShell))
    //            renderEffects &= ~RenderEffects::DoubleShell;
    //        // if we have a red shell, turn it to purple by adding blue
    //        if (renderEffects & RenderEffects::RedShell)
    //            renderEffects |= RenderEffects::BlueShell;
    //        // if we have a blue shell (and not a red shell), turn it to cyan by adding green
    //        else if (renderEffects & RenderEffects::BlueShell) {
    //            // go to green if it's on already, otherwise do cyan (flash green)
    //            if (renderEffects & RenderEffects::GreenShell)
    //                renderEffects &= ~RenderEffects::BlueShell;
    //            else
    //                renderEffects |= RenderEffects::GreenShell;
    //        }
    //    }

    // Just return the renderEffects as they were.
    return renderEffects;
}