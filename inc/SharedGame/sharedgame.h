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
//=============================================================================
//
//-----------------
// Brief Water Level.
//-----------------
struct WaterLevel {
    static constexpr int32_t Unknown = -1;
    static constexpr int32_t None = 0;
    static constexpr int32_t Feet = 1;
    static constexpr int32_t Waist = 2;
    static constexpr int32_t Under = 3;
};

//-----------------
// General player movement and capabilities classification.
//-----------------
struct PlayerMoveType {
    static constexpr uint8_t Normal = 0;    // Walking, jumping, falling, swimming, etc.
    static constexpr uint8_t Spectator= 1;  // Free-flying movement with acceleration and friction
    static constexpr uint8_t Noclip = 2;    // Like PM_SPECTATOR, but noclips through walls
    // All slots up till 32 are free for custom game PM_ defines.
};

//-----------------
// EntityState->effects
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
    static constexpr uint32_t Torch       = (1 << 17);
    static constexpr uint32_t Teleporter  = (1 << 24);

    // Maximum last effect slot, feel free to rename it and use it.
    static constexpr uint32_t Max = (1 << 31);
};

// PlayerState->refdef flags
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
    static constexpr uint8_t Shotgun = 18;
    static constexpr uint8_t SuperShotgun = 19;
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
    static constexpr uint8_t Shotgun = 1;
    static constexpr uint8_t Blaster = 2;
    static constexpr uint8_t Flare = 3;
    static constexpr uint8_t Blood = 10;
    static constexpr uint8_t MoreBlood = 11;

    static constexpr uint8_t Explosion1= 20;
    static constexpr uint8_t Explosion2 = 21;
    static constexpr uint8_t PlainExplosion = 22;
    static constexpr uint8_t BigExplosion1 = 23;
    static constexpr uint8_t NPExplosion1 = 24;

    static constexpr uint8_t Sparks = 50;
    static constexpr uint8_t BulletSparks = 51;
    static constexpr uint8_t ElectricSparks = 52;
    static constexpr uint8_t Splash = 60;

    static constexpr uint8_t BubbleTrail = 70;
    static constexpr uint8_t BubbleTrail2 = 71;
    static constexpr uint8_t DebugTrail = 79;

    static constexpr uint8_t Flame = 80;
    static constexpr uint8_t Steam = 90; 

    static constexpr uint8_t ForceWall = 100;
    static constexpr uint8_t TeleportEffect  = 101;


    static constexpr uint8_t Max = 255;
};

//-----------------
// Splash Types.
//-----------------
struct SplashType {
    static constexpr uint8_t Unknown = 0;
    static constexpr uint8_t Sparks = 1;
    static constexpr uint8_t BlueWater = 2;
    static constexpr uint8_t BrownWater = 3;
    static constexpr uint8_t Slime = 4;
    static constexpr uint8_t Lava = 5;
    static constexpr uint8_t Blood = 6;
};

//-----------------
// Deathmatch GameMode Setting Flags
//-----------------
struct GameModeFlags {
    static constexpr int16_t NoHealth       = (1 << 0);
    static constexpr int16_t NoItems        = (1 << 1);
    static constexpr int16_t WeaponsStay    = (1 << 2);
    static constexpr int16_t NoFalling      = (1 << 3);
    static constexpr int16_t InstantItems   = (1 << 4);
    static constexpr int16_t SameLevel      = (1 << 5);
    static constexpr int16_t SkinTeams      = (1 << 6);
    static constexpr int16_t ModelTeams     = (1 << 7);
    static constexpr int16_t NoFriendlyFire = (1 << 8);
    static constexpr int16_t SpawnFarthest  = (1 << 9);
    static constexpr int16_t ForceRespawn   = (1 << 10);
    static constexpr int16_t NoArmor        = (1 << 11);
    static constexpr int16_t AllowExit      = (1 << 12);
    static constexpr int16_t InfiniteAmmo   = (1 << 13);
    static constexpr int16_t FixedFOV       = (1 << 14);
};

#endif // __SHAREDGAME_SHAREDGAME_H__