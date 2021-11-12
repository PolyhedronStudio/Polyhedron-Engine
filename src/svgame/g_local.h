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

#ifndef __SVGAME_G_LOCAL_H__
#define __SVGAME_G_LOCAL_H__

#include "shared/shared.h"
#include "shared/list.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible ServersClient and Entity structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "shared/svgame.h"
#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/protocol.h"

// the "gameversion" client command will print32_t this plus compile date
#define GAMEVERSION "basepoly"


//-------------------
// Forward Declaration.
//-------------------
class SVGBaseEntity;
class PlayerClient;
class IGameMode;
struct entity_s;

//==================================================================

// view pitching times
constexpr float DAMAGE_TIME = 0.5f;
constexpr float FALL_TIME = 0.3f;

// entity->spawnFlags
// these are set with checkboxes on each entity in the map editor
struct EntitySpawnFlags {
    static constexpr int32_t NotEasy = 0x00000100;
    static constexpr int32_t NotMedium = 0x00000200;
    static constexpr int32_t NotHard = 0x00000400;
    static constexpr int32_t NotDeathMatch= 0x00000800;
    static constexpr int32_t NotCoop = 0x00001000;
};

// entity->flags
struct EntityFlags {
    static constexpr int32_t Fly = 1;
    static constexpr int32_t Swim = 2; // Implied immunity to drowining
    static constexpr int32_t ImmuneLaser = 4;
    static constexpr int32_t InWater = 8;
    static constexpr int32_t GodMode = 16;
    static constexpr int32_t NoTarget = 32;
    static constexpr int32_t ImmuneToSlime = 64;
    static constexpr int32_t ImmuneToLava = 128;
    static constexpr int32_t PartiallyOnGround = 256;  // Not all corners are valid
    static constexpr int32_t WaterJump = 512; // Player jumping out of water
    static constexpr int32_t TeamSlave = 1024;  // Not the first on the team
    static constexpr int32_t NoKnockBack = 2048;
    static constexpr int32_t PowerArmor = 4096;  // Power armor (if any) is active
    static constexpr int32_t Respawn = 0x80000000;  // Used for item respawning
};

constexpr float FRAMETIME = BASE_FRAMETIME_1000; // Adjusts to the game's tick level.

// memory tags to allow dynamic memory to be cleaned up
constexpr int32_t TAG_GAME = 765;     // clear when unloading the dll
constexpr int32_t TAG_LEVEL = 766;     // clear when loading a new level


constexpr int32_t MELEE_DISTANCE = 80;

constexpr int32_t BODY_QUEUE_SIZE = 8;

//-------------------
// TakeDamage
//
// Add custom take damage conditions here.
//-------------------
struct TakeDamage {
    static constexpr int32_t No = 0;  // Will NOT take damage if hit
    static constexpr int32_t Yes = 1; // WILL take damage if hit
    static constexpr int32_t Aim = 2; // When auto targeting is enabled, it'll recognizes this
};

//-------------------
// WeaponState
//
// Add custom weapon states here.
//-------------------
struct WeaponState {
    static constexpr int32_t Ready = 0;
    static constexpr int32_t Activating = 1;
    static constexpr int32_t Dropping = 2;
    static constexpr int32_t Firing = 3;
};

//-------------------
// AmmoType
//
// Add ammo types here.
//-------------------
struct AmmoType {
    static constexpr int32_t Bullets = 0;
    static constexpr int32_t Shells = 1;
    static constexpr int32_t Rockets = 2;
    static constexpr int32_t Grenade = 3;
    static constexpr int32_t Cells = 4;
    static constexpr int32_t Slugs = 5;
};

//-------------------
// deadFlag
//-------------------
constexpr int32_t  DEAD_NO = 0;
constexpr int32_t  DEAD_DYING = 1;
constexpr int32_t  DEAD_DEAD = 2;
constexpr int32_t  DEAD_RESPAWNABLE = 3;

//-------------------
// Ranges
//-------------------
constexpr int32_t  RANGE_MELEE = 0;
constexpr int32_t  RANGE_NEAR = 1;
constexpr int32_t  RANGE_MID = 2;
constexpr int32_t  RANGE_FAR = 3;

//-------------------
//gib types
//-------------------
constexpr int32_t GIB_ORGANIC = 0;
constexpr int32_t GIB_METALLIC = 1;

//monster ai flags
constexpr int32_t AI_STAND_GROUND = 0x00000001;
constexpr int32_t AI_TEMP_STAND_GROUND = 0x00000002;
constexpr int32_t AI_SOUND_TARGET = 0x00000004;
constexpr int32_t AI_LOST_SIGHT = 0x00000008;
constexpr int32_t AI_PURSUIT_LAST_SEEN = 0x00000010;
constexpr int32_t AI_PURSUE_NEXT = 0x00000020;
constexpr int32_t AI_PURSUE_TEMP = 0x00000040;
constexpr int32_t AI_HOLD_FRAME = 0x00000080;
constexpr int32_t AI_GOOD_GUY = 0x00000100;
constexpr int32_t AI_BRUTAL = 0x00000200;
constexpr int32_t AI_NOSTEP = 0x00000400;
constexpr int32_t AI_DUCKED = 0x00000800;
constexpr int32_t AI_COMBAT_POint32_t = 0x00001000;
constexpr int32_t AI_MEDIC = 0x00002000;
constexpr int32_t AI_RESURRECTING = 0x00004000;

//monster attack state
constexpr int32_t AS_STRAIGHT = 1;
constexpr int32_t AS_SLIDING = 2;
constexpr int32_t AS_MELEE = 3;
constexpr int32_t AS_MISSILE = 4;

//-------------------
// Armor types
//-------------------
struct ArmorType {
    static constexpr int32_t None = 0;
    static constexpr int32_t Jacket = 1;
    static constexpr int32_t Combat = 2;
    static constexpr int32_t Body = 3;
    static constexpr int32_t Shard = 4;
};

//-------------------
// Power armor types
//-------------------
constexpr int32_t POWER_ARMOR_NONE = 0;

//-------------------
// Player handedness values
//-------------------
constexpr int32_t RIGHT_HANDED = 0;
constexpr int32_t LEFT_HANDED = 1;
constexpr int32_t CENTER_HANDED = 2;


//-------------------
// game.serverflags values
//-------------------
constexpr int32_t SFL_CROSS_TRIGGER_1 = 0x00000001;
constexpr int32_t SFL_CROSS_TRIGGER_2 = 0x00000002;
constexpr int32_t SFL_CROSS_TRIGGER_3 = 0x00000004;
constexpr int32_t SFL_CROSS_TRIGGER_4 = 0x00000008;
constexpr int32_t SFL_CROSS_TRIGGER_5 = 0x00000010;
constexpr int32_t SFL_CROSS_TRIGGER_6 = 0x00000020;
constexpr int32_t SFL_CROSS_TRIGGER_7 = 0x00000040;
constexpr int32_t SFL_CROSS_TRIGGER_8 = 0x00000080;
constexpr int32_t SFL_CROSS_TRIGGER_MASK = 0x000000ff;

//-------------------
// Noise types for SVG_PlayerNoise
//-------------------
constexpr int32_t PNOISE_SELF = 0;
constexpr int32_t PNOISE_WEAPON = 1;
constexpr int32_t PNOISE_IMPACT = 2;


//-------------------
// Actual entity movetypes that can be employed. 
//-------------------
// Entity moveType values
struct MoveType {
    static constexpr int32_t None = 0;      // Never moves
    static constexpr int32_t Spectator = 1; // Special movetype for spectators to not go through walls
    static constexpr int32_t NoClip = 2;    // Origin and angles change with no interaction
    static constexpr int32_t Push = 3;      // No clip to world, push on box contact
    static constexpr int32_t Stop = 4;      // No clip to world, stops on box contact

    static constexpr int32_t Walk          = 10;    // Gravity. (Player Movement entities use this.)
    static constexpr int32_t Step          = 11;    // Gravity, fixed distance, and special edge handling if wished for.
    static constexpr int32_t Fly           = 12;    // 
    static constexpr int32_t Toss          = 13;    // Gravity
    static constexpr int32_t FlyMissile    = 14;    // Extra size to monsters
    static constexpr int32_t Bounce        = 15;
};


//-------------------
// Armor item description.
//-------------------
struct gitem_armor_t {
    int32_t     baseCount;
    int32_t     maxCount;
    float   normalProtection;
    float   energyProtection;
    int32_t     armor;
};


//-------------------
// Item flags.
//-------------------
struct ItemFlags {
    static constexpr int32_t IsWeapon = 1;       // use makes active weapon
    static constexpr int32_t IsAmmo = 2;
    static constexpr int32_t IsArmor = 4;
    static constexpr int32_t StayInCoop = 8;
    static constexpr int32_t IsKey = 16;
    static constexpr int32_t IsPowerUp = 32;
};

//-------------------
// Weapon model indexes for: gitem_t->weaponModelIndex
//-------------------
constexpr int32_t WEAP_BLASTER = 1;
constexpr int32_t WEAP_MACHINEGUN = 2;
constexpr int32_t WEAP_SHOTGUN = 3;
constexpr int32_t WEAP_SUPERSHOTGUN = 4;

//-------------------
// Special item entity structure. This is a hybrid of ammo, health,
// and actual weapons. 
//-------------------
typedef struct gitem_s {
    // Spawning classname.
    const char *className;

    // Function callbacks.
    qboolean (*Pickup)(SVGBaseEntity *ent, PlayerClient *other);
    void (*Use)(PlayerClient *ent, struct gitem_s *item);
    void (*Drop)(PlayerClient *ent, struct gitem_s *item);
    void (*WeaponThink)(PlayerClient *ent);

    // Sound used when being picked up.
    const char *pickupSound;

    // Actual world model used to display.
    const char *worldModel;
    
    // Specific worldmodel flags.
    int32_t worldModelFlags;

    // Item view model. (Used for weapons, weapons are items.)
    const char        *viewModel;

    // Client side infe.
    const char  *icon;
    const char  *pickupName;    // for printing on pickup
    int32_t countWidth;             // number of digits to display by icon

    // For ammo items this value dictates how much ammo to gain.
    // For weapon items, this value dictates how much ammo is used on a per shot basis.
    int32_t quantity;

    // Ammo string for weapons.
    const char  *ammo;

    // IT_* flags
    int32_t flags;          

    // Weaponmodel index.
    int32_t weaponModelIndex;      // weapon model index (for weapons)

    // Info string & Tag.
    void *info;
    int32_t tag;

    // An actual string of all models, sounds, and images this item will use
    const char  *precaches;     
} gitem_t;



//-------------------
// This structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//-------------------
struct GameLocals {
    // Game Mode interface, always assigned to whichever game mode we are running.
    IGameMode* gameMode;

    // List of clients, based on sv_maxclients, or rather in the game dll: maxclients cvar.
    ServersClient *clients;

    // Can't store spawnpoint32_t in level, because
    // it would get overwritten by the savegame restore
    char spawnpoint[512];    // needed for coop respawns

    // Store latched cvars here that we want to get at often
    int32_t maximumClients;
    int32_t maxEntities;

    // Cross level triggers
    int32_t serverflags;

    // Items
    int32_t numberOfItems;

    // Did we autosave?
    qboolean autoSaved;
};


//-------------------
// Stores level locals, from current time, to which entities are sighted.
// 
// This structure is cleared as each map is entered it is read/written 
// to the 'level.sav' file for savegames
//-------------------
struct LevelLocals  {
    // Current local level frame number.
    int32_t frameNumber;

    // Current local level time.
    float time;

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

    // Total level Monster stats.
    int32_t totalMonsters;
    int32_t killedMonsters;

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

// Wrap these in functions such as?:
// SVG_GetGameLocals
// SVG_GetLevelLocals
// 
extern  GameLocals   game;
extern  LevelLocals  level;
extern  ServerGameImports gi;         // CLEANUP: These were game_import_t and game_export_T
extern  ServerGameExports globals;    // CLEANUP: These were game_import_t and game_export_T
extern  TemporarySpawnFields    st;

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

// Once again, ugly.
extern Entity g_entities[MAX_EDICTS];
extern SVGBaseEntity* g_baseEntities[MAX_EDICTS];


//
// Small macros that are used to generate a field offset with. These are used
// in the save game system for example. Best not mess with these unless...
//
#define FOFS(x) q_offsetof(Entity, x)
#define STOFS(x) q_offsetof(TemporarySpawnFields, x)
#define LLOFS(x) q_offsetof(LevelLocals, x)
#define GLOFS(x) q_offsetof(GameLocals, x)
#define CLOFS(x) q_offsetof(ServersClient, x)

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
extern  cvar_t  *maximumClients;
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

extern  gitem_t itemlist[];


//
// g_cmds.c
//
void SVG_Command_Score_f(SVGBaseEntity *ent);

//
// g_items.c
//
void SVG_PrecacheItem(gitem_t *it);
void SVG_InitItems(void);
void SVG_SetItemNames(void);
gitem_t *SVG_FindItemByPickupName(const char *pickup_name);
gitem_t *SVG_FindItemByClassname(const char *className);
#define ITEM_INDEX(x) ((x)-itemlist)
Entity *SVG_DropItem(Entity *ent, gitem_t *item);
void SVG_SetRespawn(Entity *ent, float delay);
void SVG_ChangeWeapon(PlayerClient* ent);
void SVG_SpawnItem(Entity *ent, gitem_t *item);
//void SVG_ThinkWeapon(Entity *ent);
int32_t SVG_ArmorIndex(SVGBaseEntity *ent);
gitem_t *SVG_GetItemByIndex(int32_t index);
qboolean SVG_AddAmmo(Entity *ent, gitem_t *item, int32_t count);
void SVG_TouchItem(SVGBaseEntity* ent, SVGBaseEntity* other, cplane_t *plane, csurface_t *surf);

//
// g_combat.c
//
qboolean SVG_OnSameTeam(SVGBaseEntity *ent1, SVGBaseEntity *ent2);
qboolean SVG_CanDamage(SVGBaseEntity *targ, SVGBaseEntity *inflictor);
void SVG_InflictDamage(SVGBaseEntity *targ, SVGBaseEntity *inflictor, SVGBaseEntity *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int32_t damage, int32_t knockback, int32_t dflags, int32_t mod);
void SVG_InflictRadiusDamage(SVGBaseEntity *inflictor, SVGBaseEntity *attacker, float damage, SVGBaseEntity *ignore, float radius, int32_t mod);

// damage flags
struct DamageFlags {
    static constexpr int32_t IndirectFromRadius = 0x00000001;  // damage was indirect
    static constexpr int32_t NoArmorProtection = 0x00000002;  // armour does not protect from this damage
    static constexpr int32_t EnergyBasedWeapon = 0x00000004;  // damage is from an energy based weapon
    static constexpr int32_t NoKnockBack = 0x00000008;  // do not affect velocity, just view angles
    static constexpr int32_t Bullet = 0x00000010;  // damage is from a bullet (used for ricochets)
    static constexpr int32_t IgnoreProtection = 0x00000020;  // armor, shields, invulnerability, and godmode have no effect
};

//
// g_weapon.c
//
void SVG_ThrowDebris(SVGBaseEntity *self, const char *modelname, float speed, const vec3_t& origin);
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
// g_player.c
//
void SVG_Client_Pain(Entity *self, Entity *other, float kick, int32_t damage);
void SVG_Client_Die(Entity *self, Entity *inflictor, Entity *attacker, int32_t damage, const vec3_t& point);

//
// g_svcmds.c
//
void    SVG_ServerCommand(void);
qboolean SVG_FilterPacket(char *from);

//
// g_pweapon.c
//
void SVG_PlayerNoise(SVGBaseEntity *who, vec3_t where, int32_t type);

//
// g_phys.c
//
void SVG_RunEntity(SVGBaseEntity *ent);

//
// g_main.c
//
//-----------------------------------------------------------------------------------------------------------
void SVG_SaveClientData(void);
void SVG_FetchClientData(Entity *ent);

Entity* SVG_Spawn(void);

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

    // N&C: Custom added.
    vec3_t		offsets[8];	// [signbits][x] = either size[0][x] or size[1][x]
};

SVGTrace SVG_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, SVGBaseEntity* passent, const int32_t& contentMask);

std::vector<SVGBaseEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount = MAX_EDICTS, int32_t areaType = AREA_SOLID);

qhandle_t SVG_PrecacheModel(const std::string& filename);
qhandle_t SVG_PrecacheImage(const std::string& filename);
qhandle_t SVG_PrecacheSound(const std::string& filename);

void SVG_CenterPrint(SVGBaseEntity* ent, const std::string& str);
void SVG_Sound(SVGBaseEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset);


//-------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// g_chase.c
//


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

//-------------------
// The ClientPersistentData struct manages data that has to stay persistent
// across level changes.
//-------------------
struct ClientPersistentData {
    char userinfo[MAX_INFO_STRING];
    char netname[16];
    int32_t hand;

    qboolean isConnected;  // A loadgame will leave valid entities that
                           // just don't have a connection yet

    // Values saved and restored from entities when changing levels
    int32_t health;
    int32_t maxHealth;
    int32_t savedFlags;

    int32_t selectedItem;
    int32_t inventory[MAX_ITEMS];

    // Ammo capacities
    int32_t maxBullets;
    int32_t maxShells;
    int32_t maxRockets;
    int32_t maxGrenades;
    int32_t maxCells;
    int32_t maxSlugs;

    gitem_t *activeWeapon;
    gitem_t *lastWeapon;

    int32_t powerCubes;    // Used for tracking the cubes in coop games
    int32_t score;         // For calculating total unit score in coop games

    qboolean isSpectator;          // client is a isSpectator
};

//-------------------
// The ClientRespawnData struct is used to store specific information about
// respawning. Also maintains a member variable for data that has to stay
// persistent during mapchanges/respawns in a coop game.
//-------------------
struct ClientRespawnData {
    ClientPersistentData persistentCoopRespawn;   // What to set client->persistent to on a respawn
    int32_t enterGameFrameNumber;       // level.frameNumber the client entered the game
    int32_t score;                      // frags, etc
    vec3_t commandViewAngles;           // angles sent over in the last command

    qboolean isSpectator;               // client is a isSpectator
};


//-------------------
// The gclient_s, Game Client structure.
// 
// Whenever PutClientInServer is called, this structure is cleared.
// The only thing that maintains its data is the persistent member.
// 
// This is to maintain several specific client data across maps.
//-------------------
struct gclient_s {
    // known to server
    PlayerState  playerState;             // communicated by server to clients
    int32_t ping;

    // private to game
    ClientPersistentData persistent;
    ClientRespawnData respawn;

    qboolean showScores;         // set layout stat
    qboolean showInventory;      // set layout stat
    qboolean showHelpIcon;

    int32_t ammoIndex;

    int32_t buttons;
    int32_t oldButtons;
    int32_t latchedButtons;     // These are used for one time push events.

    qboolean weaponThunk;

    gitem_t *newWeapon;

    // sum up damage over an entire frame, so
    // shotgun blasts give a single big kick
    struct {
        int32_t armor;       // damage absorbed by armor
        int32_t powerArmor;      // damage absorbed by power armor
        int32_t blood;       // damage taken out of health
        int32_t knockBack;   // impact damage
        vec3_t from;        // origin for vector calculation
    } damages;

    float killerYaw;         // when dead, look at killer

    int32_t weaponState;

    vec3_t kickAngles;    // weapon kicks
    vec3_t kickOrigin;

    // View damage kicks.
    struct {
        float roll;
        float pitch;
        float time;
    } viewDamage;

    float fallTime, fallValue;      // for view drop on fall
    float damageAlpha;
    float bonusAlpha;
    vec3_t damageBlend;
    vec3_t aimAngles;            // aiming direction
    float bobTime;            // so off-ground doesn't change it

    // Old view angles and velocity.
    vec3_t oldViewAngles;
    vec3_t oldVelocity;

    float nextDrownTime;
    int32_t oldWaterLevel;

    // For weapon raising
    int32_t machinegunShots;

    // animation vars
    struct {
        int32_t     endFrame;
        int32_t     priorityAnimation;
        qboolean    isDucking;
        qboolean    isRunning;
    } animation;

    // Weapon Sound.
    int32_t weaponSound;

    // Pick up message time.
    float pickupMessageTime;

    // Flood protection struct.
    struct {
        float lockTill;     // locked from talking
        float when[10];     // when messages were said
        int32_t whenHead;     // head pointer for when said
    } flood;

    // Client can respawn when time > this
    float respawnTime;

    // The (client)player we are chasing
    Entity *chaseTarget;

    // Do we need to update chase info?
    qboolean updateChase;
};

//-------------------
// entity_s, the server side entity structure. If you know what an entity is,
// then you know what this is.
// 
// The actual SVGBaseEntity class is a member. It is where the magic happens.
// Entities can be linked to their "classname", this will in turn make sure that
// the proper inheritance entity is allocated.
//-------------------
using EntityDictionary = std::map<std::string, std::string>;

struct entity_s {
    // Actual entity state member. Contains all data that is actually networked.
    EntityState  state;

    // NULL if not a player the server expects the first part of gclient_s to
    // be a PlayerState but the rest of it is opaque
    struct gclient_s *client;       

    // An entity is in no use, in case it complies to the INUSE macro.
    qboolean inUse;
    int32_t linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t area; // Linked to a division node or leaf

    // If numClusters is -1, use headNodew instead.
    int32_t numClusters;       // if -1, use headNode instead
    int32_t clusterNumbers[MAX_ENT_CLUSTERS];

    // Only use this instead of numClusters if numClusters == -1
    int32_t headNode;           
    int32_t areaNumber;
    int32_t areaNumber2;

    //================================
    int32_t serverFlags;
    vec3_t mins, maxs;
    vec3_t absMin, absMax, size;
    uint32_t solid;
    int32_t clipMask;
    Entity *owner;

    // !!!!!!!!!!!!!!!!!
    // !! DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
    // !! EXPECTS THE FIELDS IN THAT ORDER!
    // !!!!!!!!!!!!!!!!!
    //================================
    // Pointer to the actual game class entity belonging to this server entity.
    SVGBaseEntity* classEntity;

    // Hashmap containing the key:value entity properties.
    EntityDictionary entityDictionary;

    //const char *model;       // C++20: STRING: Added const to char*
    float freeTime;     // sv.time when the object was freed

    //
    // only used locally in game, not by server
    //
    const char *message;     // C++20: STRING: Added const to char *
    const char *className;   // C++20: STRING: Made const.
    
    float timeStamp;

    char *target;
    const char *targetName;
    char *killTarget;
    char *team;
    char *pathTarget;
    Entity *targetEntityPtr;

    // For moving objects(plats, etc)
    float speed;
    float acceleration;
    float deceleration;
    vec3_t moveDirection;
    vec3_t position1, position2;

    // Regular entity velocity, gravity, mass.
    Entity *goalEntityPtr;
    Entity *moveTargetPtr;

    const char *map;           // target_changelevel // C++20: STRING: Added const to char *
    int32_t count;

    // Chain, enemy, old enemy, and activator entity pointers.
    Entity *chain;
    
    // Ground pointers.
    Entity *groundEntityPtr;

    Entity *myNoisePtr;       // can go in client only
    Entity *myNoise2Ptr;

    int32_t noiseIndex;
    int32_t noiseIndex2;
    float volume;
    float attenuation;

    // Timing variables
    float random;

    float teleportTime;



    vec3_t moveOrigin;
    vec3_t moveAngles;

    // move this to clientInfo?
    int32_t lightLevel;

    int32_t style;          // also used as areaportal number

    // Custom lightstyle.
    char *customLightStyle;

    gitem_t *item;          // for bonus items
};

#endif