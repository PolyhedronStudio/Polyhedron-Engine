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

//---------------
// ENET_ProcessHost
//
// Will shutdown enet, call at end of application.
// 
// timeOut used to be fine at 1000
//---------------
int ENET_ProcessHost(ENetHost *eHost, uint32_t timeOut) {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    while (enet_host_service(eHost, &event, timeOut) > 0) {
        switch (event.type)     {
        case ENET_EVENT_TYPE_CONNECT:
            ENET_DPrintf("A new client connected from %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);
            /* Store any relevant client information here. */
            event.peer->data = (void*)"Client information";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            ENET_DPrintf("A packet of length %u containing %s was received from %s on channel %u.\n",
                event.packet->dataLength,
                event.packet->data,
                event.peer->data,
                event.channelID);
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            ENET_DPrintf("%s disconnected.\n", event.peer->data);
            /* Reset the peer's client information. */
            event.peer->data = NULL;
        }
    }

    return 0;
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
ENetHost* ENET_InitServerHost(uint32_t maxClients, uint16_t port, uint32_t numberOfChannels, uint32_t incomingBandwidthLimit, uint32_t outGoingBandwidthlimit) {
#if defined(USE_SERVER)
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
#else
    return nullptr;
#endif
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
#if defined(USE_CLIENT)
    // Create our server host.
    ENetHost *e_clientHost = enet_host_create(
        NULL,               // The address to bind the server host to.
        maxConnections,     // Allow up to 32 clients and/or outgoing connections.
        numberOfChannels,	// Allow up to 2 channels to be used, 0.
        0,	                // Assume any amount of incoming bandwidth */,
        0                   // Assume any amount of outgoing bandwidth */);
    );

    // Report error, if need be.
    if (e_clientHost == nullptr) {
        ENET_DPrintf("ENet: An error occurred while trying to create the e_clientHost Enet Host.\n");
        return NULL;
    }

    return e_clientHost;
#else
    return nullptr;
#endif
}

//---------------
// ENET_DestroyClientHost
// 
// Destroys the ENet client host and all other resources associated with it.
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
// ENET_ConnectClientPeerHost
//
// Connects the selected client peer host to the given foreign address host.
// 
// // timeOut used to be fine at 5000
//---------------
ENetPeer* ENET_ConnectClientPeer(ENetHost* eClientHost, const std::string &address, size_t channelCount, uint32_t timeOut, enet_uint32 data) {
#if defined(USE_CLIENT)
    // Decipher the address string into an appropriate ENetAddress struct.
    ENetAddress eAddress = {};

    if (enet_address_set_host_new(&eAddress, address.c_str()) < 0) {
        ENET_DropError("No available peers for initiating an ENet connection at this address: '%s'\n", address.c_str());
        return nullptr;
    }

    // Try to connect our client host to the host of the given address.
    ENetPeer* eClientPeer = enet_host_connect(eClientHost, &eAddress, channelCount, data);

    // Tough luck, we didn't retrieve a peer.
    if (eClientPeer == nullptr) {
        ENET_DropError("No available peers for initiating an ENet connection.\n");
        return nullptr;
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    ENetEvent eEvent;
    if (enet_host_service(eClientHost, &eEvent, timeOut) > 0 &&
        eEvent.type == ENET_EVENT_TYPE_CONNECT) 
    {
        ENET_DPrintf("Connecting to '%s' succeeded\n", address.c_str());
        return nullptr;
    } else {
        // Either the 5 seconds are up or a disconnect event was */
        // received. Reset the peer in the event the 5 seconds   */
        // had run out without any significant event.            */
        enet_peer_reset(eClientPeer);
        ENET_DropError("Connecting to '%s' failed\n", address.c_str());
        return nullptr;
    }
#else
    return nullptr;
#endif
}

//---------------
// ENET_DisconnectClientPeer
// 
// Disconnects (- and resets the client peer.)
//
// timeOut used to be fine at 3000
//---------------
void ENET_DisconnectClientPeer(ENetPeer* eClientPeer, ENetHost* eClientHost, uint32_t timeOut = 0, int32_t data = 0) {
#if defined(USE_CLIENT)
    ENetEvent eEvent;

    enet_peer_disconnect(eClientPeer, data);
    while (enet_host_service(eClientHost, &eEvent, timeOut) > 0) {
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
