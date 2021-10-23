/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

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
#include <enet.h>

/*

packet header
-------------
31  sequence
1   does this message contain a reliable payload
31  acknowledge sequence
1   acknowledge receipt of even/odd message
16  qport

The remote connection never knows if it missed a reliable message, the
local side detects that it has been deltaFramePacketDrops by seeing a sequence acknowledge
higher thatn the last reliable sequence, but without the correct evon/odd
bit for the reliable set.

If the sender notices that a reliable message has been deltaFramePacketDrops, it will be
retransmitted.  It will not be retransmitted again until a message after
the retransmit has been acknowledged and the reliable still failed to get theref.

if the sequence number is -1, the packet should be handled without a netcon

The reliable message can be added to at any time by doing
MSG_Write* (&netChannel->message, <data>).

If the message buffer is overflowed, either by a single message, or by
multiple frames worth piling up while the last reliable transmit goes
unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable
message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable
parts of the message, they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be deltaFramePacketDrops, but do
not kill the connection.  This, combined with the tight window of valid
reliable acknowledgement numbers provides protection against malicious
address spoofing.


The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.


If there is no information that needs to be transfered on a given frame,
such as during the connection stage while waiting for the client to load,
then a packet only needs to be delivered if there is something in the
unacknowledged reliable
*/

#ifdef _DEBUG
static cvar_t* showpackets;
static cvar_t* showdrop;
#define SHOWPACKET(...) \
    if (showpackets->integer) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#define SHOWDROP(...) \
    if (showdrop->integer) \
        Com_LPrintf(PRINT_DEVELOPER, __VA_ARGS__)
#else
#define SHOWPACKET(...)
#define SHOWDROP(...)
#endif

cvar_t* net_qport;
cvar_t* net_maxmsglen;
cvar_t* net_chantype;

// allow either 0 (no hard limit), or an integer between 512 and 4086
static void net_maxmsglen_changed(cvar_t* self) {
    if (self->integer) {
        Cvar_ClampInteger(self, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);
    }
}

/*
===============
Netchan_Init

===============
*/
void Netchan_Init(void) {
    int     port;

#ifdef _DEBUG
    showpackets = Cvar_Get("showpackets", "0", 0);
    showdrop = Cvar_Get("showdrop", "0", 0);
#endif

    // pick a port value that should be nice and random
    port = Sys_Milliseconds() & 0xffff;
    net_qport = Cvar_Get("qport", va("%d", port), 0);
    net_maxmsglen = Cvar_Get("net_maxmsglen", va("%d", MAX_PACKETLEN_WRITABLE_DEFAULT), 0);
    net_maxmsglen->changed = net_maxmsglen_changed;
    net_chantype = Cvar_Get("net_chantype", "1", 0);
}

/*
===============
Netchan_OutOfBand

Sends a text message in an out-of-band datagram
================
*/
void Netchan_OutOfBand(NetSource sock, const netadr_t* address,
    const char* format, ...) {
    va_list     argptr;
    struct {
        uint32_t    header;
        char        data[MAX_PACKETLEN_DEFAULT - 4];
    } packet;
    size_t      len;

    // write the packet header
    packet.header = 0xffffffff;

    va_start(argptr, format);
    len = Q_vsnprintf(packet.data, sizeof(packet.data), format, argptr);
    va_end(argptr);

    if (len >= sizeof(packet.data)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    // send the datagram/
    NET_SendPacket(sock, &packet, len + 4, address);
}

// ============================================================================

/*
===============
Netchan_TransmitNextFragment
================
*/
size_t Netchan_TransmitNextFragment(NetChannel* netChannel) {
    SizeBuffer   send;
    byte        send_buf[MAX_PACKETLEN];
    qboolean    send_reliable;
    uint32_t    w1, w2;
    uint16_t    offset;
    size_t      fragment_length;
    qboolean    more_fragments;

    // Should we send a reliable message, or not?
    send_reliable = netChannel->reliableLength ? true : false;

    // Write the packet header
    w1 = (netChannel->outgoingSequence & 0x3FFFFFFF) | (1 << 30) |
        (send_reliable << 31);
    w2 = (netChannel->incomingSequence & 0x3FFFFFFF) | (0 << 30) |
        (netChannel->incomingReliableSequence << 31);

    SZ_TagInit(&send, send_buf, sizeof(send_buf), SZ_NC_SEND_FRG);

    SZ_WriteLong(&send, w1);
    SZ_WriteLong(&send, w2);

#if USE_CLIENT
    // Send the qport if we are a client
    if (netChannel->netSource == NS_CLIENT && netChannel->remoteQPort) {
        SZ_WriteByte(&send, netChannel->remoteQPort);
    }
#endif

    // Calculate Fragment length based on how much has been read so far.
    // Ensure we do not exceed the max packet length.
    fragment_length = netChannel->outFragment.currentSize - netChannel->outFragment.readCount;
    if (fragment_length > netChannel->maximumPacketLength) {
        fragment_length = netChannel->maximumPacketLength;
    }

    // More 
    more_fragments = true;
    if (netChannel->outFragment.readCount + fragment_length ==
        netChannel->outFragment.currentSize) {
        more_fragments = false;
    }

    // Write fragment offset
    offset = (netChannel->outFragment.readCount & 0x7FFF) |
        (more_fragments << 15);
    SZ_WriteShort(&send, offset);

    // Write fragment contents
    SZ_Write(&send, netChannel->outFragment.data + netChannel->outFragment.readCount,
        fragment_length);

    SHOWPACKET("send %4" PRIz " : s=%d ack=%d rack=%d "
        "fragment_offset=%" PRIz " more_fragments=%d",
        send.currentSize,
        netChannel->outgoingSequence,
        netChannel->incomingSequence,
        netChannel->incomingReliableSequence,
        netChannel->outFragment.readCount,
        more_fragments);
    if (send_reliable) {
        SHOWPACKET(" reliable=%i ", netChannel->reliableSequence);
    }
    SHOWPACKET("\n");

    // Increment read count with fragment length and store whether one more is pending or not.
    netChannel->outFragment.readCount += fragment_length;
    netChannel->fragmentPending = more_fragments;

    // If the message has been sent completely, clear the fragment buffer
    if (!netChannel->fragmentPending) {
        netChannel->outgoingSequence++;
        netChannel->lastSentTime = com_localTime;
        SZ_Clear(&netChannel->outFragment);
    }

    // Send the datagram
    NET_SendPacket(netChannel->netSource, send.data, send.currentSize,
        &netChannel->remoteNetAddress);

    return send.currentSize;
}

/*
===============
Netchan_Transmit
================
*/
size_t Netchan_Transmit(NetChannel* netChannel, size_t length, const void* data, int numpackets) {
    SizeBuffer   send;
    byte        send_buf[MAX_PACKETLEN];
    qboolean    send_reliable;
    uint32_t    w1, w2;
    int         i;

    // check for message overflow
    if (netChannel->message.overflowed) {
        netChannel->fatalError = true;
        Com_WPrintf("%s: outgoing message overflow\n",
            NET_AdrToString(&netChannel->remoteNetAddress));
        return 0;
    }

    if (netChannel->fragmentPending) {
        return Netchan_TransmitNextFragment(netChannel);
    }

    send_reliable = false;

    // if the remote side deltaFramePacketDrops the last reliable message, resend it
    if (netChannel->incomingAcknowledged > netChannel->lastReliableSequence &&
        netChannel->incomingReliableAcknowledged != netChannel->reliableSequence) {
        send_reliable = true;
    }

    // if the reliable transmit buffer is empty, copy the current message out
    if (!netChannel->reliableLength && netChannel->message.currentSize) {
        send_reliable = true;
        memcpy(netChannel->reliableBuffer, netChannel->messageBuffer,
            netChannel->message.currentSize);
        netChannel->reliableLength = netChannel->message.currentSize;
        netChannel->message.currentSize = 0;
        netChannel->reliableSequence ^= 1;
    }

    if (length > netChannel->maximumPacketLength || (send_reliable &&
                                           (netChannel->reliableLength + length > netChannel->maximumPacketLength))) {
        if (send_reliable) {
            netChannel->lastReliableSequence = netChannel->outgoingSequence;
            SZ_Write(&netChannel->outFragment, netChannel->reliableBuffer,
                netChannel->reliableLength);
        }
        // add the unreliable part if space is available
        if (netChannel->outFragment.maximumSize - netChannel->outFragment.currentSize >= length)
            SZ_Write(&netChannel->outFragment, data, length);
        else
            Com_WPrintf("%s: dumped unreliable\n",
                NET_AdrToString(&netChannel->remoteNetAddress));
        return Netchan_TransmitNextFragment(netChannel);
    }

    // write the packet header
    w1 = (netChannel->outgoingSequence & 0x3FFFFFFF) | (send_reliable << 31);
    w2 = (netChannel->incomingSequence & 0x3FFFFFFF) |
        (netChannel->incomingReliableSequence << 31);

    SZ_TagInit(&send, send_buf, sizeof(send_buf), SZ_NC_SEND_NEW);

    SZ_WriteLong(&send, w1);
    SZ_WriteLong(&send, w2);

#if USE_CLIENT
    // send the qport if we are a client
    if (netChannel->netSource == NS_CLIENT && netChannel->remoteQPort) {
        SZ_WriteByte(&send, netChannel->remoteQPort);
    }
#endif

    // copy the reliable message to the packet first
    if (send_reliable) {
        netChannel->lastReliableSequence = netChannel->outgoingSequence;
        SZ_Write(&send, netChannel->reliableBuffer, netChannel->reliableLength);
    }

    // add the unreliable part
    SZ_Write(&send, data, length);

    SHOWPACKET("send %4" PRIz " : s=%d ack=%d rack=%d",
        send.currentSize,
        netChannel->outgoingSequence,
        netChannel->incomingSequence,
        netChannel->incomingReliableSequence);
    if (send_reliable) {
        SHOWPACKET(" reliable=%d", netChannel->reliableSequence);
    }
    SHOWPACKET("\n");

    // send the datagram
    for (i = 0; i < numpackets; i++) {
        NET_SendPacket(netChannel->netSource, send.data, send.currentSize,
            &netChannel->remoteNetAddress);
    }

    netChannel->outgoingSequence++;
    netChannel->reliableAckPending = false;
    netChannel->lastSentTime = com_localTime;

    return send.currentSize * numpackets;
}

/*
=================
Netchan_Process
=================
*/
qboolean Netchan_Process(NetChannel* netChannel) {
    uint32_t    sequence, sequence_ack, reliable_ack;
    qboolean    reliable_message, fragmented_message, more_fragments;
    uint16_t    fragment_offset;
    size_t      length;

    // get sequence numbers
    MSG_BeginReading();
    sequence = MSG_ReadLong();
    sequence_ack = MSG_ReadLong();

    // read the qport if we are a server
#if USE_CLIENT
    if (netChannel->netSource == NS_SERVER)
#endif
        if (netChannel->remoteQPort) {
            MSG_ReadByte();
        }

    reliable_message = sequence >> 31;
    reliable_ack = sequence_ack >> 31;
    fragmented_message = (sequence >> 30) & 1;

    sequence &= 0x3FFFFFFF;
    sequence_ack &= 0x3FFFFFFF;

    fragment_offset = 0;
    more_fragments = false;
    if (fragmented_message) {
        fragment_offset = MSG_ReadShort();
        more_fragments = fragment_offset >> 15;
        fragment_offset &= 0x7FFF;
    }

    SHOWPACKET("recv %4" PRIz " : s=%d ack=%d rack=%d",
        msg_read.currentSize, sequence, sequence_ack, reliable_ack);
    if (fragmented_message) {
        SHOWPACKET(" fragment_offset=%d more_fragments=%d",
            fragment_offset, more_fragments);
    }
    if (reliable_message) {
        SHOWPACKET(" reliable=%d", netChannel->incomingReliableSequence ^ 1);
    }
    SHOWPACKET("\n");

    //
    // discard stale or duplicated packets
    //
    if (sequence <= netChannel->incomingSequence) {
        SHOWDROP("%s: out of order packet %i at %i\n",
            NET_AdrToString(&netChannel->remoteNetAddress),
            sequence, netChannel->incomingSequence);
        return false;
    }

    //
    // deltaFramePacketDrops packets don't keep the message from being used
    //
    netChannel->deltaFramePacketDrops = sequence - (netChannel->incomingSequence + 1);
    if (netChannel->deltaFramePacketDrops > 0) {
        SHOWDROP("%s: deltaFramePacketDrops %i packets at %i\n",
            NET_AdrToString(&netChannel->remoteNetAddress),
            netChannel->deltaFramePacketDrops, sequence);
    }

    //
    // if the current outgoing reliable message has been acknowledged
    // clear the buffer to make way for the next
    //
    netChannel->incomingReliableAcknowledged = reliable_ack;
    if (reliable_ack == netChannel->reliableSequence) {
        netChannel->reliableLength = 0;   // it has been received
    }


    //
    // parse fragment header, if any
    //
    if (fragmented_message) {
        if (netChannel->fragmentSequence != sequence) {
            // start new receive sequence
            netChannel->fragmentSequence = sequence;
            SZ_Clear(&netChannel->inFragment);
        }

        if (fragment_offset < netChannel->inFragment.currentSize) {
            SHOWDROP("%s: out of order fragment at %i\n",
                NET_AdrToString(&netChannel->remoteNetAddress), sequence);
            return false;
        }

        if (fragment_offset > netChannel->inFragment.currentSize) {
            SHOWDROP("%s: deltaFramePacketDrops fragment(s) at %i\n",
                NET_AdrToString(&netChannel->remoteNetAddress), sequence);
            return false;
        }

        length = msg_read.currentSize - msg_read.readCount;
        if (netChannel->inFragment.currentSize + length > netChannel->inFragment.maximumSize) {
            SHOWDROP("%s: oversize fragment at %i\n",
                NET_AdrToString(&netChannel->remoteNetAddress), sequence);
            return false;
        }

        SZ_Write(&netChannel->inFragment, msg_read.data +
            msg_read.readCount, length);
        if (more_fragments) {
            return false;
        }

        // message has been sucessfully assembled
        SZ_Clear(&msg_read);
        SZ_Write(&msg_read, netChannel->inFragment.data,
            netChannel->inFragment.currentSize);
        SZ_Clear(&netChannel->inFragment);
    }

    netChannel->incomingSequence = sequence;
    netChannel->incomingAcknowledged = sequence_ack;

    //
    // if this message contains a reliable message, bump incomingReliableSequence
    //
    if (reliable_message) {
        netChannel->reliableAckPending = true;
        netChannel->incomingReliableSequence ^= 1;
    }

    //
    // the message can now be read from the current message pointer
    //
    netChannel->lastReceivedTime = com_localTime;

    netChannel->totalDropped += netChannel->deltaFramePacketDrops;
    netChannel->totalReceived += netChannel->deltaFramePacketDrops + 1;

    return true;
}

/*
==============
Netchan_ShouldUpdate
==============
*/
qboolean Netchan_ShouldUpdate(NetChannel* netChannel) {
    NetChannel* channel = (NetChannel*)netChannel;

    if (netChannel->message.currentSize ||
        netChannel->reliableAckPending ||
        channel->outFragment.currentSize ||
        com_localTime - netChannel->lastSentTime > 1000) {
        return true;
    }

    return false;
}

/*
==============
Netchan_Setup
==============
*/
NetChannel* ENetchan_Setup(NetSource netSource, const std::string &host, int32_t qport, size_t maxPacketLength, int32_t protocolMajorVersion) {
    NetChannel* netChannel;

    clamp(maxPacketLength, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);

    // Temporarily still here so the OG code can still work.
    // For now we want to test ENet itself only for OOB.
    netadr_t netadr;
    NET_StringToAdr(host.c_str(), &netadr, 27910);

    netChannel = (NetChannel*)Z_TagMallocz(sizeof(*netChannel), // CPP: Cast
        netSource == NS_SERVER ? TAG_SERVER : TAG_GENERAL);
    netChannel->netSource = netSource;
    netChannel->remoteNetAddress = netadr;
    netChannel->remoteQPort = qport;
    netChannel->maximumPacketLength = maxPacketLength;
    netChannel->lastReceivedTime = com_localTime;
    netChannel->lastSentTime = com_localTime;
    netChannel->incomingSequence = 0;
    netChannel->outgoingSequence = 1;

    // Create our netChannel host.
    ENetAddress eHostAddress = ENetAddress{
        .host = ENET_HOST_ANY,
        .port = ENET_PORT_ANY
    };
    netChannel->eHost = enet_host_create(&eHostAddress,
        32, 4, 0, 0);

    // Generate the connect address struct.
    ENetAddress eConnectAddress = {};

    // First try general host IPs.
    if (enet_address_set_host_new(&eConnectAddress, host.c_str()) < 0) {
        // If that fails, we try general host URLs.
        if (enet_address_set_host_ip_new(&eConnectAddress, host.c_str()) < 0) {
            Com_Error(ERR_DROP, "NetChannel[%s] failed to connect to host address: %s\n", host.c_str());
        }
    }
    
    netChannel->ePeer = enet_host_connect(netChannel->eHost, &eConnectAddress, 4, 0);

    // Run the host for 33ms (About a frame.)
    SZ_Init(&netChannel->message, netChannel->messageBuffer,
        sizeof(netChannel->messageBuffer));
    SZ_TagInit(&netChannel->inFragment, netChannel->inFragmentBuffer,
        sizeof(netChannel->inFragmentBuffer), SZ_NC_FRG_IN);
    SZ_TagInit(&netChannel->outFragment, netChannel->outFragmentBuffer,
        sizeof(netChannel->outFragmentBuffer), SZ_NC_FRG_OUT);

    netChannel->protocolMajorVersion = protocolMajorVersion;

    return netChannel;

}

NetChannel* Netchan_Setup(NetSource netSource, const netadr_t* adr, int qport, size_t maximumPacketLength, int protocol) {
    NetChannel* netChannel;

    clamp(maximumPacketLength, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);

    // Temporarily still here so the OG code can still work.
    // For now we want to test ENet itself only for OOB.
    netChannel = (NetChannel*)Z_TagMallocz(sizeof(*netChannel), // CPP: Cast
        netSource == NS_SERVER ? TAG_SERVER : TAG_GENERAL);
    netChannel->netSource = netSource;
    netChannel->remoteNetAddress = *adr;
    netChannel->remoteQPort = qport;
    netChannel->maximumPacketLength = maximumPacketLength;
    netChannel->lastReceivedTime = com_localTime;
    netChannel->lastSentTime = com_localTime;
    netChannel->incomingSequence = 0;
    netChannel->outgoingSequence = 1;

    // Run the host for 33ms (About a frame.)
    SZ_Init(&netChannel->message, netChannel->messageBuffer,
        sizeof(netChannel->messageBuffer));
    SZ_TagInit(&netChannel->inFragment, netChannel->inFragmentBuffer,
        sizeof(netChannel->inFragmentBuffer), SZ_NC_FRG_IN);
    SZ_TagInit(&netChannel->outFragment, netChannel->outFragmentBuffer,
        sizeof(netChannel->outFragmentBuffer), SZ_NC_FRG_OUT);

    netChannel->protocolMajorVersion = protocol;

    return netChannel;
}

/*
==============
Netchan_Close
==============
*/
void Netchan_Close(NetChannel* netChannel) {
    Z_Free(netChannel);
}

