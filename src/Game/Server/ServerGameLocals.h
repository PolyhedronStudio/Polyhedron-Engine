/***
*
*	License here.
*
*	@file
*
*	GameLocal class contains all of the game. Its world, entities, clients, items, etc.
*   It stays persistently intact until the end of the game, when the dll is unloaded.
* 
*
***/
#pragma once


// Shared includes.
//#include "../../Shared/Shared.h"


// Because we define the full size ServerClient and Entity structures in this file
// we define GAME_INCLUDE so that SVGame.h does not define the short server-visible variety.
//#define GAME_INCLUDE
#include "../../Shared/Shared.h"
#include "../../Shared/List.h"
struct gclient_s;
struct PODEntity;


// The "gameversion" client command will print this including the compile date
#define GAMEVERSION "basepoly"

/**
*	SharedGame Framework.
**/
#include "../Shared/SharedGame.h"

/**
*	ClientGame Trace Results.
**/
#include "Utilities/SVGTraceResult.h"


// Predeclare.
class SVGBaseEntity;
class SVGBaseItem;
class SVGBaseItemWeapon;
class SVGBasePlayer;
class IGameWorld;
class IGameMode;
class ServerGameWorld;

struct PODEntity;


//
// TODO:    Inherit GameLocals from IGameExports. (IGameExports has yet to be created and serves as the exports class to a server.)
//
//          Add a SetMaxEntities, SetMaxClients, and Allocate functions that are friendly to several other objects.

/**
*	@brief GameLocal is the main server game class.
* 
*	@details 
**/
class GameLocals {
public:
    /**
	*	@brief Default constructor.
	**/
    GameLocals() = default;

    /**
	*	@brief Default destructor
	**/
    ~GameLocals() = default;

public:
    /**
	*	@brief Initializes the gameworld and its member objects.
	**/
    void Initialize();
    
    /**
	*	@brief Shutsdown the gamelocal.
	**/
    void Shutdown();



    /**
    *   @return A pointer to the gameworld object.
    **/
    ServerGameWorld* GetGameWorld();

    /**
    *   @return A pointer to the gameworld its current gamemode object.
    **/
    IGameMode* GetGameMode();

    /**
    *   @brief  Code shortcut for accessing gameworld's client array.
    * 
    *   @return A pointer to the gameworld's clients array.
    **/
    ServerClient* GetClients();
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxClients.
    * 
    *   @return The maximum allowed clients in this game.
    **/
    int32_t GetMaxClients();
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxEntities.
    * 
    *   @return The maximum allowed entities in this game.
    **/
    int32_t GetMaxEntities();



    /**
    *   
    **/
    
    /**
    *   
    **/
private:
    /**
    *   @brief Create the world member object and initialize it.
    **/
    void CreateWorld();
    /**
    *   @brief De-initialize the world and destroy it.
    **/
    void DestroyWorld();


    // TODO: Add Get methods and privatize the members below.
public:
    //! GameWorld.
    ServerGameWorld* world = nullptr;

    //! needed for coop respawns
    //! Can't store spawnpoint32_t in level, because
    //! it would get overwritten by the savegame restore
    char spawnpoint[512];

    //! Will be set to latched cvar equivelants due to having to access them a lot.
    //int32_t maxClients = 0;
    //int32_t maxEntities = 0;

    //! Used to store Cross level triggers.
    int32_t serverflags = 0;

    //! Did we autosave?
    qboolean autoSaved = false;
};


//==================================================================

//! MS Frametime for animations.
//static constexpr float ANIMATION_FRAMETIME = BASE_FRAMETIME;//FRAMERATE_MS;

//! Float time it takes to go over a frame. 
//static constexpr Frametime FRAMETIME = FRAMETIME_S;
#define FRAMETIME FRAMETIME_S

//! Memory tags to allow dynamic memory to be cleaned up
static constexpr int32_t TAG_GAME = 765;   // clear when unloading the dll
static constexpr int32_t TAG_LEVEL = 766;  // clear when loading a new level

//! View pitching times
static constexpr Frametime DAMAGE_TIME = 0.5s;
static constexpr Frametime FALL_TIME = 0.3s;

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



static constexpr int32_t MELEE_DISTANCE = 80;




/**
*   Combat Ranges.
**/
struct CombatRange {
    static constexpr int32_t Melee  = 0;
    static constexpr int32_t Near   = 1;
    static constexpr int32_t Middle = 2;
    static constexpr int32_t Far    = 3;
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


/**
*	@brief	Stores level locals, from current time, to which entities are sighted.
*	
*			This structure is cleared as each map is entered it is read/written 
*			to the 'level.sav' file for savegames.
**/
struct LevelLocals  {
    //! Current level time in Milliseconds.
    GameTime time = GameTime::zero();

    char levelName[MAX_QPATH];  //! The descriptive name (Outer Base, etc)
    char mapName[MAX_QPATH];    //! The server name (base1, etc)
    char nextMap[MAX_QPATH];    //! Go here when fraglimit is hit

    /**
	*	Stores the state for a games 'Intermission state'. This is where the
    *	camera hangs somewhere, at the end of a map, players get to say
    *	'hey ggg u n00bz' after seeing each other's scores. When all is said and
    *	done the game moves on to the next map. This is when exitIntermission != 0
    **/
    struct {
        GameTime time = GameTime::zero(); // Time the intermission was started
        const char* changeMap; // Map to switch to after intermission has exited.
        int32_t exitIntermission = 0; // Set to true(1) when exiting the intermission should take place.
        vec3_t origin = vec3_zero(); // Origin for intermission to take place at.
        vec3_t viewAngle = vec3_zero(); // View angle to apply for intermission.
    } intermission;

    // The actual client the AI has sight on for this current frame.
    Entity *sightClient = nullptr;  // Changed once each frame for coop games

    // The current entity that is actively being ran from SVG_RunFrame.
    IServerGameEntity *currentEntity = nullptr;

    // Index for the que pile of dead bodies.
    int32_t bodyQue = 0;

    // A bit ugly, but one can store coop values here.    
};


/**
*	Game Externals.
**/
//! Global game object.
extern  GameLocals   game;
//! Global level locals.
extern  LevelLocals  level;

/**
*   @return A pointer to the game's world object. The man that runs the show.
**/
ServerGameWorld* GetGameWorld();

/**
*   @return A pointer to the gamemode object. The man's little helper.
**/
IGameMode* GetGameMode();



/**
*	Core - Used take and give access from game module to server.
**/
extern  ServerGameImports gi;         // CLEANUP: These were game_import_t and game_export_T
extern  ServerGameExports globals;    // CLEANUP: These were game_import_t and game_export_T





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

// Very ugly macros, need to rid ourselves and inline func them at the least.
// Also, there should be alternatives in our utils for math lib as is.
#define random()    ((rand () & RAND_MAX) / ((float)RAND_MAX))
#define crandom()   (2.0f * (random() - 0.5f))

/**
*   @brief  Spawnflags for items, set by editor(s).
**/
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

/**
*   @brief  Flags describing damage details.
*/
struct DamageFlags {
    static constexpr int32_t IndirectFromRadius = 0x00000001;  // Damage was indirect
    static constexpr int32_t NoArmorProtection = 0x00000002;  // Armour does not protect from this damage
    static constexpr int32_t EnergyBasedWeapon = 0x00000004;  // Damage is from an energy based weapon
    static constexpr int32_t NoKnockBack = 0x00000008;  // Do not affect velocity, just view angles
    static constexpr int32_t Bullet = 0x00000010;  // Damage is from a bullet (used for ricochets)
    static constexpr int32_t IgnoreProtection = 0x00000020;  // Armor, shields, invulnerability, and godmode have no effect
};


void    SVG_ServerCommand(void);
qboolean SVG_FilterPacket(char *from);

/**
*   @brief  
**/
void SVG_RunEntity(SGEntityHandle &entityHandle);

//-----------------------------------------------------------------------------------------------------------

// TODO: All these go elsewhere, sometime, as does most...
void SVG_SetConfigString(const int32_t &configStringIndex, const std::string &configString);

std::vector<IServerGameEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount = MAX_WIRED_POD_ENTITIES, int32_t areaType = AreaEntities::Solid);

qhandle_t SVG_PrecacheModel(const std::string& filename);
qhandle_t SVG_PrecacheImage(const std::string& filename);
qhandle_t SVG_PrecacheSound(const std::string& filename);

void SVG_CPrint(IServerGameEntity* ent, int32_t printlevel, const std::string& str);
void SVG_DPrint(const std::string &str);
void SVG_CenterPrint(IServerGameEntity* ent, const std::string& str);
void SVG_Sound(IServerGameEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset);

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
    
    //! The level.timeStamp on which the player entered the game.
    GameTime enterGameTimestamp = GameTime::zero();

    //! A client's score, usually this means the amount of frags.
    int32_t score = 0;

    //! Angles sent over in the last client user command.
    vec3_t commandViewAngles = vec3_zero();
    
    //! Is this client a spectator?
    qboolean isSpectator = false;
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
        GameTime timeStamp = GameTime::zero();
        
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
        vec3_t fromOrigin = vec3_zero();
    } damages;

    //! Yaw angle of where our killer is located at in case we're dead.
    float killerYaw = 0.f;

    //! Current kick angles.
    vec3_t kickAngles = vec3_zero();
    //! Current kick origin.
    vec3_t kickOrigin = vec3_zero();

    /**
    *   @brief  View damage kicks.
    **/
    struct {
        //! Roll for view damage kick.
        double roll  = 0.f;
        //! Pitch for view damage kick.
        double pitch = 0.f;
        //! Time of view damage.
        Frametime time = Frametime::zero();
    } viewDamage;

    //! Falling time.
    Frametime fallTime = Frametime::zero();
    //! Falling value for adding on to the view's pitch when landing.
    double fallValue = 0.f;
    //! Alpha value for damage indicator display.
    double damageAlpha = 0.f;
    //! Alpha value for bonus indicator display.
    double bonusAlpha = 0.f;
    //! Total damage blend value.
    vec3_t damageBlend = vec3_zero();
    //! Aiming direction.
    vec3_t aimAngles = vec3_zero();
    //! Store bob time so going off-ground doesn't change it.
    double bobTime = 0.f;

    //! Old view angles.
    vec3_t oldViewAngles = vec3_zero();
    //! Old velocity.
    vec3_t oldVelocity = vec3_zero();

    //! Timer for the next drown event.
    GameTime nextDrownTime = GameTime::zero();
    //! Old water level.
    int32_t oldWaterLevel = 0;

    //! For weapon raising
    int32_t machinegunShots = 0;

    /**
    *   @brief  Animation state.
    **/
    struct ClientAnimationState {
        float endFrame = 0;
        float priorityAnimation = 0;

        qboolean    isDucking = false;
        qboolean    isRunning = false;
    } animation;

    //! Weapon Sound.
    int32_t weaponSound = 0;

    //! Pick up message time.
    GameTime pickupMessageTime = GameTime::zero();

    /**
    *   @brief  Used to protect from chat flooding.
    **/
    struct ClientFloodProtection {
        Frametime lockTill = GameTime::zero();   // Locked from talking.
        Frametime when[10] = { };    // When messages were said.
        int32_t whenHead = 0;   // Head pointer for when said
    } flood;

    //! Client can respawn when time > this
    GameTime respawnTime = GameTime::zero();

    //! The (client)player we are chasing
    Entity *chaseTarget = nullptr;

    //! Do we need to update chase info?
    qboolean updateChase = false;
};



/***
*
*
*	CVars
*
*
***/
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