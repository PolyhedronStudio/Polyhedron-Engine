// LICENSE HERE.

//
// svgame/entities/light.c
//
//
// light entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF   1

void light_use(edict_t* self, edict_t* other, edict_t* activator)
{
    if (self->spawnflags & START_OFF) {
        gi.configstring(CS_LIGHTS + self->style, "m");
        self->spawnflags &= ~START_OFF;
    }
    else {
        gi.configstring(CS_LIGHTS + self->style, "a");
        self->spawnflags |= START_OFF;
    }
}

void SP_light(edict_t* self)
{
    // no targeted lights in deathmatch, because they cause global messages
    if (!self->targetname || deathmatch->value) {
        G_FreeEdict(self);
        return;
    }

    if (self->style >= 32) {
        self->use = light_use;
        if (self->spawnflags & START_OFF)
            gi.configstring(CS_LIGHTS + self->style, "a");
        else
            gi.configstring(CS_LIGHTS + self->style, "m");
    }
}