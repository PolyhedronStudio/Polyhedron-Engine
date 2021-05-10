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
// short, server-visible gclient_t and entity_t structures,
// because we define the full size ones in this file
#define GAME_INCLUDE
#include "shared/svgame.h"
#include "sharedgame/sharedgame.h" // Include SG Base.
#include "sharedgame/protocol.h"

// the "gameversion" client command will print this plus compile date
#define GAMEVERSION "basenac"

// protocol bytes that can be directly added to messages
//#define	svg_muzzleflash		1
//#define	svg_muzzleflash2	2
//#define	svg_temp_entity		3
//#define	svg_layout			4
//#define	svg_inventory		5
//#define	svc_stufftext		11

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
    int     base_count;
    int     max_count;
    float   normal_protection;
    float   energy_protection;
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
constexpr int32_t WEAP_FLAREGUN = 5;

// C++20: STRING: Added const to the chars.
typedef struct gitem_s {
    const char        *classname; // spawning name
    qboolean    (*Pickup)(struct entity_s *ent, struct entity_s *other);
    void        (*Use)(struct entity_s *ent, struct gitem_s *item);
    void        (*Drop)(struct entity_s *ent, struct gitem_s *item);
    void        (*WeaponThink)(struct entity_s *ent);
    const char        *pickupSound;
    const char        *worldModel;
    int         worldModelFlags;
    const char        *viewModel;

    // client side info
    const char  *icon;
    const char  *pickupName;   // for printing on pickup
    int         countWidth;    // number of digits to display by icon

    int         quantity;       // for ammo how much, for weapons how much is used per shot
    const char  *ammo;          // for weapons
    int         flags;          // IT_* flags

    int         weaponModel;      // weapon model index (for weapons)

    void        *info;
    int         tag;

    const char  *precaches;     // string of all models, sounds, and images this item will use
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct {
    gclient_t   *clients;       // [maxClients]

    // can't store spawnpoint in level, because
    // it would get overwritten by the savegame restore
    char        spawnpoint[512];    // needed for coop respawns

    // store latched cvars here that we want to get at often
    int         maxClients;
    int         maxentities;

    // cross level triggers
    int         serverflags;

    // items
    int         num_items;

    qboolean    autosaved;
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct {
    int         frameNumber;
    float       time;

    char        level_name[MAX_QPATH];  // the descriptive name (Outer Base, etc)
    char        mapname[MAX_QPATH];     // the server name (base1, etc)
    char        nextmap[MAX_QPATH];     // go here when fraglimit is hit

    // intermission state
    float       intermissiontime;       // time the intermission was started
    const char        *changemap; // C++20: STRING: Added const to char*
    int         exitintermission;
    vec3_t      intermission_origin;
    vec3_t      intermission_angle;

    entity_t     *sight_client;  // changed once each frame for coop games

    entity_t     *sight_entity;
    int         sight_entity_framenum;
    entity_t     *sound_entity;
    int         sound_entity_framenum;
    entity_t     *sound2_entity;
    int         sound2_entity_framenum;

    int         pic_health;

    int         total_goals;
    int         found_goals;

    int         total_monsters;
    int         killed_monsters;

    entity_t     *current_entity;    // entity running from G_RunFrame
    int         body_que;           // dead bodies

    int         power_cubes;        // ugly necessity for coop
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in entity_t during gameplay
typedef struct {
    // world vars
    char        *sky;
    float       skyrotate;
    vec3_t      skyaxis;
    char        *nextmap;

    int         lip;
    int         distance;
    int         height;
    const char        *noise; // C++20: STRING: Added const to char *
    float       pausetime;
    const char        *item; // C++20: STRING: Added const to char *
    const char        *gravity;    // C++20: STRING: Added const to char *

    float       minyaw;
    float       maxyaw;
    float       minpitch;
    float       maxpitch;
} spawn_temp_t;


typedef struct {
    // fixed data
    vec3_t      start_origin;
    vec3_t      start_angles;
    vec3_t      end_origin;
    vec3_t      end_angles;

    int         sound_start;
    int         sound_middle;
    int         sound_end;

    float       accel;
    float       speed;
    float       decel;
    float       distance;

    float       wait;

    // state data
    int         state;
    vec3_t      dir;
    float       current_speed;
    float       move_speed;
    float       next_speed;
    float       remaining_distance;
    float       decel_distance;
    void        (*endfunc)(entity_t *);
} moveinfo_t;


typedef struct {
    void    (*aifunc)(entity_t *self, float dist);
    float   dist;
    void    (*thinkfunc)(entity_t *self);
} mframe_t;

typedef struct {
    int         firstframe;
    int         lastFrame;
    mframe_t    *frame;
    void        (*endfunc)(entity_t *self);
} mmove_t;

typedef struct {
    mmove_t     *currentmove;
    int         aiflags;
    int         nextframe;
    float       scale;

    void        (*stand)(entity_t *self);
    void        (*idle)(entity_t *self);
    void        (*search)(entity_t *self);
    void        (*walk)(entity_t *self);
    void        (*run)(entity_t *self);
    void        (*dodge)(entity_t *self, entity_t *other, float eta);
    void        (*attack)(entity_t *self);
    void        (*melee)(entity_t *self);
    void        (*sight)(entity_t *self, entity_t *other);
    qboolean    (*checkattack)(entity_t *self);

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



extern  game_locals_t   game;
extern  level_locals_t  level;
extern  svgame_import_t gi;         // CLEANUP: These were game_import_t and game_export_T
extern  svgame_export_t globals;    // CLEANUP: These were game_import_t and game_export_T
extern  spawn_temp_t    st;

extern  int sm_meat_index;
extern  int snd_fry;

//extern  int jacket_armor_index;
//extern  int combat_armor_index;
//extern  int body_armor_index;


// means of death
#define MOD_UNKNOWN         0
#define MOD_BLASTER         1
#define MOD_SHOTGUN         2
#define MOD_SSHOTGUN        3
#define MOD_MACHINEGUN      4
#define MOD_CHAINGUN        5
#define MOD_GRENADE         6
#define MOD_G_SPLASH        7
#define MOD_ROCKET          8
#define MOD_R_SPLASH        9
#define MOD_HYPERBLASTER    10
#define MOD_RAILGUN         11
#define MOD_BFG_LASER       12
#define MOD_BFG_BLAST       13
#define MOD_BFG_EFFECT      14
#define MOD_HANDGRENADE     15
#define MOD_HG_SPLASH       16
#define MOD_WATER           17
#define MOD_SLIME           18
#define MOD_LAVA            19
#define MOD_CRUSH           20
#define MOD_TELEFRAG        21
#define MOD_FALLING         22
#define MOD_SUICIDE         23
#define MOD_HELD_GRENADE    24
#define MOD_EXPLOSIVE       25
#define MOD_BARREL          26
#define MOD_BOMB            27
#define MOD_EXIT            28
#define MOD_SPLASH          29
#define MOD_TRIGGER_HURT    31
#define MOD_HIT             32
#define MOD_TARGET_BLASTER  33
#define MOD_FRIENDLY_FIRE   0x8000000

extern  int meansOfDeath;


extern  entity_t         *g_edicts;

#define FOFS(x) q_offsetof(entity_t, x)
#define STOFS(x) q_offsetof(spawn_temp_t, x)
#define LLOFS(x) q_offsetof(level_locals_t, x)
#define GLOFS(x) q_offsetof(game_locals_t, x)
#define CLOFS(x) q_offsetof(gclient_t, x)

#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0 * (random() - 0.5))

extern  cvar_t  *maxentities;
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

#define world   (&g_edicts[0])

// item spawnFlags
#define ITEM_TRIGGER_SPAWN      0x00000001
#define ITEM_NO_TOUCH           0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM            0x00010000
#define DROPPED_PLAYER_ITEM     0x00020000
#define ITEM_TARGETS_USED       0x00040000

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
void Cmd_Score_f(entity_t *ent);

//
// g_items.c
//
void PrecacheItem(gitem_t *it);
void InitItems(void);
void SetItemNames(void);
gitem_t *FindItem(const char *pickup_name);
gitem_t *FindItemByClassname(const char *classname);
#define ITEM_INDEX(x) ((x)-itemlist)
entity_t *Drop_Item(entity_t *ent, gitem_t *item);
void SetRespawn(entity_t *ent, float delay);
void ChangeWeapon(entity_t *ent);
void SpawnItem(entity_t *ent, gitem_t *item);
void Think_Weapon(entity_t *ent);
int ArmorIndex(entity_t *ent);
gitem_t *GetItemByIndex(int index);
qboolean Add_Ammo(entity_t *ent, gitem_t *item, int count);
void Touch_Item(entity_t *ent, entity_t *other, cplane_t *plane, csurface_t *surf);

//
// g_combat.c
//
qboolean OnSameTeam(entity_t *ent1, entity_t *ent2);
qboolean CanDamage(entity_t *targ, entity_t *inflictor);
void T_Damage(entity_t *targ, entity_t *inflictor, entity_t *attacker, const vec3_t &dmgDir, const vec3_t &point, const vec3_t &normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage(entity_t *inflictor, entity_t *attacker, float damage, entity_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS           0x00000001  // damage was indirect
#define DAMAGE_NO_ARMOR         0x00000002  // armour does not protect from this damage
#define DAMAGE_ENERGY           0x00000004  // damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK     0x00000008  // do not affect velocity, just view angles
#define DAMAGE_BULLET           0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION    0x00000020  // armor, shields, invulnerability, and godmode have no effect

constexpr int32_t DEFAULT_BULLET_HSPREAD = 300;
constexpr int32_t DEFAULT_BULLET_VSPREAD = 500;
constexpr int32_t DEFAULT_SHOTGUN_HSPREAD = 1000;
constexpr int32_t DEFAULT_SHOTGUN_VSPREAD = 500;
constexpr int32_t DEFAULT_DEATHMATCH_SHOTGUN_COUNT = 12;
constexpr int32_t DEFAULT_SHOTGUN_COUNT = 12;
constexpr int32_t DEFAULT_SSHOTGUN_COUNT = 20;

//
// g_monster.c
//
void monster_fire_bullet(entity_t *self, const vec3_t &start, const vec3_t &dir, int damage, int kick, int hspread, int vspread, int flashtype);
void monster_fire_blaster(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, int flashtype, int effect);
void M_droptofloor(entity_t *ent);
void monster_think(entity_t *self);
void walkmonster_start(entity_t *self);
void swimmonster_start(entity_t *self);
void flymonster_start(entity_t *self);
void AttackFinished(entity_t *self, float time);
void monster_death_use(entity_t *self);
void M_CatagorizePosition(entity_t *ent);
qboolean M_CheckAttack(entity_t *self);
void M_FlyCheck(entity_t *self);
void M_CheckGround(entity_t *ent);

//
// g_ai.c
//
void AI_SetSightClient(void);

void ai_stand(entity_t *self, float dist);
void ai_move(entity_t *self, float dist);
void ai_walk(entity_t *self, float dist);
void ai_turn(entity_t *self, float dist);
void ai_run(entity_t *self, float dist);
void ai_charge(entity_t *self, float dist);
int range(entity_t *self, entity_t *other);

void FoundTarget(entity_t *self);
qboolean infront(entity_t *self, entity_t *other);
qboolean visible(entity_t *self, entity_t *other);
qboolean FacingIdeal(entity_t *self);

//
// g_weapon.c
//
void ThrowDebris(entity_t *self, const char *modelname, float speed, const vec3_t& origin);
qboolean fire_hit(entity_t *self, vec3_t &aim, int damage, int kick);
void fire_bullet(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun(entity_t* self, const vec3_t& start, const vec3_t& aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster(entity_t *self, const vec3_t& start, const vec3_t& aimdir, int damage, int speed, int effect, qboolean hyper);

//
// g_ptrail.c
//
void PlayerTrail_Init(void);
void PlayerTrail_Add(vec3_t spot);
void PlayerTrail_New(vec3_t spot);
entity_t *PlayerTrail_PickFirst(entity_t *self);
entity_t *PlayerTrail_PickNext(entity_t *self);
entity_t *PlayerTrail_LastSpot(void);

//
// g_player.c
//
void Player_Pain(entity_t *self, entity_t *other, float kick, int damage);
void Player_Die(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t& point);

//
// g_svcmds.c
//
void    ServerCommand(void);
qboolean SV_FilterPacket(char *from);

//
// p_view.c
//
void ClientEndServerFrame(entity_t *ent);

//
// p_hud.c
//


//
// g_pweapon.c
//
void PlayerNoise(entity_t *who, vec3_t where, int type);

//
// m_move.c
//
qboolean M_CheckBottom(entity_t *ent);
qboolean M_walkmove(entity_t *ent, float yaw, float dist);
void M_MoveToGoal(entity_t *ent, float dist);
void M_ChangeYaw(entity_t *ent);

//
// g_phys.c
//
void G_RunEntity(entity_t *ent);

//
// g_main.c
//
void SaveClientData(void);
void FetchClientEntData(entity_t *ent);

entity_t* G_PickTarget(char* targetName);
entity_t* G_Find(entity_t* from, int fieldofs, const char* match); // C++20: Added const to char*
entity_t* G_FindEntitiesWithinRadius(entity_t* from, vec3_t org, float rad);

void    G_InitEntity(entity_t* e);
entity_t* G_Spawn(void);
void    G_FreeEntity(entity_t* e);

//
// g_chase.c
//
void UpdateChaseCam(entity_t *ent);
void ChaseNext(entity_t *ent);
void ChasePrev(entity_t *ent);
void GetChaseTarget(entity_t *ent);

//============================================================================

// client_t->animation.priorityAnimation
#define ANIM_BASIC      0       // stand / run
#define ANIM_WAVE       1
#define ANIM_JUMP       2
#define ANIM_PAIN       3
#define ANIM_ATTACK     4
#define ANIM_DEATH      5
#define ANIM_REVERSE    6


// client data that stays across multiple level loads
typedef struct {
    char        userinfo[MAX_INFO_STRING];
    char        netname[16];
    int         hand;

    qboolean    connected;  // a loadgame will leave valid entities that
                            // just don't have a connection yet

    // values saved and restored from edicts when changing levels
    int         health;
    int         maxHealth;
    int         savedFlags;

    int         selected_item;
    int         inventory[MAX_ITEMS];

    // ammo capacities
    int         max_bullets;
    int         max_shells;
    int         max_rockets;
    int         max_grenades;
    int         max_cells;
    int         max_slugs;

    gitem_t     *weapon;
    gitem_t     *lastweapon;

    int         power_cubes;    // used for tracking the cubes in coop games
    int         score;          // for calculating total unit score in coop games

    qboolean    spectator;          // client is a spectator
} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct {
    client_persistant_t coop_respawn;   // what to set client->persistent to on a respawn
    int         enterframe;         // level.frameNumber the client entered the game
    int         score;              // frags, etc
    vec3_t      commandViewAngles;         // angles sent over in the last command

    qboolean    spectator;          // client is a spectator
} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->persistent'
struct gclient_s {
    // known to server
    PlayerState  playerState;             // communicated by server to clients
    int             ping;

    // private to game
    client_persistant_t persistent;
    client_respawn_t    respawn;

    qboolean    showScores;         // set layout stat
    qboolean    showInventory;      // set layout stat
    qboolean    showHelpIcon;

    int         ammoIndex;

    int         buttons;
    int         oldButtons;
    int         latchedButtons;     // These are used for one time push events.

    qboolean    weaponThunk;

    gitem_t     *newweapon;

    // sum up damage over an entire frame, so
    // shotgun blasts give a single big kick
    struct {
        int         armor;       // damage absorbed by armor
        int         powerArmor;      // damage absorbed by power armor
        int         blood;       // damage taken out of health
        int         knockBack;   // impact damage
        vec3_t      from;        // origin for vector calculation
    } damages;

    float       killerYaw;         // when dead, look at killer

    int32_t     weaponState;

    vec3_t      kickAngles;    // weapon kicks
    vec3_t      kickOrigin;
    // View damage kicks.
    struct {
        float roll;
        float pitch;
        float time;
    } viewDamage;

    float       fallTime, fallValue;      // for view drop on fall
    float       damageAlpha;
    float       bonusAlpha;
    vec3_t      damageBlend;
    vec3_t      aimAngles;            // aiming direction
    float       bobtime;            // so off-ground doesn't change it

    // Old view angles and velocity.
    vec3_t      oldViewAngles;
    vec3_t      oldVelocity;

    float       nextDrownTime;
    int         oldWaterLevel;

    // For weapon raising
    int         machinegunShots;

    // animation vars
    struct {
        //int         animation.endFrame;
        //int         animation.priorityAnimation;
        //qboolean    anim_duck;
        //qboolean    anim_run;
        int         endFrame;
        int         priorityAnimation;
        qboolean    isDucking;
        qboolean    isRunning;
    } animation;

    // Weapon Sound.
    int         weaponSound;

    // Pick up message time.
    float       pickupMessageTime;

    // Flood protection struct.
    struct {
        float   lockTill;     // locked from talking
        float   when[10];     // when messages were said
        int     whenHead;     // head pointer for when said
    } flood;

    // Client can respawn when time > this
    float       respawnTime;

    // The (client)player we are chasing
    entity_t     *chaseTarget;

    // Do we need to update chase info?
    qboolean    updateChase;
};


struct entity_s {
    EntityState  state;
    struct gclient_s    *client;    // NULL if not a player
                                    // the server expects the first part
                                    // of gclient_s to be a PlayerState
                                    // but the rest of it is opaque

    qboolean    inUse;
    int         linkCount;

    // FIXME: move these fields to a server private sv_entity_t
    list_t      area;               // linked to a division node or leaf

    int         numClusters;       // if -1, use headNode instead
    int         clusterNumbers[MAX_ENT_CLUSTERS];
    int         headNode;           // unused if numClusters != -1
    int         areaNumber, areaNumber2;

    //================================

    int         serverFlags;
    vec3_t      mins, maxs;
    vec3_t      absMin, absMax, size;
    uint32_t    solid;
    int         clipMask;
    entity_t     *owner;


    // DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
    // EXPECTS THE FIELDS IN THAT ORDER!

    //================================
    int         moveType;
    int         flags;

    const char  *model;       // C++20: STRING: Added const to char*
    float       freeTime;     // sv.time when the object was freed

    //
    // only used locally in game, not by server
    //
    const char        *message;     // C++20: STRING: Added const to char *
    const char        *classname;   // C++20: STRING: Made const.
    int         spawnFlags;

    float       timestamp;

    float       angle;          // set in qe3, -1 = up, -2 = down
    char        *target;
    const char        *targetName;
    char        *killTarget;
    char        *team;
    char        *pathTarget;
    char        *deathTarget;
    char        *combatTarget;
    entity_t     *targetEntityPtr;

    float       speed, accel, decel;
    vec3_t      moveDirection;
    vec3_t      pos1, pos2;

    vec3_t      velocity;
    vec3_t      avelocity;
    int         mass;
    float       air_finished;
    float       gravity;        // per entity gravity multiplier (1.0 is normal)
                                // use for lowgrav artifact, flares

    entity_t     *goalEntityPtr;
    entity_t     *moveTargetPtr;
    float       yawSpeed;
    float       idealYaw;

    float       nextThink;
    void        (*PreThink)(entity_t *ent);
    void        (*Think)(entity_t *self);
    void        (*Blocked)(entity_t *self, entity_t *other);         // move to moveinfo?
    void        (*Touch)(entity_t *self, entity_t *other, cplane_t *plane, csurface_t *surf);
    void        (*Use)(entity_t *self, entity_t *other, entity_t *activator);
    void        (*Pain)(entity_t *self, entity_t *other, float kick, int damage);
    void        (*Die)(entity_t *self, entity_t *inflictor, entity_t *attacker, int damage, const vec3_t &point);

    float       debounceTouchTime;        // are all these legit?  do we need more/less of them?
    float       debouncePainTime;
    float       debounceDamageTime;
    float       debounceSoundTime;    // move to clientInfo
    float       lastMoveTime;

    int         health;
    int         maxHealth;
    int         gibHealth;      // Health level required in order for an edict to gib.
    int         deadFlag;
    qboolean    showHostile;

    float       powerarmor_time;

    const char        *map;           // target_changelevel // C++20: STRING: Added const to char *

    int         viewHeight;     // height above origin where eyesight is determined
    int         takeDamage;
    int         dmg;
    int         radius_dmg;
    float       dmg_radius;
    int         sounds;         // make this a spawntemp var?
    int         count;

    entity_t     *chain;
    entity_t     *enemy;
    entity_t     *oldEnemyPtr;
    entity_t     *activator;
    
    entity_t     *groundEntityPtr;
    int         groundEntityLinkCount;
    
    entity_t     *teamChainPtr;
    entity_t     *teamMasterPtr;

    entity_t     *myNoise;       // can go in client only
    entity_t     *myNoise2;

    int         noiseIndex;
    int         noiseIndex2;
    float       volume;
    float       attenuation;

    // timing variables
    float       wait;
    float       delay;          // before firing targets
    float       random;

    float       teleportTime;

    int         waterType;
    int         waterLevel;

    vec3_t      moveOrigin;
    vec3_t      moveAngles;

    // move this to clientInfo?
    int         lightLevel;

    int         style;          // also used as areaportal number

    // Custom lightstyle.
    char        *customLightStyle;

    gitem_t     *item;          // for bonus items

    // common data blocks
    moveinfo_t      moveInfo;
    monsterinfo_t   monsterInfo;
};

#endif