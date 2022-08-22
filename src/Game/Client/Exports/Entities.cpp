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

// Physics
#include "Game/Shared/Physics/Physics.h"

// WID: TODO: Gotta fix this one too.
extern qhandle_t cl_mod_laser;





/**
*
*
*	Client Local BSP Entities.
*
*
**/
/**
*   @brief  Parses and spawns the local class entities in the BSP Entity String.
* 
*   @details    When a class isn't locally registered, it'll automatically spawn
*               a CLGBasePacketEntity instead which has all the default behaviors that
*               you'd expect for it to be functional.
* 
*   @return True on success.
**/
qboolean ClientGameEntities::PrepareBSPEntities( const char *mapName, const char* bspString ) {
	ClientGameWorld *gameWorld = GetGameWorld();
	if (gameWorld) {
		return gameWorld->PrepareBSPEntities(mapName, bspString, nullptr);
	} else {
		return false;
	}
}



/**
*
*
*	'Game' Entities.
*
*
**/
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
qboolean ClientGameEntities::UpdateGameEntityFromState( PODEntity *clEntity, const EntityState* state ) {
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



/**
*
*
*	Entity Events.
*
*
**/
/**
*   @brief Executed whenever a server frame entity event is receieved.
**/
void ClientGameEntities::PacketEntityEvent( int32_t number ) {
    // Ensure entity number is in bounds.
    if ( number < 0 || number > MAX_CLIENT_POD_ENTITIES ) {
        CLG_Print( PrintType::DeveloperWarning, fmt::format( "ClientGameEntities::Event caught an OOB Entity(#{})\n", number ) );
        return;
	}

	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

    // Get the Game Entity.
	GameEntity *geEventTarget = gameWorld->GetGameEntityByIndex( number );

	// Get the POD Entity.
	// TODO: Add support for events that aren't just GameEntity specific, but can instead also
	// rely on just a plain old PODEntity. Do so by using the last bit, if set, it's a POD Event.
	PODEntity *podEventTarget = gameWorld->GetPODEntityByIndex( number );

	// Only proceed if both type of entities are existent. Warn otherwise.
	if ( !podEventTarget ) {
		CLG_Print( PrintType::DeveloperWarning, fmt::format( "({}): podEventTarget is (nullptr)!\n", __func__) );
		return;
	}

	// If valid, move on, if not, then prepare for horrible feelings of doom of course.
	if ( !geEventTarget ) {
		CLG_Print( PrintType::DeveloperWarning, fmt::format( "({}): geEventTarget for number(#{}) is (nullptr)!\n", __func__, number ) );
		return;
	}

	// With a valid entity we can notify it about its new received event.
	if (podEventTarget->currentState.eventID != 0) {
		//Com_DPrint("(%s): eventID != 0 for PODEntity(#%i)! podEntityTarget=(%s), geEntityTarget=(%s)\n", __func__, number, (podEventTarget ? podEventTarget->clientEntityNumber : -1), (geEventTarget ? geEventTarget->GetNumber() : -1));
	} else {
		//Com_DPrint("(%s): PODEntity(#%i): eventID(#%i) origin(%s), oldOrigin(%s)\n", __func__, number, podEventTarget->currentState.eventID, Vec3ToString(podEventTarget->currentState.origin), Vec3ToString(podEventTarget->currentState.oldOrigin));
	}

	PODEntity* clientEntity = &cs->entities[ number ];
	geEventTarget->OnEventID( clientEntity->currentState.eventID );
}

/**
*   @brief  Executed whenever a local client entity event is set.
**/
void ClientGameEntities::LocalEntityEvent( int32_t number ) {

}



/**
*
*
*	Refresh & View entities.
*
*
**/
/**
*   @brief  Prepares all parsed server entities, as well as local entities for rendering
*			of the current frame.
**/
void ClientGameEntities::PrepareRefreshEntities() {
	// Get Gameworld, we're about to iterate.
	ClientGameWorld *gameWorld = GetGameWorld();

    // Iterate from 0 till the amount of entities present in the current frame.
    for ( int32_t frameEntityStateNumber = 0; frameEntityStateNumber < cl->frame.numEntities; frameEntityStateNumber++ ) {
        // Get actual entity state index this packet frame entity state belogns to.
        const int32_t entityStateIndex = ( cl->frame.firstEntity + frameEntityStateNumber ) & PARSE_ENTITIES_MASK;
        
		// Get frame entity current state.
        EntityState *currentEntityState = &cl->entityStates[ entityStateIndex ];
		// Get POD Entity matching to the entity number.
        PODEntity *podEntity = &cs->entities[ currentEntityState->number ];
		// Get previous entity state 
		EntityState *previousEntityState = &podEntity->previousState;

		// At last, get its game entity.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex( podEntity->clientEntityNumber );

		// Setup the render entity ID for the renderer.
        const int32_t refreshEntityID = podEntity->clientEntityNumber;

		if (!gameEntity) {
			// TODO: Yeah, warn, maybe not..? Shouldn't really happen massively.
			continue;
		}

		// Prepare this game entity's refresh entity.
		gameEntity->PrepareRefreshEntity(refreshEntityID, currentEntityState, previousEntityState, cl->lerpFraction);
    }

	// Iterate from 0 till the amount of entities present in the current frame.
    for (int32_t localEntityNumber = LOCAL_ENTITIES_START_INDEX; localEntityNumber < MAX_CLIENT_POD_ENTITIES; localEntityNumber++) {
        PODEntity *clientEntity = gameWorld->GetPODEntityByIndex( localEntityNumber );
		// Get the current state of the given entity index.
		EntityState *entityState = &clientEntity->currentState;
		// Get the previous state of the given entity index.
		EntityState *previousEntityState = &clientEntity->previousState;

		// Get the game entity belonging to this entity.
        GameEntity *gameEntity = gameWorld->GetGameEntityByIndex( clientEntity->clientEntityNumber );
		
		// Setup the render entity ID for the renderer.
        const int32_t refreshEntityID = clientEntity->clientEntityNumber;
		
		if ( !gameEntity ) {
			// TODO: Yeah, warn, maybe not..? Shouldn't really happen massively.
			continue;
		}

		// Go on.
		gameEntity->PrepareRefreshEntity( refreshEntityID, entityState, previousEntityState, cl->lerpFraction );
    }
}

/**
*	@brief	Adds all 'view' entities to screen, the place to hook in entities that require a depth hack.
**/
void ClientGameEntities::AddViewEntities() {
    // Hidden in bsp menu mode.
    if (info_in_bspmenu->integer) {
        return;
    }

    // Acquire access to View Camera, and tell it to calculate and add our weapon viewmodel.
    ViewCamera *viewCamera = clge->view->GetViewCamera();
	viewCamera->AddWeaponViewModel();	
}



/**
*   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
**/
void ClientGameEntities::RunPacketEntitiesDeltaFrame() {
	// Get GameWorld.
	ClientGameWorld *gameWorld = GetGameWorld();

	// This needs to be set of course.
	if (!gameWorld) {
		return;
	}

	// Unlike for the server game, the level's framenumber, time and timeStamp
	GameTime svTime = GameTime(cl->serverTime);
    GameTime clTime = GameTime(cl->time);

	if (clTime > svTime) {
		level.time = svTime;
	} else {
		level.time = clTime;
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
	if ( !gameWorld ) {
		return;
	}

	// Iterate through our local client side entities.
    for ( int32_t localEntityNumber = MAX_WIRED_POD_ENTITIES; localEntityNumber < MAX_CLIENT_POD_ENTITIES; localEntityNumber++ ) {
		const int32_t entityIndex = localEntityNumber;
		PODEntity *podEntity = gameWorld->GetPODEntityByIndex( entityIndex );
		GameEntity *gameEntity = ClientGameWorld::ValidateEntity( podEntity );

		// If invalid for whichever reason, warn and continue to next iteration.
        if ( !podEntity || !gameEntity || !podEntity->inUse ) {
            continue;
        }

        // Admer: entity was marked for removal at the previous tick
        if ( podEntity && gameEntity && ( gameEntity->GetClientFlags() & EntityServerFlags::Remove ) ) {
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
			GameEntity *geGroundEntity = ClientGameWorld::ValidateEntity(gameEntity->GetGroundEntityHandle() );
			if ( geGroundEntity && ( geGroundEntity->GetLinkCount() != gameEntity->GetGroundEntityLinkCount() ) ) {
				// Reset ground entity.
				//gameEntity->SetGroundEntity( SGEntityHandle() );

				// Ensure we only check for it in case it is required (ie, certain movetypes do not want this...)
				//if (!(gameEntity->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) && (gameEntity->GetServerFlags() & EntityServerFlags::Monster)) {
					// Check for a new ground entity that resides below this entity.
					SG_CheckGround( gameEntity ); //SVG_StepMove_CheckGround(gameEntity);
				//}
			}
		}

		// Run it for a frame.
		SGEntityHandle geHandle = gameEntity;
		SG_RunEntity( geHandle );
    }
}
/**
*   @brief  Called for each prediction frame, so all entities can try and predict like the player does.
**/
void ClientGameEntities::RunPackEntitiesPredictionFrame() {

}



/**
*
*
*	Other.
*	
*
**/
/**
*	@return	The GameEntity's hashed classname value, 0 if it has no GameEntity.
**/
uint32_t ClientGameEntities::GetHashedGameEntityClassname( PODEntity* podEntity ) {
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
*	@brief	Returns a pointer to the actual client game POD Entities array residing in the ClientGame's world.
**/
PODEntity *ClientGameEntities::GetClientPODEntities() {
	// Acquire gameworld.
	ClientGameWorld *gameWorld = GetGameWorld();

	if (gameWorld) {
		return gameWorld->GetPODEntities();
	} else {
		return nullptr;
	}
}

/**
*   @brief  Gives the opportunity to adjust render effects where desired.
**/
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