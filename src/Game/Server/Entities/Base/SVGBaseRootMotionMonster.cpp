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


void SVGBaseRootMotionMonster::Move_NavigateToTarget() {
	/**
	*	#1: Very Cheap, WIP, Debug, pick the goal entity. lol.
	**/
	GameEntity *geMoveGoal= GetGoalEntity();

	if (!geMoveGoal) {
		geMoveGoal = GetEnemy();

		if (!geMoveGoal) {
			// TEMPORARY:
			/*SetVelocity(vec3_zero());
			RootMotionMove();
			return;*/
		}
	}
		
	/**
	*	#1: Calculate the direction to head into, and set the yaw angle and its turning speed.
	**/
	// Get direction vector.
	vec3_t direction = ( geMoveGoal ? ( geMoveGoal->GetOrigin() - GetOrigin() ) : vec3_zero() );
	// Cancel uit the Z direction for non flying monsters.
	direction.z = (GetFlags() & EntityFlags::Fly ? direction.z : 0);
	// if (flags::FLY { /* DO NOT CANCEL OUT Z */ }
	// Default speed.
	SetYawSpeed(20.f);
	// Prepare ideal yaw angle to rotate to.
	SetIdealYawAngle( vec3_to_yaw( { direction.x, direction.y, 0.f } ) );


	/**
	*	#2: Turn to the ideal yaw angle, and calculate our move velocity.
	*		If the yaw angle is too large, slow down the velocity.
	**/		
	// Get State.
	EntityAnimationState *animationState = &podEntity->currentState.currentAnimation;
		

	// Get the delta between wished for and current yaw angles.
	const float deltaYawAngle = TurnToIdealYawAngle( );

//	if ( deltaYawAngle > 45 && deltaYawAngle < 315 ) {
		// Should we switch to a different animation like turning?
		/*if (deltaYawAngle)*/
		// Switch if this wasn't our current animation yet.
		//if ( skm->animationMap["walk_turn_left"].index != animationIndex ) {
		//	animationIndex = SwitchAnimation("walk_turn_left");
		//}
//	} else {
	int32_t setAnimationIndex = animationState->animationIndex;
		if ( skm->animationMap["walk_standard"].index != setAnimationIndex ) {
			if  (podEntity->currentState.currentAnimation.frame == -1 || 
				podEntity->currentState.currentAnimation.frame >= podEntity->currentState.currentAnimation.endFrame) {
					setAnimationIndex = SwitchAnimation("walk_standard");
			}
		}

	//if ( !( deltaYawAngle > 5 && deltaYawAngle < 355 ) ) {
	const vec3_t oldVelocity = GetVelocity();
	const vec3_t normalizedDir = vec3_normalize(direction);

	const int32_t animationStartFrame = animationState->startFrame;
	const int32_t animationEndframe = animationState->endFrame;
	const int32_t animationFrame = animationState->frame - animationStartFrame;

	//gi.DPrintf("ANIMFRAME=%i\n", animationFrame);
	//	gi.DPrintf("ANIMFRAME=%i\n", animationFrame);
	//		gi.DPrintf("ANIMFRAME=%i\n", animationFrame);
	//			gi.DPrintf("ANIMFRAME=%i\n", animationFrame);
	// Get the total distance traveled for each frame, and also the
	// actual translation of the "root" (in our case the hip joint:06-06-2022)
	auto &frameDistances = skm->animations[setAnimationIndex]->frameDistances;
	auto &frameTranslates= skm->animations[setAnimationIndex]->frameTranslates;

	// Get the translation for this animation move frame.
	const vec3_t moveTranslate = ( frameTranslates.size() > animationFrame ? frameTranslates[animationFrame] : vec3_zero() );
	// Get the total move distance (vec3_length of a - b) for this animation move frame.
	const double moveDistance = ( frameTranslates.size() > animationFrame ? frameDistances[animationFrame] : 0.f );


	if (animationFrame < animationState->endFrame) {
		// Acts as a velocity multiplier and is determined by the Yaw Angle.
//		const double moveSpeed = ( deltaYawAngle > 45 && deltaYawAngle < 315 ? 1.15f : 1.825f );
		const double moveSpeed = 64.015f;

		// Create our move distance vector and multiply by moveSpeed
		const vec3_t vDistance = { (float)(moveSpeed * moveDistance), (float)(moveSpeed * moveDistance), 0.f };
		// Ignore the Z translation, it might get us "stuck" after all.
		const vec3_t vTranslate = vec3_t { moveTranslate.x, moveTranslate.y, 0.f };
		// Get ourselves a normalized direction without Z.
		const vec3_t vDirection = vec3_t { normalizedDir.x, normalizedDir.y, 0.f };
		// Calculate the total moveVelocity into the normal's direction.
		vec3_t moveVelocity = (vDistance + vTranslate) * vDirection;

		// Last but not least, we want to maintain our old Z velocity.
		moveVelocity.z = oldVelocity.z;

		// And let's go!.
		SetVelocity(moveVelocity);

		// Debug Output:
		//gi.DPrintf("Frame(#%i): { %f, %f, %f }\n",
		//	animationFrame,
		//	moveVelocity.x,
		//	moveVelocity.y,
		//	moveVelocity.z
		//);
	}

	// Move slower if the ideal yaw angle is out of range.
	// (Gives a more 'realistic' turning effect).
	//if (deltaYawAngle > 45 && deltaYawAngle < 315) {
	//if (animationFrame <= state.currentAnimation.endFrame) {
	//	// Get moved frame distance. (vec3_length of traversed distance.)
	//	const float frameDistance = frameDistances[animationFrame];
	//	// Calculate velocity based on distance traversed per frame.
	//	const vec3_t moveVelocity = vec3_t{
	//		frameDistance * normalizedDir.x,
	//		frameDistance* normalizedDir.y,
	//		(GetFlags() & EntityFlags::Fly ? frameDistances[animationFrame] * normalizedDir.z : oldVelocity.z),
	//	};

	//	SetVelocity(moveVelocity);
	//}
	//	// Set velocity to head into direction.
	//	//const vec3_t wishVelocity = vec3_t {
	//	//	52.f * normalizedDir.x,	52.f * normalizedDir.y, 
	//	//	(GetFlags() & EntityFlags::Fly ? 33.f * normalizedDir.z : oldVelocity.z ) 
	//	//};
	//	//SetVelocity(wishVelocity);
	//} else {
	//if (animationFrame <= state.currentAnimation.endFrame) {
	//	// Get moved frame distance. (vec3_length of traversed distance.)
	//	// Multiply it by 0.65 so the character "slow down".
	//	const float frameDistance = frameDistances[animationFrame] * 0.65f;
	//	// Calculate velocity based on distance traversed per frame.
	//	const vec3_t moveVelocity = vec3_t{
	//		frameDistance * normalizedDir.x,
	//		frameDistance* normalizedDir.y,
	//		(GetFlags() & EntityFlags::Fly ? frameDistances[animationFrame] * normalizedDir.z : oldVelocity.z),
	//	};

	//	SetVelocity(moveVelocity);
	//}
	//	// Set velocity to head into direction.
	//	//const vec3_t wishVelocity = vec3_t { 
	//	//	72.f * normalizedDir.x,	72.f * normalizedDir.y,
	//	//	(GetFlags() & EntityFlags::Fly ? 33.f * normalizedDir.z : oldVelocity.z ) 
	//	//};
	//	//SetVelocity(wishVelocity);
	//}

	/**
	*	#3: Start moving our Monster ass.
	**/
	// Perform our Root Motion Move.
	if (!geMoveGoal) {
		vec3_t vel = GetVelocity();
		vel.x = 0;
		vel.y = 0;
		SetVelocity(vel);
	}
	const int32_t slideMoveMask = PerformRootMotionMove();

	// Check the mask for animation switching.
	if ( (slideMoveMask & RootMotionMoveResult::SteppedUp) ) {
	//	gi.DPrintf("STEPPED UP DAWG\n");
		if ( skm->animationMap["run_stairs_up"].index != setAnimationIndex ) {
			setAnimationIndex = SwitchAnimation("run_stairs_up");
		}
	}

	if ( (slideMoveMask & RootMotionMoveResult::SteppedDown) ) {
	//	gi.DPrintf("STEPPED DOWN DAWG\n");
		if ( skm->animationMap["walk_stairs_down"].index != setAnimationIndex ) {
			setAnimationIndex = SwitchAnimation("walk_stairs_down");
		}
	}

	// Debug Output:

//	gi.DPrintf("SV_Entity(#%i): AnimationIndex(#%i), AnimationFrame(#%i)\n", GetState().number, setAnimationIndex, animationFrame);
}





/***
*
*	Monster Entity Functions.
*
***/
/**
*	@brief	Categorizes what other contents the entity resides in. (Water, Lava, or...)
**/
void SVGBaseRootMotionMonster::CategorizePosition() {
	// WaterLevel: Feet.
	vec3_t point = GetOrigin();
	point.z += GetMins().z + 1.f;

	// Get contents at point.
	int32_t contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents& BrushContents::Water ) ) {
		SetWaterLevel(WaterLevel::None);
		SetWaterType(0);
		return;
	}

	// Store water type.
	SetWaterType(contents);
	SetWaterLevel(1);

	// WaterLevel: Waist.
	point.z += 40.f;

	// Get contents at point.
	contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents & BrushContents::Water ) ) {
		SetWaterType(2);
		return;
	}

	// WaterLevel: Waist.
	point.z += 45.f;

	// Get contents at point.
	contents = SG_PointContents(point);

	// We're not in water, no need to further test.
	if ( !( contents & BrushContents::Water ) ) {
		SetWaterType(3);
		return;
	}
	
}

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
	// Store a copy of our previous moveState just to be sure.
	const RootMotionMoveState previousMoveState = rootMotionMoveState;
	// TODO: THIS WE SHOULD NOT NEED!!
	//rootMotionMoveState = { 
	//	.moveFlags = previousMoveState.moveFlags,
	//	.moveFlagTime = previousMoveState.moveFlagTime };

    // Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( rootMotionMoveState.groundEntityNumber != -1 ? true : false );
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
    if ( ( GetFlags() & EntityFlags::Fly ) ) {
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
	if ( ( GetFlags() & EntityFlags::Swim ) ) {
		// Friction for swimming monsters that have been given vertical velocity
		if ( ( GetFlags() & EntityFlags::Swim ) && ( GetVelocity().z != 0 ) ) {
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
	// With the previous state stored for reversion in case of need, we'll perform the new move.
	const int32_t moveResultMask = SG_RootMotion_PerformMove( 
		this, 
		( moveClipMask ? moveClipMask : BrushContentsMask::PlayerSolid ), 
		ROOTMOTION_MOVE_CLIP_BOUNCE, 
		ROOTMOTION_MOVE_GROUND_FRICTION, 
		&rootMotionMoveState 
	);


	#if defined(SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK) && (SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK == 1)
	{
		const int32_t entityNumber = GetNumber();

		// Only output if the spawnflag is set to do so.
		if ( (GetSpawnFlags() & 128) ) {

			DebugPrint(entityNumber, moveResultMask, previousMoveResultMask, rootMotionMoveState.moveFlags, rootMotionMoveState.moveFlagTime);
		}
	}
	#endif
	// Be sure to store it for the next time we move.
	if (previousMoveResultMask != moveResultMask) {
		previousMoveResultMask = moveResultMask;
	}

	// Ideally in a perfect world we'd never get trapped, but Quake and all its derivatives
	// are of course perfectly beautiful creatures from bottomless pits where no developer
	// should ever want to be found, dead... or alive.
	//
	// So... I present to you the following bold and ugly motherfucking hack:
	if ( ( moveResultMask & RootMotionMoveResult::Trapped) ) {
		// The reason we do this here is that even though we inspect for trapped inside
		// the RootMotionMove repeatedly, it performs on a sub-level. This statement catches
		// the worst of the worst situations and will resort to old origin and velocity.
		rootMotionMoveState.origin	= previousMoveState.origin;
		rootMotionMoveState.velocity	= previousMoveState.velocity;
	}


	/**
	*	Step #2:	- The Move has been Performed: Update Entity Attributes.
	**/
	// Double validate ground entity at this moment in time.
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( rootMotionMoveState.groundEntityNumber ) );

	// Update the entity with the resulting moveState values.
	SetOrigin( rootMotionMoveState.origin );
	SetVelocity( rootMotionMoveState.velocity );
	SetMins( rootMotionMoveState.mins );
	SetMaxs( rootMotionMoveState.maxs );
	SetFlags( rootMotionMoveState.entityFlags );
	SetWaterLevel( rootMotionMoveState.waterLevel );
	SetWaterType( rootMotionMoveState.waterType );

	if (rootMotionMoveState.groundEntityNumber != -1 && geNewGroundEntity) {
		SetGroundEntity( geNewGroundEntity );
		SetGroundEntityLinkCount( geNewGroundEntity->GetLinkCount() );
	} else {
		SetGroundEntity( SGEntityHandle() );
		SetGroundEntityLinkCount( 0 );
	}


	// Link entity in.
	LinkEntity();
	

	/**
	*	Step #3:	- Execute Touch Callbacks in case we add any blockedMask set.
	**/
	// Execute touch callbacks.
	if( moveResultMask != 0 ) {
		GameEntity *otherEntity = nullptr;

		// Call Touch Triggers on our slide box entity for its new position.
		SG_TouchTriggers( this );

		// Dispatch 'Touch' callback functions to all touched entities we caught and stored in our moveState.
		for( int32_t i = 0; i < rootMotionMoveState.numTouchEntities; i++ ) {
			otherEntity = gameWorld->GetGameEntityByIndex( rootMotionMoveState.touchEntites[i] );
			
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
	//// Assuming we're still in use, set ourselves to a halt if 
	//if ( IsInUse() ) {
	//	// Check for ground entity.
	//	int32_t groundEntityNumber = SG_BoxRootMotionMove_CheckForGround( this );

	//	// Revalidate it
	//	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( groundEntityNumber ) );

	//	// Set it to a halt in case velocity becomes too low, this way it won't look odd.
	//	if( geNewGroundEntity && vec3_length( GetVelocity() ) <= 1.f && oldVelocityLength > 1.f ) {
	//		// Zero out velocities.
	//		SetVelocity( vec3_zero() );
	//		SetAngularVelocity( vec3_zero() );

	//		// Stop.
	//		DispatchStopCallback( );
	//	}
	//}
	// Execute touch triggers (Since entities might move during touch callbacks, we might've
	// hit new entities.)
    SG_TouchTriggers( this );

    // Can't continue if this entity wasn't in use.
    //if ( !IsInUse( ) ) {
    //    return blockedMask;
	//}


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
	*	#7: We're done, return blockedMask.
	**/
	return moveResultMask;
}

/**
*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
**/
void SVGBaseRootMotionMonster::DebugPrint(const int32_t entityNumber, const int32_t resultMask, const int32_t previousResultMask, const int32_t moveFlags, const int32_t moveFlagTime) {
#if defined(SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK) && SG_ROOTMOTION_MOVE_DEBUG_RESULTMASK == 1
	// We only want to print if anything changed, otherwise we spam the console, we don't want to,
	// simply because the ideal places for spam are on social-media instead :P
	if ( resultMask == previousResultMask ){
		return;
	}

	// Debug Output Strings.
	std::string resultMaskStr = "";
	std::string moveFlagsStr = "";

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

	if ( resultMask & RootMotionMoveResult::FallingDown ) { resultMaskStr += "FallingDown "; }

	if ( resultMask & RootMotionMoveResult::EntityTouched ) { resultMaskStr += "EntityTouched "; }
	if ( resultMask & RootMotionMoveResult::PlaneTouched ) { resultMaskStr += "PlaneTouched "; }
	if ( resultMask & RootMotionMoveResult::WallBlocked ) { resultMaskStr += "WallBlocked "; }
	if ( resultMask & RootMotionMoveResult::Trapped ) { resultMaskStr += "Trapped"; }

	/**
	*	MoveFlags.
	**/
	// Fill up the movedFlagStr with matching stringified flag names.
	if ( resultMask & RootMotionMoveFlags::FoundGround ) { moveFlagsStr += "FoundGround "; }
	if ( resultMask & RootMotionMoveFlags::OnGround ) { moveFlagsStr += "OnGround "; }
	if ( resultMask & RootMotionMoveFlags::LostGround ) { moveFlagsStr += "LostGround "; }

	if ( resultMask & RootMotionMoveFlags::Ducked ) { moveFlagsStr += "Ducked "; }
	if ( resultMask & RootMotionMoveFlags::Jumped ) { moveFlagsStr += "Jumped "; }

	if ( resultMask & RootMotionMoveFlags::OnLadder ) { moveFlagsStr += "OnLadder "; }
	if ( resultMask & RootMotionMoveFlags::UnderWater ) { moveFlagsStr += "UnderWater "; }

	if ( resultMask & RootMotionMoveFlags::TimePushed ) { moveFlagsStr += "TimePushed "; }
	if ( resultMask & RootMotionMoveFlags::TimeWaterJump ) { moveFlagsStr += "TimeWaterJump "; }
	if ( resultMask & RootMotionMoveFlags::TimeLand ) { moveFlagsStr += "TimeLand"; }


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