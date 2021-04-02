// LICENSE HERE.

//
// svgame/entities/path_corner.c
//
//
// path_corner entity implementation.
//

#include "../g_local.h"      // SVGame funcs.
#include "../utils.h"        // Util funcs.

//=====================================================
void path_corner_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    vec3_t      v;
    edict_t* next;

    if (other->movetarget != self)
        return;

    if (other->enemy)
        return;

    if (self->pathtarget) {
        char* savetarget;

        savetarget = self->target;
        self->target = self->pathtarget;
        UTIL_UseTargets(self, other);
        self->target = savetarget;
    }

    if (self->target)
        next = G_PickTarget(self->target);
    else
        next = NULL;

    if ((next) && (next->spawnflags & 1)) {
        VectorCopy(next->s.origin, v);
        v[2] += next->mins[2];
        v[2] -= other->mins[2];
        VectorCopy(v, other->s.origin);
        next = G_PickTarget(next->target);
        other->s.event = EV_OTHER_TELEPORT;
    }

    other->goalentity = other->movetarget = next;

    if (self->wait) {
        other->monsterinfo.pausetime = level.time + self->wait;
        other->monsterinfo.stand(other);
        return;
    }

    if (!other->movetarget) {
        other->monsterinfo.pausetime = level.time + 100000000;
        other->monsterinfo.stand(other);
    }
    else {
        VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
        other->ideal_yaw = vectoyaw(v);
    }
}

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
    this path_corner targeted touches it
*/
void SP_path_corner(edict_t* self)
{
    if (!self->targetname) {
        gi.dprintf("path_corner with no targetname at %s\n", Vec3ToString(self->s.origin));
        G_FreeEdict(self);
        return;
    }

    self->solid = SOLID_TRIGGER;
    self->touch = path_corner_touch;
    VectorSet(self->mins, -8, -8, -8);
    VectorSet(self->maxs, 8, 8, 8);
    self->svflags |= SVF_NOCLIENT;
    gi.linkentity(self);
}