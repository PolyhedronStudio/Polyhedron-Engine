// LICENSE HERE.

//
// svgame/entities/point_combat.c
//
//
// point_combat entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================

/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf)
{
    edict_t* activator;

    if (other->movetarget != self)
        return;

    if (self->target) {
        other->target = self->target;
        other->goalentity = other->movetarget = G_PickTarget(other->target);
        if (!other->goalentity) {
            gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
            other->movetarget = self;
        }
        self->target = NULL;
    }
    else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM | FL_FLY))) {
        other->monsterinfo.pausetime = level.time + 100000000;
        other->monsterinfo.aiflags |= AI_STAND_GROUND;
        other->monsterinfo.stand(other);
    }

    if (other->movetarget == self) {
        other->target = NULL;
        other->movetarget = NULL;
        other->goalentity = other->enemy;
        other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
    }

    if (self->pathtarget) {
        char* savetarget;

        savetarget = self->target;
        self->target = self->pathtarget;
        if (other->enemy && other->enemy->client)
            activator = other->enemy;
        else if (other->oldenemy && other->oldenemy->client)
            activator = other->oldenemy;
        else if (other->activator && other->activator->client)
            activator = other->activator;
        else
            activator = other;
        G_UseTargets(self, activator);
        self->target = savetarget;
    }
}

void SP_point_combat(edict_t* self)
{
    if (deathmatch->value) {
        G_FreeEdict(self);
        return;
    }
    self->solid = SOLID_TRIGGER;
    self->touch = point_combat_touch;
    VectorSet(self->mins, -8, -8, -16);
    VectorSet(self->maxs, 8, 8, 16);
    self->svflags = SVF_NOCLIENT;
    gi.linkentity(self);
}