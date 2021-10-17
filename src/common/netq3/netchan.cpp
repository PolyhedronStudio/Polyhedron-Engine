#include "shared/shared.h"
#include "common/common.h"
#include "common/cvar.h"
#include "common/msg.h"
#include "common/net/net.h"
#include "common/netq3/net.h"
#include "common/netq3/netchan.h"
#include "common/protocol.h"
#include "common/sizebuf.h"
#include "common/zone.h"
#include "system/system.h"

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

#define	MAX_PACKETLEN       1400    // max size of a network packet

#define	FRAGMENT_SIZE       (MAX_PACKETLEN - 100)
#define	PACKET_HEADER		10      // two ints and a short

#define	FRAGMENT_BIT        (1<<31)

extern cvar_t* net_qport;

static const char* netsrcString[2] = {
    "client",
    "server"
};


/*
===============
Q3Netchan_Init

===============
*/
void Q3Netchan_Init(int port) {
    port &= 0xffff;
#ifdef _DEBUG
    showpackets = Cvar_Get("showpackets", "0", 0);
    showdrop = Cvar_Get("showdrop", "0", 0);
#endif
    net_qport = Cvar_Get("net_qport", va("%i", port), 0);
}

/*
=================
Q3Netchan_Process
=================
*/
qboolean Q3Netchan_Process(netchan_t* chan, msg_t* msg) {
    int			sequence;
    int			qport;
    int			fragmentStart, fragmentLength;
    qboolean	fragmented;

    // get sequence numbers		
    Q3MSG_BeginReadingOOB(msg);
    sequence = Q3MSG_ReadLong(msg);

    // check for fragment information
    if (sequence & FRAGMENT_BIT) {
        sequence &= ~FRAGMENT_BIT;
        fragmented = true;
    } else {
        fragmented = false;
    }

    // read the qport if we are a server
    if (chan->sock == NS_SERVER) {
        qport = Q3MSG_ReadShort(msg);
    }

    // read the fragment information
    if (fragmented) {
        fragmentStart = Q3MSG_ReadShort(msg);
        fragmentLength = Q3MSG_ReadShort(msg);
    } else {
        fragmentStart = 0;		// stop warning message
        fragmentLength = 0;
    }

    if (showpackets->integer) {
        if (fragmented) {
            Com_Printf("%s recv %4i : s=%i fragment=%i,%i\n"
                , netsrcString[chan->sock]
                , msg->cursize
                , sequence
                , fragmentStart, fragmentLength);
        } else {
            Com_Printf("%s recv %4i : s=%i\n"
                , netsrcString[chan->sock]
                , msg->cursize
                , sequence);
        }
    }

    //
    // discard out of order or duplicated packets
    //
    if (sequence <= chan->incomingSequence) {
        if (showdrop->integer || showpackets->integer) {
            Com_Printf("%s:Out of order packet %i at %i\n"
                , NET_AdrToString(&chan->remoteAddress)
                , sequence
                , chan->incomingSequence);
        }
        return false;
    }

    //
    // dropped packets don't keep the message from being used
    //
    chan->dropped = sequence - (chan->incomingSequence + 1);
    if (chan->dropped > 0) {
        if (showdrop->integer || showpackets->integer) {
            Com_Printf("%s:Dropped %i packets at %i\n"
                , NET_AdrToString(&chan->remoteAddress)
                , chan->dropped
                , sequence);
        }
    }

    //
    // if this is the final framgent of a reliable message,
    // bump incoming_reliable_sequence 
    //
    if (fragmented) {
        // TTimo
        // make sure we add the fragments in correct order
        // either a packet was dropped, or we received this one too soon
        // we don't reconstruct the fragments. we will wait till this fragment gets to us again
        // (NOTE: we could probably try to rebuild by out of order chunks if needed)
        if (sequence != chan->fragmentSequence) {
            chan->fragmentSequence = sequence;
            chan->fragmentLength = 0;
        }

        // if we missed a fragment, dump the message
        if (fragmentStart != chan->fragmentLength) {
            if (showdrop->integer || showpackets->integer) {
                Com_Printf("%s:Dropped a message fragment\n"
                    , NET_AdrToString(&chan->remoteAddress)
                    , sequence);
            }
            // we can still keep the part that we have so far,
            // so we don't need to clear chan->fragmentLength
            return false;
        }

        // copy the fragment to the fragment buffer
        if (fragmentLength < 0 || msg->readcount + fragmentLength > msg->cursize ||
            chan->fragmentLength + fragmentLength > sizeof(chan->fragmentBuffer)) {
            if (showdrop->integer || showpackets->integer) {
                Com_Printf("%s:illegal fragment length\n"
                    , NET_AdrToString(&chan->remoteAddress));
            }
            return false;
        }

        std::memcpy(chan->fragmentBuffer + chan->fragmentLength,
            msg->data + msg->readcount, fragmentLength);

        chan->fragmentLength += fragmentLength;

        // if this wasn't the last fragment, don't process anything
        if (fragmentLength == FRAGMENT_SIZE) {
            return false;
        }

        if (chan->fragmentLength > msg->maxsize) {
            Com_Printf("%s:fragmentLength %i > msg->maxsize\n"
                , NET_AdrToString(&chan->remoteAddress),
                chan->fragmentLength);
            return false;
        }

        // copy the full message over the partial fragment

        // make sure the sequence number is still there
        *(int*)msg->data = LittleLong(sequence);

        std::memcpy(msg->data + 4, chan->fragmentBuffer, chan->fragmentLength);
        msg->cursize = chan->fragmentLength + 4;
        chan->fragmentLength = 0;
        msg->readcount = 4;	// past the sequence number
        msg->bit = 32;	// past the sequence number

        // TTimo
        // clients were not acking fragmented messages
        chan->incomingSequence = sequence;

        return true;
    }

    //
    // the message can now be read from the current message pointer
    //
    chan->incomingSequence = sequence;

    return true;
}