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

// SpawnFlags.
#define START_OFF   1   
#define TRIGGERABLE 2

// Constructor/Deconstructor.
Light::Light(Entity* svEntity) : SVGBaseTrigger(svEntity) {

}
Light::~Light() {

}

// Interface functions. 
void Light::Precache() {
    // Parent class precache.
    Base::Precache();
}
void Light::Spawn() {
    // Parent class spawn.
    Base::Spawn();

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
            SVG_SetConfigString(ConfigStrings::Lights + style, "a");
        else
            SVG_SetConfigString(ConfigStrings::Lights + style, GetCustomLightStyle());
    }

    // Set default callbacks
    SetThinkCallback(&Light::LightThink);
    SetNextThinkTime(level.time + FRAMETIME);
}
void Light::PostSpawn() {
    // Parent class PostSpawn.
    Base::PostSpawn();
}
void Light::Think() {
    // Parent class think.
    Base::Think();
}

void Light::LightUse(SVGBaseEntity* other, SVGBaseEntity* activator) {
    // Get spawnflags.
    int32_t spawnFlags = GetSpawnFlags();

    // Fetch style.
    uint32_t style = GetStyle();

    if (GetSpawnFlags() & START_OFF) {
        // Switch style.
        SVG_SetConfigString(ConfigStrings::Lights + style, GetCustomLightStyle());

        // Change spawnflags, the light is on after all.
        SetSpawnFlags(spawnFlags & ~START_OFF);
    }
    else {
        SVG_SetConfigString(ConfigStrings::Lights + style, "a");
        SetSpawnFlags(spawnFlags | START_OFF);
    }
}

//
//===============
// Light::SpawnKey
//
//===============
//
void Light::SpawnKey(const std::string& key, const std::string& value) {
    // Wait.
    if (key == "style") {
        // Parsed float.
        int32_t parsedInteger = 0;

        // Parse.
        ParseIntegerKeyValue(key, value, parsedInteger);

        // Assign.
        SetStyle(parsedInteger);
    } 
    // Style.
    else if (key == "customLightStyle") {
        // Parsed float.
        std::string parsedString;

        // Parse.
        ParseStringKeyValue(key, value, parsedString);

        // Assign.
        SetCustomLightStyle(parsedString);
    }
    // Parent class spawnkey.
    else {
        SVGBaseEntity::SpawnKey(key, value);
    }
}

//
// Callback functions.
//
void Light::LightThink(void) {

}