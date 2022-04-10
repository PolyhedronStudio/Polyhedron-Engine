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
#include "../Huffman.h"
#include "../CVar.h"
#include "../Msg.h"
#include "NetChan.h"
#include "Net.h"
#include "../Protocol.h"
#include "../SizeBuffer.h"
#include "../Zone.h"
#include "../../System/System.h"

// For textual display.
static const char *	net_sourceString[2] = {"client", "server"};

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
        //Cvar_ClampInteger(self, MIN_PACKETLEN, MAX_PACKETLEN_WRITABLE);
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
    net_maxmsglen = Cvar_Get("net_maxmsglen", va("%d", CLIENT_MAX_PACKET_LENGTH_WRITABLE_DEFAULT), 0);
    net_maxmsglen->changed = net_maxmsglen_changed;
    net_chantype = Cvar_Get("net_chantype", "1", 0);
}

/**
*	@brief	Opens a channel to a remote system. (Or localhost.)
**/
NetChannel *Netchan_Setup(NetSource sock, const NetAdr *adr, int32_t qport, size_t maxPacketLength, int32_t protocol) {
	if (sock == NetSource::NS_CLIENT) {
		clamp(maxPacketLength, CLIENT_MIN_PACKET_LENGTH, CLIENT_MAX_PACKET_LENGTH_WRITABLE);
	} else if (sock == NetSource::NS_SERVER) {
		clamp(maxPacketLength, SERVER_MIN_PACKET_LENGTH, SERVER_MAX_PACKET_LENGTH_WRITABLE);
	}

    //netChannel = (NetChannel*)Z_TagMallocz(sizeof(*netChannel), // CPP: Cast
    //    sock == NS_SERVER ? TAG_SERVER : TAG_GENERAL);
	NetChannel *netChannel = new NetChannel;
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
    SZ_TagInit(&netChannel->inFragment, netChannel->inFragmentBuffer, sizeof(netChannel->inFragmentBuffer), SZ_NC_FRG_IN);
    SZ_TagInit(&netChannel->outFragment, netChannel->outFragmentBuffer, sizeof(netChannel->outFragmentBuffer), SZ_NC_FRG_OUT);

    netChannel->protocolVersion = protocol;

    return netChannel;
}

/**
*	@brief	Closes the network channel.
**/
void Netchan_Close(NetChannel *netChannel) {
	if (netChannel) {
		delete netChannel;
		netChannel = nullptr;
	}
}

/**
*	@brief	Sends a text message in an out-of-band datagram
**/
void Netchan_OutOfBand(NetSource sock, const NetAdr *address, const char *format, ...) {
    va_list     argptr;
    struct {
        uint32_t    header;
        char        data[MAX_PACKET_LENGTH_DEFAULT - 4];
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
size_t Netchan_Transmit(NetChannel *netChannel, uint64_t time, SizeBuffer &message) {
    SizeBuffer   send;
    byte        buffer[MAX_MSGLEN];
    byte        data[MAX_PACKET_LENGTH_DEFAULT];
    size_t		size = 0;


	// Check for any outgoing fragments, drop error if we got some. (There should be none.)
	if (netChannel->outgoingFragments) {
		Com_Error(ErrorType::Drop, "NetChan_Transmit: unsent outgoing fragments\n");
	}

	// Compress the message data.
	size = Huff_Compress(message.data, buffer, message.currentSize);

	// Show huffman debug info.
	// if (net_showhuffman->integer)
	// Com_LPrintf(PrintType::Developer, "%s compress: %4i -> %4i\n", net_sourceString[netChannel->sock], msg.GetWriteBytes(), size);

	// Fragment large messages.
	if (size >= FRAGMENT_SIZE) {
		netChannel->outgoingFragments		= true;
		netChannel->outgoingFragmentBytes	= message.currentSize;
		netChannel->outgoingFragmentSize	= size;
		memcpy(netChannel->outgoingFragmentBuffer, buffer, size);

		// Send the first fragment and exit function.
		return Netchan_TransmitNextFragment(netChannel, time);
	}

	// Initialize the message buffer.
	SZ_Init(&send, data, sizeof(data));

	// Write sequence number.
	SZ_WriteLong(&send, netChannel->outgoingSequence);

	// Write uncompressed message size.
	SZ_WriteShort(&send, message.currentSize);

	// Write the channel port in case we are a client.
#if USE_CLIENT
    if (netChannel->netSource == NS_CLIENT && netChannel->remoteQPort) {
        SZ_WriteByte(&send, netChannel->remoteQPort);
    }
#endif

	// Write the compressed message data.
	SZ_Write(&send, buffer, size);

	// Send the datagram.
	//if (netChannel->remoteNetAddress.type != NetAddressType::NA_LOOPBACK) {
	NET_SendPacket(netChannel->netSource, send.data, send.currentSize, &netChannel->remoteNetAddress);
	//} else {
	//}

	// Show debug packet info.
	DShowPacket("%s send %4i: seq=%i ack=%i\n", net_sourceString[netChannel->netSource], send.currentSize, netChannel->outgoingSequence, netChannel->incomingSequence);

	// Increase the outgoing sequence number.
	netChannel->outgoingSequence++;

	// Update rate variables.
	if (time - netChannel->outgoingRateTime > 1000) {
		netChannel->outgoingRateBytes -= netChannel->outgoingFragmentBytes * (time - netChannel->outgoingRateTime - 1000) / 1000;
		if (netChannel->outgoingRateBytes < 0) {
			netChannel->outgoingRateBytes = 0;
		}
	}

	netChannel->outgoingRateTime = time - 1000;
	netChannel->outgoingRateBytes += send.currentSize;

	// Return size.
	return send.currentSize;
}

/**
*	@brief	Sends one fragment of the current message.
**/
size_t Netchan_TransmitNextFragment(NetChannel *netChannel, uint64_t time) {
    SizeBuffer   send;
    byte        data[MAX_PACKET_LENGTH_DEFAULT];
    int32_t fragmentSize = 0;


	// Check for any outgoing fragments, drop error if we got some. (There should be none.)
	if (!netChannel->outgoingFragments) {
		Com_Error(ErrorType::Drop, "NetChan_TransmitNextFragment: no outgoing fragments\n");
	}

	// Ensure the fragmentsize is appropriately set.
	if (netChannel->outgoingFragmentOffset + FRAGMENT_SIZE <= netChannel->outgoingFragmentSize) {
		fragmentSize = FRAGMENT_SIZE;
	} else {
		fragmentSize = netChannel->outgoingFragmentSize - netChannel->outgoingFragmentOffset;
	}

	// Initialize the message buffer.
	SZ_Init(&send, data, sizeof(data));

	// Write sequence number.
	SZ_WriteLong(&send, netChannel->outgoingSequence | FRAGMENT_BIT);

	// Write uncompressed message size.
	SZ_WriteShort(&send, netChannel->outgoingFragmentBytes);

	// Write the channel port in case we are a client.
#if USE_CLIENT
    if (netChannel->netSource == NS_CLIENT && netChannel->remoteQPort) {
        SZ_WriteByte(&send, netChannel->remoteQPort);
    }
#endif

	// Write the fragment offset and size.
	SZ_WriteShort(&send, netChannel->outgoingFragmentOffset);
	SZ_WriteShort(&send, fragmentSize);

	// Write the compressed message data.
	SZ_Write(&send, netChannel->outgoingFragmentBuffer + netChannel->outgoingFragmentOffset, fragmentSize);

	// Send the datagram.
	//if (netChannel->remoteNetAddress.type != NetAddressType::NA_LOOPBACK) {
	NET_SendPacket(netChannel->netSource, send.data, send.currentSize, &netChannel->remoteNetAddress);
	//} else {
	//}

	// Show debug packet info.
	DShowPacket("%s send %4i: seq=%i ack=%i\n", net_sourceString[netChannel->netSource], send.currentSize, netChannel->outgoingSequence, netChannel->incomingSequence);

	netChannel->outgoingFragmentOffset += fragmentSize;

	// Update rate variables.
	if (time - netChannel->outgoingRateTime > 1000) {
		netChannel->outgoingRateBytes -= netChannel->outgoingFragmentBytes * (time - netChannel->outgoingRateTime - 1000) / 1000;
		if (netChannel->outgoingRateBytes < 0) {
			netChannel->outgoingRateBytes = 0;
		}
	}

	netChannel->outgoingRateTime = time - 1000;
	netChannel->outgoingRateBytes += send.currentSize;

	// This exit condition is a little tricky, because a packet that is exactly the fragment size
	// still needs to send a second packet of zero length so that the other side can tell there
	// aren't more to follow
	if (netChannel->outgoingFragmentOffset == netChannel->outgoingFragmentSize && fragmentSize < FRAGMENT_SIZE){
		netChannel->outgoingFragments = false;

		netChannel->outgoingSequence++;
	}

	// Return size.
    return send.currentSize;
}

/**
*	@brief	Sends the remaining fragments of the current message
**/
size_t Netchan_TransmitAllFragments(NetChannel* netChannel, uint64_t time) {
	if (!netChannel->outgoingFragments)
		Com_Error(false, "NetChan_TransmitAllFragments: no outgoing fragments");

	// Send all the remaining fragments at once
	size_t size = 0;
	while (netChannel->outgoingFragments) {
		size += Netchan_TransmitNextFragment(netChannel, time);
	}

	return size;
}


/**
*	@brief	Receives a message from a connection.
*			The msg parameter must be large enough to hold MAX_MSG_SIZE bytes of data, because if this is
*			the final fragment of a multi-part message, the entire thing will be copied out.
*	@return	Returns false if the message should not be processed due to being out of order or a fragment.
**/
qboolean Netchan_Process(NetChannel *netChannel, uint64_t time, SizeBuffer &message) {
	byte	buffer[MAX_MSGLEN];
	int32_t	sequence = 0, size = 0;
	int32_t	fragmentOffset = 0, fragmentSize = 0;
	bool	fragmented = false;
	int32_t	dropped = 0;

	// Allow simulation of connections that drop a lot of packets
	//if (net_dropSim->floatValue){
	//	if (net_random.RandFloat() < net_dropSim->floatValue)
	//		return false;
	//}

	// Reset readcount.
	message.readCount = 0;

	// Read the sequence number
	sequence = MSG_ReadUint16();

	// Check for a fragmented message
	if (sequence & FRAGMENT_BIT){
		sequence &= ~FRAGMENT_BIT;

		fragmented = true;
	}
	else
		fragmented = false;

	// Read the uncompressed message size
	size = MSG_ReadUint16();

	// Read the channel port if we are a server
#if USE_CLIENT
    if (netChannel->netSource == NS_SERVER) {
#endif
        if (netChannel->remoteQPort) {
            MSG_ReadUint8();//MSG_ReadByte();
        }
#if USE_CLIENT
    }
#endif

	// Read the fragment offset and size if this is a fragmented message
	if (fragmented){
		fragmentOffset = MSG_ReadUint16();
		fragmentSize = MSG_ReadUint16();
	}

	//if (net_showPackets->integerValue){
	//	if (fragmented)
	//		Com_Printf("%s recv %4i: seq=%i fragment=%i,%i\n", net_sourceString[netChannel->sock], msg.GetWriteBytes(), sequence, fragmentOffset, fragmentSize);
	//	else
	//		Com_Printf("%s recv %4i: seq=%i\n", net_sourceString[netChannel->sock], msg.GetWriteBytes(), sequence);
	//}

	// Update rate variables
	if (time - netChannel->incomingRateTime > 1000){
		netChannel->incomingRateBytes -= netChannel->incomingRateBytes * (time - netChannel->incomingRateTime - 1000) / 1000;
		if (netChannel->incomingRateBytes < 0)
			netChannel->incomingRateBytes = 0;
	}

	netChannel->incomingRateTime = time - 1000;
	netChannel->incomingRateBytes += message.currentSize;

	// Discard out of order or duplicated packets
	if (sequence <= netChannel->incomingSequence){
		DShowDrop("%s: out of order packet %i at %i\n", netChannel->remoteAddress.c_str(), sequence, netChannel->incomingSequence);

		return false;
	}

	// Dropped packets don't keep the message from being used
	dropped = sequence - (netChannel->incomingSequence + 1);
	if (dropped > 0){
		DShowDrop("%s: dropped %i packets at %i\n", netChannel->remoteAddress.c_str(), dropped, sequence);
	}

	// Discard packets with bad uncompressed message sizes
	if (size < 0 || size > MAX_MSGLEN){
		DShowDrop("%s: illegal uncompressed message size at %i\n", netChannel->remoteAddress.c_str(), sequence);

		return false;
	}

	// If a fragmented message
	if (fragmented){
		// Make sure we add the fragments in correct order
		if (netChannel->incomingFragmentSequence != sequence){
			netChannel->incomingFragmentSequence = sequence;
			netChannel->incomingFragmentSize = 0;
		}

		// If we missed a fragment, dump it
		if (netChannel->incomingFragmentSize != fragmentOffset){
			DShowDrop("%s: dropped a message fragment at %i\n", netChannel->remoteAddress.c_str(), sequence);

			return false;
		}

		// If it has a bad fragment size, dump it
		if (fragmentSize < 0 || fragmentSize > FRAGMENT_SIZE){
			DShowDrop("%s: illegal message fragment size at %i\n", netChannel->remoteAddress.c_str(), sequence);

			return false;
		}

		// Copy the fragment to the fragment buffer
		if (netChannel->incomingFragmentSize + fragmentSize > sizeof(netChannel->incomingFragmentBuffer) || message.readCount + fragmentSize > message.currentSize){
			DShowDrop("%s: illegal message fragment size at %i\n", netChannel->remoteAddress.c_str(), sequence);

			return false;
		}

		memcpy(netChannel->incomingFragmentBuffer + netChannel->incomingFragmentSize, message.data, fragmentSize);
//		msg.ReadData(netChannel->incomingFragmentBuffer + netChannel->incomingFragmentSize, fragmentSize);

		netChannel->incomingFragmentSize += fragmentSize;

		// If this wasn't the last fragment, don't process anything
		if (fragmentSize == FRAGMENT_SIZE)
			return false;

		// Decompress the message data
		if (Huff_Decompress(netChannel->incomingFragmentBuffer, buffer, size) != netChannel->incomingFragmentSize){
			DShowDrop("%s: failed to decompress message at %i\n", netChannel->remoteAddress.c_str(), sequence);

			return false;
		}

		DShowDrop("%s decompress: %4i -> %4i\n", net_sourceString[netChannel->netSource], netChannel->incomingFragmentSize, size);

		// Clear the write state
		//msg.ClearWriteState();
		message.currentSize = 0;

		// Write the sequence number
		SZ_WriteLong(&message, sequence);
		//msg.WriteLong(sequence);

		// Write the uncompressed message data
		//msg.WriteData(buffer, size);
		SZ_Write(&message, buffer, size);

		// The message can now be read from the current message pointer
		netChannel->incomingSequence = sequence;

		return true;
	}

	// Decompress the message data
	if (Huff_Decompress(message.data + message.readCount, buffer, size) != message.currentSize - message.readCount){
		DShowDrop("%s: failed to decompress message at %i\n", netChannel->remoteAddress.c_str(), sequence);

		return false;
	}

	DShowDrop("%s decompress: %4i -> %4i\n", net_sourceString[netChannel->netSource], message.currentSize - message.readCount, size);

	// Clear the write state
	message.currentSize = 0;

	// Write the sequence number
	SZ_WriteLong(&message, sequence);

	// Write the uncompressed message data
	SZ_Write(&message, buffer, size);

	// The message can now be read from the current message pointer
	netChannel->incomingSequence = sequence;

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
        chan->outFragment.currentSize ||
        com_localTime - netChannel->lastSentTime > 1000) {
        return true;
    }

    return false;
}
