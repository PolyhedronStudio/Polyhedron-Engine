/***
*
*	License here.
*
*	@file
* 
*   Local Client-Side Gib Entity, updates locally, does not receive state updates
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
#include "Game/Client/Entities/GibEntity.h"


static int32_t sm_meat_index = 0;

static inline const vec3_t CalculateDamageVelocity(int32_t damage, const vec3_t &baseDamageVelocity) {
    // Pick random velocities.
    vec3_t velocity = {
        baseDamageVelocity.x * crandom(),
        baseDamageVelocity.y * crandom(),
        baseDamageVelocity.z + (baseDamageVelocity.z / 2.f) * random()
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
**/
GibEntity* GibEntity::Create(const vec3_t &origin, const vec3_t &size, const vec3_t &velocity, const std::string& gibModel, int32_t damage, int32_t gibType) {
    
	PODEntity *localGibEntity = GetGameWorld()->GetUnusedPODEntity(false);

	// Create a gib entity.
    GibEntity* gibEntity = GetGameWorld()->CreateGameEntity<GibEntity>(localGibEntity , false, false);

	if (!sm_meat_index) {
		//sm_meat_index = CLG_PrecacheModel("models/objects/gibs/sm_meat/tris.md2");
		sm_meat_index = CLG_PrecacheModel(gibModel);
	}

	// Set size. // TODO: Use size and getabsmin from somewhere I guess.
    const vec3_t gibSize = vec3_scale( size, 0.25f);
    gibEntity->SetSize(gibSize);

    // Generate the origin to start from.
    vec3_t newOrigin = origin + gibSize;

    //// Add some random values to it, so they all differ.
    //origin.x += crandom() * size.x;
    //origin.y += crandom() * size.y;
    //origin.z += crandom() * size.z;

    // Set the origin.
	gibEntity->SetOrigin(vec3_t{
		newOrigin.x + crandom() * gibSize.x,
		newOrigin.y + crandom() * gibSize.y,
		newOrigin.z + crandom() * gibSize.z,
	});

    // Set the model.
    gibEntity->SetModel( gibModel );

    // Set solid and other properties.
    gibEntity->SetSolid( Solid::OctagonBox );
    gibEntity->SetEffects( gibEntity->GetEffects() | EntityEffectType::Gib );
    gibEntity->SetClipMask( BrushContentsMask::MonsterSolid | BrushContentsMask::PlayerSolid );

	//gibEntity->SetFlags( gibEntity->GetFlags() | EntityFlags::NoKnockBack );
    //gibEntity->SetTakeDamage( 1 );
    //gibEntity->SetDieCallback( &GibEntity::GibEntityDie );

    // Default velocity scale for non organic materials.
    float velocityScale = 1.f;

    // Is it an organic gib type?
    if (gibType == GibType::Organic) {
    	// Different move type for organic gibs.
	    gibEntity->SetMoveType(MoveType::SlideBoxMove);
	    // Touch callback to stop the gib from bleeding as well as try and change into a halt.
		gibEntity->SetTouchCallback(&GibEntity::GibEntityTouch);
		// Mass.
		gibEntity->SetMass( 40 );
	    // Adjust the velocity scale.
	    velocityScale = 0.5f;
    } else {
	    // Most of all, we setup a touch callback too ofc.
		gibEntity->SetTouchCallback(&GibEntity::GibEntityTouch);

		// Pick a different movetype, bouncing. No touch callback :)
	    gibEntity->SetMoveType(MoveType::Bounce);
    }

    // Comment later...
    const vec3_t velocityDamage = CalculateDamageVelocity(damage, vec3_t{ 150.f, 150.f, 300.f });

    // Reassign 'velocityDamage' and multiply 'self->GetVelocity' to scale, and then
    // adding it on to 'velocityDamage' its old value.
    vec3_t gibVelocity = vec3_fmaf( velocityDamage, velocityScale, velocity );

    // Be sure to clip our velocity, just in case.
    gibEntity->ClipGibVelocity(gibVelocity);

    // Last but not least, set our velocity.
    gibEntity->SetVelocity(gibVelocity);
    
    // Generate angular velocity.
    gibEntity->SetAngularVelocity({
		100.f + (Randomf() * 950.f), 
		100.f + (Randomf() * 950.f), 
		100.f + (Randomf() * 1100.f)
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f)
	});

    // Setup the Gib think function so it'll check for ground and add gravity.
    gibEntity->SetThinkCallback( &GibEntity::GibEntityThink );
	gibEntity->SetNextThinkTime( level.time + FRAMERATE_MS );

    // Link entity into the world.
    gibEntity->LinkEntity();

    return gibEntity;
}


// Constructor/Deconstructor.
GibEntity::GibEntity(PODEntity *svEntity) : CLGBaseLocalEntity(svEntity) { }

//
//===============
// SVGBasePlayer::Precache
//
//===============
//
void GibEntity::Precache() {
    Base::Precache();
}

//
//===============
// SVGBasePlayer::Spawn
//
//===============
//
void GibEntity::Spawn() {
    // Spawn.
    Base::Spawn();
}

//
//===============
// GibEntity::Respawn
//
//===============
//
void GibEntity::Respawn() {
    Base::Respawn();
}

//
//===============
// GibEntity::PostSpawn
//
//===============
//
void GibEntity::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// GibEntity::Think
//
//===============
//
void GibEntity::Think() {
    // Parent class Think.
    Base::Think();
}

//===============
// GibEntity::SpawnKey
//
// GibEntity spawn key handling.
//===============
void GibEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//===============
// GibEntity::ClipGibVelocity
//
// Clips gib velocity, in case it goes out of control that is.
//===============
void GibEntity::ClipGibVelocity(vec3_t &velocity) {
    // Y Axis.
    if (velocity.x < -300)
        velocity.x = -300;
    else if (velocity.x > 300)
        velocity.x = 300;
    
    // Y Axis.
    if (velocity.y < -300)
        velocity.y = -300;
    else if (velocity.y > 300)
        velocity.y = 300;

    // Z Axis.
    if (velocity.z < 200)
        velocity.z = 200;
    else if (velocity.z > 750)
        velocity.y = 750;
}

//===============
// GibEntity::CalculateVelocityForDamage
//
// 'other' is the actual entity that is spawning these gibs.
//===============
//float GibEntity::CalculateVelocityForDamage(SVGBaseEntity *other, const int32_t damage, vec3_t &velocity) {
//    // Calculate the velocity based on the damage passed over.
//    velocity = {
//        100.f * crandom(),
//        100.f * crandom(),
//        200.f + 100.f * random()
//    };
//
//    // Set velocity scale value based on the intensity of the amount of 'damage'.
//    float velocityScale = 1.2f;
//    if (damage < 50)
//        velocityScale = 0.7f;
//    
//    // Calculate velocity for damage.
//    velocity = vec3_scale(velocity, 0.7);
//
//    // Return.
//    return velocityScale;
//}

//===============
// GibEntity::GibEntityThink
//
//===============
void GibEntity::GibEntityThink() {   
	// Increase frame and set a new think time.
    //SetAnimationFrame(GetAnimationFrame() + FRAMETIME_S.count());

    // Play frames for these meshes, cut the crap at frame 10.
    //if (GetAnimationFrame() == 10) {
		//SetEffects(0);
       // SetThinkCallback(&CLGBaseLocalEntity::CLGBaseLocalEntityThinkFree);
        //SetNextThinkTime(level.time + 8s + Randomui() * 10s);
    //}

	//const vec3_t avel = GetAngularVelocity();
	//if ( avel.x || avel.y || avel.z ) {
	//	SetAngles( GetAngles() +
	//		vec3_euler( vec3_normalize(GetAngularVelocity()) )
	//	);
	//} else {
	//	SetAngularVelocity( vec3_zero() );
	//}

	SG_CheckGround(this);
	SG_AddGravity(this);
	//const vec3_t avel = GetAngularVelocity();
	//if (!GetGroundEntityHandle() && (avel.x || avel.y || avel.z) ) {
	//	SetAngularVelocity( vec3_scale( GetAngularVelocity(), 0.98 ) );
	//} else {

	//}

	// Next Think.
	SetThinkCallback(&GibEntity::GibEntityThink);
    SetNextThinkTime(level.time + FRAMERATE_MS);
}

//===============
// GibEntity::GibEntityTouch
//
//===============
void GibEntity::GibEntityStopBleeding() {
	// Set effects to nothing.
	SetEffects(0);
	//SetAngularVelocity(vec3_zero());
	// Old, would remove it after some time. May want to reverse this later on.
	//gibEntity->SetThinkCallback(&CLGBaseLocalEntity::CLGBaseLocalEntityThinkFree);
    //gibEntity->SetNextThinkTime(level.time + 10s + GameTime(Randomui() * 10));

	// Now just call its regular thinking method for this frame, it'll perform addgravity checkground and reassign think
	GibEntityThink();
}
void GibEntity::GibEntityTouch(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
    vec3_t  right;

	//// If we don't have ground we keep bleeding.
 //   if (!ClientGameWorld::ValidateEntity(other))
 //       return;

    // Did we get a plane passed?
    if ( plane ) {
        //CLG_Sound(this, SoundChannel::Voice, gi.PrecacheSound("misc/fhit3.wav"), 1, Attenuation::Normal, 0);
		

    // Reset touch callback to nullptr.
    SetTouchCallback(nullptr);

		// Calculate new angles to 'stop' at when hitting the plane.
        vec3_t normalAngles = vec3_euler( plane->normal );
        vec3_vectors( normalAngles, NULL, &right, NULL );
		
		//SetAngles( vec3_euler( right ) );

		// New direction.
        vec3_t newDirection = vec3_cross( normalAngles, right );//vec3_dot( vec3_normalize( GetAngles() ), right );

		const vec3_t oldAngles = GetAngles();
		SetAngles( vec3_euler( vec3_fmaf( oldAngles, FRAMETIME_S.count(), newDirection ) ) );

        //if (GetModelIndex() == sm_meat_index) {
            //SetAnimationFrame(GetAnimationFrame() + Frametime(2.5).count());
		// Set effects to nothing.
		SetEffects(0);
		SetAngularVelocity( vec3_zero() );

        SetThinkCallback(&GibEntity::GibEntityThink);
        SetNextThinkTime(level.time + FRAMETIME_S);
        //}
    }
}

//===============
// GibEntity::GibEntityDie
//
// Savely call Remove so it queues up for removal without causing 
// serverEntity/gameEntity conflicts.
//===============
void GibEntity::GibEntityDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) {
    // Time to queue it for removal.
    Remove();
}