// LICENSE HERE.

//
// svgame/entities/misc_explobox.c
//
//
// misc_explobox entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../effects.h"

//=====================================================
/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and damage (150).
*/

void barrel_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf)

{
    float   ratio;
    vec3_t  v;

    if ((!other->groundEntityPtr) || (other->groundEntityPtr == self))
        return;

    ratio = (float)other->mass / (float)self->mass;
    VectorSubtract(self->state.origin, other->state.origin, v);
//    M_walkmove(self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode(Entity* self)
{
    vec3_t  org;
    float   spd;
    vec3_t  save;

    T_RadiusDamage(self, self->activator, self->damage, NULL, self->damage + 40, MeansOfDeath::Barrel);

    VectorCopy(self->state.origin, save);
    VectorMA(self->absMin, 0.5, self->size, self->state.origin);

    // a few big chunks
    spd = 1.5 * (float)self->damage / 200.0;
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

    // bottom corners
    spd = 1.75 * (float)self->damage / 200.0;
    VectorCopy(self->absMin, org);
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absMin, org);
    org[0] += self->size[0];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absMin, org);
    org[1] += self->size[1];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
    VectorCopy(self->absMin, org);
    org[0] += self->size[0];
    org[1] += self->size[1];
    ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

    // a bunch of little chunks
    spd = 2 * self->damage / 200;
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
    org[0] = self->state.origin[0] + crandom() * self->size[0];
    org[1] = self->state.origin[1] + crandom() * self->size[1];
    org[2] = self->state.origin[2] + crandom() * self->size[2];
    ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);

    VectorCopy(save, self->state.origin);
    if (self->groundEntityPtr)
        BecomeExplosion2(self);
    else
        BecomeExplosion1(self);
}

void barrel_delay(Entity* self, Entity* inflictor, Entity* attacker, int damage, const vec3_t& point)
{
    self->takeDamage = TakeDamage::No;
    self->nextThink = level.time + 2 * FRAMETIME;
    self->Think = barrel_explode;
    self->activator = attacker;
}

void barrel_think(Entity* self) {

}

void SP_misc_explobox(Entity* self)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(self);
        return;
    }

    gi.ModelIndex("models/objects/debris1/tris.md2");
    gi.ModelIndex("models/objects/debris2/tris.md2");
    gi.ModelIndex("models/objects/debris3/tris.md2");

    self->solid = Solid::BoundingBox;
    self->moveType = MoveType::Step;

    self->model = "models/objects/barrels/tris.md2";
    self->state.modelIndex = gi.ModelIndex(self->model);
    VectorSet(self->mins, -16, -16, 0);
    VectorSet(self->maxs, 16, 16, 40);

    if (!self->mass)
        self->mass = 400;
    if (!self->health)
        self->health = 10;
    if (!self->damage)
        self->damage = 150;

    self->Die = barrel_delay;
    self->takeDamage = TakeDamage::Yes;

    self->Touch = barrel_touch;

    //self->Think = barrel_think;
    //self->nextThink = level.time + 2 * FRAMETIME;

    gi.LinkEntity(self);
}