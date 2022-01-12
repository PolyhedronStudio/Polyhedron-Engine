/*
// LICENSE HERE.

//
// sharedgame/protocol.h
//
// The server game module gets a chance to define its own commands to send to
// the client here.
//
*/
#ifndef __SHAREDGAME_PROTOCOL_H__
#define __SHAREDGAME_PROTOCOL_H__

//-------------------
// These are server game commands that get send to the client game module.
// Add your own.
//
// The enum starts at 22, and has room up to 32 for custom commands.
//-------------------
typedef enum {
    SVG_CMD_MUZZLEFLASH = 22,
    SVG_CMD_MUZZLEFLASH2,
    SVG_CMD_TEMP_ENTITY,
    SVG_CMD_LAYOUT,
    SVG_CMD_INVENTORY,

    SVG_CMD_NUM_TYPES
} ServerGameCommands;

//-------------------
// These are client game commands that get send to the server game module.
// Add your own.
//
// The enum starts at 13, and has room up to 32 for custom commands.
//-------------------
typedef enum {
    CLG_CMD_NUM_TYPES = 13,
} ClientGameCommands;



#endif