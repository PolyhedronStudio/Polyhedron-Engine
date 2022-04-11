/***
*
*	License here.
*
*	@file
*
*	Network Channels:
*	
*	A network channel handles packet compression and decompression, packet fragmentation and
*	reassembly, and out of order / duplicate suppression.

*	Packet header:
*	--------------
*	4		Outgoing sequence (FRAGMENT_BIT will be set if this is a fragmented message)
*	2		Uncompressed message size
*	2		Channel port (only for client to server)
*	2		Fragment offset (only if this is a fragmented message)
*	2		Fragment size (if < FRAGMENT_SIZE, this is the last fragment)
*	
*	If the sequence number is OOB_SEQUENCE, the packet should be handled as an out-of-band message
*	instead of as part of a network connection.
*	
*	All fragments will have the same sequence numbers.
*	
*	The channel port field is a workaround for bad address translating routers that sometimes remap
*	the client's source port on a packet during gameplay.
*	If the base part of the network address matches and the channel port matches, then the channel
*	matches even if the IP port differs. The IP port should be updated to the new value before sending
*	out any replies.
*
***/
#include "../../Shared/Shared.h"
#include "../Common.h"
#include "../CVar.h"
#include "../Msg.h"
#include "NetChan.h"
#include "Net.h"
#include "../Protocol.h"
#include "../SizeBuffer.h"
#include "../Zone.h"
#include "../../System/System.h"



/**
*	NetChannel Debug Output Methods.
**/
#ifdef _DEBUG
static cvar_t       *showpackets = nullptr;
static cvar_t       *showdrop = nullptr;

// Shows packet debug information if showpackets cvar is enabled.
static inline void DShowPacket(const char *fmt, ...) {
	if (showpackets && showpackets->integer) {
		char buffer[MAX_STRING_CHARS];
		va_list args;
		va_start (args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		Com_LPrintf(PrintType::Developer, "%s", buffer);
		va_end (args);
	}
}

// Shows packet drop debug information if showpackets cvar is enabled.
static inline void DShowDrop(const char *fmt, ...) {
	if (showpackets && showpackets->integer) {
		char buffer[MAX_STRING_CHARS];
		va_list args;
		va_start (args, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		Com_LPrintf(PrintType::Developer, "%s", buffer);
		va_end (args);
	}
}
#else
#define DShowPacket(...)
#define DShowDrop(...)
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

/**
*	@brief	Initializes the network channel subsystem.
**/
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

/**
*	@brief	Opens a channel to a remote system. (Or localhost.)
**/
NetChannel *Netchan_Setup(NetSource sock, const NetAdr *adr, int32_t qport, size_t maxPacketLength, int32_t protocol) {
    NetChannel *netChannel;

    clamp(maxPacketLength, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);

    netChannel = (NetChannel*)Z_TagMallocz(sizeof(*netChannel), // CPP: Cast
        sock == NS_SERVER ? TAG_SERVER : TAG_GENERAL);
    netChannel->netSource = sock;
    netChannel->remoteNetAddress = *adr;
    netChannel->remoteQPort = qport;
    netChannel->maximumPacketLength = maxPacketLength;
    netChannel->lastReceivedTime = com_localTime;
    netChannel->lastSentTime = com_localTime;
    netChannel->incomingSequence = 0;
    netChannel->outgoingSequence = 1;

    SZ_Init(&netChannel->message, netChannel->messageBuffer,
        sizeof(netChannel->messageBuffer));
    SZ_TagInit(&netChannel->inFragment, netChannel->inFragmentBuffer,
        sizeof(netChannel->inFragmentBuffer), SZ_NC_FRG_IN);
    SZ_TagInit(&netChannel->outFragment, netChannel->outFragmentBuffer,
        sizeof(netChannel->outFragmentBuffer), SZ_NC_FRG_OUT);

    netChannel->protocolVersion = protocol;

    return netChannel;
}

/**
*	@brief	Closes the network channel.
**/
void Netchan_Close(NetChannel *netChannel) {
    Z_Free(netChannel);
}

/**
*	@brief	Sends a text message in an out-of-band datagram
**/
void Netchan_OutOfBand(NetSource sock, const NetAdr *address, const char *format, ...) {
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

/**
*	@brief	Sends a message to a connection, fragmenting if necessary. 
*			A zero sized message will still generate a packet.
**/
size_t Netchan_Transmit(NetChannel *netChannel, size_t length, const void *data, int32_t numberOfPackets, uint64_t time)
{
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

// If we got any fragments pending, send them out.
    if (netChannel->fragmentPending) {
        return Netchan_TransmitNextFragment(netChannel, time);
    }

    send_reliable = false;

// If the remote side dropped the last reliable message, resend it.
    if (netChannel->incomingAcknowledged > netChannel->lastReliableSequence &&
        netChannel->incomingReliableAcknowledged != netChannel->reliableSequence) {
        send_reliable = true;
    }

// If the reliable transmit buffer is empty, copy the current message out.
    if (!netChannel->reliableLength && netChannel->message.currentSize) {
        send_reliable = true;
        memcpy(netChannel->reliableBuffer, netChannel->messageBuffer,
               netChannel->message.currentSize);
        netChannel->reliableLength = netChannel->message.currentSize;
        netChannel->message.currentSize = 0;
        netChannel->reliableSequence ^= 1;
    }

// If our length exceeds maximum packet length, or we are about to send a reliable which exceeds MTU size,
// separate and send the packet in multiplate fragments.
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
        return Netchan_TransmitNextFragment(netChannel, time);
    }

// Write the packet header
    w1 = (netChannel->outgoingSequence & 0x3FFFFFFF) | (send_reliable << 31);
    w2 = (netChannel->incomingSequence & 0x3FFFFFFF) |
         (netChannel->incomingReliableSequence << 31);

// Tag init our Send SizeBuffer.
    SZ_TagInit(&send, send_buf, sizeof(send_buf), SZ_NC_SEND_NEW);

// Write packet header.
    SZ_WriteLong(&send, w1);
    SZ_WriteLong(&send, w2);

#if USE_CLIENT
    // If we are a client, send the 'QPort'.
    if (netChannel->netSource == NS_CLIENT && netChannel->remoteQPort) {
        SZ_WriteByte(&send, netChannel->remoteQPort);
    }
#endif

// Copy the reliable message to the packet first
    if (send_reliable) {
        netChannel->lastReliableSequence = netChannel->outgoingSequence;
        SZ_Write(&send, netChannel->reliableBuffer, netChannel->reliableLength);
    }

// Add the unreliable part to our send sizebuffer.
    SZ_Write(&send, data, length);

    DShowPacket("send %4" PRIz " : s=%d ack=%d rack=%d", send.currentSize, netChannel->outgoingSequence, netChannel->incomingSequence, netChannel->incomingReliableSequence);
    if (send_reliable) {
        DShowPacket(" reliable=%d", netChannel->reliableSequence);
    }
    DShowPacket("\n");

// Send the datagram
    for (i = 0; i < numberOfPackets; i++) {
        NET_SendPacket(netChannel->netSource, send.data, send.currentSize,
                       &netChannel->remoteNetAddress);
    }

    netChannel->outgoingSequence++;
    netChannel->reliableAckPending = false;
    netChannel->lastSentTime = com_localTime;

    return send.currentSize * numberOfPackets;
}

/**
*	@brief	Sends one fragment of the current message.
**/
size_t Netchan_TransmitNextFragment(NetChannel *netChannel, uint64_t time) {
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

	// Debug printing.
    DShowPacket("send %4" PRIz " : s=%d ack=%d rack=%d " "fragment_offset=%" PRIz " more_fragments=%d",
               send.currentSize,
               netChannel->outgoingSequence,
               netChannel->incomingSequence,
               netChannel->incomingReliableSequence,
               netChannel->outFragment.readCount,
               more_fragments
	);
    if (send_reliable) {
        DShowPacket(" reliable=%i ", netChannel->reliableSequence);
    }
    DShowPacket("\n");

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

/**
*	@brief	Sends the remaining fragments of the current message
**/
size_t Netchan_TransmitAllFragments(NetChannel* netChannel, uint64_t time) {
return 0;
}


/**
*	@brief	Receives a message from a connection.
*			The msg parameter must be large enough to hold MAX_MSG_SIZE bytes of data, because if this is
*			the final fragment of a multi-part message, the entire thing will be copied out.
*	@return	Returns false if the message should not be processed due to being out of order or a fragment.
**/
qboolean Netchan_Process(NetChannel *netChannel) {
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
    if (netChannel->netSource == NS_SERVER) {
#endif
        if (netChannel->remoteQPort) {
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

    DShowPacket("recv %4" PRIz " : s=%d ack=%d rack=%d",
               msg_read.currentSize, sequence, sequence_ack, reliable_ack);
    if (fragmented_message) {
        DShowPacket(" fragment_offset=%d more_fragments=%d",
                   fragment_offset, more_fragments);
    }
    if (reliable_message) {
        DShowPacket(" reliable=%d", netChannel->incomingReliableSequence ^ 1);
    }
    DShowPacket("\n");

//
// discard stale or duplicated packets
//
    if (sequence <= netChannel->incomingSequence) {
        DShowDrop("%s: out of order packet %i at %i\n",
                 NET_AdrToString(&netChannel->remoteNetAddress),
                 sequence, netChannel->incomingSequence);
        return false;
    }

//
// dropped packets don't keep the message from being used
//
    netChannel->deltaFramePacketDrops = sequence - (netChannel->incomingSequence + 1);
    if (netChannel->deltaFramePacketDrops > 0) {
        DShowDrop("%s: dropped %i packets at %i\n",
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
            DShowDrop("%s: out of order fragment at %i\n",
                     NET_AdrToString(&netChannel->remoteNetAddress), sequence);
            return false;
        }

        if (fragment_offset > netChannel->inFragment.currentSize) {
            DShowDrop("%s: dropped fragment(s) at %i\n",
                     NET_AdrToString(&netChannel->remoteNetAddress), sequence);
            return false;
        }

        length = msg_read.currentSize - msg_read.readCount;
        if (netChannel->inFragment.currentSize + length > netChannel->inFragment.maximumSize) {
            DShowDrop("%s: oversize fragment at %i\n",
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

/**
*	@return	True in case the channel should update.
**/
qboolean Netchan_ShouldUpdate(NetChannel *netChannel)
{
    NetChannel *chan = (NetChannel *)netChannel;

    if (netChannel->message.currentSize ||
        netChannel->reliableAckPending ||
        chan->outFragment.currentSize ||
        com_localTime - netChannel->lastSentTime > 1000) {
        return true;
    }

    return false;
}
