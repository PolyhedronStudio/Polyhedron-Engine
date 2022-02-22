/*
// LICENSE HERE.

//
// messaging.h
//
// Contains messaging related structures.
//
*/
#ifndef __SHARED__MESSAGING_H__
#define __SHARED__MESSAGING_H__

//
//=============================================================================
//
//	Messaging/Network
//
//=============================================================================
//
//-----------------
// Destination class for gi.Multicast()
//-----------------
struct Multicast {
    static constexpr int32_t All = 0;
    static constexpr int32_t PHS = 1;
    static constexpr int32_t PVS = 2;
    static constexpr int32_t All_R = 3;
    static constexpr int32_t PHS_R = 4;
    static constexpr int32_t PVS_R = 5;
};

//-----------------
// Connection State of the client.
//-----------------
struct ClientConnectionState {
    static constexpr int32_t Uninitialized = 0;
    static constexpr int32_t Disconnected = 1;  // Not talking to a server
    static constexpr int32_t Challenging = 2;   // Sending getchallenge packets to the server
    static constexpr int32_t Connecting = 3;    // Sending connect packets to the server
    static constexpr int32_t Connected = 4;     // Netchan_t established, waiting for svc_serverdata
    static constexpr int32_t Loading = 5;       // Loading level data
    static constexpr int32_t Precached = 6;     // Loaded level data, waiting for svc_frame
    static constexpr int32_t Active = 7;        // Game views should be displayed
    static constexpr int32_t Cinematic = 8;     // Running a cinematic
};

//-----------------
// Run State of the server.
//-----------------
struct ServerState {
    static constexpr int32_t Dead = 0;            // No map loaded
    static constexpr int32_t Loading = 1;         // Spawning level edicts
    static constexpr int32_t Game = 2;            // Actively running
    static constexpr int32_t Pic = 3;             // Showing static picture
    static constexpr int32_t Cinematic = 4;
};

//-----------------
// EntityState->event values
// 
// Entity events are for effects that take place relative to an existing 
// entities origin. Very network efficient.
// 
// All muzzle flashes really should be converted to events...
//-----------------
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

//-----------------
// EntityState is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
//-----------------
struct EntityState {
    //! Entity index number.
    int32_t number = 0;

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

    //! Current animation frame the entity is at.
    float animationFrame = 0.f;
    //! Current animation playback framerate.
    float animationFramerate = 0.f;

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

//-----------------
// PlayerState is the information needed in addition to PlayerMoveState
// to rendered a view.  There will only be 10 PlayerState sent each second,
// but the number of PlayerMoveState changes will be reletive to client
// frame rates
//-----------------
// Maximum amount of stats available to the player state.
#define MAX_STATS               32

struct PlayerState {
    //! State of the actual movement. (Used for client side movement prediction.)
    PlayerMoveState   pmove;

    // These fields do not need to be communicated bit-precise
    //! Adds to view direction to get render angles. (Set by weapon kicks, pain effects, etc.)
    vec3_t      kickAngles;

    //! Gun angles.
    vec3_t      gunAngles;
    //! Gun offset.
    vec3_t      gunOffset;
    //! Actual gun index.
    int         gunIndex;
    //! Gun animation frame. 
    float       gunAnimationFrame;
    //! Gun animation framerate.
    float gunAnimationFramerate;
    //! Server Time at which the gun animation started.
    float gunAnimationStartTime;

    //! RGBA Full Screen blend effect.
    float       blend[4];       // RGBA full screen effect
    //! Field of View.
    float       fov;            // Horizontal field of view
    // Refresh render flags.
    int         rdflags;        // Refdef flags
    //! Status bar information.
    short       stats[MAX_STATS];
}; 

#endif // __SHARED__MESSAGING_H__
