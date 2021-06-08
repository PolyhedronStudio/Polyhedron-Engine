/*
// LICENSE HERE.

//
// Light.cpp
//
//
*/
#include "../g_local.h"              // SVGame.

#include "base/SVGBaseEntity.h"
#include "base/SVGBaseTrigger.h"
#include "Light.h"

// Yeah, the spawnflag for start off.
#define START_OFF   1

// Constructor/Deconstructor.
Light::Light(Entity* svEntity) : SVGBaseTrigger(svEntity) {

}
Light::~Light() {

}

// Interface functions. 
void Light::Precache() {
    // Parent class precache.
    SVGBaseTrigger::Precache();
}
void Light::Spawn() {
    // Parent class spawn.
    SVGBaseTrigger::Spawn();

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
    SVGBaseTrigger::PostSpawn();
}
void Light::Think() {
    // Parent class think.
    SVGBaseTrigger::Think();
}

void Light::LightUse(SVGBaseEntity* other, SVGBaseEntity* activator) {
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