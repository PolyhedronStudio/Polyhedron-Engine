/***
*
*	License here.
*
*	@file
* 
*   Local Client-Side Debris Entity, updates locally, does not receive state updates
*	from over the wire. 
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
//! ClientGame World.
#include "Game/Client/World/ClientGameWorld.h"
//! Particle effects.
#include "Game/Client/Effects/ParticleEffects.h"

#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0f * (random() - 0.5f))


#include "Game/Shared/Physics/Physics.h"

// Class Entities.
#include "Game/Client/Entities/Base/CLGBaseLocalEntity.h"
#include "Game/Client/Entities/DebrisEntity.h"


//! Constructor/Deconstructor.
DebrisEntity::DebrisEntity(PODEntity *podEntity)
    : CLGBaseLocalEntity(podEntity) {

}

static inline const vec3_t CalculateDamageVelocity(int32_t damage) {
    // Pick random velocities.
    vec3_t velocity = {
        100.0f * crandom(),
        100.0f * crandom(),
        200.0f + 100.0f * random()
    };

    // Scale velocities.
    if (damage < 50)
        velocity = vec3_scale(velocity, 0.7f);
    else
        velocity = vec3_scale(velocity, 1.2f);

    // Return.
    return velocity;
}

/**
*   @brief  Used by game modes to spawn server side gibs.
*   @param  debrisser The entity that is about to spawn debris.
**/
DebrisEntity* DebrisEntity::Create( GameEntity* debrisser, const std::string& debrisModel, const vec3_t& origin, const float speed, const int32_t damage ) {
	//PODEntity *localDebrisEntity = GetGameWorld()->GetUnusedPODEntity(false);

	// Create a gib entity.
    DebrisEntity *debrisEntity = GetGameWorld()->CreateGameEntity<DebrisEntity>(nullptr, true, false);


	// Set actual size of 
    vec3_t size = vec3_scale(debrisser->GetSize() , 0.5f);
    debrisEntity->SetSize(size);

    // Set the origin.
    debrisEntity->SetOrigin(origin);
	debrisEntity->SetInUse(true);
    // Set the model.
    debrisEntity->SetModel(debrisModel);
	//const float velocityScale = 1.f;
 //   // Comment later...
    const vec3_t velocityDamage = CalculateDamageVelocity( damage );

	//vec3_t velocity = vec3_t{ speed, speed, speed };
 //   // Reassign 'velocityDamage' and multiply 'self->GetVelocity' to scale, and then
 //   // adding it on to 'velocityDamage' its old value.
 //   vec3_t gibVelocity = vec3_fmaf( velocityDamage, velocityScale,  velocity );

 //   // Be sure to clip our velocity, just in case.
 //   ClipGibVelocity(gibVelocity);
	//debrisEntity->SetMoveType(MoveType::TossSlide);
 //   debrisEntity->SetVelocity(gibVelocity);
    // Calculate and set the velocity.
    const vec3_t velocity = { 250.f + (Randomf() * 500.f), 250.f + (Randomf() * 500.f), 250.f + (Randomf() * 900.f) };
    debrisEntity->SetVelocity( vec3_fmaf( debrisser->GetVelocity(), speed, velocity ) );

    // Set Movetype and Solid.
    debrisEntity->SetMoveType(MoveType::TossSlideBox);
    debrisEntity->SetSolid(Solid::BoundingBox);

    // Generate angular velocity.
    debrisEntity->SetAngularVelocity({
		100.f + (Randomf() * 1100.f), 
		100.f + (Randomf() * 1100.f), 
		100.f + (Randomf() * 1500.f)
	});

    // Setup the Gib think function so it'll check for ground and add gravity.
    debrisEntity->SetThinkCallback( &DebrisEntity::DebrisEntityThink );
	debrisEntity->SetNextThinkTime( level.time + FRAMERATE_MS );

    // Setup the other properties.
    debrisEntity->SetAnimationFrame( 0 );
	debrisEntity->SetFlags( 0 );
    debrisEntity->SetTakeDamage( TakeDamage::Yes );
    debrisEntity->SetDieCallback( &DebrisEntity::DebrisEntityDie );
	debrisEntity->SetTouchCallback( &DebrisEntity::DebrisEntityTouch );

    // Link it up.
    debrisEntity->LinkEntity();

    // Return the debris entity.
    return debrisEntity;
}

/**
*	@brief	Think callback, checks for ground, applies gravity, and sets a free callback after deathTime passed.
**/
void DebrisEntity::DebrisEntityThink() {   
	// Check for ground.
	SG_CheckGround( this );
	// Apply gravity.
	SG_AddGravity( this );

	// Set free think, IF, deathtime has passed.
	if ( deathTime != GameTime::zero() && level.time >= deathTime ) {
		SetThinkCallback( &CLGBaseLocalEntity::CLGBaseLocalEntityThinkFree );
	// Default callback otherwise.
	} else {
		SetThinkCallback( &DebrisEntity::DebrisEntityThink );
	}
	SetNextThinkTime( level.time + FRAMERATE_MS );
}

/**
*	@brief	Die callback.
**/
void DebrisEntity::DebrisEntityDie( GameEntity* inflictor, GameEntity* attacker, int32_t damage, const vec3_t& point ) {
    // Save to queue for removal.
    Remove();
}

/**
*	@brief	Touch callback.
**/
void DebrisEntity::DebrisEntityTouch( GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
    // Did we get a plane passed?
    if ( plane ) {
		// Reset touch callback to nullptr.
		SetTouchCallback(nullptr);

		// Calculate new angles to 'stop' at when hitting the plane.
        const vec3_t normalAngles = vec3_euler( plane->normal );
		vec3_t vRight = vec3_zero();
        vec3_vectors( normalAngles, NULL, &vRight, NULL );

		// Set new angles to match plane.
        const vec3_t newDirection = vec3_cross( normalAngles, vRight );
		const vec3_t oldAngles = GetAngles();
		SetAngles( vec3_euler( vec3_fmaf( oldAngles, FRAMETIME_S.count(), newDirection ) ) );

		// Disable particle effects and zero out angular velocity.
		SetEffects(0);
		SetAngularVelocity( vec3_zero() );

		// Initiate the "deathTime" so this debris will fade away sooner or later.
		deathTime = duration_cast<GameTime>( level.time + 5s + random() * 5s );

		// And make sure that our next think is the regular debris thinking.
        SetThinkCallback( &DebrisEntity::DebrisEntityThink );
        SetNextThinkTime(level.time + FRAMETIME_S);
    }
}