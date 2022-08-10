/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.


// Base Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseSkeletalAnimator.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

#include "Game/Shared/Physics/Physics.h"
#include "Game/Shared/Physics/RootMotionMove.h"

// Base Slide Monster.
#include "SVGBaseRootMotionMonster.h"

// Game World.
#include "../../World/ServerGameWorld.h"

#define STEPSIZE 18

//! Constructor/Destructor.
SVGBaseRootMotionMonster::SVGBaseRootMotionMonster(PODEntity *svEntity) : Base(svEntity) { }


/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseRootMotionMonster::Precache() { 
	Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseRootMotionMonster::Spawn() { 
	// Base Spawn.
	Base::Spawn();

	// Use an Octagon Shaped Hull by default. Allows for more realistic character sliding.
    SetSolid( Solid::OctagonBox );
	// Set move type.
    SetMoveType( MoveType::RootMotionMove );
	// Notify the server this is, specifically a monster, by adding the Monster flag.
    SetServerFlags( EntityServerFlags::Monster );
	// Set clip mask to Monster and Player solid.
	// (Clip to Monster specific brushes, and those that players clip to as well.)
    SetClipMask( BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid );

	// Entity is alive.
	SetDeadFlag( DeadFlags::Alive );
    // Set entity to allow taking damage.
    SetTakeDamage( TakeDamage::Yes );

    // Set default values in case we have none.
    if (!GetMass()) { SetMass( 200 ); }
    if (!GetHealth()) { SetHealth( 200 ); }
}

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseRootMotionMonster::Think() { 
	// Base think.
	Base::Think();
}

/**
*   @brief 
**/
void SVGBaseRootMotionMonster::SpawnKey(const std::string& key, const std::string& value) { 
	Base::SpawnKey(key, value); 
}

/***
*
*
*	Animation Functionality:
*
*
***/
/**
*	@return	A pointer to the current animation state.
**/
const EntityAnimationState* SVGBaseRootMotionMonster::GetCurrentAnimationState() {
	return &podEntity->currentState.currentAnimation;
}
/**
*	@return	Calculates a 0 starting index based current frame for the given
*			animation state.
**/
const int32_t SVGBaseRootMotionMonster::GetAnimationStateRelativeFrame( const EntityAnimationState* animationState ) {
	// Return 0 in case there is no valid state data.
	if ( !animationState ) {
		return 0;
	}

	// The actual relative frame we're in.
	int32_t actionRelativeFrame = animationState->frame;

	// The animation runs by the 'dominant' blend action, so we calculate the actual 'relative' end frame,
	// since IQM stores all animations in 1 continuous array of frames.
	const int32_t actionRelativeEndFrame = animationState->endFrame - animationState->startFrame;

	// Ensure if it is -1, it means the action's animation has ended so we want to remain in the end frame for this moment in time.
	if (actionRelativeFrame == -1) {
		actionRelativeFrame = actionRelativeEndFrame;
	}

	// Subtract from current frame to get and return the 0 start index based frame number.
	return Clampf( actionRelativeFrame - animationState->startFrame, 0, animationState->endFrame );
}

/**
*	@brief	Sets the 'translate' vector to the value of the 'root bone's' requested frame number 
*			translation
*	@return	True if the 'translate' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionMonster::GetActionFrameTranslate( const int32_t actionIndex, const int32_t actionFrame, vec3_t& rootBoneTranslation ) {
	// Get animation, if failing to do so reset animation switching to none.
	SkeletalAnimationAction *action = GetAction( actionIndex );
	if ( !action ) {
		return false;
	}

	// We assume the pointer is not tempered with.
	auto &frameTranslates = action ->frameTranslates;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( actionFrame < 0 || actionFrame >= frameTranslates.size() ) {
		// Can't find Translation data.
		rootBoneTranslation = vec3_zero();
		// Failure.
		return false;
	} else {
		rootBoneTranslation = frameTranslates[ actionFrame ];

		// See if there are any specific rootBoneAxisFlags set.
		const int32_t rootBoneAxisFlags = action->rootBoneAxisFlags;

		// Zero out X Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation) ) {
			rootBoneTranslation.x = 0.0;
		}
		// Zero out Y Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation) ) {
			rootBoneTranslation.y = 0.0;
		}
		// Zero out Z Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation) ) {
			rootBoneTranslation.z = 0.0;
		}

		// Success.
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionMonster::GetActionFrameTranslate( const std::string &actionName, const int32_t actionFrame, vec3_t& rootBoneTranslation ) {
	// Need a valid skm.
	if ( !skm ) {
		return false;
	}

	// See if the action data exists.
	if ( !skm->actionMap.contains( actionName ) ) {
		return false;
	}

	return GetActionFrameTranslate( skm->actionMap[ actionName ].index, actionFrame, rootBoneTranslation);
}
/**
*	@brief	Sets the 'distance' double to the value of the 'root bones' requested frame number 
*			translation distance. (vec3_dlength)
*	@return	True if the 'distance' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionMonster::GetActionFrameDistance( const int32_t actionIndex, const int32_t actionFrame, double &rootBoneDistance ) {
	// Get animation, if failing to do so reset animation switching to none.
	SkeletalAnimationAction *action = GetAction( actionIndex );
	if ( !action ) {
		return false;
	}

	// We assume the pointer is not tempered with.
	auto &frameDistances = action->frameDistances;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( actionFrame < 0 || actionFrame >= frameDistances.size() ) {
		// TODO: gi.DPrintf(..)
		rootBoneDistance = 0.0;
		return false;
	} else {
		rootBoneDistance = frameDistances[actionFrame];
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionMonster::GetActionFrameDistance( const std::string &actionName, const int32_t actionFrame, double &rootBoneDistance ) {
	// Need a valid skm.
	if ( !skm ) {
		return false;
	}

	// See if the action data exists.
	if ( !skm->actionMap.contains( actionName ) ) {
		return false;
	}

	return GetActionFrameDistance( skm->actionMap[ actionName ].index, actionFrame, rootBoneDistance );
}

/**
*	@brief	Calculated the move speed of the root bone for the given 'totalMoveDistance' and moveTranslate.
*	@return	Value of the calculated move speed.
**/
const double SVGBaseRootMotionMonster::GetMoveSpeedForTraversedFrameDistance(const double &totalMoveDistance, const float &frameMoveDistance, const double &unitScale) {
	// Calculate move distance scaled to Quake units.
	const double scaledMoveDistance = unitScale * frameMoveDistance; //moveDistance * unitScale;

	// Length of move translate.
	const double moveFrameMoveDistance = frameMoveDistance; //vec3_dlength(moveTranslate);

	// Calculate move speed.
	if (scaledMoveDistance == 0) {
		//return 0;
	} else if (moveFrameMoveDistance == 0) {
		return 0;
	}
	return scaledMoveDistance / moveFrameMoveDistance;
}


const bool SVGBaseRootMotionMonster::HasExoticMoveResults( const int32_t resultsMask ) {
	const int32_t nonExoticStatesMask = RootMotionMoveResult::Moved | RootMotionMoveResult::PlaneTouched | RootMotionMoveResult::WallBlocked | RootMotionMoveResult::EntityTouched;

	if ( (currentMoveResultMask & nonExoticStatesMask) != 0 ) {
		return false;
	}

	return true;
}

/**
*	@brief	For future filling.
**/
const bool SVGBaseRootMotionMonster::AnimationFinished( const EntityAnimationState *animationState ) {
	return Base::AnimationFinished( animationState );
}

/**
*	@brief	For future filling.
**/
const bool SVGBaseRootMotionMonster::CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex ) {
	return Base::CanSwitchAnimation( animationState, wishedAnimationIndex );
}

/**
*	@brief	Can be overridden to add custom Monster animation change inspection.
*			When doing so, call Base::RefreshAnimation in case you want to resort
*			to so called 'basic behavior', explained blow.
*
*			The animation's are determined by te moveResultMasks. We inspect them for
*			steps, falling, and if there are steps ahead in order to keep on playing
*			a step up or down animation.
*
*			When none of the above is happening, we check to see if an animation has
*			finished playing so we can resort to our standard walk/run animation instead.
*			
**/
void SVGBaseRootMotionMonster::RefreshAnimationState() {

}



/***
*
*	Monster Entity Functions.
*
***/
/**
*	@brief	Rotates/Turns the monster into the Ideal Yaw angle direction.
*	@return	The delta yaw angles of this Turn.
**/
float SVGBaseRootMotionMonster::TurnToIdealYawAngle() {
	// Get current(and to be, previous) Angles.
	const vec3_t _previousAngles = GetAngles();

	// Angle Mod the current angles and compare to Ideal Yaw angle.
	float _currentYawAngle = _previousAngles[vec3_t::Yaw];
	float _idealYawAngle = GetIdealYawAngle();
		
	if ( _currentYawAngle == _idealYawAngle) {
		return 0.f;
	}

	// We're not at ideal yaw angle yet, so we'll calculate how
	// far to move it, and at what speed.
	float _yawMove = _idealYawAngle - _currentYawAngle;
	float _yawTurningSpeed = GetYawSpeed();

	if ( _idealYawAngle > _currentYawAngle) {
		if ( _yawMove >= 180.f) {
			_yawMove = _yawMove - 360.f;
		}
	} else {
		if ( _yawMove <= -180.f ) {
			_yawMove = _yawMove + 360.f;
		}
	}
	if ( _yawMove > 0.f ) {
		if ( _yawMove > _yawTurningSpeed ) {
			_yawMove = _yawTurningSpeed;
		}
	} else {
		if ( _yawMove < -_yawTurningSpeed ) {
			_yawMove = - _yawTurningSpeed;
		}
	}

	// Set the new angles, Angle Modding the Yaw.
	SetAngles( { _previousAngles.x, AngleMod(_currentYawAngle + _yawMove * (float)FRAMETIME_S.count() * _yawTurningSpeed), _previousAngles.z});

	// Return delta angles.
	return AngleMod(GetAngles()[vec3_t::Yaw]) - GetIdealYawAngle();
}



/***
*
*	Slide Movement Functions:
*
***/
/**
*	@brief	Performs a basic RootMotionMove by setting up a RootMotionMoveState and calling into
*			RootMotionMove physics.
*			
*			It'll try and step down, as well as step up stairs. If it's non steppable,
*			it resorts to sliding along the edge "crease".
**/
const int32_t SVGBaseRootMotionMonster::PerformRootMotionMove() {
	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

	// Get Entity Flags.
	const int32_t entityFlags = GetFlags();
	// Get WaterLevel.
	const int32_t waterLevel = GetWaterLevel();
    // Default mask is solid.
    const int32_t moveClipMask = GetClipMask();
    // Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( currentMoveState.groundEntityNumber != -1 ? true : false );
    // Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;


	/**
	*	Step #0:	- Check and clamp our Velocity.
	*				- Apply Rotation Friction to Angular Velocity.
	*				- Apply Ground Friction to Velocity
	*				- Apply Gravity:
	*					* For Walking Monsters	: Gravity if not on ground, ground friction otherwise.
	*					* For Swimming Monsters	: Gravity if not in water (Try and fall into water.)
	*					* For Flying Monsters	: ...
	**/
	const float oldVelocityLength = vec3_length( GetVelocity() );

	// Bound our velocity within sv_maxvelocity limits.
	SG_CheckVelocity( this );

    // Get angular velocity for applying rotational friction.
    vec3_t angularVelocity = GetAngularVelocity();

	// If we got any angular velocity, apply friction.
    if (angularVelocity.x || angularVelocity.y || angularVelocity.z) {
		SG_AddRotationalFriction( this );
	}

	// - Walking Monsters:
	if ( !wasOnGround ) {
		// In case of: Walking Monsters:
		if ( !( entityFlags & EntityFlags::Fly ) && !( entityFlags & EntityFlags::Swim ) ) {
			// Set HitSound Playing to True in case the velocity was a downfall one.
            if ( GetVelocity().z < sv_gravity->value * -0.1f ) {
                hitSound = true;
            }

            // They don't fly, and if it ain't in any water... well, add gravity.
            if ( GetWaterLevel() == 0 ) {
                SG_AddGravity( this );
            }
		}
	} else {
		// TODO: Move elsewhere.
		static constexpr int32_t FRICTION = 10;
		SG_AddGroundFriction( this, FRICTION );
	}
	// - Flying Monsters:
    if ( ( entityFlags & EntityFlags::Fly ) ) {
		// Friction for Vertical Velocity.
		if ( ( GetVelocity().z != 0 ) ) {
			// Calculate: Speed, Control, Friction.
			const float speed = fabs( GetVelocity().z );
			const float control = speed < ROOTMOTION_MOVE_STOP_SPEED ? ROOTMOTION_MOVE_STOP_SPEED : speed;
			const float friction = ROOTMOTION_MOVE_GROUND_FRICTION / 3;

			// Calculate: new Speed.
			float newSpeed = speed - ( FRAMETIME_S.count() * control * friction );
			if ( newSpeed < 0 ) {
				newSpeed = 0;
			}
			newSpeed /= speed;

			// Calculate new velocity based on old velocity.
			const vec3_t velocity = GetVelocity();
			SetVelocity( { velocity.x, velocity.y, velocity.z * newSpeed } );
		}
	}
	// - Swimming Monsters:
	if ( ( entityFlags & EntityFlags::Swim ) ) {
		// Friction for swimming monsters that have been given vertical velocity
		if ( ( entityFlags & EntityFlags::Swim ) && ( GetVelocity().z != 0 ) ) {
			const float speed = fabs( GetVelocity().z );
			const float control = speed < ROOTMOTION_MOVE_STOP_SPEED ? ROOTMOTION_MOVE_STOP_SPEED : speed;
			float newSpeed = speed - ( FRAMETIME_S.count() * control * ROOTMOTION_MOVE_WATER_FRICTION * GetWaterLevel() );
			if (newSpeed < 0) {
				newSpeed = 0;
			}
			newSpeed /= speed;
			const vec3_t velocity = GetVelocity();
			SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
		}
    }
	//  /**
	//  *	@brief //      // Apply friction: Let dead NPCs who aren't completely onground slide.
	//  **/
	//  if ( geBoxSlide->GetVelocity().z || geBoxSlide->GetVelocity().y || geBoxSlide->GetVelocity().x ) {
	//      // Apply friction: Let dead NPCs who aren't completely onground slide.
	//      if ( ( wasOnGround ) || ( geBoxSlide->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) ) {
	//          if ( geBoxSlide->GetDeadFlag() == DeadFlags::Dead) {//!( geBoxSlide->GetHealth() <= 0.0 ) ) {
	//              vec3_t newVelocity = geBoxSlide->GetVelocity();
	//              const float speed = sqrtf( newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1] );
	//              if (speed) {
	//                  const float friction = ROOTMOTION_MOVE_GROUND_FRICTION;
	//                  const float control = speed < ROOTMOTION_MOVE_STOP_SPEED ? ROOTMOTION_MOVE_STOP_SPEED : speed;
	//                  float newSpeed = speed - FRAMETIME_S.count() * control * friction;
	//                  if (newSpeed < 0) {
	//                      newSpeed = 0;
	//              }
	//                  newSpeed /= speed;
	//                  newVelocity[0] *= newSpeed;
	//                  newVelocity[1] *= newSpeed;
	//                  // Set the velocity.
	//                  geBoxSlide->SetVelocity( newVelocity );
	//              }
	//          }
	//      }
	//  }


	/**
	*	Step #1:	- Get appropriate Clip Mask.
	*				- Try and perform our Root Motion Move: Including, if wished for, stepping down/up.
	*				- ?
	**/
	// Backup a copy of the current moveState.
	previousMoveState = currentMoveState;
	// Backup a copy of the current moveResultMask.
	previousMoveResultMask = currentMoveResultMask;

	// With the previous frame's move info that was still 'current' backed up,
	// it is time to perform another new move and make it our actual fresh and new 'current' state.
	currentMoveResultMask = SG_RootMotion_PerformMove( 
		this, 
		( moveClipMask ? moveClipMask : BrushContentsMask::PlayerSolid ), 
		ROOTMOTION_MOVE_CLIP_BOUNCE, 
		ROOTMOTION_MOVE_GROUND_FRICTION, 
		&currentMoveState 
	);

	// Debugging Output:
	#if defined(SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK) && (SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK == 1)
	{
		const int32_t entityNumber = GetNumber();

		// Only output if the spawnflag is set to do so.
		if ( (GetSpawnFlags() & 128) ) {
			if ( previousMoveResultMask != currentMoveResultMask ) {
				DebugPrint(entityNumber, currentMoveResultMask, previousMoveResultMask, currentMoveState.moveFlags, currentMoveState.moveFlagTime);
			}
		}
	}
	#endif
	// Ideally in a perfect world we'd never get trapped, but Quake and all its derivatives
	// are of course perfectly beautiful creatures from bottomless pits where no developer
	// should ever want to be found, dead... or alive.
	//
	// So... I present to you the following bold and ugly motherfucking hack:
	//if ( ( currentMoveResultMask & RootMotionMoveResult::Trapped) ) {
	//	// The reason we do this here is that even though we inspect for trapped inside
	//	// the RootMotionMove repeatedly, it performs on a sub-level. This statement catches
	//	// the worst of the worst situations and will resort to old origin and velocity.
	//	currentMoveState.origin		= previousMoveState.origin;
	//	currentMoveState.velocity	= previousMoveState.velocity;
	//}


	/**
	*	Step #2:	- The Move has been Performed: Update Entity Attributes.
	**/
	// Double validate ground entity at this moment in time.
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( currentMoveState.groundEntityNumber ) );
	// Update the entity's state with the resulting move state information.
	ApplyMoveState( &currentMoveState );
	// Link entity in.
	LinkEntity();

	/**
	*	Step #3:	- Execute Touch Callbacks in case we add any blockedMask set.
	**/
	// Execute touch callbacks.
	if( currentMoveResultMask != 0 ) {
		GameEntity *otherEntity = nullptr;

		// Call Touch Triggers on our slide box entity for its new position.
		SG_TouchTriggers( this );

		// Dispatch 'Touch' callback functions to all touched entities we caught and stored in our moveState.
		for( int32_t i = 0; i < currentMoveState.numTouchEntities; i++ ) {
			otherEntity = gameWorld->GetGameEntityByIndex( currentMoveState.touchEntites[i] );
			
			// Don't touch projectiles.
			if( !otherEntity || !otherEntity->IsInUse() ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			// First dispatch a touch on the object we're hitting.
			otherEntity->DispatchTouchCallback( otherEntity, this, nullptr, nullptr );

			// Now dispatch a touch callback for THIS entity.
			DispatchTouchCallback( this, otherEntity, nullptr, nullptr );

			// In case touch callbacks caused it to be non 'in-use':
			if( !IsInUse() ) {
				break; // Break here.
			}
		}
	}

	/**
	*	Step #5:	- If still in use, check for ground, and see if our velocity came to a halt
	*				so we can safely trigger a Stop Dispatch callback.
	**/
	// Assuming we're still in use, set ourselves to a halt if 
	if ( IsInUse() ) {
		// Check for ground entity.
		int32_t groundEntityNumber = currentMoveState.groundEntityNumber;

		// Revalidate it
		GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( groundEntityNumber ) );

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geNewGroundEntity && vec3_length( GetVelocity() ) <= 0.0001f && oldVelocityLength > 1.f ) {
			// Zero out velocities.
			SetVelocity( vec3_zero() );
			SetAngularVelocity( vec3_zero() );

			// Stop.
			DispatchStopCallback( );
		}
	}
	// Execute touch triggers (Since entities might move during touch callbacks, we might've
	// hit new entities.)
    SG_TouchTriggers( this );

    // Can't continue if this entity wasn't in use.
    if ( !IsInUse( ) ) {
        return currentMoveResultMask;
	}

	/**
	*	#6:	In case we need to play any audio for this entity, do so.
	**/
    // Check for whether to play a land sound.
    if ( geNewGroundEntity ) {
        if ( !wasOnGround ) {
            if ( hitSound ) {
                SVG_Sound(this, 0, gi.PrecacheSound("world/land.wav"), 1, 1, 0);
            }
        }
    }

	/**
	*	#7: We're done, return result mask.
	**/
	return currentMoveResultMask;
}

/**
*	@brief	Applies the state data (origin, velocity, etc) to this entity's state.
**/
void SVGBaseRootMotionMonster::ApplyMoveState(RootMotionMoveState* moveState) {
	if (!moveState) {
		// TODO: gi.DPrintf
		return;
	}

	// Double validate ground entity at this moment in time.
	ServerGameWorld *gameWorld = GetGameWorld();
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( moveState->groundEntityNumber ) );

	// Update the entity with the resulting moveState values.
	SetOrigin( moveState->origin );
	SetVelocity( moveState->velocity );
	SetMins( moveState->mins );
	SetMaxs( moveState->maxs );
	SetFlags( moveState->entityFlags );
	SetWaterLevel( moveState->waterLevel );
	SetWaterType( moveState->waterType );

	if (moveState->groundEntityNumber != -1 && geNewGroundEntity) {
		SetGroundEntity( geNewGroundEntity );
		SetGroundEntityLinkCount( geNewGroundEntity->GetLinkCount() );
	} else {
		SetGroundEntity( SGEntityHandle() );
		SetGroundEntityLinkCount( 0 );
	}
}

/**
*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
**/
void SVGBaseRootMotionMonster::DebugPrint(const int32_t entityNumber, const int32_t resultMask, const int32_t previousResultMask, const int32_t moveFlags, const int32_t moveFlagTime) {
#if defined(SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK) && SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK == 1
	// Debug Output Strings.
	std::string resultMaskStr = "";
	std::string moveFlagsStr = "";

	// For debugging purposes we have "fictional" states. It makes the debug output easier 
	// to readsince we then know where it started doing something:
	// (moving up a step, or down an edge/step, or falling).
	const std::string stateSeparator = "[ Engaged Fictional State: "; // + " StepDown"/+ " StepUp"/+ "StepEdge"
	std::string stateSeparatorName = "Nameless"; // + " StepDown"/+ " StepUp"/+ "StepEdge"
	const std::string stateSeparatorEnd = " ================================================== ]";

	/**
	*	BlockedMaskStr.
	**/
	// Fill up the blockMaskStr with matching stringified flag names.
	if ( resultMask & RootMotionMoveResult::Moved ) { resultMaskStr += "Moved "; }
	
	if ( resultMask & RootMotionMoveResult::SteppedUp ) { resultMaskStr += "SteppedUp "; }
	if ( resultMask & RootMotionMoveResult::SteppedDown ) { resultMaskStr += "SteppedDown "; }
	if ( resultMask & RootMotionMoveResult::SteppedEdge ) { resultMaskStr += "SteppedEdge "; }
	if ( resultMask & RootMotionMoveResult::SteppedFall ) { resultMaskStr += "SteppedFall "; }

	if ( resultMask & RootMotionMoveResult::StepUpAhead ) { resultMaskStr += "StepUpAhead "; }
	if ( resultMask & RootMotionMoveResult::StepDownAhead ) { resultMaskStr += "StepDownAhead "; }
	if ( resultMask & RootMotionMoveResult::StepEdgeAhead ) { resultMaskStr += "StepEdgeAhead "; }

	if ( resultMask & RootMotionMoveResult::FallingDown ) { resultMaskStr += "FallingDown "; }

	if ( resultMask & RootMotionMoveResult::EntityTouched ) { resultMaskStr += "EntityTouched "; }
	if ( resultMask & RootMotionMoveResult::PlaneTouched ) { resultMaskStr += "PlaneTouched "; }
	if ( resultMask & RootMotionMoveResult::WallBlocked ) { resultMaskStr += "WallBlocked "; }
	if ( resultMask & RootMotionMoveResult::Trapped ) { resultMaskStr += "Trapped"; }

	/**
	*	MoveFlags.
	**/
	// Fill up the movedFlagStr with matching stringified flag names.
	if ( moveFlags & RootMotionMoveFlags::FoundGround ) { moveFlagsStr += "FoundGround "; }
	if ( moveFlags & RootMotionMoveFlags::OnGround ) { moveFlagsStr += "OnGround "; }
	if ( moveFlags & RootMotionMoveFlags::LostGround ) { moveFlagsStr += "LostGround "; }

	if ( moveFlags & RootMotionMoveFlags::Ducked ) { moveFlagsStr += "Ducked "; }
	if ( moveFlags & RootMotionMoveFlags::Jumped ) { moveFlagsStr += "Jumped "; }

	if ( moveFlags & RootMotionMoveFlags::OnLadder ) { moveFlagsStr += "OnLadder "; }
	if ( moveFlags & RootMotionMoveFlags::UnderWater ) { moveFlagsStr += "UnderWater "; }

	if ( moveFlags & RootMotionMoveFlags::TimePushed ) { moveFlagsStr += "TimePushed "; }
	if ( moveFlags & RootMotionMoveFlags::TimeWaterJump ) { moveFlagsStr += "TimeWaterJump "; }
	if ( moveFlags & RootMotionMoveFlags::TimeLand ) { moveFlagsStr += "TimeLand"; }


	/**
	*	Print info!
	**/
	std::string debugStr = "RootMotionMove(#";
	debugStr += std::to_string( entityNumber );
	debugStr += ") Result( ";
	debugStr += resultMaskStr;
	debugStr += ") Flags( ";
	debugStr += moveFlagsStr;
	debugStr += ") FlagTime(";
	debugStr += std::to_string(moveFlagTime);
	debugStr += ")\n";//(MoveFlags: % s) (MoveFlagTime: % i)\n";

	if (
		( ( resultMask & RootMotionMoveResult::SteppedUp ) && !( resultMask & RootMotionMoveResult::StepUpAhead ) )
		|| ( ( resultMask & RootMotionMoveResult::SteppedDown ) && !( resultMask & RootMotionMoveResult::StepDownAhead ) ) 
		|| ( ( resultMask & RootMotionMoveResult::SteppedEdge ) && !( resultMask & RootMotionMoveResult::StepEdgeAhead ) ) 
		|| ( ( resultMask & RootMotionMoveResult::SteppedFall ) /*&& !( resultMask & RootMotionMoveResult::StepFallAhead )*/ ) 
	) {
		gi.DPrintf("---------------------\n");
	}
	gi.DPrintf(debugStr.c_str());
#endif
}



/***
*
*
*
*	Navigation Functionality:
*
*
***/
/**
*	@brief	Navigates the monster down a path into its destined origin. Stepping down, up 
*			and obstacles if needed.
*	@return	The resulting final move's resultflags ( RootMotionMoveResult::xxx )
**/
const int32_t SVGBaseRootMotionMonster::NavigateToOrigin( const vec3_t &navigationOrigin ) {
	/**
	*	#0: Gather required data needed to process a frame for navigating.
	**/
	// Get the animation state data.
	const EntityAnimationState *animationState = GetCurrentAnimationState();
	// Get the animation index.
	const int32_t animationIndex = animationState->animationIndex;

	// Get pointers to our animation, actions, and finally the dominating blend action.
	SkeletalAnimation *animation = GetAnimation( animationIndex );
	SkeletalAnimationBlendAction *blendAction = GetBlendAction( animation, 0 );
	SkeletalAnimationAction *action = GetAction( blendAction->actionIndex );

	// If we are missing either of the pointers, fail out.
	if ( !animation || !blendAction || !action ) {
		return 0;
	}

	// Get action index.
	const int32_t actionIndex = action->index;

	// Get the animation frame we're at right now.
	// (We ensure it is within bounds to either 0 or endFrame when exceeding.)
	const int32_t animationFrame = GetAnimationStateRelativeFrame( animationState );

	const bool traverseZAxis = (
		!(GetFlags() & EntityFlags::Fly) && !(GetFlags() & EntityFlags::Swim)
	) ? true : false;

	/**
	*	#0: Calculate the direction to head into, and set the yaw angle and its turning speed.
	**/
	// Determine whether to enable Z Axis or not.
	const vec3_t vOriginalDirection = vec3_direction( navigationOrigin, GetOrigin() );//navigationOrigin - GetOrigin();
	
	// Calculate the desired final move direction. (Ignore Z Axis if need be.)
	const vec3_t vNavigateDirection = { 
		vOriginalDirection.x, 
		vOriginalDirection.y,
		( traverseZAxis == false ? vOriginalDirection.z : 0.0f )
	};

	// Prepare ideal yaw angle to rotate to.
	SetIdealYawAngle( vec3_euler( vOriginalDirection ).y );


	/**
	*	#1: Turn to the ideal yaw angle, and calculate our move velocity.
	*		If the yaw angle is too large, slow down the velocity.
	**/		
	// Get the delta between wished for and current yaw angles.
	const float deltaYawAngle = TurnToIdealYawAngle( );


	/**
	*	#2: Gather needed animation state info.
	**/
	// Get the 'root bone' translation offset for this move's animation frame.
	vec3_t frameTranslate = vec3_zero();
	const bool frameHasTranslate = GetActionFrameTranslate( actionIndex, animationFrame, frameTranslate );

	// Get the total move distance (vec3_length of a - b) for this animation move frame.
	double frameDistance = 0.0;
	const bool frameHasDistance	= GetActionFrameDistance( actionIndex, animationFrame, frameDistance );

	// Calculate the actual move speed based for the current animation frame.
	const double totalTraversedDistance = action->animationDistance;

	// Calculate the Unit Scale based on FRAMETIME_S * 8 units = 1 pixel.
	//static constexpr double unitScale = BASE_FRAMETIME * 8.;
	// TODO: Neaten this up, this is a lame quick thing.
	SkeletalAnimationAction *runAction = GetAction( "RunForward" );
	double unitScale = BASE_FRAMETIME * 8.;
	if (runAction && runAction->index == animationIndex) {
		unitScale = BASE_FRAMETIME * 12.;
	}
	// END OF TODO.

	// Calculate frame move speed.
	const double frameMoveSpeed = GetMoveSpeedForTraversedFrameDistance( totalTraversedDistance, frameDistance, unitScale );


	/**
	*	#3: Calculate the new MoveVelocity to use for this frame.
	**/
	// Normalize our direction 
	const vec3_t vDirectionNormal = vNavigateDirection;

	// Calculate the move's distance into its heading navigation direction.
	const vec3_t vFrameMoveDistance		= vec3_t { 
		(float)( frameMoveSpeed ), 
		(float)( frameMoveSpeed ), 
		(float)( frameMoveSpeed )
	} * vNavigateDirection;

	// Get the frame's translation without the Z axis.
	const vec3_t vFrameTranslate		= vec3_t { frameTranslate.x, frameTranslate.y, 0.f };
	// Get frame translate move direction.
	const vec3_t vFrameTranslateDir		= vec3_normalize( ( vFrameTranslate ) );
	
	// Calculate the frame's final moveDistance for its translation directory.
	const vec3_t vFrameMoveTranslate	= vFrameTranslate * vFrameTranslateDir;
	
	// Now calculate our final move velocity.
	vec3_t moveVelocity = vFrameMoveTranslate + vFrameMoveDistance;//vec3_zero();

	if (animationIndex == skm->animationMap["TPose"].index
		|| animationIndex == skm->animationMap["Idle"].index 
		|| animationIndex == skm->animationMap["IdleReload"].index
		|| animationIndex == skm->animationMap["IdleRifleAim"].index 
		|| animationIndex == skm->animationMap["IdleRifleFire"].index 
		|| animationIndex == skm->animationMap["WalkingToDying"].index 
		
	) {
		moveVelocity = vec3_zero();
	}

	/**
	*	#4: (Apply Gravity here for now...) Let's begin performing our root motion movement.
	**/
	// Get the current velocity stored as "oldVelocity".
	const vec3_t oldVelocity = GetVelocity();

	// Old Velocity for Z Axis (Maintain darn gravity)
	moveVelocity.z = oldVelocity.z;

	// And let's go!.
	SetVelocity(moveVelocity);

	return PerformRootMotionMove();
}