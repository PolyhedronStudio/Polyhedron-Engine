// ENet Implementation definition resides ONLY here.
#define ENET_IMPLEMENTATION
#include <enet.h>

#include "shared/shared.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/msg.h"
#include "common/enet/enetchan.h"
#include "common/net/net.h"
#include "common/protocol.h"
#include "common/sizebuf.h"
#include "common/zone.h"
#include "system/system.h"

// ENet Callbacks.
static ENetCallbacks enetCallbacks;

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

// ENet Listen address.
ENetAddress enetListenAddress = {
    ENET_HOST_ANY,
    ENET_PORT_SERVER,
};

// ENet Listen host.
ENetHost* enetListenHost = nullptr;
#if USE_CLIENT
// ENet Listen client.
ENetHost* enetClientHost = nullptr;
#endif

std::unordered_map<int, netchan_t> net_activeSockets;
std::vector<netchan_t> net_connectingSockets;
std::vector<netchan_t> net_disconnectingSockets;

//---------------
// ENET_Init
//
// Will initialize enet itself, in case it isn't so already.
// 
// Returns true with success, false by failure.
//---------------
int ENET_Init(void) {
	// We need to ensure this one is zero-ed out.
	std::memset(&enetCallbacks, 0, sizeof(ENetCallbacks));

	// Add any callbacks to it here.
    // Options are:
    // malloc
    // free
    // no_memory
    // packet_create
    // packet_destroy

	// Initialize ENET with callbacks.
    if (enet_initialize_with_callbacks(ENET_VERSION, &enetCallbacks) < 0) {
        Com_Printf("ENET: Failed to initialize ENet. Application won't function without it.\n");
    }

    // Create our server listening host.
    enetListenHost = enet_host_create(&enetListenAddress, 8, 2, 0, 0);
    
    if (!enetListenHost) {
        // Warn, drop out.
        Com_Printf("ENET: Failed to create enetListenHost. Application won't function without it.\n");
    }

	return ENET_OK;
}

//---------------
// ENET_Shutdown
//
// Will shutdown enet, call at end of application.
//---------------
void ENET_Shutdown(void) {
    // Deinitialize enet.
    enet_deinitialize();
}

//---------------
// ENET_ProcessHost
//
// Will shutdown enet, call at end of application.
//---------------
int ENET_ProcessHost(ENetHost* eHost, uint32_t timeOut) {
    ENetEvent event;

    while (enet_host_service(eHost, &event, timeOut) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);
                /* Store any relevant client information here. */
                event.peer->data = (void*)"Client information";
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                printf("A packet of length %ui containing %s was received from %s on channel %u.\n",
                    event.packet->dataLength,
                    (const char*)event.packet->data,
                    (const char*)event.peer->data,
                    event.channelID);
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                printf("%s disconnected.\n", (const char*)event.peer->data);
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }

    return ENET_OK;
}

//---------------
// ENET_SendToPeer
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
qboolean ENET_SendBufferToPeer(ENetPeer* ePeer, ENetHost* eHost, sizebuf_t* dataBuffer, int32_t flags) {

    // Create the actual packet.
    ENetPacket* packet = enet_packet_create(dataBuffer->data, dataBuffer->cursize, flags);
    if (!packet) {
        return false;
    }

    // Send the packet to the peer.
    if (enet_peer_send(ePeer, 0, packet) < 0) {
        return false;
    }

    // Flush the host, aka, send it away.
    enet_host_flush(eHost);

    // Success.
    return true;
}

//---------------
// ENET_PacketToBuffer
//
// Clear the given buffer and write over the packet data into the buffer.
// Optionally can be told NOT to destroy the packet instantly after doing so.
//---------------
void ENET_ConvertPacketToBuffer(ENetPacket* ePacket, sizebuf_t* destDataBuffer, qboolean destroyPacket) {
    // Clear.
    SZ_Clear(destDataBuffer);

    // Write.
    SZ_Write(destDataBuffer, ePacket->data, ePacket->dataLength);

    // Destroy the packet.
    enet_packet_destroy(ePacket);
}

//---------------
// ENET_GetSocketIDAddress
//
// Return the string socket address matching the socket ID.
//---------------
// Return the string of the socket ID address.
const std::string& ENET_GetSocketIDAddress(int32_t socketID) {
    return net_activeSockets[socketID].address;
}