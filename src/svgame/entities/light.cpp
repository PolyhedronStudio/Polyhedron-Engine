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

//
//void light_use(Entity* self, Entity* other, Entity* activator)
//{
//    if (self->spawnFlags & START_OFF) {
//        gi.configstring(ConfigStrings::Lights + self->style, self->customLightStyle);
//        self->spawnFlags &= ~START_OFF;
//    }
//    else {
//        gi.configstring(ConfigStrings::Lights + self->style, "a");
//        self->spawnFlags |= START_OFF;
//    }
//}
//
//void SP_light(Entity* self)
//{
//    // no targeted lights in deathmatch, because they cause global messages
//    if (!self->targetName || deathmatch->value) {
//        SVG_FreeEntity(self);
//        return;
//    }
//
//    if (self->style >= 32) {
//        self->Use = light_use;
//        if (self->spawnFlags & START_OFF)
//            gi.configstring(ConfigStrings::Lights + self->style, "a");
//        else
//            gi.configstring(ConfigStrings::Lights + self->style, self->customLightStyle);
//    }
//}

/*
// LICENSE HERE.

//
// PlayerClient.cpp
//
//
*/
#include "../g_local.h"              // SVGame.

#include "base/SVGBaseEntity.h"
#include "Light.h"

// Yeah, the spawnflag for start off.
#define START_OFF   1

// Constructor/Deconstructor.
Light::Light(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
Light::~Light() {

}

// Interface functions. 
void Light::Precache() {
    // Parent class precache.
    SVGBaseEntity::Precache();
}
void Light::Spawn() {
    // Parent class spawn.
    SVGBaseEntity::Spawn();

    // WatIsDeze: I think we want lights. This is for the old 1997/1998
    // no targeted lights in deathmatch, because they cause global messages
    //if (!self->targetName || deathmatch->value) {
    //    SVG_FreeEntity(self);
    //    return;
    //}

    // Fetch style.
    uint32_t style = GetStyle();

    // Styles of 32 and above are meant to be used by the mappers.
    if (style >= 32) {
        SetUseCallback(&Light::LightUse);

        if (GetSpawnFlags() & START_OFF)
            gi.configstring(ConfigStrings::Lights + style, "a");
        else
            gi.configstring(ConfigStrings::Lights + style, GetServerEntity()->customLightStyle);
    }
}
void Light::PostSpawn() {
    // Parent class PostSpawn.
    SVGBaseEntity::PostSpawn();
}
void Light::Think() {
    // Parent class think.
    SVGBaseEntity::Think();
}

void Light::LightUse(SVGBaseEntity* activator, SVGBaseEntity* caller, float value) {
    // Get spawnflags.
    int32_t spawnFlags = GetSpawnFlags();

    // Fetch style.
    uint32_t style = GetStyle();

    if (GetSpawnFlags() & START_OFF) {
        // Switch style.
        gi.configstring(ConfigStrings::Lights + style, GetServerEntity()->customLightStyle);

        // Change spawnflags, the light is on after all.
        SetSpawnFlags(spawnFlags & ~START_OFF);
    }
    else {
        gi.configstring(ConfigStrings::Lights + style, "a");
        SetSpawnFlags(spawnFlags | START_OFF);
    }
}

// Functions.