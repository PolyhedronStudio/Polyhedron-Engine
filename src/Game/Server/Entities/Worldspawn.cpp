/*
// LICENSE HERE.

//
// Worldspawn.cpp
//
//
*/

// Core.
#include "../ServerGameLocals.h"              // SVGame.

// Entities.
#include "../Entities.h"
#include "Worldspawn.h"

// GameModes.
#include "../GameModes/IGameMode.h"
#include "../GameModes/DeathMatchGameMode.h"

// World.
#include "../World/ServerGameWorld.h"

static const char single_statusbar[] =
"yb -24 "

// health
"xv 0 "
"hnum "
"xv 50 "
"pic 0 "

// ammo
"if 2 "
"   xv  100 "
"   anum "
"   xv  150 "
"   pic 2 "
"endif "

// armor
"if 4 "
"   xv  200 "
"   rnum "
"   xv  250 "
"   pic 4 "
"endif "

// selected item
"if 6 "
"   xv  296 "
"   pic 6 "
"endif "

"yb -50 "

// picked up item
"if 7 "
"   xv  0 "
"   pic 7 "
"   xv  26 "
"   yb  -42 "
"   stat_string 8 "
"   yb  -50 "
"endif "

// timer
"if 9 "
"   xv  262 "
"   num 2   10 "
"   xv  296 "
"   pic 9 "
"endif "

//  help / weapon icon
"if 11 "
"   xv  148 "
"   pic 11 "
"endif "
;

static const char dm_statusbar[] =
"yb -24 "

// health
"xv 0 "
"hnum "
"xv 50 "
"pic 0 "

// ammo
"if 2 "
"   xv  100 "
"   anum "
"   xv  150 "
"   pic 2 "
"endif "

// armor
"if 4 "
"   xv  200 "
"   rnum "
"   xv  250 "
"   pic 4 "
"endif "

// selected item
"if 6 "
"   xv  296 "
"   pic 6 "
"endif "

"yb -50 "

// picked up item
"if 7 "
"   xv  0 "
"   pic 7 "
"   xv  26 "
"   yb  -42 "
"   stat_string 8 "
"   yb  -50 "
"endif "

// timer
"if 9 "
"   xv  246 "
"   num 2   10 "
"   xv  296 "
"   pic 9 "
"endif "

//  help / weapon icon
"if 11 "
"   xv  148 "
"   pic 11 "
"endif "

//  frags
"xr -50 "
"yt 2 "
"num 3 14 "

// isSpectator
"if 17 "
"xv 0 "
"yb -58 "
"string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
"xv 0 "
"yb -68 "
"string \"Chasing\" "
"xv 64 "
"stat_string 16 "
"endif "
;

// Constructor/Deconstructor.
Worldspawn::Worldspawn(PODEntity *svEntity) : Base(svEntity) {

}

//
//===============
// Worldspawn::Precache
//
// You can precache all game related data here.
//===============
//
void Worldspawn::Precache() {
    // Parent class precache.
    Base::Precache();

    /**
    *   Audio Precache: General Weaponry.
    **/
    SVG_PrecacheSound("weapons/bullet_drop1.wav");
    SVG_PrecacheSound("weapons/bullet_drop2.wav");
    SVG_PrecacheSound("weapons/bullet_drop3.wav");
    SVG_PrecacheSound("weapons/hide_default.wav");
    SVG_PrecacheSound("weapons/holster_weapon1.wav");
    SVG_PrecacheSound("weapons/pickup1.wav");
    SVG_PrecacheSound("weapons/ready_generic1.wav");


    /**
    *   Image Precache: HUD Icons
    **/


    /**
    *   CVars.
    **/
    if (!globalGravity) {
        gi.cvar_set("sv_gravity", std::to_string(DEFAULT_GRAVITY).c_str());
    } else {
        gi.cvar_set("sv_gravity", std::to_string(globalGravity).c_str());
    }

    snd_fry = SVG_PrecacheSound("player/fry.wav");  // standing in lava / slime

    SVG_PrecacheSound("player/lava1.wav");
    SVG_PrecacheSound("player/lava2.wav");

    SVG_PrecacheSound("misc/talk.wav");

    SVG_PrecacheSound("misc/gibdeath1.wav");

    // Gibs
    SVG_PrecacheSound("items/respawn1.wav");

    // Sexed sounds
    SVG_PrecacheSound("player/death1.wav");
    SVG_PrecacheSound("player/death2.wav");
    SVG_PrecacheSound("player/death3.wav");
    SVG_PrecacheSound("player/death4.wav");
    SVG_PrecacheSound("player/fall1.wav");
    SVG_PrecacheSound("player/fall2.wav");
    SVG_PrecacheSound("*gurp1.wav");        // drowning damage
    SVG_PrecacheSound("*gurp2.wav");
    SVG_PrecacheSound("player/jump1.wav");        // player jump
    SVG_PrecacheSound("player/pain25_1.wav");
    SVG_PrecacheSound("player/pain25_2.wav");
    SVG_PrecacheSound("player/pain50_1.wav");
    SVG_PrecacheSound("player/pain50_2.wav");
    SVG_PrecacheSound("player/pain75_1.wav");
    SVG_PrecacheSound("player/pain75_2.wav");
    SVG_PrecacheSound("player/pain100_1.wav");
    SVG_PrecacheSound("player/pain100_2.wav");


    //-------------------

    SVG_PrecacheSound("player/gasp1.wav");      // gasping for air
    SVG_PrecacheSound("player/gasp2.wav");      // head breaking surface, not gasping

    SVG_PrecacheSound("player/watr_in.wav");    // feet hitting water
    SVG_PrecacheSound("player/watr_out.wav");   // feet leaving water

    SVG_PrecacheSound("player/watr_un.wav");    // head going underwater

    SVG_PrecacheSound("player/u_breath1.wav");
    SVG_PrecacheSound("player/u_breath2.wav");

    SVG_PrecacheSound("items/pkup.wav");        // bonus item pickup
    SVG_PrecacheSound("world/land.wav");        // landing thud
    SVG_PrecacheSound("misc/h2ohit1.wav");      // landing splash

    SVG_PrecacheSound("items/damage.wav");
    SVG_PrecacheSound("items/protect.wav");
    SVG_PrecacheSound("items/protect4.wav");
    SVG_PrecacheSound("weapons/noammo.wav");

    SVG_PrecacheSound("infantry/inflies1.wav");

    sm_meat_index = SVG_PrecacheModel("models/objects/gibs/sm_meat/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/arm/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/bone/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/bone2/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/chest/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/skull/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/head2/tris.md2");
}

//
//===============
// Worldspawn::Spawn
//
// Sets Worldspawn related properties. Nice right?
//===============
//
void Worldspawn::Spawn() {
    // Parent class spawn.
    Base::Spawn();

    // Set Worldspawn specific properties.
    SetMoveType(MoveType::Push);
    SetSolid(Solid::BSP);
    SetInUse(true);                 // Since the world doesn't use SVG_Spawn()
    SetModelIndex(1);               // World model is always index 1
    SetClipMask(0);
    //---------------



    // Configure item name configstrings.
    //SVG_SetItemNames();

    // Setup max client config strings.
    SVG_SetConfigString(ConfigStrings::MaxClients, std::to_string(maximumclients->integer));

    // Status bar program
    if (GetGameMode()->IsClass<DeathmatchGameMode>()) {
        SVG_SetConfigString(ConfigStrings::StatusBar, dm_statusbar);
    } else {
        SVG_SetConfigString(ConfigStrings::StatusBar, single_statusbar);
    }

    //---------------
    // Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
	for (int32_t i = 0; i < lightStylePresets.size(); i++) {
		SVG_SetConfigString( ConfigStrings::Lights + i, lightStylePresets[i] ); // 0 normal.
	}
    // Styles starting at 32 and up to 62 are assigned by the light program for switchable lights.
}

//===============
// Worldspawn::PostSpawn
//
// Placeholder, can be used though.
//===============
void Worldspawn::PostSpawn() {
    // Parent class PostSpawn.
    Base::PostSpawn();
}

//===============
// Worldspawn::Think
//
// Placeholder, can be used though.
//===============
void Worldspawn::Think() {
    // Parent class think.
    Base::Think();
}

//===============
// Worldspawn::SpawnKey
//
// This function can be overrided, to allow for custom entity key:value parsing.
//===============
void Worldspawn::SpawnKey(const std::string& key, const std::string& value) {
    if (key == "gravity") {
        // Parse Gravity.
        int32_t parsedInteger = 0;
        ParseKeyValue(key, value, parsedInteger);
        
        // Assign.
        globalGravity = parsedInteger;
    } else if (key == "message") {
        // Parse message.
        std::string message = "";
        ParseKeyValue(key, value, message);

        // Assign level name, in case there is one.
        if (message != "") {
            SVG_SetConfigString(ConfigStrings::Name, message.c_str());
        } else {
            strncpy(level.levelName, level.mapName, sizeof(level.levelName));
        }
    } else if (key == "nextmap") {
        // Parse message.
        std::string parsedString = "";
        ParseKeyValue(key, value, parsedString);

        // Assign.
	    if (!parsedString.empty()) {
            strcpy(level.nextMap, parsedString.c_str());
        }
    } else if (key == "sky") {
        // Parse message.
        std::string parsedString = "";
        ParseKeyValue(key, value, parsedString);

        // Assign.
        if (!parsedString.empty())
            SVG_SetConfigString(ConfigStrings::Sky, parsedString.c_str());
        else
            SVG_SetConfigString(ConfigStrings::Sky, "unit1_");
    } else if (key == "skyrotate") {
        // Parse skyrotate.
        float parsedFloat = 0.f;
        ParseKeyValue(key, value, parsedFloat);

        // Assign.
        SVG_SetConfigString(ConfigStrings::SkyRotate, va("%f", parsedFloat));

    } else if (key == "skyaxis") {
        // Parse skyaxis.
        vec3_t parsedVector = vec3_zero();
        ParseKeyValue(key, value, parsedVector);

        // Assign.
        SVG_SetConfigString(ConfigStrings::SkyAxis, va("%f %f %f", parsedVector.x, parsedVector.y, parsedVector.z));
    } else if (key == "sounds") {
        // Parse sounds.
        int32_t parsedInteger = 0;
        ParseKeyValue(key, value, parsedInteger);

        // Assign.
        SVG_SetConfigString(ConfigStrings::CdTrack, std::to_string(parsedInteger));
    } else {
        // Pass it on.
        Base::SpawnKey(key, value);
    }
}

//
//===============
// Worldspawn::WorldSpawnThink
//
// 'Think' callback placeholder, can be used though.
//===============
//
void Worldspawn::WorldspawnThink(void) {
    //SetThinkCallback(&Worldspawn::WorldSpawnThink);
    //SetNextThinkTime(level.time + 0.1f);
}