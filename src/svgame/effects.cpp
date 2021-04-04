// LICENSE HERE.

//
// misc.c
//
//
// Contains misc definitions.
//

#include "g_local.h"         // Include SVGame funcs.
#include "utils.h"           // Include Utilities funcs.
#include "effects.h"

//=====================================================




void ClipGibVelocity(entity_t *ent)
{
    if (ent->velocity[0] < -300)
        ent->velocity[0] = -300;
    else if (ent->velocity[0] > 300)
        ent->velocity[0] = 300;
    if (ent->velocity[1] < -300)
        ent->velocity[1] = -300;
    else if (ent->velocity[1] > 300)
        ent->velocity[1] = 300;
    if (ent->velocity[2] < 200)
        ent->velocity[2] = 200; // always some upwards
    else if (ent->velocity[2] > 500)
        ent->velocity[2] = 500;
}


//
//=================
// Gibs
//=================
//
void gib_think(entity_t *self)
{
    self->s.frame++;
    self->nextThink = level.time + FRAMETIME;

    if (self->s.frame == 10) {
        self->Think = G_FreeEntity;
        self->nextThink = level.time + 8 + random() * 10;
    }
}

void gib_touch(entity_t *self, entity_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t  normal_angles, right;

    if (!self->groundEntityPtr)
        return;

    self->Touch = NULL;

    if (plane) {
        gi.Sound(self, CHAN_VOICE, gi.SoundIndex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

        vectoangles(plane->normal, normal_angles);
        vec3_vectors(normal_angles, NULL, &right, NULL);
        vectoangles(right, self->s.angles);

        if (self->s.modelindex == sm_meat_index) {
            self->s.frame++;
            self->Think = gib_think;
            self->nextThink = level.time + FRAMETIME;
        }
    }
}

void gib_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point)
{
    G_FreeEntity(self);
}

void ThrowGib(entity_t *self, const char *gibname, int damage, int type)
{
    entity_t *gib;
    vec3_t  vd;
    vec3_t  origin;
    vec3_t  size;
    float   vscale;

    gib = G_Spawn();

    VectorScale(self->size, 0.5, size);
    VectorAdd(self->absMin, size, origin);
    gib->s.origin[0] = origin[0] + crandom() * size[0];
    gib->s.origin[1] = origin[1] + crandom() * size[1];
    gib->s.origin[2] = origin[2] + crandom() * size[2];

    gi.SetModel(gib, gibname);
    gib->solid = SOLID_NOT;
    gib->s.effects |= EF_GIB;
    gib->flags |= FL_NO_KNOCKBACK;
    gib->takeDamage = DAMAGE_YES;
    gib->Die = gib_die;

    if (type == GIB_ORGANIC) {
        gib->moveType = MOVETYPE_TOSS;
        gib->Touch = gib_touch;
        vscale = 0.5;
    } else {
        gib->moveType = MOVETYPE_BOUNCE;
        vscale = 1.0;
    }

    // Calculate velocity for given damage.
    vd = VelocityForDamage(damage);
    VectorMA(self->velocity, vscale, vd, gib->velocity);
    ClipGibVelocity(gib);
    gib->avelocity[0] = random() * 600;
    gib->avelocity[1] = random() * 600;
    gib->avelocity[2] = random() * 600;

    gib->Think = G_FreeEntity;
    gib->nextThink = level.time + 10 + random() * 10;

    gi.LinkEntity(gib);
}

void ThrowHead(entity_t *self, const char *gibname, int damage, int type)
{
    vec3_t  vd;
    float   vscale;

    self->s.skinnum = 0;
    self->s.frame = 0;
    VectorClear(self->mins);
    VectorClear(self->maxs);

    self->s.modelindex2 = 0;
    gi.SetModel(self, gibname);
    self->solid = SOLID_NOT;
    self->s.effects |= EF_GIB;
    self->s.effects &= ~EF_FLIES;
    self->s.sound = 0;
    self->flags |= FL_NO_KNOCKBACK;
    self->svFlags &= ~SVF_MONSTER;
    self->takeDamage = DAMAGE_YES;
    self->Die = gib_die;

    if (type == GIB_ORGANIC) {
        self->moveType = MOVETYPE_TOSS;
        self->Touch = gib_touch;
        vscale = 0.5;
    } else {
        self->moveType = MOVETYPE_BOUNCE;
        vscale = 1.0;
    }

    // Calculate velocity for given damage.
    vd = VelocityForDamage(damage);
    VectorMA(self->velocity, vscale, vd, self->velocity);
    ClipGibVelocity(self);

    self->avelocity[vec3_t::Yaw] = crandom() * 600;

    self->Think = G_FreeEntity;
    self->nextThink = level.time + 10 + random() * 10;

    gi.LinkEntity(self);
}


void ThrowClientHead(entity_t *self, int damage)
{
    vec3_t  vd;
    const char    *gibname; // C++20 VKPT: added const.

    if (rand() & 1) {
        gibname = "models/objects/gibs/head2/tris.md2";
        self->s.skinnum = 1;        // second skin is player
    } else {
        gibname = "models/objects/gibs/skull/tris.md2";
        self->s.skinnum = 0;
    }

    self->s.origin[2] += 32;
    self->s.frame = 0;
    gi.SetModel(self, gibname);
    VectorSet(self->mins, -16, -16, 0);
    VectorSet(self->maxs, 16, 16, 16);

    self->takeDamage = DAMAGE_NO;
    self->solid = SOLID_NOT;
    self->s.effects = EF_GIB;
    self->s.sound = 0;
    self->flags |= FL_NO_KNOCKBACK;

    self->moveType = MOVETYPE_BOUNCE;
    // Calculate velocity for given damage.
    vd = VelocityForDamage(damage);
    VectorAdd(self->velocity, vd, self->velocity);

    if (self->client) { // bodies in the queue don't have a client anymore
        self->client->anim_priority = ANIM_DEATH;
        self->client->anim_end = self->s.frame;
    } else {
        self->Think = NULL;
        self->nextThink = 0;
    }

    gi.LinkEntity(self);
}


//
//=================
// Debris
//=================
//
void debris_die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point)
{
    G_FreeEntity(self);
}

void ThrowDebris(entity_t *self, const char *modelname, float speed, const vec3_t &origin) // C++20: STRING: Added const to char*
{
    entity_t *chunk;
    vec3_t  v;

    chunk = G_Spawn();
    VectorCopy(origin, chunk->s.origin);
    gi.SetModel(chunk, modelname);
    v[0] = 100 * crandom();
    v[1] = 100 * crandom();
    v[2] = 100 + 100 * crandom();
    VectorMA(self->velocity, speed, v, chunk->velocity);
    chunk->moveType = MOVETYPE_BOUNCE;
    chunk->solid = SOLID_NOT;
    chunk->avelocity[0] = random() * 600;
    chunk->avelocity[1] = random() * 600;
    chunk->avelocity[2] = random() * 600;
    chunk->Think = G_FreeEntity;
    chunk->nextThink = level.time + 5 + random() * 5;
    chunk->s.frame = 0;
    chunk->flags = 0;
    chunk->classname = "debris";
    chunk->takeDamage = DAMAGE_YES;
    chunk->Die = debris_die;
    gi.LinkEntity(chunk);
}


void BecomeExplosion1(entity_t *self)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_EXPLOSION1);
    gi.WritePosition(self->s.origin);
    gi.Multicast(&self->s.origin, MULTICAST_PVS);

    G_FreeEntity(self);
}


void BecomeExplosion2(entity_t *self)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_EXPLOSION2);
    gi.WritePosition(self->s.origin);
    gi.Multicast(&self->s.origin, MULTICAST_PVS);

    G_FreeEntity(self);
}