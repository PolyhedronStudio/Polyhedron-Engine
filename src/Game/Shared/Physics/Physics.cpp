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
cvar_t *GetSVMaxVelocity() {
	return sv_maxvelocity;
}
cvar_t *GetSVGravity() {
	return sv_gravity;
}
void CheckSVCvars() {

}
#endif

#ifdef SHAREDGAME_CLIENTGAME
static cvar_t *sv_maxvelocity = nullptr;
static cvar_t *sv_gravity = nullptr;
cvar_t *GetSVMaxVelocity() {
	return sv_maxvelocity;
}
cvar_t *GetSVGravity() {
	return sv_gravity;
}
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

void SG_Physics_PrintWarning(const std::string& message) {
//void SG_Physics_PrintWarning(const std::string& message, const std::string &functionName = ) {
    // Only continue if developer warnings for physics are enabled.
    //extern cvar_t* dev_show_physwarnings;
    //if (!dev_show_physwarnings->integer) {
    //    return;
    //}

    // Show warning.
#ifdef SHAREDGAME_CLIENTGAME
	std::string warning = "SGPhysics(Client) Warning:";
#endif
#ifdef SHAREDGAME_SERVERGAME
	std::string warning =  "SGPhysics(Server) Warning:";
#endif
//    warning += functionName;
	warning += "]): ";
    warning += message;
    warning += "\n";

#ifdef SHAREDGAME_CLIENTGAME
	Com_DPrintf(warning.c_str());
#endif
#ifdef SHAREDGAME_SERVERGAME
	gi.DPrintf(warning.c_str());
#endif
}

void SG_Physics_PrintDeveloper(const std::string& message) {
//void SG_Physics_PrintWarning(const std::string& message, const std::string &functionName = ) {
    // Only continue if developer warnings for physics are enabled.
    //extern cvar_t* dev_show_physwarnings;
    //if (!dev_show_physwarnings->integer) {
    //    return;
    //}

    // Show warning.
#ifdef SHAREDGAME_CLIENTGAME
	std::string devmessage = "SGPhysics(Client) Developer: ";
#endif
#ifdef SHAREDGAME_SERVERGAME
	std::string devmessage =  "SGPhysics(Server) Developer: ";
#endif
//    warning += functionName;
    devmessage += message;
    devmessage += "\n";

#ifdef SHAREDGAME_CLIENTGAME
	Com_DPrintf(devmessage.c_str());
#endif
#ifdef SHAREDGAME_SERVERGAME
	gi.DPrintf(devmessage.c_str());
#endif
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
#if defined(SG_SLIDEMOVE_CLAMPING) && SG_SLIDEMOVE_CLAMPING == 1
	{
		float oldSpeed = vec3_length(inVelocity);
		float newSpeed = vec3_length(outVelocity);

		if (newSpeed > oldSpeed) {
			outVelocity = vec3_scale(vec3_normalize(outVelocity), oldSpeed);
		}
	}
	#endif

	return outVelocity;

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
void SG_AddGroundFriction( GameEntity *geGroundFriction, const float friction ) {
	if (!geGroundFriction) {
		SG_Physics_PrintWarning(std::string(__func__) + "called with geGroundFriction(nullptr)!");
	}

	const vec3_t geVelocity = geGroundFriction->GetVelocity();
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
		const vec3_t velocity = geGroundFriction->GetVelocity();

		// Update velocity.
		geGroundFriction->SetVelocity(vec3_fmaf(velocity, -fspeed, frictionVec));
	}
}

/**
*	@return	The proper Solid mask to use for the passed game entity.
**/
int32_t SG_SolidMaskForGameEntity( GameEntity *gameEntity ) {
	if (!gameEntity) {
		return BrushContentsMask::Solid;
	}

	const int32_t geClipMask = gameEntity->GetClipMask();
	return (geClipMask ? geClipMask : BrushContentsMask::Solid);
}

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
**/
void SG_CheckGround( GameEntity *geCheck ) {
	// Actual offset to determine for when a Hull is on-ground.
	static constexpr float groundOffset = 0.3125f;

	// Sanity Check.
	if (!geCheck) {
		return;
	}

	// Check For: (EntityFlags::Swim | EntityFlags::Fly)
	// If an entity suddenly has the Swim, or Fly flag set, unset its ground entity
	// and return. It does not need GroundEntity behavior.
	if( geCheck->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// Check For: Client Entity.
	if( geCheck->GetClient() && geCheck->GetVelocity().z > 180) { // > 100
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// If the hull point one-quarter unit down is solid the entity is on ground.
	const vec3_t geOrigin = geCheck->GetOrigin();
	vec3_t traceEndPoint = {
		geOrigin.x,
		geOrigin.y,
		geOrigin.z - groundOffset 
	};

	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck));

	// Check steepness.
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// Unset Ground Entity For Non Clients: When the trace result was not in a solid, and the velocity is > 1.
	if( ( geCheck->GetVelocity().z > 1 && !geCheck->GetClient()) && !traceResult.startSolid) {
		geCheck->SetGroundEntity( SGEntityHandle() );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// If it did not start in a solid, and it's not stopping inside of a solid...
	if( !traceResult.startSolid && !traceResult.allSolid ) {
		// We must've hit some entity, so set it.
		geCheck->SetGroundEntity(traceResult.gameEntity);
		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;
		
		// Since we hit ground, zero out the Z velocity in case it is lower than 0.
		vec3_t geCheckVelocity = geCheck->GetVelocity();
		if( geCheckVelocity.z < 0) {
			geCheckVelocity.z = 0;
			geCheck->SetVelocity(geCheckVelocity);
		}
	}
}


//================================================================================

// Types of entities:
// "PushMove":	Is used by world brush entities: func_button, func_door, func_plat, func_train, etc.
//				It does NOT obey gravity, and does not intersect with each other. All it does is push other entities, and damage them or stop, when blocked.
//
// "Toss":	OnGround is set when they come to a complete rest.
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
void SG_CheckVelocity( GameEntity *geCheck ) {
	
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



/**
*	@brief	Gives the entity a chance to process 'Think' callback logic if the
*			time is there for it to do so.
*	@return	True if it failed. Yeah, odd, I know, it was that way, it stays that way for now.
**/
qboolean SG_RunThink(GameEntity *geThinker) {
    if (!geThinker) {
	    //SVG_PhysicsEntityWPrint(__func__, "[start of]", "nullptr entity!");
        return true;
    }

    // Fetch think time.
    GameTime nextThinkTime = geThinker->GetNextThinkTime();

    // Should we think at all? 
    // Condition A: Below 0, aka -(1+) means no thinking.
    // Condition B: > level.time, means we're still waiting before we can think.
#ifdef SHAREDGAME_CLIENTGAME
	const auto nextFrameTime = (geThinker->GetPODEntity()->isLocal ? level.time : level.nextServerTime);
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > nextFrameTime ) { //nextThinkTime > level.nextServerTime) {
#endif
#ifdef SHAREDGAME_SERVERGAME
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
#endif
		return true;
    }

    // Reset think time before thinking.
    geThinker->SetNextThinkTime(GameTime::zero());

	if (!geThinker) {
	    SG_Physics_PrintWarning("*geThinker is (nullptr)!");
        return false;
    }

#if _DEBUG
    if ( !geThinker->HasThinkCallback() ) {
		SG_Physics_PrintWarning("GameEntity(#" + std::to_string(geThinker->GetNumber()) + ")[" + geThinker->GetTypeInfo()->mapClass + "] has no Think callback set.");
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
    //if (!entityHandle || !(*entityHandle) || !entityHandle.Get()) {
	if (!SGGameWorld::ValidateEntity(entityHandle)) {
        SG_Physics_PrintWarning( std::string(__func__) + "got an invalid entity handle!" );
		return;
    }

    GameEntity *ent = SGGameWorld::ValidateEntity( entityHandle );

    if (!ent) {
	    SG_Physics_PrintWarning( std::string(__func__) + "got an entity handle that still has a broken game entity ptr!" );
        return;
    }

    // Execute the proper physics that belong to its movetype.
    const int32_t moveType = ent->GetMoveType();

	//if( ISEVENTENTITY( &ent->s ) ) { // events do not think
	//	return;
	//}

	//if( ent->timeDelta && !( ent->r.svflags & SVF_PROJECTILE ) ) {
	//	G_Printf( "Warning: G_RunEntity 'Fixing timeDelta on non projectile entity" );
	//	ent->timeDelta = 0;
	//}

	//// only team captains decide the think, and they make think their team members when they do
	//if(ent && !( ent->GetFlags() & EntityFlags::TeamSlave)) {
	//	for (GameEntity* gePart = ent; gePart != nullptr; gePart = gePart->GetTeamChainEntity()) {
	//		SG_RunThink( gePart );
	//	}
	//	//for( GameEntity *gePart = ent; gePart ; gePart = (gePart ? gePart->GetTeamChainEntity() : nullptr)) {
	//	//	if (gePart) {
	//	//		SG_RunThink( gePart );
	//	//	}
	//	//}
	////}

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
			SG_Physics_None(entityHandle);
        break;
	// SG_Physics_BoxSlideMove:
		case MoveType::BoxSlideMove:
			SG_Physics_BoxSlideMove(entityHandle);
			break;
	// SG_Physics_Toss:
        case MoveType::Toss:
		case MoveType::TossSlide:
        case MoveType::Bounce:
        case MoveType::Fly:
        case MoveType::FlyMissile:
	        SG_Physics_Toss(entityHandle);
        break;
	// SG_BoxSlideMove:
		//case MoveType::TossSlide: {
		//	GameEntity *geTossSlider = *entityHandle;
		//	if (geTossSlider) {
		//		const int32_t geClipMask = geTossSlider->GetClipMask();
		//		SG_BoxSlideMove( ent, ( geClipMask ? geClipMask : BrushContentsMask::PlayerSolid ), 1.01f, 10 );
		//	}
		//break;
		//}
	default:
		//	G_Error( "SV_Physics: bad movetype %i", (int)ent->movetype );
		break;
	}
}

/**
*	@brief	The basic solid body movement clip that slides along multiple planes
*	@return	The clipflags if the velocity was modified (hit something solid)
*			1 = floor
*			2 = wall / step
*			4 = dead stop
**/
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

/**
*	@brief	Calls GS_SlideMove for the SharedGameEntity and triggers touch functions of touched entities.
**/
const int32_t SG_BoxSlideMove( GameEntity *geSlider, const int32_t contentMask, const float slideBounce, const float friction ) {
	int32_t i;
	MoveState entMove = {};
	int32_t blockedMask = 0;

	if (!geSlider) {
		SG_PhysicsEntityWPrint(__func__, "[start]", "geSlider is (nullptr)!");
		return 0;
	}

	// Store current velocity as old velocity.
	float oldVelocity = VectorLength( geSlider->GetVelocity() );

	// Apply gravitational force if no ground entity is set. Otherwise, apply ground friction forces.
	if( !geSlider->GetGroundEntityHandle() ) {
		SG_AddGravity( geSlider );
	} else { // horizontal friction
		SG_AddGroundFriction( geSlider, friction );
	}

	// Initialize our move state.
	entMove.numClipPlanes		= 0;
	entMove.numTouchEntities	= 0;

	// When the entity isn't idle, move its properties over into our move state.
	if( oldVelocity > 0 ) {
		// Setup remaining move time.
		Frametime remainingTime = level.time;
		entMove.remainingTime	= FRAMETIME.count();

		// Setup general properties.
		entMove.passEntity		= geSlider;
		entMove.contentMask		= contentMask;
		entMove.gravityDir		= vec3_t { 0.f, 0.f, -1.f };
		entMove.slideBounce		= slideBounce;
		entMove.groundEntity	= ( *geSlider->GetGroundEntityHandle() );

		// Setup Physical properties.
		entMove.origin			= geSlider->GetOrigin( );
		entMove.velocity		= geSlider->GetVelocity( );
		entMove.mins			= geSlider->GetMins( );
		entMove.maxs			= geSlider->GetMaxs( );

		// Execute actual slide movement.
		blockedMask = SG_SlideMove( &entMove );

		// If we got blocked, we'll check to see if we can step over it.
		if (blockedMask & SlideMoveFlags::WallBlocked) {

		}

		// Update our entity with the resulting values.
		geSlider->SetOrigin(entMove.origin);
		geSlider->SetVelocity(entMove.velocity);
		geSlider->SetGroundEntity(entMove.groundEntity);

		// Link entity in.
		geSlider->LinkEntity();
	}

	// Execute touch callbacks.
	if( contentMask != 0 ) {
		GameEntity *otherEntity = nullptr;
		//GClip_TouchTriggers( ent );
		SG_TouchTriggers(geSlider);

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

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geSlider->GetGroundEntityHandle() && vec3_length(geSlider->GetVelocity()) <= 1.f && oldVelocity > 1.f) {
			// Zero out velocities.
			geSlider->SetVelocity( vec3_zero() );
			geSlider->SetAngularVelocity( vec3_zero() );

			// Stop.
			geSlider->DispatchStopCallback( );
		}
	}

	return blockedMask;
}

#endif