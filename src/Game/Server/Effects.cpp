// LICENSE HERE.

//
// misc.c
//
//
// Contains misc definitions.
//

#include "ServerGameLocals.h"         // Include SVGame funcs.
#include "Entities.h"
#include "Utilities.h"           // Include Utilities funcs.
#include "Effects.h"

// Game Mode interface.
#include "Gamemodes/IGamemode.h"

// Class Entities.
//#include "Entities/Base/SVGBasePlayer.h"
#include "Entities/Base/DebrisEntity.h"
#include "Entities/Base/GibEntity.h"

// World.
#include "World/ServerGameWorld.h"

//=================
// SVG_ThrowClientHead
// 
// Tosses a client head entity around.
//=================
void SVG_ThrowClientHead(SVGBasePlayer* self, int damage) {
    // Set model based on randomness.
    if (rand() & 1) {
        self->SetModel("models/objects/gibs/head2/tris.md2");
        self->SetSkinNumber(1); // second skin is player
    } else {
        // WID: This just seems odd to me.. but hey. Sure.
        self->SetModel("models/objects/gibs/skull/tris.md2");
        self->SetSkinNumber(0); // Original skin.
    }

    // Let's fetch origin, add some Z, get going.
    vec3_t origin = self->GetOrigin();
    origin.z += 32;
    self->SetOrigin(origin);

    // Set frame back to 0.
    self->SetAnimationFrame(0);

    // Set mins/maxs.
    self->SetMins(vec3_t{ -16.f, -16.f, 0.f });
    self->SetMaxs(vec3_t{ 16.f, 16.f, 0.f });

    // Set MoveType and Solid.
    self->SetSolid(Solid::Not);
    self->SetMoveType(MoveType::Bounce);

    // Other properties.
    self->SetTakeDamage(TakeDamage::No);
    self->Base::SetEffects(EntityEffectType::Gib);
    self->Base::SetSound(0);
    self->SetFlags(EntityFlags::NoKnockBack);

    // Calculate the velocity for the given damage, fetch its scale.
    vec3_t velocityDamage = GetGameMode()->CalculateDamageVelocity(damage);

    // Add the velocityDamage up to the current velocity.
    self->SetVelocity(self->GetVelocity() + velocityDamage);

    // Bodies in the queue don't have a client anymore.
    ServerClient* client = self->GetClient();
    if (client) {
        client->animation.priorityAnimation = PlayerAnimation::Death;
        client->animation.endFrame = self->GetAnimationFrame();
    } else {
        self->SetThinkCallback(nullptr);
        self->SetNextThinkTime(Frametime::zero());
    }

    // Relink entity, this'll make it... be "not around", but in the "queue".
    self->LinkEntity();
}

//=================
// SVG_BecomeExplosion1
// 
// Sends an explosion effect as a TE cmd, and queues the entity up for removal.
//=================
void SVG_BecomeExplosion1(IServerGameEntity *self)
{
	if (!self) {
		// Warn?
		return;
	}
    // Fetch origin.
    vec3_t origin = self->GetOrigin();

    // Execute a TE effect.
    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent); // Write Byte.
    gi.MSG_WriteUint8(TempEntityEvent::Explosion1); //WriteByte(TempEntityEvent::Explosion1);
    gi.MSG_WriteUint16(self->GetNumber()); //gi.MSG_WriteVector3(origin, false);
    gi.Multicast(origin, Multicast::PVS);

    // Queue for removal.
    //self->Remove();
    //SVG_FreeEntity(self->GetPODEntity());
}

//=================
// SVG_BecomeExplosion2
// 
// Sends an explosion effect as a TE cmd, and queues the entity up for removal.
//=================
void SVG_BecomeExplosion2(IServerGameEntity*self)
{
	if (!self) {
		// Warn?
		return;
	}

    // Fetch origin.
    vec3_t origin = self->GetOrigin();

    // Execute a TE effect.
    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    gi.MSG_WriteUint8(TempEntityEvent::Explosion2);//WriteByte(TempEntityEvent::Explosion2);
    gi.MSG_WriteUint16(self->GetNumber());//gi.MSG_WriteVector3(origin, false);
    gi.Multicast(origin, Multicast::PVS);
}