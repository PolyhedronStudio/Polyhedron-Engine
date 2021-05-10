// LICENSE HERE.

//
// svgame/entities/v.c
//
//
// func_explosive entity implementation.
//

// Include local game header.
#include "../../g_local.h"      // Include SVGame funcs.
#include "../../effects.h"      // Include Effect funcs.
#include "../../utils.h"        // Include Util funcs.

//=====================================================
/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t &point)
{
    vec3_t  origin;
    vec3_t  chunkorigin;
    vec3_t  size;
    int     count;
    int     mass;

    // bmodel origins are (0 0 0), we need to adjust that here
    VectorScale(self->size, 0.5, size);
    VectorAdd(self->absMin, size, origin);
    VectorCopy(origin, self->state.origin);

    self->takeDamage = TakeDamage::No;

    if (self->dmg)
        T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);

    VectorSubtract(self->state.origin, inflictor->state.origin, self->velocity);
    VectorNormalize(self->velocity);
    VectorScale(self->velocity, 150, self->velocity);

    // start chunks towards the center
    VectorScale(size, 0.5, size);

    mass = self->mass;
    if (!mass)
        mass = 75;

    // big chunks
    if (mass >= 100) {
        count = mass / 100;
        if (count > 8)
            count = 8;
        while (count--) {
            chunkorigin[0] = origin[0] + crandom() * size[0];
            chunkorigin[1] = origin[1] + crandom() * size[1];
            chunkorigin[2] = origin[2] + crandom() * size[2];
            ThrowDebris(self, "models/objects/debris1/tris.md2", 1, chunkorigin);
        }
    }

    // small chunks
    count = mass / 25;
    if (count > 16)
        count = 16;
    while (count--) {
        chunkorigin[0] = origin[0] + crandom() * size[0];
        chunkorigin[1] = origin[1] + crandom() * size[1];
        chunkorigin[2] = origin[2] + crandom() * size[2];
        ThrowDebris(self, "models/objects/debris2/tris.md2", 2, chunkorigin);
    }

    UTIL_UseTargets(self, attacker);

    if (self->dmg)
        BecomeExplosion1(self);
    else
        G_FreeEntity(self);
}

void func_explosive_use(entity_t* self, entity_t* other, entity_t* activator)
{
    func_explosive_explode(self, other, activator, self->health, vec3_origin);
}

void func_explosive_spawn(entity_t* self, entity_t* other, entity_t* activator)
{
    self->solid = Solid::BSP;
    self->serverFlags &= ~EntityServerFlags::NoClient;
    self->Use = NULL;
    KillBox(self);
    gi.LinkEntity(self);
}

void SP_func_explosive(entity_t* self)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(self);
        return;
    }

    self->moveType = MoveType::Push;

    gi.ModelIndex("models/objects/debris1/tris.md2");
    gi.ModelIndex("models/objects/debris2/tris.md2");

    gi.SetModel(self, self->model);

    if (self->spawnFlags & 1) {
        self->serverFlags |= EntityServerFlags::NoClient;
        self->solid = Solid::Not;
        self->Use = func_explosive_spawn;
    }
    else {
        self->solid = Solid::BSP;
        if (self->targetName)
            self->Use = func_explosive_use;
    }

    if (self->spawnFlags & 2)
        self->state.effects |= EntityEffectType::AnimCycleAll2hz;
    if (self->spawnFlags & 4)
        self->state.effects |= EntityEffectType::AnimCycleAll30hz;

    if (self->Use != func_explosive_use) {
        if (!self->health)
            self->health = 100;
        self->Die = func_explosive_explode;
        self->takeDamage = TakeDamage::Yes;
    }

    gi.LinkEntity(self);
}