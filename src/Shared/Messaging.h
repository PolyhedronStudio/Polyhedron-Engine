/***
*
*	License here.
*
*	@file
*
*	Contains network messaging related POD structures, and related constexpr
*   enums, etc.
*
***/
#pragma once


//! Maximum amount of stats available to the player state.
static constexpr int32_t MAX_PLAYERSTATS = 32;


/**
*
*	Messaging/Network
*
**/
/**
*   Destination class for gi.Multicast()
* 
*   Reliable messages will always arrive even if it takes multiple frames,
*   unreliable messages will accept being dropped by a lag or if a frame's message
*   data has already been pumped to the rim with reliable messages.
*   
*   It is best to use reliables only for things that are truly cognitively important
*   to clients. Anything else should be put in unreliable messages.
**/
struct Multicast {
    static constexpr int32_t All = 0;       //! Send an unreliable message to all clients.
    static constexpr int32_t PHS = 1;       //! Send an unreliable message to all clients who are in the possible hearing set.
    static constexpr int32_t PVS = 2;       //! Send an unreliable message to all clients who are in the possible visibility set.
    static constexpr int32_t All_R = 3;     //! Send a reliable message to all clients.
    static constexpr int32_t PHS_R = 4;     //! Send a reliable message to all clients who are in the possible hearing set.
    static constexpr int32_t PVS_R = 5;     //! Send a reliable message to all clients who are in the possible visibility set.
};

/**
*   Connection State of the client.
**/
struct ClientConnectionState {
    static constexpr int32_t Uninitialized = 0;
    static constexpr int32_t Disconnected = 1;  //! Not talking to a server.
    static constexpr int32_t Challenging = 2;   //! Sending getchallenge packets to the server.
    static constexpr int32_t Connecting = 3;    //! Sending connect packets to the server.
    static constexpr int32_t Connected = 4;     //! Netchan_t established, waiting for ServerCommand::ServerData.
    static constexpr int32_t Loading = 5;       //! Loading level data.
    static constexpr int32_t Precached = 7;     //! Loaded level data, waiting for ServerCommand::Frame.
    static constexpr int32_t Spawning = 8;      //! Spawning local client entities.
    static constexpr int32_t Active = 9;        //! Game views should be displayed.
    static constexpr int32_t Cinematic = 10;    //! Running a cinematic.
};

/**
*   Run State of the Server.
**/
struct ServerState {
    static constexpr int32_t Dead = 0;            //! No map loaded.
    static constexpr int32_t Loading = 1;         //! Spawning level edicts.
    static constexpr int32_t Game = 2;            //! Actively running.
    static constexpr int32_t Pic = 3;             //! Showing static picture.
    static constexpr int32_t Cinematic = 4;
};

/**
*   EntityState->event values
*   
*   Entity events are for effects that take place relative to an existing 
*   entities origin. Very network efficient.
*   
*   All muzzle flashes really should be converted to events...
**/
struct EntityEvent {
    static constexpr int32_t None = 0;
    static constexpr int32_t ItemRespawn = 1;
    static constexpr int32_t Footstep = 2;
    static constexpr int32_t FallShort = 3;
    static constexpr int32_t Fall = 4;
    static constexpr int32_t FallFar = 5;
    static constexpr int32_t PlayerTeleport = 6;
    static constexpr int32_t OtherTeleport = 7;
};

/**
*	@brief	Animation State Object for the PODEntity.
**/
struct EntityAnimationState {
	/**
	*	The following are sent over the wire.
	**/
	//! Animation Index that
	uint32_t animationIndex = 0;
	//! Server time of the start of the animation.
	uint64_t startTime = 0;

	/**
	*	The following are used to keep track and process the animation.
	*
	*	None of these are sent over the wire.
	**/
	//! Current animation frame.
	int32_t		frame = 0;
	//! Animation frametime.
	float		frameTime = 0;
	//! Backlerp
	double		backLerp = 0;

	//! Animation start IQM frame number.
	uint32_t	startFrame = 0;
	//! Animation end IQM frame number. (start + number of frames.)
	uint32_t	endFrame = 0;
	//! 0 means to play on forever, but do return -1.
	uint32_t	loopCount = 0;
	//! When force loop is set it'll never return a -1 event.
	bool		forceLoop = 0;
};

/**
*   EntityState is the information conveyed from the server
*   in an update message about entities that the client will
*   need to render in some way
**/
struct EntityState {
    //! Entity index number.
    int32_t number = 0;
    //! Hashed class name.
    uint32_t hashedClassname = 0;

    //! Entity Origin.
    vec3_t origin = vec3_zero();
    //! Entity Angles.
    vec3_t angles = vec3_zero();
    //! Old entity origin, used for lerping.
    vec3_t oldOrigin = vec3_zero();
    //! Main entity model index.
    int32_t modelIndex = 0;
    //! Extended model indices.
    int32_t modelIndex2 = 0, modelIndex3 = 0, modelIndex4 = 0;


	/**
	*	An Entity State, stores two Animation States:
	*		- An Entity State is a specific moment in time, with key data representing the entity itself.
	*		- Because of the above, we want to store which animation was current, and previous, at THAT moment in time.
	**/
	//! Current Animation for the moment in time of EntityState.
	EntityAnimationState currentAnimation	= {};
	//! Previous Animation for the moment in time of EntityState.
	EntityAnimationState previousAnimation	= {};

	/**
	*	General 'old' animation data. TODO: Have another close look, no need to keep it as float anymore is there?
	**/
    //! Current animation frame the entity is at.
    float animationFrame = 0.f;


    /**
	*	Linear Movement, used for prediction:
	*		- Currently in try-out for 'func_plat'.
	**/
	//! Whether we are moving along a linear movement path, or not.
	bool linearMovement				= false;
	//! The current velocity that this entity is traveling along its linear movement path.
	vec3_t linearMovementVelocity	= vec3_zero();
	//! The starting origin for the entity's linear movement path
	vec3_t linearMovementBeginOrigin	= vec3_zero();
	//! The end origin for the entity's linear movement path
	vec3_t linearMovementEndOrigin		= vec3_zero();

	//! uint32_t movement duration.
	uint32_t linearMovementDuration	= 0;
	//! Movement start timestamp.
	int64_t linearMovementTimeStamp	= 0;


	/**
	*	General Physical and Rendering State Data.
	**/
    //! Model skin number.
    int32_t skinNumber = 0;
    //! Entity Effects. (Rotating etc.)
    uint32_t effects = 0;  // PGM - we're filling it, so it needs to be unsigned
    //! Entity Render Effects. (For Shells, Transparency etc.)
    int32_t renderEffects = 0;
    
	//! For client side prediction.
    int32_t solid = 0;	//! LinkEntity sets this properly from entity state to podentity itself.
	//! For client side prediction. x/y need to be integral values(we won't go less than 1 unit.)
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();

    //! For looping sounds, used to guarantee a shutoff.
    int32_t sound = 0;
    //! Impulse events, cleared after each frame. (Muzzle flashes, footsteps, etc.)
    int32_t eventID = 0;
};


/**
*   @brief  A PlayerState contains the information needed in addition to the PlayerMoveState
*           to render a view. Every game frame of a second, a PlayerState is sent.
*
*           The number of PlayerMoveState changes will be reletive to client frame rates
**/
struct PlayerState {
    //! State of the actual movement. (Used for client side movement prediction.)
    PlayerMoveState pmove = {};

    // These fields do not need to be communicated bit-precise
    //! Adds to view direction to get render angles. (Set by weapon kicks, pain effects, etc.)
    vec3_t  kickAngles = vec3_zero();

    //! Gun angles.
    vec3_t  gunAngles = vec3_zero();
    //! Gun offset.
    vec3_t  gunOffset = vec3_zero();

    //! View Model weapon index.
    int32_t     gunIndex = 0;

    //! Server start time of current animation.
    uint64_t     gunAnimationStartTime = 0;
    //! Animation Start Frame
    int16_t     gunAnimationStartFrame = 1;
    //! Animation End Frame
    int16_t     gunAnimationEndFrame = 2;
    //! Current animation playback frame time.
    float       gunAnimationFrametime = 15.f;
    //! Amount of loops to do.
    uint8_t     gunAnimationLoopCount = 0;
    //! Force loop?
    uint8_t     gunAnimationForceLoop = false;

    //! RGBA Full Screen blend effect.
    float   blend[4] = { 0.f, 0.f, 0.f, 0.f };  // RGBA full screen effect
    //! Field of View.
    float   fov = 0;            // Horizontal field of view
    // Refresh render flags.
    int32_t rdflags = 0;        // Refdef flags
    //! Status bar information.
    int16_t   stats[MAX_PLAYERSTATS] = {};
};