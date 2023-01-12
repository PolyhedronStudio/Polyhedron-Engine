/*
// LICENSE HERE.

//
// MiscSphereBall.cpp
//
//
*/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"            // Util funcs.

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"

// Physics.
#include "Game/Shared/Physics/Physics.h"
#include "Game/Shared/Physics/RootMotionMove.h"

// Misc Explosion Box Entity.
#include "MiscSphereBall.h"

#include "../../Gamemodes/IGamemode.h"
#include "../../World/ServerGameWorld.h"

//
// Constructor/Deconstructor.
//
MiscSphereBall::MiscSphereBall(PODEntity *svEntity) 
    : Base(svEntity) {

}


//
// Interface functions. 
//
//
//===============
// MiscSphereBall::Precache
//
//===============
//
void MiscSphereBall::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache actual barrel model.
    SVG_PrecacheModel("models/misc/spheres/sphereball");

    // Precache the debris.
    SVG_PrecacheModel("models/objects/debris1/tris.md2");
    SVG_PrecacheModel("models/objects/debris2/tris.md2");
    SVG_PrecacheModel("models/objects/debris3/tris.md2");
}

//
//===============
// MiscSphereBall::Spawn
//
//===============
//
void MiscSphereBall::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    //SetRenderEffects(GetRenderEffects() | RenderEffects::DebugBoundingBox);

    // Set solid.
    SetSolid( Solid::Sphere );
    // Set clip mask.
    SetClipMask( BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid );
    // Set the barrel model, and model index.
    SetModel( "models/misc/spheres/sphereball.iqm" );

	bbox3_t radiusBounds = bbox3_from_center_radius( 20, vec3_zero() );
	SetMins( radiusBounds.mins );
	SetMaxs( radiusBounds.maxs );
	//SetMins( { -24, -24, -24 } );
	//SetMaxs( { 24, 24, 24 } );
    // Set move type.
    SetMoveType( MoveType::TossSlideBox );
	// Since this is a "monster", after all...
    //SetServerFlags( EntityServerFlags::Monster );

    // Set default values in case we have none.
    //if ( !GetMass() ) {
        SetMass( 100 );
    //}
    //if ( !GetHealth() ) {
        SetHealth(99999);
    //}
    if ( !GetDamage() ) {
        SetDamage( 150 );
    }

    // We need it to take damage in case we want it to explode.
    SetTakeDamage( TakeDamage::Yes );

    // Setup our MiscSphereBall callbacks.
    SetUseCallback( &MiscSphereBall::SphereBallUse );
	SetDieCallback( &MiscSphereBall::SphereBallDie );
    SetTouchCallback( &MiscSphereBall::SphereBallTouch );
	SetTakeDamageCallback( &MiscSphereBall::SphereBallTakeDamage );

	// Physics callbacks.
	SetStopCallback(&MiscSphereBall::SphereBallStop);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2.f * FRAMETIME_S);
    SetThinkCallback(&MiscSphereBall::SphereBallDropToFloor);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscSphereBall::Respawn
//
//===============
//
void MiscSphereBall::Respawn() {
    Base::Respawn();
}

//
//===============
// MiscSphereBall::PostSpawn
//
//===============
//
void MiscSphereBall::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// MiscSphereBall::Think
//
//===============
//
void MiscSphereBall::Think() {
    // Always call parent class method.
    Base::Think();
}

//===============
// MiscSphereBall::SpawnKey
//
//===============
void MiscSphereBall::SpawnKey(const std::string& key, const std::string& value) {
	Base::SpawnKey(key, value);
}


//
// Callback Functions.
//

// ==============
// MiscSphereBall::SphereBallUse
// 
// So that mappers can trigger this entity in order to blow it up
// ==============
void MiscSphereBall::SphereBallUse( IServerGameEntity* caller, IServerGameEntity* activator )
{
    SphereBallDie( caller, activator, 999, GetOrigin() );
}

void MiscSphereBall::SphereBallThink(void) {
	// Store whether we had a ground entity at all.
    const qboolean wasOnGround = ( GetGroundEntityHandle() ? true : false );

	// Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;
	// Entity flags that were set at start of the think move.
	const int32_t entityFlags = GetFlags();
	// Get velocity length.
	const vec3_t oldVelocity = GetVelocity();
	const float oldVelocityLength = vec3_length( oldVelocity );

	// Bound our velocity within sv_maxvelocity limits.
	//SG_BoundVelocity( this );
	if ( !wasOnGround ) {
		SG_AddGravity( this );
	}
	SG_CheckGround( this );

    // Get angular velocity for applying rotational friction.
    vec3_t angularVelocity = GetAngularVelocity();

	// If we got any angular velocity, apply friction.
    if (angularVelocity.x || angularVelocity.y || angularVelocity.z) {
		//SetAngularVelocity( SG_CalculateRotationalFriction( this ) );
		// Acquire the rotational velocity first.
		vec3_t angularVelocity = GetAngularVelocity();

		// Calculate adjustment to apply.
		static constexpr float SPHEREBALL_MOVE_STOP_SPEED = 1.01f;
		static constexpr float SPHEREBALL_MOVE_GROUND_FRICTION = 2.5f;
		const float adjustment = FRAMETIME_S.count() * SPHEREBALL_MOVE_STOP_SPEED * SPHEREBALL_MOVE_GROUND_FRICTION;

		// Apply adjustments.
		angularVelocity = GetAngularVelocity();
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

		// Set angles in proper direction.
		SetAngles( vec3_clamp_euler( vec3_fmaf( GetAngles(), FRAMETIME_S.count(), angularVelocity ) ) );

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
            }
		}

		/**
		*	Apply friction: Let it slide.
		**/
	} else {
		const vec3_t currentVelocity = GetVelocity();
		if ( currentVelocity.z || currentVelocity.y || currentVelocity.x ) {
			// Apply friction: Let dead NPCs who aren't completely onground slide.
			if ( ( wasOnGround ) || ( GetFlags() & (EntityFlags::Swim | EntityFlags::Fly) ) ) {
			//if ( geBoxSlide->GetDeadFlag() == DeadFlags::Dead) {//!( geBoxSlide->GetHealth() <= 0.0 ) ) {
				vec3_t newVelocity = currentVelocity;
				const float speed = sqrtf( newVelocity[0] * newVelocity[0] + newVelocity[1] * newVelocity[1] );
				if (speed) {
					// TODO: Defines?
					constexpr float GROUNDFRICTION = 6.f; // Ground friction.
					constexpr float STOPSPEED = 100.f / GROUNDFRICTION; // Divided by mass.

					// Calculate friction, and ground control.
					const float friction = GROUNDFRICTION;
					const float control = speed < STOPSPEED ? STOPSPEED : speed;
					float newSpeed = speed - FRAMETIME_S.count() * control * friction;
					if (newSpeed < 0) {
						newSpeed = 0;
				}
					newSpeed /= speed;
					newVelocity[0] *= newSpeed;
					newVelocity[1] *= newSpeed;
					newVelocity[2] *= newSpeed;
					// Set the velocity.
					SetVelocity( newVelocity );
				}
			//}
			}    
		}
	}

	//SVG_DPrint( fmt::format( "sv_gravity={}, wasOnGround={}, velocity({},{},{}), oldvelocity({},{},{})\n",
	//sv_gravity->value,
	//		   wasOnGround,
	//		   velocity.x,velocity.y,velocity.z,
	//		   oldVelocity.x, oldVelocity.y, oldVelocity.z
	//));

	LinkEntity();

	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &MiscSphereBall::SphereBallThink );
}

//
//===============
// MiscSphereBall::SphereBallDropToFloor
//
// Think callback, to execute the needed physics for this pusher object.
//===============
//
void MiscSphereBall::SphereBallDropToFloor(void) {
	constexpr float groundOffset = 0.03125;

    // First, ensure our origin is +1 off the floor.
    vec3_t traceStart = GetOrigin() + vec3_t{
        0.f, 0.f, 1.f
    };
    
    SetOrigin(traceStart);

    // Calculate the end origin to use for tracing.
    vec3_t traceEnd = traceStart + vec3_t{
        0, 0, -1.f + groundOffset
    };
    
    // Exceute the trace.
    SVGTraceResult trace = SVG_Trace(traceStart, GetMins(), GetMaxs(), traceEnd, this, BrushContentsMask::MonsterSolid);
    
    // Return in case we hit anything.
    if (trace.fraction != 1.f || trace.allSolid) {
	    return;
    }
    
    // Set new entity origin.
    SetOrigin(trace.endPosition);

    SG_CheckGround(this);

    // Link entity back in.
    LinkEntity();

	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &MiscSphereBall::SphereBallThink );
}

//
//===============
// MiscSphereBall::MiscSphereBallExplode
//
// 'Think' callback that is set when the explosion box is exploding.
// (Has died due to taking damage.)
//===============
//
void MiscSphereBall::MiscSphereBallExplode(void) {
    // Execute radius damage.
    GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), NULL, GetDamage() + 40, MeansOfDeath::Barrel);

    // Retrieve origin.
    vec3_t save = GetOrigin();

    // Set the new origin.
    vec3_t debrisSpawnOrigin = (vec3_fmaf(GetAbsoluteMin(), 0.5f, GetSize()));

    // Throw several "debris1/tris.md2" chunks.
    SpawnDebris1Chunk();
    SpawnDebris1Chunk();

    // Bottom corners
    vec3_t origin = GetAbsoluteMin();
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);
    origin = GetAbsoluteMin();
    origin.x += GetSize().x;
    origin.y += GetSize().y;
    SpawnDebris3Chunk(origin);

    // Spawn 8 "debris2/tris.md2" chunks.
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();
    SpawnDebris2Chunk();

    // Reset origin to saved origin.
    //SetOrigin(save);

    // Depending on whether we have a ground entity or not, we determine which explosion to use.
    if (ServerGameWorld::ValidateEntity(GetGroundEntityHandle())) {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
        gi.MSG_WriteUint8(TempEntityEvent::Explosion1);//WriteByte(TempEntityEvent::Explosion1);
        gi.MSG_WriteUint16(GetNumber());//gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());						
        gi.Multicast(GetOrigin(), Multicast::PHS);

        GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

        const Frametime save = GetDelayTime();
        SetDelayTime(GameTime::zero());
        UseTargets(GetActivator());
        SetDelayTime(save);
    } else {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
        gi.MSG_WriteUint8(TempEntityEvent::Explosion2);//WriteByte(TempEntityEvent::Explosion2);
        gi.MSG_WriteUint16(GetNumber());   //gi.MSG_WriteVector3(GetOrigin(), false);//WriteVector3(GetOrigin());
        gi.Multicast(GetOrigin(), Multicast::PHS);

        GetGameMode()->InflictRadiusDamage(this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

        const Frametime save = GetDelayTime();
        SetDelayTime(GameTime::zero());
        UseTargets(GetActivator());
        SetDelayTime(save);
    }

	// Notify the server this is, specifically a monster, by adding the Monster flag.
    SetServerFlags( GetServerFlags() | EntityServerFlags::DeadMonster );
	// Unset movetype and solid.
    SetMoveType(MoveType::None);
    SetSolid(Solid::Not);
	// Entity is alive.
	SetDeadFlag( DeadFlags::Dead );
    // Set entity to allow taking damage.
    SetTakeDamage( TakeDamage::No );
	// Link it in.
	LinkEntity();

    // Ensure we have no more think callback pointer set when this entity has "died"
    //SetNextThinkTime(level.time + 1.f * FRAMETIME_S);
    //SetThinkCallback(&MiscSphereBall::SVGBaseEntityThinkFree);
	Remove();
}

//
//===============
// MiscSphereBall::SphereBallDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscSphereBall::SphereBallDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
    // Entity is dying, it can't take any more damage.
    SetTakeDamage(TakeDamage::No);

    // Attacker becomes this entity its "activator".
    SetActivator(attacker);

	// Get the general length of velocity, if it's non 0 we want to give this box time to fly. Teeehee :-)
	const float velocityLength = vec3_length(GetVelocity());

	if (velocityLength > 0) {
		// Setup the next think and think time.
		uint32_t nextThinkOffset = RandomRangeui(15, 35);
		SetNextThinkTime(level.time + (float)nextThinkOffset * FRAMETIME_S);
	} else {
		// Setup the next think and think time.
		SetNextThinkTime(level.time + 2 * FRAMETIME_S);
	}

	// Set explosion callback.
	//SetThinkCallback(&MiscSphereBall::MiscSphereBallExplode);

	// For temporary reasons we do this right here for now.
    // Set think function.
	SetThinkCallback( &MiscSphereBall::SVGBaseEntityThinkNull );
   	// Notify the server this is, specifically a monster, by adding the Monster flag.
    SetServerFlags( GetServerFlags() | EntityServerFlags::DeadMonster );
	// Unset movetype and solid.
    SetMoveType(MoveType::None);
    SetSolid(Solid::Not);
	// Entity is alive.
	SetDeadFlag( DeadFlags::Dead );
    // Set entity to allow taking damage.
    SetTakeDamage( TakeDamage::No );
	// Link it in.
	LinkEntity();

	// Remove entity.
	Remove();

}

//
//===============
// MiscSphereBall::SphereBallTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void MiscSphereBall::SphereBallTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// Validate 'other' first.
	GameEntity *geValidatedOther = ServerGameWorld::ValidateEntity(other);

    // Safety checks.
    if ( !geValidatedOther || geValidatedOther == this ) {
	    return;
    }

    // Ground entity checks.
	GameEntity *geGroundEntity = ServerGameWorld::ValidateEntity( geValidatedOther->GetGroundEntityHandle() );

    if ( geGroundEntity == this ) {
	    return;
    }

    // Calculate ratio to use.
    double ratio = (static_cast<double>(geValidatedOther->GetMass()) / static_cast<double>(GetMass()));

	if (ratio < 0 ) {
		return;
	}
	// Now calculate speed.
	constexpr float speedConst = 66.25f;
	// Speed vec
	const vec3_t speedVector = vec3_t{ 
		(float)speedConst * (float)ratio, 
		(float)speedConst * (float)ratio, 
		(float)speedConst * (float)ratio 
	};
	//const vec3_t speedVector = vec3_t{ 
	//	(float)speedConst , 
	//	(float)speedConst , 
	//	(float)speedConst 
	//};

    // Calculate direction.
    vec3_t damageDirection = vec3_normalize( vec3_negate( GetOrigin() - geValidatedOther->GetOrigin() ) );
	
	// Direction for velocities.
	const vec3_t velocityDirection = vec3_normalize( damageDirection );
	const vec3_t invVelocityDirection = vec3_negate( vec3_normalize ( damageDirection ) );
	
	// Angular Force.
	const vec3_t angularForce = velocityDirection * speedVector;

	// Force.
	const vec3_t force = invVelocityDirection * speedVector;

	// Adjust angular velocity.
	const vec3_t oldAngularVelocity = GetAngularVelocity();
	SetAngularVelocity( oldAngularVelocity + angularForce );
	
	// Calculate new velocity.
	const vec3_t oldVelocity = GetVelocity();
	SetVelocity( oldVelocity + force ); // SG_BounceVelocity( GetVelocity(), angularForce, 1.025 ) );

    // Last but not least, move a step ahead.
//    SVG_StepMove_Walk(this, yaw, (30.0 / static_cast<double>(BASE_FRAMEDIVIDER) * ratio * FRAMETIME_S.count()));
}

void MiscSphereBall::SphereBallStop() {
	//gi.DPrintf("SphereBall STOP! :-)\n");
}

/**
*	@brief	
**/
void MiscSphereBall::SphereBallTakeDamage( IServerGameEntity *other, float kick, int32_t damage, const vec3_t &damageDirection ) {
	// Direction for velocities.
	const vec3_t velocityDirection = vec3_normalize( damageDirection );
	const vec3_t invVelocityDirection = vec3_negate( vec3_normalize ( damageDirection ) );
	
	// Kick vector.
	const vec3_t kickVector = { kick, kick, kick };
	// Dmg vector.
	const vec3_t damageVector = { (float)damage, (float)damage, (float)damage };


	// Divide kick by damage
	const vec3_t kickDamageDiv = kickVector / damageVector;

	// Now calculate speed.
	constexpr float speedConst = 6.25f;
	const vec3_t speedVector = vec3_t{ speedConst, speedConst, speedConst } * kickDamageDiv;


	// Angular Force.
	const vec3_t angularForce = invVelocityDirection * speedVector;

	// Force.
	const vec3_t force = velocityDirection * speedVector;

	// Adjust angular velocity.
	const vec3_t oldAngularVelocity = GetAngularVelocity();
	SetAngularVelocity( oldAngularVelocity + angularForce );
	
	// Calculate new velocity.
	const vec3_t oldVelocity = GetVelocity();
	SetVelocity( oldVelocity + force ); // SG_BounceVelocity( GetVelocity(), angularForce, 1.025 ) );}
}

//
//===============
// MiscSphereBall::SpawnDebris1Chunk
// 
// Function to spawn "debris1/tris.md2" chunks.
//===============
//
void MiscSphereBall::SpawnDebris1Chunk() {
    // Acquire a pointer to the game world.
    ServerGameWorld* gameworld = GetGameWorld();

    // Speed to throw debris at.
    float speed = 1.5 * (float)GetDamage() / 200.0f;

    // Calculate random direction vector.
    vec3_t randomDirection = {
        RandomRangef(-1.f, 1.f), //crandom(),
        RandomRangef(-1.f, 1.f),//crandom(),
        RandomRangef(-1.f, 1.f),//crandom()      
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Throw debris!
    gameworld->ThrowDebris(this, 1, origin, speed, GetDamage() );
}


//
//===============
// MiscSphereBall::SpawnDebris2Chunk
//
// Function to spawn "debris2/tris.md2" chunks.
//===============
//
void MiscSphereBall::SpawnDebris2Chunk() {
    // Speed to throw debris at.
    float speed = 2.f * GetDamage() / 200.f;

    // Calculate random direction vector.
    vec3_t randomDirection = {
        RandomRangef(-1.f, 1.f), //crandom(),
        RandomRangef(-1.f, 1.f),//crandom(),
        RandomRangef(-1.f, 1.f),//crandom()      
    };

    // Calculate origin to spawn them at.
    vec3_t origin = GetOrigin() + randomDirection * GetSize();

    // Last but not least, throw debris.
    GetGameWorld()->ThrowDebris(this, 2, origin, speed, GetDamage() );
}

//
//===============
// MiscSphereBall::SpawnDebris3Chunk
// 
// Function to spawn "debris3/tris.md2" chunks.
//===============
//
void MiscSphereBall::SpawnDebris3Chunk(const vec3_t &origin) {
    // Speed to throw debris at.
    float speed = 1.75 * (float)GetDamage() / 200.0f;

    // Throw debris!
    GetGameWorld()->ThrowDebris(this, 3, origin, speed, GetDamage() );
}
