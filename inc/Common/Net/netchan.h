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

#ifndef NET_CHAN_H
#define NET_CHAN_H

#include "common/msg.h"
#include "common/net/net.h"
#include "common/sizebuffer.h"

//struct NetChannel {
//    int         protocol;
//    size_t      maxPacketLength;
//
//    qboolean    fatalError;
//
//    NetSource    sock;
//
//    int         dropped;            // between last packet and previous
//    unsigned    totalDropped;      // for statistics
//    unsigned    totalReceived;
//
//    unsigned    lastReceivedTime;      // for timeouts
//    unsigned    lastSentTime;          // for retransmits
//
//    NetAdr    remoteNetAddress;
//    int         qport;              // qport value to write when transmitting
//
//    size_t      reliableLength;
//
//    // Pending states.
//    qboolean    reliableAckPending;   // set to true each time reliable is received
//    qboolean    fragmentPending;
//
//    // sequencing variables
//    int         incomingSequence;
//    int         incomingAcknowledged;
//    int         outgoingSequence;
//
//    // sequencing variables
//    int         incomingReliableAcknowledged; // single bit
//    int         incomingReliableSequence;     // single bit, maintained local
//    int         reliableSequence;              // single bit
//    int         lastReliableSequence;         // sequence number of last send
//    int         fragmentSequence;
//
//    // reliable staging and holding areas
//    SizeBuffer   message;                        // writing buffer for reliable data
//    byte        messageBuffer[MAX_MSGLEN];        // leave space for header
//
//// message is copied to this buffer when it is first transfered
//    SizeBuffer   reliable;
//    byte        reliableBuffer[MAX_MSGLEN];   // unacked reliable message
//
//    SizeBuffer   inFragment;
//    byte        inFragmentBuffer[MAX_MSGLEN];
//
//    SizeBuffer   outFragment;
//    byte        outFragmentBuffer[MAX_MSGLEN];
//};

struct NetChannel {
public:
    int32_t     protocolVersion;
    size_t      maximumPacketLength;

    qboolean    fatalError;         // True in case we ran into a major error.

    NetSource   netSource;          // The source this channel is from: Client, or Server.

    int         deltaFramePacketDrops;  // Between last packet and previous.
    uint32_t    totalDropped;           // For statistics.
    uint32_t    totalReceived;

    uint32_t    lastReceivedTime;   // For timeouts.
    uint32_t    lastSentTime;       // For retransmits.

    NetAdr    remoteNetAddress;   // NetChan settled to the remote ahost.
    std::string remoteAddress;      // Textual address.
    int32_t     remoteQPort;        // qport value to write when transmitting

    size_t      reliableLength;

    // Pending states.
    qboolean    reliableAckPending; // Set to true each time reliable is received
    qboolean    fragmentPending;    // Set to true when there is still a fragment pending.

    // sequencing variables
    int         incomingSequence;
    int         incomingAcknowledged;
    int         outgoingSequence;

    // sequencing variables
    int         incomingReliableAcknowledged;   // single bit
    int         incomingReliableSequence;       // single bit, maintained local
    int         reliableSequence;               // single bit
    int         lastReliableSequence;           // sequence number of last send
    int         fragmentSequence;

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

extern cvar_t       *net_qport;
extern cvar_t       *net_maxmsglen;
extern cvar_t       *net_chantype;

void Netchan_Init(void);
void Netchan_OutOfBand(NetSource sock, const NetAdr *adr, const char *format, ...) q_printf(3, 4);
NetChannel *Netchan_Setup(NetSource sock, const NetAdr *adr, int qport, size_t maxPacketLength, int protocol);

size_t      Netchan_Transmit(NetChannel*, size_t, const void*, int);
size_t      Netchan_TransmitNextFragment(NetChannel*);
qboolean    Netchan_Process(NetChannel*);
qboolean    Netchan_ShouldUpdate(NetChannel*);

void Netchan_Close(NetChannel*netchan);

#define OOB_PRINT(sock, addr, data) \
    NET_SendPacket(sock, CONST_STR_LEN("\xff\xff\xff\xff" data), addr)

//============================================================================

#endif // NET_CHAN_H
