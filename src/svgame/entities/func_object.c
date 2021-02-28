// LICENSE HERE.

//
// svgame/entities/func_object.c
//
//
// func_object entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    // only squash thing we fall on top of
    if (!plane)
        return;
    if (plane->normal[2] < 1.0)
        return;
    if (other->takedamage == DAMAGE_NO)
        return;
    T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release(edict_t* self)
{
    self->movetype = MOVETYPE_TOSS;
    self->touch = func_object_touch;
}

void func_object_use(edict_t* self, edict_t* other, edict_t* activator)
{
    self->solid = SOLID_BSP;
    self->svflags &= ~SVF_NOCLIENT;
    self->use = NULL;
    KillBox(self);
    func_object_release(self);
}

void SP_func_object(edict_t* self)
{
    gi.setmodel(self, self->model);

    self->mins[0] += 1;
    self->mins[1] += 1;
    self->mins[2] += 1;
    self->maxs[0] -= 1;
    self->maxs[1] -= 1;
    self->maxs[2] -= 1;

    if (!self->dmg)
        self->dmg = 100;

    if (self->spawnflags == 0) {
        self->solid = SOLID_BSP;
        self->movetype = MOVETYPE_PUSH;
        self->think = func_object_release;
        self->nextthink = level.time + 2 * FRAMETIME;
    }
    else {
        self->solid = SOLID_NOT;
        self->movetype = MOVETYPE_PUSH;
        self->use = func_object_use;
        self->svflags |= SVF_NOCLIENT;
    }

    if (self->spawnflags & 2)
        self->s.effects |= EF_ANIM_ALL;
    if (self->spawnflags & 4)
        self->s.effects |= EF_ANIM_ALLFAST;

    self->clipmask = MASK_MONSTERSOLID;

    gi.linkentity(self);
}