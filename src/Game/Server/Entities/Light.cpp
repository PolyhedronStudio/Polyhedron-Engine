/***
*
*	License here.
*
*	@file
*
*	Server Light Entity Implementation.
*
***/
#include "../ServerGameLocals.h"

// Server Game Base Entity.
#include "Base/SVGBaseEntity.h"
#include "Base/SVGBaseTrigger.h"

// World.
#include "../World/ServerGameWorld.h"

// WorldSpawn : For lightStylePresets.
#include "Worldspawn.h"

// Misc Server Model Entity.
#include "Light.h"

// SpawnFlags.
static constexpr int32_t START_OFF		= 1;   
static constexpr int32_t TRIGGERABLE	= 2;

// Constructor/Deconstructor.
Light::Light(PODEntity *svEntity) : SVGBaseTrigger(svEntity) {

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
	SetSolid( Solid::Not );
	
    // Fetch possible Light Style.
    const int32_t _style = GetStyle();
		
	// Switch LightState On or Off depending on spawn flag that is set.
	if ( GetSpawnFlags() & START_OFF ) {
		// Our lightState is fully off.
		lightState = LightState::Off;

		// Set its lightstyle index to be "a" <-- lowest == off.
        SVG_SetConfigString(ConfigStrings::Lights + _style, "a");		
	} else {
		// Our lightState is switched on.
		lightState = LightState::On;

		// Built-in LightStyle Presets.
		if (_style < 32) {
			SVG_SetConfigString(ConfigStrings::Lights + _style, Worldspawn::lightStylePresets[_style] );
		// Custom Mapper LightStyle.
		} else {
			SVG_SetConfigString(ConfigStrings::Lights + _style, GetCustomLightStyle());
		}
	}

	// Set Use callback.
	SetUseCallback( &Light::LightUse );

    // Set NextThink callback.
    SetThinkCallback(&Light::LightThink);
    SetNextThinkTime(level.time + FRAMETIME);

	LinkEntity();
}
void Light::PostSpawn() {
    // Parent class PostSpawn.
    Base::PostSpawn();
}
void Light::Think() {
    // Parent class think.
    Base::Think();
}

void Light::LightUse(IServerGameEntity* other, IServerGameEntity* activator) {
    // Fetch possible Light Style.
    const int32_t _style = GetStyle();
		
	// Switch LightState On or Off depending on spawn flag that is set.
	if ( lightState == LightState::On ) {
		SetSpawnFlags( GetSpawnFlags() | START_OFF );
		// Our lightState is fully off.
		lightState = LightState::Off;

		// Set its lightstyle index to be "a" <-- lowest == off.
        SVG_SetConfigString(ConfigStrings::Lights + _style, "a");		
	} else {
		SetSpawnFlags( GetSpawnFlags() & ~START_OFF );
		// Our lightState is switched on.
		lightState = LightState::On;

		// Built-in LightStyle Presets.
		if (_style < 32) {
			SVG_SetConfigString(ConfigStrings::Lights + _style, Worldspawn::lightStylePresets[style] );
		// Custom Mapper LightStyle.
		} else {
			SVG_SetConfigString(ConfigStrings::Lights + _style, GetCustomLightStyle());
		}
	}

	// Fetch style string for debugging.
	const std::string styleStr = ( style < 32 ? Worldspawn::lightStylePresets[_style] : GetCustomLightStyle() );

	gi.DPrintf("MiscServerModel:Use(#%i): { lightState(%s) }, { lightStyle(#%i), str='%s' } \n",
		GetNumber(),
		lightState == LightState::On ? "\"On\"" : "\"Off\"",
		_style,
		styleStr.c_str()
	);
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
        ParseKeyValue(key, value, parsedInteger);

        // Assign.
        SetStyle(parsedInteger);
    } 
    // Style.
    else if (key == "customLightStyle") {
        // Parsed string.
        std::string parsedString;

        // Parse.
        ParseKeyValue(key, value, parsedString);

        // Assign.
        SetCustomLightStyle(parsedString);
    } else if (key == "spawnflags") {
		// Parse damage.
		int32_t parsedSpawnFlags = 0;
		ParseKeyValue(key, value, parsedSpawnFlags);

		// Set SpawnFlags.
		SetSpawnFlags(parsedSpawnFlags);
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