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

//=====================================================

void ClipGibVelocity(Entity *ent)
{
    //if (ent->velocity[0] < -300)
    //    ent->velocity[0] = -300;
    //else if (ent->velocity[0] > 300)
    //    ent->velocity[0] = 300;
    //if (ent->velocity[1] < -300)
    //    ent->velocity[1] = -300;
    //else if (ent->velocity[1] > 300)
    //    ent->velocity[1] = 300;
    //if (ent->velocity[2] < 200)
    //    ent->velocity[2] = 200; // always some upwards
    //else if (ent->velocity[2] > 500)
    //    ent->velocity[2] = 500;
}


//
//=================
// Gibs
//=================
//
void gib_think(Entity *self)
{
    self->state.frame++;
    self->nextThinkTime = level.time + FRAMETIME;

    if (self->state.frame == 10) {
        //self->Think = SVG_FreeEntity;
        self->nextThinkTime = level.time + 8 + random() * 10;
    }
}

void gib_touch(Entity *self, Entity *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t  normal_angles, right;

    if (!self->groundEntityPtr)
        return;

    //self->Touch = NULL;

    if (plane) {
        gi.Sound(self, CHAN_VOICE, gi.SoundIndex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

        normal_angles = vec3_euler(plane->normal);
        AngleVectors(normal_angles, NULL, &right, NULL);
        self->state.angles = vec3_euler(right);

        if (self->state.modelIndex == sm_meat_index) {
            self->state.frame++;
            //self->Think = gib_think;
            self->nextThinkTime = level.time + FRAMETIME;
        }
    }
}

void gib_die(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t& point)
{
    SVG_FreeEntity(self);
}

void ThrowGib(Entity *self, const char *gibname, int damage, int type)
{
    //Entity *gib;
    //vec3_t  vd;
    //vec3_t  origin;
    //vec3_t  size;
    //float   vscale;

    //gib = SVG_Spawn();

    //VectorScale(self->size, 0.5, size);
    //VectorAdd(self->absMin, size, origin);
    //gib->state.origin[0] = origin[0] + crandom() * size[0];
    //gib->state.origin[1] = origin[1] + crandom() * size[1];
    //gib->state.origin[2] = origin[2] + crandom() * size[2];

    //gi.SetModel(gib, gibname);
    //gib->solid = Solid::Not;
    //gib->state.effects |= EntityEffectType::Gib;
    //gib->flags |= EntityFlags::NoKnockBack;
    //gib->takeDamage = TakeDamage::Yes;
    ////gib->Die = gib_die;

    //if (type == GIB_ORGANIC) {
    //    gib->moveType = MoveType::Toss;
    //    //gib->Touch = gib_touch;
    //    vscale = 0.5;
    //} else {
    //    gib->moveType = MoveType::Bounce;
    //    vscale = 1.0;
    //}

    //// Calculate velocity for given damage.
    //vd = SVG_VelocityForDamage(damage);
    //VectorMA(self->velocity, vscale, vd, gib->velocity);
    //ClipGibVelocity(gib);
    //gib->angularVelocity[0] = random() * 600;
    //gib->angularVelocity[1] = random() * 600;
    //gib->angularVelocity[2] = random() * 600;

    ////gib->Think = SVG_FreeEntity;
    //gib->nextThinkTime = level.time + 10 + random() * 10;

    //gi.LinkEntity(gib);
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


//
//=================
// Debris
//=================
//
void debris_die(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t& point)
{
    SVG_FreeEntity(self);
}

void SVG_ThrowDebris(SVGBaseEntity *self, const char *modelname, float speed, const vec3_t &origin) // C++20: STRING: Added const to char*
{
//    Entity *chunk;
//    vec3_t  v;
//
//    chunk = SVG_Spawn();
//    VectorCopy(origin, chunk->state.origin);
//    gi.SetModel(chunk, modelname);
//    v[0] = 100 * crandom();
//    v[1] = 100 * crandom();
//    v[2] = 100 + 100 * crandom();
//    //VectorMA(self->velocity, speed, v, chunk->velocity);
//    //chunk->moveType = MoveType::Bounce;
//    chunk->solid = Solid::Not;
//    chunk->angularVelocity[0] = random() * 600;
//    chunk->angularVelocity[1] = random() * 600;
//    chunk->angularVelocity[2] = random() * 600;
////    chunk->Think = SVG_FreeEntity;
//    chunk->nextThinkTime = level.time + 5 + random() * 5;
//    chunk->state.frame = 0;
//    chunk->flags = 0;
//    chunk->className = "debris";
//    chunk->takeDamage = TakeDamage::Yes;
////    chunk->Die = debris_die;
//    gi.LinkEntity(chunk);
}


void BecomeExplosion1(SVGBaseEntity *self)
{
    vec3_t origin = self->GetOrigin();
    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(TempEntityEvent::Explosion1);
    gi.WriteVector3(origin);
    
    gi.Multicast(&origin, MultiCast::PVS);

    SVG_FreeEntity(self->GetServerEntity());
}


void BecomeExplosion2(SVGBaseEntity*self)
{
    vec3_t origin = self->GetOrigin();

    gi.WriteByte(SVG_CMD_TEMP_ENTITY);
    gi.WriteByte(TempEntityEvent::Explosion2);
    gi.WriteVector3(origin);
    gi.Multicast(&origin, MultiCast::PVS);

    SVG_FreeEntity(self->GetServerEntity());
}