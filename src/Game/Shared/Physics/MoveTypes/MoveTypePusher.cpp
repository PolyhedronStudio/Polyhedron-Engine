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
#include "../SlideMove.h"

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
*	@brief	Pushes the entity. Does not change the entities velocity at all
**/
SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset ) {
	SGTraceResult trace;
    int     mask;

	GameEntity *ent = gePushEntity;
	vec3_t push = pushOffset;
    // Calculate start for push.
    vec3_t start = ent->GetOrigin();

    // Calculate end for push.
    vec3_t end = start + push;

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

    if ( trace.fraction != 1.0 ) {
        SG_Impact(ent, trace);

        // if the pushed entity went away and the pusher is still there
        if ( !trace.gameEntity->IsInUse() && ent->GetMoveType() == MoveType::Push && ent->IsInUse() ) {
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

	//	// Entity clipping mask.
//	const int32_t entityClipMask = gePushEntity->GetClipMask();
//	// Entity movetype.
//	// Actual trace results.
//	SGTraceResult traceResult = {};
//	// Set to the clipping mask of the entity if it has one set, 'Solid' otherwise.
//	int32_t traceClipMask = BrushContentsMask::Solid;
//	// Start trace point.
//	const vec3_t traceStart = gePushEntity->GetOrigin();
//	// End trace point.
//	const vec3_t traceEnd = traceStart + pushOffset;
//	
//retry:
//	// Use the entity clipmask if its set, otherwise use 'Solid'.
//	if (entityClipMask) {
//		traceClipMask = entityClipMask;
//	} else {
//		traceClipMask = BrushContentsMask::Solid;
//	}
//
//	// Execute trace.
//	traceResult = SG_Trace(traceStart, gePushEntity->GetMins(), gePushEntity->GetMaxs(), traceEnd, gePushEntity, traceClipMask);
//	//G_Trace4D( &trace, start, ent->r.mins, ent->r.maxs, end, ent, clipMask, ent->timeDelta );
//	
//	//if( gePushEntity->GetMoveType() == MoveType::Push || !traceResult.startSolid) {
//	if (!traceResult.startSolid) {
//		gePushEntity->SetOrigin(traceResult.endPosition);
//		//VectorCopy( trace.endpos, ent->s.origin );
//	}
//
//	// Link entity.
//	gePushEntity->LinkEntity();
//
//	// Impact response.
//	if ( traceResult.fraction < 1.0 ) {
//		// Trigger impact touch callbacks.
//		SG_Impact( gePushEntity, traceResult );
//
//		// if the pushed entity went away and the pusher is still there
//		GameEntity *traceResultGameEntity = traceResult.gameEntity;
//		//if( !game.edicts[trace.ent].r.inuse && ent->movetype == MOVETYPE_PUSH && ent->r.inuse ) {
//		if (traceResultGameEntity && !traceResultGameEntity->IsInUse() 
//			&& gePushEntity->GetMoveType() == MoveType::Push && gePushEntity->IsInUse()
//		) {
//			// move the pusher back and try again
//			gePushEntity->SetOrigin(traceStart);
//			gePushEntity->LinkEntity();
//			//GClip_LinkEntity( ent );
//			goto retry;
//		}
//	}
//
//	//if( ent->r.inuse ) {
//	if (gePushEntity->IsInUse()) {
//		SG_TouchTriggers( gePushEntity );
//	}
//
//	return traceResult;
}

/**
*	@brief	Contains data of entities that are pushed by MoveType::Push objects. (BrushModels usually.)
**/
struct PushedGameEntityState {
	GameEntity *entityHandle;
	vec3_t origin = vec3_zero();
	vec3_t angles = vec3_zero();
	float deltaYaw = 0.f;
	// This is used as velocity wtf?
	vec3_t playerMoveOrigin = vec3_zero();
};

//! List of pushed entities to use.
//PushedEntityState pushedEntityStates[MAX]
static PushedGameEntityState pushedGameEntities[4096];
static PushedGameEntityState *lastPushedState = nullptr;
//pushed_t pushed[MAX_EDICTS], *pushed_p;
static GameEntity *pushObstacle = nullptr;


/**
*	@brief	Objects need to be moved back on a failed push, otherwise 
*			riders would continue to slide.
**/
static bool SG_Push( SGEntityHandle &entityHandle, const vec3_t &move, const vec3_t &angularMove ) {
    GameEntity* geCheck = nullptr;
    GameEntity* geBlock = nullptr;
    PushedGameEntityState *p = nullptr;
    vec3_t org, org2;
	vec3_t move2;
	vec3_t forward, right, up;

    // Assign handle to base entity.
    GameEntity* gePusher = *entityHandle;

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

    // Save the pusher's original origin, and angles.
    lastPushedState->entityHandle = gePusher;
    lastPushedState->origin = gePusher->GetOrigin(); // VectorCopy(pusher->state.origin, pushed_p->origin);
    lastPushedState->angles = gePusher->GetAngles();

#if USE_SMOOTH_DELTA_ANGLES
    if (gePusher->GetClient()) {
        lastPushedState->deltaYaw = gePusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
    } else {
		lastPushedState->deltaYaw = gePusher->GetAngles()[vec3_t::Yaw];
	}
#endif
    lastPushedState++;

	// Move the Pusher to its wished final position.
    gePusher->SetOrigin(gePusher->GetOrigin() + move);
    gePusher->SetAngles(gePusher->GetAngles() + angularMove);
    gePusher->LinkEntity();

	// See if any solid entities are inside the final position.
	SGGameWorld *gameWorld = GetGameWorld();
	for ( int32_t i = 1; i < 4096; i++ ) {
		// Get the entity to check position for.
		GameEntity *geCheck = gameWorld->GetGameEntityByIndex(i);

		// Only proceed if the game entity is valid, and in use.
		if (!geCheck || !geCheck->IsInUse()) {
			// Skip if it's not in use.
			continue;
		}

        // Fetch its properties to work with.
        const qboolean isInUse = geCheck->IsInUse();
        const int32_t moveType = geCheck->GetMoveType();
        const vec3_t absMin = geCheck->GetAbsoluteMin();
        const vec3_t absMax = geCheck->GetAbsoluteMax();

		// Skip moveTypes that aren't pushed around at all.
        if ( moveType == MoveType::Push		|| 
			moveType == MoveType::Stop		||
			moveType == MoveType::None		|| 
			moveType == MoveType::NoClip	|| 
			moveType == MoveType::Spectator 
		) {
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
        if ( geCheck->GetGroundEntityHandle() != gePusher ) {
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

        if ( (gePusher->GetMoveType() == MoveType::Push) || (geCheck->GetGroundEntityHandle() == gePusher) ) {
            // move this entity
            lastPushedState->entityHandle = geCheck;
            lastPushedState->origin = geCheck->GetOrigin();  //VectorCopy(geCheck->state.origin, pushed_p->origin);
            lastPushedState->angles = geCheck->GetAngles(); //VectorCopy(geCheck->state.angles, pushed_p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (geCheck->GetClient()) {
                lastPushedState->deltaYaw = geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
			}
			else {
				lastPushedState->deltaYaw = geCheck->GetAngles()[vec3_t::Yaw];
			}
#endif
            lastPushedState++;

            // Try moving the contacted entity.
            geCheck->SetOrigin(geCheck->GetOrigin() + move);
#if USE_SMOOTH_DELTA_ANGLES
            if (geCheck->GetClient()) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
                geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
			} else {
				vec3_t angles = geCheck->GetAngles();
				angles[vec3_t::Yaw] = angularMove[vec3_t::Yaw];
				geCheck->SetAngles(angles);
			}
#endif

            // figure movement due to the pusher's amove
            org = geCheck->GetOrigin() - gePusher->GetOrigin(); //VectorSubtract(geCheck->state.origin, pusher->state.origin, org);
            org2[0] = vec3_dot(org, forward);
            org2[1] = -vec3_dot(org, right);
            org2[2] = vec3_dot(org, up);
            move2 = org2 - org;
            geCheck->SetOrigin(geCheck->GetOrigin() + move2);//VectorAdd(geCheck->state.origin, move2, geCheck->state.origin);

            // may have pushed them off an edge
            if ( geCheck->GetGroundEntityHandle() != gePusher ) {
                geCheck->SetGroundEntity(nullptr);
			}

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
                lastPushedState--;
                continue;
            }
        }

        // Save off the obstacle so we can call the block function later on.
        pushObstacle = geCheck;

        // Move back any entities we already moved go backwards, so if the same entity was pushed
        // twice, it goes back to the original position.
        for (p = lastPushedState - 1 ; p >= pushedGameEntities; p--) {
	        // Fetch pusher's base entity.
            GameEntity* pusherEntity = p->entityHandle;

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
			}
			else {
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
    for (p = lastPushedState - 1; p >= pushedGameEntities; p--) {
        // Fetch pusher's base entity.
        GameEntity* pusherEntity = p->entityHandle;

        // Ensure we are dealing with a valid pusher entity.
	    if (!pusherEntity) {
		    SG_Physics_PrintWarning(std::string(__func__) + "got an invalid entity handle!");
            continue;
	    }

		// Enjoy,
	    SG_TouchTriggers(p->entityHandle);
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
//	if (!gePusher || !lastPushedState ) {
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
//	lastPushedState->entityHandle = gePusher;
//	lastPushedState->origin = gePusher->GetOrigin();
//	lastPushedState->angles = gePusher->GetAngles();
//	if ( gePusher->GetClient() ) {
//		// Store velocity. I know, it's named origin wtf?.
//		lastPushedState->playerMoveOrigin = gePusher->GetClient()->playerState.pmove.velocity;
//		// Store delta yaw angle.
//		lastPushedState->deltaYaw = gePusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
//	}
//	lastPushedState++;
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
//			lastPushedState->entityHandle = geCheck;
//			lastPushedState->origin = geCheck->GetOrigin();
//			lastPushedState->angles = geCheck->GetAngles();
//#if USE_SMOOTH_DELTA_ANGLES
//			if ( geCheck->GetClient() ) {
//				lastPushedState->deltaYaw = geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
//			}
//#endif
//			lastPushedState++;
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
//					geCheck->SetGroundEntity(nullptr);
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
//					lastPushedState--;
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
//		for( PushedGameEntityState *pushedState = lastPushedState - 1; pushedState >= lastPushedState; pushedState-- ) {
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
//	for( PushedGameEntityState *pushedState = lastPushedState - 1; pushedState >= lastPushedState; pushedState-- ) {
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
    GameEntity* ent = *gePusherHandle;

    // Ensure it is a valid entity.
    if (!ent) {
    	SG_Physics_PrintWarning(std::string(__func__) + "got an invalid entity handle!");
        return;
    }

    // if not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave)
        return;

    // make sure all team slaves can move before commiting
    // any moves or calling any Think functions
    // if the move is Blocked, all moved objects will be backed out
retry:
    lastPushedState = pushedGameEntities;
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
	if (lastPushedState > &pushedGameEntities[MAX_WIRED_POD_ENTITIES]) {
//        gi.Error("pushed_p > &pushed[MAX_WIRED_POD_ENTITIES], memory corrupted");
	}

    if (part) {
        // the move failed, bump all nextThinkTime times and back out moves
        for (mv = ent ; mv ; mv = mv->GetTeamChainEntity()) {
            if (mv->GetNextThinkTime() > GameTime::zero()) {
                mv->SetNextThinkTime(mv->GetNextThinkTime() + FRAMERATE_MS);// 1_hz);
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
//	lastPushedState = pushedGameEntities;
//
//	GameEntity *gePushPart = nullptr;
//    GameEntity *gePart = nullptr, *geMove = nullptr;
//retry:
//	lastPushedState = pushedGameEntities;
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
//	if( lastPushedState > &pushedGameEntities[4096] ) {
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