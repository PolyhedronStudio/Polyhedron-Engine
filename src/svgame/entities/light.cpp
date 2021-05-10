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

void light_use(entity_t* self, entity_t* other, entity_t* activator)
{
    if (self->spawnFlags & START_OFF) {
        gi.configstring(ConfigStrings::Lights + self->style, self->customLightStyle);
        self->spawnFlags &= ~START_OFF;
    }
    else {
        gi.configstring(ConfigStrings::Lights + self->style, "a");
        self->spawnFlags |= START_OFF;
    }
}

void SP_light(entity_t* self)
{
    // no targeted lights in deathmatch, because they cause global messages
    if (!self->targetName || deathmatch->value) {
        G_FreeEntity(self);
        return;
    }

    if (self->style >= 32) {
        self->Use = light_use;
        if (self->spawnFlags & START_OFF)
            gi.configstring(ConfigStrings::Lights+ self->style, "a");
        else
            gi.configstring(ConfigStrings::Lights+ self->style, self->customLightStyle);
    }
}