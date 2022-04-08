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

    //----------------------------------------------------------
    //! Server time at which this animation started.
    //uint32_t animationEventStartTime = 0;
    ////! Animation Index
    //uint16_t animationEventIndex = 0;
    ////! Animation Start Frame
    //uint16_t animationStartFrame = 0;
    ////! Animation End Frame
    //uint16_t animationEndFrame = 0;
    ////! Animation Frame Time. (30fps for a 30fps mesh = 1 second, etc.)
    //float animationEventframeTime = 30.f;

    //+++ Get the below working first, then do the on top, use events??
    
    //    Events might be easier to use with regards to possible predictions?
    //! Server start time of current animation.
    int32_t animationStartTime = 0;
    //! Animation Start Frame
    uint16_t animationStartFrame = 1;
    //! Animation End Frame
    uint16_t animationEndFrame = 2;
    //! Current animation playback framerate.
    float animationFramerate = 30.f;
    //! Amount of loops to do.
    uint8_t animationLoopCount = 0;
    //! Force loop?
    uint8_t animationForceLoop = false;

    //--- Part of the old frame code :P
    
    //! Current animation frame the entity is at.
    float animationFrame = 0.f;
    //----------------------------------------------------------

    //! Model skin number.
    int32_t skinNumber = 0;
    //! Entity Effects. (Rotating etc.)
    uint32_t effects = 0;  // PGM - we're filling it, so it needs to be unsigned
    //! Entity Render Effects. (For Shells, Transparency etc.)
    int32_t renderEffects = 0;
    //! For client side prediction. 8*(bits0-4) is x/y radius. 8*(bits 5-9) is z down distance. 8(bits10-15) is z up.
    int32_t solid = 0;	//! gi.LinkEntity sets this properly.
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