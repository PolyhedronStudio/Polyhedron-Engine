// LICENSE HERE.

//
// misc.c
//
//
// Contains misc definitions.
//

#include "g_local.h"         // Include SVGame funcs.
#include "entities.h"
#include "utils.h"           // Include Utilities funcs.
#include "effects.h"

#include "entities/base/SVGBaseEntity.h"
#include "entities/base/PlayerClient.h"
#include "entities/base/DebrisEntity.h"
#include "entities/base/GibEntity.h"

//=====================================================

//
//=================
// Gibs
//=================
//
void ThrowGib(SVGBaseEntity*self, const char *gibname, int damage, int type)
{
    // Create a gib entity.
    GibEntity* gibClassEntity = SVG_CreateClassEntity<GibEntity>();

    // Set size.
    vec3_t size = vec3_scale(self->GetSize(), 0.5f);
    gibClassEntity->SetSize(size);
    
    // Generate the origin to start from.
    vec3_t origin = self->GetAbsoluteMin() + self->GetSize();

    // Add some random values to it, so they all differ.
    origin.x += crandom() * size.x;
    origin.y += crandom() * size.y;
    origin.z += crandom() * size.z;

    // Set the origin.
    gibClassEntity->SetOrigin(origin);

    // Set the model.
    gibClassEntity->SetModel(gibname);

    // Set solid and other properties.
    gibClassEntity->SetSolid(Solid::Not);
    gibClassEntity->SetEffects(gibClassEntity->GetEffects() | EntityEffectType::Gib);
    gibClassEntity->SetFlags(gibClassEntity->GetFlags() | EntityFlags::NoKnockBack);
    gibClassEntity->SetTakeDamage(TakeDamage::Yes);
    gibClassEntity->SetDieCallback(&GibEntity::GibEntityDie);

    // Default velocity scale for non organic materials.
    float velocityScale = 1.f;

    // Is it an organic gib type?
    if (type == GIB_ORGANIC) {
        // Then we pick a different movetype ;-)
        gibClassEntity->SetMoveType(MoveType::Toss);

        // Most of all, we setup a touch callback too ofc.
        gibClassEntity->SetTouchCallback(&GibEntity::GibEntityTouch);

        // Adjust the velocity scale.
        velocityScale = 0.5f;
    } else {
        // Pick a different movetype, bouncing. No touch callback :)
        gibClassEntity->SetMoveType(MoveType::Bounce);
    }

    // Calculate the velocity for the given damage, fetch its scale.
    vec3_t velocityForDamage; // WID: Ugly, but ... yeah, gets set below.
    velocityScale = gibClassEntity->CalculateVelocityForDamage(self, damage, velocityForDamage);

    // Reassign 'velocityForDamage' and multiply 'self->GetVelocity' to scale, and then 
    // adding it on to 'velocityForDamage' its old value.
    velocityForDamage = vec3_fmaf(self->GetVelocity(), velocityScale, velocityForDamage);

    // Be sure to clip our velocity, just in case.
    gibClassEntity->ClipGibVelocity(velocityForDamage);

    // Last but not least, set our velocity.
    gibClassEntity->SetVelocity(velocityForDamage);

    // Generate angular velocity.
    vec3_t angularVelocity = {
        random() * 600.f,
        random() * 600.f,
        random() * 600.f
    };

    // Set angular velocity.
    gibClassEntity->SetAngularVelocity(angularVelocity);

    // Setup the Gib think function and its think time.
    gibClassEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    gibClassEntity->SetNextThinkTime(level.time + 10 + random() * 10);

    // Link entity into the world.
    gibClassEntity->LinkEntity();
}

void ThrowClientHead(SVGBaseEntity* self, int damage) {
    vec3_t  vd;

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
    self->SetFrame(0);

    // Set mins/maxs.
    self->SetMins(vec3_t{ -16.f, -16.f, 0.f });
    self->SetMaxs(vec3_t{ 16.f, 16.f, 0.f });

    // Set MoveType and Solid.
    self->SetSolid(Solid::Not);
    self->SetMoveType(MoveType::Bounce);

    // Other properties.
    self->SetTakeDamage(TakeDamage::No);
    self->SetEffects(EntityEffectType::Gib);
    self->SetSound(0);
    self->SetFlags(EntityFlags::NoKnockBack);

    // Calculate the velocity for the given damage, fetch its scale.
    vec3_t velocityForDamage; // WID: Ugly, but ... yeah, gets set below.
    float velocityScale = game.gameMode->CalculateVelocityForDamage(self, damage, velocityForDamage);

    // Add the velocityForDamage up to the current velocity.
    self->SetVelocity(self->GetVelocity() + velocityForDamage);

    //VectorAdd(self->velocity, vd, self->velocity);

    // Bodies in the queue don't have a client anymore.
    GameClient* client = self->GetClient();
    if (client) {
        client->animation.priorityAnimation = PlayerAnimation::Death;
        client->animation.endFrame = self->GetFrame();
    } else {
        self->SetThinkCallback(nullptr);
        self->SetNextThinkTime(0);
    }

    // Relink entity, this'll make it... be "not around", but in the "queue".
    self->LinkEntity();
}

//=================
// SVG_ThrowDebris
// 
// Thorws a debris piece around.
//=================
void SVG_ThrowDebris(SVGBaseEntity *self, const char *modelname, float speed, const vec3_t &origin) // C++20: STRING: Added const to char*
{
    // Chunk Entity.
    SVGBaseEntity* chunkEntity = SVG_CreateClassEntity<DebrisEntity>();

    // Set the origin.
    chunkEntity->SetOrigin(origin);

    // Set the model.
    chunkEntity->SetModel(modelname);

    // Calculate and set the velocity.
    vec3_t velocity = {
        100.f * crandom(),
        100.f * crandom(),
        100.f + 100.f * crandom()
    };
    chunkEntity->SetVelocity(vec3_fmaf(self->GetVelocity(), speed, velocity));

    // Set Movetype and Solid.
    chunkEntity->SetMoveType(MoveType::Bounce);
    chunkEntity->SetSolid(Solid::Not);

    // Calculate and set angular velocity.
    vec3_t angularVelocity = {
        random() * 600,
        random() * 600,
        random() * 600
    };
    chunkEntity->SetAngularVelocity(angularVelocity);

    // Set up the thinking machine.
    chunkEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    chunkEntity->SetNextThinkTime(level.time + 5 + random() * 5);

    // Setup the other properties.
    chunkEntity->SetFrame(0);
    chunkEntity->SetFlags(0);
    chunkEntity->SetTakeDamage(TakeDamage::Yes);
    chunkEntity->SetDieCallback(&DebrisEntity::DebrisEntityDie);

    // Link it up.
    chunkEntity->LinkEntity();
}

//=================
// BecomeExplosion1
// 
// Sends an explosion effect as a TE cmd, and queues the entity up for removal.
//=================
void BecomeExplosion1(SVGBaseEntity *self)
{
    // Fetch origin.
    vec3_t origin = self->GetOrigin();

    // Execute a TE effect.
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(TempEntityEvent::Explosion1);
    gi.WriteVector3(origin);
    gi.Multicast(origin, MultiCast::PVS);

    // Queue for removal.
    self->Remove();
    //SVG_FreeEntity(self->GetServerEntity());
}

//=================
// BecomeExplosion2
// 
// Sends an explosion effect as a TE cmd, and queues the entity up for removal.
//=================
void BecomeExplosion2(SVGBaseEntity*self)
{
    // Fetch origin.
    vec3_t origin = self->GetOrigin();

    // Execute a TE effect.
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(TempEntityEvent::Explosion2);
    gi.WriteVector3(origin);
    gi.Multicast(origin, MultiCast::PVS);

    // Queue for removal.
    self->Remove();
    //SVG_FreeEntity(self->GetServerEntity());
}