// LICENSE HERE.

//
// svgame/entities/misc_explobox.c
//
//
// misc_explobox entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)

{
    float   ratio;
    vec3_t  v;

    if ((!other->groundentity) || (other->groundentity == self))
        return;

    ratio = (float)other->mass / (float)self->mass;
    VectorSubtract(self->s.origin, other->s.origin, v);
    M_walkmove(self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode(edict_t* self)
{
    vec3_t  org;
    float   spd;
    vec3_t  save;

    T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg + 40, MOD_BARREL);

    VectorCopy(self->s.origin, save);
    VectorMA(self->absmin, 0.5, self->size, self->s.origin);

    // a few big chunks
    spd = 1.5 * (float)self->dmg / 200.0;
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

    // bottom corners
    spd = 1.75 * (float)self->dmg / 200.0;
    VectorCopy(self->absmin, org);
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absmin, org);
    org[0] += self->size[0];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absmin, org);
    org[1] += self->size[1];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absmin, org);
    org[0] += self->size[0];
    org[1] += self->size[1];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

    // a bunch of little chunks
    spd = 2 * self->dmg / 200;
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->s.origin[0] + crandom() * self->size[0];
    org[1] = self->s.origin[1] + crandom() * self->size[1];
    org[2] = self->s.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);

    VectorCopy(save, self->s.origin);
    if (self->groundentity)
        BecomeExplosion2(self);
    else
        BecomeExplosion1(self);
}

void barrel_delay(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point)
{
    self->takedamage = DAMAGE_NO;
    self->nextthink = level.time + 2 * FRAMETIME;
    self->think = barrel_explode;
    self->activator = attacker;
}

void SP_misc_explobox(edict_t* self)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(self);
        return;
    }

    gi.modelindex("models/objects/debris1/tris.md2");
    gi.modelindex("models/objects/debris2/tris.md2");
    gi.modelindex("models/objects/debris3/tris.md2");

    self->solid = SOLID_BBOX;
    self->movetype = MOVETYPE_STEP;

    self->model = "models/objects/barrels/tris.md2";
    self->s.modelindex = gi.modelindex(self->model);
    VectorSet(self->mins, -16, -16, 0);
    VectorSet(self->maxs, 16, 16, 40);

    if (!self->mass)
        self->mass = 400;
    if (!self->health)
        self->health = 10;
    if (!self->dmg)
        self->dmg = 150;

    self->die = barrel_delay;
    self->takedamage = DAMAGE_YES;
    self->monsterinfo.aiflags = AI_NOSTEP;

    self->touch = barrel_touch;

    self->think = M_droptofloor;
    self->nextthink = level.time + 2 * FRAMETIME;

    gi.linkentity(self);
}