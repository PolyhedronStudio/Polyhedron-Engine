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

//
// server to client
//
typedef enum {
    // these ops are known to the game dll
    //svg_muzzleflash = svc_num_types,
    //svg_muzzleflash2,
    //svg_temp_entity,
    //svg_layout,
    //svg_inventory,
    //svg_stufftext,              // [string] stuffed into client's console buffer
    //                         // should be \n terminated
    //svg_num_types
} svg_ops_t;

#endif