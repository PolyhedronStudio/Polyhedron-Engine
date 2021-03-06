/*
// LICENSE HERE.

//
// BlasterBolt.cpp
//
//
*/
#include "../../ServerGameLocals.h"      // SVGame.
#include "../../Effects.h"              // Effects.
#include "../../Entities.h"             // Entities.
#include "../../Player/Client.h"        // Player Client functions.
#include "../../Player/Animations.h"    // Include Player Client Animations.
#include "../../Utilities.h"            // Util funcs.

#include "../../Gamemodes/IGamemode.h"
// Class Entities.
#include "BlasterBolt.h"

// Constructor/Deconstructor.
BlasterBolt::BlasterBolt(PODEntity *svEntity) 
    : SVGBaseEntity(svEntity) {

}

//
//===============
// SVGBasePlayer::Precache
//
//===============
//
void BlasterBolt::Precache() {
    Base::Precache();
}

//
//===============
// SVGBasePlayer::Spawn
//
//===============
//
void BlasterBolt::Spawn() {
    // Spawn.
    Base::Spawn();
}

//
//===============
// BlasterBolt::Respawn
//
//===============
//
void BlasterBolt::Respawn() {
    Base::Respawn();
}

//
//===============
// BlasterBolt::PostSpawn
//
//===============
//
void BlasterBolt::PostSpawn() {
    Base::PostSpawn();
}

//
//===============
// BlasterBolt::Think
//
//===============
//
void BlasterBolt::Think() {
    // Parent class Think.
    Base::Think();
}

//
//===============
// BlasterBolt::SpawnKey
//
// BlasterBolt spawn key handling.
//===============
//
void BlasterBolt::SpawnKey(const std::string& key, const std::string& value) {
    // Parent class spawnkey.
    Base::SpawnKey(key, value);
}

//
//===============
// BlasterBolt::BlasterBoltTouch
//
// 'Touch' callback, to hurt the entities touching it.
//===============
//
void BlasterBolt::BlasterBoltTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
    if (!self || !other) { // Plane and Surf can be NULL
        Remove();
        return;
    }

    if (other == self->GetOwner()) {
        return;
    }

    if (surf && (surf->flags & SurfaceFlags::Sky)) {
        Remove(); 
        return;
    }

    if (self->GetOwner()->GetClient()) {
        // Play impact noise.
        //SVG_PlayerNoise(self->GetOwner(), self->GetOrigin(), PNOISE_IMPACT);
    }

    // Does the other entity take damage?
    if (other->GetTakeDamage()) {
        // Determine its means of death.
        int32_t meansOfDeath = MeansOfDeath::Blaster;

        // Fix for when there is no plane to base a normal of. (Taken from Yamagi Q2)
        if (plane) {
            GetGameMode()->InflictDamage(other, self, self->GetOwner(), self->GetVelocity(), self->GetOrigin(),
                plane->normal, self->GetDamage(), 1, DamageFlags::EnergyBasedWeapon, meansOfDeath);
        } else {
            GetGameMode()->InflictDamage(other, self, self->GetOwner(), self->GetVelocity(), self->GetOrigin(),
                vec3_zero(), self->GetDamage(), 1, DamageFlags::EnergyBasedWeapon, meansOfDeath);
        }

    } else {
        gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
        gi.MSG_WriteUint8(TempEntityEvent::Blaster );//WriteByte(TempEntityEvent::Blaster);
        gi.MSG_WriteVector3( self->GetOrigin(), false );//WriteVector3(self->GetOrigin());

        if (!plane) {
            gi.MSG_WriteUint8(DirectionToByte(vec3_zero()));//gi.MSG_WriteVector3( vec3_zero(), false );//WriteVector3(vec3_zero());
        } else {
            gi.MSG_WriteUint8(DirectionToByte(plane->normal));//gi.MSG_WriteVector3( plane->normal, false );//WriteVector3(plane->normal);
        }

        vec3_t origin = self->GetOrigin();
        gi.Multicast(origin, Multicast::PVS);
    }

    // Queue the entity for removal. 
    Remove();
}