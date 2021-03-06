/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once

// Shared Game.
#include "../../SharedGame.h"

#ifdef SHAREDGAME_SERVERGAME 
	#include "../../../Server/ServerGameLocals.h"
	#include "../../../Server/World/ServerGameWorld.h"
#endif
#ifdef SHAREDGAME_CLIENTGAME
	#include "../../../Client/ClientGameLocals.h"
	#include "../../../Client/World/ClientGameWorld.h"
#endif

// Physics.
#include "../Physics.h"
#include "../RootMotionMove.h"

// TODO: This needs some fixing hehe... ugly method but hey.
#ifdef SHAREDGAME_SERVERGAME
extern cvar_t *sv_maxvelocity;
extern cvar_t *sv_gravity;
extern void CheckSVCvars();
#endif

#ifdef SHAREDGAME_CLIENTGAME
extern cvar_t *GetSVMaxVelocity();
extern cvar_t *GetSVGravity();
#endif
//========================================================================

/**
*	@brief	Contains data of entities that are pushed by MoveType::Push objects. (BrushModels usually.)
**/
struct PushedEntityState {
	int32_t entityNumber = -1;
	//int32_t entityNumber = -1;
	//GameEntity *entityHandle;
	vec3_t origin = vec3_zero();
	vec3_t angles = vec3_zero();
	float deltaYaw = 0.f;
	// This is used as velocity wtf?
	vec3_t playerMoveOrigin = vec3_zero();
};

//! Reserved size std::vector containing all the pushed entity states.
//static std::vector<PushedEntityState> pushedEntities(MAX_POD_ENTITIES);
static PushedEntityState pushedEntities[MAX_POD_ENTITIES];

//! Pointer to the LAST pushed entity state.
static PushedEntityState *lastPushedEntityState = nullptr;
static int32_t lastPushedStateNumber = 0;

//! Pointer to a game entity that's pointing to our pushed obstacle.
static GameEntity *pushObstacle = nullptr;

/**
*	@brief	Pushes a new Pushed Entity State for the gePusher Game Entity.
**/
const PushedEntityState SG_PushEntityState(GameEntity* gePusher) {
	const bool isClientEntity = ( gePusher->GetClient() != nullptr ? true : false );
	const float deltaYaw = ( isClientEntity ? gePusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] : gePusher->GetAngles()[vec3_t::Yaw] );
	const vec3_t playerMoveOrigin = ( isClientEntity ? gePusher->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;

	// Create state.
	PushedEntityState returnedState = *lastPushedEntityState;
	*lastPushedEntityState = {
		.entityNumber = gePusher->GetNumber(),
		.origin = gePusher->GetOrigin(),
		.angles = gePusher->GetAngles(),
		.deltaYaw = deltaYaw,
		.playerMoveOrigin = playerMoveOrigin,
	};
	lastPushedEntityState++;

	return returnedState;
}

void SG_PopPushedEntityState(GameEntity* gePusher) {
	lastPushedEntityState--;
}
/**
*	@brief	Utilitzes the EntityHandle class to retreive a valid GameEntity for
*			the Pushed Entity State.
*
*	@return On success, a pointer to a valid GameEntity. On failure, a (nullptr).
**/
static GameEntity *SG_GetGameEntityFromPushedState( PushedEntityState *pushedEntityState) {
	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

	if (pushedEntities->entityNumber < 0) {
		return nullptr;
	}

	// Utilize the EntityHandle class.
	SGEntityHandle ehPushedStateEntity = gameWorld->GetPODEntityByIndex(pushedEntityState->entityNumber);

	// Validate the actual EntityHandle and get a hopefully non (nullptr) GameEntity. 
	GameEntity *validEntity = SGGameWorld::ValidateEntity(ehPushedStateEntity);

	// Return our pointer.
	return validEntity;
}

/**
*	@brief	Pushes the entity. Does not change the entities velocity at all
**/
SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset ) {
	SGTraceResult trace;
    int32_t     mask = 0;

	GameEntity *ent = gePushEntity;

    // Calculate start for push.
    vec3_t start = ent->GetOrigin();

    // Calculate end for push.
    vec3_t end = start + pushOffset;

retry:
    if (ent->GetClipMask())
        mask = ent->GetClipMask();
    else
        mask = BrushContentsMask::Solid;

    trace = SG_Trace(start, ent->GetMins(), ent->GetMaxs(), end, ent, mask);

	//if (ent->GetMoveType() == MoveType::Push || !trace.startSolid) {
	if (!trace.startSolid) {//if (!trace.startSolid) {
		ent->SetOrigin(trace.endPosition);
	}
	//}
    //ent->SetOrigin(trace.endPosition);
    ent->LinkEntity();

    if ( trace.fraction != 1.0f ) {
		// Dispatch impact callbacks.
        SG_Impact(ent, trace);

        // if the pushed entity went away and the pusher is still there
        if ( (!trace.gameEntity || !trace.gameEntity->IsInUse()) && ent->GetMoveType() == MoveType::Push && ent->IsInUse()) {
            // move the pusher back and try again
            ent->SetOrigin( start );
            ent->LinkEntity();
            goto retry;
        }
    }

    if ( ent->IsInUse() ) {
        SG_TouchTriggers( ent );
	}

    return trace;
}

/**
*	@brief	Objects need to be moved back on a failed push, otherwise 
*			riders would continue to slide.
**/
static bool SG_Push( SGEntityHandle &entityHandle, const vec3_t &move, const vec3_t &angularMove ) {
    GameEntity* geCheck = nullptr;
    GameEntity* geBlock = nullptr;
    PushedEntityState *p = nullptr;
    vec3_t org, org2;
	vec3_t move2;
	vec3_t forward, right, up;

    // Assign handle to base entity.
    GameEntity* gePusher = SGGameWorld::ValidateEntity(entityHandle);

    // Ensure it is a valid entity.
    if (!gePusher) {
	    SG_Physics_PrintWarning( std::string(__func__) + "called with an invalid entity handle!" );
	    return false;
    }

    // Find the bounding box.
    vec3_t mins = gePusher->GetAbsoluteMin() + move;
    vec3_t maxs = gePusher->GetAbsoluteMax() + move;

    // We need this for pushing things later
    org = vec3_negate(angularMove);
    AngleVectors(org, &forward, &right, &up);

	/**	
	*	Store the needed pushed entity information in a pushed entity state.
	**/
	SG_PushEntityState(gePusher);

	/**	
	*	Move the Pusher to its wished final position.
	**/
    gePusher->SetOrigin(gePusher->GetOrigin() + move);
    gePusher->SetAngles(gePusher->GetAngles() + angularMove);
    gePusher->LinkEntity();

	/**	
	*	See if the position has been taken by other entities, and if so, try and push each other.
	**/
	// Get a range of all pushable entities in our world. (A valid GameEntity and Inuse.)
	SGGameWorld *gameWorld = GetGameWorld();
	auto gePushables = gameWorld->GetGameEntityRange(0, MAX_POD_ENTITIES) | cef::IsValidPointer | cef::InUse;

	// Iterate over the pushable entities.
	for ( auto geCheck : gePushables ) {
        // Fetch its properties to work with.
        const qboolean isInUse = geCheck->IsInUse();
        const int32_t moveType = geCheck->GetMoveType();
        const vec3_t absMin = geCheck->GetAbsoluteMin();
        const vec3_t absMax = geCheck->GetAbsoluteMax();

		// Skip moveTypes that aren't pushed around at all.
        if ( moveType == MoveType::Push   || moveType == MoveType::Stop   ||
			 moveType == MoveType::None   || moveType == MoveType::NoClip ||
			 moveType == MoveType::Spectator ) {
            continue;
		}

		// Entity has to be linked in.
#if SHAREDGAME_CLIENTGAME
		if ( !geCheck->GetLinkCount() ) {
		//if ( !geCheck->GetPODEntity()->area.prev ) {
#endif
#if SHAREDGAME_SERVERGAME
        if ( !geCheck->GetPODEntity()->area.prev ) {
#endif
            continue;       // not linked in anywhere
#if SHAREDGAME_CLIENTGAME
		}
#endif
#if SHAREDGAME_SERVERGAME
		}
#endif
        // if the entity is standing on the pusher, it will definitely be moved
		GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity(geCheck->GetGroundEntityHandle());

        if ( geCheckGroundEntity != gePusher ) {
            // see if the ent needs to be tested
            if ( absMin[0] >= maxs[0]	||
                absMin[1] >= maxs[1]	||
                absMin[2] >= maxs[2]	||
                absMax[0] <= mins[0]	||
                absMax[1] <= mins[1]	||
                absMax[2] <= mins[2] ) {
                continue;
			}

            // see if the ent's bbox is inside the pusher's final position
            if ( !SG_TestEntityPosition( geCheck ) ) {
                continue;
			}
            
        }

        if ( (gePusher->GetMoveType() == MoveType::Push) || (geCheckGroundEntity == gePusher) ) {
			/**
			*	Store the needed pushed entity information in a pushed entity state.	
			**/
			SG_PushEntityState(geCheck);

			/**
			*	Try moving the contacted entity.
			**/
            geCheck->SetOrigin(geCheck->GetOrigin() + move);
#if USE_SMOOTH_DELTA_ANGLES
            if (geCheck->GetClient()) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
                geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
			} else {
				vec3_t angles = geCheck->GetAngles();
				angles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
				geCheck->SetAngles(angles);
			}
#endif

			/**
			*	Figure movement due to the pusher's Angular Move.
			**/
            org = geCheck->GetOrigin() - gePusher->GetOrigin(); //VectorSubtract(geCheck->state.origin, pusher->state.origin, org);
            org2[0] = vec3_dot(org, forward);
            org2[1] = -vec3_dot(org, right);
            org2[2] = vec3_dot(org, up);
            move2 = org2 - org;
            geCheck->SetOrigin(geCheck->GetOrigin() + move2);//VectorAdd(geCheck->state.origin, move2, geCheck->state.origin);

            // May have pushed them off an edge
            if ( geCheckGroundEntity != gePusher ) {
                geCheck->SetGroundEntity( SGEntityHandle() );
			}

			// Test whether entity is inside of another, if not, push was okay so link it and move
			// on to the next entity that needs pushing.
            geBlock = SG_TestEntityPosition( geCheck );
            if ( !geBlock ) {
                // pushed ok
                geCheck->LinkEntity();
                // impact?
                continue;
            }

            // Of it is ok to leave in the old position, do it.
            // This is only relevent for riding entities, not pushed
            // FIXME: this doesn't acount for rotation
            geCheck->SetOrigin( geCheck->GetOrigin() - move );//geCheck->state.origin -= move;
			geCheck->SetAngles( geCheck->GetAngles() - move2 );
            geBlock = SG_TestEntityPosition( geCheck );

            if ( !geBlock ) {
				SG_PopPushedEntityState( geCheck );

                continue;
            }
        }

        // Save off the obstacle so we can call the block function later on.
        pushObstacle = geCheck;

        // Move back any entities we already moved. We'll go backwards, so if the same entity was pushed
        // twice, it goes back to the original position.
		for (p = lastPushedEntityState - 1 ; p >= pushedEntities; p--) {
	        // Fetch pusher's base entity.
            GameEntity *pusherEntity = SG_GetGameEntityFromPushedState(p);//GameEntity* pusherEntity = p->entityHandle;

            // Ensure we are dealing with a valid pusher entity.
            if (!pusherEntity) {
    		    SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
                continue;
            }

            pusherEntity->SetOrigin(p->origin);
            pusherEntity->SetAngles(p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (pusherEntity->GetClient()) {
                pusherEntity->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = p->deltaYaw;
				pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
			} else {
				vec3_t newAngles = pusherEntity->GetAngles();
				newAngles[vec3_t::Yaw] = p->deltaYaw;
				pusherEntity->SetAngles(newAngles);
			}
#endif
			// Link Entity back in.
            pusherEntity->LinkEntity();
        }

        return false;
    }

	// Call TouchTriggers on all our moved entities.
    for (p = lastPushedEntityState - 1; p >= pushedEntities; p--) {
        // Fetch pusher's base entity.
        GameEntity* pusherEntity = SG_GetGameEntityFromPushedState(p);

        // Ensure we are dealing with a valid pusher entity.
	    if (!pusherEntity) {
		    SG_Physics_PrintWarning(std::string(__func__) + "got an invalid entity handle!");
            continue;
	    }

		// Enjoy,
	    SG_TouchTriggers(pusherEntity);
    }

    return true;

//	// Get GameEntity from handle.
//    if (!entityHandle || !(*entityHandle) || !entityHandle.Get() || !entityHandle.Get()->inUse) {
//        //SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
//		return false;
//    }
//
//	// Get gePusher.
//	GameEntity *gePusher = (*entityHandle);
//
//	if (!gePusher || !lastPushedEntityState ) {
//		return false;
//	}
//
//    // Calculate the exact the bounding box
//    const vec3_t mins = gePusher->GetAbsoluteMin() + move;
//    const vec3_t maxs = gePusher->GetAbsoluteMax() + move;
//
//    // We need this for pushing things later
//    vec3_t org = vec3_negate(angularMove);
//	vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();
//    AngleVectors(org, &forward, &right, &up);
//
//	// Save the pusher's original position.
//	lastPushedEntityState->entityHandle = gePusher;
//	lastPushedEntityState->origin = gePusher->GetOrigin();
//	lastPushedEntityState->angles = gePusher->GetAngles();
//	if ( gePusher->GetClient() ) {
//		// Store velocity. I know, it's named origin wtf?.
//		lastPushedEntityState->playerMoveOrigin = gePusher->GetClient()->playerState.pmove.velocity;
//		// Store delta yaw angle.
//		lastPushedEntityState->deltaYaw = gePusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
//	}
//	lastPushedEntityState++;
//
//	// move the pusher to its final position
//    gePusher->SetOrigin(gePusher->GetOrigin() + move);
//    gePusher->SetAngles(gePusher->GetAngles() + angularMove);
//    gePusher->LinkEntity();
//
//	// See if any solid entities are inside the final position.
//	SGGameWorld *gameWorld = GetGameWorld();
//	for ( int32_t i = 1; i < 4096; i++ ) {
//		// Get the entity to check position for.
//		GameEntity *geCheck = gameWorld->GetGameEntityByIndex(i);
//
//		// Skip Checks.
//		if (!geCheck || !geCheck->IsInUse()) {
//			// Skip if it's not in use.
//			continue;
//		}
//
//		// Get some data.
//		const bool isInUse = geCheck->IsInUse();
//		const int32_t moveType = geCheck->GetMoveType();
//		const vec3_t absMin = geCheck->GetAbsoluteMin();
//		const vec3_t absMax = geCheck->GetAbsoluteMax();
//
//		if (moveType == MoveType::Push ||
//			moveType == MoveType::Stop ||
//			moveType == MoveType::None || 
//			moveType == MoveType::NoClip ||
//			moveType == MoveType::Spectator) {
//			// Skip if it's not having an appropriate MoveType.
//			continue;	
//		}	
//
//#ifdef SHAREDGAME_SERVERGAME
//		if (!geCheck->GetPODEntity()->area.prev) {
//#endif
//#ifdef SHAREDGAME_CLIENTGAME
////		if (!geCheck->GetLinkCount()) {
//			if (1 == 1) {
//#endif
//			// If it's not linked in...
//			continue;
//		}
//
//
//		// See whether the entity's ground entity is the actual pusher entity.
//		if ( (geCheck->GetGroundEntityHandle()) != gePusher ) {
//			// If it's not, check whether at least the bounding box intersects.
//			//if ( (absMin[0] >= maxs[0]) &&
//			//	(absMin[1] >= maxs[1]) &&
//			//	(absMin[2] >= maxs[2]) &&
//			//	(absMax[0] >= mins[0]) &&
//			//	(absMax[1] >= mins[1]) &&
//			//	(absMax[2] >= mins[2]) ) {            // See if the ent needs to be tested
//            if ((!absMin[0] >= maxs[0]
//                || absMin[1] >= maxs[1]
//                || absMin[2] >= maxs[2]
//                || absMax[0] <= mins[0]
//                || absMax[1] <= mins[1]
//                || absMax[2] <= mins[2])) {
//				continue;
//			}
//
//			// See if the ent's bbox is inside the pusher's final position.
//			if( !SG_TestEntityPosition( geCheck ) ) {
//				continue;
//			}
//		}
//		
//		if( (gePusher->GetMoveType() == MoveType::Push) || (geCheck->GetGroundEntityHandle() == gePusher) ) {
//			// Move this entity.
//			//pushed_p->ent = check;
//			//VectorCopy( geCheck->s.origin, pushed_p->origin );
//			//VectorCopy( geCheck->s.angles, pushed_p->angles );
//			//pushed_p++;
//			lastPushedEntityState->entityHandle = geCheck;
//			lastPushedEntityState->origin = geCheck->GetOrigin();
//			lastPushedEntityState->angles = geCheck->GetAngles();
//#if USE_SMOOTH_DELTA_ANGLES
//			if ( geCheck->GetClient() ) {
//				lastPushedEntityState->deltaYaw = geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
//			}
//#endif
//			lastPushedEntityState++;
//
//
//			// Try moving the contacted entity
//			geCheck->SetOrigin(geCheck->GetOrigin() + move);
//#if USE_SMOOTH_DELTA_ANGLES
//            if ( geCheck->GetClient() ) {
//                // FIXME: doesn't rotate monsters?
//                // FIXME: skuller: needs client side interpolation
//                //geCheck->GetClient()->playerState.pmove.origin += move;
//				geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
//            }/* else {
//				geCheck->SetAngles(geCheck->GetAngles() + angularMove);
//			}*/
//#endif
//			// Figure movement due to the pusher's amove.
//			const vec3_t org = geCheck->GetOrigin() - gePusher->GetOrigin(); //VectorSubtract( geCheck->s.origin, pusher->s.origin, org );
//			const vec3_t org2 = {
//				vec3_dot( org, forward ),
//				vec3_dot( org, right ),
//				vec3_dot( org, up ),
//			};
//			const vec3_t move2 = org2 - org;
//			geCheck->SetOrigin( geCheck->GetOrigin() + move2 );
//			//Matrix3_TransformVector( axis, org, org2 );
//			//VectorSubtract( org2, org, move2 );
//			//VectorAdd( geCheck->s.origin, move2, geCheck->s.origin );
//
//			//if( geCheck->GetMoveType() != MoveType::BounceGrenade ) {
//				// may have pushed them off an edge
//				if( geCheck->GetGroundEntityHandle() != gePusher ) {
//					geCheck->SetGroundEntity( SGEntityHandle() );
//				}
//			//}
//
//			GameEntity *geBlocked = SG_TestEntityPosition( geCheck );
//			if( !geBlocked ) {
//				// pushed ok
//				//GClip_LinkEntity( check );
//				geCheck->LinkEntity();
//
//				// impact?
//				continue;
//			} //else {
//				// try to fix block
//				// if it is ok to leave in the old position, do it
//				// this is only relevant for riding entities, not pushed
//				geCheck->SetOrigin( geCheck->GetOrigin() - move );
//				//geCheck->SetOrigin( geCheck->GetOrigin() - move2 );
//				//VectorSubtract( geCheck->s.origin, move, geCheck->s.origin );
//				//VectorSubtract( geCheck->s.origin, move2, geCheck->s.origin );
//				geBlocked = SG_TestEntityPosition( geCheck );
//				if( !geBlocked ) {
//					lastPushedEntityState--;
//					continue;
//				}
//		//	}
//		}
//
//		// Save obstacle so we can call its blocked callback.
//		pushObstacle = geCheck;
//
//		// move back any entities we already moved
//		// go backwards, so if the same entity was pushed
//		// twice, it goes back to the original position
//		for( PushedEntityState *pushedState = lastPushedEntityState - 1; pushedState >= lastPushedEntityState; pushedState-- ) {
//	        // Fetch pusher's game entity.
//            GameEntity* pusherEntity = pushedState->entityHandle;
//
//            // Ensure we are dealing with a valid pusher entity.
//            if ( !pusherEntity ) {
//    		    SG_PhysicsEntityWPrint(__func__, "[move back loop]", "got an invalid entity handle!\n");
//                continue;
//            }
//
//            pusherEntity->SetOrigin(pushedState->origin);
//            pusherEntity->SetAngles(pushedState->angles);
//#if USE_SMOOTH_DELTA_ANGLES
//            if ( pusherEntity->GetClient() ) {
//				//pusherEntity->GetClient()->playerState.pmove.velocity = pushedState->playerMoveOrigin;
//
//				//pusherEntity->SetOrigin(pushedState->playerMoveOrigin);//pusherEntity->GetClient()->playerState.pmove.origin);
//                pusherEntity->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = pushedState->deltaYaw;
//            }
//#endif
//            pusherEntity->LinkEntity();
//		}
//		return false;
//	}
//
//	//FIXME: is there a better way to handle this?
//	// see if anything we moved has touched a trigger
//	for( PushedEntityState *pushedState = lastPushedEntityState - 1; pushedState >= lastPushedEntityState; pushedState-- ) {
//        // Fetch pusher's base entity.
//        GameEntity* pusherEntity = pushedState->entityHandle;
//
//        // Ensure we are dealing with a valid pusher entity.
//	    if ( !pusherEntity ) {
//		    SG_PhysicsEntityWPrint(__func__, "[was moved loop] ", "got an invalid entity handle!\n");
//            continue;
//	    }
//
//	    SG_TouchTriggers(pushedState->entityHandle);
//	}
//
//	return true;
}

/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle ) {
	    vec3_t      move, amove;
    GameEntity     *part = nullptr, *mv = nullptr;

    // Assign handle to base entity.
    GameEntity* ent = SGGameWorld::ValidateEntity(gePusherHandle);

    // Ensure it is a valid entity.
    if (!ent) {
    	SG_Physics_PrintWarning(std::string(__func__) + "got an invalid entity handle!");
        return;
    }

    // if not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave) {
        return;
	}

	// First clear out the vector.
retry:
	lastPushedEntityState = pushedEntities;

    for (part = ent ; part ; part = part->GetTeamChainEntity()) {
        // Fetch pusher part, its Velocity.
        vec3_t partVelocity = part->GetVelocity();

        // Fetch pusher part, its Angular Velocity.
        vec3_t partAngularVelocity = part->GetAngularVelocity();

        if (partVelocity.x || partVelocity.y || partVelocity.z ||
            partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z) 
        {
            // object is moving
            move = vec3_scale(part->GetVelocity(), FRAMETIME.count());
            amove = vec3_scale(part->GetAngularVelocity(), FRAMETIME.count());

            SGEntityHandle partHandle(part);
            if (!SG_Push(partHandle, move, amove))
                break;  // move was Blocked
        }
    }


	if (lastPushedEntityState > &pushedEntities[MAX_WIRED_POD_ENTITIES]) {
        Com_Error(ErrorType::Drop, "pushed_p > &pushed[MAX_WIRED_POD_ENTITIES], memory corrupted");
	}

    if (part) {
        // the move failed, bump all nextThinkTime times and back out moves
        for (mv = ent ; mv ; mv = mv->GetTeamChainEntity()) {
            if (mv->GetNextThinkTime() > GameTime::zero()) {
                mv->SetNextThinkTime(mv->GetNextThinkTime() + FRAMERATE_MS);//);//FRAMETIME);// 1_hz);
            }
        }

        // if the pusher has a "Blocked" function, call it
        // otherwise, just stay in place until the obstacle is gone
        if (pushObstacle) {
            part->DispatchBlockedCallback(pushObstacle);
        }

//#if 0
        // if the pushed entity went away and the pusher is still there
        if ((pushObstacle && !pushObstacle->IsInUse()) && (part && part->IsInUse())) {
            goto retry;
		}
//#endif
    } else {
        // the move succeeded, so call all Think functions
        for (part = ent ; part ; part = part->GetTeamChainEntity()) {
            SG_RunThink(part);
        }
    }
	//	if (!gePusherHandle.Get() || !*gePusherHandle) {
//		return;
//	}
//	// Get GameEntity from Handle.
//	GameEntity *gePusher = *gePusherHandle;
//
//    // Ensure it is a valid entity.
//    if ( !gePusher ) {
//    	SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
//        return;
//    }
//
//    // if not a team captain, so movement will be handled elsewhere
//	if ( gePusher->GetFlags() & EntityFlags::TeamSlave ) {
//        return;
//	}
//
//	// Make sure all team followers can move before commiting
//	// any moves or calling any think functions
//	// If the move is blocked, all moved objects will be backed out
//	lastPushedEntityState = pushedEntities;
//
//	GameEntity *gePushPart = nullptr;
//    GameEntity *gePart = nullptr, *geMove = nullptr;
//retry:
//	lastPushedEntityState = pushedEntities;
//
//	gePushPart = nullptr;
//    gePart = nullptr;
//	geMove = nullptr;
//
//	for( gePushPart = gePusher; gePushPart; gePushPart = gePushPart->GetTeamChainEntity() ) {
//		// Get pusher part velocity.
//		const vec3_t partVelocity = gePushPart->GetVelocity( );
//
//		// Get pusher part Angular Velocity.
//		const vec3_t partAngularVelocity = gePushPart->GetAngularVelocity();
//
//		if (partVelocity.x || partVelocity.y || partVelocity.z ||
//			partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z) 
//		{
//			// object is moving
//		//if( part->s.linearMovement ) {
//		//	GS_LinearMovement( &part->s, game.serverTime, move );
//		//	VectorSubtract( move, part->s.origin, move );
//		//	VectorScale( part->avelocity, FRAMETIME, amove );
//		//} else {
//			const vec3_t velocity = vec3_scale(partVelocity, FRAMETIME.count()); //VectorScale( part->velocity, FRAMETIME, move );
//			const vec3_t angularVelocity = vec3_scale(partAngularVelocity, FRAMETIME.count()); //VectorScale( part->avelocity, FRAMETIME, amove );
//
//			SGEntityHandle partHandle(gePushPart);
//			if ( !SG_Push(partHandle, velocity, angularVelocity) ) {
//				break;  // Move was Blocked.
//			}
//	   //}
//		}
//	}
//
//	if( lastPushedEntityState > &pushedEntities[4096] ) {
//		//G_Error( "pushed_p > &pushed[MAX_EDICTS], memory corrupted" );
//	}
//
//	if( gePushPart ) {
//		// the move failed, bump all nextthink times and back out moves
//		for( GameEntity *mover = gePusher; mover; mover = mover->GetTeamChainEntity() ) {
//			auto nextThinkTime = mover->GetNextThinkTime();
//			if( nextThinkTime > GameTime::zero()) {
//				mover->SetNextThinkTime(nextThinkTime + FRAMERATE_MS);
////				mover->nextThink += game.frametime;
//			}
//		}
//
//		// if the pusher has a "blocked" function, call it
//		// otherwise, just stay in place until the obstacle is gone
//        if (gePushPart && pushObstacle) {
//            gePushPart->DispatchBlockedCallback(pushObstacle);
//        }
////#if 0
//
//		// if the obstacle went away and the pusher is still there
//		//if( !obstacle->r.inuse && part->r.inuse ) {
//		//	goto retry;
//		//}
//        // if the pushed entity went away and the pusher is still there
//        if ( (pushObstacle && !pushObstacle->IsInUse()) && (gePushPart && gePushPart->IsInUse()) ) {
//            goto retry;
//		}
////#endif
//	} else {
//        // the move succeeded, so call all Think functions
//        for (gePart = gePusher ; gePart ; gePart = gePart->GetTeamChainEntity()) {
//            SG_RunThink(gePart);
//        }
//    }
}