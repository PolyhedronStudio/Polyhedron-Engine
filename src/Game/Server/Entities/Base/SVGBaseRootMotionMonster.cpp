/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
#include "../../ServerGameLocals.h"   // SVGame.
#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.


// Base Entities.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseSkeletalAnimator.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Slide Monster.
#include "SVGBaseRootMotionMonster.h"

// Game World.
#include "../World/ServerGameWorld.h"

#define STEPSIZE 18

//! Constructor/Destructor.
SVGBaseRootMotionMonster::SVGBaseRootMotionMonster(PODEntity *svEntity) : Base(svEntity) { }
SVGBaseRootMotionMonster::~SVGBaseRootMotionMonster() { }


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
*   @brief 
**/
void SVGBaseRootMotionMonster::PostSpawn() { 
	Base::Spawn(); 
}

/**
*   @brief 
**/
void SVGBaseRootMotionMonster::Respawn() { 
	Base::Respawn(); 
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
const int32_t SVGBaseRootMotionMonster::GetAnimationStateFrame( const EntityAnimationState* animationState ) {
	// Return 0 in case there is no valid state data.
	if ( !animationState ) {
		// TODO: gi.DPrintf("");
		return 0;
	}

	// Get animation start frame.
	const int32_t animationStartFrame = animationState->startFrame;
	// Get animation End frame.
	const int32_t animationEndFrame = animationState->endFrame - animationState->startFrame;
	// Get current animation frame.
	int32_t animationFrame = animationState->frame;

	// Ensure if it is -1, it means animation ended so we must be stuck in end frame.
	if (animationFrame == -1) {
		animationFrame = animationEndFrame;
	}

	// Subtract from current frame to get and return the 0 start index based frame number.
	return Clampf(animationState->frame - animationStartFrame, 0, animationEndFrame );
}

/**
*	@brief	Sets the 'translate' vector to the value of the 'root bone's' requested frame number 
*			translation
*	@return	True if the 'translate' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionMonster::GetAnimationFrameTranslate( const int32_t animationIndex, const int32_t animationFrame, vec3_t& rootBoneTranslation ) {
	// Need a valid skm.
	if ( !skm ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	// Check if the animation index is valid.
	if ( animationIndex < 0 || animationIndex >= skm->animations.size() ) {
		// TODO: gi.DPrintf("");
		return false;
	}
	
	// Get our current animation state.
	const EntityAnimationState *animationState = GetCurrentAnimationState();

	// It is valid, get a hold of the animation data.
	auto animationData = skm->animations[animationIndex];

	// We assume the pointer is not tempered with.
	auto &frameTranslates= animationData->frameTranslates;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( animationFrame < 0 || animationFrame >= frameTranslates.size() ) {
		// Can't find Translation data.
		rootBoneTranslation = vec3_zero();
		// Failure.
		return false;
	} else {
		rootBoneTranslation = frameTranslates[animationFrame];

		// See if there are any specific rootBoneAxisFlags set.
		const int32_t rootBoneAxisFlags = animationData->rootBoneAxisFlags;

		// Zero out X Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
			rootBoneTranslation.x = 0.0;
		}
		// Zero out Y Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
			rootBoneTranslation.y = 0.0;
		}
		// Zero out Z Axis.
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
			rootBoneTranslation.z = 0.0;
		}

		// Success.
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionMonster::GetAnimationFrameTranslate( const std::string &animationName, const int32_t animationFrame, vec3_t& rootBoneTranslation ) {
	// Need a valid skm.
	if ( !skm ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	// See if the animation data exists.
	if ( !skm->animationMap.contains( animationName) ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	return GetAnimationFrameTranslate( animationName, animationFrame, rootBoneTranslation );
}
/**
*	@brief	Sets the 'distance' double to the value of the 'root bones' requested frame number 
*			translation distance. (vec3_dlength)
*	@return	True if the 'distance' frame data exists. False otherwise.
**/
const bool SVGBaseRootMotionMonster::GetAnimationFrameDistance( const int32_t animationIndex, const int32_t animationFrame, double &rootBoneDistance ) {
	// Need a valid skm.
	if ( !skm ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	// Check if the animation index is valid.
	if ( animationIndex < 0 || animationIndex >= skm->animations.size() ) {
		// TODO: gi.DPrintf("");
		return false;
	}
	
	// Get our current animation state.
	const EntityAnimationState *animationState = GetCurrentAnimationState();

	// It is valid, get a hold of the animation data.
	auto *animationData = skm->animations[animationIndex];

	// We assume the pointer is not tempered with.
	auto &frameDistances = animationData->frameDistances;

	// Ensure the frame is within bounds of the pre-calculated translates.
	if ( animationFrame < 0 || animationFrame >= frameDistances.size() ) {
		// TODO: gi.DPrintf(..)
		rootBoneDistance = 0.0;
		return false;
	} else {
		rootBoneDistance = frameDistances[animationFrame];
		return true;
	}

	// Should not happen.
	return false;
}
const bool SVGBaseRootMotionMonster::GetAnimationFrameDistance( const std::string &animationName, const int32_t animationFrame, double &rootBoneDistance ) {
	// Need a valid skm.
	if ( !skm ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	// See if the animation data exists.
	if ( !skm->animationMap.contains( animationName) ) {
		// TODO: gi.DPrintf("");
		return false;
	}

	return GetAnimationFrameDistance( animationName, animationFrame, rootBoneDistance );
}

/**
*	@brief	Calculated the move speed of the root bone for the given 'totalMoveDistance' and moveTranslate.
*	@return	Value of the calculated move speed.
**/
const double SVGBaseRootMotionMonster::GetMoveSpeedForTraversedFrameDistance(const double &totalMoveDistance, const float &frameMoveDistance, const double &unitScale) {
	// Calculate move distance scaled to Quake units.
	const double scaledMoveDistance = totalMoveDistance * unitScale; //moveDistance * unitScale;

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


// TODO: Add in class or so, whatever, but not here.
// Also this function might just better accept an animationstate...
const bool SVGBaseRootMotionMonster::AnimationFinished( const EntityAnimationState *animationState ) {
	// Get the frame and end frame.
	const int32_t animationStartFrame	= animationState->endFrame;
	const int32_t animationEndFrame	= animationState->endFrame;
	const int32_t animationFrame	= animationState->frame;

	// We purposely do not do: animationFrame <= startFrame, because even
	// though -1 complies to this rule, if we are coming from an animation
	// that has a lesser frame index, we're screwed. Specifically
	// check for -1 it is. Don't fuck with that.
	if (animationFrame == -1 || animationFrame >= animationEndFrame) {
		return true;
	}

	// Did not end.
	return false;
}
// Returns true if we should switch. Does a check if the other animation has finished playing before actually performing the switch.
// It stores the animation to switch to, ensuring that next time we call it might give us a GO.
const bool SVGBaseRootMotionMonster::CanSwitchAnimation( const EntityAnimationState *animationState, const int32_t wishedAnimationIndex ) {
	if ( animationState->animationIndex != wishedAnimationIndex ) {
		if ( AnimationFinished( animationState ) ) {
			return true;
		}
	}

	return false;
}

// Returns -1 if we can't.
// Returns 0 if we have to wait (ie, we got steps ahead but we also stepped. )
// Returns 1 if we the state is clean.
const bool SVGBaseRootMotionMonster::HasExoticMoveResults( const int32_t resultsMask ) {
	/**
	*	#0: A walking state has to comply to either of the following masks:
	**/
//int mask = 8 | 12345;
//if (mask & bitmask == mask) {
////true if, and only if, bitmask contains 8 | 12345
//}
//
//if (mask & bitmask != 0) {
////true if bitmask contains 8 or 12345 or (8 | 12345)
//}

	//const int32_t maskA = RootMotionMoveResult::Moved;
	//const int32_t maskB = RootMotionMoveResult::Moved | RootMotionMoveResult::PlaneTouched;
	//const int32_t maskC = RootMotionMoveResult::Moved | RootMotionMoveResult::PlaneTouched | RootMotionMoveResult::WallBlocked;
	const int32_t nonExoticStatesMask = RootMotionMoveResult::Moved | RootMotionMoveResult::PlaneTouched | RootMotionMoveResult::WallBlocked | RootMotionMoveResult::EntityTouched;

	//if ( (currentMoveResultMask & maskA) ) {
	//	if ( (currentMoveResultMask & maskA) ) {
	//		if ( (currentMoveResultMask & maskA) ) {
				if ( (currentMoveResultMask & nonExoticStatesMask) != 0 ) {
					return false;
				}
	//		}
	//	}
	//}

	return true;
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
	//// Get current animation state.
	//const EntityAnimationState *currentAnimationState = GetCurrentAnimationState();
	//
	//// Re-explain: current = what we got
	//// new = what we set it to
	//// 
	//int32_t currentAnimationIndex	= currentAnimationState->animationIndex;
	//int32_t newAnimationIndex		= 0;
	//
	//// Get the frame and end frame.
	//const int32_t animationEndFrame	= currentAnimationState->endFrame;
	//const int32_t animationFrame	= currentAnimationState->frame;

	//// Are we coming from an animation that just finished last frame?
	//const bool animationFinished	= AnimationFinished( currentAnimationState );
	//// Gather animation indexes. TODO: Don't look these up constantly, that's bothersome.
	//// Walk.
	//const int32_t standardWalkIndex		= skm->animationMap["WalkForward"].index;
	//// Stairs Up
	//const int32_t walkStairsUpIndex		= skm->animationMap["PistolIdleTense"].index;	// Yes, I am aware it is atm named run_stairs_up, sue me.
	//// Stairs Down.
	//const int32_t walkStairsDownIndex	= skm->animationMap["Reload"].index;

	//// There's two things we need to take care of:
	////		- Animations end playing, when they do, they either hit its end frame or reached
	////		  a value of -1 due to it being time based. When this happens, we need to see
	////		  if we should replay the animation or resort to walking/running.
	////		- 
	////
	////
	////

	///**
	//*	#0:	Gather information about if we are stepping, and if there are any
	//*		steps ahead of us or not.
	//**/
	//// If we got no exotic move results, 
	//bool hasExoticResults = HasExoticMoveResults( currentMoveResultMask );
	//
	//// Get step information for previous move results.
	//bool previousStepUpAhead	= (previousMoveResultMask & RootMotionMoveResult::StepUpAhead);
	//bool previousSteppedUp		= (previousMoveResultMask & RootMotionMoveResult::SteppedUp);
	//bool previousStepDownAhead	= (previousMoveResultMask & RootMotionMoveResult::StepDownAhead);
	//bool previousSteppedDown	= (previousMoveResultMask & RootMotionMoveResult::SteppedDown);

	//// Get step information for current move results.
	//bool currentStepUpAhead		= (currentMoveResultMask & RootMotionMoveResult::StepUpAhead);
	//bool currentSteppedUp		= (currentMoveResultMask & RootMotionMoveResult::SteppedUp);
	//bool currentStepDownAhead	= (currentMoveResultMask & RootMotionMoveResult::StepDownAhead);
	//bool currentSteppedDown		= (currentMoveResultMask & RootMotionMoveResult::SteppedDown);

	//// Are we playing stair animations at all?
	//bool isAnimStairsDown	= ( currentAnimationIndex == walkStairsDownIndex ? true : false );
	//bool isAnimStairsUp		= ( currentAnimationIndex == walkStairsUpIndex ? true : false  );

	//// Are we playing walk animation?
	//bool isAnimWalking = ( currentAnimationIndex == standardWalkIndex ? true : false );

	///*if (currentAnimationIndex != standardWalkIndex) {
	//	currentAnimationIndex = SwitchAnimation( "WalkForward" );
	//}*/

	//// If we are playing walking animation.
	///**
	//*	#1: Stairs Down:
	//**/	
	//if ( ( currentStepDownAhead && currentSteppedDown ) ||
	//	currentStepDownAhead ) {
	//	// Only set it if:
	//	// - A: We got no new animation set yet.
	//	// - B: We aren't playing the animation already.
	//	if ( !newAnimationIndex && !isAnimStairsDown ) {
	//		newAnimationIndex = SwitchAnimation ("Reload" );
	//	}
	//}
	//// See if we are playing it, and if we are, whether we should prepare to cancel it.
	//else if ( isAnimStairsDown ) {
	//	 if ( !currentStepDownAhead && !currentSteppedDown ) {
	//		if ( !newAnimationIndex ) {
	//			if ( animationFinished && currentAnimationIndex != standardWalkIndex) {
	//				newAnimationIndex = SwitchAnimation("WalkForward");
	//			}
	//		}
	//	 }
	//}

	///**
	//*	#2: Stairs Up:
	//**/
	//// See if we should start playing stairs up.
	//if ( previousStepUpAhead && currentSteppedUp ) {
	//	// Only set it if:
	//	// - A: We got no new animation set yet.
	//	// - B: We aren't playing the animation already.
	//	if (!newAnimationIndex && !isAnimStairsUp) {
	//		newAnimationIndex = SwitchAnimation ("PistolIdleTense" );
	//	}
	//}
	//// See if we are playing it, and if we are, whether we should prepare to cancel it.
	//else if ( isAnimStairsUp ) {
	//	if ( !currentStepUpAhead && !currentSteppedUp ) {
	//		if (!newAnimationIndex && !isAnimWalking) {
	//			if (animationFinished && currentAnimationIndex != standardWalkIndex ) {
	//				newAnimationIndex = SwitchAnimation("WalkForward");
	//			}
	//		}
	//	}
	//}


	/**
	*	#2: Stairs Up:
	**/
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
	float _currentYawAngle = AngleMod( _previousAngles[vec3_t::Yaw] );
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
	SetAngles( { _previousAngles.x, AngleMod( _currentYawAngle + _yawMove * FRAMETIME.count() * _yawTurningSpeed ), _previousAngles.z});

	// Return delta angles.
	return GetAngles()[vec3_t::Yaw] - GetIdealYawAngle();
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
			float newSpeed = speed - ( FRAMETIME.count() * control * friction );
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
			float newSpeed = speed - ( FRAMETIME.count() * control * ROOTMOTION_MOVE_WATER_FRICTION * GetWaterLevel() );
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
	//                  float newSpeed = speed - FRAMETIME.count() * control * friction;
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
//	gi.DPrintf( debugStr.c_str(), resultMaskStr.c_str(), moveFlagsStr.c_str(), moveFlagTime );
#endif
}

void SVGBaseRootMotionMonster::RootMotionMove_FixCheckBottom() {
	SetFlags( GetFlags() | EntityFlags::PartiallyOnGround );
}

const bool SVGBaseRootMotionMonster::RootMotionMove_CheckBottom() {
	//vec3_t	mins, maxs, start, stop;
	SVGTraceResult trace;
	int32_t 		x, y;
	float	mid, bottom;

	const vec3_t origin = GetOrigin();
	vec3_t mins = origin + GetMins();
	vec3_t maxs = origin + GetMaxs();
	
// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
	vec3_t start, stop;

	start[2] = mins[2] - 1;
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (SG_PointContents (start) != BrushContents::Solid) {
				goto realcheck;
			}
		}

	return true;		// we got out easy

realcheck:
//
// check it for real...
//
	start[2] = mins[2];
	
// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
	stop[2] = start[2] - 2*STEPSIZE;
	trace = SVG_Trace (start, vec3_zero(), vec3_zero(), stop, this, BrushContentsMask::MonsterSolid);

	if (trace.fraction == 1.0) {
		return false;
	}
	mid = bottom = trace.endPosition[2];
	
// the corners must be within 16 of the midpoint	
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			
			trace = SVG_Trace (start, vec3_zero(), vec3_zero(), stop, this, BrushContentsMask::MonsterSolid);
			
			if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
				bottom = trace.endPosition[2];
			if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
				return false;
		}

	return true;
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
	// Get the animation frame we're at right now.
	// (We ensure it is within bounds to either 0 or endFrame when exceeding.)
	const int32_t animationFrame = GetAnimationStateFrame( animationState );

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
	SetIdealYawAngle( vec3_to_yaw( vOriginalDirection ) );


	/**
	*	#1: Turn to the ideal yaw angle, and calculate our move velocity.
	*		If the yaw angle is too large, slow down the velocity.
	**/		
	// Get the delta between wished for and current yaw angles.
	const float deltaYawAngle = TurnToIdealYawAngle( );

	//if ( !(animationIndex == skm->animationMap["WalkForwardRight"].index
	//	|| animationIndex == skm->animationMap["WalkForwardLeft"].index 
	//	|| animationIndex == skm->animationMap["WalkLeft"].index
	//	|| animationIndex == skm->animationMap["WalkRight"].index 
	//) ) {
		//deltaYawAngle = TurnToIdealYawAngle( );
//	}


	/**
	*	#2: Gather needed animation state info.
	**/
	// Get the 'root bone' translation offset for this move's animation frame.
	vec3_t frameTranslate = vec3_zero();
	const bool frameHasTranslate = GetAnimationFrameTranslate( animationIndex, animationFrame, frameTranslate );
	// Get the total move distance (vec3_length of a - b) for this animation move frame.
	double frameDistance = 0.0;
	const bool frameHasDistance	= GetAnimationFrameDistance( animationIndex, animationFrame, frameDistance );
	
	// Calculate the actual move speed based for the current animation frame.
	const double totalTraversedDistance = skm->animations[animationIndex]->animationDistance;

	float frameTimeThing = ANIMATION_FRAMETIME / 4;
	if (animationIndex == skm->animationMap["RunForward"].index) {
		frameTimeThing = ANIMATION_FRAMETIME;
	}
	const double frameMoveSpeed = GetMoveSpeedForTraversedFrameDistance( totalTraversedDistance, frameDistance, frameTimeThing);


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
	//vec3_t vStrafeDistance = vec3_zero();
	//if (animationIndex == skm->animationMap["WalkForwardRight"].index
	//	|| animationIndex == skm->animationMap["WalkForwardLeft"].index 
	//	|| animationIndex == skm->animationMap["WalkLeft"].index
	//	|| animationIndex == skm->animationMap["WalkRight"].index 
	//) {
	//	const vec3_t vNormalizedAngles = vec3_normalize( vec3_euler( GetAngles() ) );

	//	//const vec3_t vStrafeDirection = vec3_negate( vec3_normalize( vec3_cross( vec3_up(), GetAngles() ) ) );
	//	
	//	moveVelocity = vFrameMoveTranslate + vFrameMoveDistance;// * vStrafeDirection;
	//	
	//} else {
	//	moveVelocity = (vTranslate + vDistance ) * vDirectionNormal;
	//}

	if (animationIndex == skm->animationMap["TPose"].index
		|| animationIndex == skm->animationMap["Idle"].index 
		|| animationIndex == skm->animationMap["IdleAiming"].index
		|| animationIndex == skm->animationMap["RifleAim"].index 
		|| animationIndex == skm->animationMap["RifleFire"].index 
		|| animationIndex == skm->animationMap["WalkingToDying"].index 
		
	) {
		moveVelocity = vec3_zero();
	}


////Progress animation frames based on root bone distance
//const vec3_t vAnimationFullTranslation      = vRootBonePositionLastFrame - vRootBonePositionFirstFrame; //Only needed once per animation (walk, stairs_up, stairs_down)
//const float fAnimationFullDistance          = vec3_magnitude( vAnimationFullTranslation ); //Also only needed once
//const float fDistancePerFrame               = fAnimationFullDistance / float(iAnimationFrameAmount); //Assuming rootbone changes position more or less linear over the full anim, if not, calculate this per in-between frame
//
////fCurrentRatioBetweenCurrentFrameAndNextFrame => current blend factor of current>next frame in 0.0-1.0, where 0.0 => 100% current and 1.0 => 100% next frame
//const vec3_t vAnimationLocalTranslation     = vRootBonePositionCurrentFrame + (vRootBonePositionNextFrame - vRootBonePositionCurrentFrame) * fCurrentRatioBetweenCurrentFrameAndNextFrame;
//const float fAnimationLocalDistance         = vec3_magnitude( vAnimationLocalTranslation );
//const float fAmountOfFrameProgressByDist    = ( fAnimationLocalDistance / fAnimationFullDistance ) / fDistancePerFrame;
//
////Expectation: the faster the character goes, the faster the animation speed. Eliminating skating effect of feet on ground.
//PROGRESS_FRAME_FOR_CURRENT_ANIMATION_BY(fAmountOfFrameProgressByDist);
	
	// OLD VELOCITY CALCULATION:
	//// Create our move distance vector and multiply by moveSpeed
	//const vec3_t vDistance = { (float)(rbFrameMoveSpeed), (float)(rbFrameMoveSpeed), 0.f };
	//// Ignore the Z translation, it might get us "stuck" after all. (We negate the x and y to get positive forward results.)
	//const vec3_t vTranslate = vec3_t { moveTranslate.x * (float)FRAMETIME.count(), moveTranslate.y * (float)FRAMETIME.count(), 0.f};
	//// Get ourselves a normalized direction without Z.
	//const vec3_t vDirection = vec3_t { normalizedDir.x, normalizedDir.y, 0.f };
	//// Calculate the total moveVelocity into the normal's direction.
	//vec3_t moveVelocity = (vDistance + vTranslate) * vDirection;
	// EOF OLD VELOCITY CALCULATION.

	/**
	*	#4: (Apply Gravity here for now...) Let's begin performing our root motion movement.
	**/
	// Get the current velocity stored as "oldVelocity".
	const vec3_t oldVelocity = GetVelocity();

	// Old Velocity for Z Axis (Maintain darn gravity)
	moveVelocity.z = oldVelocity.z;
	
	//gi.DPrintf("Velocity Debug (Frame #%i, frameMoveSpeed=%f, frameMoveDistance=%f):\n", 
	//	animationFrame,
	//	frameMoveSpeed,
	//	frameDistance
	//);
	//gi.DPrintf("	moveVelocity(%f,%f,%f),	vTranslate(%f,%f,%f)\n	vDistance(%f,%f,%f),\n	vTranslateDir(%f,%f,%f)\n",
	//	moveVelocity.x, moveVelocity.y, moveVelocity.z,
	//	vTranslate.x, vTranslate.y, vTranslate.z,
	//	vDistance.x, vDistance.y, vDistance.z,
	//	vTranslateDir.x, vTranslateDir.y, vTranslateDir.z
	//);

	// And let's go!.
	SetVelocity(moveVelocity);

	return PerformRootMotionMove();
}