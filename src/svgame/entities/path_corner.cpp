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
void path_corner_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    vec3_t      v;
    entity_t* next;

    if (other->moveTargetPtr != self)
        return;

    if (other->enemy)
        return;

    if (self->pathTarget) {
        char* savetarget;

        savetarget = self->target;
        self->target = self->pathTarget;
        UTIL_UseTargets(self, other);
        self->target = savetarget;
    }

    if (self->target)
        next = G_PickTarget(self->target);
    else
        next = NULL;

    if ((next) && (next->spawnFlags & 1)) {
        VectorCopy(next->s.origin, v);
        v[2] += next->mins[2];
        v[2] -= other->mins[2];
        VectorCopy(v, other->s.origin);
        next = G_PickTarget(next->target);
        other->s.event = EV_OTHER_TELEPORT;
    }

    other->goalEntityPtr = other->moveTargetPtr = next;

    if (self->wait) {
        other->monsterInfo.pausetime = level.time + self->wait;
        other->monsterInfo.stand(other);
        return;
    }

    if (!other->moveTargetPtr) {
        other->monsterInfo.pausetime = level.time + 100000000;
        other->monsterInfo.stand(other);
    }
    else {
        VectorSubtract(other->goalEntityPtr->s.origin, other->s.origin, v);
        other->idealYaw = vectoyaw(v);
    }
}

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
    this path_corner targeted touches it
*/
void SP_path_corner(entity_t* self)
{
    if (!self->targetName) {
        gi.DPrintf("path_corner with no targetName at %s\n", Vec3ToString(self->s.origin));
        G_FreeEntity(self);
        return;
    }

    self->solid = SOLID_TRIGGER;
    self->Touch = path_corner_touch;
    VectorSet(self->mins, -8, -8, -8);
    VectorSet(self->maxs, 8, 8, 8);
    self->svFlags |= SVF_NOCLIENT;
    gi.LinkEntity(self);
}