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
message     two letters; starting lightlevel and ending lightlevel
*/

void target_lightramp_think(edict_t* self)
{
    char    style[2];

    style[0] = 'a' + self->movedir[0] + (level.time - self->timestamp) / FRAMETIME * self->movedir[2];
    style[1] = 0;
    gi.configstring(CS_LIGHTS + self->enemy->style, style);

    if ((level.time - self->timestamp) < self->speed) {
        self->nextthink = level.time + FRAMETIME;
    }
    else if (self->spawnflags & 1) {
        char    temp;

        temp = self->movedir[0];
        self->movedir[0] = self->movedir[1];
        self->movedir[1] = temp;
        self->movedir[2] *= -1;
    }
}

void target_lightramp_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (!self->enemy) {
        edict_t* e;

        // check all the targets
        e = NULL;
        while (1) {
            e = G_Find(e, FOFS(targetname), self->target);
            if (!e)
                break;
            if (strcmp(e->classname, "light") != 0) {
                gi.dprintf("%s at %s ", self->classname, Vec3ToString(self->s.origin));
                gi.dprintf("target %s (%s at %s) is not a light\n", self->target, e->classname, Vec3ToString(e->s.origin));
            }
            else {
                self->enemy = e;
            }
        }

        if (!self->enemy) {
            gi.dprintf("%s target %s not found at %s\n", self->classname, self->target, Vec3ToString(self->s.origin));
            G_FreeEdict(self);
            return;
        }
    }

    self->timestamp = level.time;
    target_lightramp_think(self);
}

void SP_target_lightramp(edict_t* self)
{
    if (!self->message || strlen(self->message) != 2 || self->message[0] < 'a' || self->message[0] > 'z' || self->message[1] < 'a' || self->message[1] > 'z' || self->message[0] == self->message[1]) {
        gi.dprintf("target_lightramp has bad ramp (%s) at %s\n", self->message, Vec3ToString(self->s.origin));
        G_FreeEdict(self);
        return;
    }

    if (deathmatch->value) {
        G_FreeEdict(self);
        return;
    }

    if (!self->target) {
        gi.dprintf("%s with no target at %s\n", self->classname, Vec3ToString(self->s.origin));
        G_FreeEdict(self);
        return;
    }

    self->svflags |= SVF_NOCLIENT;
    self->use = target_lightramp_use;
    self->think = target_lightramp_think;

    self->movedir[0] = self->message[0] - 'a';
    self->movedir[1] = self->message[1] - 'a';
    self->movedir[2] = (self->movedir[1] - self->movedir[0]) / (self->speed / FRAMETIME);
}