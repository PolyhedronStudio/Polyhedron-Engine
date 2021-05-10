// LICENSE HERE.

//
// svgame/entities/func_timer.c
//
//
// func_timer entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.

//=====================================================
/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
"wait"          base time between triggering all targets, default is 1
"random"        wait variance, default is 0

so, the basic time between firing is a random time between
(wait - random) and (wait + random)

"delay"         delay before first firing when turned on, default is 0

"pausetime"     additional delay used only the very first time
                and only if spawned with START_ON

These can used but not touched.
*/
void func_timer_think(entity_t* self)
{
    UTIL_UseTargets(self, self->activator);
    self->nextThink = level.time + self->wait + crandom() * self->random;
}

void func_timer_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->activator = activator;

    // if on, turn it off
    if (self->nextThink) {
        self->nextThink = 0;
        return;
    }

    // turn it on
    if (self->delay)
        self->nextThink = level.time + self->delay;
    else
        func_timer_think(self);
}

void SP_func_timer(entity_t* self)
{
    if (!self->wait)
        self->wait = 1.0;

    self->Use = func_timer_use;
    self->Think = func_timer_think;

    if (self->random >= self->wait) {
        self->random = self->wait - FRAMETIME;
        gi.DPrintf("func_timer at %s has random >= wait\n", Vec3ToString(self->state.origin));
    }

    if (self->spawnFlags & 1) {
        self->nextThink = level.time + 1.0 + st.pausetime + self->delay + self->wait + crandom() * self->random;
        self->activator = self;
    }

    self->serverFlags = EntityServerFlags::NoClient;
}