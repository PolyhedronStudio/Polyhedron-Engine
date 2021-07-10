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
    vec3_t velocityScale = { 1.f, 1.f, 1.f };

    // Is it an organic gib type?
    if (type == GIB_ORGANIC) {
        // Then we pick a different movetype ;-)
        gibClassEntity->SetMoveType(MoveType::Toss);

        // Most of all, we setup a touch callback too ofc.
        gibClassEntity->SetTouchCallback(&GibEntity::GibEntityTouch);

        // Adjust the velocity scale.
        velocityScale = vec3_t{
            0.5f, 0.5f, 0.5f
        };
    } else {
        // Pick a different movetype, bouncing. No touch callback :)
        gibClassEntity->SetMoveType(MoveType::Bounce);
    }

    // Calculate the velocity for the given damage.
    gibClassEntity->CalculateVelocity(self, damage);

    // Setup the Gib think function and its think time.
    gibClassEntity->SetThinkCallback(&SVGBaseEntity::SVGBaseEntityThinkFree);
    gibClassEntity->SetNextThinkTime(level.time + 10 + random() * 10);

    // Link entity into the world.
    gibClassEntity->LinkEntity();
}

void ThrowHead(Entity *self, const char *gibname, int damage, int type)
{
    //vec3_t  vd;
    //float   vscale;

    //self->state.skinNumber = 0;
    //self->state.frame = 0;
    //VectorClear(self->mins);
    //VectorClear(self->maxs);

    //self->state.modelIndex2 = 0;
    //gi.SetModel(self, gibname);
    //self->solid = Solid::Not;
    //self->state.effects |= EntityEffectType::Gib;
    //self->state.sound = 0;
    //self->flags |= EntityFlags::NoKnockBack;
    //self->serverFlags &= ~EntityServerFlags::Monster;
    //self->takeDamage = TakeDamage::Yes;
    ////self->Die = gib_die;

    //if (type == GIB_ORGANIC) {
    //    self->moveType = MoveType::Toss;
    //    //self->Touch = gib_touch;
    //    vscale = 0.5;
    //} else {
    //    self->moveType = MoveType::Bounce;
    //    vscale = 1.0;
    //}

    //// Calculate velocity for given damage.
    //vd = SVG_VelocityForDamage(damage);
    //VectorMA(self->velocity, vscale, vd, self->velocity);
    //ClipGibVelocity(self);

    //self->angularVelocity[vec3_t::Yaw] = crandom() * 600;

    ////self->Think = SVG_FreeEntity;
    //self->nextThinkTime = level.time + 10 + random() * 10;

    //gi.LinkEntity(self);
}


void ThrowClientHead(Entity *self, int damage)
{
    //vec3_t  vd;
    //const char    *gibname; // C++20 VKPT: added const.

    //if (rand() & 1) {
    //    gibname = "models/objects/gibs/head2/tris.md2";
    //    self->state.skinNumber = 1;        // second skin is player
    //} else {
    //    gibname = "models/objects/gibs/skull/tris.md2";
    //    self->state.skinNumber = 0;
    //}

    //self->state.origin[2] += 32;
    //self->state.frame = 0;
    //gi.SetModel(self, gibname);
    //VectorSet(self->mins, -16, -16, 0);
    //VectorSet(self->maxs, 16, 16, 16);

    //self->takeDamage = TakeDamage::No;
    //self->solid = Solid::Not;
    //self->state.effects = EntityEffectType::Gib;
    //self->state.sound = 0;
    //self->flags |= EntityFlags::NoKnockBack;

    //self->moveType = MoveType::Bounce;
    //// Calculate velocity for given damage.
    //vd = SVG_VelocityForDamage(damage);
    //VectorAdd(self->velocity, vd, self->velocity);

    //if (self->client) { // bodies in the queue don't have a client anymore
    //    self->client->animation.priorityAnimation = PlayerAnimation::Death;
    //    self->client->animation.endFrame = self->state.frame;
    //} else {
    //    //self->Think = NULL;
    //    self->nextThinkTime = 0;
    //}

    //gi.LinkEntity(self);
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