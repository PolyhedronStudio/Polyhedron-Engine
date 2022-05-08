/***
*
*	License here.
*
*	@file
*
*	Contains core types that are shared across client and server game dlls.
*
***/
#pragma once

// Include general shared header.
#ifdef SHAREDGAME_SERVERGAME
#define GAME_INCLUDE 1
#endif
#ifdef SHAREDGAME_CLIENTGAME
#define CGAME_INCLUDE 1
#endif

#include "../../Shared/Shared.h"



/***
*
*
*
*	Game Definitions that are shared across both modules.
*
*
*
***/


/**
*   Take Damage.
**/
struct TakeDamage {
    //! Will NOT take damage if hit.
    static constexpr int32_t No     = 0;  
    //! WILL take damage if hit
    static constexpr int32_t Yes    = 1;
    //! When auto targeting is enabled, it'll recognizes this
    static constexpr int32_t Aim    = 2; 
};

/**
*   Dead Flags.
**/
struct DeadFlags {
    static constexpr int32_t Alive = 0;
    static constexpr int32_t Dead = 1;
};

/**
*   Gib Types.
**/
struct GibType {
    static constexpr int32_t Organic = 0;
    static constexpr int32_t Metallic = 1;
};


/**
*   Entity Move Types.
**/
struct MoveType {
    //! An entity that never moves at all.
    static constexpr int32_t None       = 0;
    //! Spectator movement allows for flying around like noclip, but it does clip to BSP.
    static constexpr int32_t Spectator  = 1;
    //! Similar to spectator movement however it does not clip.
    static constexpr int32_t NoClip     = 2;
    //! No clip to world, push on box contact.
    static constexpr int32_t Push = 3;
    //! No clip to world, stops on box contact
    static constexpr int32_t Stop = 4;

    //! This entity makes use of the player movement code. 
    static constexpr int32_t PlayerMove = 10;    // Gravity. (Player Movement entities use this.)
    //! This entity makes use of the step movement physics.
    static constexpr int32_t Step       = 11;    // Fixed distance per frame, impacted by gravity, supports special edge handling.
    //! Similar to step movement however it does not care for gravity.
    static constexpr int32_t Fly        = 12;
    //! Similar to step fly movement, but with an extra size bounding box for hitting other entities. It's for missiles after all.
    static constexpr int32_t FlyMissile = 13;
    //! Toss is used for when dropping items, dying entities etc. It simply does basic velocity and gravity movement.
    static constexpr int32_t Toss       = 14;
    //! Similar to Toss but doesn't halt to a stop when having landed on-ground.
    static constexpr int32_t TossSlide  = 15;
    //! Similar to Toss but bounces back from the impacted surface instead.
    static constexpr int32_t Bounce     = 16;
};


/**
*   @brief Entity Flags. These are set in-game during gameplay.
**/
struct EntityFlags {
    static constexpr int32_t Fly            = 1;
    static constexpr int32_t Swim           = 2;        //! Implied immunity to drowining
    static constexpr int32_t ImmuneLaser    = 4;
    static constexpr int32_t InWater        = 8;
    static constexpr int32_t GodMode        = 16;
    static constexpr int32_t NoTarget       = 32;
    static constexpr int32_t ImmuneToSlime  = 64;
    static constexpr int32_t ImmuneToLava   = 128;
    static constexpr int32_t PartiallyOnGround = 256;   //! Not all corners are valid
    static constexpr int32_t WaterJump      = 512;      //! Player jumping out of water
    static constexpr int32_t TeamSlave      = 1024;     //! Not the first on the team
    static constexpr int32_t NoKnockBack    = 2048;
    static constexpr int32_t PowerArmor     = 4096;     //! Power armor (if any) is active
    static constexpr int32_t Respawn        = 0x80000000;   //! Used for item respawning
};


/**
*   @brief  These are used for game logic. They are set in clg_input.cpp. One is free
*           to use up the remaining slots for their own custom needs.
**/
struct ButtonBits {
    //! Button bit for when a player is primary firing.
    static constexpr uint8_t PrimaryFire    = (1 << 0);
    //! Button bit for when a player is secondary firing.
    static constexpr uint8_t SecondaryFire  = (1 << 1);
    //! Button bit for when a player is reloading its weapon.
    static constexpr uint8_t Reload         = (1 << 4);
    //! Button bit for when a player is using an entity.
    //! TODO: This is currently still unimplemented. Should replace "auto touch" buttons etc.
    static constexpr uint8_t Use            = (1 << 2);
    //! Button bit that is set when a player is moving.
    static constexpr uint8_t Walk           = (1 << 3);
    //! Unused 0.
    static constexpr uint8_t Unused0        = (1 << 5);
    //! Unused 1.
    static constexpr uint8_t Unused1        = (1 << 6);
    //! Set when any button is pressed.
    static constexpr uint8_t Any = (1 << 7);
};


/**
*   @brief  Player Weapon IDs are used to identify which weapon the player is actively
*           holding.
**/
//struct PlayerWeaponID {
//    //! Barehands. Currently unused.
//    static constexpr uint8_t Barehands  = 0;
//    //! Pistol.
//    static constexpr uint8_t Baretta    = 1;
//    //! SMG
//    static constexpr uint8_t SMG        = 2;
//    //! Shotgun
//    static constexpr uint8_t Shotgun    = 3;
//};

/**
*   @brief  Used to determine the state a weapon is currently in.
**/
struct WeaponState {
    //! None state, meaning it has no logic to process.
    static constexpr int32_t None = 0;
    //! Draw state, when set it'll process a draw animation.
    static constexpr int32_t Holster = 1;
    //! Holster state, when set it'll process a holster animation.
    static constexpr int32_t Draw = 2;
    //! Idle state, when set it'll play idle animations at random intervals.
    static constexpr int32_t Idle = 3;
    //! Reload, when set it'll try and reload.
    static constexpr int32_t Reload = 4;
    //! Primary Fire. Speaks for itself.
    static constexpr int32_t PrimaryFire = 5;
    //! Secondary Fire. This too, speaks for itself.
    static constexpr int32_t SecondaryFire = 6;
};

/**
*   @brief Item identifiers.
**/
struct ItemID { 
    /***
    * Weapons.
    ***/
    //! Bare hands.
    static constexpr uint32_t Barehands     = 1;
    //! Pistol.
    static constexpr uint32_t Beretta       = 2;
    //! SMG.
    static constexpr uint32_t SMG           = 3;
    //! Shotgun.
    static constexpr uint32_t Shotgun       = 4;
    //! Last item slot that can be used for weapons.
    static constexpr uint32_t MaxWeapons    = 64;

    /***
    * Ammo.
    ***/
    //! 9 millimeter ammo.
    static constexpr uint32_t Ammo9mm       = 65;
    //! 9 millimeter ammo.
    static constexpr uint32_t MaxAmmos      = 85;    

    /***
    * Medical Stats Items.
    ***/
    //! Mega Health.
    static constexpr uint32_t MegaHealth = 86;

    /**
    *   ... :-)
    **/
    //! Total amount of items.
    static constexpr uint32_t Total = 87; 
    //! Maximum amount of allowed items.
    static constexpr uint32_t Maximum = 255;
};

/**
*   Armor Types.
**/
struct ArmorType {
    static constexpr int32_t None = 0;
    static constexpr int32_t Jacket = 1;
    static constexpr int32_t Combat = 2;
    static constexpr int32_t Body = 3;
    static constexpr int32_t Shard = 4;
};


/**
*   @brief  GameMode specific flags such as FixedFOV, InstantItems, No Friendly Fire etc.
**/
struct GameModeFlags {
    static constexpr int16_t NoHealthItems      = (1 << 0);
    static constexpr int16_t NoItems            = (1 << 1);
    static constexpr int16_t NoFallingDamage    = (1 << 2);
    static constexpr int16_t SameLevel          = (1 << 3);
    static constexpr int16_t SkinTeams          = (1 << 4);
    static constexpr int16_t ModelTeams         = (1 << 5);
    static constexpr int16_t NoFriendlyFire     = (1 << 6);
    static constexpr int16_t ForceRespawn       = (1 << 7);
    static constexpr int16_t InfiniteAmmo       = (1 << 8);
    static constexpr int16_t FixedFOV           = (1 << 9);
};


/**
*   @brief  The water level that a certain entity is at.
**/
struct WaterLevel {
    static constexpr int32_t Unknown    = -1;
    static constexpr int32_t None       = 0;
    static constexpr int32_t Feet       = 1;
    static constexpr int32_t Waist      = 2;
    static constexpr int32_t Under      = 3;
};
/**
*   @brief  General player movement and capabilities classification.
*           One can add custom types up till index 32.
**/
struct PlayerMoveType {
    //! Default walking behavior, supports: Walking, jumping, falling, swimming, etc.
    static constexpr uint8_t Normal     = 0;
    //! Free-flying movement with acceleration and friction, no gravity.
    static constexpr uint8_t Spectator  = 1;
    //! Like Spectator, Free-Flying, but no clipping. Meaning you can move through walls and entities.
    static constexpr uint8_t Noclip     = 2;
};


/**
*   @details    Effects are things handled on the client side (lights, particles,
*               frame animations) that happen constantly on a given entity.
*
*               NOTE: An entity that has effects will be sent to the client even if it has a zero
*               index model.
**/
struct EntityEffectType {
    // Animation Effects.
    //! Auto cycle between the frames 0, and 1, at 2 hz.
    static constexpr uint32_t AnimCycleFrames01hz2  = (1 << 0);
    //! Auto cycle between the frames 2, and 3, at 2 hz.
    static constexpr uint32_t AnimCycleFrames23hz2  = (1 << 1);
    //! Auto cycle through all frames at 2 hz.
    static constexpr uint32_t AnimCycleAll2hz       = (1 << 2);
    //! Auto cycle through all frames at 30 hz.
    static constexpr uint32_t AnimCycleAll30hz      = (1 << 3);
    
    //! Color Shell around model.
    static constexpr uint32_t ColorShell    = (1 << 6);
    //! Rotate (Items.)
    static constexpr uint32_t Rotate        = (1 << 8);

    // Entity 'type' Effects that dictate special entity 'type' treatment.
    //! Entity is of type 'gib', and needs special treatment.
    static constexpr uint32_t Gib           = (1 << 10);
    //! Entity is of type 'corpse', and needs special treatment.
    static constexpr uint32_t Corpse        = (1 << 11);

    // 'Other' Effects. (Mostly null model entity stuff, weapon particles.)
    static constexpr uint32_t Blaster       = (1 << 16);
    static constexpr uint32_t Torch         = (1 << 17);
    static constexpr uint32_t Teleporter    = (1 << 24);

    // Maximum last effect slot, feel free to rename it and use it.
    static constexpr uint32_t Max = (1 << 31);
};


/**
*   @brief  Temp entity events are for things that happen at a location seperate from 
*           any existing entity. Temporary entity messages are explicitly constructed
*           and broadcast.
**/
struct TempEntityEvent {
    //! General gunshot particle effect.
    static constexpr uint8_t Gunshot = 0;
    //! Shotgun particle effect.
    static constexpr uint8_t Shotgun = 1;
    //! Blaster particle effect.
    static constexpr uint8_t Blaster = 2;
    //! Flare particle effect.
    static constexpr uint8_t Flare = 3;
    //! Bl00d particle effect.
    static constexpr uint8_t Blood = 10;
    //! M0r3 bl00d particle effect.
    static constexpr uint8_t MoreBlood = 11;

    //! Explosion 1 sprite and particle effect.
    static constexpr uint8_t Explosion1 = 20;
    //! Explosion 2 sprite and particle effect.
    static constexpr uint8_t Explosion2 = 21;
    //! Plain Explosion sprite and particle effect.
    static constexpr uint8_t PlainExplosion = 22;
    //! Big Explosion sprite and particle effect.
    static constexpr uint8_t BigExplosion1 = 23;
    //! Same as Explosion1, but without particles.
    static constexpr uint8_t NoParticleExplosion1 = 24;

	//! Spawns body gibs of said count.
	static constexpr uint8_t BodyGib = 30;
	//! Spawns debris gibs of said count and said debris type.
	static constexpr uint8_t DebrisGib = 31;

    //! General sparks particle effect.
    static constexpr uint8_t Sparks = 50;
    //! Bullet sparks particle effect.
    static constexpr uint8_t BulletSparks = 51;
    //! Electrical sparks particle effect.
    static constexpr uint8_t ElectricSparks = 52;
    //! Splash particle effect.
    static constexpr uint8_t Splash = 60;

    //! Bubble Trail 1 particle effect.
    static constexpr uint8_t BubbleTrailA = 70;
    //! Bubble Trail 2 particle effect.
    static constexpr uint8_t BubbleTrailB = 71;

    //! Flame sprite particle effect.
    static constexpr uint8_t Flame = 80;
    //! Steam sprite particle effect.
    static constexpr uint8_t Steam = 90;

    static constexpr uint8_t ForceWall = 100;
    static constexpr uint8_t TeleportEffect = 101;

    static constexpr uint8_t DebugTrail = 254;

    static constexpr uint8_t Max = 255;
};
/**
*   @brief  Splash Type determining the effect to be displayed for the Flash TE.
**/
struct SplashType {
    static constexpr uint8_t Unknown = 0;
    static constexpr uint8_t Sparks = 1;
    static constexpr uint8_t BlueWater = 2;
    static constexpr uint8_t BrownWater = 3;
    static constexpr uint8_t Slime = 4;
    static constexpr uint8_t Lava = 5;
    static constexpr uint8_t Blood = 6;
};

/**
*   @brief  Muzzle Flashes are effects applied to an entity itself and as such
*           take place at its origin.
**/
struct MuzzleFlashType {
    //! Shows a respawn particle effect at the entity's origin.
    static constexpr uint8_t Respawn     = 0;
    //! Shows an item specific respawn particle effect at the entity's origin.
    static constexpr uint8_t ItemRespawn = 1;
    //! Shows a login particle effect. (When a client connects and spawns for the first time in a game.)
    static constexpr uint8_t Login       = 2;
    //! Shows a logout particle effect. (When a client disconnects from a game.)
    static constexpr uint8_t Logout      = 3;

    //! Shows a Pistol muzzleflash effect.
    static constexpr uint8_t Smg45        = 16;
    //! Shows a MachineGun muzzleflash effect.
    static constexpr uint8_t MachineGun     = 17;
    //! Shows a Shotgun muzzleflash effect.
    static constexpr uint8_t Shotgun        = 18;
    //! Shows a SuperShotgun muzzleflash effect.
    static constexpr uint8_t SuperShotgun   = 19;
};


/**
*   @brief  Monster entity specific muzzleflash types.
**/
struct MonsterMuzzleFlashType {
    static constexpr int32_t FictionalMonsterWeapon = 0;
};


/**
*   @brief  Render Draw Flags for telling the client what screen effect to use.
**/
struct RenderDrawFlags {
    static constexpr int32_t Underwater = 1;    // warp the screen as apropriate
    static constexpr int32_t NoWorldModel = 2;  // used for player configuration screen
    static constexpr int32_t InfraRedGoggles = 4;
    static constexpr int32_t UVGoggles = 8;
};




/***
*
*
*
*	Using =, Predeclarations, and specific (Client-/Server-)Game includes.
*
*
*
***/
/**
*	Predeclare the actual game entity type that we intend to use depending on which game module
*	this file is being compiled along with.
**/
//! Predeclared for game locals include files.
class SGEntityHandle;
class ISharedGameEntity;

/**
*	Include needed headers for building SharedGame code for the ClientGame module.
*
*	Very ugly, but currently needed includes. Something that came of the old, needs to be cleaned in the future.
**/
#ifdef SHAREDGAME_CLIENTGAME
/**
*	Includes:
**/
// Needed define for said includes.
#define CGAME_INCLUDE 1
// Needed includes.
#include "../../Common/CModel.h"
#include "../../Common/Cmd.h"
#include "../../Common/Msg.h"
#include "../../Common/Protocol.h"
#include "../../Shared/SVGame.h"
#include "../../Shared/Refresh.h"
#include "../../Shared/CLTypes.h"
#include "../../Shared/CLGame.h"
//#endif

//! Actual game entity type for the ClientGame module.
class IClientGameEntity;
//! GameWorld.
class ClientGameWorld;
//! TraceResult.
struct CLGTraceResult;


/**
*	Using:
**/
//! Using: GameEntity
using GameEntity	= IClientGameEntity;
//! Using GameWorld.
using SGGameWorld	= ClientGameWorld;
//! Using: Trace Results
//using SGTraceResult	= CLGTraceResult;
//! Using: TouchTriggers
//using SGTouchTriggers = UTIL_TouchTriggers;




// Included ClientGameLocals.
#include "../Client/ClientGameLocals.h"

// Include IClientGameEntity.
#include "../Client/Entities/IClientGameEntity.h"

/**
*	Functions:
**/
//! Trace Function.
//extern CLGTraceResult CLG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, IClientGameEntity* passent, const int32_t& contentMask);//extern CLGTraceResult CLG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, GameEntity* passent, const int32_t& contentMask);
//using SGTrace = CLG_Trace;
//#define SGTrace CLG_Trace;


/**
*	ConstExpr:
**/
//! Maximum amount of POD Entities.
static constexpr int32_t MAX_POD_ENTITIES = MAX_CLIENT_POD_ENTITIES;

#endif //! End of ClientGame Specifics.
/**
*
*	SHAREDGAME: ServerGame
*
*	Here we place predeclarations, using =, and include minimal necessities.
*
**/
#ifdef SHAREDGAME_SERVERGAME
/**
*	Predeclarations:
**/
//! Actual game entity type for the ServerGame module.
class IServerGameEntity;
//! GameWorld.
class ServerGameWorld;
//! TraceResult.
struct SVGTraceResult;


/**
*	Type Using:
**/
//! Using: GameEntity.
using GameEntity = IServerGameEntity;
//! Using: GameWorld.
using SGGameWorld = ServerGameWorld;
//! Using: Trace Results.
//using SGTraceResult = SVGTraceResult;


/**
*	Includes:
**/
//! SVGame needed includes.
//#define GAME_INCLUDE
#include "../../Shared/SVGame.h"
//! ServerGameLocals.
#include "../Server/ServerGameLocals.h"
//! IServerGameEntity.
#include "../Server/Entities/IServerGameEntity.h"
//! Utilities.


/**
*	Functions:
**/
//! Trace Function.
//extern auto SVG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, GameEntity* passent, const int32_t& contentMask);
//#define SGTrace SVG_Trace;
//extern SVGTraceResult SVG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, GameEntity* skipEntity, const int32_t& contentMask);
//using SGTrace = SVG_Trace;
/**
*	ConstExpr:
**/
//! Maximum amount of POD Entities.
static constexpr int32_t MAX_POD_ENTITIES = MAX_SERVER_POD_ENTITIES;

#endif //! End of ServerGame Specifics.


/**
*   Protocol
**/
#include "Protocol.h"


/**
*   Skeletal Animation
**/
#include "SkeletalAnimation.h"


/**
*   Game Time Utilities.
**/
#include "Time.h"


/**
*	Tracing.
**/
#include "Tracing.h"


/**
*   Entity Framework
**/
#include "Entities.h"


/**
*   Protocol
**/
#include "PlayerMove.h"

/**
*	Physics
**/
#include "Physics/Physics.h"