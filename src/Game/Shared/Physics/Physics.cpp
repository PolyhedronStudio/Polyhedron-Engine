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

//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"

// Physics.
#include "Physics.h"
#include "RootMotionMove.h"
#include "SlideBox.h"

//#define SG_SLIDEBOX_DEBUG_TRAPPED
#define SG_VELOCITY_BOUNCE_CLAMPING
#define SG_VELOCITY_CLIP_CLAMPING

static constexpr float STOP_EPSILON = 0.1;

//static inline const bool IsGroundPlane( const CollisionPlane &plane, const vec3_t &gravityDir) {
//	return ( vec3_dot( plane.normal, gravityDir ) < -0.45f);
//}

/***
*
*
*	Velocity:
*
*
***/
/**
*	@brief	Bounce Velocity.
**/
const vec3_t SG_BounceVelocity( const vec3_t &in, const vec3_t &normal, float overBounce ) {
	// Calculate 'back off' factor.
	float backOff = vec3_dot( in, normal );

	if( backOff <= 0 ) {
		backOff *= overBounce;
	} else {
		backOff /= overBounce;
	}

	// Change in velocity.
	const vec3_t change = normal * backOff;

#ifndef SG_VELOCITY_BOUNCE_CLAMPING
	// Our final bounce back velocity.
	return in - change;
#else
	vec3_t bounceVelocity = in - change;
	const float oldSpeed = vec3_length( in );
	const float newSpeed = vec3_length( bounceVelocity );
	if( newSpeed > oldSpeed ) {
		bounceVelocity = vec3_normalize( bounceVelocity );
		bounceVelocity = vec3_scale( bounceVelocity, oldSpeed );
	}
	return bounceVelocity;
#endif
}

/**
*	@brief	Clip Velocity.
**/
const vec3_t SG_ClipVelocity( const vec3_t &velocity, const vec3_t &normal ) {
	// Calculate 'back off' factor.
	const float backOff = vec3_dot( velocity, normal );
	
#ifndef SG_VELOCITY_CLIP_CLAMPING
	return velocity - ( normal * backOff );
#else
	vec3_t clippedVelocity = velocity - ( normal * backOff );
	const float oldSpeed = vec3_length( velocity );
	const float newSpeed = vec3_length( clippedVelocity );
	if( newSpeed > oldSpeed ) {
		clippedVelocity = vec3_normalize( clippedVelocity );
		clippedVelocity = vec3_scale( clippedVelocity, oldSpeed );
	}
	return clippedVelocity;
#endif
}

/**
*	@brief	Applies 'downward' gravity forces to the entity.
**/
void SG_AddGravity( GameEntity *sharedGameEntity ) {
    // Fetch velocity.
    vec3_t velocity = sharedGameEntity->GetVelocity();

    // Apply gravity.
    velocity.z += -sharedGameEntity->GetGravity() * sv_gravity->value * FRAMETIME_S.count();

    // Apply new velocity to entity.
    sharedGameEntity->SetVelocity(velocity);
}

/**
*	@brief	The rotational friction is NOT SET to geRotateFriction!
*	@return	The angular velocity of geRotateFriction after applying rotational friction.
**/
const vec3_t SG_CalculateRotationalFriction( GameEntity *geRotateFriction ) { 
    // Ensure it is a valid entity.
    if ( !geRotateFriction ) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return vec3_zero();
    }

    // Acquire the rotational velocity first.
    vec3_t angularVelocity = geRotateFriction->GetAngularVelocity();

    // Set angles in proper direction.
    geRotateFriction->SetAngles( vec3_fmaf( geRotateFriction->GetAngles(), FRAMETIME_S.count(), angularVelocity ) );

    // Calculate adjustment to apply.
    const float adjustment = FRAMETIME_S.count() * ROOTMOTION_MOVE_STOP_SPEED * ROOTMOTION_MOVE_GROUND_FRICTION;

    // Apply adjustments.
    angularVelocity = geRotateFriction->GetAngularVelocity();
    for (int32_t n = 0; n < 3; n++) {
        if (angularVelocity[n] > 0) {
            angularVelocity[n] -= adjustment;
            if (angularVelocity[n] < 0)
                angularVelocity[n] = 0;
        } else {
            angularVelocity[n] += adjustment;
            if (angularVelocity[n] > 0)
                angularVelocity[n] = 0;
        }
    }

	// Return the angular velocity.
	return angularVelocity;
}

/**
*	@brief	Apply ground friction forces to entity.
**/
void SG_AddGroundFriction( GameEntity *geGroundFriction, const float friction, const float stopSpeed ) {
	if (!geGroundFriction) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): can't add ground friction, geGroundFriction == (nullptr)!\n", __func__, sharedModuleName ) );
		return;
	}

	vec3_t newVelocity = geGroundFriction->GetVelocity();
	const float speed = sqrtf( newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1] );
	if (speed) {
		//const float friction = ROOTMOTION_MOVE_GROUND_FRICTION;
		const float control = speed < stopSpeed ? stopSpeed : speed;
		float newSpeed = speed - FRAMETIME_S.count() * control * friction;
		if (newSpeed < 0) {
			newSpeed = 0;
		}
		newSpeed /= speed;
		newVelocity[0] *= newSpeed;
		newVelocity[1] *= newSpeed;
		// Set the velocity.
		geGroundFriction->SetVelocity( newVelocity );
	}

	//const vec3_t geVelocity = geGroundFriction->GetVelocity();
	//vec3_t groundVelocity = {
	//	geVelocity.x,
	//	geVelocity.y,
	//	0.f
	//};

	//// Calculate speed velocity based on ground velocity.
	//vec3_t frictionVec = vec3_zero();
	//const float speed = VectorNormalize2( groundVelocity, frictionVec );
	//if( speed ) {
	//	// Calculate ground friction speed.
	//	float fspeed = friction * FRAMETIME_S.count();
	//	if( fspeed > speed ) {
	//		fspeed = speed;
	//	}

	//	// Actual velocity.
	//	const vec3_t velocity = geGroundFriction->GetVelocity();

	//	// Update velocity.
	//	geGroundFriction->SetVelocity( vec3_fmaf( velocity, -fspeed, frictionVec ) );
	//}
}

/**
*	@return	The proper Solid mask to use for the passed game entity.
**/
int32_t SG_SolidMaskForGameEntity( GameEntity *gameEntity ) {
	if (!gameEntity) {
		return BrushContentsMask::Solid;
	}

	const int32_t geClipMask = gameEntity->GetClipMask();
	return ( geClipMask ? geClipMask : BrushContentsMask::Solid );
}

/**
*	@brief	Checks if this entity should have a groundEntity set or not.
**/
void SG_CheckGround( GameEntity *geCheck ) {
	// Actual offset to determine for when a Hull is on-ground.
	static constexpr float groundOffset = 0.25;

	// Sanity Check.
	if ( !geCheck ) {
		return;
	}
	
	// Get velocity.
	const vec3_t geCheckVelocity = geCheck->GetVelocity();

	// Check For: (EntityFlags::Swim | EntityFlags::Fly)
	// If an entity suddenly has the Swim, or Fly flag set, unset its ground entity
	// and return. It does not need GroundEntity behavior.
	if( geCheck->GetFlags() & ( EntityFlags::Swim | EntityFlags::Fly ) ) {
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// Check For: Client Entity.
	if( geCheck->GetClient() && geCheck->GetVelocity().z > 180 ) { // > 100
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
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

	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck) );

	// Check steepness.
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// Unset Ground Entity For Non Clients: When the trace result was not in a solid, and the velocity is > 1.
	if( ( geCheck->GetVelocity().z > 1 && !geCheck->GetClient()) && !traceResult.startSolid) {
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
		geCheck->SetGroundEntityLinkCount(0);
		return;
	}

	// If it did not start in a solid, and it's not stopping inside of a solid...
	if( !traceResult.startSolid && !traceResult.allSolid ) {
		// TODO: We don't always WANT to do this, so instead have a specific CheckGround that can be utilized
		// for monster use.
		//geCheck->SetOrigin( traceResult.endPosition );
		// Link entity.
		//geCheck->LinkEntity();
		
		// We must've hit some entity, so set it.
		geCheck->SetGroundEntity(traceResult.gameEntity);
		if ( traceResult.gameEntity ) {
			geCheck->SetGroundEntityLinkCount( traceResult.gameEntity->GetLinkCount() ); //ent->groundentity_linkcount = ent->groundentity->linkcount;
		} else {
			//geCheck->SetGroundEntityLinkCount( 0 );
		}

		// Since we hit ground, zero out the Z velocity in case it is lower than 0.
		if ( geCheck->GetVelocity().z < 0 ) {
			geCheck->SetVelocity( vec3_t { geCheckVelocity.x, geCheckVelocity.y, 0.f } );
		}

		// Impact.
		SG_Impact( geCheck, traceResult );
	}
}

/**
*	@brief	Checks whether the monster entity has ground, if so, snaps it to it and stops Z velocity.
**/
void SG_Monster_CheckGround( GameEntity *geCheck ) {
	// Actual offset to determine for when a Hull is on-ground.
	static constexpr float groundOffset = 0.25;

	// Sanity Check.
	if ( !geCheck ) {
		return;
	}
	
	// Get velocity.
	const vec3_t geCheckVelocity = geCheck->GetVelocity();

	// Check For: (EntityFlags::Swim | EntityFlags::Fly)
	// If an entity suddenly has the Swim, or Fly flag set, unset its ground entity
	// and return. It does not need GroundEntity behavior.
	if( geCheck->GetFlags() & ( EntityFlags::Swim | EntityFlags::Fly ) ) {
		return;
	}

	// Check For: Client Entity.
	if( geCheck->GetVelocity().z > 180 ) { // > 100
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
		return;
	}

	// If the hull point one-quarter unit down is solid the entity is on ground.
	const vec3_t geOrigin = geCheck->GetOrigin();
	vec3_t traceEndPoint = {
		geOrigin.x,
		geOrigin.y,
		geOrigin.z - groundOffset 
	};

	SGTraceResult traceResult = SG_Trace( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), traceEndPoint, geCheck, SG_SolidMaskForGameEntity(geCheck) );

	// Check steepness.
	if( !IsWalkablePlane( traceResult.plane ) && !traceResult.startSolid ) {
		geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
		return;
	}

	// If it did not start in a solid, and it's not stopping inside of a solid...
	if( !traceResult.startSolid && !traceResult.allSolid ) {
		// TODO: We don't always WANT to do this, so instead have a specific CheckGround that can be utilized
		// for monster use.
		geCheck->SetOrigin( traceResult.endPosition );
		
		// We must've hit some entity, so set it.
		geCheck->SetGroundEntity(traceResult.gameEntity);
		if ( traceResult.gameEntity ) {
			geCheck->SetGroundEntityLinkCount( traceResult.gameEntity->GetLinkCount() ); //ent->groundentity_linkcount = ent->groundentity->linkcount;
		}

		// Since we hit ground, zero out the Z velocity in case it is lower than 0.
		geCheck->SetVelocity( vec3_t { geCheckVelocity.x, geCheckVelocity.y, 0.f } );

		// Impact.
		//SG_Impact( geCheck, traceResult );
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
*	@brief	Keep entity velocity within bounds.
**/
void SG_BoundVelocity( GameEntity *geCheck ) {
	
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
const bool SG_RunThink( GameEntity *geThinker ) {
	if (!geThinker) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): Can't perform thinking routine, *geThinker == (nullptr)!\n", __func__, sharedModuleName ) );
		return true;
    }

    // Fetch think time.
    GameTime nextThinkTime = geThinker->GetNextThinkTime();

    // Should we think at all? 
    // Condition A: Below 0, aka -(1+) means no thinking.
    // Condition B: > level.time, means we're still waiting before we can think.
#ifdef SHAREDGAME_CLIENTGAME
	// For non extrapolating entities:
	if ( !geThinker->IsExtrapolating() && !geThinker->GetPODEntity()->linearMovement.isMoving ) {
		if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
			return true;
		}

	} else {
		//if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.extrapolatedTime + FRAMERATE_MS ) {
		if (nextThinkTime <= GameTime::zero() || nextThinkTime >= level.extrapolatedTime + FRAMERATE_MS ) {
		//if (nextThinkTime <= GameTime::zero() || nextThinkTime > GameTime( cl->serverTime ) + FRAMERATE_MS ) {
		//if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time + FRAMERATE_MS ) {
			return true;
		}
	}
#endif
#ifdef SHAREDGAME_SERVERGAME
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
		return true;
    }
#endif

    // Reset think time before thinking.
    geThinker->SetNextThinkTime( GameTime::zero() );

#if _DEBUG
    if ( !geThinker->HasThinkCallback() ) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): GameEntity(#{})[{}]: can't perform 'Think' because there is no callback set.\n", __func__, sharedModuleName, geThinker->GetNumber(), geThinker->GetTypeInfo()->mapClass ) );
    } else 
#endif
	{
		// Still call upon think, it does a check for a set callback on itself, but since Think
		// is a virtual method we want to make sure we call it regardless of.
		geThinker->Think();
	}

    return false;
}

void SG_RunEntity(SGEntityHandle &entityHandle) {
	// Get GameEntity from handle.
	if (!SGGameWorld::ValidateEntity(entityHandle)) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
		return;
    }

    GameEntity *ent = SGGameWorld::ValidateEntity( entityHandle );

    if (!ent) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return;
    }

    // Execute the proper physics that belong to its movetype.
    const int32_t moveType = ent->GetMoveType();

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
        case MoveType::StepMove:
            //SG_Physics_Step(entityHandle);
			//SG_Physics_None(entityHandle);
        break;
	// SG_Physics_RootMotionMove:
		case MoveType::RootMotionMove:
			SG_Physics_RootMotionMove(entityHandle);
			break;
	// SG_Physics_SlideBoxMove:
		case MoveType::TossSlideBox: {
			GameEntity *geSlide = *entityHandle;
			SlideBoxMove slideBoxMove;
			int32_t geSlideContentMask = (geSlide && geSlide->GetClipMask() ? geSlide->GetClipMask() : BrushContentsMask::PlayerSolid);
			const float slideBounce = 1.01f;
			const float slideFriction = 10.f;

			SG_Physics_TossSlideBox( geSlide, geSlideContentMask, slideBounce, slideFriction, &slideBoxMove );
			break;
		}
	// SG_Physics_Toss:
        case MoveType::Toss:
		case MoveType::TossSlide:
        case MoveType::Bounce:
        case MoveType::Fly:
        case MoveType::FlyMissile:
	        SG_Physics_Toss(entityHandle);
        break;
	default:
			SG_Error( ErrorType::Drop, fmt::format( "SG_RunEntity: Bad MoveType {}", moveType ) );//Error( "SV_Physics: bad movetype %i", (int)ent->movetype );
		break;
	}
}