// LICENSE HERE.

//
// svgame/entities/point_combat.c
//
//
// point_combat entity implementation.
//

#include "../g_local.h"      // SVGame funcs.
#include "../utils.h"        // Util funcs.

//=====================================================

/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf)
{
    entity_t* activator;

    if (other->moveTargetPtr != self)
        return;

    if (self->target) {
        other->target = self->target;
        other->goalEntityPtr = other->moveTargetPtr = G_PickTarget(other->target);
        if (!other->goalEntityPtr) {
            gi.DPrintf("%s at %s target %s does not exist\n", self->classname, Vec3ToString(self->s.origin), self->target);
            other->moveTargetPtr = self;
        }
        self->target = NULL;
    }
    else if ((self->spawnFlags & 1) && !(other->flags & (FL_SWIM | FL_FLY))) {
        other->monsterInfo.pausetime = level.time + 100000000;
        other->monsterInfo.aiflags |= AI_Stand_GROUND;
        other->monsterInfo.stand(other);
    }

    if (other->moveTargetPtr == self) {
        other->target = NULL;
        other->moveTargetPtr = NULL;
        other->goalEntityPtr = other->enemy;
        other->monsterInfo.aiflags &= ~AI_COMBAT_POINT;
    }

    if (self->pathTarget) {
        char* savetarget;

        savetarget = self->target;
        self->target = self->pathTarget;
        if (other->enemy && other->enemy->client)
            activator = other->enemy;
        else if (other->oldEnemyPtr && other->oldEnemyPtr->client)
            activator = other->oldEnemyPtr;
        else if (other->activator && other->activator->client)
            activator = other->activator;
        else
            activator = other;
        UTIL_UseTargets(self, activator);
        self->target = savetarget;
    }
}

void SP_point_combat(entity_t* self)
{
    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }
    self->solid = SOLID_TRIGGER;
    self->Touch = point_combat_touch;
    VectorSet(self->mins, -8, -8, -16);
    VectorSet(self->maxs, 8, 8, 16);
    self->svFlags = SVF_NOCLIENT;
    gi.LinkEntity(self);
}