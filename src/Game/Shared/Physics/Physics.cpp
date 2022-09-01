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

/*
* SG_ClipVelocity
*/
vec3_t SG_ClipVelocity( const vec3_t &inVelocity, const vec3_t &normal, float overbounce ) {
	float backoff = vec3_dot( inVelocity, normal );

	if( backoff <= 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	// Calculate out velocity vector.
	vec3_t outVelocity = ( inVelocity - vec3_scale( normal, backoff ) );

	// SlideMove clamp it.
#if defined(SG_ROOTMOTION_MOVE_CLAMPING) && SG_ROOTMOTION_MOVE_CLAMPING == 1
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
void SG_AddGroundFriction( GameEntity *geGroundFriction, const float friction ) {
	if (!geGroundFriction) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): can't add ground friction, geGroundFriction == (nullptr)!\n", __func__, sharedModuleName ) );
		return;
	}

	vec3_t newVelocity = geGroundFriction->GetVelocity();
	const float speed = sqrtf( newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1] );
	if (speed) {
		const float friction = ROOTMOTION_MOVE_GROUND_FRICTION;
		const float control = speed < ROOTMOTION_MOVE_STOP_SPEED ? ROOTMOTION_MOVE_STOP_SPEED : speed;
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
		geCheck->SetGroundEntityLinkCount(traceResult.gameEntity ? traceResult.gameEntity->GetGroundEntityLinkCount() : 0); //ent->groundentity_linkcount = ent->groundentity->linkcount;
		
		// Since we hit ground, zero out the Z velocity in case it is lower than 0.
		vec3_t geCheckVelocity = geCheck->GetVelocity();
		if( geCheckVelocity.z < 0) {
			geCheckVelocity.z = 0;
			geCheck->SetVelocity(geCheckVelocity);
		}

		// Impact.
		SG_Impact( geCheck, traceResult );
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
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
#endif
#ifdef SHAREDGAME_SERVERGAME
	if (nextThinkTime <= GameTime::zero() || nextThinkTime > level.time) {
#endif
		return true;
    }

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
		case MoveType::SlideBoxMove: {
			GameEntity *geSlide = *entityHandle;
			SlideBoxMove slideBoxMove;
			int32_t geSlideContentMask = (geSlide && geSlide->GetClipMask() ? geSlide->GetClipMask() : BrushContentsMask::PlayerSolid);
			const float slideBounce = 1.01f;
			const float slideFriction = 10.f;

			SG_Physics_SlideBoxMove( geSlide, geSlideContentMask, slideBounce, slideFriction, &slideBoxMove );
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