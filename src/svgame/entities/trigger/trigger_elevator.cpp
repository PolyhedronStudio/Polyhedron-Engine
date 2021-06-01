// LICENSE HERE.

//
// svgame/entities/trigger_elevator.c
//
//
// trigger_elevator entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

// extern, is in func_train.c
extern void train_resume(Entity* self);

//=====================================================
/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use(Entity* self, Entity* other, Entity* activator)
{
    Entity* target;

    if (self->moveTargetPtr->nextThinkTime) {
        //      gi.DPrintf("elevator busy\n");
        return;
    }

    if (!other->pathTarget) {
        gi.DPrintf("elevator used with no pathTarget\n");
        return;
    }

    target = SVG_PickTarget(other->pathTarget);
    if (!target) {
        gi.DPrintf("elevator used with bad pathTarget: %s\n", other->pathTarget);
        return;
    }

    self->moveTargetPtr->targetEntityPtr = target;
    train_resume(self->moveTargetPtr);
}

void trigger_elevator_init(Entity* self)
{
    if (!self->target) {
        gi.DPrintf("trigger_elevator has no target\n");
        return;
    }
    self->moveTargetPtr = SVG_PickTarget(self->target);
    if (!self->moveTargetPtr) {
        gi.DPrintf("trigger_elevator unable to find target %s\n", self->target);
        return;
    }
    if (strcmp(self->moveTargetPtr->className, "func_train") != 0) {
        gi.DPrintf("trigger_elevator target %s is not a train\n", self->target);
        return;
    }

    self->Use = trigger_elevator_use;
    self->serverFlags = EntityServerFlags::NoClient;

}

void SP_trigger_elevator(Entity* self)
{
    self->Think = trigger_elevator_init;
    self->nextThinkTime = level.time + FRAMETIME;
}