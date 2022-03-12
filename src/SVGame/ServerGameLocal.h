/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// g_local.h -- local definitions for game module

#pragma once

#include "Shared/Shared.h"
#include "Shared/list.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible ServerClient and Entity structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "Shared/SVGame.h"
#include "SharedGame/SharedGame.h" // Include SG Base.
#include "SharedGame/Protocol.h"

// the "gameversion" client command will print32_t this plus compile date
#define GAMEVERSION "basepoly"


//-------------------
// Forward Declarations.
//-------------------
class SVGBaseEntity;
class SVGBaseItem;
class SVGBaseItemWeapon;
class SVGBasePlayer;
class Gameworld;
class IGamemode;
struct entity_s;

#include "GameLocals.h"
//==================================================================

//! MS Frametime for animations.
static constexpr float ANIMATION_FRAMETIME = BASE_FRAMETIME;

//! Float time it takes to go over a frame. 
static constexpr float FRAMETIME = BASE_FRAMETIME_1000;

//! Memory tags to allow dynamic memory to be cleaned up
static constexpr int32_t TAG_GAME = 765;   // clear when unloading the dll
static constexpr int32_t TAG_LEVEL = 766;  // clear when loading a new level

//! View pitching times
static constexpr float DAMAGE_TIME = 0.5f;
static constexpr float FALL_TIME = 0.3f;

/**
*   @brief Entity Spawn Flags. Can be set for each entity in the map editor. (Most likely TrenchBroom.)
**/
struct EntitySpawnFlags {
    static constexpr int32_t NotEasy = 0x00000100;
    static constexpr int32_t NotMedium = 0x00000200;
    static constexpr int32_t NotHard = 0x00000400;
    static constexpr int32_t NotDeathMatch= 0x00000800;
    static constexpr int32_t NotCoop = 0x00001000;
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

static constexpr int32_t MELEE_DISTANCE = 80;
static constexpr int32_t BODY_QUEUE_SIZE = 8;

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
*   Combat Ranges.
**/
struct CombatRange {
    static constexpr int32_t Melee  = 0;
    static constexpr int32_t Near   = 1;
    static constexpr int32_t Middle = 2;
    static constexpr int32_t Far    = 3;
};

/**
*   Gib Types.
**/
struct GibType {
    static constexpr int32_t Organic = 0;
    static constexpr int32_t Metallic = 1;
};

////monster ai flags
//struct 
//constexpr int32_t AI_STAND_GROUND = 0x00000001;
//constexpr int32_t AI_TEMP_STAND_GROUND = 0x00000002;
//constexpr int32_t AI_SOUND_TARGET = 0x00000004;
//constexpr int32_t AI_LOST_SIGHT = 0x00000008;
//constexpr int32_t AI_PURSUIT_LAST_SEEN = 0x00000010;
//constexpr int32_t AI_PURSUE_NEXT = 0x00000020;
//constexpr int32_t AI_PURSUE_TEMP = 0x00000040;
//constexpr int32_t AI_HOLD_FRAME = 0x00000080;
//constexpr int32_t AI_GOOD_GUY = 0x00000100;
//constexpr int32_t AI_BRUTAL = 0x00000200;
//constexpr int32_t AI_NOSTEP = 0x00000400;
//constexpr int32_t AI_DUCKED = 0x00000800;
//constexpr int32_t AI_COMBAT_POint32_t = 0x00001000;
//constexpr int32_t AI_MEDIC = 0x00002000;
//constexpr int32_t AI_RESURRECTING = 0x00004000;
//
////monster attack state
//constexpr int32_t AS_STRAIGHT = 1;
//constexpr int32_t AS_SLIDING = 2;
//constexpr int32_t AS_MELEE = 3;
//constexpr int32_t AS_MISSILE = 4;

/**
*   Player Handedness.
**/
constexpr int32_t RIGHT_HANDED = 0;
constexpr int32_t LEFT_HANDED = 1;
constexpr int32_t CENTER_HANDED = 2;


/**
*   Game specific server flags.
**/
struct ServerFlags {
    static constexpr int32_t CrossTrigger1      = 0x00000001;
    static constexpr int32_t CrossTrigger2      = 0x00000002;
    static constexpr int32_t CrossTrigger3      = 0x00000004;
    static constexpr int32_t CrossTrigger4      = 0x00000008;
    static constexpr int32_t CrossTrigger5      = 0x00000010;
    static constexpr int32_t CrossTrigger6      = 0x00000020;
    static constexpr int32_t CrossTrigger7      = 0x00000040;
    static constexpr int32_t CrossTrigger8      = 0x00000080;
    static constexpr int32_t CrossTriggerMask   = 0x000000ff;
};

/**
*   Player Noise Types.
**/
struct PlayerNoiseType {
    //! Jump, fall/land, getting hurt etc.
    static constexpr int32_t Self   = 0;
    //! Shooting a gun, reloading a gun, etc.
    static constexpr int32_t Weapon = 1;
    //! Noise created by impacting other entities with ballistics.
    static constexpr int32_t Impact = 2;
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
*   Item Flags.
**/
struct ItemFlags {
    static constexpr int32_t IsWeapon = 1;       // use makes active weapon
    static constexpr int32_t IsAmmo = 2;
    static constexpr int32_t IsArmor = 4;
    static constexpr int32_t StayInCoop = 8;
    static constexpr int32_t IsKey = 16;
    static constexpr int32_t IsPowerUp = 32;
};


//-------------------
// Stores level locals, from current time, to which entities are sighted.
// 
// This structure is cleared as each map is entered it is read/written 
// to the 'level.sav' file for savegames
//-------------------
struct LevelLocals  {
    // Current local level frame number.
    int32_t frameNumber = 0;

    //! Current sum of total frame time taken.
    float time = 0.f;
    //! Same as time, but multiplied by a 1000 to get a proper integer.
    int64_t timeStamp = 0;

    char levelName[MAX_QPATH];  // The descriptive name (Outer Base, etc)
    char mapName[MAX_QPATH];    // The server name (base1, etc)
    char nextMap[MAX_QPATH];    // Go here when fraglimit is hit

    //
    // Stores the state for a games 'Intermission state'. This is where the
    // camera hangs somewhere, at the end of a map, players get to say
    // 'hey ggg u n00bz' after seeing each other's scores. When all is said and
    // done the game moves on to the next map. This is when exitIntermission != 0
    //
    struct {
        float time; // Time the intermission was started
        const char* changeMap; // Map to switch to after intermission has exited.
        int32_t exitIntermission; // Set to true(1) when exiting the intermission should take place.
        vec3_t origin; // Origin for intermission to take place at.
        vec3_t viewAngle; // View angle to apply for intermission.
    } intermission;

    // The actual client the AI has sight on for this current frame.
    Entity *sightClient;  // Changed once each frame for coop games

    // Entity which the AI has sight on.
    Entity *sightEntity;
    int32_t sightEntityFrameNumber;

    // Sound entities are set to the entity that caused the AI to be triggered.
    Entity *soundEntity;            // In case of a footstep, jumping sound, etc.
    int32_t soundEntityFrameNumber;
    Entity *sound2Entity;           // In case of a weapon action.
    int32_t sound2EntityFrameNumber;

    // Not renaming this one, it has to go in the future.
    int32_t pic_health;

    // The current entity that is actively being ran from SVG_RunFrame.
    SVGBaseEntity *currentEntity;

    // Index for the que pile of dead bodies.
    int32_t bodyQue;

    // Ugly place for storing coop variables.
    int32_t powerCubes; // Ugly necessity for coop
};

//-------------------
// Holds entity field values that can be set from the editor, but aren't actualy present
// in Entity during gameplay
//-------------------
struct TemporarySpawnFields {
    // world vars
    char *sky;
    float skyrotate;
    vec3_t skyaxis;
    char *nextMap;

    int32_t lip;
    int32_t distance;
    int32_t height;
    const char *noise; // C++20: STRING: Added const to char *
    float pausetime;
    const char *item; // C++20: STRING: Added const to char *
    const char *gravity;    // C++20: STRING: Added const to char *

    float minyaw;
    float maxyaw;
    float minpitch;
    float maxpitch;
};


// Externized.
extern  GameLocals   game;
extern  LevelLocals  level;
extern  ServerGameImports gi;         // CLEANUP: These were game_import_t and game_export_T
extern  ServerGameExports globals;    // CLEANUP: These were game_import_t and game_export_T


/**
*   @return A pointer to the game's world object. The man that runs the show.
**/
Gameworld* GetGameworld();

/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
IGamemode* GetGamemode();

// These too need to be taken care of.
extern  int32_t sm_meat_index;
extern  int32_t snd_fry;

//-------------------
//
// Means of death
//
// Used for registring means of death types so they can be displayed 
// accordingly in the obituaries.
//-------------------
struct MeansOfDeath {
    static constexpr int32_t Unknown = 0;
    static constexpr int32_t Blaster = 1;
    static constexpr int32_t Shotgun = 2;
    static constexpr int32_t SuperShotgun = 3;
    static constexpr int32_t Machinegun = 4;
    static constexpr int32_t Unused = 5;
    static constexpr int32_t Grenade = 6;
    static constexpr int32_t GrenadeSplash = 7;
    static constexpr int32_t Rocket = 8;
    static constexpr int32_t RocketSplash = 9;

    static constexpr int32_t Water = 14;
    static constexpr int32_t Slime = 15;
    static constexpr int32_t Lava = 16;
    static constexpr int32_t Crush = 17;
    static constexpr int32_t TeleFrag = 18;
    static constexpr int32_t Falling = 19;
    static constexpr int32_t Suicide = 20;
    static constexpr int32_t Explosive = 21;
    static constexpr int32_t Barrel = 22;
    static constexpr int32_t Exit = 23;
    static constexpr int32_t Splash = 24;
    static constexpr int32_t TriggerHurt = 25;
    static constexpr int32_t Hit = 26;
    static constexpr int32_t FriendlyFire = 27;
};

//
// Small macros that are used to generate a field offset with. These are used
// in the save game system for example. Best not mess with these unless...
//
#define FOFS(x) q_offsetof(Entity, x)
#define STOFS(x) q_offsetof(TemporarySpawnFields, x)
#define LLOFS(x) q_offsetof(LevelLocals, x)
#define GLOFS(x) q_offsetof(GameLocals, x)
#define CLOFS(x) q_offsetof(ServerClient, x)

// Very ugly macros, need to rid ourselves and inline func them at the least.
// Also, there should be alternatives in our utils for math lib as is.
#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0f * (random() - 0.5f))

//-------------------
// Server game related cvars.
//-------------------
extern  cvar_t  *deathmatch;
extern  cvar_t  *coop;
extern  cvar_t  *gamemodeflags;
extern  cvar_t  *skill;
extern  cvar_t  *fraglimit;
extern  cvar_t  *timelimit;
extern  cvar_t  *password;
extern  cvar_t  *spectator_password;
extern  cvar_t  *needpass;
extern  cvar_t  *g_select_empty;
extern  cvar_t  *dedicated;

extern  cvar_t  *filterban;

extern  cvar_t  *sv_gravity;
extern  cvar_t  *sv_maxvelocity;

extern  cvar_t  *gun_x, *gun_y, *gun_z;
extern  cvar_t  *sv_rollspeed;
extern  cvar_t  *sv_rollangle;

extern  cvar_t  *run_pitch;
extern  cvar_t  *run_roll;
extern  cvar_t  *bob_up;
extern  cvar_t  *bob_pitch;
extern  cvar_t  *bob_roll;

extern  cvar_t  *sv_cheats;
extern  cvar_t  *maximumclients;
extern  cvar_t  *maxspectators;

extern  cvar_t  *flood_msgs;
extern  cvar_t  *flood_persecond;
extern  cvar_t  *flood_waitdelay;

extern  cvar_t  *sv_maplist;

extern  cvar_t  *sv_flaregun;

extern  cvar_t  *cl_monsterfootsteps;

//-------------------
// Spawnflags for items, set by editor(s).
//-------------------
struct ItemSpawnFlags {
    static constexpr int32_t TriggerSpawn = 0x00000001;
    static constexpr int32_t NoTouch = 0x00000002;
    // 6 bits reserved for editor flags
    // 8 bits used as power cube id bits for coop games
    static constexpr int32_t DroppedItem = 0x00010000;
    static constexpr int32_t DroppedPlayerItem = 0x00020000;
    static constexpr int32_t TargetsUsed = 0x00040000;
};


//
// fields are needed for spawning from the entity string
// and saving / loading games
//
typedef enum {
    F_BAD,
    F_BYTE,
    F_SHORT,
    F_INT,
    F_FLOAT,
    F_LSTRING,          // string on disk, pointer in memory, TAG_LEVEL
    F_GSTRING,          // string on disk, pointer in memory, TAG_GAME
    F_ZSTRING,          // string on disk, string in memory
    F_VECTOR,
    F_ANGLEHACK,
    F_EDICT,            // index on disk, pointer in memory
    F_ITEM,             // index on disk, pointer in memory
    F_CLIENT,           // index on disk, pointer in memory
    F_FUNCTION,
    F_POINTER,
    F_IGNORE
} fieldtype_t;

//extern  gitem_t itemlist[];


//
// g_cmds.c
//
void SVG_Command_Score_f(SVGBasePlayer *player, ServerClient *client);

//
// g_combat.c
//
qboolean SVG_OnSameTeam(SVGBaseEntity *ent1, SVGBaseEntity *ent2);

// damage flags
struct DamageFlags {
    static constexpr int32_t IndirectFromRadius = 0x00000001;  // Damage was indirect
    static constexpr int32_t NoArmorProtection = 0x00000002;  // Armour does not protect from this damage
    static constexpr int32_t EnergyBasedWeapon = 0x00000004;  // Damage is from an energy based weapon
    static constexpr int32_t NoKnockBack = 0x00000008;  // Do not affect velocity, just view angles
    static constexpr int32_t Bullet = 0x00000010;  // Damage is from a bullet (used for ricochets)
    static constexpr int32_t IgnoreProtection = 0x00000020;  // Armor, shields, invulnerability, and godmode have no effect
};

//
// g_weapon.c
//
qboolean SVG_FireHit(SVGBaseEntity *self, vec3_t &aim, int32_t damage, int32_t kick);
void SVG_FireBullet(SVGBaseEntity *self, const vec3_t& start, const vec3_t& aimdir, int32_t damage, int32_t kick, int32_t hspread, int32_t vspread, int32_t mod);
void SVG_FireShotgun(SVGBaseEntity *self, const vec3_t& start, const vec3_t& aimdir, int32_t damage, int32_t kick, int32_t hspread, int32_t vspread, int32_t count, int32_t mod);
void SVG_FireBlaster(SVGBaseEntity *self, const vec3_t& start, const vec3_t& aimdir, int32_t damage, int32_t speed, int32_t effect, qboolean hyper);

//
// g_ptrail.c
//
void SVG_PlayerTrail_Init(void);
void SVG_PlayerTrail_Add(vec3_t spot);
void SVG_PlayerTrail_New(vec3_t spot);
Entity *SVG_PlayerTrail_PickFirst(Entity *self);
Entity *SVG_PlayerTrail_PickNext(Entity *self);
Entity *SVG_PlayerTrail_LastSpot(void);

//
// g_svcmds.c
//
void    SVG_ServerCommand(void);
qboolean SVG_FilterPacket(char *from);

//
// g_phys.c
//
class SVGEntityHandle;
void SVG_RunEntity(SVGEntityHandle &entityHandle);

//
// g_main.c
//
//-----------------------------------------------------------------------------------------------------------

// TODO: All these go elsewhere, sometime, as does most...
void SVG_SetConfigString(const int32_t &configStringIndex, const std::string &configString);

//
// Custom server game trace struct, stores SVGBaseEntity* instead.
//
struct SVGTrace {
    SVGTrace() {
        allSolid = false;
        startSolid = false;
        fraction = 0.f;
        endPosition = vec3_t{ 0.f, 0.f, 0.f };
        surface = nullptr;
        contents = 0;
        ent = nullptr;
        offsets[0] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[1] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[2] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[3] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[4] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[5] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[6] = vec3_t{ 0.f, 0.f, 0.f };
        offsets[7] = vec3_t{ 0.f, 0.f, 0.f };
    }

    // If true, the trace startedand ended within the same solid.
    qboolean    allSolid;
    // If true, the trace started within a solid, but exited it.
    qboolean    startSolid;
    // The fraction of the desired distance traveled(0.0 - 1.0).If
    // 1.0, no plane was impacted.
    float       fraction;

    // The destination position.
    vec3_t      endPosition;

    // The impacted plane, or empty.Note that a copy of the plane is
    // returned, rather than a pointer.This is because the plane may belong to
    // an inline BSP model or the box hull of a solid entity, in which case it must
    // be transformed by the entity's current position.
    cplane_t    plane;
    // The impacted surface, or `NULL`.
    csurface_t* surface;
    // The contents mask of the impacted brush, or 0.
    int         contents;

    // The impacted entity, or `NULL`.
    SVGBaseEntity *ent;   // Not set by CM_*() functions

    // PH: Custom added.
    vec3_t		offsets[8];	// [signBits][x] = either size[0][x] or size[1][x]
};

SVGTrace SVG_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, SVGBaseEntity* passent, const int32_t& contentMask);

std::vector<SVGBaseEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount = MAX_EDICTS, int32_t areaType = AREA_SOLID);

qhandle_t SVG_PrecacheModel(const std::string& filename);
qhandle_t SVG_PrecacheImage(const std::string& filename);
qhandle_t SVG_PrecacheSound(const std::string& filename);

void SVG_CPrint(SVGBaseEntity* ent, int32_t printlevel, const std::string& str);
void SVG_DPrint(const std::string &str);
void SVG_CenterPrint(SVGBaseEntity* ent, const std::string& str);
void SVG_Sound(SVGBaseEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset);

//============================================================================

//-------------------
// Player Animations.
//-------------------
struct PlayerAnimation {
    static constexpr int32_t Basic = 0;       // Stand / Run
    static constexpr int32_t Wave = 1;
    static constexpr int32_t Jump = 2;
    static constexpr int32_t Pain = 3;
    static constexpr int32_t Attack = 4;
    static constexpr int32_t Death = 5;
    static constexpr int32_t Reverse = 6;
};

/**
*   @brief  The ClientPersistentData struct manages data that has to stay persistent
*           across level changes.
**/
struct ClientPersistentData {
    /**
    *   @brief Client Identity Data.
    **/
    //! User Info string.
    char userinfo[MAX_INFO_STRING];
    //! The client's beautiful netname string.
    char netname[16];
    //! The hand with which the player is holding the gun. (Left or right.)
    int32_t hand;
    //! Whether the client is currently actively connected, or not.
    qboolean isConnected = false;   // A loadgame will leave valid entities that
                                    // just don't have a connection yet
    //! Flags to save.
    int32_t savedFlags = 0;

    /***
    *   @brief Values saved and restored from clients when changing levels
    ***/
    struct {
        //! Client's health.
        int32_t health = 100;
        //! The maximum amount of health.
        int32_t maxHealth = 100;
    } stats;

    /**
    *   @brief Inventory member structure.
    **/
    struct {
        //! The currently active weapon item ID.
        int32_t activeWeaponID = ItemID::Barehands;
        //! The last active weapon ID.
        int32_t previousActiveWeaponID = 0;
        //! Used to store the next weapon to switch to, it is set when 
        //! the current weapon was still busy with an action.
        int32_t nextWeaponID = 0;

        //! All the items this client posesses.
        int32_t items[MAX_ITEMS] = {};

        //! Stores the clip ammo that is currently in a clip of said weaponID.
        int32_t clipAmmo[ItemID::MaxWeapons] = {};

        //! The item currently selected. NOTE: Not in use currently.
        //int32_t selectedItem = 0;

        //! Maximum Ammo Capacities for this client.
        int32_t maxAmmo9mm = 150;
    } inventory;
    
    //int32_t powerCubes = 0;    // Used for tracking the cubes in coop games
    int32_t score = 0;         // For calculating total unit score in coop games

    //! Spectator mode or not?
    qboolean isSpectator = false;          // client is a isSpectator
};

/**
*   @brief  The ClientRespawnData struct is used to store specific information about
*           respawning. Also maintains a member variable for data that has to stay
*           persistent during mapchanges/respawns in a coop game.
**/
struct ClientRespawnData {
    //! What to set client->persistent to in a coop game.
    ClientPersistentData persistentCoopRespawn;
    
    //! The level.frameNumber on which the player entered the game.
    int32_t enterGameFrameNumber;

    //! A client's score, usually this means the amount of frags.
    int32_t score;

    //! Angles sent over in the last client user command.
    vec3_t commandViewAngles;
    
    //! Is this client a spectator?
    qboolean isSpectator;
};


/**
*   @brief  The Game Client structure.
*
*   @details    This structure is cleared whenever PlacePlayerInGame is called,
*               with the exception of all data in its persistent member variable.
**/
struct gclient_s {
    //! Communicated by the server to the client.
    PlayerState  playerState;
    //! Client Ping.
    int32_t ping;

    //! Persistent data private to the game.
    ClientPersistentData persistent;
    //! Respawn data private to the game.
    ClientRespawnData respawn;

    //! Whether to generate a show score layout stat.
    qboolean showScores;
    //! Whether to generate a show inventory layout stat.
    qboolean showInventory;
	//! Whether to generate a show help layout stat.
    qboolean showHelpIcon;

    //! Primary ammo index.
    int32_t primaryAmmoIndex;
    //! Primary ammo index.
    int32_t secondaryAmmoIndex;
    //! Clip ammo index. Looks up in the inventory.clipAmmo array.
    int32_t clipAmmoIndex;

    //! State of buttons for this frame.
    int32_t buttons;
    //! State of buttons in the previous frame.
    int32_t oldButtons;
    //! Latched Buttons are used for single key push events.
    int32_t latchedButtons;

    struct WeaponState {
        struct Flags {
            //! Set if and only if an animation is still playing.
            static constexpr uint32_t IsAnimating           = 1 << 0;
            //! Set if and only if an animation is still playing.
            static constexpr uint32_t IsProcessingState     = 1 << 1;

            //! Is the weapon holstered?
            static constexpr uint32_t IsHolstered           = 1 << 2;
            
            ////! Set if the weapon is processing a primary fire.
            //static constexpr uint32_t PrimaryFire           = 1 << 2;
            ////! Set if the weapon is processing a secondary fire.
            //static constexpr uint32_t SecondaryFire       = 1 << 3;
        };

        //! Start time of the current active weapon state.
        uint32_t timeStamp = 0;
        
        //! Flags of the weapon's state, gets set to 0 after each successful weapon switch.
        uint32_t flags  = Flags::IsHolstered;
        
        //! None by default.
        int32_t current     = ::WeaponState::None;
        //! Same as 'current' by default.
        int32_t previous    = current;
        //! Queued weapon state to switch to after finishing the current state.
        int32_t queued      = ::WeaponState::None;

        //! Current frame the weapon animation(if any) is residing in. -1 if finished/none.
        int32_t animationFrame  = 0;
        //! Sound to play for this weapon frame.
        uint32_t sound          = 0;
    } weaponState;

    /**
    *   @brief  Used to sum up damage over an entire frame so weapons and other
    *           activities can give the player a single big kick.
    **/
    struct {
        //! Damage absorbed by the armor. (Unused.)
        int32_t armor = 0;
        //! Damage absorbed by the power armor. (Unused.)
        int32_t powerArmor = 0;
        //! Damage taken out of health, determines how much blood to display.
        int32_t blood = 0;
        //! Actual impact damage to knock the client back with.
        int32_t knockBack = 0;
        //! Origin of where the damage came from.
        vec3_t from = vec3_zero();
    } damages;

    //! Yaw angle of where our killer is located at in case we're dead.
    float killerYaw;

    //! Current kick angles.
    vec3_t kickAngles;
    //! Current kick origin.
    vec3_t kickOrigin;

    /**
    *   @brief  View damage kicks.
    **/
    struct {
        //! Roll for view damage kick.
        float roll  = 0.f;
        //! Pitch for view damage kick.
        float pitch = 0.f;
        //! Time of view damage.
        float time  = 0.f;
    } viewDamage;

    //! Falling time.
    float fallTime;
    //! Falling value for adding on to the view's pitch when landing.
    float fallValue;
    //! Alpha value for damage indicator display.
    float damageAlpha;
    //! Alpha value for bonus indicator display.
    float bonusAlpha;
    //! Total damage blend value.
    vec3_t damageBlend;
    //! Aiming direction.
    vec3_t aimAngles;
    //! Store bob time so going off-ground doesn't change it.
    float bobTime;

    //! Old view angles.
    vec3_t oldViewAngles;
    //! Old velocity.
    vec3_t oldVelocity;

    //! Timer for the next drown event.
    float nextDrownTime;
    //! Old water level.
    int32_t oldWaterLevel;

    //! For weapon raising
    int32_t machinegunShots;

    /**
    *   @brief  Animation state.
    **/
    struct ClientAnimationState {
        float endFrame;
        float priorityAnimation;

        qboolean    isDucking;
        qboolean    isRunning;
    } animation;

    //! Weapon Sound.
    int32_t weaponSound;

    //! Pick up message time.
    float pickupMessageTime;

    /**
    *   @brief  Used to protect from chat flooding.
    **/
    struct ClientFloodProtection {
        float lockTill;     // locked from talking
        float when[10];     // when messages were said
        int32_t whenHead;     // head pointer for when said
    } flood;

    //! Client can respawn when time > this
    float respawnTime;

    //! The (client)player we are chasing
    Entity *chaseTarget;

    //! Do we need to update chase info?
    qboolean updateChase;
};


/**
*   @brief  An std::map storing an entity's key/value dictionary.
**/
using EntityDictionary = std::map<std::string, std::string>;

/**
*   @brief  entity_s, the server side entity structure. If you know what an entity is,
*           then you know what this is.
*
*   @details    The actual SVGBaseEntity class is a member. It is where the magic happens.
*               Entities can be linked to their "classname", this will in turn make sure that
*               the proper inheritance entity is allocated.
**/
struct entity_s {
    //! Actual entity state member, a POD type that contains all data that is actually networked.
    EntityState state;

    //! NULL if not a player the server expects the first part of gclient_s to
    //! be a PlayerState but the rest of it is opaque
    struct gclient_s *client = nullptr;       

    //! An entity is in no use, in case it complies to the INUSE macro.
    qboolean inUse = false;
    //! Keeps track of whether this entity is linked, or unlinked.
    int32_t linkCount = 0;

    // FIXME: move these fields to a server private sv_entity_t
    //=================================
    list_t area; // Linked to a division node or leaf

    //! If numClusters is -1, use headNode instead.
    int32_t numClusters = 0;
    int32_t clusterNumbers[MAX_ENT_CLUSTERS] = {};

    //! Only use this instead of numClusters if numClusters == -1
    int32_t headNode = 0;
    int32_t areaNumber = 0;
    int32_t areaNumber2 = 0;
    //================================
    //! An entity's server state flags.
    int32_t serverFlags = 0;
    //! Min and max bounding box.
    vec3_t mins = vec3_zero(), maxs = vec3_zero();
    //! Absolute world transform bounding box.
    vec3_t absMin = vec3_zero(), absMax = vec3_zero(), size = vec3_zero();
    //! The type of 'Solid' this entity contains.
    uint32_t solid = Solid::Not;
    //! Clipping mask this entity belongs to.
    int32_t clipMask = 0;
    //! Pointer to the owning entity (if any.)
    Entity *owner = nullptr;

    //---------------------------------------------------
    // Do not modify the fields above, they're shared memory wise with the server itself.
    //---------------------------------------------------
    //! Actual class entity implementation pointer.
    SVGBaseEntity* classEntity = nullptr;

    //! Dictionary containing the initial key:value entity properties.
    EntityDictionary entityDictionary;

    //! Actual sv.time when this entity was freed.
    float freeTime = 0.f;

    // Move this to clientInfo?
    int32_t lightLevel = 0;
};