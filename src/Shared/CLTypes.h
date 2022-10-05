/***
*
*	License here.
*
*	@file
*
*	Contains shared structures between the client and the game module.
* 
***/
#pragma once

#include "Shared.h"
#include "Refresh.h"

#include "../Common/Bsp.h"
#include "../Common/CollisionModel.h"
#include "../Common/Bsp.h"
#include "../Common/Cmd.h"
#include "../Common/Protocol.h"
#include "../Common/CollisionModel.h"
#include "../Common/Common.h"


/**
*	Client Load States that are common.
**/
enum LoadState {
    LOAD_NONE,
    LOAD_MAP,
    LOAD_MODELS,
    LOAD_IMAGES,
    LOAD_CLIENTS,
    LOAD_SOUNDS
};

/**
*   Contains states for the KeyBindings.
**/
struct ButtonState {
    static constexpr uint32_t Held  = (1 << 0);
    static constexpr uint32_t Down  = (1 << 1);
    static constexpr uint32_t Up    = (1 << 2);
};

/**
*   Explosion particle entity effect structure.
**/
struct explosion_t {
	enum {
		ex_free,
		ex_explosion,
		ex_misc,
		ex_flash,
		ex_mflash,
		ex_poly,
		ex_poly2,
		ex_light,
		ex_blaster,
		ex_flare
	} type;

	r_entity_t  ent;
	int32_t     frames;
	float       light;
	vec3_t      lightcolor;
	float       start;
	int64_t     baseFrame;
	int64_t     frameTime; /* in milliseconds */
};

// Maximum amount of explosions.
static constexpr uint32_t MAX_EXPLOSIONS = 32;

// No Particle Settings.
static constexpr uint32_t NOPART_GRENADE_EXPLOSION = 1;
static constexpr uint32_t NOPART_GRENADE_TRAIL = 2;
static constexpr uint32_t NOPART_ROCKET_EXPLOSION = 4;
static constexpr uint32_t NOPART_ROCKET_TRAIL = 8;
static constexpr uint32_t NOPART_BLOOD = 16;

// No Explosion settings.
static constexpr uint32_t NOEXP_GRENADE = 1;
static constexpr uint32_t NOEXP_ROCKET = 2;


//
// Temporarl Entity parameters.
// Used for parsing EFFECTS in the client.
//
struct tent_params_t {
	/**
	*	New Properties.
	**/
	//! Possible received velocity.
	vec3_t velocity = vec3_zero();
	//! Received damage.
	float damage = 0.f;
	//! Received speed.
	float speed = 0.f;
	//! Received model index.
	int32_t modelIndex1 = 0;

	// Debris Specific.
	int32_t debrisGibType = 0;	//! Type of specific Debris Gib..

    /**
    *	Classical Q2 TE Properties.
    **/
    int32_t type = 0;
    vec3_t position1 = vec3_zero();
    vec3_t position2 = vec3_zero();
    vec3_t offset = vec3_zero();
    vec3_t dir = vec3_zero();
    int32_t count = 0;
    int32_t color = 0;
    int32_t entity1 = 0;
    int32_t entity2 = 0;
    int64_t time = 0;
};

//
// MuzzleFlash parameters.
// Used for parsing MUZZLEFLASHE messages in the client.
//
struct mz_params_t {
    int32_t entity = 0;
    int32_t weapon = 0;
    int32_t silenced = 0;
};

//
// Sound parameters.
// Used for parsing SOUND messages in the client.
//
struct snd_params_t {
    int32_t flags;
    int32_t index;
    int32_t entity;
    int32_t channel;
    vec3_t  pos;
    float   volume;
    float   attenuation;
    float   timeofs;
};

//
// Client Sustain structure.
//
struct cl_sustain_t {
    int32_t id = 0;
    int32_t type = 0;
    int64_t endTime = 0;
    int64_t nextThinkTime = 0;
    int64_t thinkinterval = 0;
    vec3_t  org = vec3_zero();
    vec3_t  dir = vec3_zero();
    int32_t color = 0;
    int32_t count = 0;
    int32_t magnitude = 0;
    void    (*Think)(cl_sustain_t *self) = nullptr;
};

//
// Client Particle Structure.
//
struct cparticle_t {
    cparticle_t    *next;

    float   time;

    vec3_t  org;
    vec3_t  vel;
    vec3_t  acceleration;
    int32_t color;      // -1 => use rgba
    float   alpha;
    float   alphavel;
    color_t rgba;
	float   brightness;
};

//
// Client DLight structure.
//
struct cdlight_t {
	//! So entities can reuse same entry
    int32_t key;

	//! Color of this dynamic light.
    vec3_t color;
	//! Origin.
    vec3_t origin;

	//! Light radius.
    float radius;

	//! Kills/Stops the light after 'die' amount of time has passed.
    double die;
	//! The value at which to decay this light with over each second that passes. (A game frame.)
    double decay;
	//! Move velocity(distance) for each second that passes. (A game frame.)
	vec3_t velocity;
	//! Don't add when contributing less
    //float   minlight;
};

/**
*	TODO: Currently not really used... ?
*	Maximum amount of weapon models allowed.
**/
static constexpr int32_t MAX_CLIENTWEAPONMODELS = 20;        // PGM -- upped from 16 to fit the chainfist vwep

/**
*	Contains all the info about a client we need to know.
**/
struct ClientInfo {
    char name[MAX_QPATH];           // The client name.
    char model_name[MAX_QPATH];     // The model name.
    char skin_name[MAX_QPATH];      // The skin name.

    qhandle_t skin;                 // The skin handle.
    qhandle_t icon;                 // The icon handle. 
    qhandle_t model;                // Model handle.

    qhandle_t weaponmodel[MAX_CLIENTWEAPONMODELS];  // The weapon model handles.
};

/**
*	Data needed for storing a 'client user input' command history. Store its time at which
*	it was sent as well as when it was received. Used for calculating pings.
**/
struct ClientUserCommandHistory {
	//! Current commandNumber for this frame,
    uint64_t commandNumber = 0;

	//! Time sent, for calculating pings.
	uint64_t timeSent = 0;
	//! Time received, for calculating pings.
    uint64_t timeReceived = 0;    
};

/**
*	@brief Stores the received from server's frame data.
**/
struct ServerFrame {
    qboolean valid = false; //! False if delta parsing failed.

	//! Sequential unique identifier.
    int64_t number = 0;
	//! Amount of delta between frames between previous and most recent received 'valid frames'.
    int64_t delta = 0;

	//! Area bits of this frame.
    byte    areaBits[MAX_MAP_AREA_BYTES];
	//! Area bytes.
    int32_t areaBytes = 0;

	//! The player state data received for this server frame.
	PlayerState playerState = {};
	//! The client number. (May differ if lets say, spectating an other client.)
    int32_t     clientNumber = 0;

	//! Amount of entity states received in this frame:
	//! firstEntity + numEntities = first index up to last index of entities in received frame.
    int32_t numEntities = 0;
	//! The first entity number index in the received frame.
    int32_t firstEntity = 0;
};

/**
*	@brief	Stores the actual current client frame data.
*			NOTE: May in the future also be similar to ServerFrame in that it gets a history log.
**/
struct ClientLocalFrame {
	//! Local client side game frame number. Does NOT need to match that of the server.
	uint64_t number = 0;
};


//
// This structure contains all (persistent)shared data with the client.
//
struct ClientShared {
    virtual ~ClientShared() = default;

    // Stores the entities.
    PODEntity *entities = nullptr;//PODEntity entities[MAX_CLIENT_POD_ENTITIES];
    int num_entities;
};

/**
*	Stores the final move results after processing all our game frame's client user movement
*	commands. This in return is later on compared to that of the newly received frame in order
*	to determine if we need to correct our prediction in case the mismatch is too large.
**/
struct ClientPredictedState {
    //! These are the actual predicted results that should align with the server's.
    //! Predicted view origin.
	vec3_t viewOrigin	= vec3_zero();
	//! Predicted view offset.
    vec3_t viewOffset	= vec3_zero();
	//! Predicted view angles.
    vec3_t viewAngles	= vec3_zero();
	//! Predicted stepping offset. (Up or down)
    float stepOffset	= 0.;

	//! Predicted velocity.
    vec3_t velocity = vec3_zero();

	//! The current player move mins/maxs.
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();

    //! Ground entity pointer of the predicted frame.
	int32_t groundEntityNumber = -1;

    // Prediction error that is interpolated over the server frame.
    vec3_t error = vec3_zero();

	// Flags of the predicted state. (Allows us to check whether we are about to extrapolate for example.)
	uint16_t flags = 0;
};

/**
*	@brief	Stores all 'current game' client state data that is true to its heart. Not to be confused with 
*			client static, which stores state data that is persistent and incremental in that it never 
*			gets reset during each 'game change'.
**/
struct ClientState {
	//! Chat time out counter.
    int32_t timeoutCount = 0;

	//! TODO: Stores the icon but do we need it here?!
    byte            dcs[CS_BITMAP_BYTES] = {};

	/**
    *
    *   Client User Command Related.
    *
    **/
    //! Initial outgoing command sequence number.
    int32_t initialSequence = 0;

	//! Determines whether to send the user command packet asap, and preferably, NOW.
    bool sendPacketNow = false;
    //! The exact 'simulation time' we last transmitted a user command.
    uint64_t lastTransmitTime = 0;
    //! The last transmitted command number. This may differ from the one below.
    uint64_t lastTransmitCmdNumber = 0;
    //! The ACTUAL last transmitted number which wasn't stalled by not being ready to send yet.
    uint64_t lastTransmitCmdNumberReal = 0;

    //! Predicted Client State. (Used for movement.)
    ClientPredictedState predictedState = {};
	//! The client maintains its own idea of view angles, which are
    //! sent to the server each frame.  It is cleared to 0 upon entering each level.
    //! the server sends a delta each frame which is added to the locally
    //! tracked view angles to account for standing on rotating objects,
    //! and teleport direction changes
    vec3_t viewAngles = vec3_zero();
    //! Interpolated movement vector used for local prediction, never sent to server, rebuilt each client frame
    vec3_t localMove = vec3_zero();
    //! Accumulated mouse forward/side movement, added to both localMove and pending cmd, cleared each time cmd is finalized.
    vec2_t mouseMove = vec2_zero();

    //! Current client move command number.
    uint64_t currentClientCommandNumber = 0;
	//! The current local client's frame move command that is to be processed.
    ClientMoveCommand moveCommand = {};
	    
	//! History of processed client move commands for during an entire game frame (1 second).
	//! Each client to server packet mesage will send several old clientUserCommands(3) along with it.
    ClientMoveCommand clientUserCommands[CMD_BACKUP] = {};
    //! A history log indexing in clientUserCommands that keeps track of their 'timeSent' and 'timeReceived'.
    ClientUserCommandHistory clientCommandHistory[CMD_BACKUP] = {};


    /**
    *
    *   Entity States.
    *
    **/
    //! Solid Entities, these are REBUILT during EACH FRAME.
    PODEntity *solidEntities[MAX_PACKET_ENTITIES];// = {};
    int32_t numSolidEntities = 0;
    //! Solid Local Entities, these are REBUILT during EACH FRAME.
    PODEntity *solidLocalEntities[MAX_NON_WIRED_POD_ENTITIES];// = {};
    int32_t numSolidLocalEntities = 0;

    //! Stores the so called entity state 'baselines' which are transmitted during connecting.
    EntityState entityBaselines[MAX_WIRED_POD_ENTITIES];//= {};

    //! Stores all received packet entity states. MAX_PARSE_ENTITIES is based on how many frames we want to backup.
	//! These frames are used to 
    EntityState entityStates[MAX_PARSE_ENTITIES]; // DO NOT initialize this using {} or VS2022 will stall your machine and give a nasty stack error.
    int32_t numEntityStates  = 0;
	//! Stores all local entity states also from previous and current frame. (Client frames).
	EntityState localEntityStates[MAX_PARSE_ENTITIES];

    //! The current client entity state messaging flags.
    uint32_t entityStateFlags = 0;


    /**
    *
    *   Server Frames.
    *
    **/
	//! The actual time on the server at the time of receiving the most recent 'valid frame'.
    int64_t serverTime = 0;
	//! Amount of possibly missed frames between the previous and most recent received 'valid frames'.
    int64_t serverDelta = 0;

    //! A list of server frames received.
    ServerFrame  frames[UPDATE_BACKUP];
    uint32_t     frameFlags = 0;

    //! The current(most recent) received frame.
    ServerFrame frame = {}; 
    //! The previous (last/most recent) received frame.
    ServerFrame oldframe = {};


    /**
    *
    *   Client Frames.
    *
    **/
	//! Current client frame data.
	ClientLocalFrame clientFrame = {};

    //! This is the 'current moment in time' value of the client's game state at.  
    //! Always <= cl.serverTime
    int64_t	time = 0;
	//! This is the 'extrapolated moment in time' value of the client's game state.
	//! Always >= cl.serverTime and <= cl.serverTime + FRAMERATE_MS
	int64_t extrapolatedTime = 0;

    //! Linear interpolation fraction between cl.oldframe and cl.frame.
    double		lerpFraction = 0.f;
    //! We always extrapolate only a single frame ahead. Linear extrapolation fraction between cl.oldframe and cl.frame.
    double		xerpFraction = 0.f;


    /**
    *
    *   Client Sound.
    *
    **/
    //! Used for storing the listener origin of the client.
    vec3_t      listener_origin = vec3_zero();

    //! Special Effect: UNDERWATER
    qboolean    snd_is_underwater = false;
    qboolean    snd_is_underwater_enabled = false;


    /**
    *
    *   Client Input.
    *
    **/
    float       autosens_x = 0.f;
    float       autosens_y = 0.f;


    /**
    *
    *   Refresh Related.
    *
    **/
    //! Refresh Definitions of current frame for the renderer.
    refdef_t refdef = {};
    //! X Field of View.
    float fov_x = 0.f; //! Interpolated
    //! Y Field of View.
    float fov_y = 0.f; //! Derived from fov_x assuming 4/3 aspect ratio
    
	//! Client Possible Visibility Set, used for local entities.
	struct ClientPVS {
		// PVS Set.
		byte pvs[8192];
		// Last valid cluster.
		int32_t lastValidCluster = -1;

		//! Leaf, Area, and Current Cluster.
		mleaf_t *leaf = nullptr;
		int32_t leafArea = -1;
		int32_t leafCluster = -1;

		// Area Bits.
		byte areaBits[32];
		// Area Btes.
		int32_t areaBytes;
	} clientPVS;

	//! UNUSED: LightLevel, server has no concept of where rays are so... not done.
    int32_t lightLevel  = 0;

	//! Whether in third person or not.
    qboolean thirdPersonView = false;
    //! Predicted values, used for smooth player entity movement in thirdperson view
    vec3_t playerEntityOrigin = vec3_zero();
    vec3_t playerEntityAngles = vec3_zero();

    bsp_t *bsp = nullptr;	//! BSP For rendering.
	cm_t cm;				//! Collision Model for local entities.


    /**
    *
    *   Transient Data From Server.
    *
    **/
    //char    layout[MAX_NET_STRING] = {};     //! general 2D overlay
    int32_t inventory[MAX_ITEMS] = {};


    /**
    *
    *   Server State Information.
    *
    **/
    int32_t serverState = 0;    //! ss_* constants
    int32_t serverCount = 0;    //! server identification for prespawns
    
    int32_t clientNumber = 0;   //! never changed during gameplay, set by serverdata packet
    int32_t maximumClients = 0;

    char gamedir[MAX_QPATH] = {};
    char mapName[MAX_QPATH] = {}; // short format - q2dm1, etc

    char baseConfigStrings[ConfigStrings::MaxConfigStrings][MAX_QPATH] = {};
    char configstrings[ConfigStrings::MaxConfigStrings][MAX_QPATH] = {};


#if USE_AUTOREPLY
    uint32_t replyTime = 0;
    uint32_t replyDelta = 0;
#endif


    /**
    *
    *   Locally Derived Information from Server State.
    *
    **/
    qhandle_t drawModels[MAX_MODELS] = {};   //! Handles for loaded draw models (MD2, MD3, ...).
    mmodel_t *clipModels[MAX_MODELS] = {};   //! mmodel_t ptr handles for loaded clip models (Brush models).

    struct {
        qhandle_t sounds[MAX_SOUNDS] = {};   //! Handles to the loaded sounds.
        qhandle_t images[MAX_IMAGES] = {};   //! Handles to the loaded images.
    } precaches;

    //! Client info for all clients.
    ClientInfo clientInfo[MAX_CLIENTS] = {};
    //! Local, Player, Client Info.
    ClientInfo baseClientInfo = {};

    //! Weapon Models string paths.
    char    weaponModels[MAX_CLIENTWEAPONMODELS][MAX_QPATH] = {};
    //! Number of weapon models.
    int32_t numWeaponModels = 0;
};
