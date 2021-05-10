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
extern void train_resume(entity_t* self);

//=====================================================
/*QUAKED trigger_elevator (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/
void trigger_elevator_use(entity_t* self, entity_t* other, entity_t* activator)
{
    entity_t* target;

    if (self->moveTargetPtr->nextThink) {
        //      gi.DPrintf("elevator busy\n");
        return;
    }

    if (!other->pathTarget) {
        gi.DPrintf("elevator used with no pathTarget\n");
        return;
    }

    target = G_PickTarget(other->pathTarget);
    if (!target) {
        gi.DPrintf("elevator used with bad pathTarget: %s\n", other->pathTarget);
        return;
    }

    self->moveTargetPtr->targetEntityPtr = target;
    train_resume(self->moveTargetPtr);
}

void trigger_elevator_init(entity_t* self)
{
    if (!self->target) {
        gi.DPrintf("trigger_elevator has no target\n");
        return;
    }
    self->moveTargetPtr = G_PickTarget(self->target);
    if (!self->moveTargetPtr) {
        gi.DPrintf("trigger_elevator unable to find target %s\n", self->target);
        return;
    }
    if (strcmp(self->moveTargetPtr->classname, "func_train") != 0) {
        gi.DPrintf("trigger_elevator target %s is not a train\n", self->target);
        return;
    }

    self->Use = trigger_elevator_use;
    self->serverFlags = EntityServerFlags::NoClient;

}

void SP_trigger_elevator(entity_t* self)
{
    self->Think = trigger_elevator_init;
    self->nextThink = level.time + FRAMETIME;
}