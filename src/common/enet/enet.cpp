/*
License here.
*/
#define ENET_IMPLEMENTATION // Specifically only defined here, this file is the enet implementation.
#include <enet.h>

// Include the rest of what we'll be needing.
#include "shared/shared.h"
#include "common/common.h"
#include "common/cvar.h"
#ifdef _DEBUG
#include "common/files.h"
#endif
#include "common/msg.h"
#include "common/net/net.h"
#include "common/protocol.h"
#include "common/zone.h"
#include "client/client.h"
#include "server/server.h"
#include "system/system.h"
#include "common/enet/enet.h"

// We'll be needing this.
extern cvar_t* developer;

//==============================================================================================
//
// Utility Functions for Developers, and even end users.
//
// This one is quite useful since you'd like to actually notify people at times.
//
//==============================================================================================
#define ENET_DPrintf(...) \
    if (developer && developer->integer > 0) { \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__); \
    } else { \
        Com_LPrintf(PRINT_NOTICE, __VA_ARGS__); \
    }

// Super duper deluxe fatal error, exits the entire game/dedicated server with an awesome
// popup window containing your sweet sweet error!
#define ENET_FatalError(...) \
        Com_Error(ERR_FATAL, __VA_ARGS__);

// Print to the console and disconnect from the game. (Assuming the client/server were connected.)
#define ENET_DropError(...) \
        Com_Error(ERR_DROP, __VA_ARGS__);

// Like ENET_DropError, however it is NOT an error.
#define ENET_DisconnectError(...) \
        Com_Error(ERR_DISCONNECT, __VA_ARGS__);

// Will make the server broadcast reconnect messages.
#define ENET_ReconnectError(...) \
        Com_Error(ERR_RECONNECT, __VA_ARGS__);



//==============================================================================================
//
// ENET: Core/General.
//
//==============================================================================================
//---------------
// ENET_Init
//
// Will initialize enet itself, in case it isn't so already.
// 
// Returns true with success, false by failure.
//---------------
int ENET_Init(void) {
    return (enet_initialize() < 0 ? false : true);
}

//---------------
// ENET_Shutdown
//
// Will shutdown enet, call at end of application.
//---------------
void ENET_Shutdown(void) {
    // Deinitialize enet, obviously.
    enet_deinitialize();
}



//==============================================================================================
//
// SERVER: Hosts
//
//==============================================================================================
//---------------
// ENET_InitServerHost
//
// Create an ENet Server Host. Everything is a host in this UDP layered protocol.
// Connections are Peers, and they can go both ways, or just a single direction.
//---------------
ENetHost* ENET_InitServerHost(uint32_t maxClients, uint16_t port = 27910, uint32_t numberOfChannels = 2, uint32_t outGoingBandwidthlimit = 0, uint32_t incomingBandwidthLimit = 0) {
    // Bind the server to the default localhost.
    // A specific host address can be specified by
    // enet_address_set_host (& address, "x.x.x.x");
    ENetAddress address{
        .host = ENET_HOST_ANY, // This allows
        // Bind the server to port 27910. */
        .port = port
    };

    // Create our server host.
    ENetHost* e_ServerHost = enet_host_create(&address,	// The address to bind the server host to.
        maxClients,	// Allow up to 32 clients and/or outgoing connections.
        numberOfChannels,	// Allow up to 2 channels to be used, 0.
        0,	// Assume any amount of incoming bandwidth */,
        0   // Assume any amount of outgoing bandwidth */);
    );

    // Report error, if need be.
    if (e_ServerHost == nullptr) {
        ENET_DPrintf("ENet: An error occurred while trying to create the e_Server Enet Host.\n");
    }

    return e_ServerHost;
}

//---------------
// ENET_DestroyServerHost
// 
// Destroys the Enet server host.
//---------------
void ENET_DestroyServerHost(ENetHost* e_ServerHost) {
#if defined(USE_SERVER)
    enet_host_destroy(e_ServerHost);
#endif
}


//==============================================================================================
//
// CLIENT: Hosts
//
//==============================================================================================
//---------------
// ENET_InitClientHost
//
// Create an ENet Client Host. Everything is a host in this UDP layered protocol.
// Connections are Peers, and they can go both ways, or just a single direction.
//---------------
ENetHost *ENET_InitClientHost(uint32_t maxConnections, uint32_t numberOfChannels) {
    // Create our server host.
    ENetHost *e_clientHost = enet_host_create(
        NULL,	// The address to bind the server host to.
        maxConnections,	// Allow up to 32 clients and/or outgoing connections.
        numberOfChannels,	// Allow up to 2 channels to be used, 0.
        0,	// Assume any amount of incoming bandwidth */,
        0   // Assume any amount of outgoing bandwidth */);
    );

    // Report error, if need be.
    if (e_clientHost == nullptr) {
        ENET_DPrintf("ENet: An error occurred while trying to create the e_clientHost Enet Host.\n");
        return NULL;
    }

    return e_clientHost;
}

//---------------
// ENET_DestroyClientHost
// 
// Destroys the Enet Client Host.
//---------------
void ENET_DestroyClientHost(ENetHost *eClientHost) {
#if defined(USE_CLIENT)
    enet_host_destroy(eClientHost);
#endif
}



//==============================================================================================
//
// CLIENT: Peers
//
//==============================================================================================
//---------------
// ENET_DestroyClientPeer
// 
// Destroys (resets the client peer.)
//---------------
void ENET_DisconnectClientPeer(ENetPeer* eClientPeer, ENetHost* eClientHost) {
#if defined(USE_CLIENT)
    ENetEvent eEvent;

    enet_peer_disconnect(eClientPeer, 0);
    while (enet_host_service(eClientHost, &eEvent, 3000) > 0) {
        switch (eEvent.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(eEvent.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            puts("Disconnection succeeded.");
            break;
        }
    }

    // Reset our client Peer.
    enet_peer_reset(eClientPeer);
#endif
}
