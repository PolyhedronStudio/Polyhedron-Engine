#pragma once
/*
License here.
*/
#include <enet.h>
#include "common/sizebuf.h"

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


//=============================================================================
// 
// Core ENet Functionality.
// 
//=============================================================================
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


//=============================================================================
// 
// Connection Functionality.
// 
//=============================================================================
//---------------
// ENET_Connect
//
// Connects (attempts that is) to the host listening actively on the given address.
//---------------
#if USE_CLIENT
void ENET_Connect(const std::string &hostAddress);
#endif

//---------------
// ENET_Close
//
// Closes an active socket connection, default behavior kindly requests it
// and might give it some time before it processes. 
// 
// It can also be harshly forced, aka done DIRECTLY.
//---------------
void ENET_Close(int32_t socketID, qboolean closeNow = false);



//=============================================================================
// 
// (Size-)Buffer Functionality.
// 
//=============================================================================
//---------------
// ENET_SendBufferToPeer
//
// Send a packet of data, to the given peer from the given host, with the given flags.
// 
// Optional flags:
//    ENET_PACKET_FLAG_RELIABLE - packet must be received by the target peer and resend attempts should be made until the packet is delivered
//    ENET_PACKET_FLAG_UNSEQUENCED - packet will not be sequenced with other packets(not supported for reliable packets)
//    ENET_PACKET_FLAG_NO_ALLOCATE - packet will not allocate data, and user must supply it instead
//    ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT - packet will be fragmented using unreliable(instead of reliable) sends if it exceeds the MTU
//    ENET_PACKET_FLAG_SENT - whether the packet has been sent from all queues it has been entered into
//---------------
qboolean ENET_SendBufferToPeer(ENetPeer* ePeer, ENetHost* eHost, SizeBuffer* dataBuffer, int32_t flags = 0);

//---------------
// ENET_ConvertPacketToBuffer
//
// Clear the given buffer and write over the packet data into the buffer.
// Optionally can be told NOT to destroy the packet instantly after doing so.
//---------------
void ENET_ConvertPacketToBuffer(ENetPacket* ePacket, SizeBuffer* destDataBuffer, qboolean destroyPacket = true);


//=============================================================================
// 
// Other Functionality.
// 
//=============================================================================
//---------------
// ENET_GetSocketIDAddress
//
// Return the string address matching the socket ID.
//---------------
const std::string& ENET_GetSocketIDAddress(int32_t socketID);