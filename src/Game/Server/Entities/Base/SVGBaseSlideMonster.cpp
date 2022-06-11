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
#include "SVGBaseSlideMonster.h"

// Game World.
#include "../World/ServerGameWorld.h"

#define STEPSIZE 18

//! Constructor/Destructor.
SVGBaseSlideMonster::SVGBaseSlideMonster(PODEntity *svEntity) : Base(svEntity) { }
SVGBaseSlideMonster::~SVGBaseSlideMonster() { }


/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseSlideMonster::Precache() { 
	Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseSlideMonster::Spawn() { 
	// Base Spawn.
	Base::Spawn();

	// Use an Octagon Shaped Hull by default. Allows for more realistic character sliding.
    SetSolid( Solid::OctagonBox );
	// Set move type.
    SetMoveType( MoveType::SlideMove );
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
void SVGBaseSlideMonster::PostSpawn() { 
	Base::Spawn(); 
}

/**
*   @brief 
**/
void SVGBaseSlideMonster::Respawn() { 
	Base::Respawn(); 
}

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseSlideMonster::Think() { 
	// Base think.
	Base::Think();
}

/**
*   @brief 
**/
void SVGBaseSlideMonster::SpawnKey(const std::string& key, const std::string& value) { 
	Base::SpawnKey(key, value); 
}


void SVGBaseSlideMonster::Move_NavigateToTarget() {
	/**
	*	#1: Very Cheap, WIP, Debug, pick the goal entity. lol.
	**/
	GameEntity *geMoveGoal= GetGoalEntity();

	if (!geMoveGoal) {
		geMoveGoal = GetEnemy();

		if (!geMoveGoal) {
			geMoveGoal = GetGameWorld()->GetGameEntities()[1];

			// if !geGoal .. geGoal = ... ?
		}
	}
		
	/**
	*	#1: Calculate the direction to head into, and set the yaw angle and its turning speed.
	**/
	// Get direction vector.
	vec3_t direction = geMoveGoal->GetOrigin() - GetOrigin();
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
	// Perform our slide move.
	const int32_t slideMoveMask = SlideMove();

	// Check the mask for animation switching.
	if ( (slideMoveMask & SlideMoveFlags::SteppedUp) ) {
	//	gi.DPrintf("STEPPED UP DAWG\n");
		if ( skm->animationMap["run_stairs_up"].index != setAnimationIndex ) {
			setAnimationIndex = SwitchAnimation("run_stairs_up");
		}
	}

	if ( (slideMoveMask & SlideMoveFlags::SteppedDown) ) {
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
void SVGBaseSlideMonster::CategorizePosition() {
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
float SVGBaseSlideMonster::TurnToIdealYawAngle() {
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
static constexpr float SLIDEMOVE_STOP_SPEED = 100.f;
static constexpr float SLIDEMOVE_FRICTION = 6.f;
static constexpr float SLIDEMOVE_WATER_FRICTION = 1.f;

/**
*	@brief	Performs a basic SlideMove by setting up a SlideMoveState and calling into
*			SlideMove physics.
*			
*			It'll try and step down, as well as step up stairs. If it's non steppable,
*			it resorts to sliding along the edge "crease".
**/
const int32_t SVGBaseSlideMonster::SlideMove() {
	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

	// Get Entity Flags.
	const int32_t entityFlags = GetFlags();
	// Get WaterLevel.
	const int32_t waterLevel = GetWaterLevel();
    // Default mask is solid.
    const int32_t moveClipMask = GetClipMask();
	// Store a copy of our previous moveState just to be sure.
	const SlideMoveState previousMoveState = slideMoveState;
	// TODO: THIS WE SHOULD NOT NEED!!
	slideMoveState = { .moveFlags = previousMoveState.moveFlags,
	.moveFlagTime = previousMoveState.moveFlagTime };

    // Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( slideMoveState.groundEntityNumber != -1 ? true : false );
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
			const float control = speed < SLIDEMOVE_STOP_SPEED ? SLIDEMOVE_STOP_SPEED : speed;
			const float friction = SLIDEMOVE_FRICTION / 3;

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
			const float control = speed < SLIDEMOVE_STOP_SPEED ? SLIDEMOVE_STOP_SPEED : speed;
			float newSpeed = speed - ( FRAMETIME.count() * control * SLIDEMOVE_WATER_FRICTION * GetWaterLevel() );
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
	//                  const float friction = SLIDEMOVE_FRICTION;
	//                  const float control = speed < SLIDEMOVE_STOP_SPEED ? SLIDEMOVE_STOP_SPEED : speed;
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
	*				- Try and perform our slide move: Including, if wished for, stepping down/up.
	*				- ?
	**/
	// With the previous state stored for reversion in case of need, we'll perform the new move.
	const int32_t blockedMask = SG_BoxSlideMove( 
		this, 
		( moveClipMask ? moveClipMask : BrushContentsMask::PlayerSolid ), 
		SLIDEMOVE_CLIP_BOUNCE, 
		SLIDEMOVE_FRICTION, 
		slideMoveState 
	);
	#if defined(SG_SLIDEMOVE_DEBUG_BLOCKMASK) && (SG_SLIDEMOVE_DEBUG_BLOCKMASK == 1)
	{
		if ( (GetSpawnFlags() & 128) ) {
			DebugPrint(blockedMask);
		}
	}
	#endif

	// Ideally in a perfect world we'd never get trapped, but Quake and all its derivatives
	// are of course perfectly beautiful creatures from bottomless pits where no developer
	// should ever want to be found, dead... or alive.
	//
	// So... I present to you the following bold and ugly motherfucking hack:
	if ( (blockedMask & SlideMoveFlags::Trapped) ) {
		// The reason we do this here is that even though we inspect for trapped inside
		// the SlideMove repeatedly, it performs on a sub-level. This statement catches
		// the worst of the worst situations and will resort to old origin and velocity.
		slideMoveState.origin	= previousMoveState.origin;
		slideMoveState.velocity	= previousMoveState.velocity;
	}


	/**
	*	Step #2:	- The Move has been Performed: Update Entity Attributes.
	**/
	// Double validate ground entity at this moment in time.
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( slideMoveState.groundEntityNumber ) );

	// Update the entity with the resulting moveState values.
	SetOrigin( slideMoveState.origin );
	SetVelocity( slideMoveState.velocity );
	SetMins( slideMoveState.mins );
	SetMaxs( slideMoveState.maxs );
	SetFlags( slideMoveState.entityFlags );
	SetWaterLevel( slideMoveState.waterLevel );
	SetWaterType( slideMoveState.waterType );

	if (slideMoveState.groundEntityNumber != -1 && geNewGroundEntity) {
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
	if( blockedMask != 0 ) {
		GameEntity *otherEntity = nullptr;

		// Call Touch Triggers on our slide box entity for its new position.
		SG_TouchTriggers( this );

		// Dispatch 'Touch' callback functions to all touched entities we caught and stored in our moveState.
		for( int32_t i = 0; i < slideMoveState.numTouchEntities; i++ ) {
			otherEntity = gameWorld->GetGameEntityByIndex( slideMoveState.touchEntites[i] );
			
			// Don't touch projectiles.
			if( !otherEntity || !otherEntity->IsInUse() ) { //|| otherEntity->GetFlags() & PROJECTILE_THING_FLAG) {
				continue;
			}

			// First dispatch a touch on the object we're hitting.
			otherEntity->DispatchTouchCallback( otherEntity, this, nullptr, nullptr );

			// Now dispatch a touch callback for THIS entity.
			DispatchTouchCallback( this, otherEntity, nullptr, nullptr );

			// In case touch callbacks caused it to be non 'in-use', return blockedMask since we're done.
			// -> old behavior: break out of the loop.
			if( !IsInUse() ) {
				return blockedMask;
				//break; // TODO: Should we do a break or just return here?
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
	//	int32_t groundEntityNumber = SG_BoxSlideMove_CheckForGround( this );

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
	return blockedMask;
}

/**
*	@brief	Useful for debugging slidemoves, enable on an entity by setting spawnflag 128
**/
void SVGBaseSlideMonster::DebugPrint(const int32_t blockedMask) {
#if defined(SG_SLIDEMOVE_DEBUG_BLOCKMASK) && SG_SLIDEMOVE_DEBUG_BLOCKMASK == 1
		if (blockedMask != 0) {
			std::string blockMaskString = std::to_string(GetSpawnFlags()) + "SlideMove Entity(#" + std::to_string(GetNumber()) + ") blockMask: (";
			if (blockedMask & SlideMoveFlags::WallBlocked) { blockMaskString += "WallBlocked, "; }
			if (blockedMask & SlideMoveFlags::Trapped) { blockMaskString += "Trapped, "; }
		

			if (blockedMask & SlideMoveFlags::Moved) { blockMaskString += "Moved, "; }
			if (blockedMask & SlideMoveFlags::EdgeMoved) { blockMaskString += "EdgeMoved, "; }
			
			if (blockedMask & SlideMoveFlags::EntityTouched) { blockMaskString += "EntityTouched, "; }
			if (blockedMask & SlideMoveFlags::PlaneTouched) { blockMaskString += "PlaneTouched, "; }

			if (blockedMask & SlideMoveFlags::SteppedUp) { blockMaskString += "SteppedUp, "; }
			if (blockedMask & SlideMoveFlags::SteppedDown) { blockMaskString += "SteppedDown, "; }
			if (blockedMask & SlideMoveFlags::SteppedDownFall) { blockMaskString += "SteppedDownFall, "; }
			blockMaskString += ")";
		
			gi.DPrintf( "%s\n", blockMaskString.c_str());
		} else {
			std::string blockMaskString = "SlideMove Entity(#" + std::to_string(GetNumber()) + ") blockMask: (0)\n";
			gi.DPrintf( "%s\n", blockMaskString.c_str());
		}
#endif
}

void SVGBaseSlideMonster::SlideMove_FixCheckBottom() {
	SetFlags( GetFlags() | EntityFlags::PartiallyOnGround );
}

const bool SVGBaseSlideMonster::SlideMove_CheckBottom() {
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