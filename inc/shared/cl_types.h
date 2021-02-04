/*
// LICENSE HERE.

//
// clg_types.h
//
// Contains shared structures between the client and the game module.
//
*/
#ifndef __SHARED_CL_TYPES_H__
#define __SHARED_CL_TYPES_H__

//
//=============================================================================
//
// SHARED ENGINE/CLIENT DEFINITIONS.
//
// These should never be tampered with, unless you intend to recompile the
// whole engine afterwards, with the risk of possibly breaking any 
// compatibility with current mods.
//=============================================================================
//
// game print flags
#define PRINT_LOW           0       // pickup messages
#define PRINT_MEDIUM        1       // death messages
#define PRINT_HIGH          2       // critical messages
#define PRINT_CHAT          3       // chat messages    

// destination class for gi.multicast()
typedef enum {
    MULTICAST_ALL,
    MULTICAST_PHS,
    MULTICAST_PVS,
    MULTICAST_ALL_R,
    MULTICAST_PHS_R,
    MULTICAST_PVS_R
} multicast_t;

// WatIsDeze: connstate_t has been moved here, so it is known to the client game dll.
typedef enum {
    ca_uninitialized,
    ca_disconnected,    // not talking to a server
    ca_challenging,     // sending getchallenge packets to the server
    ca_connecting,      // sending connect packets to the server
    ca_connected,       // netchan_t established, waiting for svc_serverdata
    ca_loading,         // loading level data
    ca_precached,       // loaded level data, waiting for svc_frame
    ca_active,          // game views should be displayed
    ca_cinematic        // running a cinematic
} connstate_t;

// WatIsDeze: server_state_t has been moved here, so it is known to the client game dll.
typedef enum {
    ss_dead,            // no map loaded
    ss_loading,         // spawning level edicts
    ss_game,            // actively running
    ss_pic,             // showing static picture
    ss_broadcast,       // running MVD client
    ss_cinematic,
} server_state_t;

// WatIsDeze: file_info_t has been moved here, so it is known to the client game dll.
typedef struct file_info_s {
    size_t  size;
    time_t  ctime;
    time_t  mtime;
    char    name[1];
} file_info_t;

#endif // __SHARED_CL_TYPES_H__