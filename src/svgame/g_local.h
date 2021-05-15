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
// short, server-visible GameClient and Entity structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "shared/svgame.h"
#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/protocol.h"

// the "gameversion" client command will print this plus compile date
#define GAMEVERSION "basenac"

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
    static constexpr int32_t Fly = 0x00000001;
    static constexpr int32_t Swim = 0x00000002; // Implied immunity to drowining
    static constexpr int32_t ImmuneLaser = 0x00000004;
    static constexpr int32_t InWater = 0x00000008;
    static constexpr int32_t GodMode = 0x00000010;
    static constexpr int32_t NoTarget = 0x00000020;
    static constexpr int32_t ImmuneToSlime = 0x00000040;
    static constexpr int32_t ImmuneToLava = 0x00000080;
    static constexpr int32_t PartiallyOnGround = 0x00000100;  // Not all corners are valid
    static constexpr int32_t WaterJump = 0x00000200; // Player jumping out of water
    static constexpr int32_t TeamSlave = 0x00000400;  // Not the first on the team
    static constexpr int32_t NoKnockBack = 0x00000800;
    static constexpr int32_t PowerArmor = 0x00001000;  // Power armor (if any) is active
    static constexpr int32_t Respawn = 0x80000000;  // Used for item respawning
};

constexpr float FRAMETIME = 0.1f;

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
constexpr int32_t AI_COMBAT_POINT = 0x00001000;
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
// edict->moveType values
struct MoveType {
    static constexpr int32_t None = 0;      // Never moves
    static constexpr int32_t Spectator = 1; // Special movetype for spectators to not go through walls
    static constexpr int32_t NoClip = 2;    // Origin and angles change with no interaction
    static constexpr int32_t Push = 3;      // No clip to world, push on box contact
    static constexpr int32_t Stop = 4;      // No clip to world, stops on box contact

    static constexpr int32_t Walk          = 10;    // Gravity
    static constexpr int32_t Step          = 11;    // Gravity, special edge handling
    static constexpr int32_t Fly           = 12;
    static constexpr int32_t Toss          = 13;    // Gravity
    static constexpr int32_t FlyMissile    = 14;    // Extra size to monsters
    static constexpr int32_t Bounce        = 15;
};


//-------------------
// Armor item description.
//-------------------
typedef struct {
    int     baseCount;
    int     maxCount;
    float   normalProtection;
    float   energyProtection;
    int     armor;
} gitem_armor_t;


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
    qboolean (*Pickup)(struct entity_s *ent, struct entity_s *other);
    void (*Use)(struct entity_s *ent, struct gitem_s *item);
    void (*Drop)(struct entity_s *ent, struct gitem_s *item);
    void (*WeaponThink)(struct entity_s *ent);

    // Sound used when being picked up.
    const char *pickupSound;

    // Actual world model used to display.
    const char *worldModel;
    
    // Specific worldmodel flags.
    int worldModelFlags;

    // Item view model. (Used for weapons, weapons are items.)
    const char        *viewModel;

    // Client side infe.
    const char  *icon;
    const char  *pickupName;    // for printing on pickup
    int countWidth;             // number of digits to display by icon

    // For ammo items this value dictates how much ammo to gain.
    // For weapon items, this value dictates how much ammo is used on a per shot basis.
    int quantity;

    // Ammo string for weapons.
    const char  *ammo;

    // IT_* flags
    int flags;          

    // Weaponmodel index.
    int weaponModelIndex;      // weapon model index (for weapons)

    // Info string & Tag.
    void *info;
    int tag;

    // An actual string of all models, sounds, and images this item will use
    const char  *precaches;     
} gitem_t;



//-------------------
// This structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//-------------------
typedef struct {
    GameClient *clients;       // [maxClients]

    // Can't store spawnpoint in level, because
    // it would get overwritten by the savegame restore
    char spawnpoint[512];    // needed for coop respawns

    // Store latched cvars here that we want to get at often
    int maxClients;
    int maxEntities;

    // Cross level triggers
    int serverflags;

    // Items
    int numberOfItems;

    // Did we autosave?
    qboolean autoSaved;
} GameLocals;


//-------------------
// Stores level locals, from current time, to which entities are sighted.
// 
// This structure is cleared as each map is entered it is read/written 
// to the 'level.sav' file for savegames
//-------------------
typedef struct {
    // Current local level frame number.
    int frameNumber;

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
        int exitIntermission; // Set to true(1) when exiting the intermission should take place.
        vec3_t origin; // Origin for intermission to take place at.
        vec3_t viewAngle; // View angle to apply for intermission.
    } intermission;

    // The actual client the AI has sight on for this current frame.
    Entity *sightClient;  // Changed once each frame for coop games

    // Entity which the AI has sight on.
    Entity *sightEntity;
    int sightEntityFrameNumber;

    // Sound entities are set to the entity that caused the AI to be triggered.
    Entity *soundEntity;            // In case of a footstep, jumping sound, etc.
    int soundEntityFrameNumber;
    Entity *sound2Entity;           // In case of a weapon action.
    int sound2EntityFrameNumber;

    // Not renaming this one, it has to go in the future.
    int pic_health;

    // Total level Monster stats.
    int totalMonsters;
    int killedMonsters;

    // The current entity that is actively being ran from SVG_RunFrame.
    Entity *currentEntity;

    // Index for the que pile of dead bodies.
    int bodyQue;

    // Ugly place for storing coop variables.
    int powerCubes; // Ugly necessity for coop
} LevelLocals;

//-------------------
// Holds entity field values that can be set from the editor, but aren't actualy present
// in Entity during gameplay
//-------------------
typedef struct {
    // world vars
    char *sky;
    float skyrotate;
    vec3_t skyaxis;
    char *nextMap;

    int lip;
    int distance;
    int height;
    const char *noise; // C++20: STRING: Added const to char *
    float pausetime;
    const char *item; // C++20: STRING: Added const to char *
    const char *gravity;    // C++20: STRING: Added const to char *

    float minyaw;
    float maxyaw;
    float minpitch;
    float maxpitch;
} TemporarySpawnFields;

//-------------------
// Contains data for keeping track of velocity based moving entities.
// (In other words, entities that aren't a: Client or AI Player.
//-------------------
typedef struct {
    // fixed data
    vec3_t startOrigin;
    vec3_t startAngles;
    vec3_t endOrigin;
    vec3_t endAngles;

    int startSoundIndex;
    int middleSoundIndex;
    int endSoundIndex;

    float acceleration;
    float speed;
    float deceleration;
    float distance;

    float wait;

    // state data
    int state;
    vec3_t dir;
    float currentSpeed;
    float moveSpeed;
    float nextSpeed;
    float remainingDistance;
    float deceleratedDistance;
    void (*OnEndFunction)(Entity *);
} PushMoveInfo;

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
extern  int sm_meat_index;
extern  int snd_fry;

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

// Extern variable, really ugly.
extern  int meansOfDeath;

// Once again, ugly.
extern  Entity g_entities[MAX_EDICTS];

//
// Small macros that are used to generate a field offset with. These are used
// in the save game system for example. Best not mess with these unless...
//
#define FOFS(x) q_offsetof(Entity, x)
#define STOFS(x) q_offsetof(TemporarySpawnFields, x)
#define LLOFS(x) q_offsetof(LevelLocals, x)
#define GLOFS(x) q_offsetof(GameLocals, x)
#define CLOFS(x) q_offsetof(GameClient, x)

// Very ugly macros, need to rid ourselves and inline func them at the least.
// Also, there should be alternatives in our utils for math lib as is.
#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0 * (random() - 0.5))

//-------------------
// Server game related cvars.
//-------------------
extern  cvar_t  *deathmatch;
extern  cvar_t  *coop;
extern  cvar_t  *dmflags;
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
extern  cvar_t  *maxClients;
extern  cvar_t  *maxspectators;

extern  cvar_t  *flood_msgs;
extern  cvar_t  *flood_persecond;
extern  cvar_t  *flood_waitdelay;

extern  cvar_t  *sv_maplist;

extern  cvar_t  *sv_flaregun;

extern  cvar_t  *cl_monsterfootsteps;

//
// Returns a pointer to the first entity, this is always the "worldspawn" entity.
// Used to be an old macro in the Q2 past: world
// 
// Since that is rather unclear, I now present to you:
//
Entity* SVG_GetWorldEntity();

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
void SVG_Command_Score_f(Entity *ent);

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
void SVG_ChangeWeapon(Entity *ent);
void SVG_SpawnItem(Entity *ent, gitem_t *item);
void SVG_ThinkWeapon(Entity *ent);
int SVG_ArmorIndex(Entity *ent);
gitem_t *SVG_GetItemByIndex(int index);
qboolean SVG_AddAmmo(Entity *ent, gitem_t *item, int count);
void SVG_TouchItem(Entity *ent, Entity *other, cplane_t *plane, csurface_t *surf);

//
// g_combat.c
//
qboolean SVG_OnSameTeam(Entity *ent1, Entity *ent2);
qboolean SVG_CanDamage(Entity *targ, Entity *inflictor);
void SVG_Damage(Entity *targ, Entity *inflictor, Entity *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockback, int dflags, int mod);
void SVG_RadiusDamage(Entity *inflictor, Entity *attacker, float damage, Entity *ignore, float radius, int mod);

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
void SVG_ThrowDebris(Entity *self, const char *modelname, float speed, const vec3_t& origin);
qboolean SVG_FireHit(Entity *self, vec3_t &aim, int damage, int kick);
void SVG_FireBullet(Entity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int mod);
void SVG_FireShotgun(Entity* self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void SVG_FireBlaster(Entity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, int effect, qboolean hyper);

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
void SVG_Player_Pain(Entity *self, Entity *other, float kick, int damage);
void SVG_Player_Die(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t& point);

//
// g_svcmds.c
//
void    SVG_ServerCommand(void);
qboolean SVG_FilterPacket(char *from);

//
// p_view.c
//
void SVG_ClientEndServerFrame(Entity *ent);

//
// p_hud.c
//


//
// g_pweapon.c
//
void SVG_PlayerNoise(Entity *who, vec3_t where, int type);

//
// g_phys.c
//
void SVG_RunEntity(Entity *ent);

//
// g_main.c
//
void SVG_SaveClientData(void);
void SVG_FetchClientData(Entity *ent);

Entity* SVG_PickTarget(char* targetName);
Entity* SVG_Find(Entity* from, int fieldofs, const char* match); // C++20: Added const to char*
Entity* SVG_FindEntitiesWithinRadius(Entity* from, vec3_t org, float rad);

void    SVG_InitEntity(Entity* e);
Entity* SVG_Spawn(void);
void    SVG_FreeEntity(Entity* e);

//
// g_chase.c
//
void SVG_UpdateChaseCam(Entity *ent);
void SVG_ChaseNext(Entity *ent);
void SVG_ChasePrev(Entity *ent);
void SVG_GetChaseTarget(Entity *ent);

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
// The ClientPersistantData struct manages data that has to stay persistent
// across level changes.
//-------------------
typedef struct {
    char userinfo[MAX_INFO_STRING];
    char netname[16];
    int hand;

    qboolean isConnected;  // A loadgame will leave valid entities that
                           // just don't have a connection yet

    // Values saved and restored from entities when changing levels
    int health;
    int maxHealth;
    int savedFlags;

    int selectedItem;
    int inventory[MAX_ITEMS];

    // Ammo capacities
    int maxBullets;
    int maxShells;
    int maxRockets;
    int maxGrenades;
    int maxCells;
    int maxSlugs;

    gitem_t *activeWeapon;
    gitem_t *lastWeapon;

    int powerCubes;    // Used for tracking the cubes in coop games
    int score;         // For calculating total unit score in coop games

    qboolean isSpectator;          // client is a isSpectator
} ClientPersistantData;

//-------------------
// The ClientRespawnData struct is used to store specific information about
// respawning. Also maintains a member variable for data that has to stay
// persistent during mapchanges/respawns in a coop game.
//-------------------
typedef struct {
    ClientPersistantData persistentCoopRespawn;   // what to set client->persistent to on a respawn
    int enterFrame;         // level.frameNumber the client entered the game
    int score;              // frags, etc
    vec3_t commandViewAngles;         // angles sent over in the last command

    qboolean isSpectator;          // client is a isSpectator
} ClientRespawnData;


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
    int ping;

    // private to game
    ClientPersistantData persistent;
    ClientRespawnData respawn;

    qboolean showScores;         // set layout stat
    qboolean showInventory;      // set layout stat
    qboolean showHelpIcon;

    int ammoIndex;

    int buttons;
    int oldButtons;
    int latchedButtons;     // These are used for one time push events.

    qboolean weaponThunk;

    gitem_t *newWeapon;

    // sum up damage over an entire frame, so
    // shotgun blasts give a single big kick
    struct {
        int armor;       // damage absorbed by armor
        int powerArmor;      // damage absorbed by power armor
        int blood;       // damage taken out of health
        int knockBack;   // impact damage
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
    int oldWaterLevel;

    // For weapon raising
    int machinegunShots;

    // animation vars
    struct {
        int         endFrame;
        int         priorityAnimation;
        qboolean    isDucking;
        qboolean    isRunning;
    } animation;

    // Weapon Sound.
    int weaponSound;

    // Pick up message time.
    float pickupMessageTime;

    // Flood protection struct.
    struct {
        float lockTill;     // locked from talking
        float when[10];     // when messages were said
        int whenHead;     // head pointer for when said
    } flood;

    // Client can respawn when time > this
    float respawnTime;

    // The (client)player we are chasing
    Entity *chaseTarget;

    // Do we need to update chase info?
    qboolean updateChase;
};

//-------------------
// Predeclarations.
//-------------------
class SVGBaseEntity;

//-------------------
// entity_s, the server side entity structure. If you know what an entity is,
// then you know what this is.
// 
// The actual SVGBaseEntity class is a member. It is where the magic happens.
// Entities can be linked to their "classname", this will in turn make sure that
// the proper inheritance entity is allocated.
//-------------------
struct entity_s {
    // Actual entity state member. Contains all data that is actually networked.
    EntityState  state;

    // NULL if not a player the server expects the first part of gclient_s to
    // be a PlayerState but the rest of it is opaque
    struct gclient_s *client;       

    // An entity is in no use, in case it complies to the INUSE macro.
    qboolean inUse;
    int linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t area; // Linked to a division node or leaf

    // If numClusters is -1, use headNodew instead.
    int numClusters;       // if -1, use headNode instead
    int clusterNumbers[MAX_ENT_CLUSTERS];
    // Only use this instead of numClusters if numClusters == -1
    int headNode;           
    int areaNumber;
    int areaNumber2;

    //================================
    int serverFlags;
    vec3_t mins, maxs;
    vec3_t absMin, absMax, size;
    uint32_t solid;
    int clipMask;
    Entity *owner;


    // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
    // EXPECTS THE FIELDS IN THAT ORDER!

    //================================
    SVGBaseEntity* classEntity;

    int moveType;
    int flags;

    const char *model;       // C++20: STRING: Added const to char*
    float freeTime;     // sv.time when the object was freed

    //
    // only used locally in game, not by server
    //
    const char *message;     // C++20: STRING: Added const to char *
    const char *className;   // C++20: STRING: Made const.
    int spawnFlags;

    float timeStamp;

    float angle;          // set in qe3, -1 = up, -2 = down
    char *target;
    const char *targetName;
    char *killTarget;
    char *team;
    char *pathTarget;
    char *deathTarget;
    char *combatTarget;
    Entity *targetEntityPtr;

    // For moving objects(plats, etc)
    float speed;
    float acceleration;
    float deceleration;
    vec3_t moveDirection;
    vec3_t position1, position2;

    // Regular entity velocity, gravity, mass.
    vec3_t velocity;
    vec3_t angularVelocity;
    int mass;
    float airFinished;
    float gravity;        // per entity gravity multiplier (1.0 is normal)
                                // use for lowgrav artifact, flares

    Entity *goalEntityPtr;
    Entity *moveTargetPtr;
    float yawSpeed;
    float idealYaw;

    float nextThink;
    void (*PreThink)(Entity *ent);
    void (*Think)(Entity *self);
    void (*Blocked)(Entity *self, Entity *other);         // move to moveinfo?
    void (*Touch)(Entity *self, Entity *other, cplane_t *plane, csurface_t *surf);
    void (*Use)(Entity *self, Entity *other, Entity *activator);
    void (*Pain)(Entity *self, Entity *other, float kick, int damage);
    void (*Die)(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t &point);

    float debounceTouchTime;        // are all these legit?  do we need more/less of them?
    float debouncePainTime;
    float debounceDamageTime;
    float debounceSoundTime;    // move to clientInfo
    float lastMoveTime;

    int health;
    int maxHealth;
    int gibHealth;      // Health level required in order for an edict to gib.
    int deadFlag;
    qboolean showHostile;

    float powerArmorTime;

    const char *map;           // target_changelevel // C++20: STRING: Added const to char *

    int viewHeight;     // height above origin where eyesight is determined
    int takeDamage;
    int damage;
    int radiusDamage;
    float damageRadius;
    int sounds;         // make this a spawntemp var?
    int count;

    // Chain, enemy, old enemy, and activator entity pointers.
    Entity *chain;
    Entity *enemy;
    Entity *oldEnemyPtr;
    Entity *activator;
    
    // Ground pointers.
    Entity *groundEntityPtr;
    int groundEntityLinkCount;
    
    Entity *teamChainPtr;
    Entity *teamMasterPtr;

    Entity *myNoisePtr;       // can go in client only
    Entity *myNoise2Ptr;

    int noiseIndex;
    int noiseIndex2;
    float volume;
    float attenuation;

    // timing variables
    float wait;
    float delay;          // before firing targets
    float random;

    float teleportTime;

    int waterType;
    int waterLevel;

    vec3_t moveOrigin;
    vec3_t moveAngles;

    // move this to clientInfo?
    int lightLevel;

    int style;          // also used as areaportal number

    // Custom lightstyle.
    char *customLightStyle;

    gitem_t *item;          // for bonus items

    // common data blocks
    PushMoveInfo moveInfo;
};

#endif