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

#include "../../Shared/Shared.h"
#include "../Common.h"
#include "../CVar.h"
#include "../Messaging.h"
#include "NetChan.h"
#include "Net.h"
#include "../Protocol.h"
#include "../SizeBuffer.h"
#include "../Zone.h"
#include "../../System/System.h"

/*

packet header
-------------
31  sequence
1   does this message contain a reliable payload
31  acknowledge sequence
1   acknowledge receipt of even/odd message
16  qport

The remote connection never knows if it missed a reliable message, the
local side detects that it has been dropped by seeing a sequence acknowledge
higher thatn the last reliable sequence, but without the correct evon/odd
bit for the reliable set.

If the sender notices that a reliable message has been dropped, it will be
retransmitted.  It will not be retransmitted again until a message after
the retransmit has been acknowledged and the reliable still failed to get theref.

if the sequence number is -1, the packet should be handled without a netcon

The reliable message can be added to at any time by doing
MSG_Write* (&netchan->message, <data>).

If the message buffer is overflowed, either by a single message, or by
multiple frames worth piling up while the last reliable transmit goes
unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable
message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable
parts of the message, they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be dropped, but do
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
static cvar_t       *showpackets;
static cvar_t       *showdrop;
#define SHOWPACKET(...) \
    if (showpackets->integer) \
        Com_LPrintf(PrintType::Developer, __VA_ARGS__)
#define SHOWDROP(...) \
    if (showdrop->integer) \
        Com_LPrintf(PrintType::Developer, __VA_ARGS__)
#else
#define SHOWPACKET(...)
#define SHOWDROP(...)
#endif

cvar_t      *net_qport;
cvar_t      *net_maxmsglen;
cvar_t      *net_chantype;

// allow either 0 (no hard limit), or an integer between 512 and 4086
static void net_maxmsglen_changed(cvar_t *self)
{
    if (self->integer) {
        Cvar_ClampInteger(self, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);
    }
}

/*
===============
Netchan_Init

===============
*/
void Netchan_Init(void)
{
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
void Netchan_OutOfBand(NetSource sock, const NetAdr *address,
                       const char *format, ...)
{
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
size_t Netchan_TransmitNextFragment(NetChannel *netchan)
{
    SizeBuffer   send;
    byte        send_buf[MAX_PACKETLEN];
    qboolean    send_reliable;
    uint32_t    w1, w2;
    uint16_t    offset;
    size_t      fragment_length;
    qboolean    more_fragments;

    // Should we send a reliable message, or not?
    send_reliable = netchan->reliableLength ? true : false;

    // Write the packet header
    w1 = (netchan->outgoingSequence & 0x3FFFFFFF) | (1 << 30) |
         (send_reliable << 31);
    w2 = (netchan->incomingSequence & 0x3FFFFFFF) | (0 << 30) |
         (netchan->incomingReliableSequence << 31);

    SZ_TagInit(&send, send_buf, sizeof(send_buf), SZ_NC_SEND_FRG);

    SZ_WriteLong(&send, w1);
    SZ_WriteLong(&send, w2);

#if USE_CLIENT
    // Send the qport if we are a client
    if (netchan->netSource == NS_CLIENT && netchan->remoteQPort) {
        SZ_WriteByte(&send, netchan->remoteQPort);
    }
#endif

    // Calculate Fragment length based on how much has been read so far.
    // Ensure we do not exceed the max packet length.
    fragment_length = netchan->outFragment.currentSize - netchan->outFragment.readCount;
    if (fragment_length > netchan->maximumPacketLength) {
        fragment_length = netchan->maximumPacketLength;
    }

    // More 
    more_fragments = true;
    if (netchan->outFragment.readCount + fragment_length ==
        netchan->outFragment.currentSize) {
        more_fragments = false;
    }

    // Write fragment offset
    offset = (netchan->outFragment.readCount & 0x7FFF) |
             (more_fragments << 15);
    SZ_WriteShort(&send, offset);

    // Write fragment contents
    SZ_Write(&send, netchan->outFragment.data + netchan->outFragment.readCount,
             fragment_length);

    SHOWPACKET("send %4" PRIz " : s=%d ack=%d rack=%d "
               "fragment_offset=%" PRIz " more_fragments=%d",
               send.currentSize,
               netchan->outgoingSequence,
               netchan->incomingSequence,
               netchan->incomingReliableSequence,
               netchan->outFragment.readCount,
               more_fragments);
    if (send_reliable) {
        SHOWPACKET(" reliable=%i ", netchan->reliableSequence);
    }
    SHOWPACKET("\n");

    // Increment read count with fragment length and store whether one more is pending or not.
    netchan->outFragment.readCount += fragment_length;
    netchan->fragmentPending = more_fragments;

    // If the message has been sent completely, clear the fragment buffer
    if (!netchan->fragmentPending) {
        netchan->outgoingSequence++;
        netchan->lastSentTime = com_localTime;
        SZ_Clear(&netchan->outFragment);
    }

    // Send the datagram
    NET_SendPacket(netchan->netSource, send.data, send.currentSize,
                   &netchan->remoteNetAddress);

    return send.currentSize;
}

/*
===============
Netchan_Transmit
================
*/
size_t Netchan_Transmit(NetChannel *netchan, size_t length, const void *data, int numpackets)
{
    SizeBuffer   send;
    byte        send_buf[MAX_PACKETLEN];
    qboolean    send_reliable;
    uint32_t    w1, w2;
    int         i;

// check for message overflow
    if (netchan->message.overflowed) {
        netchan->fatalError = true;
        Com_WPrintf("%s: outgoing message overflow\n",
                    NET_AdrToString(&netchan->remoteNetAddress));
        return 0;
    }

    if (netchan->fragmentPending) {
        return Netchan_TransmitNextFragment(netchan);
    }

    send_reliable = false;

// if the remote side dropped the last reliable message, resend it
    if (netchan->incomingAcknowledged > netchan->lastReliableSequence &&
        netchan->incomingReliableAcknowledged != netchan->reliableSequence) {
        send_reliable = true;
    }

// if the reliable transmit buffer is empty, copy the current message out
    if (!netchan->reliableLength && netchan->message.currentSize) {
        send_reliable = true;
        memcpy(netchan->reliableBuffer, netchan->messageBuffer,
               netchan->message.currentSize);
        netchan->reliableLength = netchan->message.currentSize;
        netchan->message.currentSize = 0;
        netchan->reliableSequence ^= 1;
    }

    if (length > netchan->maximumPacketLength || (send_reliable &&
                                           (netchan->reliableLength + length > netchan->maximumPacketLength))) {
        if (send_reliable) {
            netchan->lastReliableSequence = netchan->outgoingSequence;
            SZ_Write(&netchan->outFragment, netchan->reliableBuffer,
                     netchan->reliableLength);
        }
        // add the unreliable part if space is available
        if (netchan->outFragment.maximumSize - netchan->outFragment.currentSize >= length)
            SZ_Write(&netchan->outFragment, data, length);
        else
            Com_WPrintf("%s: dumped unreliable\n",
                        NET_AdrToString(&netchan->remoteNetAddress));
        return Netchan_TransmitNextFragment(netchan);
    }

// write the packet header
    w1 = (netchan->outgoingSequence & 0x3FFFFFFF) | (send_reliable << 31);
    w2 = (netchan->incomingSequence & 0x3FFFFFFF) |
         (netchan->incomingReliableSequence << 31);

    SZ_TagInit(&send, send_buf, sizeof(send_buf), SZ_NC_SEND_NEW);

    SZ_WriteLong(&send, w1);
    SZ_WriteLong(&send, w2);

#if USE_CLIENT
    // send the qport if we are a client
    if (netchan->netSource == NS_CLIENT && netchan->remoteQPort) {
        SZ_WriteByte(&send, netchan->remoteQPort);
    }
#endif

    // copy the reliable message to the packet first
    if (send_reliable) {
        netchan->lastReliableSequence = netchan->outgoingSequence;
        SZ_Write(&send, netchan->reliableBuffer, netchan->reliableLength);
    }

    // add the unreliable part
    SZ_Write(&send, data, length);

    SHOWPACKET("send %4" PRIz " : s=%d ack=%d rack=%d",
               send.currentSize,
               netchan->outgoingSequence,
               netchan->incomingSequence,
        netchan->incomingReliableSequence);
    if (send_reliable) {
        SHOWPACKET(" reliable=%d", netchan->reliableSequence);
    }
    SHOWPACKET("\n");

    // send the datagram
    for (i = 0; i < numpackets; i++) {
        NET_SendPacket(netchan->netSource, send.data, send.currentSize,
                       &netchan->remoteNetAddress);
    }

    netchan->outgoingSequence++;
    netchan->reliableAckPending = false;
    netchan->lastSentTime = com_localTime;

    return send.currentSize * numpackets;
}

/*
=================
Netchan_Process
=================
*/
qboolean Netchan_Process(NetChannel *netchan)
{
    uint32_t    sequence, sequence_ack, reliable_ack;
    qboolean    reliable_message, fragmented_message, more_fragments;
    uint16_t    fragment_offset;
    size_t      length;

    // get sequence numbers
    MSG_BeginReading();
    sequence = static_cast<uint32_t>(MSG_ReadInt32()); // MSG_ReadLong
    sequence_ack = static_cast<uint32_t>(MSG_ReadInt32()); // MSG_ReadLong

    // read the qport if we are a server
#if USE_CLIENT
    if (netchan->netSource == NS_SERVER) {
#endif
        if (netchan->remoteQPort) {
            MSG_ReadUint8();//MSG_ReadByte();
        }
#if USE_CLIENT
    }
#endif

    reliable_message = sequence >> 31;
    reliable_ack = sequence_ack >> 31;
    fragmented_message = (sequence >> 30) & 1;

    sequence &= 0x3FFFFFFF;
    sequence_ack &= 0x3FFFFFFF;

    fragment_offset = 0;
    more_fragments = false;
    if (fragmented_message) {
        fragment_offset = MSG_ReadUint16();//MSG_ReadShort();
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
        SHOWPACKET(" reliable=%d", netchan->incomingReliableSequence ^ 1);
    }
    SHOWPACKET("\n");

//
// discard stale or duplicated packets
//
    if (sequence <= netchan->incomingSequence) {
        SHOWDROP("%s: out of order packet %i at %i\n",
                 NET_AdrToString(&netchan->remoteNetAddress),
                 sequence, netchan->incomingSequence);
        return false;
    }

//
// dropped packets don't keep the message from being used
//
    netchan->deltaFramePacketDrops = sequence - (netchan->incomingSequence + 1);
    if (netchan->deltaFramePacketDrops > 0) {
        SHOWDROP("%s: dropped %i packets at %i\n",
                 NET_AdrToString(&netchan->remoteNetAddress),
                 netchan->deltaFramePacketDrops, sequence);
    }

//
// if the current outgoing reliable message has been acknowledged
// clear the buffer to make way for the next
//
    netchan->incomingReliableAcknowledged = reliable_ack;
    if (reliable_ack == netchan->reliableSequence) {
        netchan->reliableLength = 0;   // it has been received
    }


//
// parse fragment header, if any
//
    if (fragmented_message) {
        if (netchan->fragmentSequence != sequence) {
            // start new receive sequence
            netchan->fragmentSequence = sequence;
            SZ_Clear(&netchan->inFragment);
        }

        if (fragment_offset < netchan->inFragment.currentSize) {
            SHOWDROP("%s: out of order fragment at %i\n",
                     NET_AdrToString(&netchan->remoteNetAddress), sequence);
            return false;
        }

        if (fragment_offset > netchan->inFragment.currentSize) {
            SHOWDROP("%s: dropped fragment(s) at %i\n",
                     NET_AdrToString(&netchan->remoteNetAddress), sequence);
            return false;
        }

        length = msg_read.currentSize - msg_read.readCount;
        if (netchan->inFragment.currentSize + length > netchan->inFragment.maximumSize) {
            SHOWDROP("%s: oversize fragment at %i\n",
                     NET_AdrToString(&netchan->remoteNetAddress), sequence);
            return false;
        }

        SZ_Write(&netchan->inFragment, msg_read.data +
                 msg_read.readCount, length);
        if (more_fragments) {
            return false;
        }

        // message has been sucessfully assembled
        SZ_Clear(&msg_read);
        SZ_Write(&msg_read, netchan->inFragment.data,
            netchan->inFragment.currentSize);
        SZ_Clear(&netchan->inFragment);
    }

    netchan->incomingSequence = sequence;
    netchan->incomingAcknowledged = sequence_ack;

//
// if this message contains a reliable message, bump incomingReliableSequence
//
    if (reliable_message) {
        netchan->reliableAckPending = true;
        netchan->incomingReliableSequence ^= 1;
    }

//
// the message can now be read from the current message pointer
//
    netchan->lastReceivedTime = com_localTime;

    netchan->totalDropped += netchan->deltaFramePacketDrops;
    netchan->totalReceived += netchan->deltaFramePacketDrops + 1;

    return true;
}

/*
==============
Netchan_ShouldUpdate
==============
*/
qboolean Netchan_ShouldUpdate(NetChannel *netchan)
{
    NetChannel *chan = (NetChannel *)netchan;

    if (netchan->message.currentSize ||
        netchan->reliableAckPending ||
        chan->outFragment.currentSize ||
        com_localTime - netchan->lastSentTime > 1000) {
        return true;
    }

    return false;
}

/*
==============
Netchan_Setup
==============
*/
NetChannel *Netchan_Setup(NetSource sock, const NetAdr *adr, int qport, size_t maxPacketLength, int protocol)
{
    NetChannel *netchan;

    clamp(maxPacketLength, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);

    netchan = (NetChannel*)Z_TagMallocz(sizeof(*netchan), // CPP: Cast
        sock == NS_SERVER ? TAG_SERVER : TAG_GENERAL);
    netchan->netSource = sock;
    netchan->remoteNetAddress = *adr;
    netchan->remoteQPort = qport;
    netchan->maximumPacketLength = maxPacketLength;
    netchan->lastReceivedTime = com_localTime;
    netchan->lastSentTime = com_localTime;
    netchan->incomingSequence = 0;
    netchan->outgoingSequence = 1;

    SZ_Init(&netchan->message, netchan->messageBuffer,
        sizeof(netchan->messageBuffer));
    SZ_TagInit(&netchan->inFragment, netchan->inFragmentBuffer,
        sizeof(netchan->inFragmentBuffer), SZ_NC_FRG_IN);
    SZ_TagInit(&netchan->outFragment, netchan->outFragmentBuffer,
        sizeof(netchan->outFragmentBuffer), SZ_NC_FRG_OUT);

    netchan->protocolVersion = protocol;

    return netchan;
}

/*
==============
Netchan_Close
==============
*/
void Netchan_Close(NetChannel *netchan)
{
    Z_Free(netchan);
}

