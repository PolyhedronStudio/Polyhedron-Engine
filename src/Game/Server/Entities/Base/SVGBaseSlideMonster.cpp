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
		//
		// Goal Management.
		//
		// Setup Client as our enemy.
		//GameEntity *geClientEnemy = GetGameWorld()->GetGameEntities()[1]; // Client.
		//SetEnemy(geClientEnemy);
		//SetGoalEntity(geClientEnemy);

		// Our Goal entity is either...:
		// 1: Goal
		// 2: Enemy
		// 3: None.
		GameEntity *geGoal = GetGoalEntity();

		if (!geGoal) {
			geGoal = GetEnemy();

			if (!geGoal) {
				geGoal = GetGameWorld()->GetGameEntities()[1];

				// if !geGoal .. geGoal = ... ?
			}
		}
		
		//
		// Yaw Speed.
		//
		SetYawSpeed(20.f);

		//
		// Direction.
		//
		// Get direction vector.
		vec3_t direction = geGoal->GetOrigin() - GetOrigin();
		// Cancel uit the Z direction.
		direction.z = 0;
		// if (flags::FLY {
		// //direction.z = 0;
		// }
		// Prepare ideal yaw angle to rotate to.
		SetIdealYawAngle( vec3_to_yaw( { direction.x, direction.y, 0.f } ) );


		//
		// Yaw Angle.
		//
		// Get the delta between wished for and current yaw angles.
		const float deltaYawAngle = TurnToIdealYawAngle( );

		//if ( !( deltaYawAngle > 5 && deltaYawAngle < 355 ) ) {
		const vec3_t oldVelocity = GetVelocity();
		const vec3_t normalizedDir = vec3_normalize(direction);
		
		// Move slower if the ideal yaw angle is out of range.
		// (Gives a more 'realistic' turning effect).
		if (deltaYawAngle > 45 && deltaYawAngle < 315) {
			// Set velocity to head into direction.
			const vec3_t wishVelocity = vec3_t {
				62.f * normalizedDir.x,
				62.f * normalizedDir.y,
				oldVelocity.z,
				// if (Flags::Fly) {
				//33.f * normalizedDir.z,
				// }
			};
			SetVelocity(wishVelocity);
		} else {
			// Set velocity to head into direction.
			const vec3_t wishVelocity = vec3_t {
				92.f * normalizedDir.x,
				92.f * normalizedDir.y,
				oldVelocity.z,
				// if (Flags::Fly) {
				//33.f * normalizedDir.z,
				// }
			};
			SetVelocity(wishVelocity);
		}
	// Perform our slide move.
	SlideMove();
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

static constexpr float STEPMOVE_STOPSPEED = 100.f;
static constexpr float STEPMOVE_FRICTION = 6.f;
static constexpr float STEPMOVE_WATERFRICTION = 1.f;

const int32_t SVGBaseSlideMonster::SlideMove() {
    // Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;

	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

	/**
	*	Step #2:	If there is a valid ground entity, store that we're 'on-ground'.
	*				If there is no valid ground entity, check and see if we've got one for this frame.
	**/
    // Get ground entity.
    GameEntity* geGroundEntity = SGGameWorld::ValidateEntity( GetGroundEntityHandle() );

    // Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( geGroundEntity ? true : false );

	// Defaults to -1.
	int32_t groundEntityNumber = -1;

    // If we have no ground entity.
    if ( !geGroundEntity ) {
        // Ensure we check if we aren't on one in this frame already. If so, store its number for our
		// movement below.
        groundEntityNumber = SG_BoxSlideMove_CheckForGround( this );
    } else {
		groundEntityNumber = geGroundEntity->GetNumber();
	}


	/**
	*	Step #3:	- Check and clamp our Velocity.
	*				- Apply Rotation Friction to Angular Velocity.
	*				- Apply Ground Friction to Velocity
	*				- LASTLY: Apply Gravity:
	*							- For Walking Monsters	: Gravity if not on ground, ground friction otherwise.
	*							- For Swimming Monsters	: Gravity if not in water (Try and fall into water.)
	*							- For Flying Monsters	: ...
	**/
	const float oldVelocityLength = vec3_length( GetVelocity() );

	// Bound our velocity within sv_maxvelocity limits.
	SG_CheckVelocity( this );

    // Check for angular velocities. If found, add rotational friction.
    vec3_t angularVelocity = GetAngularVelocity();

    if (angularVelocity.x || angularVelocity.y || angularVelocity.z) {
        SG_AddRotationalFriction( this );
	}

    // Re-ensure we fetched its latest angular velocity.
    angularVelocity = GetAngularVelocity();

	// Get EntityFlags.
	const int32_t entityFlags = GetFlags();
	const int32_t waterLevel = GetWaterLevel();

	// ## Walking Monsters:
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

	// ## Flying Monsters:
    if ( ( GetFlags() & EntityFlags::Fly ) ) {
		// Friction for Vertical Velocity.
		if ( ( GetVelocity().z != 0 ) ) {
			const float speed = fabs( GetVelocity().z );
			const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
			const float friction = STEPMOVE_FRICTION / 3;
			float newSpeed = speed - ( FRAMETIME.count() * control * friction );
			if ( newSpeed < 0 ) {
				newSpeed = 0;
			}
			newSpeed /= speed;
			const vec3_t velocity = GetVelocity();
			SetVelocity( { velocity.x, velocity.y, velocity.z * newSpeed } );
		}
	}

	// ## Swimming Monsters:
	if ( ( GetFlags() & EntityFlags::Swim ) ) {
		// Friction for swimming monsters that have been given vertical velocity
		if ( ( GetFlags() & EntityFlags::Swim ) && ( GetVelocity().z != 0 ) ) {
			const float speed = fabs( GetVelocity().z );
			const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
			float newSpeed = speed - ( FRAMETIME.count() * control * STEPMOVE_WATERFRICTION * GetWaterLevel() );
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
	//                  const float friction = STEPMOVE_FRICTION;
	//                  const float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
	//                  float newSpeed = speed - FRAMETIME.count() * control * friction;

	//                  if (newSpeed < 0) {
	//                      newSpeed = 0;
	//			}
	//                  newSpeed /= speed;

	//                  newVelocity[0] *= newSpeed;
	//                  newVelocity[1] *= newSpeed;

	//                  // Set the velocity.
	//                  geBoxSlide->SetVelocity( newVelocity );
	//              }
	//          }
	//}


	/**
	*	Step #4:	- Get appropriate Clip Mask.
	*				- Try and perform our slide move: Including, if wished for, stepping down/up.
	*				- ?
	**/
    // Default mask is solid.
    int32_t mask = GetClipMask();
        
	// Store old velocity for stepping.
	const vec3_t vel0 = GetVelocity();
	const vec3_t org0 = GetOrigin();

    // Execute "BoxSlideMove", essentially also our water move.
	MoveState slideMoveState;
    int32_t blockedMask = SG_BoxSlideMove( this, ( mask ? mask : BrushContentsMask::PlayerSolid ), 1.01f, 10, slideMoveState );

	if ( blockedMask & SlideMoveFlags::EdgeMoved) {
		//slideMoveState.velocity.z = vel0.z;
		
		//slideMoveState.origin = org0;
		// Resort to old origin.
		//slideMoveState.origin = org0; //{ org0.x, org0.y, slideMoveState.origin.z };
		//slideMoveState.
	}
	#if defined(SG_SLIDEMOVE_DEBUG_BLOCKMASK) && SG_SLIDEMOVE_DEBUG_BLOCKMASK == 1
	if (blockedMask != 0) {
		std::string blockMaskString = "SlideMove Entity(#" + std::to_string(GetNumber()) + ") blockMask: (";
		if (blockedMask & SlideMoveFlags::CanStepUp) { blockMaskString += "CanStepUp, "; }
		if (blockedMask & SlideMoveFlags::CanStepDown) { blockMaskString += "CanStepDown, "; }

		if (blockedMask & SlideMoveFlags::EntityTouched) { blockMaskString += "EntityTouched, "; }
		if (blockedMask & SlideMoveFlags::PlaneTouched) { blockMaskString += "PlaneTouched, "; }
		if (blockedMask & SlideMoveFlags::WallBlocked) { blockMaskString += "WallBlocked, "; }
		if (blockedMask & SlideMoveFlags::Trapped) { blockMaskString += "Trapped, "; }
		
		if (blockedMask & SlideMoveFlags::EdgeMoved) { blockMaskString += "EdgeMoved, "; }
		if (blockedMask & SlideMoveFlags::Moved) { blockMaskString += "Moved "; }
		blockMaskString += ")";
		
		gi.DPrintf( "%s\n", blockMaskString.c_str());
	} else {
		std::string blockMaskString = "SlideMove Entity(#%i) blockMask: (0)\n";
		gi.DPrintf( "%s\n", blockMaskString.c_str());
	}
#endif
	/**
	*	Step #6:	- The Move has been Performed: Update Entity Attributes.
	**/
	// Double validate ground entity at this moment in time.
	GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( slideMoveState.groundEntityNumber ) );

	// Update the entity with the resulting moveState values.
	SetOrigin( slideMoveState.origin );
	SetVelocity( slideMoveState.velocity );
	SetMins( slideMoveState.mins );
	SetMaxs( slideMoveState.maxs );
	SetFlags( slideMoveState.entityFlags );
	SetGroundEntity( geNewGroundEntity );
	SetGroundEntityLinkCount( slideMoveState.groundEntityLinkCount );

	// Link entity in.
	LinkEntity();

	/**
	*	Step #5:	- Prevent us from moving into water if we're not a swimming monster.
	*				- Try and perform our slide move: Including, if wished for, stepping down/up.
	*				- ?
	**/
		//// This is set to false whenever one of the following checks failed:
		//// - Water Check:	We don't want these entities to just walk into a pool of water.
		//// - Edge Check:	Unless the edge is of a legit staircase height, we don't want the
		////					NPC entity to blindly walk off of it.
		//// - 
		//bool setOrigin = true;

	//----------------------------
	// CHECK: Water Check
	//----------------------------
	// To prevent this entity from going into the water. We assume that if 
	// its waterLevel already was 0, we'll maintain it at 0.
	//----------------------------
	if ( !( entityFlags & EntityFlags::Swim ) ) {
		if ( GetWaterLevel() == 0 ) {
			vec3_t point = {
				slideMoveState.origin.x,
				slideMoveState.origin.y,
				slideMoveState.origin.z + slideMoveState.mins.z - 1,
			};
			const int32_t pointContents = SG_PointContents(point);

			if (pointContents & BrushContentsMask::Liquid) {
				// TODO: ?? DO SOMETHING, MOVE FAILED.
				//return false;
			}
		}
	}

	//----------------------------
	// CHECK: Edge Check
	//----------------------------
	// Notify entity it hit an edge/a ledge.
	//----------------------------


	
	/**
	*	Step #7:	- Execute Touch Callbacks in case we add any blockedMask set.
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

			// Call Touch for the other entity right before calling touch on geSlider.
			otherEntity->DispatchTouchCallback( otherEntity, this, nullptr, nullptr );

			DispatchTouchCallback( this, otherEntity, nullptr, nullptr );

			// Check if it may have been freed by the touch function, if so, break out.
			if( !IsInUse() ) {
				break;
			}
		}
	}


	/**
	*	Step #8:	- If still in use, check for ground, and see if our velocity came to a halt
	*				so we can safely trigger a Stop Dispatch callback.
	**/
	// If it's still in use, search for ground.
	if ( IsInUse() ) {
		// Check for ground entity.
		int32_t groundEntityNumber = SG_BoxSlideMove_CheckForGround( this );

		// Revalidate it
		GameEntity *geNewGroundEntity = SGGameWorld::ValidateEntity( gameWorld->GetGameEntityByIndex( groundEntityNumber ) );

		// Set it to a halt in case velocity becomes too low, this way it won't look odd.
		if( geNewGroundEntity && vec3_length( GetVelocity() ) <= 1.f && oldVelocityLength > 1.f ) {
			// Zero out velocities.
			SetVelocity( vec3_zero() );
			SetAngularVelocity( vec3_zero() );

			// Stop.
			DispatchStopCallback( );
		}
	}

		// Execute touch triggers.
        SG_TouchTriggers( this );

        // Can't continue if this entity wasn't in use.
        if ( IsInUse( ) ) {
            return 0;
		}

#ifdef SHAREDGAME_SERVERGAME
        // Check for whether to play a land sound.
        if ( geNewGroundEntity ) {
            if ( !wasOnGround ) {
                if ( hitSound ) {
                    SVG_Sound(this, 0, gi.SoundIndex("world/land.wav"), 1, 1, 0);
                }
            }
        }
#endif

		return 0;
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