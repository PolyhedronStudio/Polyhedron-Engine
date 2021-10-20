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
#include "common/sizebuffer.h"
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

// Active net time.
double enetNetTime = 0.;

double ENET_SetNetTime() {
    enetNetTime = com_localTime;
    return enetNetTime;
}

// Unordered map, based on socket indexes which contain the netchannels that are active.
std::unordered_map<uint32_t, NetChannel> enetActiveSockets;
// Vector containing the netchannels that are currently in a connection process.
std::vector<NetChannel> enetConnectingSockets;
// Vector containing the netchannels that are currently in a disconnection process.
std::vector<NetChannel> enetDisconnectingSockets;

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
// Will shutdown ENet, only called at the end of the application of course.
//---------------
void ENET_Shutdown(void) {
    // If there are any sockets, we got to close them down.
    for (auto& enetSocket : enetActiveSockets) {
        ENET_Close(enetSocket.first, true);
    }
    
#if USE_CLIENT
    // Destroy the client host.
    enet_host_destroy(enetClientHost);
    enetClientHost = nullptr;
#endif

    // Destroy the server listening host.
    enet_host_destroy(enetListenHost);
    enetListenHost = nullptr;

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
// ENET_Connect
//
// Connects (attempts that is) to the host listening actively on the given address.
//---------------
#if USE_CLIENT
void ENET_Connect(const std::string& hostAddress) {
    // ENet address struct, needs to be filled in nicely.
    ENetAddress address;

    // Our sweet holy peer.
    ENetPeer* peer = nullptr;

    // Set Net Time.
    ENET_SetNetTime();
    
    // Create a client host in case we haven't done so yet.
    if (!enetClientHost) {
        // Allow up to 4 connections on the client host, to give us space to
        // timeout and disconnect stale sockets.
        enetClientHost = enet_host_create(NULL, 4, 2, 0, 0);

        if (!enetClientHost)
            Com_Error(ERR_DROP, "ENET: Cannot create a client host, aborting\n");
    }

    // See if we can resolve the host name
    if (enet_address_set_host_new(&address, hostAddress.c_str()) < 0) {
        Com_Error(ERR_DROP, "ENET: Cannot resolve address %s, aborting\n", hostAddress);
        return;
    }
    address.port = ENET_PORT_SERVER;

    // Connect our client host and fetch our peer to use for it.
    peer = enet_host_connect(enetClientHost, &address, 2, 0);
    if (!peer)
        Sys_Error("Could not allocate peer for ENet connection, aborting\n");

    // Create our new netchan socket.
    NetChannel netChannel{};
    netChannel.connectTime = enetNetTime;
    netChannel.ePeer = peer;
    netChannel.eHost = enetClientHost;
    netChannel.lastMessageTime = enetNetTime;

    char addressBuffer[256];
    if (enet_address_get_host_new(&peer->address, addressBuffer, 256) == 0)
        netChannel.remoteAddress = addressBuffer;
    else if (enet_address_get_host_ip_new(&peer->address, addressBuffer, 256) == 0)
        netChannel.remoteAddress = addressBuffer;
    else
        netChannel.remoteAddress = "<unknown>";

    // From here on, we'll be polling the connection process.
    enetConnectingSockets.push_back(netChannel);
}
#endif

//---------------
// ENET_Close
//
// Closes an active socket connection, default behavior kindly requests it
// and might give it some time before it processes. 
// 
// It can also be harshly forced, aka done DIRECTLY.
//---------------
void ENET_Close(int32_t socketID, qboolean closeNow) {
    // Surely do need a valid ID.
    if (!socketID)
        return;

    // It isn't an active socket.
    if (!enetActiveSockets.count(socketID)) {
        Com_WPrintf("ENET: Unable to close SocketID #%i since it isn't active.\n");
        return;
    }

    // Fetch a reference to the active socket.
    NetChannel& socket = enetActiveSockets[socketID];

    // Disconnect from the peer in case we have one actively going.
    if (socket.ePeer) {
        if (closeNow) {
            // Force a disconnection from a peer.
            enet_peer_disconnect_now(socket.ePeer, 0); // WID: TODO: Perhaps pass some data along?

            // Forcefully disconnect the peer.
            enet_peer_reset(socket.ePeer);
        } else {
            // Just a regular disconnect request.
            enet_peer_disconnect(socket.ePeer, 0); // WID: TODO: Perhaps pass some data along?

            // Add a copy of the socket to the disconnecting queue.
            enetDisconnectingSockets.push_back(socket);
        }
    }

    // Take it out of the active queue.
    enetActiveSockets.erase(socketID);
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
qboolean ENET_SendBufferToPeer(ENetPeer* ePeer, ENetHost* eHost, SizeBuffer* dataBuffer, int32_t flags) {

    // Create the actual packet.
    ENetPacket* packet = enet_packet_create(dataBuffer->data, dataBuffer->currentSize, flags);
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
void ENET_ConvertPacketToBuffer(ENetPacket* ePacket, SizeBuffer* destDataBuffer, qboolean destroyPacket) {
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
    return enetActiveSockets[socketID].remoteAddress;
}