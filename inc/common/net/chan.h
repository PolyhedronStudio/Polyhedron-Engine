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
#include "common/sizebuf.h"

typedef struct netchan_s {
    int         protocol;
    size_t      maxpacketlen;

    qboolean    fatal_error;

    netsrc_t    sock;

    int         dropped;            // between last packet and previous
    unsigned    totalDropped;      // for statistics
    unsigned    totalReceived;

    unsigned    lastReceived;      // for timeouts
    unsigned    lastSent;          // for retransmits

    netadr_t    remoteAddress;
    int         qport;              // qport value to write when transmitting

    size_t      reliableLength;

    // Pending states.
    qboolean    reliableAckPending;   // set to true each time reliable is received
    qboolean    fragmentPending;

    // sequencing variables
    int         incomingSequence;
    int         incomingAcknowledged;
    int         outgoingSequence;

    // sequencing variables
    int         incomingReliableAcknowledged; // single bit
    int         incomingReliableSequence;     // single bit, maintained local
    int         reliableSequence;              // single bit
    int         lastReliableSequence;         // sequence number of last send
    int         fragmentSequence;

    // reliable staging and holding areas
    sizebuf_t   message;                        // writing buffer for reliable data
    byte        messageBuffer[MAX_MSGLEN];        // leave space for header

// message is copied to this buffer when it is first transfered
    sizebuf_t   reliable;
    byte        reliableBuffer[MAX_MSGLEN];   // unacked reliable message

    sizebuf_t   inFragment;
    byte        inFragmentBuffer[MAX_MSGLEN];

    sizebuf_t   outFragment;
    byte        outFragmentBuffer[MAX_MSGLEN];
} netchan_t;

extern cvar_t       *net_qport;
extern cvar_t       *net_maxmsglen;
extern cvar_t       *net_chantype;

void Netchan_Init(void);
void Netchan_OutOfBand(netsrc_t sock, const netadr_t *adr, const char *format, ...) q_printf(3, 4);
netchan_t *Netchan_Setup(netsrc_t sock, const netadr_t *adr, int qport, size_t maxpacketlen, int protocol);

size_t      Netchan_Transmit(struct netchan_s*, size_t, const void*, int);
size_t      Netchan_TransmitNextFragment(struct netchan_s*);
qboolean    Netchan_Process(struct netchan_s*);
qboolean    Netchan_ShouldUpdate(struct netchan_s*);

void Netchan_Close(netchan_t *netchan);

#define OOB_PRINT(sock, addr, data) \
    NET_SendPacket(sock, CONST_STR_LEN("\xff\xff\xff\xff" data), addr)

//============================================================================

#endif // NET_CHAN_H
