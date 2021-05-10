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

void target_lightramp_think(entity_t* self)
{
    char    style[2];

    style[0] = 'a' + self->moveDirection[0] + (level.time - self->timestamp) / FRAMETIME * self->moveDirection[2];
    style[1] = 0;
    gi.configstring(ConfigStrings::Lights+ self->enemy->style, style);

    if ((level.time - self->timestamp) < self->speed) {
        self->nextThink = level.time + FRAMETIME;
    }
    else if (self->spawnFlags & 1) {
        char    temp;

        temp = self->moveDirection[0];
        self->moveDirection[0] = self->moveDirection[1];
        self->moveDirection[1] = temp;
        self->moveDirection[2] *= -1;
    }
}

void target_lightramp_use(entity_t* self, entity_t* other, entity_t* activator)
{
    if (!self->enemy) {
        entity_t* e;

        // check all the targets
        e = NULL;
        while (1) {
            e = G_Find(e, FOFS(targetName), self->target);
            if (!e)
                break;
            if (strcmp(e->classname, "light") != 0) {
                gi.DPrintf("%s at %s ", self->classname, Vec3ToString(self->state.origin));
                gi.DPrintf("target %s (%s at %s) is not a light\n", self->target, e->classname, Vec3ToString(e->state.origin));
            }
            else {
                self->enemy = e;
            }
        }

        if (!self->enemy) {
            gi.DPrintf("%s target %s not found at %s\n", self->classname, self->target, Vec3ToString(self->state.origin));
            G_FreeEntity(self);
            return;
        }
    }

    self->timestamp = level.time;
    target_lightramp_think(self);
}

void SP_target_lightramp(entity_t* self)
{
    if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1]) {
        gi.DPrintf("target_lightramp has bad ramp (%s) at %s\n", self->message, Vec3ToString(self->state.origin));
        G_FreeEntity(self);
        return;
    }

    if (deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    if (!self->target) {
        gi.DPrintf("%s with no target at %s\n", self->classname, Vec3ToString(self->state.origin));
        G_FreeEntity(self);
        return;
    }

    self->serverFlags |= EntityServerFlags::NoClient;
    self->Use = target_lightramp_use;
    self->Think = target_lightramp_think;

    self->moveDirection[0] = self->message[0] - 'a';
    self->moveDirection[1] = self->message[1] - 'a';
    self->moveDirection[2] = (self->moveDirection[1] - self->moveDirection[0]) / (self->speed / FRAMETIME);
}