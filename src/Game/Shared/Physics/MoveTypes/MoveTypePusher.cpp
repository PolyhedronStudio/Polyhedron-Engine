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
	// Entity clipping mask.
	const int32_t entityClipMask = gePushEntity->GetClipMask();
	// Entity movetype.
	// Actual trace results.
	SGTraceResult traceResult = {};
	// Set to the clipping mask of the entity if it has one set, 'Solid' otherwise.
	int32_t traceClipMask = BrushContentsMask::Solid;
	// Start trace point.
	const vec3_t traceStart = gePushEntity->GetOrigin();
	// End trace point.
	const vec3_t traceEnd = traceStart + pushOffset;
	
retry:
	// Use the entity clipmask if its set, otherwise use 'Solid'.
	if (entityClipMask) {
		traceClipMask = entityClipMask;
	} else {
		traceClipMask = BrushContentsMask::Solid;
	}

	// Execute trace.
	traceResult = SG_Trace(traceStart, gePushEntity->GetMins(), gePushEntity->GetMaxs(), traceEnd, gePushEntity, traceClipMask);
	//G_Trace4D( &trace, start, ent->r.mins, ent->r.maxs, end, ent, clipMask, ent->timeDelta );
	
	if( gePushEntity->GetMoveType() == MoveType::Push || !traceResult.startSolid) {
		gePushEntity->SetOrigin(traceResult.endPosition);
		//VectorCopy( trace.endpos, ent->s.origin );
	}

	// Link entity.
	gePushEntity->LinkEntity();

	// Impact response.
	if ( traceResult.fraction < 1.0 ) {
		// Trigger impact touch callbacks.
		SG_Impact( gePushEntity, traceResult );

		// if the pushed entity went away and the pusher is still there
		GameEntity *traceResultGameEntity = traceResult.gameEntity;
		//if( !game.edicts[trace.ent].r.inuse && ent->movetype == MOVETYPE_PUSH && ent->r.inuse ) {
		if (traceResultGameEntity && !traceResultGameEntity->IsInUse() 
			&& gePushEntity->GetMoveType() == MoveType::Push && gePushEntity->IsInUse()
		) {
			// move the pusher back and try again
			gePushEntity->SetOrigin(traceStart);
			gePushEntity->LinkEntity();
			//GClip_LinkEntity( ent );
			goto retry;
		}
	}

	//if( ent->r.inuse ) {
	if (gePushEntity->IsInUse()) {
		SG_TouchTriggers( gePushEntity );
	}

	return traceResult;
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
	/*int i, e;
	edict_t *check, *block;
	vec3_t mins, maxs;
	pushed_t *p;
	mat3_t axis;
	vec3_t org, org2, move2;*/

	// Get GameEntity from handle.
    if (!entityHandle || !(*entityHandle) || !entityHandle.Get() || !entityHandle.Get()->inUse) {
        //SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
		return false;
    }

	// Get gePusher.
	GameEntity *gePusher = (*entityHandle);

	if (!gePusher || !lastPushedState ) {
		return false;
	}

    // Calculate the exact the bounding box
    const vec3_t mins = gePusher->GetAbsoluteMin() + move;
    const vec3_t maxs = gePusher->GetAbsoluteMax() + move;

    // We need this for pushing things later
    vec3_t org = vec3_negate(angularMove);
	vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();
    AngleVectors(org, &forward, &right, &up);

	// Save the pusher's original position.
	lastPushedState->entityHandle = gePusher;
	lastPushedState->origin = gePusher->GetOrigin();
	lastPushedState->angles = gePusher->GetAngles();
	if ( gePusher->GetClient() ) {
		// Store velocity. I know, it's named origin wtf?.
		lastPushedState->playerMoveOrigin = gePusher->GetClient()->playerState.pmove.velocity;
		// Store delta yaw angle.
		lastPushedState->deltaYaw = gePusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
	}
	lastPushedState++;

	// move the pusher to its final position
    gePusher->SetOrigin(gePusher->GetOrigin() + move);
    gePusher->SetAngles(gePusher->GetAngles() + angularMove);
    gePusher->LinkEntity();
	//VectorAdd( pusher->s.origin, move, pusher->s.origin );
	//VectorAdd( pusher->s.angles, amove, pusher->s.angles );
	//GClip_LinkEntity( pusher );

	// See if any solid entities are inside the final position.
	SGGameWorld *gameWorld = GetGameWorld();
	for ( int32_t i = 1; i < 4096; i++ ) {
		// Get the entity to check position for.
		GameEntity *geCheck = gameWorld->GetGameEntityByIndex(i);

		// Skip Checks.
		if (!geCheck || !geCheck->IsInUse()) {
			// Skip if it's not in use.
			continue;
		}

		// Get some data.
		const bool isInUse = geCheck->IsInUse();
		const int32_t moveType = geCheck->GetMoveType();
		const vec3_t absMin = geCheck->GetAbsoluteMin();
		const vec3_t absMax = geCheck->GetAbsoluteMax();

		if (moveType == MoveType::Push ||
			moveType == MoveType::Stop ||
			moveType == MoveType::None || 
			moveType == MoveType::NoClip ||
			moveType == MoveType::Spectator) {
			// Skip if it's not having an appropriate MoveType.
			continue;	
		}	
		if (!geCheck->GetLinkCount()) {
			// If it's not linked in...
			continue;
		}


		// See whether the entity's ground entity is the actual pusher entity.
		if ( *(geCheck->GetGroundEntityHandle()) != gePusher ) {
			// If it's not, check whether at least the bounding box intersects.
			if ( (absMin[0] >= maxs[0]) &&
				(absMin[1] >= maxs[1]) &&
				(absMax[2] >= mins[2]) &&
				(absMax[0] >= mins[0]) &&
				(absMax[1] >= mins[1]) &&
				(absMax[2] >= mins[2]) ) {            // See if the ent needs to be tested
   //         if (absMin[0] >= maxs[0]
   //             || absMin[1] >= maxs[1]
   //             || absMin[2] >= maxs[2]
   //             || absMax[0] <= mins[0]
   //             || absMax[1] <= mins[1]
   //             || absMax[2] <= mins[2]) {
				continue;
			}

			// See if the ent's bbox is inside the pusher's final position.
			if( !SG_TestEntityPosition( geCheck ) ) {
				continue;
			}
		}
		
		if( (gePusher->GetMoveType() == MoveType::Push) || (geCheck->GetGroundEntityHandle() == gePusher) ) {
			// Move this entity.
			//pushed_p->ent = check;
			//VectorCopy( check->s.origin, pushed_p->origin );
			//VectorCopy( check->s.angles, pushed_p->angles );
			//pushed_p++;
			lastPushedState->entityHandle = geCheck;
			lastPushedState->origin = geCheck->GetOrigin();
			lastPushedState->angles = geCheck->GetAngles();
			lastPushedState++;

			// Try moving the contacted entity
			geCheck->SetOrigin(geCheck->GetOrigin() + move);
#if USE_SMOOTH_DELTA_ANGLES
            if ( geCheck->GetClient() ) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
                geCheck->GetClient()->playerState.pmove.origin += move;
				geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
            }
#endif
			// Figure movement due to the pusher's amove.
			const vec3_t org = geCheck->GetOrigin() - gePusher->GetOrigin(); //VectorSubtract( check->s.origin, pusher->s.origin, org );
			const vec3_t org2 = {
				vec3_dot(org, forward),
				vec3_dot(org, right),
				vec3_dot(org, up),
			};
			const vec3_t move2 = org2 - org;
			geCheck->SetOrigin(geCheck->GetOrigin() + move2);
			//Matrix3_TransformVector( axis, org, org2 );
			//VectorSubtract( org2, org, move2 );
			//VectorAdd( check->s.origin, move2, check->s.origin );

			//if( geCheck->GetMoveType() != MoveType::BounceGrenade ) {
				// may have pushed them off an edge
				if( geCheck->GetGroundEntityHandle() != gePusher ) {
					geCheck->SetGroundEntity(nullptr);
				}
			//}

			GameEntity *geBlocked = SG_TestEntityPosition( geCheck );
			if( !geBlocked ) {
				// pushed ok
				//GClip_LinkEntity( check );
				geCheck->LinkEntity();

				// impact?
				continue;
			} else {
				// try to fix block
				// if it is ok to leave in the old position, do it
				// this is only relevant for riding entities, not pushed
				geCheck->SetOrigin(geCheck->GetOrigin() - move);
				geCheck->SetOrigin(geCheck->GetOrigin() - move2);
				//VectorSubtract( check->s.origin, move, check->s.origin );
				//VectorSubtract( check->s.origin, move2, check->s.origin );
				geBlocked = SG_TestEntityPosition( geCheck );
				if( !geBlocked ) {
					lastPushedState--;
					continue;
				}
			}
		}

		// Save obstacle so we can call its blocked callback.
		pushObstacle = geCheck;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for( PushedGameEntityState *pushedState = lastPushedState - 1; pushedState >= lastPushedState; pushedState-- ) {
	        // Fetch pusher's game entity.
            GameEntity* pusherEntity = pushedState->entityHandle;

            // Ensure we are dealing with a valid pusher entity.
            if ( !pusherEntity ) {
    		    SG_PhysicsEntityWPrint(__func__, "[move back loop]", "got an invalid entity handle!\n");
                continue;
            }

            pusherEntity->SetOrigin(pushedState->origin);
            pusherEntity->SetAngles(pushedState->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if ( pusherEntity->GetClient() ) {
				pusherEntity->SetOrigin(pusherEntity->GetClient()->playerState.pmove.origin);
                pusherEntity->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = pushedState->deltaYaw;
            }
#endif
            pusherEntity->LinkEntity();
		}
		return false;
	}

	//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for( PushedGameEntityState *pushedState = lastPushedState - 1; pushedState >= lastPushedState; pushedState-- ) {
        // Fetch pusher's base entity.
        GameEntity* pusherEntity = pushedState->entityHandle;

        // Ensure we are dealing with a valid pusher entity.
	    if ( !pusherEntity ) {
		    SG_PhysicsEntityWPrint(__func__, "[was moved loop] ", "got an invalid entity handle!\n");
            continue;
	    }

	    SG_TouchTriggers(pushedState->entityHandle);
	}

	return true;
}

/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle ) {
	if (!gePusherHandle.Get() || !*gePusherHandle) {
		return;
	}
	// Get GameEntity from Handle.
	GameEntity *gePusher = *gePusherHandle;

    // Ensure it is a valid entity.
    if ( !gePusher ) {
    	SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // if not a team captain, so movement will be handled elsewhere
	if ( gePusher->GetFlags() & EntityFlags::TeamSlave ) {
        return;
	}

	// Make sure all team followers can move before commiting
	// any moves or calling any think functions
	// If the move is blocked, all moved objects will be backed out
	lastPushedState = pushedGameEntities;

	GameEntity *gePushPart = nullptr;
    GameEntity *gePart = nullptr, *geMove = nullptr;
retry:
	lastPushedState = pushedGameEntities;

	gePushPart = nullptr;
    gePart = nullptr;
	geMove = nullptr;

	for( gePushPart = gePusher; gePushPart; gePushPart = gePushPart->GetTeamChainEntity() ) {
		// Get pusher part velocity.
		const vec3_t partVelocity = gePushPart->GetVelocity( );

		// Get pusher part Angular Velocity.
		const vec3_t partAngularVelocity = gePushPart->GetAngularVelocity();

		if (partVelocity.x || partVelocity.y || partVelocity.z ||
			partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z) 
		{
			// object is moving
		//if( part->s.linearMovement ) {
		//	GS_LinearMovement( &part->s, game.serverTime, move );
		//	VectorSubtract( move, part->s.origin, move );
		//	VectorScale( part->avelocity, FRAMETIME, amove );
		//} else {
			const vec3_t move = vec3_scale(gePushPart->GetVelocity(), FRAMETIME.count()); //VectorScale( part->velocity, FRAMETIME, move );
			const vec3_t amove = vec3_scale(gePushPart->GetAngularVelocity(), FRAMETIME.count()); //VectorScale( part->avelocity, FRAMETIME, amove );

			SGEntityHandle partHandle(gePushPart);
			if ( !SG_Push(partHandle, move, amove) ) {
				break;  // Move was Blocked.
			}
	   //}
		}
	}

	if( lastPushedState > &pushedGameEntities[4096] ) {
		//G_Error( "pushed_p > &pushed[MAX_EDICTS], memory corrupted" );
	}

	if( gePushPart ) {
		// the move failed, bump all nextthink times and back out moves
		for( GameEntity *mover = gePusher; mover; mover = mover->GetTeamChainEntity() ) {
			auto nextThinkTime = mover->GetNextThinkTime();
			if( nextThinkTime > GameTime::zero()) {
				mover->SetNextThinkTime(nextThinkTime + FRAMERATE_MS);
//				mover->nextThink += game.frametime;
			}
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
        if (gePushPart && pushObstacle) {
            gePushPart->DispatchBlockedCallback(pushObstacle);
        }
//#if 0

		// if the obstacle went away and the pusher is still there
		//if( !obstacle->r.inuse && part->r.inuse ) {
		//	goto retry;
		//}
        // if the pushed entity went away and the pusher is still there
        if ((pushObstacle && !pushObstacle->IsInUse()) ) {
            goto retry;
		}
//#endif
	} else {
        // the move succeeded, so call all Think functions
        for (gePart = gePusher ; gePart ; gePart = gePart->GetTeamChainEntity()) {
            SG_RunThink(gePart);
        }
    }
}