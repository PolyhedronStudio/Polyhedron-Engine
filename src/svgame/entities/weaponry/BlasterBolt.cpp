/*
// LICENSE HERE.

//
// BlasterBolt.cpp
//
//
*/
#include "../../g_local.h"              // SVGame.
#include "../../effects.h"              // Effects.
#include "../../entities.h"             // Entities.
#include "../../player/client.h"        // Player Client functions.
#include "../../player/animations.h"    // Include Player Client Animations.
#include "../../player/view.h"          // Include Player View functions..
#include "../../utils.h"                // Util funcs.

// Class Entities.
#include "../base/SVGBaseEntity.h"
#include "BlasterBolt.h"

// Constructor/Deconstructor.
BlasterBolt::BlasterBolt(Entity* svEntity) 
    : SVGBaseEntity(svEntity) {

}
BlasterBolt::~BlasterBolt() {

}

//
//===============
// PlayerClient::Precache
//
//===============
//
void BlasterBolt::Precache() {
    Base::Precache();
}

//
//===============
// PlayerClient::Spawn
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
void BlasterBolt::BlasterBoltTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
    // N&C: From Yamagi Q2, this seems to resolve our random crashes at times.
    if (!self || !other) { // Plane and Surf can be NULL
        Remove();
        return;
    }

    if (other == self->GetOwner()) {
        return;
    }

    if (surf && (surf->flags & SURF_SKY)) {
        Remove(); 
        return;
    }

    if (self->GetOwner()->GetClient()) {
        // Play impact noise.
        SVG_PlayerNoise(self->GetOwner(), self->GetOrigin(), PNOISE_IMPACT);
    }

    // Does the other entity take damage?
    if (other->GetTakeDamage()) {
        // Determine its means of death.
        int32_t meansOfDeath = MeansOfDeath::Blaster;

        // Fix for when there is no plane to base a normal of. (Taken from Yamagi Q2)
        if (plane) {
            SVG_InflictDamage(other, self, self->GetOwner(), self->GetVelocity(), self->GetOrigin(),
                plane->normal, self->GetDamage(), 1, DamageFlags::EnergyBasedWeapon, meansOfDeath);
        } else {
            SVG_InflictDamage(other, self, self->GetOwner(), self->GetVelocity(), self->GetOrigin(),
                vec3_zero(), self->GetDamage(), 1, DamageFlags::EnergyBasedWeapon, meansOfDeath);
        }

    } else {
        gi.WriteByte(SVG_CMD_TEMP_ENTITY);
        gi.WriteByte(TempEntityEvent::Blaster);
        gi.WriteVector3(self->GetOrigin());

        if (!plane) {
            gi.WriteVector3(vec3_zero());
        } else {
            gi.WriteVector3(plane->normal);
        }

        vec3_t origin = self->GetOrigin();
        gi.Multicast(origin, MultiCast::PVS);
    }

    // Queue the entity for removal. 
    Remove();
}