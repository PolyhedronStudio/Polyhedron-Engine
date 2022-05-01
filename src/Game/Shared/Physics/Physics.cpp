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
#include "../SharedGame.h"

#ifdef SHAREDGAME_SERVERGAME 
	#include "../../Server/ServerGameLocals.h"
	#include "../../Server/World/ServerGameWorld.h"
#endif
#ifdef SHAREDGAME_CLIENTGAME
	#include "../../Client/ClientGameLocals.h"
	#include "../../Client/World/ClientGameWorld.h"
#endif

// Physics.
#include "Physics.h"
#include "SlideMove.h"

// TODO: This needs some fixing hehe... ugly method but hey.
#ifdef SHAREDGAME_SERVERGAME
extern cvar_t *sv_maxvelocity;
extern cvar_t *sv_gravity;
void CheckSVCvars() {

}
#endif

#ifdef SHAREDGAME_CLIENTGAME
static cvar_t *sv_maxvelocity = nullptr;
static cvar_t *sv_gravity = nullptr;
static inline void CheckSVCvars() {
	if (!sv_maxvelocity) {
		#ifdef SHAREDGAME_CLIENTGAME
		sv_maxvelocity = clgi.Cvar_Get("sv_maxvelocity", "2000", 0);
		#endif	
	}
	if (!sv_gravity) {
		#ifdef SHAREDGAME_CLIENTGAME
		sv_gravity = clgi.Cvar_Get("sv_gravity", "875", 0);
		#endif
	}
}
#endif
//========================================================================

void SG_PhysicsEntityWPrint(const std::string &functionName, const std::string &functionSector, const std::string& message) {
    // Only continue if developer warnings for physics are enabled.
    //extern cvar_t* dev_show_physwarnings;
    //if (!dev_show_physwarnings->integer) {
    //    return;
    //}

    // Show warning.
    std::string warning = "SGPhysics Warning: (";
    warning += functionName;
    warning += ")[ ";
    warning += functionSector;
    warning += "]: ";
    warning += message;
    warning += "\n";
    //Com_DPrint(warning.c_str());
 //   // Write the index, programmers may look at that thing first
 //   std::string errorString = "";
 //   if (ent->GetPODEntity()) {
	//errorString += "entity (index " + std::to_string(ent->GetNumber());
 //   } else {
	//errorString += "entity has no ServerEntity ";
 //   }

 //   // Write the targetname as well, if it exists
 //   if (!ent->GetTargetName().empty()) {
	//errorString += ", name '" + ent->GetTargetName() + "'";
 //   }

 //   // Write down the C++ class name too
 //   errorString += ", class '";
 //   errorString += ent->GetTypeInfo()->classname;
 //   errorString += "'";

 //   // Close it off and state what's actually going on
 //   errorString += ") has a nullptr think callback \n";
 //   //
 //   gi.Error(errorString.c_str());
}

//================================================================================
/*
* GS_ClipVelocity
*/
static inline vec3_t GS_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce ) {
	float backoff = vec3_dot( inVelocity, normal );

	if( backoff <= 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	// Calculate out velocity vector.
	vec3_t outVelocity = ( inVelocity - vec3_scale( normal, backoff ) );

	// SlideMove clamp it.
	//#ifdef GS_SLIDEMOVE_CLAMPING
	{
		float oldSpeed = vec3_length(inVelocity);
		float newSpeed = vec3_length(outVelocity);

		if (newSpeed > oldSpeed) {
			outVelocity = vec3_scale(vec3_normalize(outVelocity), oldSpeed);
		}
	}

	return outVelocity;
	//#endif GS_SLIDEMOVE_CLAMPING
	//for( i = 0; i < 3; i++ ) {
	//	change = normal[i] * backoff;
	//	out[i] = in[i] - change;
	//}
	//#ifdef GS_SLIDEMOVE_CLAMPING
	//	{
	//		float oldspeed, newspeed;
	//		oldspeed = VectorLength( in );
	//		newspeed = VectorLength( out );
	//		if( newspeed > oldspeed ) {
	//			VectorNormalize( out );
	//			VectorScale( out, oldspeed, out );
	//		}
	//	}
	//#endif
}

//================================================================================

/**
*	@brief	Applies 'downward' gravity forces to the entity.
**/
void SG_AddGravity( GameEntity *sharedGameEntity ) {
    // Fetch velocity.
    vec3_t velocity = sharedGameEntity->GetVelocity();

    // Apply gravity.
    velocity.z -= sharedGameEntity->GetGravity() * sv_gravity->value * FRAMETIME_S.count();

    // Apply new velocity to entity.
    sharedGameEntity->SetVelocity(velocity);
}

/**
*	@brief	Apply ground friction forces to entity.
**/
void SG_AddGroundFriction( GameEntity *sharedGameEntity, float friction ) {
	if (!sharedGameEntity) {
		Com_DPrintf("SGWarning: %s called with sharedGameEntity(nullptr)!\n", __func__);
	}

	const vec3_t geVelocity = sharedGameEntity->GetVelocity();
	vec3_t groundVelocity = {
		geVelocity.x,
		geVelocity.y,
		0.f
	};

	// Calculate speed velocity based on ground velocity.
	vec3_t frictionVec = vec3_zero();
	float speed = VectorNormalize2( groundVelocity, frictionVec );
	if( speed ) {
		// Calculate ground friction speed.
		float fspeed = friction * FRAMETIME_S.count();
		if( fspeed > speed ) {
			fspeed = speed;
		}

		// Actual velocity.
		const vec3_t velocity = sharedGameEntity->GetVelocity();

		// Update velocity.
		sharedGameEntity->SetVelocity(vec3_fmaf(velocity, -fspeed, frictionVec));
	}
}

/**
*
**/
/*
* G_SolidMaskForEnt
*/
int32_t SG_SolidMaskForGameEntity( GameEntity *gameEntity ) {
	if (!gameEntity) {
		return BrushContentsMask::Solid;
	}

	const int32_t geClipMask = gameEntity->GetClipMask();
	return (geClipMask ? geClipMask : BrushContentsMask::Solid);
}
/**
*
**/
void SG_CheckGround( GameEntity *geCheck ) {
	if (!geCheck) {
		return;
	}

	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	if( geCheck->GetClient()  && geCheck->GetVelocity().z > 180) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// if the hull point one-quarter unit down is solid the entity is on ground
	const vec3_t geOrigin = geCheck->GetOrigin();
	vec3_t traceEndPoint = {
		geOrigin.x,
		geOrigin.y,
		geOrigin.z - 0.25f
	};

	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck));

	// check steepness
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	if( ( geCheck->GetVelocity().z > 1 && !geCheck->GetClient()) && !traceResult.startSolid) {
		geCheck->SetGroundEntity(nullptr);
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	if( !traceResult.startSolid && !traceResult.allSolid ) {
		//VectorCopy( trace.endpos, ent->s.origin );
		geCheck->SetGroundEntity(traceResult.gameEntity);
		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;
		
		vec3_t geCheckVelocity = geCheck->GetVelocity();
		if( geCheckVelocity.z < 0) {
			geCheckVelocity.z = 0;
			geCheck->SetVelocity(geCheckVelocity);
		}
	}
}

/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, int32_t contentMask, float slideBounce, float friction ) {
	int32_t i;
	MoveState entMove = {};
	int32_t blockedMask = 0;

	if (!geSlider) {
		SG_PhysicsEntityWPrint(__func__, "[start]", "geSlider is (nullptr)!\n");
		return 0;
	}

	// Store current velocity as old velocity.
	float oldVelocity = VectorLength( geSlider->GetVelocity() );

	// Apply gravitational force if no ground entity is set. Otherwise, apply ground friction forces.
	if( !geSlider->GetGroundEntity() ) {
		SG_AddGravity( geSlider );
	} else { // horizontal friction
		SG_AddGroundFriction( geSlider, friction );
	}

	// Initialize our move state.
	entMove.numClipPlanes		= 0;
	entMove.numTouchEntities	= 0;

	// When the entity isn't idle, move its properties over into our move state.
	if( oldVelocity > 0 ) {
		// Setup physical properties.
		entMove.origin = geSlider->GetOrigin();
		entMove.velocity = geSlider->GetVelocity();
		entMove.mins = geSlider->GetMins();
		entMove.maxs = geSlider->GetMaxs();
		/*VectorCopy( ent->s.origin, entMove.origin );
		VectorCopy( ent->velocity, entMove.velocity );
		VectorCopy( ent->r.mins, entMove.mins );
		VectorCopy( ent->r.maxs, entMove.maxs );*/
		entMove.gravityDir = vec3_t{ 0.f, 0.f, -1.f };
		entMove.slideBounce = slideBounce;
		entMove.groundEntity = ( *geSlider->GetGroundEntity() );

		// Setup passing entity and contentmask.
		entMove.passEntity = geSlider;
		entMove.contentMask = contentMask;

		entMove.remainingTime = FRAMETIME_S.count();

		// Execute actual slide movement.
		blockedMask = SG_SlideMove( &entMove );

		// Update our entity with the resulting values.
		geSlider->SetOrigin(entMove.origin); //VectorCopy( entMove.origin, ent->s.origin );
		geSlider->SetVelocity(entMove.velocity); //VectorCopy( entMove.velocity, ent->velocity );
		geSlider->SetGroundEntity(entMove.groundEntity);

		// Link entity in.
		geSlider->LinkEntity();

		//GClip_LinkEntity( ent );
	}

	// Execute touch callbacks.
	if( contentMask != 0 ) {
		GameEntity *otherEntity = nullptr;
		//GClip_TouchTriggers( ent );
		SG_TouchTriggers(otherEntity);

		// touch other objects
		for( int32_t i = 0; i < entMove.numTouchEntities; i++ ) {
			otherEntity = entMove.touchEntites[i];
			
			// Don't touch projectiles.
			if( !otherEntity ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			//G_CallTouch( other, ent, NULL, 0 );
			otherEntity->DispatchTouchCallback(otherEntity, geSlider, nullptr, nullptr);

			// if self touch function, fire up touch and if freed stop
			// It may have been deleted after all.
			if (geSlider) {
				//G_CallTouch( ent, other, NULL, 0 );
				geSlider->DispatchTouchCallback(geSlider, otherEntity, nullptr, nullptr);
			}

			// It may have been freed by the touch function.
			if( geSlider->IsInUse()) {
				break;
			}
		}
	}

	// If it's still in use, search for ground.
	if ( geSlider && geSlider->IsInUse() ) {
		// Check for ground entity.
		SG_CheckGround( geSlider );

		// We've found ground.
		if( geSlider->GetGroundEntity() && vec3_length(geSlider->GetVelocity()) <= 1.f && oldVelocity > 1.f) {
			// Zero out velocities.
			geSlider->SetVelocity( vec3_zero() );
			geSlider->SetAngularVelocity( vec3_zero() );

			// Stop.
			//geSlider->DispatchStopCallback(geSlider, oldVelocity, oldAngularVelocity );
		}
	}

	return blockedMask;
}



//================================================================================

//pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.
//
//onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects
//
//doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
//bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
//corpses are SOLID_NOT and MOVETYPE_TOSS
//crates are SOLID_BBOX and MOVETYPE_TOSS
//walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
//flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY
//
//solid_edge items only clip against bsp models.


/**
*	@brief	Tests whether the entity position would be trapped in a Solid.
*	@return	(nullptr) in case it is free from being trapped. Worldspawn entity otherwise.
**/
GameEntity *SG_TestEntityPosition(GameEntity *geTestSubject) {
	int32_t clipMask = 0;

    if (geTestSubject->GetClipMask()) {
	    clipMask = geTestSubject->GetClipMask();
    } else {
        clipMask = BrushContentsMask::Solid;
    }

    SGTraceResult trace = SG_Trace(geTestSubject->GetOrigin(), geTestSubject->GetMins(), geTestSubject->GetMaxs(), geTestSubject->GetOrigin(), geTestSubject, clipMask);

    if (trace.startSolid) {
		SGGameWorld *gameWorld = GetGameWorld();

	    return (GameEntity*)(gameWorld->GetWorldspawnGameEntity());
    }

    return nullptr;
}

/**
*	@brief	Keep entity velocity within bounds.
**/
static void SG_CheckVelocity( GameEntity *geCheck ) {
	
	// Get Velocity.
	const vec3_t entityVelocity = geCheck->GetVelocity();

	// Get velocity vector length.
	float scale = vec3_length( entityVelocity );
	//if( ( scale > g_maxvelocity->value ) && ( scale ) ) {

	// If it exceeds max velocity...
	if ( (scale > sv_maxvelocity->value) && (scale) ) {
		// Calculate scale to adjust velocity by.
		scale = sv_maxvelocity->value / scale;

		// Scale and set new velocity.
		geCheck->SetVelocity(vec3_scale(entityVelocity, scale));
	}
}

///**
//* SG_RunThink
//*
//* Runs thinking code for this frame if necessary.
//**/
//static void SG_RunThink( edict_t *ent ) {
//	int64_t thinktime;
//
//	thinktime = ent->nextThink;
//	if( thinktime <= 0 ) {
//		return;
//	}
//	if( thinktime > level.time ) {
//		return;
//	}
//
//	ent->nextThink = 0;
//
//	if( ISEVENTENTITY( &ent->s ) ) { // events do not think
//		return;
//	}
//
//	G_CallThink( ent );
//}

/**
*	@brief	Called when two entities have touched so we can safely call their touch callback functions.
**/
void SG_Impact( GameEntity *entityA, const SGTraceResult &traceResult ) {
	// Get gameWorld pointer.
	SGGameWorld *gameWorld = GetGameWorld();

	// Ensure antityA is non (nullptr).
	if (!entityA) {
		// Com_DPrint(
	}

	if( traceResult.gameEntity != (GameEntity*)gameWorld->GetWorldspawnGameEntity() ) {
		// Entity to dispatch against.
		GameEntity *entityB = traceResult.gameEntity;

		// Execute touch functionality for entityA if its solid is not a Solid::Not.
		if (entityA->GetSolid() != Solid::Not) {
			//e1->DispatchTouchCallback(e1, e2, &trace->plane, trace->surface);
			entityA->DispatchTouchCallback(entityA, entityB, (CollisionPlane*)& traceResult.plane, traceResult.surface);
		}

		// Execute touch functionality for entityB exists, and is not a Solid::Not.
		if (entityB != nullptr && entityB->GetSolid() != Solid::Not) {
			//e2->DispatchTouchCallback(e2, e1, NULL, NULL);
			entityB->DispatchTouchCallback(entityB, entityA, nullptr, nullptr);
		}
	}
}

/*
* SV_FlyMove
*
* The basic solid body movement clip that slides along multiple planes
* Returns the clipflags if the velocity was modified (hit something solid)
* 1 = floor
* 2 = wall / step
* 4 = dead stop
*/
#if 0
#define MAX_CLIP_PLANES 5
int SV_FlyMove( edict_t *ent, float time, int mask ) {
	edict_t *hit;
	int bumpcount, numbumps;
	vec3_t dir;
	float d;
	int numplanes;
	vec3_t planes[MAX_CLIP_PLANES];
	vec3_t primal_velocity, original_velocity, new_velocity;
	int i, j;
	trace_t trace;
	vec3_t end;
	float time_left;
	int blocked;

	numbumps = 4;

	blocked = 0;
	VectorCopy( ent->velocity, original_velocity );
	VectorCopy( ent->velocity, primal_velocity );
	numplanes = 0;

	time_left = time;

	ent->groundentity = NULL;
	for( bumpcount = 0; bumpcount < numbumps; bumpcount++ ) {
		for( i = 0; i < 3; i++ )
			end[i] = ent->s.origin[i] + time_left * ent->velocity[i];

		G_Trace4D( &trace, ent->s.origin, ent->r.mins, ent->r.maxs, end, ent, mask, ent->timeDelta );
		if( trace.allsolid ) { // entity is trapped in another solid
			VectorClear( ent->velocity );
			return 3;
		}

		if( trace.fraction > 0 ) { // actually covered some distance
			VectorCopy( trace.endpos, ent->s.origin );
			VectorCopy( ent->velocity, original_velocity );
			numplanes = 0;
		}

		if( trace.fraction == 1 ) {
			break; // moved the entire distance

		}
		hit = &game.edicts[trace.ent];

		if( IsWalkablePlane( &trace.plane ) ) {
			blocked |= 1; // floor
			if( hit->r.solid == SOLID_BSP ) {
				ent->groundentity = hit;
				ent->groundentity_linkcount = hit->r.linkcount;
			}
		}
		if( !trace.plane.normal[2] ) {
			blocked |= 2; // step
		}

		//
		// run the impact function
		//
		SV_Impact( ent, &trace );
		if( !ent->r.inuse ) {
			break; // removed by the impact function

		}
		time_left -= time_left * trace.fraction;

		// cliped to another plane
		if( numplanes >= MAX_CLIP_PLANES ) { // this shouldn't really happen
			VectorClear( ent->velocity );
			return 3;
		}

		VectorCopy( trace.plane.normal, planes[numplanes] );
		numplanes++;

		//
		// modify original_velocity so it parallels all of the clip planes
		//
		for( i = 0; i < numplanes; i++ ) {
			GS_ClipVelocity( original_velocity, planes[i], new_velocity, 1 );
			for( j = 0; j < numplanes; j++ )
				if( j != i ) {
					if( DotProduct( new_velocity, planes[j] ) < 0 ) {
						break; // not ok
					}
				}
			if( j == numplanes ) {
				break;
			}
		}

		if( i != numplanes ) { // go along this plane
			VectorCopy( new_velocity, ent->velocity );
		} else {   // go along the crease
			if( numplanes != 2 ) {
				VectorClear( ent->velocity );
				return 7;
			}
			CrossProduct( planes[0], planes[1], dir );
			VectorNormalize( dir );
			d = DotProduct( dir, ent->velocity );
			VectorScale( dir, d, ent->velocity );
		}

		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if( DotProduct( ent->velocity, primal_velocity ) <= 0 ) {
			VectorClear( ent->velocity );
			return blocked;
		}
	}

	return blocked;
}
#endif

//===============================================================================
//
//PUSHMOVE
//
//===============================================================================


/*
* SG_PushEntity
*
* Does not change the entities velocity at all
*/
static SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset ) {
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
	SGEntityHandle entityHandle;
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
static bool SG_Push( GameEntity *gePusher, const vec3_t &move, const vec3_t &angularMove ) {
	/*int i, e;
	edict_t *check, *block;
	vec3_t mins, maxs;
	pushed_t *p;
	mat3_t axis;
	vec3_t org, org2, move2;*/

	if (!gePusher ) {
		return false;
	}
	PushedGameEntityState *pushed;
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
	if (gePusher->GetClient()) {
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
	for (int32_t i = 1; i < 4096; i++) {
		// Get the entity to check position for.
		GameEntity *geCheck = gameWorld->GetGameEntityByIndex(i);

		// Get some data.
		const bool IsInUse = geCheck->IsInUse();
		const int32_t moveType = geCheck->GetMoveType();
		const vec3_t absMin = geCheck->GetAbsoluteMin();
		const vec3_t absMax = geCheck->GetAbsoluteMax();

		// Skip Checks.
		if (!geCheck || !geCheck->IsInUse()) {
			// Skip if it's not in use.
			continue;
		}
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
		if ( *(geCheck->GetGroundEntity()) != gePusher ) {
			// If it's not, check whether at least the bounding box intersects.
			if ( (absMin[0] >= maxs[0]) &&
				(absMin[1] >= maxs[1]) &&
				(absMax[2] >= mins[2]) &&
				(absMax[0] >= mins[0]) &&
				(absMax[1] >= mins[1]) &&
				(absMax[2] >= mins[2])) {
				continue;
			}

			// See if the ent's bbox is inside the pusher's final position.
			if( !SG_TestEntityPosition( geCheck ) ) {
				continue;
			}
		}
		
		if( ( gePusher->GetMoveType() == MoveType::Push) || (geCheck->GetGroundEntity() == gePusher)) {
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
            if (geCheck->GetClient()) {
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
				if( geCheck->GetGroundEntity() != gePusher) {
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
            GameEntity* pusherEntity = *pushedState->entityHandle;

            // Ensure we are dealing with a valid pusher entity.
            if (!pusherEntity) {
    		    SG_PhysicsEntityWPrint(__func__, "[move back loop]", "got an invalid entity handle!\n");
                continue;
            }

            pusherEntity->SetOrigin(pushedState->origin);
            pusherEntity->SetAngles(pushedState->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (pusherEntity->GetClient()) {
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
        GameEntity* pusherEntity = *pushedState->entityHandle;

        // Ensure we are dealing with a valid pusher entity.
	    if (!pusherEntity) {
		    SG_PhysicsEntityWPrint(__func__, "[was moved loop] ", "got an invalid entity handle!\n");
            continue;
	    }

	    SG_TouchTriggers(*pushedState->entityHandle);
	}

	return true;
}

/**
*	@brief Pushes all objects except for brush models. 
**/
static void SG_Physics_Pusher( SGEntityHandle &gePusherHandle ) {
	GameEntity *gePusher = *gePusherHandle;
    // Ensure it is a valid entity.
    if (!gePusher ) {
    	SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // if not a team captain, so movement will be handled elsewhere
    if (gePusher ->GetFlags() & EntityFlags::TeamSlave) {
        return;
	}

	// make sure all team followers can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	//retry:
	PushedGameEntityState *gePushed = pushedGameEntities;

	GameEntity *gePushPart = nullptr;
	for( gePushPart = gePusher; gePushPart; gePushPart = gePushPart->GetTeamChainEntity() ) {
		// Get pusher part velocity.
		const vec3_t partVelocity = gePushPart->GetVelocity();

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

			if (!SG_Push(gePushPart, move, amove)) {
				break;  // Move was Blocked.
			}
	   //}
		}
	}

	if( gePushed > &pushedGameEntities[4096] ) {
		//G_Error( "pushed_p > &pushed[MAX_EDICTS], memory corrupted" );
	}

	if( gePushPart ) {
		// the move failed, bump all nextthink times and back out moves
		for( GameEntity *mover = gePusher; mover; mover = mover->GetTeamChainEntity() ) {
			auto nextThinkTime = mover->GetNextThinkTime();
			if( nextThinkTime > GameTime::zero()) {
				mover->SetNextThinkTime(nextThinkTime + FRAMETIME_S);
//				mover->nextThink += game.frametime;
			}
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
        if (gePushPart) {
            gePushPart->DispatchBlockedCallback(pushObstacle);
        }
#if 0

		// if the obstacle went away and the pusher is still there
		if( !obstacle->r.inuse && part->r.inuse ) {
			goto retry;
		}
#endif
	}
}

//==================================================================


/**
*   @brief  Executes MoveType::None behavior for said entity. Meaning it
*           only gets a chance to think.
**/
void SG_Physics_None(SGEntityHandle& entityHandle) {
    // Assign handle to base entity.
    GameEntity* gameEntity = *entityHandle;

    // Ensure it is a valid entity.
    if (!gameEntity) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Run think method.
   // SG_RunThink(gameEntity);
}


/**
*   @brief  Executes MoveType::Noclip, behavior on said entity meaning it
*           does not clip to world, or respond to other entities.
**/
void SG_Physics_NoClip(SGEntityHandle &entityHandle) {
    // Assign handle to base entity.
    GameEntity* gameEntity = *entityHandle;

    // Ensure it is a valid entity.
    if (!gameEntity) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // regular thinking
 //   if (!SG_RunThink(gameEntity)) {
 //       return;
	//}
    if (!gameEntity->IsInUse()) {
        return;
	}

    gameEntity->SetAngles(vec3_fmaf(gameEntity->GetAngles(), FRAMETIME.count(), gameEntity->GetAngularVelocity()));
    gameEntity->SetOrigin(vec3_fmaf(gameEntity->GetOrigin(), FRAMETIME.count(), gameEntity->GetVelocity()));

    gameEntity->LinkEntity();
}

//==============================================================================
//
//TOSS / BOUNCE
//
//==============================================================================

/*
* SV_Physics_Toss
*
* Toss, bounce, and fly movement.  When onground, do nothing.
*
* FIXME: This function needs a serious rewrite
*/
/**
*   @brief  Executes MoveType::Toss, TossSlide, Bounce and Fly movement behaviors on
*           said entity.
**/
static void SG_Physics_Toss(SGEntityHandle& entityHandle) {
    // Get Gameworld.
	SGGameWorld *gameWorld = GetGameWorld();

	// Assign handle to base entity.
    GameEntity* ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
    	SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // If not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave) {
        return;
	}

	// Refresh the ground entity for said MoveType entities:
	//if( ent->movetype == MOVETYPE_BOUNCE || ent->movetype == MOVETYPE_BOUNCEGRENADE ) {
		if( ent->GetVelocity().z > 0.1f) {
			ent->SetGroundEntity(nullptr);
		}
	//}

	// Check whether the ground entity has disappeared(aka not in use).
	GameEntity *entGroundEntity = *ent->GetGroundEntity();
	//	if( ent->groundentity && ent->groundentity != world && !ent->groundentity->r.inuse ) {
	if (entGroundEntity && entGroundEntity != (GameEntity*)gameWorld->GetWorldspawnGameEntity() && !entGroundEntity->IsInUse()) {
		ent->SetGroundEntity(nullptr); //ent->groundentity = NULL;
	}

	// Calculate old speed based on velocity vec length.
	float oldSpeed = vec3_length( ent->GetVelocity() );

	// Check if the ent still has a valid ground entity.
	if ( ent->GetGroundEntity() ) {
		// Exit if there's no velocity(speed) activity.
		if( !oldSpeed ) {
			return;
		}

		// Special movetype Toss behavior.
		if( ent->GetMoveType() == MoveType::Toss) {
			// 8 = 1 Unit on the Quake scale I think.
			if( ent->GetVelocity().z >= 8) {
				// it's moving in-air so unset the ground entity.
				ent->SetGroundEntity(nullptr);
			} else {
				// Otherwise, let it fall to a stop.
				ent->SetVelocity(vec3_zero());
				ent->SetAngularVelocity(vec3_zero());
				// Dispatch Stop callback.
				//ent->DispatchStopCallback(); //G_CallStop( ent );		
				return;
			}
		}
	}

	// Get origin.
	const vec3_t oldOrigin = ent->GetOrigin(); // VectorCopy( ent->s.origin, old_origin );

	// As long as there's any acceleration at all, calculate it for the current frame.
	if( ent->GetAcceleration()  != 0) {
		// Negative acceleration and a low velocity makes it come to a halt.
		if( ent->GetAcceleration() < 0 && vec3_length(ent->GetVelocity()) < 50) {
			ent->SetVelocity(vec3_zero());
			//VectorClear( ent->velocity );
		} else {
			//VectorNormalize2
			vec3_t acceldir = vec3_zero();
			VectorNormalize2( ent->GetVelocity(), acceldir);
			acceldir = vec3_scale( acceldir, ent->GetAcceleration()  * FRAMETIME_S.count());

			// Add directional acceleration to velocity.
			ent->SetVelocity(ent->GetVelocity() + acceldir);//VectorAdd( ent->velocity, acceldir, ent->velocity );
		}
	}

	// Check velocity and maintain it in bounds.
	SG_CheckVelocity( ent );

	// Add gravitational forces.
	if( ent->GetMoveType()  != MoveType::Fly && !ent->GetGroundEntity()) {
		SG_AddGravity( ent );
	}

	// Hacks
	//if( ent->s.type != ET_PLASMA && ent->s.type != ET_ROCKET ) {
	//	// move angles
	//	VectorMA( ent->s.angles, FRAMETIME, ent->avelocity, ent->s.angles );
	//}

	// move origin
	const vec3_t move = vec3_scale( ent->GetVelocity(), FRAMETIME_S.count() ); //VectorScale( ent->velocity, FRAMETIME, move );

	SGTraceResult traceResult = SG_PushEntity( ent, move );
	if( !ent->IsInUse() ) {
		return;
	}

	// In case the trace hit anything...
	if( traceResult.fraction < 1.0f ) {
		const int32_t entMoveType = ent->GetMoveType();

		// 1 = default backOff value.
		float backOff = 1;

		// Adjust value for specific moveTypes.
		if( entMoveType == MoveType::Bounce) {
			backOff = 1.5;
		}/* else if( entMoveType == MoveType::BounceGrenade) {
			backOff = 1.4;
		} */

		// Clip velocity.
		ent->SetVelocity(SG_ClipVelocity( ent->GetVelocity(), traceResult.plane.normal, backOff ));

		// stop if on ground

		if( entMoveType == MoveType::Bounce ) {// || entMoveType == MoveType::BounceGrenade ) {
			// stop dead on allsolid

			// LA: hopefully will fix grenades bouncing down slopes
			// method taken from Darkplaces sourcecode
			if( traceResult.allSolid ||
				( IsWalkablePlane( traceResult.plane ) && fabsf( vec3_dot( traceResult.plane.normal, ent->GetVelocity() ) ) < 60 ) ) {
				GameEntity *geTrace = traceResult.gameEntity;
				// Update ground entity.
				ent->SetGroundEntity(geTrace);
				ent->SetGroundEntityLinkCount((geTrace ? geTrace->GetLinkCount() : 0));

				// Zero out (angular-)velocities.
				ent->SetVelocity(vec3_zero());
				ent->SetAngularVelocity(vec3_zero());

				// Dispatch a Stop callback.
				//ent->DispatchStopCallback(); //G_CallStop( ent );
			}
		} else {
			// in movetype_toss things stop dead when touching ground
#if 0
			G_CheckGround( ent );

			if( ent->groundentity ) {
#else

			// Walkable or trapped inside solid brush
			if( traceResult.allSolid || IsWalkablePlane( traceResult.plane ) ) {
				GameEntity *geTrace = (traceResult.gameEntity ? traceResult.gameEntity : (GameEntity*)gameWorld->GetWorldspawnGameEntity());
				// Update ground entity.
				ent->SetGroundEntity(geTrace);
				ent->SetGroundEntityLinkCount((geTrace ? geTrace->GetLinkCount() : 0));
#endif
				// Zero out (angular-)velocities.
				ent->SetVelocity(vec3_zero());
				ent->SetAngularVelocity(vec3_zero());

				// Dispatch a Stop callback.
				//ent->DispatchStopCallback(); //G_CallStop( ent );
			}
		}
	}

    //
	// Check for whether we're transitioning in or out of a liquid(mostly, water, it is considered as, water...).
	//
	// Were we in water?
    const bool wasInWater = (ent->GetWaterType() & BrushContentsMask::Liquid ? true : false);

	// Get content type for current origin.
	const int32_t pointContentType = SG_PointContents(ent->GetOrigin());
	const bool isInWater = (pointContentType & BrushContentsMask::Liquid);

	// Set the watertype(or whichever the contents are.)
	ent->SetWaterType(pointContentType);
	
	// never allow items in CONTENTS_NODROP
	//if( ent->item && ( ent->watertype & CONTENTS_NODROP ) ) {
	//	G_FreeEdict( ent );
	//	return;
	//}
	// Update entity's waterlevel.
	if( isInWater ) {
		ent->SetWaterLevel(1);
	} else {
		ent->SetWaterLevel(0);
	}

	// Play transitioning sounds.
#ifdef SHAREDGAME_SERVERGAME
    // Determine what sound to play.
    if (!wasInWater && isInWater) {
        gi.PositionedSound(oldOrigin, game.world->GetPODEntities(), SoundChannel::Auto, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
	} else if (wasInWater && !isInWater) {
        gi.PositionedSound(ent->GetOrigin(), game.world->GetPODEntities(), SoundChannel::Auto, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
	}
#endif
	//if( !wasInWater && isInWater ) {
	//	SG_PositionedSound( oldOrigin, SoundChannel::Auto, SG_SoundIndex( S_HIT_WATER ), Attenuation::Idle );
	//} else if( wasInWater && !isInWater ) {
	//	SG_PositionedSound( ent->GetOrigin(), SoundChannel::Auto, SG_SoundIndex(S_HIT_WATER), Attenuation::Idle );
	//}

	// Move followers.
	for( GameEntity *follower = ent->GetTeamChainEntity(); follower; follower = follower->GetTeamChainEntity() ) {
		// Set follower origin to ent origin.
		follower->SetOrigin(ent->GetOrigin());

		// Link follower.
		follower->LinkEntity();
	}

	// Hacks
	//if( ent->s.type == ET_PLASMA || ent->s.type == ET_ROCKET ) {
	//	if( const auto squaredSpeed = VectorLengthSquared( ent->velocity ); squaredSpeed > 1.0f ) {
	//		vec3_t velocityDir { ent->velocity[0], ent->velocity[1], ent->velocity[2] };
	//		const float invSpeed = 1.0f / std::sqrt( squaredSpeed );
	//		VectorScale( velocityDir, invSpeed, velocityDir );
	//		VecToAngles( velocityDir, ent->s.angles );
	//	}
	//}
}

//============================================================================

//void SV_Physics_LinearProjectile( edict_t *ent, int lookAheadTime ) {
//	// if not a team captain movement will be handled elsewhere
//	if( ent->flags & FL_TEAMFOLLOWER ) {
//		return;
//	}
//
//	const int old_waterLevel = ent->waterlevel;
//	const int mask = ( ent->r.clipmask ) ? ent->r.clipmask : MASK_SOLID;
//
//	const float startLineParam = ( ent->s.linearMovementPrevServerTime - ent->s.linearMovementTimeStamp ) * 0.001f;
//	ent->s.linearMovementPrevServerTime = game.serverTime;
//	const float endLineParam = ( lookAheadTime + game.serverTime - ent->s.linearMovementTimeStamp ) * 0.001f;
//
//	vec3_t start, end;
//	VectorMA( ent->s.linearMovementBegin, startLineParam, ent->s.linearMovementVelocity, start );
//	VectorMA( ent->s.linearMovementBegin, endLineParam, ent->s.linearMovementVelocity, end );
//
//	// Make sure the segments that are checked every frame overlap by a unit.
//	// This is not obligatory but let's ensure they make a continuous joint segment.
//	const float squareSpeed = VectorLengthSquared( ent->s.linearMovementVelocity );
//	if( squareSpeed > 1 ) {
//		const float invSpeed = 1.0f / std::sqrt( squareSpeed );
//		vec3_t velocityDir;
//		VectorScale( ent->s.linearMovementVelocity, invSpeed, velocityDir );
//		VectorSubtract( start, velocityDir, start );
//		VectorAdd( end, velocityDir, end );
//	}
//
//	trace_t trace;
//	G_Trace4D( &trace, start, ent->r.mins, ent->r.maxs, end, ent, mask, ent->timeDelta );
//	VectorCopy( trace.endpos, ent->s.origin );
//	GClip_LinkEntity( ent );
//	SV_Impact( ent, &trace );
//
//	if( !ent->r.inuse ) { // the projectile may be freed if touched something
//		return;
//	}
//
//	// update some data required for the transmission
//	//VectorCopy( ent->velocity, ent->s.linearMovementVelocity );
//
//	GClip_TouchTriggers( ent );
//	ent->groundentity = NULL; // projectiles never have ground entity
//	ent->waterlevel = ( G_PointContents4D( ent->s.origin, ent->timeDelta ) & MASK_WATER ) ? true : false;
//
//	if( !old_waterLevel && ent->waterlevel ) {
//		G_PositionedSound( start, CHAN_AUTO, trap_SoundIndex( S_HIT_WATER ), ATTN_IDLE );
//	} else if( old_waterLevel && !ent->waterlevel ) {
//		G_PositionedSound( ent->s.origin, CHAN_AUTO, trap_SoundIndex( S_HIT_WATER ), ATTN_IDLE );
//	}
//}

//============================================================================


/*
* G_RunEntity
*
*/
//#ifdef SHAREDGAME_SERVERGAME
// Runs entity thinking code for this frame if necessary
//===============
qboolean SG_RunThink(GameEntity *geThinker) {
    if (!geThinker) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "geThinker is (nullptr)!\n");
        return false;
    }

    // Fetch think time.
    GameTime nextThinkTime = geThinker->GetNextThinkTime();

	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
		return true;
    }

    // Reset think time before thinking.
    geThinker->SetNextThinkTime(GameTime::zero());

#if _DEBUG
    if ( !geThinker->HasThinkCallback() ) {
        // Write the index, programmers may look at that thing first
        std::string errorString = "";
        if (geThinker->GetPODEntity()) {
            errorString += "entity (index " + std::to_string(geThinker->GetNumber());
        } else {
            errorString += "entity has no PODEntity ";
        }

        // Write the targetname as well, if it exists
        if ( !geThinker->GetTargetName().empty() ) {
            errorString += ", name '" + geThinker->GetTargetName() + "'";
        }

        // Write down the C++ class name too
        errorString += ", class '";
        errorString += geThinker->GetTypeInfo()->classname;
        errorString += "'";

        // Close it off and state what's actually going on
        errorString += ") has a nullptr think callback \n";
    //    
        //gi.Error( errorString.c_str() );

        // Return true.
        return true;
    }
#endif

    // Last but not least, let the entity execute its think behavior callback.
    geThinker->Think();

    return false;
}

void SG_RunEntity(SGEntityHandle &entityHandle) {
	// TODO: Make this less hacky ofc, get normal sane access to cvars.
	CheckSVCvars();

	// Get GameEntity from handle.
    GameEntity *ent = reinterpret_cast<GameEntity*>(*entityHandle);

    if (!ent) {
	    SG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Execute the proper physics that belong to its movetype.
    const int32_t moveType = ent->GetMoveType();

	//if( ISEVENTENTITY( &ent->s ) ) { // events do not think
	//	return;
	//}

	//if( ent->timeDelta && !( ent->r.svflags & SVF_PROJECTILE ) ) {
	//	G_Printf( "Warning: G_RunEntity 'Fixing timeDelta on non projectile entity\n" );
	//	ent->timeDelta = 0;
	//}

	// only team captains decide the think, and they make think their team members when they do
	if( !( ent->GetFlags() & EntityFlags::TeamSlave)) {
		for( GameEntity *gePart = ent; gePart ; gePart = gePart->GetTeamChainEntity()) {
			SG_RunThink( gePart );
		}
	}

	switch( moveType ) {
	// SG_Physics_None:
		case MoveType::None:
		case MoveType::PlayerMove:
	        SG_Physics_None(entityHandle);
        break;
	
	// SG_Physics_Pusher:
		case MoveType::Push:
        case MoveType::Stop:
	        SG_Physics_Pusher(entityHandle);
        break;
	// SG_Physics_NoClip:
        case MoveType::NoClip:
        case MoveType::Spectator:
	        SG_Physics_NoClip(entityHandle);
        break;
	// SG_Physics_Step:
        case MoveType::Step:
            //SG_Physics_Step(entityHandle);
        break;
	// SG_Physics_Toss:
        case MoveType::Toss:
        case MoveType::Bounce:
        case MoveType::Fly:
        case MoveType::FlyMissile:
	        SG_Physics_Toss(entityHandle);
        break;
	// SG_BoxSlideMove:
		case MoveType::TossSlide:
			//	SG_BoxSlideMove( ent, ent->r.clipmask ? ent->r.clipmask : MASK_PLAYERSOLID, 1.01f, 10 );
		break;
	default:
		//	G_Error( "SV_Physics: bad movetype %i", (int)ent->movetype );
		break;
		//case MoveType::None:
		//case MoveType::NoClip: // only used for clients, that use pmove
		//	SV_Physics_None( ent );
		//	break;
		//case MOVETYPE_PLAYER:
		//	SV_Physics_None( ent );
		//	break;
		//case MoveType::Push:
		//case MoveType::Stop:
		//	SV_Physics_Pusher( ent );
		//	break;
		//case MOVETYPE_BOUNCE:
		//case MOVETYPE_BOUNCEGRENADE:
		//	SV_Physics_Toss( ent );
		//	break;
		//case MOVETYPE_TOSS:
		//	SV_Physics_Toss( ent );
		//	break;
		//case MOVETYPE_FLY:
		//	SV_Physics_Toss( ent );
		//	break;
		//case MOVETYPE_LINEARPROJECTILE:
		//	SV_Physics_LinearProjectile( ent, 0 );
		//	break;
		//case MOVETYPE_TOSSSLIDE:
		//	G_BoxSlideMove( ent, ent->r.clipmask ? ent->r.clipmask : MASK_PLAYERSOLID, 1.01f, 10 );
		//	break;
		//default:
		
	}
}
