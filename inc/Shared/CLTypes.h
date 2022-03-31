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
    int32_t     key;        // so entities can reuse same entry
    vec3_t  color;
    vec3_t  origin;
    float   radius;
    float   die;        // stop lighting after this time
    float   decay;      // drop this each second
	vec3_t  velocity;     // move this far each second
    //float   minlight;   // don't add when contributing less
};

//
// Maximum amount of weapon models allowed.
//
static constexpr int32_t MAX_CLIENTWEAPONMODELS = 20;        // PGM -- upped from 16 to fit the chainfist vwep

//
// Contains all the info about a client we need to know.
//
struct ClientInfo {
    char name[MAX_QPATH];           // The client name.
    char model_name[MAX_QPATH];     // The model name.
    char skin_name[MAX_QPATH];      // The skin name.

    qhandle_t skin;                 // The skin handle.
    qhandle_t icon;                 // The icon handle. 
    qhandle_t model;                // Model handle.

    qhandle_t weaponmodel[MAX_CLIENTWEAPONMODELS];  // The weapon model handles.
};

//
// Used for storing client input commands.
//
struct ClientUserCommandHistory {
    uint64_t timeSent;      // Time sent, for calculating pings.
    uint64_t timeReceived;  // Time received, for calculating pings.
    uint64_t commandNumber; // Current commandNumber for this frame,
};

//
// The server frame structure contains information about the frame
// being sent from the server.
//
struct ServerFrame {
    qboolean valid; // False if delta parsing failed.

    int32_t number; // Sequential identifier, used for delta.
    int32_t delta;  // Delta between frames.

    byte    areaBits[MAX_MAP_AREA_BYTES];   // Area bits of this frame.
    int32_t areaBytes;                      // Area bytes.

    PlayerState playerState;    // The player state.
    int32_t     clientNumber;   // The client number.

    int32_t numEntities;    // firstEntity + numEntities = first index up to last index of entities in received frame.
    int32_t firstEntity;    // The first entity number in the received frame.
};

//
// This structure contains all (persistent)shared data with the client.
//
struct ClientShared {
    virtual ~ClientShared() = default;

    // Stores the entities.
    ClientEntity entities[MAX_ENTITIES];
    int num_entities;
};

//
// Contains the predicted state (view origin, offset, angles, etc) of the client.
//
struct ClientPredictedState {
    // These are the actual predicted results that should align with the server's.
    vec3_t viewOrigin;  // Predicted view origin.
    vec3_t viewOffset;  // Predicted view offset.
    vec3_t viewAngles;  // Predicted view angles.
    float stepOffset;   // Predicted stepping offset. (Up or down)

    // Predicted velocity.
    vec3_t velocity;

    // Ground entity pointer of the predicted frame.
    struct entity_s* groundEntityPtr;

    // Prediction error that is interpolated over the server frame.
    vec3_t error;
};

//
// The client structure is cleared at each level load, and is exposed to
// * the client game module to provide access to media and other client state.
//
struct ClientState {
    /**
    *
    *   Client User Command Related.
    *
    **/
    int32_t     timeoutCount = 0;

    //! The time we last transmitted a user command.
    uint64_t    lastTransmitTime = 0;
    //! The last transmitted command number. This may differ from the one below.
    uint64_t    lastTransmitCmdNumber = 0;
    //! The ACTUAL last transmitted number which wasn't stalled by not being ready to send yet.
    uint64_t    lastTransmitCmdNumberReal = 0;
    //! Determines whether to send the user command packet asap, and preferably, NOW.
    qboolean    sendPacketNow = 0;

    //! Actual current client user command.
    ClientMoveCommand    moveCommand = {};
    //! Actual current client user command list.
    ClientMoveCommand    clientUserCommands[CMD_BACKUP] = {};    // each mesage will send several old clientUserCommands
    //! Current command number.
    uint64_t     currentClientCommandNumber = 0;
    //! History book of time sent, received, and command number.
    ClientUserCommandHistory clientCommandHistory[CMD_BACKUP] = {};

    //! Initial outgoing sequence number.
    int32_t initialSequence = 0;

    //! Predicted Client State. (Used for movement.)
    ClientPredictedState predictedState = {};


    /**
    *
    *   Entity States.
    *
    **/
    //! Solid Entities, these are REBUILT during EACH FRAME.
    ClientEntity *solidEntities[MAX_PACKET_ENTITIES];// = {};
    int32_t numSolidEntities = 0;

    //! Entity Baseline States. These are where to start working from.
    EntityState entityBaselines[MAX_EDICTS];//= {};

    //! The actual current Entity States.
    EntityState entityStates[MAX_PARSE_ENTITIES]; // DO NOT initialize this using {} or VS2022 will stall your machine and give a nasty stack error.
    int32_t numEntityStates  = 0;

    //! The current client entity state messaging flags.
    uint32_t    entityStateFlags = 0;


    /**
    *
    *   Server Frames.
    *
    **/
    //! A list of server frames received.
    ServerFrame  frames[UPDATE_BACKUP];
    uint32_t     frameFlags = 0;

    //! The current(last received frame from the server)
    ServerFrame frame = {}; 
    //! The previous frame received, right before the current frame.
    ServerFrame oldframe = {};
    int64_t serverTime = 0;
    int64_t serverDelta = 0;

    byte            dcs[CS_BITMAP_BYTES] = {};

    //! The client maintains its own idea of view angles, which are
    //! sent to the server each frame.  It is cleared to 0 upon entering each level.
    //! the server sends a delta each frame which is added to the locally
    //! tracked view angles to account for standing on rotating objects,
    //! and teleport direction changes
    vec3_t      viewAngles = vec3_zero();

    //! Interpolated movement vector used for local prediction, never sent to server, rebuilt each client frame
    vec3_t      localMove = vec3_zero();

    //! Accumulated mouse forward/side movement, added to both localMove and pending cmd, cleared each time cmd is finalized.
    vec2_t      mouseMove = vec2_zero();
    //! This is the time value that the client is rendering at.  always <= cl.serverTime
    int64_t         time = 0;
    //! between oldframe and frame
    float       lerpFraction = 0.f;


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
    *   Client Rendering Variables.
    *
    **/
    refdef_t refdef = {};
    float fov_x = 0.f; //! Interpolated
    float fov_y = 0.f; //! Derived from fov_x assuming 4/3 aspect ratio
    int32_t lightLevel  = 0;

    //! Updated in ClientGameExports::ClientUpdateOrigin.
    vec3_t v_forward = vec3_zero(), v_right = vec3_zero(), v_up = vec3_zero();

    qboolean thirdPersonView = false;

    //! Predicted values, used for smooth player entity movement in thirdperson view
    vec3_t playerEntityOrigin = vec3_zero();
    vec3_t playerEntityAngles = vec3_zero();
    

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
    vec3_t deltaAngles = vec3_zero();


    /**
    *
    *   Locally Derived Information from Server State.
    *
    **/
    bsp_t *bsp = nullptr;                  //! Pointer to the actual BSP.

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
