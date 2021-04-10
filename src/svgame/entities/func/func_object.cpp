// LICENSE HERE.

//
// svgame/entities/func_object.c
//
//
// func_object entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.

//=====================================================
/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
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

void func_object_release(entity_t* self)
{
    self->moveType = MOVETYPE_TOSS;
    self->Touch = func_object_touch;
}

void func_object_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->solid = SOLID_BSP;
    self->svFlags &= ~SVF_NOCLIENT;
    self->Use = NULL;
    KillBox(self);
    func_object_release(self);
}

void SP_func_object(entity_t* self)
{
    gi.SetModel(self, self->model);

    self->mins[0] += 1;
    self->mins[1] += 1;
    self->mins[2] += 1;
    self->maxs[0] -= 1;
    self->maxs[1] -= 1;
    self->maxs[2] -= 1;

    if (!self->dmg)
        self->dmg = 100;

    if (self->spawnFlags == 0) {
        self->solid = SOLID_BSP;
        self->moveType = MOVETYPE_PUSH;
        self->Think = func_object_release;
        self->nextThink = level.time + 2 * FRAMETIME;
    }
    else {
        self->solid = SOLID_NOT;
        self->moveType = MOVETYPE_PUSH;
        self->Use = func_object_use;
        self->svFlags |= SVF_NOCLIENT;
    }

    if (self->spawnFlags & 2)
        self->s.effects |= EF_ANIM_ALL;
    if (self->spawnFlags & 4)
        self->s.effects |= EF_ANIM_ALLFAST;

    self->clipMask = CONTENTS_MASK_MONSTERSOLID;

    gi.LinkEntity(self);
}