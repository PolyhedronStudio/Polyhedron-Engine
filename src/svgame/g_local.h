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


//deadFlag
constexpr int32_t  DEAD_NO = 0;
constexpr int32_t  DEAD_DYING = 1;
constexpr int32_t  DEAD_DEAD = 2;
constexpr int32_t  DEAD_RESPAWNABLE = 3;

//range
constexpr int32_t  RANGE_MELEE = 0;
constexpr int32_t  RANGE_NEAR = 1;
constexpr int32_t  RANGE_MID = 2;
constexpr int32_t  RANGE_FAR = 3;

//gib types
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

// armor types
constexpr int32_t ARMOR_NONE = 0;
constexpr int32_t ARMOR_JACKET = 1;
constexpr int32_t ARMOR_COMBAT = 2;
constexpr int32_t ARMOR_BODY = 3;
constexpr int32_t ARMOR_SHARD = 4;

// power armor types
constexpr int32_t POWER_ARMOR_NONE = 0;

// handedness values
constexpr int32_t RIGHT_HANDED = 0;
constexpr int32_t LEFT_HANDED = 1;
constexpr int32_t CENTER_HANDED = 2;


// game.serverflags values
constexpr int32_t SFL_CROSS_TRIGGER_1 = 0x00000001;
constexpr int32_t SFL_CROSS_TRIGGER_2 = 0x00000002;
constexpr int32_t SFL_CROSS_TRIGGER_3 = 0x00000004;
constexpr int32_t SFL_CROSS_TRIGGER_4 = 0x00000008;
constexpr int32_t SFL_CROSS_TRIGGER_5 = 0x00000010;
constexpr int32_t SFL_CROSS_TRIGGER_6 = 0x00000020;
constexpr int32_t SFL_CROSS_TRIGGER_7 = 0x00000040;
constexpr int32_t SFL_CROSS_TRIGGER_8 = 0x00000080;
constexpr int32_t SFL_CROSS_TRIGGER_MASK = 0x000000ff;


// noise types for PlayerNoise
constexpr int32_t PNOISE_SELF = 0;
constexpr int32_t PNOISE_WEAPON = 1;
constexpr int32_t PNOISE_IMPACT = 2;


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



typedef struct {
    int     baseCount;
    int     maxCount;
    float   normalProtection;
    float   energyProtection;
    int     armor;
} gitem_armor_t;


// gitem_t->flags
constexpr int32_t IT_WEAPON = 1;       // use makes active weapon
constexpr int32_t IT_AMMO = 2;
constexpr int32_t IT_ARMOR = 4;
constexpr int32_t IT_STAY_COOP = 8;
constexpr int32_t IT_KEY = 16;
constexpr int32_t IT_POWERUP = 32;

// gitem_t->weaponModel for weapons indicates model index
constexpr int32_t WEAP_BLASTER = 1;
constexpr int32_t WEAP_MACHINEGUN = 2;
constexpr int32_t WEAP_SHOTGUN = 3;
constexpr int32_t WEAP_SUPERSHOTGUN = 4;

// C++20: STRING: Added const to the chars.
typedef struct gitem_s {
    const char *className; // spawning name
    qboolean (*Pickup)(struct entity_s *ent, struct entity_s *other);
    void (*Use)(struct entity_s *ent, struct gitem_s *item);
    void (*Drop)(struct entity_s *ent, struct gitem_s *item);
    void (*WeaponThink)(struct entity_s *ent);
    const char *pickupSound;
    const char *worldModel;
    int worldModelFlags;
    const char        *viewModel;

    // client side info
    const char  *icon;
    const char  *pickupName;   // for printing on pickup
    int countWidth;    // number of digits to display by icon

    int quantity;       // for ammo how much, for weapons how much is used per shot
    const char  *ammo;          // for weapons
    int flags;          // IT_* flags

    int weaponModel;      // weapon model index (for weapons)

    void *info;
    int tag;

    const char  *precaches;     // string of all models, sounds, and images this item will use
} gitem_t;



//
// This structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
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

    qboolean autosaved;
} GameLocals;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct {
    int frameNumber;
    float time;

    char level_name[MAX_QPATH];  // the descriptive name (Outer Base, etc)
    char mapname[MAX_QPATH];     // the server name (base1, etc)
    char nextmap[MAX_QPATH];     // go here when fraglimit is hit

    // intermission state
    float intermissiontime;       // time the intermission was started
    const char *changemap; // C++20: STRING: Added const to char*
    int exitintermission;
    vec3_t intermissionOrigin;
    vec3_t intermissionAngle;

    Entity *sightClient;  // changed once each frame for coop games

    Entity *sightEntity;
    int sightEntityFrameNumber;
    Entity *soundEntity;
    int soundEntityFrameNumber;
    Entity *sound2Entity;
    int sound2EntityFrameNumber;

    // Not renaming this one, it has to go in the future.
    int pic_health;

    // Total level Monster stats.
    int totalMonsters;
    int killedMonsters;

    // The current entity that is actively being ran from G_RunFrame.
    Entity *currentEntity;

    // Index for the que pile of dead bodies.
    int bodyQue;

    // Ugly place for storing coop variables.
    int powerCubes; // Ugly necessity for coop
} LevelLocals;


// TemporarySpawnFields is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in Entity during gameplay
typedef struct {
    // world vars
    char *sky;
    float skyrotate;
    vec3_t skyaxis;
    char *nextmap;

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
} moveinfo_t;


typedef struct {
    void    (*aifunc)(Entity *self, float dist);
    float   dist;
    void    (*thinkfunc)(Entity *self);
} mframe_t;

typedef struct {
    int         firstframe;
    int         lastFrame;
    mframe_t    *frame;
    void        (*OnEndFunction)(Entity *self);
} mmove_t;

typedef struct {
    mmove_t     *currentmove;
    int         aiflags;
    int         nextframe;
    float       scale;

    void        (*stand)(Entity *self);
    void        (*idle)(Entity *self);
    void        (*search)(Entity *self);
    void        (*walk)(Entity *self);
    void        (*run)(Entity *self);
    void        (*dodge)(Entity *self, Entity *other, float eta);
    void        (*attack)(Entity *self);
    void        (*melee)(Entity *self);
    void        (*sight)(Entity *self, Entity *other);
    qboolean    (*checkattack)(Entity *self);

    float       pausetime;
    float       attack_finished;

    vec3_t      saved_goal;
    float       search_time;
    float       trail_time;
    vec3_t      last_sighting;
    int         attack_state;
    int         lefty;
    float       idle_time;
    int         linkCount;

    int         power_armor_type;
    int         power_armor_power;
} monsterinfo_t;

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

//
// Means of death
//
// Used for registring means of death types so they can be displayed 
// accordingly in the obituaries.
//
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
extern  Entity         *g_entities;

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

//
// Extern cvars, we want to access these all over ofc.
//
extern  cvar_t  *maxEntities;
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
extern  cvar_t  *nomonsters;

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
Entity* G_GetWorldEntity();

//
// Spawn flags set by editors for items.
//
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
void Cmd_Score_f(Entity *ent);

//
// g_items.c
//
void PrecacheItem(gitem_t *it);
void InitItems(void);
void SetItemNames(void);
gitem_t *FindItem(const char *pickup_name);
gitem_t *FindItemByClassname(const char *className);
#define ITEM_INDEX(x) ((x)-itemlist)
Entity *Drop_Item(Entity *ent, gitem_t *item);
void SetRespawn(Entity *ent, float delay);
void ChangeWeapon(Entity *ent);
void SpawnItem(Entity *ent, gitem_t *item);
void Think_Weapon(Entity *ent);
int ArmorIndex(Entity *ent);
gitem_t *GetItemByIndex(int index);
qboolean Add_Ammo(Entity *ent, gitem_t *item, int count);
void Touch_Item(Entity *ent, Entity *other, cplane_t *plane, csurface_t *surf);

//
// g_combat.c
//
qboolean OnSameTeam(Entity *ent1, Entity *ent2);
qboolean CanDamage(Entity *targ, Entity *inflictor);
void T_Damage(Entity *targ, Entity *inflictor, Entity *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage(Entity *inflictor, Entity *attacker, float damage, Entity *ignore, float radius, int mod);

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
// g_monster.c
//
void monster_fire_bullet(Entity *self, const vec3_t &start, const vec3_t &dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_blaster(Entity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, int flashtype, int effect);
void M_droptofloor(Entity *ent);
void monster_think(Entity *self);
void walkmonster_start(Entity *self);
void swimmonster_start(Entity *self);
void flymonster_start(Entity *self);
void AttackFinished(Entity *self, float time);
void monster_death_use(Entity *self);
void M_CatagorizePosition(Entity *ent);
qboolean M_CheckAttack(Entity *self);
void M_FlyCheck(Entity *self);
void M_CheckGround(Entity *ent);

//
// g_ai.c
//
void AI_SetSightClient(void);

void ai_stand(Entity *self, float dist);
void ai_move(Entity *self, float dist);
void ai_walk(Entity *self, float dist);
void ai_turn(Entity *self, float dist);
void ai_run(Entity *self, float dist);
void ai_charge(Entity *self, float dist);
int range(Entity *self, Entity *other);

void FoundTarget(Entity *self);
qboolean infront(Entity *self, Entity *other);
qboolean visible(Entity *self, Entity *other);
qboolean FacingIdeal(Entity *self);

//
// g_weapon.c
//
void ThrowDebris(Entity *self, const char *modelname, float speed, const vec3_t& origin);
qboolean fire_hit(Entity *self, vec3_t &aim, int damage, int kick);
void fire_bullet(Entity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun(Entity* self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster(Entity *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, int effect, qboolean hyper);

//
// g_ptrail.c
//
void PlayerTrail_Init(void);
void PlayerTrail_Add(vec3_t spot);
void PlayerTrail_New(vec3_t spot);
Entity *PlayerTrail_PickFirst(Entity *self);
Entity *PlayerTrail_PickNext(Entity *self);
Entity *PlayerTrail_LastSpot(void);

//
// g_player.c
//
void Player_Pain(Entity *self, Entity *other, float kick, int damage);
void Player_Die(Entity *self, Entity *inflictor, Entity *attacker, int damage, const vec3_t& point);

//
// g_svcmds.c
//
void    ServerCommand(void);
qboolean SV_FilterPacket(char *from);

//
// p_view.c
//
void ClientEndServerFrame(Entity *ent);

//
// p_hud.c
//


//
// g_pweapon.c
//
void PlayerNoise(Entity *who, vec3_t where, int type);

//
// m_move.c
//
qboolean M_CheckBottom(Entity *ent);
qboolean M_walkmove(Entity *ent, float yaw, float dist);
void M_MoveToGoal(Entity *ent, float dist);
void M_ChangeYaw(Entity *ent);

//
// g_phys.c
//
void G_RunEntity(Entity *ent);

//
// g_main.c
//
void SaveClientData(void);
void FetchClientEntData(Entity *ent);

Entity* G_PickTarget(char* targetName);
Entity* G_Find(Entity* from, int fieldofs, const char* match); // C++20: Added const to char*
Entity* G_FindEntitiesWithinRadius(Entity* from, vec3_t org, float rad);

void    G_InitEntity(Entity* e);
Entity* G_Spawn(void);
void    G_FreeEntity(Entity* e);

//
// g_chase.c
//
void UpdateChaseCam(Entity *ent);
void ChaseNext(Entity *ent);
void ChasePrev(Entity *ent);
void GetChaseTarget(Entity *ent);

//============================================================================

// client_t->animation.priorityAnimation
struct PlayerAnimation {
    static constexpr int32_t Basic = 0;       // Stand / Run
    static constexpr int32_t Wave = 1;
    static constexpr int32_t Jump = 2;
    static constexpr int32_t Pain = 3;
    static constexpr int32_t Attack = 4;
    static constexpr int32_t Death = 5;
    static constexpr int32_t Reverse = 6;
};

// Client data that stays across multiple level loads
typedef struct {
    char userinfo[MAX_INFO_STRING];
    char netname[16];
    int hand;

    qboolean connected;  // A loadgame will leave valid entities that
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

// client data that stays across deathmatch respawns
typedef struct {
    ClientPersistantData persistentCoopRespawn;   // what to set client->persistent to on a respawn
    int enterFrame;         // level.frameNumber the client entered the game
    int score;              // frags, etc
    vec3_t commandViewAngles;         // angles sent over in the last command

    qboolean isSpectator;          // client is a isSpectator
} ClientRespawnData;

// this structure is cleared on each PutClientInServer(),
// except for 'client->persistent'
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


struct entity_s {
    EntityState  state;
    struct gclient_s *client;    // NULL if not a player
                                    // the server expects the first part
                                    // of gclient_s to be a PlayerState
                                    // but the rest of it is opaque

    qboolean inUse;
    int linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t area;               // linked to a division node or leaf

    int numClusters;       // if -1, use headNode instead
    int clusterNumbers[MAX_ENT_CLUSTERS];
    int headNode;           // unused if numClusters != -1
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
    moveinfo_t moveInfo;
    monsterinfo_t monsterInfo;
};

#endif