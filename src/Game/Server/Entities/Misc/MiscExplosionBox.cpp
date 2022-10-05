/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
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
#include "MiscExplosionBox.h"

#include "../../Gamemodes/IGamemode.h"
#include "../../World/ServerGameWorld.h"

//
// Constructor/Deconstructor.
//
MiscExplosionBox::MiscExplosionBox(PODEntity *svEntity) 
    : Base(svEntity) {

}


//
// Interface functions. 
//
//
//===============
// MiscExplosionBox::Precache
//
//===============
//
void MiscExplosionBox::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache actual barrel model.
    SVG_PrecacheModel("models/env/barrels/barrel_red.iqm");

    // Precache the debris.
    SVG_PrecacheModel("models/objects/debris1/tris.md2");
    SVG_PrecacheModel("models/objects/debris2/tris.md2");
    SVG_PrecacheModel("models/objects/debris3/tris.md2");
}

//
//===============
// MiscExplosionBox::Spawn
//
//===============
//
void MiscExplosionBox::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    //SetRenderEffects(GetRenderEffects() | RenderEffects::DebugBoundingBox);

    // Set solid.
    SetSolid( Solid::OctagonBox );
    // Set clip mask.
    SetClipMask( BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid );
    // Set the barrel model, and model index.
    SetModel( "models/env/barrels/barrel_red.iqm" );
    // Set the bounding box.
    SetBoundingBox(
        // Mins.
        { -16.f, -16.f, 0.f },
        // Maxs.
        { 16.f, 16.f, 58.f }
    );

    // Set move type.
    SetMoveType( MoveType::TossSlideBox );
	// Since this is a "monster", after all...
    SetServerFlags(EntityServerFlags::Monster);

    // Set default values in case we have none.
    if ( !GetMass() ) {
        SetMass( 100 );
    }
    if ( !GetHealth() ) {
        SetHealth(80);
    }
    if ( !GetDamage() ) {
        SetDamage( 150 );
    }

    // We need it to take damage in case we want it to explode.
    SetTakeDamage( TakeDamage::Yes );

    // Setup our MiscExplosionBox callbacks.
    SetUseCallback( &MiscExplosionBox::ExplosionBoxUse );
	SetDieCallback( &MiscExplosionBox::ExplosionBoxDie );
    SetTouchCallback( &MiscExplosionBox::ExplosionBoxTouch );

	// Physics callbacks.
	SetStopCallback(&MiscExplosionBox::ExplosionBoxStop);

    // Setup the next think time.
    SetNextThinkTime(level.time + 2.f * FRAMETIME_S);
    SetThinkCallback(&MiscExplosionBox::ExplosionBoxDropToFloor);

    // Link the entity to world, for collision testing.
    LinkEntity();
}

//
//===============
// MiscExplosionBox::Respawn
//
//===============
//
void MiscExplosionBox::Respawn() {
    Base::Respawn();
}

//
//===============
// MiscExplosionBox::PostSpawn
//
//===============
//
void MiscExplosionBox::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// MiscExplosionBox::Think
//
//===============
//
void MiscExplosionBox::Think() {
    // Always call parent class method.
    Base::Think();
}

//===============
// MiscExplosionBox::SpawnKey
//
//===============
void MiscExplosionBox::SpawnKey(const std::string& key, const std::string& value) {
	Base::SpawnKey(key, value);
}


//
// Callback Functions.
//

// ==============
// MiscExplosionBox::ExplosionBoxUse
// 
// So that mappers can trigger this entity in order to blow it up
// ==============
void MiscExplosionBox::ExplosionBoxUse( IServerGameEntity* caller, IServerGameEntity* activator )
{
    ExplosionBoxDie( caller, activator, 999, GetOrigin() );
}

void MiscExplosionBox::ExplosionBoxThink(void) {
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
		SetAngularVelocity( SG_CalculateRotationalFriction( this ) );
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
	SetThinkCallback( &MiscExplosionBox::ExplosionBoxThink );
}

//
//===============
// MiscExplosionBox::ExplosionBoxDropToFloor
//
// Think callback, to execute the needed physics for this pusher object.
//===============
//
void MiscExplosionBox::ExplosionBoxDropToFloor(void) {
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
	SetThinkCallback( &MiscExplosionBox::ExplosionBoxThink );
}

//
//===============
// MiscExplosionBox::MiscExplosionBoxExplode
//
// 'Think' callback that is set when the explosion box is exploding.
// (Has died due to taking damage.)
//===============
//
void MiscExplosionBox::MiscExplosionBoxExplode(void) {
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
    //SetThinkCallback(&MiscExplosionBox::SVGBaseEntityThinkFree);
	Remove();
}

//
//===============
// MiscExplosionBox::ExplosionBoxDie
//
// 'Die' callback, the explosion box has been damaged too much.
//===============
//
void MiscExplosionBox::ExplosionBoxDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) {
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


    // Set think function.
    SetThinkCallback(&MiscExplosionBox::MiscExplosionBoxExplode);
}

//
//===============
// MiscExplosionBox::ExplosionBoxTouch
//
// 'Touch' callback, to calculate the direction to move into.
//===============
//
void MiscExplosionBox::ExplosionBoxTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// Validate 'other' first.
	GameEntity *geValidatedOther = ServerGameWorld::ValidateEntity(other);

    // Safety checks.
    if ( !geValidatedOther || geValidatedOther == this ) {
	    return;
    }

    // Ground entity checks.
	GameEntity *geGroundEntity = ServerGameWorld::ValidateEntity( geValidatedOther->GetGroundEntityHandle() );

    if ( !geGroundEntity || geGroundEntity == this ) {
	    return;
    }

    // Calculate ratio to use.
    double ratio = (static_cast<double>(geValidatedOther->GetMass()) / static_cast<double>(GetMass()));

    // Calculate direction.
    vec3_t dir = GetOrigin() - geValidatedOther->GetOrigin();

    // Calculate yaw to use based on direction.
    double yaw = vec3_to_yaw(dir);

    // Last but not least, move a step ahead.
//    SVG_StepMove_Walk(this, yaw, (30.0 / static_cast<double>(BASE_FRAMEDIVIDER) * ratio * FRAMETIME_S.count()));
}

void MiscExplosionBox::ExplosionBoxStop() {
	//gi.DPrintf("ExplosionBox STOP! :-)\n");
}

//
//===============
// MiscExplosionBox::SpawnDebris1Chunk
// 
// Function to spawn "debris1/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris1Chunk() {
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
// MiscExplosionBox::SpawnDebris2Chunk
//
// Function to spawn "debris2/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris2Chunk() {
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
// MiscExplosionBox::SpawnDebris3Chunk
// 
// Function to spawn "debris3/tris.md2" chunks.
//===============
//
void MiscExplosionBox::SpawnDebris3Chunk(const vec3_t &origin) {
    // Speed to throw debris at.
    float speed = 1.75 * (float)GetDamage() / 200.0f;

    // Throw debris!
    GetGameWorld()->ThrowDebris(this, 3, origin, speed, GetDamage() );
}
