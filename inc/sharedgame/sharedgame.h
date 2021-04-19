/*
// LICENSE HERE.

//
// sharedgame/sharedgame.h
//
//
// Contains the definitions of the import, and export API structs.
//
*/

#ifndef __SHAREDGAME_SHAREDGAME_H__
#define __SHAREDGAME_SHAREDGAME_H__

//-----------------
// Button Bits
// 
// These are used for game logic. They are set in clg_input.cpp. One is free
// to use up the remaining slots for their own custom needs.
//-----------------
constexpr uint8_t BUTTON_ATTACK		= (1 << 0);
constexpr uint8_t BUTTON_USE			= (1 << 1);
constexpr uint8_t BUTTON_WALK			= (1 << 2);
constexpr uint8_t BUTTON_UNUSED_0		= (1 << 3);
constexpr uint8_t BUTTON_UNUSED_1		= (1 << 4);
constexpr uint8_t BUTTON_UNUSED_2		= (1 << 5);
constexpr uint8_t BUTTON_UNUSED_3		= (1 << 6);
constexpr uint8_t BUTTON_ANY			= (1 << 7);

//
//=============================================================================
//
//	Game Related, some is shared with the client/server.
// 
// TODO: Clean up, ensure that the client/server do not need these flags.
// So, do an ENGINE_ENTITY_FLAG, ENGINE_ENTITY_GIB, ENGINE_ENTITY_GAMEFLAG,
// 
// EF_SOMEFLAG = (ENGINE_ENTITY_GAMEFLAG << 0) etc.
//
//=============================================================================
//
//-----------------
// entity_state_t->effects
// Effects are things handled on the client side (lights, particles, 
// frame animations) that happen constantly on the given entity.
// 
// An entity that has effects will be sent to the client even if it has a zero
// index model.
//-----------------
struct EntityEffectType {
    // Animation Effects.
    static constexpr uint32_t AnimCycleFrames01hz2    = (1 << 0); // Auto cycle between the frames 0, and 1, at 2 hz.
    static constexpr uint32_t AnimCycleFrames23hz2    = (1 << 1); // Auto cycle between the frames 2, and 3, at 2 hz.
    static constexpr uint32_t AnimCycleAll2hz         = (1 << 2); // Auto cycle through all frames at 2 hz.
    static constexpr uint32_t AnimCycleAll30hz        = (1 << 3); // Auto cycle through all frames at 30 hz.
    
    static constexpr uint32_t ColorShell              = (1 << 6); // Color Shell around model.

    static constexpr uint32_t Rotate                  = (1 << 8); // Rotate (Items.)

    // Entity 'type' Effects that dictate special entity 'type' treatment.
    static constexpr uint32_t Gib     = (1 << 10);    // Entity is of type 'gib', and needs special treatment.
    static constexpr uint32_t Corpse  = (1 << 11);    // Entity is of type 'corpse', and needs special treatment.

    // 'Other' Effects. (Mostly null model entity stuff, weapon particles.)
    static constexpr uint32_t Blaster     = (1 << 16);
    static constexpr uint32_t Teleporter  = (1 << 24);

    // Maximum last effect slot, feel free to rename it and use it.
    static constexpr uint32_t Max = (1 << 31);
};

// player_state_t->refdef flags
#define RDF_UNDERWATER      1       // warp the screen as apropriate
#define RDF_NOWORLDMODEL    2       // used for player configuration screen

//ROGUE
#define RDF_IRGOGGLES       4
#define RDF_UVGOGGLES       8
//ROGUE

//-----------------
// muzzle flashes / player effects
//-----------------
struct MuzzleFlashType {


    // These aren't weapons, but are effects displayed in the player's view.
    // Hence, as such, they are actually treated as muzzleflashes.
    static constexpr uint8_t Respawn = 0;
    static constexpr uint8_t ItemRespawn = 1;
    static constexpr uint8_t Login = 2;
    static constexpr uint8_t Logout = 3;

    // Weapon Muzzleflashes.
    static constexpr uint8_t Blaster = 16;
    static constexpr uint8_t MachineGun = 17;
    static constexpr uint8_t Flare = 18;
};

//-----------------
// monster muzzle flashes
//-----------------
#define MZ2_SOLDIER_BLASTER_1           39
#define MZ2_SOLDIER_BLASTER_2           40
#define MZ2_SOLDIER_SHOTGUN_1           41
#define MZ2_SOLDIER_SHOTGUN_2           42
#define MZ2_SOLDIER_MACHINEGUN_1        43
#define MZ2_SOLDIER_MACHINEGUN_2        44

#define MZ2_SOLDIER_BLASTER_3           83
#define MZ2_SOLDIER_SHOTGUN_3           84
#define MZ2_SOLDIER_MACHINEGUN_3        85
#define MZ2_SOLDIER_BLASTER_4           86
#define MZ2_SOLDIER_SHOTGUN_4           87
#define MZ2_SOLDIER_MACHINEGUN_4        88
#define MZ2_SOLDIER_BLASTER_5           89
#define MZ2_SOLDIER_SHOTGUN_5           90
#define MZ2_SOLDIER_MACHINEGUN_5        91
#define MZ2_SOLDIER_BLASTER_6           92
#define MZ2_SOLDIER_SHOTGUN_6           93
#define MZ2_SOLDIER_MACHINEGUN_6        94
#define MZ2_SOLDIER_BLASTER_7           95
#define MZ2_SOLDIER_SHOTGUN_7           96
#define MZ2_SOLDIER_MACHINEGUN_7        97
#define MZ2_SOLDIER_BLASTER_8           98
#define MZ2_SOLDIER_SHOTGUN_8           99
#define MZ2_SOLDIER_MACHINEGUN_8        100

//-----------------
// Temp Entity Events (TE)
//
// Temp entity events are for things that happen at a location seperate from 
// any existing entity. Temporary entity messages are explicitly constructed
// and broadcast.
//-----------------
struct TempEntityEvent {
    static constexpr uint8_t Gunshot = 0;
    static constexpr uint8_t Blood = 1;
    static constexpr uint8_t Blaster = 2;
    static constexpr uint8_t Explosion1= 3;
    static constexpr uint8_t Explosion2 = 4;
    static constexpr uint8_t Sparks = 5;
    static constexpr uint8_t Splash = 6;
    static constexpr uint8_t BubbleTrail = 7;
    static constexpr uint8_t BulletSparks = 8;
    static constexpr uint8_t Flame = 9;
    static constexpr uint8_t DebugTrail = 10;
    static constexpr uint8_t PlainExplosion= 11;
    static constexpr uint8_t ForceWall = 12;
    static constexpr uint8_t Steam = 13; 
    static constexpr uint8_t BubbleTrail2 = 14;
    static constexpr uint8_t MoreBlood = 15;
    static constexpr uint8_t ElectricSparks = 16;
    static constexpr uint8_t TeleportEffect  = 17;
    static constexpr uint8_t BigExplosion1 = 18;
    static constexpr uint8_t NPExplosion1 = 19;
    static constexpr uint8_t Flare = 20;

    static constexpr uint8_t Max = 255;
};

//-----------------
// Splash Types.
//-----------------
#define SPLASH_UNKNOWN      0
#define SPLASH_SPARKS       1
#define SPLASH_BLUE_WATER   2
#define SPLASH_BROWN_WATER  3
#define SPLASH_SLIME        4
#define SPLASH_LAVA         5
#define SPLASH_BLOOD        6

//-----------------
// Deathmatch GameMode Setting Flags
//-----------------
#define DF_NO_HEALTH        0x00000001  // 1
#define DF_NO_ITEMS         0x00000002  // 2
#define DF_WEAPONS_STAY     0x00000004  // 4
#define DF_NO_FALLING       0x00000008  // 8
#define DF_INSTANT_ITEMS    0x00000010  // 16
#define DF_SAME_LEVEL       0x00000020  // 32
#define DF_SKINTEAMS        0x00000040  // 64
#define DF_MODELTEAMS       0x00000080  // 128
#define DF_NO_FRIENDLY_FIRE 0x00000100  // 256
#define DF_SPAWN_FARTHEST   0x00000200  // 512
#define DF_FORCE_RESPAWN    0x00000400  // 1024
#define DF_NO_ARMOR         0x00000800  // 2048
#define DF_ALLOW_EXIT       0x00001000  // 4096
#define DF_INFINITE_AMMO    0x00002000  // 8192
#define DF_FIXED_FOV        0x00004000  // 16384
#define DF_UNUSED           0x00008000  // 32768

#endif // __SHAREDGAME_SHAREDGAME_H__