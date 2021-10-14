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
ENetHost* ENET_InitServerHost(uint32_t maxClients, uint16_t port, uint32_t numberOfChannels, uint32_t outGoingBandwidthlimit, uint32_t incomingBandwidthLimit);

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
ENetHost* ENET_InitClientHost(uint32_t maxConnections, uint32_t numberOfChannels = 2);

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
// ENET_DestroyClientPeer
// 
// Destroys (resets the client peer.)
//---------------
void ENET_DisconnectClientPeer(ENetPeer* eClientPeer, ENetHost* eClientHost);
