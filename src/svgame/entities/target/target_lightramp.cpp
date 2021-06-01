// LICENSE HERE.

//
// svgame/entities/target_lightramp.c
//
//
// target_lightramp entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.


//=====================================================
/*QUAKED target_lightramp (0 .5 .8) (-8 -8 -8) (8 8 8) TOGGLE
speed       How many seconds the ramping will take
message     two letters; starting lightLevel and ending lightLevel
*/

void target_lightramp_think(Entity* self)
{
    char    style[2];

    style[0] = 'a' + self->moveDirection[0] + (level.time - self->timeStamp) / FRAMETIME * self->moveDirection[2];
    style[1] = 0;
    gi.configstring(ConfigStrings::Lights+ self->enemy->style, style);

    if ((level.time - self->timeStamp) < self->speed) {
        self->nextThinkTime = level.time + FRAMETIME;
    }
    else if (self->spawnFlags & 1) {
        char    temp;

        temp = self->moveDirection[0];
        self->moveDirection[0] = self->moveDirection[1];
        self->moveDirection[1] = temp;
        self->moveDirection[2] *= -1;
    }
}

void target_lightramp_use(Entity* self, Entity* other, Entity* activator)
{
    if (!self->enemy) {
        Entity* e;

        // check all the targets
        e = NULL;
        while (1) {
            e = SVG_Find(e, FOFS(targetName), self->target);
            if (!e)
                break;
            if (strcmp(e->className, "light") != 0) {
                gi.DPrintf("%s at %s ", self->className, Vec3ToString(self->state.origin));
                gi.DPrintf("target %s (%s at %s) is not a light\n", self->target, e->className, Vec3ToString(e->state.origin));
            }
            else {
                self->enemy = e;
            }
        }

        if (!self->enemy) {
            gi.DPrintf("%s target %s not found at %s\n", self->className, self->target, Vec3ToString(self->state.origin));
            SVG_FreeEntity(self);
            return;
        }
    }

    self->timeStamp = level.time;
    target_lightramp_think(self);
}

void SP_target_lightramp(Entity* self)
{
    if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1]) {
        gi.DPrintf("target_lightramp has bad ramp (%s) at %s\n", self->message, Vec3ToString(self->state.origin));
        SVG_FreeEntity(self);
        return;
    }

    if (deathmatch->value) {
        SVG_FreeEntity(self);
        return;
    }

    if (!self->target) {
        gi.DPrintf("%s with no target at %s\n", self->className, Vec3ToString(self->state.origin));
        SVG_FreeEntity(self);
        return;
    }

    self->serverFlags |= EntityServerFlags::NoClient;
    self->Use = target_lightramp_use;
    self->Think = target_lightramp_think;

    self->moveDirection[0] = self->message[0] - 'a';
    self->moveDirection[1] = self->message[1] - 'a';
    self->moveDirection[2] = (self->moveDirection[1] - self->moveDirection[0]) / (self->speed / FRAMETIME);
}