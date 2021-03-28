// LICENSE HERE.

//
// misc.c
//
//
// Contains misc definitions.
//

#include "g_local.h"
#include "misc.h"

//=====================================================


//
//=================
// Misc functions
//=================
//
void VelocityForDamage(int damage, vec3_t v)
{
    v[0] = 100.0 * crandom();
    v[1] = 100.0 * crandom();
    v[2] = 200.0 + 100.0 * random();

    if (damage < 50)
        VectorScale(v, 0.7, v);
    else
        VectorScale(v, 1.2, v);
}

void ClipGibVelocity(edict_t *ent)
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
void gib_think(edict_t *self)
{
    self->s.frame++;
    self->nextthink = level.time + FRAMETIME;

    if (self->s.frame == 10) {
        self->think = G_FreeEdict;
        self->nextthink = level.time + 8 + random() * 10;
    }
}

void gib_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    vec3_t  normal_angles, right;

    if (!self->groundentity)
        return;

    self->touch = NULL;

    if (plane) {
        gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

        vectoangles(plane->normal, normal_angles);
        AngleVectors(normal_angles, NULL, &right, NULL);
        vectoangles(right, self->s.angles);

        if (self->s.modelindex == sm_meat_index) {
            self->s.frame++;
            self->think = gib_think;
            self->nextthink = level.time + FRAMETIME;
        }
    }
}

void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    G_FreeEdict(self);
}

void ThrowGib(edict_t *self, const char *gibname, int damage, int type)
{
    edict_t *gib;
    vec3_t  vd;
    vec3_t  origin;
    vec3_t  size;
    float   vscale;

    gib = G_Spawn();

    VectorScale(self->size, 0.5, size);
    VectorAdd(self->absmin, size, origin);
    gib->s.origin[0] = origin[0] + crandom() * size[0];
    gib->s.origin[1] = origin[1] + crandom() * size[1];
    gib->s.origin[2] = origin[2] + crandom() * size[2];

    gi.setmodel(gib, gibname);
    gib->solid = SOLID_NOT;
    gib->s.effects |= EF_GIB;
    gib->flags |= FL_NO_KNOCKBACK;
    gib->takedamage = DAMAGE_YES;
    gib->die = gib_die;

    if (type == GIB_ORGANIC) {
        gib->movetype = MOVETYPE_TOSS;
        gib->touch = gib_touch;
        vscale = 0.5;
    } else {
        gib->movetype = MOVETYPE_BOUNCE;
        vscale = 1.0;
    }

    VelocityForDamage(damage, vd);
    VectorMA(self->velocity, vscale, vd, gib->velocity);
    ClipGibVelocity(gib);
    gib->avelocity[0] = random() * 600;
    gib->avelocity[1] = random() * 600;
    gib->avelocity[2] = random() * 600;

    gib->think = G_FreeEdict;
    gib->nextthink = level.time + 10 + random() * 10;

    gi.linkentity(gib);
}

void ThrowHead(edict_t *self, const char *gibname, int damage, int type)
{
    vec3_t  vd;
    float   vscale;

    self->s.skinnum = 0;
    self->s.frame = 0;
    VectorClear(self->mins);
    VectorClear(self->maxs);

    self->s.modelindex2 = 0;
    gi.setmodel(self, gibname);
    self->solid = SOLID_NOT;
    self->s.effects |= EF_GIB;
    self->s.effects &= ~EF_FLIES;
    self->s.sound = 0;
    self->flags |= FL_NO_KNOCKBACK;
    self->svflags &= ~SVF_MONSTER;
    self->takedamage = DAMAGE_YES;
    self->die = gib_die;

    if (type == GIB_ORGANIC) {
        self->movetype = MOVETYPE_TOSS;
        self->touch = gib_touch;
        vscale = 0.5;
    } else {
        self->movetype = MOVETYPE_BOUNCE;
        vscale = 1.0;
    }

    VelocityForDamage(damage, vd);
    VectorMA(self->velocity, vscale, vd, self->velocity);
    ClipGibVelocity(self);

    self->avelocity[YAW] = crandom() * 600;

    self->think = G_FreeEdict;
    self->nextthink = level.time + 10 + random() * 10;

    gi.linkentity(self);
}


void ThrowClientHead(edict_t *self, int damage)
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
    gi.setmodel(self, gibname);
    VectorSet(self->mins, -16, -16, 0);
    VectorSet(self->maxs, 16, 16, 16);

    self->takedamage = DAMAGE_NO;
    self->solid = SOLID_NOT;
    self->s.effects = EF_GIB;
    self->s.sound = 0;
    self->flags |= FL_NO_KNOCKBACK;

    self->movetype = MOVETYPE_BOUNCE;
    VelocityForDamage(damage, vd);
    VectorAdd(self->velocity, vd, self->velocity);

    if (self->client) { // bodies in the queue don't have a client anymore
        self->client->anim_priority = ANIM_DEATH;
        self->client->anim_end = self->s.frame;
    } else {
        self->think = NULL;
        self->nextthink = 0;
    }

    gi.linkentity(self);
}


//
//=================
// Debris
//=================
//
void debris_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
    G_FreeEdict(self);
}

void ThrowDebris(edict_t *self, const char *modelname, float speed, vec3_t origin) // C++20: STRING: Added const to char*
{
    edict_t *chunk;
    vec3_t  v;

    chunk = G_Spawn();
    VectorCopy(origin, chunk->s.origin);
    gi.setmodel(chunk, modelname);
    v[0] = 100 * crandom();
    v[1] = 100 * crandom();
    v[2] = 100 + 100 * crandom();
    VectorMA(self->velocity, speed, v, chunk->velocity);
    chunk->movetype = MOVETYPE_BOUNCE;
    chunk->solid = SOLID_NOT;
    chunk->avelocity[0] = random() * 600;
    chunk->avelocity[1] = random() * 600;
    chunk->avelocity[2] = random() * 600;
    chunk->think = G_FreeEdict;
    chunk->nextthink = level.time + 5 + random() * 5;
    chunk->s.frame = 0;
    chunk->flags = 0;
    chunk->classname = "debris";
    chunk->takedamage = DAMAGE_YES;
    chunk->die = debris_die;
    gi.linkentity(chunk);
}


void BecomeExplosion1(edict_t *self)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_EXPLOSION1);
    gi.WritePosition(self->s.origin);
    gi.multicast(&self->s.origin, MULTICAST_PVS);

    G_FreeEdict(self);
}


void BecomeExplosion2(edict_t *self)
{
    gi.WriteByte(svg_temp_entity);
    gi.WriteByte(TE_EXPLOSION2);
    gi.WritePosition(self->s.origin);
    gi.multicast(&self->s.origin, MULTICAST_PVS);

    G_FreeEdict(self);
}