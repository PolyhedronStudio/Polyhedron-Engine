/*
// LICENSE HERE.

//
// Light.cpp
//
//
*/

// Core.
#include "../ServerGameLocal.h"              // SVGame.
#include "../Entities.h"

// Entities.
#include "base/SVGBaseTrigger.h"
#include "Light.h"

// SpawnFlags.
#define START_OFF   1   
#define TRIGGERABLE 2

// Constructor/Deconstructor.
Light::Light(Entity* svEntity) : SVGBaseTrigger(svEntity), lightState(0) {

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
    // Light value.
    if (key == "light") {
        // Just here to prevent debug warnings about it not having a "light" key.
    }
    // Style Index #.
    else if (key == "style") {
        // Parsed int.
        int32_t parsedInteger = 0;

        // Parse.
        ParseIntegerKeyValue(key, value, parsedInteger);

        // Assign.
        SetStyle(parsedInteger);
    } 
    // Style.
    else if (key == "customLightStyle") {
        // Parsed string.
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