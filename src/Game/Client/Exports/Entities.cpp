/***
*
*	License here.
*
*	@file
*
*	Client Game Entities Interface Implementation.
* 
***/
// Core.
#include "../ClientGameLocals.h"

// Base Client Game Functionality.
#include "../Debug.h"
#include "../TemporaryEntities.h"

// Export classes.
#include "Entities.h"
#include "View.h"

// Effects.
#include "../Effects/ParticleEffects.h"

// Entitiess.
#include "../Entities/Base/CLGBasePacketEntity.h"
#include "../Entities/Base/CLGBaseLocalEntity.h"

// World.
#include "../World/ClientGameWorld.h"


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
qboolean ClientGameEntities::UpdateGameEntityFromState(PODEntity *clEntity, const EntityState& state) {
    // Sanity check. Even though it shouldn't have reached this point of execution if the entity was nullptr.
    if (!clEntity) {
        // Developer warning.
        Com_DPrint("Warning: ClientGameEntities::UpdateFromState called with a nullptr(clEntity)!!\n");

        return false;
    }

	// Acquire gameworld.
	ClientGameWorld *gameWorld = GetGameWorld();
    
    // Depending on the state it will either return a pointer to a new classentity type, or to an already existing in place one.
    IClientGameEntity *clgEntity = gameWorld->UpdateGameEntityFromState(state, clEntity);

	// Debug.
	if (!clgEntity) {
		Com_DPrint("Warning: ClientGameEntities::UpdateFromState had a nullptr returned from ClientGameWorld::UpdateGameEntityFromState\n");
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
    	    clgi.Com_LPrintf(PrintType::Warning, "CLG UpdateFromState: clEntNumber=%i, svEntNumber=%i, mapClass=%s, hashedMapClass=%i\n", podEntity->clientEntityNumber, state.number, mapClass, hashedMapClass);
        } else {
    	    clgi.Com_LPrintf(PrintType::Warning, "CLG UpdateFromState: clEntity=nullptr, svEntNumber=%i, mapClass=%s, hashedMapClass=%i\n", state.number, mapClass, hashedMapClass);
        }
#endif
		return true;
	}
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
        
        Com_WPrint( errorString.c_str() );

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
		// Get PODEntity.
		PODEntity *podEntity = gameWorld->GetPODEntityByIndex(entityIndex);
		// Get GameEnity.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(entityIndex);//gameWorld->GetGameEntityByIndex(12 + entityNumber);

        // If invalid for whichever reason, warn and continue to next iteration.
        if (!podEntity || !gameEntity) {
            //Com_DPrint("ClientGameEntites::RunFrame: Entity #%i is nullptr\n", entityNumber);
            continue;
        }

		// Let the world know about the current entity we're running.
		level.currentEntity = gameEntity;
		
        // Store previous(old) origin.
        gameEntity->SetOldOrigin(gameEntity->GetOrigin());

        // If the ground entity moved, make sure we are still on it
		//if (!gameEntity->GetClient()) {
			SGEntityHandle groundEntity = gameEntity->GetGroundEntityHandle();
			if (groundEntity.Get() && *groundEntity && (groundEntity->GetLinkCount() != gameEntity->GetGroundEntityLinkCount())) {
				// Reset ground entity.
				gameEntity->SetGroundEntity(nullptr);

				// Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
				if (!(gameEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (gameEntity->GetServerFlags() & EntityServerFlags::Monster)) {
					// Check for a new ground entity that resides below this entity.
					SG_CheckGround(gameEntity); //SVG_StepMove_CheckGround(gameEntity);
				}
			}
		//}

        // Run it for a frame.
        // Acquire game entity object.    
		SGEntityHandle handle = podEntity;
		SG_RunEntity(handle);
		//// Update the podEntity's hashedClassname for the next frame.
		//if (podEntity) {
		//	podEntity->previousState.hashedClassname = podEntity->currentState.hashedClassname;
		//	
		//	if (podEntity->gameEntity) {
		//		// Keep it up to date with whatever the game entities type info 
		//		podEntity->currentState.hashedClassname = podEntity->gameEntity->GetTypeInfo()->hashedMapClass;
		//	} else {
		//		podEntity->currentState.hashedClassname = 0;
		//	}
		//}
//		CLG_RunServerEntity(handle);
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
		// Fetch the entity index.
        const int32_t entityIndex = localEntityNumber;
		// Get PODEntity.
		PODEntity *podEntity = gameWorld->GetPODEntityByIndex(entityIndex);
		// Get GameEnity.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(entityIndex);//gameWorld->GetGameEntityByIndex(12 + entityNumber);

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
		//if (!gameEntity->GetClient()) {
			SGEntityHandle groundEntity = gameEntity->GetGroundEntityHandle();
			if (groundEntity.Get() && *groundEntity && (groundEntity->GetLinkCount() != gameEntity->GetGroundEntityLinkCount())) {
				// Reset ground entity.
				gameEntity->SetGroundEntity(nullptr);

				// Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
				if (!(gameEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (gameEntity->GetServerFlags() & EntityServerFlags::Monster)) {
					// Check for a new ground entity that resides below this entity.
					SG_CheckGround(gameEntity); //SVG_StepMove_CheckGround(gameEntity);
				}
			}
		//}

		// Run it for a frame.
		SGEntityHandle handle = gameEntity;
		//CLG_RunLocalClientEntity(handle);
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

///**
//*   @brief  Gets us a pointer to the entity that is currently being viewed.
//*           This could be an other client to in case of spectator mode.
//**/
//PODEntity* CLG_GetClientViewEntity(void) {
//    // Default is of course our own client entity number.
//    int32_t index = cl->clientNumber;
//
//    // However, faith has it that a chase client ID might be set, in which case we want to switch to its number instead.
//    if (cl->frame.playerState.stats[PlayerStats::ChaseClientID]) {
//        index = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;
//    }
//
//    return &cs->entities[index + 1];
//}
//
///**
//*   @return True if the specified entity is bound to the local client's view.
//**/
//qboolean CLG_IsClientViewEntity(const PODEntity* ent) {
//    // If the entity number matches, then we're good.
//    if (ent->current.number == cl->clientNumber + 1) {
//        return true;
//    }
//
//    // If not, then we are viewing an other client entity, check whether it is in corpse mode.
//    if ((ent->current.effects & EntityEffectType::Corpse) == 0) {
//        // No corpse, and modelIndex #255 indicating that we are dealing with a client entity.
//        if (ent->current.modelIndex == 255) {
//            // If the entity number matches our client number, we're good to go.
//            if (ent->current.number == cl->clientNumber) {
//                return true;
//            } 
//
//            // Otherwise we'll fetch the number of currently being chased client and see if that matches with this entity's number instead.
//            const int16_t chase = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;
//
//            // Oh boy, it matched, someone is really happy right now.
//            if (ent->current.number == chase) {
//                return true;
//            }
//        }
//    }
//
//    // All bets are off, this is no client entity which we are viewing.
//    return false;
//}
//-------------------------------------------------------------------------------------------------------------------------

/**
*   @brief Executed whenever a server frame entity event is receieved.
**/
void ClientGameEntities::PacketEntityEvent(int32_t number) {
    // Ensure entity number is in bounds.
    if (number < 0 || number > MAX_ENTITIES) {
        Com_WPrint("ClientGameEntities::Event caught an OOB Entity ID: %i\n", number);
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
		Com_WPrint("(%s): podEventTarget is (nullptr)!\n", __func__);
		return;
	}

	// If valid, move on, if not, then prepare for horrible feelings of doom of course.
	if (!geEventTarget) {
		Com_WPrint("(%s): geEventTarget for number(#%i) is (nullptr)!\n", __func__, number);
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
*   @brief  Parse the server frame for server entities to add to our client view.
*           Also applies special rendering effects to them where desired.
**/
void ClientGameEntities::AddPacketEntities() {

    //// Client Info.
    //ClientInfo*  clientInfo = nullptr;
    //// Entity specific effects. (Such as whether to rotate or not.)
    //int32_t effects = 0;
    //// Entity render effects. (Shells and the like.)
    //uint32_t renderEffects = 0;
    // Bonus items rotate at a fixed rate
    float autoRotate = AngleMod(cl->time * BASE_1_FRAMETIME);
    // Brush models can auto animate their frames
    int32_t autoAnimation = BASE_FRAMERATE * cl->time / BASE_FRAMETIME_1000;


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
        PODEntity *clientEntity = &cs->entities[entityNumber]; 		//PODEntity *clientEntity = gameWorld->GetPODEntityByIndex(entityNumber);
		// Get the game entity to inquire.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(clientEntity->clientEntityNumber);
		// Get a const reference to the previous entity state.
		EntityState *previousEntityState = &clientEntity->previousState;
		// Setup the render entity ID for the renderer.
        const int32_t refreshEntityID = clientEntity->clientEntityNumber; //clientEntity->clientEntityNumber + RESERVED_ENTITIY_COUNT + localEntityNumber;

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
        const int32_t refreshEntityID = clientEntity->clientEntityNumber; //clientEntity->clientEntityNumber + RESERVED_ENTITIY_COUNT + localEntityNumber;
		
		if (!gameEntity) {
			// Ouche..?

			continue;
		}

		//if (localEntityNumber > 1040) {
		//	return;
		//}

		// Go on.
		gameEntity->PrepareRefreshEntity(refreshEntityID, entityState, previousEntityState, cl->lerpFraction);
    }
}

/**
*   @brief  Parse the server frame for server entities to add to our client view.
*           Also applies special rendering effects to them where desired.
**/
//void ClientGameEntities::AddPacketEntities() {
//    // Render entity that is about to be passed to the current render frame.
//    r_entity_t   renderEntity = {}; // Ensure it is clear aka set to 0.
//    // State of the current entity.
//    EntityState* entityState = nullptr;
//    // Current processing client entity ptr.
//    PODEntity* clientEntity = nullptr;
//    // Client Info.
//    ClientInfo*  clientInfo = nullptr;
//    // Entity specific effects. (Such as whether to rotate or not.)
//    uint32_t effects = 0;
//    // Entity render effects. (Shells and the like.)
//    uint32_t renderEffects = 0;
//    // Bonus items rotate at a fixed rate
//    float autoRotate = AngleMod(cl->time * BASE_1_FRAMETIME);
//    // Brush models can auto animate their frames
//    int32_t autoAnimation = BASE_FRAMERATE * cl->time / BASE_FRAMETIME_1000;
//
//    // Iterate from 0 till the amount of entities present in the current frame.
//    for (int32_t pointerNumber = 0; pointerNumber < cl->frame.numEntities; pointerNumber++) {
//        // C++20: Had to be placed here because of label skip.
//        int32_t baseEntityFlags = 0;
//
//        //
//        // Fetch Entity.
//        // 
//        // Fetch the entity index.
//        int32_t entityIndex = (cl->frame.firstEntity + pointerNumber) & PARSE_ENTITIES_MASK;
//        // Fetch the state of the given entity index.
//        entityState = &cl->entityStates[entityIndex];
//        // Fetch the actual entity to process based on the entity's state index number.
//        clientEntity = &cs->entities[entityState->number];
//        // Setup the render entity ID for the renderer.
//        renderEntity.id = clientEntity->clientEntityNumber;// + RESERVED_ENTITIY_COUNT;
////
////
//// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////          class entities test code.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        //Com_DPrint("entityIndex=%i, entityState->number=%i, clientEntity->id=%i, pointernr=%i\n", entityIndex, entityState->number, clientEntity->id, pointerNumber);
//
//        // Loop through class entities, and see if their IDs still match 
//        //classEntities.UpdateFrame();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// 
//// 
//// 
//        //
//        // Effects.
//        // 
//        // Fetch the effects of current entity.
//        effects = entityState->effects;
//        // Fetch the render effects of current entity.
//        renderEffects = entityState->renderEffects;
//
//        //
//        // Frame Animation Effects.
//        //
//        if (effects & EntityEffectType::AnimCycleFrames01hz2)
//            renderEntity.frame = autoAnimation & 1;
//        else if (effects & EntityEffectType::AnimCycleFrames23hz2)
//            renderEntity.frame = 2 + (autoAnimation & 1);
//        else if (effects & EntityEffectType::AnimCycleAll2hz)
//            renderEntity.frame = autoAnimation;
//        else if (effects & EntityEffectType::AnimCycleAll30hz)
//            renderEntity.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
//	    else {
//    	    //// Fetch the iqm animation index data.
//         //   if (clientEntity->currentState.animationIndex != 0 && clientEntity->currentState.modelIndex != 0) {
//		       // model_t* iqmData = clgi.MOD_ForHandle(clientEntity->currentState.modelIndex);
//
//         //       if (iqmData) {
//			      //  Com_DPrint("WOW!!!\n");
//         //       }
//         //   } else {
//		        //renderEntity.frame = entityState->animationFrame;
//		 //       renderEntity.oldframe = clientEntity->previousState.animationFrame;
//		//        renderEntity.backlerp = 1.0 - cl->lerpFraction;
////            }
//
// 
//            //clientEntity->currentState.animationFrame, 
//	 //   framefrac = GS_FrameForTime(&curframe, cg.time, viewweapon->baseAnimStartTime,  // start time
//		//weaponInfo->frametime[viewweapon->baseAnim],				    // current frame time?
//		//weaponInfo->firstframe[viewweapon->baseAnim],				    // first frame.
//		//weaponInfo->lastframe[viewweapon->baseAnim],				    // last frame.
//		//weaponInfo->loopingframes[viewweapon->baseAnim],			    // looping frames.
//		//true); 
//        }
//        
//
//        // Optionally remove the glowing effect.
//        if (cl_noglow->integer)
//            renderEffects &= ~RenderEffects::Glow;
//
//        // Setup the proper lerp and model frame to render this pass.
//        // Moved into the if statement's else case up above.
//        renderEntity.oldframe = clientEntity->previousState.animationFrame;
//        renderEntity.backlerp = 1.0 - SG_FrameForTime(&renderEntity.frame,
//            GameTime(cl->serverTime),                                     // Current Time.
//            GameTime(clientEntity->currentState.animationStartTime),           // Animation Start time. (TODO: This needs to changed to a stored cl->time of the moment where the animation event got through.)
//            clientEntity->currentState.animationFramerate,           // Current frame time.
//            clientEntity->currentState.animationStartFrame,          // Start frame.
//            clientEntity->currentState.animationEndFrame,            // End frame.
//            0,                                                  // Loop count.
//            true                                                // Force loop
//        );
//        clientEntity->currentState.animationFrame = renderEntity.frame;
//    //clientEntity->previousState.animationFrame = clientEntity->currentState.animationFrame;
//
//        //
//        // Setup renderEntity origin.
//        //
//        if (renderEffects & RenderEffects::FrameLerp) {
//            // Step origin discretely, because the model frames do the animation properly.
//            renderEntity.origin = clientEntity->currentState.origin;
//            renderEntity.oldorigin = clientEntity->currentState.oldOrigin;
//        } else if (renderEffects & RenderEffects::Beam) {
//            // Interpolate start and end points for beams
//            renderEntity.origin = vec3_mix(clientEntity->previousState.origin, clientEntity->currentState.origin, cl->lerpFraction);
//            renderEntity.oldorigin = vec3_mix(clientEntity->previousState.oldOrigin, clientEntity->currentState.oldOrigin, cl->lerpFraction);
//        } else {
//            if (entityState->number == cl->frame.clientNumber + 1) {
//                // In case of this being our actual client entity, we use the predicted origin.
//                renderEntity.origin = cl->playerEntityOrigin;
//                renderEntity.oldorigin = cl->playerEntityOrigin;
//            } else {
//                // Ohterwise, just neatly interpolate the origin.
//                renderEntity.origin = vec3_mix(clientEntity->previousState.origin, clientEntity->currentState.origin, cl->lerpFraction);
//                // Neatly copy it as the renderEntity's oldorigin.
//                renderEntity.oldorigin = renderEntity.origin;
//            }
//        }
//
//	    // Draw debug bounding box for client entity.
//	    if (renderEffects & RenderEffects::DebugBoundingBox) {
//	        CLG_DrawDebugBoundingBox(clientEntity->lerpOrigin, clientEntity->mins, clientEntity->maxs);
//	    }
//
//        // tweak the color of beams
//        if (renderEffects & RenderEffects::Beam) {
//            // The four beam colors are encoded in 32 bits of skinNumber (hack)
//            renderEntity.alpha = 0.30;
//            renderEntity.skinNumber = (entityState->skinNumber >> ((rand() % 4) * 8)) & 0xff;
//            renderEntity.model = 0;
//        } else {
//            //
//            // Set the entity model skin
//            //
//            if (entityState->modelIndex == 255) {
//                // Use a custom player skin
//                clientInfo = &cl->clientInfo[entityState->skinNumber & 255];
//                renderEntity.skinNumber = 0;
//                renderEntity.skin = clientInfo->skin;
//                renderEntity.model = clientInfo->model;
//
//                // Setup default base client info in case of 0.
//                if (!renderEntity.skin || !renderEntity.model) {
//                    renderEntity.skin = cl->baseClientInfo.skin;
//                    renderEntity.model = cl->baseClientInfo.model;
//                    clientInfo = &cl->baseClientInfo;
//                }
//
//                // Special Disguise render effect handling.
//                if (renderEffects & RenderEffects::UseDisguise) {
//                    char buffer[MAX_QPATH];
//
//                    Q_concat(buffer, sizeof(buffer), "players/", clientInfo->model_name, "/disguise.pcx", NULL);
//                    renderEntity.skin = clgi.R_RegisterSkin(buffer);
//                }
//            } else {
//                // Default entity skin number handling behavior.
//                renderEntity.skinNumber = entityState->skinNumber;
//                renderEntity.skin = 0;
//                renderEntity.model = cl->drawModels[entityState->modelIndex];
//
//                // Disable shadows on lasers and dm spots.
//                if (renderEntity.model == cl_mod_laser || renderEntity.model == cl_mod_dmspot)
//                    renderEffects |= RF_NOSHADOW;
//            }
//        }
//
//        // Only used for black hole model right now, FIXME: do better
//        if ((renderEffects & RenderEffects::Translucent) && !(renderEffects & RenderEffects::Beam)) {
//            renderEntity.alpha = 0.70;
//        }
//
//        // Render effects (fullbright, translucent, etc)
//        if ((effects & EntityEffectType::ColorShell)) {
//            renderEntity.flags = 0;  // Render effects go on color shell entity
//        } else {
//            renderEntity.flags = renderEffects;
//        }
//
//        //
//        // Angles.
//        //
//        if (effects & EntityEffectType::Rotate) {
//            // Autorotate for bonus item entities.
//            renderEntity.angles[0] = 0;
//            renderEntity.angles[1] = autoRotate;
//            renderEntity.angles[2] = 0;
//        } else if (entityState->number == cl->frame.clientNumber + 1) {
//            // Predicted angles for client entities.
//            renderEntity.angles = cl->playerEntityAngles;
//        } else {
//            // Otherwise, lerp angles by default.
//            renderEntity.angles = vec3_mix(clientEntity->previousState.angles, clientEntity->currentState.angles, cl->lerpFraction);
//
//            // Mimic original ref_gl "leaning" bug (uuugly!)
//            if (entityState->modelIndex == 255 && cl_rollhack->integer) {
//                renderEntity.angles[vec3_t::Roll] = -renderEntity.angles[vec3_t::Roll];
//            }
//        }
//
//        //
//        // Entity Effects for in case the entity is the actual client.
//        //
//        if (entityState->number == cl->frame.clientNumber + 1) {
//            if (!cl->thirdPersonView)
//            {
//                // Special case handling for RTX rendering. Only draw third person model from mirroring surfaces.
//                if (vid_rtx->integer) {
//                    baseEntityFlags |= RenderEffects::ViewerModel;
//				} else {
//					// Assign renderEntity origin to clientEntity lerp origin in the case of a skip.
//			        clientEntity->lerpOrigin = renderEntity.origin;
//					continue;
//				}
//            }
//
//            // Don't tilt the model - looks weird
//            renderEntity.angles[0] = 0.f;
//
//            //
//            // TODO: This needs to be fixed properly for the shadow to render.
//            // 
//            // Offset the model back a bit to make the view point located in front of the head
//            //constexpr float offset = -15.f;
//            //constexpr float offset = 8.f;// 0.0f;
//            //vec3_t angles = { 0.f, renderEntity.angles[1], 0.f };
//            //vec3_t forward;
//            //AngleVectors(angles, &forward, NULL, NULL);
//            //renderEntity.origin = vec3_fmaf(renderEntity.origin, offset, forward);
//            //renderEntity.oldorigin = vec3_fmaf(renderEntity.oldorigin, offset, forward);
//
//            // Temporary fix, not quite perfect though. Add some z offset so the shadow isn't too dark under the feet.
//            renderEntity.origin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
//            renderEntity.oldorigin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
//        }
//
//        // If set to invisible, skip
//        if (!entityState->modelIndex) {
//			// Assign renderEntity origin to clientEntity lerp origin in the case of a skip.
//			clientEntity->lerpOrigin = renderEntity.origin;
//			continue;
//        }
//
//        // Add the baseEntityFlags to the renderEntity flags.
//        renderEntity.flags |= baseEntityFlags;
//
//        // In rtx mode, the base entity has the renderEffects for shells
//        if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
//            renderEffects = ApplyRenderEffects(renderEffects);
//            renderEntity.flags |= renderEffects;
//        }
//
//        // Last but not least, add the entity to the refresh render list.
//        clge->view->AddRenderEntity(renderEntity);
//
//        // Keeping it here commented to serve as an example case.
//        // Add dlights for flares
//        //model_t* model;
//        //if (renderEntity.model && !(renderEntity.model & 0x80000000) &&
//        //    (model = clgi.MOD_ForHandle(renderEntity.model)))
//        //{
//        //    if (model->model_class == MCLASS_FLARE)
//        //    {
//        //        float phase = (float)cl->time * 0.03f + (float)renderEntity.id;
//        //        float anim = sinf(phase);
//
//        //        float offset = anim * 1.5f + 5.f;
//        //        float brightness = anim * 0.2f + 0.8f;
//
//        //        vec3_t origin;
//        //        VectorCopy(renderEntity.origin, origin);
//        //        origin[2] += offset;
//
//        //        V_AddLightEx(origin, 500.f, 1.6f * brightness, 1.0f * brightness, 0.2f * brightness, 5.f);
//        //    }
//        //}
//
//        // For color shells we generate a separate entity for the main model.
//        // (Use the settings of the already rendered model, and apply translucency to it.
//        if ((effects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
//            renderEffects = ApplyRenderEffects(renderEffects);
//            renderEntity.flags = renderEffects | baseEntityFlags | RenderEffects::Translucent;
//            renderEntity.alpha = 0.30;
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        renderEntity.skin = 0;       // never use a custom skin on others
//        renderEntity.skinNumber = 0;
//        renderEntity.flags = baseEntityFlags;
//        renderEntity.alpha = 0;
//
//        //
//        // ModelIndex2
//        // 
//        // Add an entity to the current rendering frame that has model index 2 attached to it.
//        // Duplicate for linked models
//        if (entityState->modelIndex2) {
//            if (entityState->modelIndex2 == 255) {
//                // Custom weapon
//                clientInfo = &cl->clientInfo[entityState->skinNumber & 0xff];
//                
//                // Determine skinIndex.
//                int32_t skinIndex = (entityState->skinNumber >> 8); // 0 is default weapon model
//                if (skinIndex < 0 || skinIndex > cl->numWeaponModels - 1) {
//                    skinIndex = 0;
//                }
//
//                // Fetch weapon model.
//                renderEntity.model = clientInfo->weaponmodel[skinIndex];
//
//                // If invalid, use defaults.
//                if (!renderEntity.model) {
//                    if (skinIndex != 0) {
//                        renderEntity.model = clientInfo->weaponmodel[0];
//                    }
//                    if (!renderEntity.model) {
//                        renderEntity.model = cl->baseClientInfo.weaponmodel[0];
//                    }
//                }
//            } else {
//                renderEntity.model = cl->drawModels[entityState->modelIndex2];
//            }
//
//
//            if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
//                renderEntity.flags |= renderEffects;
//            }
//
//            clge->view->AddRenderEntity(renderEntity);
//
//            //PGM - make sure these get reset.
//            renderEntity.flags = baseEntityFlags;
//            renderEntity.alpha = 0;
//        }
//
//        //
//        // ModelIndex3
//        // 
//        // Add an entity to the current rendering frame that has model index 3 attached to it.
//        if (entityState->modelIndex3) {
//            renderEntity.model = cl->drawModels[entityState->modelIndex3];
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        //
//        // ModelIndex4
//        // 
//        // Add an entity to the current rendering frame that has model index 4 attached to it.
//        if (entityState->modelIndex4) {
//            renderEntity.model = cl->drawModels[entityState->modelIndex4];
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        //
//        // Particle Trail Effects.
//        // 
//        // Add automatic particle trail effects where desired.
//        if (effects & ~EntityEffectType::Rotate) {
//            if (effects & EntityEffectType::Gib) {
//                ParticleEffects::DiminishingTrail(clientEntity->lerpOrigin, renderEntity.origin, clientEntity, effects);
//            } else if (effects & EntityEffectType::Torch) {
//                const float anim = sinf((float)renderEntity.id + ((float)cl->time / 60.f + frand() * 3.3)) / (3.14356 - (frand() / 3.14356));
//                const float offset = anim * 0.0f;
//                const float brightness = anim * 1.2f + 1.6f;
//                const vec3_t origin = { 
//                    renderEntity.origin.x,
//                    renderEntity.origin.y,
//                    renderEntity.origin.z + offset 
//                };
//
//                clge->view->AddLight(origin, vec3_t{ 1.0f * brightness, 0.425f * brightness, 0.1f * brightness }, 25.f, 3.6f);
//            }
//        }
//
//    }
//
//	ClientGameWorld *gameWorld = GetGameWorld();
//	GameEntityVector gameEntities = gameWorld->GetGameEntities();
//
//    // Iterate from 0 till the amount of entities present in the current frame.
//    for (int32_t localEntityNumber = 2048; localEntityNumber < MAX_CLIENT_POD_ENTITIES; localEntityNumber++) {
//        // C++20: Had to be placed here because of label skip.
//        int32_t baseEntityFlags = 0;
//
//        //
//        // Fetch Entity.
//        // 
//        // Fetch the entity index.
//        int32_t entityIndex = localEntityNumber;
//
//        // Fetch the actual entity to process based on the entity's state index number.
//        clientEntity = gameWorld->GetPODEntityByIndex(localEntityNumber);
//		// Fetch the state of the given entity index.
//		entityState = &clientEntity->currentState;
//		// Fetch the game entity belonging to this entity.
//        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex(localEntityNumber);
//		// Setup the render entity ID for the renderer.
//        renderEntity.id = clientEntity->clientEntityNumber + RESERVED_ENTITIY_COUNT;
////
////
//// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////          class entities test code.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        //Com_DPrint("entityIndex=%i, entityState->number=%i, clientEntity->id=%i, pointernr=%i\n", entityIndex, entityState->number, clientEntity->id, pointerNumber);
//
//        // Loop through class entities, and see if their IDs still match 
//        //classEntities.UpdateFrame();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// 
//// 
//// 
//        //
//        // Effects.
//        // 
//        // Fetch the effects of current entity.
//        effects = entityState->effects;
//        // Fetch the render effects of current entity.
//        renderEffects = entityState->renderEffects;
//
//        //
//        // Frame Animation Effects.
//        //
//        if (effects & EntityEffectType::AnimCycleFrames01hz2)
//            renderEntity.frame = autoAnimation & 1;
//        else if (effects & EntityEffectType::AnimCycleFrames23hz2)
//            renderEntity.frame = 2 + (autoAnimation & 1);
//        else if (effects & EntityEffectType::AnimCycleAll2hz)
//            renderEntity.frame = autoAnimation;
//        else if (effects & EntityEffectType::AnimCycleAll30hz)
//            renderEntity.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
//	    else {
//    	    //// Fetch the iqm animation index data.
//         //   if (clientEntity->currentState.animationIndex != 0 && clientEntity->currentState.modelIndex != 0) {
//		       // model_t* iqmData = clgi.MOD_ForHandle(clientEntity->currentState.modelIndex);
//
//         //       if (iqmData) {
//			      //  Com_DPrint("WOW!!!\n");
//         //       }
//         //   } else {
//		        //renderEntity.frame = entityState->animationFrame;
//		 //       renderEntity.oldframe = clientEntity->previousState.animationFrame;
//		//        renderEntity.backlerp = 1.0 - cl->lerpFraction;
////            }
//
// 
//            //clientEntity->currentState.animationFrame, 
//	 //   framefrac = GS_FrameForTime(&curframe, cg.time, viewweapon->baseAnimStartTime,  // start time
//		//weaponInfo->frametime[viewweapon->baseAnim],				    // current frame time?
//		//weaponInfo->firstframe[viewweapon->baseAnim],				    // first frame.
//		//weaponInfo->lastframe[viewweapon->baseAnim],				    // last frame.
//		//weaponInfo->loopingframes[viewweapon->baseAnim],			    // looping frames.
//		//true); 
//        }
//        
//
//        // Optionally remove the glowing effect.
//        if (cl_noglow->integer)
//            renderEffects &= ~RenderEffects::Glow;
//
//        // Setup the proper lerp and model frame to render this pass.
//        // Moved into the if statement's else case up above.
//        renderEntity.oldframe = clientEntity->previousState.animationFrame;
//        renderEntity.backlerp = 1.0 - SG_FrameForTime(&renderEntity.frame,
//            GameTime(cl->serverTime),                                     // Current Time.
//            GameTime(clientEntity->currentState.animationStartTime),           // Animation Start time. (TODO: This needs to changed to a stored cl->time of the moment where the animation event got through.)
//            clientEntity->currentState.animationFramerate,           // Current frame time.
//            clientEntity->currentState.animationStartFrame,          // Start frame.
//            clientEntity->currentState.animationEndFrame,            // End frame.
//            0,                                                  // Loop count.
//            true                                                // Force loop
//        );
//        clientEntity->currentState.animationFrame = renderEntity.frame;
//    //clientEntity->previousState.animationFrame = clientEntity->currentState.animationFrame;
//
//        //
//        // Setup renderEntity origin.
//        //
//        if (renderEffects & RenderEffects::FrameLerp) {
//            // Step origin discretely, because the model frames do the animation properly.
//            renderEntity.origin = clientEntity->currentState.origin;
//            renderEntity.oldorigin = clientEntity->currentState.oldOrigin;
//        } else if (renderEffects & RenderEffects::Beam) {
//            // Interpolate start and end points for beams
//            renderEntity.origin = vec3_mix(clientEntity->previousState.origin, clientEntity->currentState.origin, cl->lerpFraction);
//            renderEntity.oldorigin = vec3_mix(clientEntity->previousState.oldOrigin, clientEntity->currentState.oldOrigin, cl->lerpFraction);
//        } else {
//            if (entityState->number == cl->frame.clientNumber + 1) {
//                // In case of this being our actual client entity, we use the predicted origin.
//                renderEntity.origin = cl->playerEntityOrigin;
//                renderEntity.oldorigin = cl->playerEntityOrigin;
//            } else {
//                // Ohterwise, just neatly interpolate the origin.
//                renderEntity.origin = vec3_mix(clientEntity->previousState.origin, clientEntity->currentState.origin, cl->lerpFraction);
//                // Neatly copy it as the renderEntity's oldorigin.
//                renderEntity.oldorigin = renderEntity.origin;
//            }
//        }
//
//	    // Draw debug bounding box for client entity.
//	    if (renderEffects & RenderEffects::DebugBoundingBox) {
//	        CLG_DrawDebugBoundingBox(clientEntity->lerpOrigin, clientEntity->mins, clientEntity->maxs);
//	    }
//
//        // tweak the color of beams
//        if (renderEffects & RenderEffects::Beam) {
//            // The four beam colors are encoded in 32 bits of skinNumber (hack)
//            renderEntity.alpha = 0.30;
//            renderEntity.skinNumber = (entityState->skinNumber >> ((rand() % 4) * 8)) & 0xff;
//            renderEntity.model = 0;
//        } else {
//            //
//            // Set the entity model skin
//            //
//            if (entityState->modelIndex == 255) {
//                // Use a custom player skin
//                clientInfo = &cl->clientInfo[entityState->skinNumber & 255];
//                renderEntity.skinNumber = 0;
//                renderEntity.skin = clientInfo->skin;
//                renderEntity.model = clientInfo->model;
//
//                // Setup default base client info in case of 0.
//                if (!renderEntity.skin || !renderEntity.model) {
//                    renderEntity.skin = cl->baseClientInfo.skin;
//                    renderEntity.model = cl->baseClientInfo.model;
//                    clientInfo = &cl->baseClientInfo;
//                }
//
//                // Special Disguise render effect handling.
//                if (renderEffects & RenderEffects::UseDisguise) {
//                    char buffer[MAX_QPATH];
//
//                    Q_concat(buffer, sizeof(buffer), "players/", clientInfo->model_name, "/disguise.pcx", NULL);
//                    renderEntity.skin = clgi.R_RegisterSkin(buffer);
//                }
//            } else {
//                // Default entity skin number handling behavior.
//                renderEntity.skinNumber = entityState->skinNumber;
//                renderEntity.skin = 0;
//                renderEntity.model = cl->drawModels[entityState->modelIndex];
//
//                // Disable shadows on lasers and dm spots.
//                if (renderEntity.model == cl_mod_laser || renderEntity.model == cl_mod_dmspot)
//                    renderEffects |= RF_NOSHADOW;
//            }
//        }
//
//        // Only used for black hole model right now, FIXME: do better
//        if ((renderEffects & RenderEffects::Translucent) && !(renderEffects & RenderEffects::Beam)) {
//            renderEntity.alpha = 0.70;
//        }
//
//        // Render effects (fullbright, translucent, etc)
//        if ((effects & EntityEffectType::ColorShell)) {
//            renderEntity.flags = 0;  // Render effects go on color shell entity
//        } else {
//            renderEntity.flags = renderEffects;
//        }
//
//        //
//        // Angles.
//        //
//        if (effects & EntityEffectType::Rotate) {
//            // Autorotate for bonus item entities.
//            renderEntity.angles[0] = 0;
//            renderEntity.angles[1] = autoRotate;
//            renderEntity.angles[2] = 0;
//        } else if (entityState->number == cl->frame.clientNumber + 1) {
//            // Predicted angles for client entities.
//            renderEntity.angles = cl->playerEntityAngles;
//        } else {
//            // Otherwise, lerp angles by default.
//            renderEntity.angles = vec3_mix(clientEntity->previousState.angles, clientEntity->currentState.angles, cl->lerpFraction);
//
//            // Mimic original ref_gl "leaning" bug (uuugly!)
//            if (entityState->modelIndex == 255 && cl_rollhack->integer) {
//                renderEntity.angles[vec3_t::Roll] = -renderEntity.angles[vec3_t::Roll];
//            }
//        }
//
//        //
//        // Entity Effects for in case the entity is the actual client.
//        //
//        if (entityState->number == cl->frame.clientNumber + 1) {
//            if (!cl->thirdPersonView)
//            {
//                // Special case handling for RTX rendering. Only draw third person model from mirroring surfaces.
//                if (vid_rtx->integer) {
//                    baseEntityFlags |= RenderEffects::ViewerModel;
//				} else {
//					// Assign renderEntity origin to clientEntity lerp origin in the case of a skip.
//					clientEntity->lerpOrigin = renderEntity.origin;
//							continue;
//				}
//            }
//
//            // Don't tilt the model - looks weird
//            renderEntity.angles[0] = 0.f;
//
//            //
//            // TODO: This needs to be fixed properly for the shadow to render.
//            // 
//            // Offset the model back a bit to make the view point located in front of the head
//            //constexpr float offset = -15.f;
//            //constexpr float offset = 8.f;// 0.0f;
//            //vec3_t angles = { 0.f, renderEntity.angles[1], 0.f };
//            //vec3_t forward;
//            //AngleVectors(angles, &forward, NULL, NULL);
//            //renderEntity.origin = vec3_fmaf(renderEntity.origin, offset, forward);
//            //renderEntity.oldorigin = vec3_fmaf(renderEntity.oldorigin, offset, forward);
//
//            // Temporary fix, not quite perfect though. Add some z offset so the shadow isn't too dark under the feet.
//            renderEntity.origin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
//            renderEntity.oldorigin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
//        }
//
//        // If set to invisible, skip
//        if (!entityState->modelIndex) {
//        // Assign renderEntity origin to clientEntity lerp origin in the case of a skip.
//        clientEntity->lerpOrigin = renderEntity.origin;
//		continue;
//        }
//
//        // Add the baseEntityFlags to the renderEntity flags.
//        renderEntity.flags |= baseEntityFlags;
//
//        // In rtx mode, the base entity has the renderEffects for shells
//        if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
//            renderEffects = ApplyRenderEffects(renderEffects);
//            renderEntity.flags |= renderEffects;
//        }
//
//        // Last but not least, add the entity to the refresh render list.
//        clge->view->AddRenderEntity(renderEntity);
//
//        // Keeping it here commented to serve as an example case.
//        // Add dlights for flares
//        //model_t* model;
//        //if (renderEntity.model && !(renderEntity.model & 0x80000000) &&
//        //    (model = clgi.MOD_ForHandle(renderEntity.model)))
//        //{
//        //    if (model->model_class == MCLASS_FLARE)
//        //    {
//        //        float phase = (float)cl->time * 0.03f + (float)renderEntity.id;
//        //        float anim = sinf(phase);
//
//        //        float offset = anim * 1.5f + 5.f;
//        //        float brightness = anim * 0.2f + 0.8f;
//
//        //        vec3_t origin;
//        //        VectorCopy(renderEntity.origin, origin);
//        //        origin[2] += offset;
//
//        //        V_AddLightEx(origin, 500.f, 1.6f * brightness, 1.0f * brightness, 0.2f * brightness, 5.f);
//        //    }
//        //}
//
//        // For color shells we generate a separate entity for the main model.
//        // (Use the settings of the already rendered model, and apply translucency to it.
//        if ((effects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
//            renderEffects = ApplyRenderEffects(renderEffects);
//            renderEntity.flags = renderEffects | baseEntityFlags | RenderEffects::Translucent;
//            renderEntity.alpha = 0.30;
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        renderEntity.skin = 0;       // never use a custom skin on others
//        renderEntity.skinNumber = 0;
//        renderEntity.flags = baseEntityFlags;
//        renderEntity.alpha = 0;
//
//        //
//        // ModelIndex2
//        // 
//        // Add an entity to the current rendering frame that has model index 2 attached to it.
//        // Duplicate for linked models
//        if (entityState->modelIndex2) {
//            if (entityState->modelIndex2 == 255) {
//                // Custom weapon
//                clientInfo = &cl->clientInfo[entityState->skinNumber & 0xff];
//                
//                // Determine skinIndex.
//                int32_t skinIndex = (entityState->skinNumber >> 8); // 0 is default weapon model
//                if (skinIndex < 0 || skinIndex > cl->numWeaponModels - 1) {
//                    skinIndex = 0;
//                }
//
//                // Fetch weapon model.
//                renderEntity.model = clientInfo->weaponmodel[skinIndex];
//
//                // If invalid, use defaults.
//                if (!renderEntity.model) {
//                    if (skinIndex != 0) {
//                        renderEntity.model = clientInfo->weaponmodel[0];
//                    }
//                    if (!renderEntity.model) {
//                        renderEntity.model = cl->baseClientInfo.weaponmodel[0];
//                    }
//                }
//            } else {
//                renderEntity.model = cl->drawModels[entityState->modelIndex2];
//            }
//
//
//            if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
//                renderEntity.flags |= renderEffects;
//            }
//
//            clge->view->AddRenderEntity(renderEntity);
//
//            //PGM - make sure these get reset.
//            renderEntity.flags = baseEntityFlags;
//            renderEntity.alpha = 0;
//        }
//
//        //
//        // ModelIndex3
//        // 
//        // Add an entity to the current rendering frame that has model index 3 attached to it.
//        if (entityState->modelIndex3) {
//            renderEntity.model = cl->drawModels[entityState->modelIndex3];
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        //
//        // ModelIndex4
//        // 
//        // Add an entity to the current rendering frame that has model index 4 attached to it.
//        if (entityState->modelIndex4) {
//            renderEntity.model = cl->drawModels[entityState->modelIndex4];
//            clge->view->AddRenderEntity(renderEntity);
//        }
//
//        //
//        // Particle Trail Effects.
//        // 
//        // Add automatic particle trail effects where desired.
//        if (effects & ~EntityEffectType::Rotate) {
//            if (effects & EntityEffectType::Gib) {
//                ParticleEffects::DiminishingTrail(clientEntity->lerpOrigin, renderEntity.origin, clientEntity, effects);
//            } else if (effects & EntityEffectType::Torch) {
//                const float anim = sinf((float)renderEntity.id + ((float)cl->time / 60.f + frand() * 3.3)) / (3.14356 - (frand() / 3.14356));
//                const float offset = anim * 0.0f;
//                const float brightness = anim * 1.2f + 1.6f;
//                const vec3_t origin = { 
//                    renderEntity.origin.x,
//                    renderEntity.origin.y,
//                    renderEntity.origin.z + offset 
//                };
//
//                clge->view->AddLight(origin, vec3_t{ 1.0f * brightness, 0.425f * brightness, 0.1f * brightness }, 25.f, 3.6f);
//            }
//        }
//    }
//}

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
    gunRenderEntity.flags = RenderEffects::MinimalLight | RenderEffects::DepthHack | RenderEffects::WeaponModel;
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