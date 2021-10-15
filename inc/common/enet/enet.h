#pragma once
/*
License here.
*/
#include <enet.h>

//==============================================================================================
//
// ENET: Core/General.
//
//==============================================================================================
//---------------
// Defauult settings: Port # only in this case (for now.)
//---------------
// Default Q2 (And for now still, N&C port to use.)
#define ENET_PORT_SERVER         27910

// Portable network error codes (Taken from Q2 also)
#define ENET_OK       0  // success
#define ENET_ERROR   -1  // failure (NET_ErrorString returns error message)
#define ENET_AGAIN   -2  // operation would block, try again
#define ENET_CLOSED  -3  // peer has closed connection

//---------------
// ENET_Init
//
// Will initialize enet itself, in case it isn't so already.
// 
// Returns true with success, false by failure.
//---------------
int ENET_Init(void);

//---------------
// ENET_Shutdown
//
// Will shutdown enet, call at end of application.
//---------------
void ENET_Shutdown(void);

//---------------
// ENET_ProcessHost
//
// Will shutdown enet, call at end of application.
//---------------
int ENET_ProcessHost(ENetHost* eHost, uint32_t timeOut = 0);



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
ENetHost* ENET_InitServerHost(uint32_t maxClients = MAX_CLIENTS, uint16_t port = ENET_PORT_SERVER, uint32_t numberOfChannels = 2, uint32_t incomingBandwidthLimit = 0, uint32_t outGoingBandwidthlimit = 0);

//---------------
// ENET_DestroyServerHost
// 
// Destroys the Enet server host.
//---------------
void ENET_DestroyServerHost(ENetHost* e_ServerHost);



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
ENetHost* ENET_InitClientHost(uint32_t maxConnections = 1, uint32_t numberOfChannels = 2);

//---------------
// ENET_DestroyClientHost
// 
// Destroys the Enet Client Host.
//---------------
void ENET_DestroyClientHost(ENetHost * eClientHost);



//==============================================================================================
//
// CLIENT: Peers
//
//==============================================================================================
//---------------
// ENET_ConnectClientPeerHost
//
// Connects the selected client peer host to the given foreign address host.
//---------------
ENetPeer* ENET_ConnectClientPeer(ENetHost* eClientHost, const std::string &address = "127.0.0.1", size_t channelCount = 2, uint32_t timeOut = 0, enet_uint32 data = 0);

//---------------
// ENET_DisconnectClientPeer
// 
// Disconnects (- and resets the client peer.)
//---------------
void ENET_DisconnectClientPeer(ENetPeer* eClientPeer, ENetHost* eClientHost, uint32_t timeOut = 0, uint32_t data = 0);
