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
constexpr uint32_t BUTTON_ATTACK		= (1 << 0);
constexpr uint32_t BUTTON_USE			= (1 << 1);
constexpr uint32_t BUTTON_WALK			= (1 << 2);
constexpr uint32_t BUTTON_UNUSED_0		= (1 << 3);
constexpr uint32_t BUTTON_UNUSED_1		= (1 << 4);
constexpr uint32_t BUTTON_UNUSED_2		= (1 << 5);
constexpr uint32_t BUTTON_UNUSED_3		= (1 << 6);
constexpr uint32_t BUTTON_ANY			= (1 << 7);

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
#define EF_ROTATE           0x00000001      // rotate (bonus items)
#define EF_GIB              0x00000002      // leave a trail
#define EF_BLASTER          0x00000008      // redlight + trail
#define EF_ROCKET           0x00000010      // redlight + trail
#define EF_GRENADE          0x00000020
#define EF_HYPERBLASTER     0x00000040
#define EF_BFG              0x00000080
#define EF_COLOR_SHELL      0x00000100
#define EF_POWERSCREEN      0x00000200
#define EF_ANIM01           0x00000400      // automatically cycle between frames 0 and 1 at 2 hz
#define EF_ANIM23           0x00000800      // automatically cycle between frames 2 and 3 at 2 hz
#define EF_ANIM_ALL         0x00001000      // automatically cycle through all frames at 2hz
#define EF_ANIM_ALLFAST     0x00002000      // automatically cycle through all frames at 30hz
#define EF_FLIES            0x00004000
#define EF_QUAD             0x00008000
#define EF_PENT             0x00010000
#define EF_TELEPORTER       0x00020000      // particle fountain
#define EF_CORPSE           0x00040000      // to differentiate own corpse from self
#define EF_FLAG2            0x00080000
// RAFAEL
#define EF_IONRIPPER        0x00100000
#define EF_GREENGIB         0x00200000
#define EF_BLUEHYPERBLASTER 0x00400000
#define EF_SPINNINGLIGHTS   0x00800000
#define EF_PLASMA           0x01000000
#define EF_TRAP             0x02000000

//ROGUE
#define EF_TRACKER          0x04000000
#define EF_DOUBLE           0x08000000
#define EF_SPHERETRANS      0x10000000
#define EF_TAGTRAIL         0x20000000
#define EF_HALF_DAMAGE      0x40000000
#define EF_TRACKERTRAIL     0x80000000
//ROGUE

// entity_state_t->renderfx flags
#define RF_MINLIGHT         1       // allways have some light (viewmodel)
#define RF_VIEWERMODEL      2       // don't draw through eyes, only mirrors
#define RF_WEAPONMODEL      4       // only draw through eyes
#define RF_FULLBRIGHT       8       // allways draw full intensity
#define RF_DEPTHHACK        16      // for view weapon Z crunching
#define RF_TRANSLUCENT      32
#define RF_FRAMELERP        64
#define RF_BEAM             128
#define RF_CUSTOMSKIN       256     // skin is an index in image_precache
#define RF_GLOW             512     // pulse lighting for bonus items
#define RF_SHELL_RED        1024
#define RF_SHELL_GREEN      2048
#define RF_SHELL_BLUE       4096

//ROGUE
#define RF_IR_VISIBLE       0x00008000      // 32768
#define RF_SHELL_DOUBLE     0x00010000      // 65536
#define RF_SHELL_HALF_DAM   0x00020000
#define RF_USE_DISGUISE     0x00040000
//ROGUE

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
#define MZ_BLASTER          0
#define MZ_MACHINEGUN       1
#define MZ_SHOTGUN          2
#define MZ_CHAINGUN1        3
#define MZ_CHAINGUN2        4
#define MZ_CHAINGUN3        5
#define MZ_RAILGUN          6
#define MZ_ROCKET           7
#define MZ_GRENADE          8
#define MZ_LOGIN            9
#define MZ_LOGOUT           10
#define MZ_RESPAWN          11
#define MZ_BFG              12
#define MZ_SSHOTGUN         13
#define MZ_HYPERBLASTER     14
#define MZ_ITEMRESPAWN      15
// RAFAEL
#define MZ_IONRIPPER        16
#define MZ_BLUEHYPERBLASTER 17
#define MZ_PHALANX          18
#define MZ_SILENCED         128     // bit flag ORed with one of the above numbers

//ROGUE
#define MZ_ETF_RIFLE        30
#define MZ_UNUSED           31
#define MZ_SHOTGUN2         32
#define MZ_HEATBEAM         33
#define MZ_BLASTER2         34
#define MZ_TRACKER          35
#define MZ_NUKE1            36
#define MZ_NUKE2            37
#define MZ_NUKE4            38
#define MZ_NUKE8            39
//ROGUE
// Q2RTX
#define MZ_FLARE            40
// Q2RTX

//-----------------
// monster muzzle flashes
//-----------------
#define MZ2_TANK_BLASTER_1              1
#define MZ2_TANK_BLASTER_2              2
#define MZ2_TANK_BLASTER_3              3
#define MZ2_TANK_MACHINEGUN_1           4
#define MZ2_TANK_MACHINEGUN_2           5
#define MZ2_TANK_MACHINEGUN_3           6
#define MZ2_TANK_MACHINEGUN_4           7
#define MZ2_TANK_MACHINEGUN_5           8
#define MZ2_TANK_MACHINEGUN_6           9
#define MZ2_TANK_MACHINEGUN_7           10
#define MZ2_TANK_MACHINEGUN_8           11
#define MZ2_TANK_MACHINEGUN_9           12
#define MZ2_TANK_MACHINEGUN_10          13
#define MZ2_TANK_MACHINEGUN_11          14
#define MZ2_TANK_MACHINEGUN_12          15
#define MZ2_TANK_MACHINEGUN_13          16
#define MZ2_TANK_MACHINEGUN_14          17
#define MZ2_TANK_MACHINEGUN_15          18
#define MZ2_TANK_MACHINEGUN_16          19
#define MZ2_TANK_MACHINEGUN_17          20
#define MZ2_TANK_MACHINEGUN_18          21
#define MZ2_TANK_MACHINEGUN_19          22
#define MZ2_TANK_ROCKET_1               23
#define MZ2_TANK_ROCKET_2               24
#define MZ2_TANK_ROCKET_3               25

#define MZ2_INFANTRY_MACHINEGUN_1       26
#define MZ2_INFANTRY_MACHINEGUN_2       27
#define MZ2_INFANTRY_MACHINEGUN_3       28
#define MZ2_INFANTRY_MACHINEGUN_4       29
#define MZ2_INFANTRY_MACHINEGUN_5       30
#define MZ2_INFANTRY_MACHINEGUN_6       31
#define MZ2_INFANTRY_MACHINEGUN_7       32
#define MZ2_INFANTRY_MACHINEGUN_8       33
#define MZ2_INFANTRY_MACHINEGUN_9       34
#define MZ2_INFANTRY_MACHINEGUN_10      35
#define MZ2_INFANTRY_MACHINEGUN_11      36
#define MZ2_INFANTRY_MACHINEGUN_12      37
#define MZ2_INFANTRY_MACHINEGUN_13      38

#define MZ2_SOLDIER_BLASTER_1           39
#define MZ2_SOLDIER_BLASTER_2           40
#define MZ2_SOLDIER_SHOTGUN_1           41
#define MZ2_SOLDIER_SHOTGUN_2           42
#define MZ2_SOLDIER_MACHINEGUN_1        43
#define MZ2_SOLDIER_MACHINEGUN_2        44

#define MZ2_GUNNER_MACHINEGUN_1         45
#define MZ2_GUNNER_MACHINEGUN_2         46
#define MZ2_GUNNER_MACHINEGUN_3         47
#define MZ2_GUNNER_MACHINEGUN_4         48
#define MZ2_GUNNER_MACHINEGUN_5         49
#define MZ2_GUNNER_MACHINEGUN_6         50
#define MZ2_GUNNER_MACHINEGUN_7         51
#define MZ2_GUNNER_MACHINEGUN_8         52
#define MZ2_GUNNER_GRENADE_1            53
#define MZ2_GUNNER_GRENADE_2            54
#define MZ2_GUNNER_GRENADE_3            55
#define MZ2_GUNNER_GRENADE_4            56

#define MZ2_CHICK_ROCKET_1              57

#define MZ2_FLYER_BLASTER_1             58
#define MZ2_FLYER_BLASTER_2             59

#define MZ2_MEDIC_BLASTER_1             60

#define MZ2_GLADIATOR_RAILGUN_1         61

#define MZ2_HOVER_BLASTER_1             62

#define MZ2_ACTOR_MACHINEGUN_1          63

#define MZ2_SUPERTANK_MACHINEGUN_1      64
#define MZ2_SUPERTANK_MACHINEGUN_2      65
#define MZ2_SUPERTANK_MACHINEGUN_3      66
#define MZ2_SUPERTANK_MACHINEGUN_4      67
#define MZ2_SUPERTANK_MACHINEGUN_5      68
#define MZ2_SUPERTANK_MACHINEGUN_6      69
#define MZ2_SUPERTANK_ROCKET_1          70
#define MZ2_SUPERTANK_ROCKET_2          71
#define MZ2_SUPERTANK_ROCKET_3          72

#define MZ2_BOSS2_MACHINEGUN_L1         73
#define MZ2_BOSS2_MACHINEGUN_L2         74
#define MZ2_BOSS2_MACHINEGUN_L3         75
#define MZ2_BOSS2_MACHINEGUN_L4         76
#define MZ2_BOSS2_MACHINEGUN_L5         77
#define MZ2_BOSS2_ROCKET_1              78
#define MZ2_BOSS2_ROCKET_2              79
#define MZ2_BOSS2_ROCKET_3              80
#define MZ2_BOSS2_ROCKET_4              81

#define MZ2_FLOAT_BLASTER_1             82

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
// Extern - TODO: Remove, ofc.
//-----------------
extern  const vec3_t monster_flash_offset[];


//-----------------
// Temp Entity Events (TE)
//
// Temp entity events are for things that happen at a location seperate from 
// any existing entity. Temporary entity messages are explicitly constructed
// and broadcast.
//-----------------
typedef enum {
    TE_GUNSHOT,
    TE_BLOOD,
    TE_BLASTER,
    TE_RAILTRAIL,
    TE_SHOTGUN,
    TE_EXPLOSION1,
    TE_EXPLOSION2,
    TE_ROCKET_EXPLOSION,
    TE_GRENADE_EXPLOSION,
    TE_SPARKS,
    TE_SPLASH,
    TE_BUBBLETRAIL,
    TE_SCREEN_SPARKS,
    TE_SHIELD_SPARKS,
    TE_BULLET_SPARKS,
    TE_LASER_SPARKS,
    TE_PARASITE_ATTACK,
    TE_ROCKET_EXPLOSION_WATER,
    TE_GRENADE_EXPLOSION_WATER,
    TE_MEDIC_CABLE_ATTACK,
    TE_BFG_EXPLOSION,
    TE_BFG_BIGEXPLOSION,
    TE_BOSSTPORT,           // used as '22' in a map, so DON'T RENUMBER!!!
    TE_BFG_LASER,
    TE_GRAPPLE_CABLE,
    TE_WELDING_SPARKS,
    TE_GREENBLOOD,
    TE_BLUEHYPERBLASTER,
    TE_PLASMA_EXPLOSION,
    TE_TUNNEL_SPARKS,
    //ROGUE
    TE_BLASTER2,
    TE_RAILTRAIL2,
    TE_FLAME,
    TE_LIGHTNING,
    TE_DEBUGTRAIL,
    TE_PLAIN_EXPLOSION,
    TE_FLASHLIGHT,
    TE_FORCEWALL,
    TE_HEATBEAM,
    TE_MONSTER_HEATBEAM,
    TE_STEAM,
    TE_BUBBLETRAIL2,
    TE_MOREBLOOD,
    TE_HEATBEAM_SPARKS,
    TE_HEATBEAM_STEAM,
    TE_CHAINFIST_SMOKE,
    TE_ELECTRIC_SPARKS,
    TE_TRACKER_EXPLOSION,
    TE_TELEPORT_EFFECT,
    TE_DBALL_GOAL,
    TE_WIDOWBEAMOUT,
    TE_NUKEBLAST,
    TE_WIDOWSPLASH,
    TE_EXPLOSION1_BIG,
    TE_EXPLOSION1_NP,
    TE_FLECHETTE,
    //ROGUE
    // Q2RTX
    TE_FLARE,
    // Q2RTX

    TE_NUM_ENTITIES
} temp_event_t;

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










// dmflags->value flags
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
#define DF_QUAD_DROP        0x00004000  // 16384
#define DF_FIXED_FOV        0x00008000  // 32768

// RAFAEL
#define DF_QUADFIRE_DROP    0x00010000  // 65536

//ROGUE
#define DF_NO_MINES         0x00020000
#define DF_NO_STACK_DOUBLE  0x00040000
#define DF_NO_NUKES         0x00080000
#define DF_NO_SPHERES       0x00100000
//ROGUE

/*
ROGUE - VERSIONS
1234    08/13/1998      Activision
1235    08/14/1998      Id Software
1236    08/15/1998      Steve Tietze
1237    08/15/1998      Phil Dobranski
1238    08/15/1998      John Sheley
1239    08/17/1998      Barrett Alexander
1230    08/17/1998      Brandon Fish
1245    08/17/1998      Don MacAskill
1246    08/17/1998      David "Zoid" Kirsch
1247    08/17/1998      Manu Smith
1248    08/17/1998      Geoff Scully
1249    08/17/1998      Andy Van Fossen
1240    08/20/1998      Activision Build 2
1256    08/20/1998      Ranger Clan
1257    08/20/1998      Ensemble Studios
1258    08/21/1998      Robert Duffy
1259    08/21/1998      Stephen Seachord
1250    08/21/1998      Stephen Heaslip
1267    08/21/1998      Samir Sandesara
1268    08/21/1998      Oliver Wyman
1269    08/21/1998      Steven Marchegiano
1260    08/21/1998      Build #2 for Nihilistic
1278    08/21/1998      Build #2 for Ensemble

9999    08/20/1998      Internal Use
*/
#define ROGUE_VERSION_ID        1278

#define ROGUE_VERSION_STRING    "08/21/1998 Beta 2 for Ensemble"

// ROGUE





//==============================================


#endif // __SHAREDGAME_SHAREDGAME_H__