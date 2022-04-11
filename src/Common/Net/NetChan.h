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
#pragma once

#include "Common/Msg.h"
#include "Common/Net/Net.h"
#include "Common/SizeBuffer.h"

//! Random port value in-use.
extern cvar_t       *net_qport;
//! Maximum packet message length.
extern cvar_t       *net_maxmsglen;
//! Actual netchan type in use.
extern cvar_t       *net_chantype;

/**
*	@brief	The actual 'Network Channel' handles the receiving, sending, and fragmentation
*			of the network packets. It serves as a higher-level wrapper around the actual
*			net code itself.
**/
struct NetChannel {
public:
    int32_t     protocolVersion = 0;
    size_t      maximumPacketLength = 0;

    qboolean    fatalError = false;         // True in case we ran into a major error.

    NetSource   netSource;          // The source this channel is from: Client, or Server.

    int32_t     deltaFramePacketDrops = 0;  // Between last packet and previous.
    uint32_t    totalDropped = 0;           // For statistics.
    uint32_t    totalReceived = 0;

    uint32_t    lastReceivedTime = 0;   // For timeouts.
    uint32_t    lastSentTime = 0;       // For retransmits.

    NetAdr		remoteNetAddress;   // NetChan settled to the remote ahost.
    std::string remoteAddress = "";      // Textual address.
    int32_t     remoteQPort = 0;        // qport value to write when transmitting

    size_t      reliableLength = 0;

    // Pending states.
    qboolean    reliableAckPending = 0; // Set to true each time reliable is received
    qboolean    fragmentPending = 0;    // Set to true when there is still a fragment pending.

    // sequencing variables
    int32_t     incomingSequence = 0;
    int32_t     incomingAcknowledged = 0;
    int32_t     outgoingSequence = 0;

    // sequencing variables
    int32_t     incomingReliableAcknowledged = 0;   // single bit
    int32_t     incomingReliableSequence = 0;       // single bit, maintained local
    int32_t     reliableSequence = 0;               // single bit
    int32_t     lastReliableSequence = 0;           // sequence number of last send
    int32_t     fragmentSequence = 0;

    // Reliable staging and holding areas
    SizeBuffer  message;                    // writing buffer for reliable data
    byte        messageBuffer[MAX_MSGLEN];  // leave space for header

    // Message is copied to this buffer when it is first transfered
    SizeBuffer  reliable;
    byte        reliableBuffer[MAX_MSGLEN];   // unacked reliable message

    SizeBuffer  inFragment;
    byte        inFragmentBuffer[MAX_MSGLEN];

    SizeBuffer  outFragment;
    byte        outFragmentBuffer[MAX_MSGLEN];
};


/**
*	@brief	Initializes the network channel subsystem.
**/
void Netchan_Init(void);

/**
*	@brief	Sends a text message in an out-of-band datagram
**/
void Netchan_OutOfBand(NetSource sock, const NetAdr *adr, const char *format, ...) q_printf(3, 4);

/**
*	@brief	Opens a channel to a remote system. (Or localhost.)
**/
NetChannel *Netchan_Setup(NetSource sock, const NetAdr *adr, int32_t qport, size_t maxPacketLength, int32_t protocol);

/**
*	@brief	Sends a message to a connection, fragmenting if necessary. 
*			A zero sized message will still generate a packet.
**/
size_t      Netchan_Transmit(NetChannel *netChannel, size_t, const void*, int32_t numberOfPackets, uint64_t time);

/**
*	@brief	Sends one fragment of the current message.
**/
size_t      Netchan_TransmitNextFragment(NetChannel *netChannel, uint64_t time);

/**
*	@brief	Sends the remaining fragments of the current message
**/
size_t      Netchan_TransmitAllFragments(NetChannel *netChannel, uint64_t time);

/**
*	@brief	Receives a message from a connection.
*			The msg parameter must be large enough to hold MAX_MSG_SIZE bytes of data, because if this is
*			the final fragment of a multi-part message, the entire thing will be copied out.
*	@return	Returns false if the message should not be processed due to being out of order or a fragment.
**/
qboolean    Netchan_Process(NetChannel *netChannel);


/**
*	@return	True in case the channel should update.
**/
qboolean    Netchan_ShouldUpdate(NetChannel *netChannel);

/**
*	@brief	Closes the network channel.
**/
void Netchan_Close(NetChannel*netchan);

#define OOB_PRINT(sock, addr, data) \
    NET_SendPacket(sock, CONST_STR_LEN("\xff\xff\xff\xff" data), addr)