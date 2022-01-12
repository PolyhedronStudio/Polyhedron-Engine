/*
// LICENSE HERE.

//
// pmove.h
//
// Contains engine/game shared pmove declarations.
//
*/
#ifndef __SHARED__PMOVE_H__
#define __SHARED__PMOVE_H__

struct EnginePlayerMoveType {
    static constexpr int32_t Dead = 32;     // No movement, but the ability to rotate in place
    static constexpr int32_t Freeze = 33;   // No movement at all
    static constexpr int32_t Gib = 34;      // No movement, different bounding box
};

//-----------------
// Player movement flags.The game is free to define up to 16 bits.
//-----------------
constexpr int32_t PMF_ENGINE = (1 << 0);                    // Engine flags first.
constexpr int32_t PMF_TIME_TELEPORT = (PMF_ENGINE << 1);    // time frozen in place
constexpr int32_t PMF_NO_PREDICTION = (PMF_ENGINE << 2);    // temporarily disables client side prediction
constexpr int32_t PMF_GAME = (PMF_ENGINE << 3);             // Game flags start from here.

//-----------------
// This structure needs to be communicated bit-accurate from the server to the 
// client to guarantee that prediction stays in sync, so no floats are used.
// 
// If any part of the game code modifies this struct, it will result in a 
// prediction error of some degree.
//-----------------
struct PlayerMoveState {
    uint32_t    type;

    vec3_t      origin;
    vec3_t      velocity;

    uint16_t    flags;       // Ducked, jump_held, etc
    uint16_t    time;        // Each unit = 8 ms
    uint16_t    gravity;

    // Changed by spawns, rotating objects, and teleporters
    vec3_t deltaAngles; // Add to command angles to get view direction

    // View offsets. (Only Z is used atm, beware.)
    vec3_t viewOffset;
    vec3_t viewAngles;

    // Step offset, used for stair interpolations.
    float stepOffset;
};

//-----------------
// PlayerMoveInput is part of each client user cmd.
//-----------------
struct PlayerMoveInput {
    uint8_t msec;       // Duration of the command, in milliseconds
    vec3_t viewAngles;  // The final view angles for this command
    int16_t forwardMove, rightMove, upMove; // Directional intentions
    uint8_t buttons;    // Bit mask of buttons down
    uint8_t impulse;    // Impulse cmd.
    uint8_t lightLevel; // Lightlevel.
};

//-----------------
// ClientMoveCommand is sent to the server each client frame
//-----------------
struct ClientMoveCommand {
    PlayerMoveInput input;  // the movement command

    uint32_t timeSent;      // Time sent, for calculating pings
    uint32_t timeReceived;  // Time rcvd, for calculating pings
    uint32_t commandNumber; // Current commandNumber for this move command frame

    struct {
        uint32_t simulationTime;    // The simulation time when prediction was run
        vec3_t origin;              // The predicted origin for this command
        vec3_t error;               // The prediction error for this command
    } prediction;
};

#endif // #ifndef __SHARED__PMOVE_H__