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
    // these ops are known to the game dll
    svg_muzzleflash = 22,
    svg_muzzleflash2,
    svg_temp_entity,
    svg_layout,
    svg_inventory,

    svg_num_types
} svg_ops_t;

//-------------------
// These are client game commands that get send to the server game module.
// Add your own.
//
// The enum starts at 13, and has room up to 32 for custom commands.
//-------------------
typedef enum {
    clg_custom_command = 32,
} clg_ops_t;

#endif