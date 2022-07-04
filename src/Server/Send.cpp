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
// sv_send.c

#include "Server.h"

/*
=============================================================================

MISC

=============================================================================
*/

char sv_outputbuf[SV_OUTPUTBUF_LENGTH];

void SV_FlushRedirect(int redirected, char *outputbuf, size_t len)
{
    byte    buffer[MAX_PACKETLEN_DEFAULT];

    if (redirected == RD_PACKET) {
        memcpy(buffer, "\xff\xff\xff\xffprint\n", 10);
        memcpy(buffer + 10, outputbuf, len);
        NET_SendPacket(NS_SERVER, buffer, len + 10, &net_from);
    } else if (redirected == RD_CLIENT) {
        MSG_WriteUint8(ServerCommand::Print);//MSG_WriteByte(ServerCommand::Print);
        MSG_WriteUint8(PRINT_HIGH);//MSG_WriteByte(PRINT_HIGH);
        MSG_WriteData(outputbuf, len);
        MSG_WriteUint8(0);//MSG_WriteByte(0);
        SV_ClientAddMessage(sv_client, MSG_RELIABLE | MSG_CLEAR);
    }
}

/*
=======================
SV_RateDrop

Returns true if the client is over its current
bandwidth estimation and should not be sent another packet
=======================
*/
static qboolean SV_RateDrop(client_t *client)
{
    // never drop over the loopback
    if (!client->rate) {
        return false;
    }

    size_t totalRate = 0;
    for (uint32_t i = 0; i < SERVER_MESSAGES_TICKRATE; i++) {
        totalRate += client->messageSizes[i];
    }

    // Divide the total by rate divisor.
    totalRate /= SERVER_RATE_MULTIPLIER;

    if (totalRate > client->rate) {
        SV_DPrintf(0, "Frame %d suppressed for %s (total = %" PRIz ")\n",
                   client->frameNumber, client->name, totalRate);
        client->frameFlags |= FrameFlags::Suppressed;
        client->suppressCount++;
        client->messageSizes[client->frameNumber % SERVER_MESSAGES_TICKRATE] = 0;
        return true;
    }

    return false;
}

static void SV_CalcSendTime(client_t *client, size_t size)
{
    // never drop over the loopback
    if (!client->rate) {
        client->sendTime = svs.realTime;
        client->sendDelta = 0;
        return;
    }

    if (client->connectionState == ConnectionState::Spawned)
        client->messageSizes[client->frameNumber % SERVER_MESSAGES_TICKRATE] = size;

    client->sendTime = svs.realTime;
    client->sendDelta = size * 1000 / client->rate;
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/


/**
*	@brief	Sends the print '..string..' command to a specific client. Clears the msg_write buffer after adding
*			the command to the client's reliable list.
**/
void SV_ClientPrintf(client_t *client, int level, const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    size_t      len;

    if (level < client->messageLevel)
        return;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::Print);//MSG_WriteByte(ServerCommand::Print);
    MSG_WriteUint8(level); //MSG_WriteByte(level);
    MSG_WriteData(string, len + 1);

    SV_ClientAddMessage(client, MSG_RELIABLE | MSG_CLEAR);
}

/**
*	@brief	Sends the print '..string..' command to all active clients. Clears the msg_write buffer 
*			after adding the command to all clients' reliable message list.
**/
void SV_BroadcastPrintf(int level, const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    client_t    *client;
    size_t      len;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::Print); //MSG_WriteByte(ServerCommand::Print);
    MSG_WriteUint8(level);//MSG_WriteByte(level);
    MSG_WriteData(string, len + 1);

    FOR_EACH_CLIENT(client) {
        if (client->connectionState != ConnectionState::Spawned)
            continue;
        if (level < client->messageLevel)
            continue;
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SZ_Clear(&msg_write);
}

/**
*	@brief	Sends the specified command to a specific client. Clears the msg_write buffer after adding
*			the command to the client's reliable list.
**/
void SV_ClientCommand(client_t *client, const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    size_t      len;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::StuffText);//MSG_WriteByte(ServerCommand::StuffText);
    MSG_WriteData(string, len + 1);

    SV_ClientAddMessage(client, MSG_RELIABLE | MSG_CLEAR);
}

/**
*	@brief	Sends the specified command to all active clients. Clears the msg_write buffer 
*			after adding the command to all clients' reliable message list.
**/
void SV_BroadcastCommand(const char *fmt, ...)
{
    va_list     argptr;
    char        string[MAX_STRING_CHARS];
    client_t    *client;
    size_t      len;

    va_start(argptr, fmt);
    len = Q_vsnprintf(string, sizeof(string), fmt, argptr);
    va_end(argptr);

    if (len >= sizeof(string)) {
        Com_WPrintf("%s: overflow\n", __func__);
        return;
    }

    MSG_WriteUint8(ServerCommand::StuffText);//MSG_WriteByte(ServerCommand::StuffText);
    MSG_WriteData(string, len + 1);

    FOR_EACH_CLIENT(client) {
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SZ_Clear(&msg_write);
}

/**
*	@description	Sends the contents of the write buffer to a subset of the clients,
*					then clears the write buffer.
*	
*					Multicast::All	Same as broadcast (origin can be NULL)
*					Multicast::PVS	Send to clients potentially visible from origin
*					Multicast::PHS	Send to clients potentially hearable from origin
**/
void SV_Multicast(const vec3_t &origin, int32_t to) {
    client_t	*client = nullptr;
    static byte mask[VIS_MAX_BYTES];
    mleaf_t *leafA = nullptr;
	mleaf_t *leafB = nullptr;
    int32_t	leafnum q_unused;
    
	// Sanitize.
    if (!sv.cm.cache) {
        Com_Error(ErrorType::Drop, "%s: no map loaded", __func__);
    }

	// Flags.
    int32_t flags = 0;

    switch (to) {
    case Multicast::All_R:
		// Add Reliable flag.
        flags |= MSG_RELIABLE;
        // intentional fallthrough
    case Multicast::All:
        leafA = NULL;
        leafnum = 0;
        break;
    case Multicast::PHS_R:
		// Add Reliable Flag.
        flags |= MSG_RELIABLE;

        // Intentional fallthrough.
    case Multicast::PHS:
		// Point leaf.
        leafA = CM_PointLeaf(&sv.cm, origin);
        leafnum = leafA - sv.cm.cache->leafs;
		// Cluster Vis.
        BSP_ClusterVis(sv.cm.cache, mask, leafA->cluster, DVIS_PHS);
        break;
    case Multicast::PVS_R:
		// Add Reliable Flag.
        flags |= MSG_RELIABLE;

        // Intentional fallthrough
    case Multicast::PVS:
		// Point leaf.
        leafA = CM_PointLeaf(&sv.cm, origin);
        leafnum = leafA - sv.cm.cache->leafs;
		// Cluster Vis.
        BSP_ClusterVis(sv.cm.cache, mask, leafA->cluster, DVIS_PVS2);
        break;
    default:
        Com_Error(ErrorType::Drop, "SV_Multicast: bad to: %i", to);
    }

    // send the data to all relevent clients
    FOR_EACH_CLIENT(client) {
		// Ensure connection state > Primed.
        if (client->connectionState < ConnectionState::Primed) {
            continue;
        }

        // Do not send unreliables to connecting clients.
        if (!(flags & MSG_RELIABLE) && (client->connectionState != ConnectionState::Spawned || client->download.bytes || client->nodata)) {
            continue;
        }

        if (leafA) {
			// Find the client's PVS.
			// "FIXME: for some strange reason, game code assumes the server uses entity origin for PVS/PHS culling, not the view origin."
			//#if 0
            //PlayerState *ps = &client->edict->client->playerState;
            //VectorAdd(ps->viewOffset, ps->pmove.origin, orig);
            //VectorMA(ps->viewOffset, 0.125f, ps->pmove.origin, org);
			//#else
            const vec3_t clientOrigin = client->edict->currentState.origin; // VectorCopy(client->edict->state.origin, org);
			//#endif
			
			// Do a point leaf.
            leafB = CM_PointLeaf(&sv.cm, clientOrigin);

			// Skip If: Areas aren't connected.
            if (!CM_AreasConnected(&sv.cm, leafA->area, leafB->area))
                continue;
			// Skip If: It has no cluster..
            if (leafB->cluster == -1)
                continue;
			// Skip if: No cluster mask match.
            if (!Q_IsBitSet(mask, leafB->cluster))
                continue;
        }

		// Add message to the client (reliable-)message list.
        SV_ClientAddMessage(client, flags);
    }

    // Clear the msg_write buffer.
    SZ_Clear(&msg_write);
}

/**
*	@brief	Add message data.
**/
static void SV_AddMessagePacket(client_t *client, byte *data, size_t len, qboolean reliable);
static void SV_AddMessage(client_t *client, byte *data, size_t len, qboolean reliable) {
	if (reliable) {
        // Don't packetize, netchan level will do fragmentation as needed
        SZ_Write(&client->netChan->message, data, len);
    } else {
        // Still have to packetize, relative sounds need special processing
        SV_AddMessagePacket(client, data, len, false);
    }
}

/**
*	@brief		Compresses and adds message to the client.
*	@return		True if compressed and added, false otherwise.
**/
static qboolean SV_CompressMessage(client_t *client, int flags)
{
#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    byte    buffer[MAX_MSGLEN];

    if (!(flags & MSG_COMPRESS))
        return false;

    if (!client->has_zlib)
        return false;

    // compress only sufficiently large layouts
    if (msg_write.currentSize < client->netChan->maximumPacketLength / 2)
        return false;

    deflateReset(&svs.z);
    svs.z.next_in = msg_write.data;
    svs.z.avail_in = (uInt)msg_write.currentSize;
    svs.z.next_out = buffer + 5;
    svs.z.avail_out = (uInt)(MAX_MSGLEN - 5);

    if (deflate(&svs.z, Z_FINISH) != Z_STREAM_END)
        return false;

    buffer[0] = ServerCommand::ZPacket;
    buffer[1] = svs.z.total_out & 255;
    buffer[2] = (svs.z.total_out >> 8) & 255;
    buffer[3] = msg_write.currentSize & 255;
    buffer[4] = (msg_write.currentSize >> 8) & 255;

    SV_DPrintf(0, "%s: comp: %lu into %lu\n",
               client->name, svs.z.total_in, svs.z.total_out + 5);

    if (svs.z.total_out + 5 > msg_write.currentSize)
        return false;

	// Add message to netchan message buffer.
    SV_AddMessage(client, buffer, svs.z.total_out + 5,
                       (flags & MSG_RELIABLE) ? true : false);
    return true;
#else
    return false;
#endif
}

/**
*	Adds contents of the current write buffer to client's message list.
*	Does NOT clean the buffer for multicast delivery purpose, unless told otherwise.
**/
void SV_ClientAddMessage(client_t *client, int32_t flags) {
	// Debug print.
    SV_DPrintf(1, "Added %sreliable message to %s: %" PRIz " bytes\n",
               (flags & MSG_RELIABLE) ? "" : "un", client->name, msg_write.currentSize);

	// Return in case there is no message to write out.
    if (!msg_write.currentSize) {
        return;
    }

	// Compress and add message, if successfull, skip this add message and clear buffer if wished for.
    if (SV_CompressMessage(client, flags)) {
        goto clear;
    }

	// Add client message.
    SV_AddMessage(client, msg_write.data, msg_write.currentSize, (flags & MSG_RELIABLE) ? true : false);

clear:
    if (flags & MSG_CLEAR) {
        SZ_Clear(&msg_write);
    }
}

/**
*
*
*	Frame Updates - Common.
*
*
**/
/**
*	@brief	Removes the message from the client's packet list. If the client's message size 
*			overflowed it'll free the packet from memory instead of adding it to the msg_free_list.
**/
static inline void SV_FreeMessagePacket(client_t *client, MessagePacket *msg)
{
	// Remove packet from list.
    List_Remove(&msg->entry);

	// Check if packet overflowed.
    if (msg->currentSize > MSG_TRESHOLD) {
		// Did it cross our limits?
        if (msg->currentSize > client->msg_dynamic_bytes) {
            Com_Error(ErrorType::Fatal, "%s: bad packet size", __func__);
        }
		// Remove packet size from dynamic bytes count.
        client->msg_dynamic_bytes -= msg->currentSize;
		// Free packet from memory.
        Z_Free(msg);
    } else {
		// Insert packet into our msg_free_list.
		List_Insert(&client->msg_free_list, &msg->entry);
    }
}

#define FOR_EACH_MSG_SAFE(list) \
    LIST_FOR_EACH_SAFE(MessagePacket, msg, next, list, entry)
#define MSG_FIRST(list) \
    LIST_FIRST(MessagePacket, list, entry)

/**
*	@brief	Removes all messages from the client's unreliable, and reliable packet lists.
**/
static void SV_FreeAllMessagePackets(client_t *client)
{
    MessagePacket *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        SV_FreeMessagePacket(client, msg);
    }
    FOR_EACH_MSG_SAFE(&client->msg_reliable_list) {
        SV_FreeMessagePacket(client, msg);
    }
    client->msg_unreliable_bytes = 0;
    client->msg_dynamic_bytes = 0;
}

/**
*	@brief	Adds a message packet to the client message list.
**/
static void SV_AddMessagePacket(client_t *client, byte *data, size_t len, qboolean reliable) {
    MessagePacket    *msg;

    if (!client->msg_pool) {
        return; // already dropped
    }

    if (len > MSG_TRESHOLD) {
        if (len > MAX_MSGLEN) {
            Com_Error(ErrorType::Fatal, "%s: oversize packet", __func__);
        }
        if (client->msg_dynamic_bytes + len > MAX_MSGLEN) {
            Com_WPrintf("%s: %s: out of dynamic memory\n",
                        __func__, client->name);
            goto overflowed;
        }
        msg = (MessagePacket*)SV_Malloc(sizeof(*msg) + len - MSG_TRESHOLD); // CPP: Cast
        client->msg_dynamic_bytes += len;
    } else {
        if (LIST_EMPTY(&client->msg_free_list)) {
            Com_WPrintf("%s: %s: out of message slots\n",
                        __func__, client->name);
            goto overflowed;
        }
        msg = MSG_FIRST(&client->msg_free_list);
        List_Remove(&msg->entry);
    }

    memcpy(msg->packet.data, data, len);
    msg->currentSize = (uint16_t)len;

    if (reliable) {
        List_Append(&client->msg_reliable_list, &msg->entry);
    } else {
        List_Append(&client->msg_unreliable_list, &msg->entry);
        client->msg_unreliable_bytes += len;
    }

    return;

overflowed:
    if (reliable) {
        SV_FreeAllMessagePackets(client);
        SV_DropClient(client, "reliable queue overflowed");
    }
}

/**
*	@return	True if this entity is present in current client frame.
**/
static qboolean SV_IsFrameEntity(client_t *client, int entnum) {
	// Acquire current frame.
    ClientFrame *frame = &client->frames[client->frameNumber & UPDATE_MASK];

	// Loop over entities.
    for (uint32_t i = 0; i < frame->num_entities; i++) {
		// Acquire actual state index.
        uint32_t j = (frame->first_entity + i) % svs.num_entities;

		//And if the state's entity number matches our argument entity number, boom, it's in the current frame.
        if (svs.entities[j].number == entnum) {
            return true;
        }
    }

	// Not in our current frame.
    return false;
}

/**
*	@brief	Sets up a sound emit MessagePacket for the given client.
*			Sounds relative to entities are handled specially.
**/
static void SV_EmitSound(client_t *client, MessagePacket *msg)
{
    int flags, entnum;

    entnum = msg->packet.ps.sendchan >> 3;
    flags = msg->packet.ps.flags;

    // Check if position needs to be explicitly sent
    if (!(flags & SoundCommandBits::Position) && !SV_IsFrameEntity(client, entnum)) {
        SV_DPrintf(0, "Forcing position on entity %d for %s\n", entnum, client->name);
		// Entity is not present in frame
        flags |= SoundCommandBits::Position;
    }

	// Sound Server Command.
    MSG_WriteUint8(ServerCommand::Sound);
    MSG_WriteUint8(flags);
    MSG_WriteUint8(msg->packet.ps.index);

	// Volume bit.
    if (flags & SoundCommandBits::Volume) {
        MSG_WriteUint8(msg->packet.ps.volume);
    }
	// Attenuation bit.
    if (flags & SoundCommandBits::Attenuation) {
        MSG_WriteUint8(msg->packet.ps.attenuation);//MSG_WriteByte(msg->attenuation);
    }
	// OFfset bit.
    if (flags & SoundCommandBits::Offset) {
        MSG_WriteUint8(msg->packet.ps.timeofs);//MSG_WriteByte(msg->timeofs);
    }

	// Send out the sound channel.
    MSG_WriteInt16(msg->packet.ps.sendchan);//MSG_WriteShort(msg->sendchan);

	// Send out the sound position.
    if (flags & SoundCommandBits::Position) {
        MSG_WriteVector3(msg->packet.ps.pos, false);
    }
}

static inline void SV_WriteSoundMessagePacket(client_t *client, MessagePacket *msg, size_t maximumSize)
{
    // if this msg fits, write it
    if (msg_write.currentSize + MAX_SOUND_PACKET <= maximumSize) {
        SV_EmitSound(client, msg);
    }
    List_Remove(&msg->entry);
    List_Insert(&client->msg_free_list, &msg->entry);
}

static inline void SV_WriteMessagePacket(client_t *client, MessagePacket *msg, size_t maximumSize)
{
    // if this msg fits, write it
    if (msg_write.currentSize + msg->currentSize <= maximumSize) {
        MSG_WriteData(msg->packet.data, msg->currentSize);
    }
    SV_FreeMessagePacket(client, msg);
}

static inline void SV_WriteUnreliableMessagePackets(client_t *client, size_t maximumSize)
{
    MessagePacket *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        if (msg->currentSize) {
            SV_WriteMessagePacket(client, msg, maximumSize);
        } else {
            SV_WriteSoundMessagePacket(client, msg, maximumSize);
        }
    }
}

/*
===============================================================================

FRAME UPDATES 

===============================================================================
*/
/**
*	@brief	Writes out all the messages that were collected to be send this current frame.
*			First it sends out all the reliables which are 
**/
static void SV_WriteDatagram(client_t *client) {
    size_t currentSize = 0;

    // send over all the relevant EntityState
    // and the PlayerState
    SV_WriteFrameToClient(client);

    if (msg_write.overflowed) {
        // should never really happen
        Com_WPrintf("Frame overflowed for %s\n", client->name);
        SZ_Clear(&msg_write);
    }

    // now write unreliable messages
    // for this client out to the message
    // it is necessary for this to be after the WriteFrame
    // so that entity references will be current
    if (msg_write.currentSize + client->msg_unreliable_bytes > msg_write.maximumSize) {
        Com_WPrintf("Dumping datagram for %s\n", client->name);
    } else {
        SV_WriteUnreliableMessagePackets(client, msg_write.maximumSize);
    }

#ifdef _DEBUG
    if (sv_pad_packets->integer) {
        size_t pad = msg_write.currentSize + sv_pad_packets->integer;

        if (pad > msg_write.maximumSize) {
            pad = msg_write.maximumSize;
        }
        for (; pad > 0; pad--) {
            MSG_WriteUint8(ServerCommand::Padding);//MSG_WriteByte(ServerCommand::Padding);
        }
    }
#endif

    // send the datagram
    currentSize = Netchan_Transmit(client->netChan,
                                        msg_write.currentSize,
                                        msg_write.data,
                                        client->numpackets, svs.realTime);

    // record the size for rate estimation
    SV_CalcSendTime(client, currentSize);

    // clear the write buffer
    SZ_Clear(&msg_write);
}


/****
*
*
*	Client Message Send Processing.
*
*	
****/
/**
*	@brief	Frees the remaining unreliable packets. Used after exceeding our packet size.
**/
static void SV_FreeUnreliablePackets(client_t *client) {
    MessagePacket *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        SV_FreeMessagePacket(client, msg);
    }
    client->msg_unreliable_bytes = 0;
}

/**
*	@brief	Called each game frame, sends ServerCommand::Frame messages to spawned clients only.
*			Clients in earlier connection state are handled in SV_SendAsyncPackets.
**/
void SV_SendClientMessages(void)
{
    client_t *client = nullptr;
    size_t currentSize = 0;

    // send a message to each connected client
    FOR_EACH_CLIENT(client) {
        if (client->connectionState != ConnectionState::Spawned || client->download.bytes || client->nodata)
            goto finish;

        // if the reliable message overflowed,
        // drop the client (should never happen)
        if (client->netChan->message.overflowed) {
            SZ_Clear(&client->netChan->message);
            SV_DropClient(client, "reliable message overflowed");
            goto finish;
        }

        // don't overrun bandwidth
        if (SV_RateDrop(client))
            goto advance;

        // Don't write any frame data until all fragments are sent
        if (client->netChan->fragmentPending) {
			// Set suppressed bits.
            client->frameFlags |= FrameFlags::Suppressed;

			// Transmit next fragment.
            currentSize = Netchan_TransmitNextFragment(client->netChan, svs.realTime);

			// Calculate sendtime.
            SV_CalcSendTime(client, currentSize);

			// Go on to next frame.
            goto advance;
        }

        // Build the client frame packet(s).
        SV_BuildClientFrame(client);

		// WRite out datagram to client.
        SV_WriteDatagram(client);

advance:
        // advance for next frame
        client->frameNumber++;

finish:
        // clear all unreliable messages still left
        SV_FreeUnreliablePackets(client);
    }
}

/**
*	@brief	Writes out pending download to client netchannel message sizebuffer. If there is any 
*			reliable length it will exit early before doing so.
**/
static void SV_WritePendingDownload(client_t *client) {

	// EXIT ON: No bytes to download.
    if (!client->download.bytes) {
        return;
	}

	// EXIT ON: No more downloads pending.
    if (!client->download.isPending) {
        return;
	}

	// EXIT ON: If there is a reliable data length.
    if (client->netChan->reliableLength) {
        return;
	}

	// Acquire pointer to the netchannel message buffer.
    SizeBuffer *netChannelMessage = &client->netChan->message;
	// Exit in case the packet size exceeds our limit.
    if (netChannelMessage->currentSize > client->netChan->maximumPacketLength) {
        return;
	}

	// Calculate remaining bytes left to download.
    size_t remaining = client->netChan->maximumPacketLength - netChannelMessage->currentSize;
    if (remaining <= 4) { // Exit if any less than 4 cuz that'd be our header.
        return;
	}

	// Calculate the chunk size to send.
    int32_t chunk = client->download.fileSize - client->download.bytesSent;
    if (chunk > remaining - 4) {
        chunk = remaining - 4;
	}

    client->download.isPending = false;
    client->download.bytesSent += chunk;

    int32_t percent = client->download.bytesSent * 100 / client->download.fileSize;

    SZ_WriteByte(netChannelMessage, client->download.command);
    SZ_WriteShort(netChannelMessage, chunk);
    SZ_WriteByte(netChannelMessage, percent);
    SZ_Write(netChannelMessage, client->download.bytes + client->download.bytesSent - chunk, chunk);

	// Close download when all bytes are sent.
    if (client->download.bytesSent == client->download.fileSize) {
        SV_CloseDownload(client);
    }
}

/***
*	@brief	If the client is just connecting, it is pointless to wait another 100ms
*			before sending next command and/or reliable acknowledge, send it as soon
*			as client rate limit allows.
*	
*			For spawned clients, this is not used, as we are forced to send ServerCommand::Frame
*			packets synchronously with game DLL ticks.
***/
void SV_SendAsyncPackets(void) {
    qboolean    retransmit;
    client_t    *client;
    NetChannel   *netchan;
    size_t      currentSize;

    FOR_EACH_CLIENT(client) {
        // don't overrun bandwidth
        if (svs.realTime - client->sendTime < client->sendDelta) {
            continue;
        }

        netchan = client->netChan;

        // make sure all fragments are transmitted first
        if (netchan->fragmentPending) {
            currentSize = Netchan_TransmitNextFragment(netchan, svs.realTime);
            SV_DPrintf(0, "%s: frag: %" PRIz "\n", client->name, currentSize);
            goto calctime;
        }

        // spawned clients are handled elsewhere
        if (client->connectionState == ConnectionState::Spawned && !client->download.bytes && !client->nodata && !SV_PAUSED) {
            continue;
        }

        // see if it's time to resend a (possibly dropped) packet
        retransmit = (com_localTime - netchan->lastSentTime > 1000);

        // don't write new reliables if not yet acknowledged
        if (netchan->reliableLength && !retransmit && client->connectionState != ConnectionState::Zombie) {
            continue;
        }

        // just update reliable if needed
        //if (netchan->type == NETCHAN_OLD) {
        //    write_reliables_old(client, netchan->maximumPacketLength);
        //}

        // now fill up remaining buffer space with download
        SV_WritePendingDownload(client);

        if (netchan->message.currentSize || netchan->reliableAckPending || netchan->reliableLength || retransmit) {
            currentSize = Netchan_Transmit(netchan, 0, NULL, 1, svs.realTime);
            SV_DPrintf(0, "%s: send: %" PRIz "\n", client->name, currentSize);
calctime:
            SV_CalcSendTime(client, currentSize);
        }
    }
}

/**
*	@brief	Allocates a new client message pool, and initializes all message lists.
**/
void SV_InitClientSend(client_t *newcl) {
	// Initialize lists.
    List_Init(&newcl->msg_free_list);
    List_Init(&newcl->msg_unreliable_list);
    List_Init(&newcl->msg_reliable_list);

	// Allocate pool.
    newcl->msg_pool = (MessagePacket*)SV_Malloc(sizeof(MessagePacket) * MSG_POOLSIZE); // CPP: Cast
    for (int32_t i = 0; i < MSG_POOLSIZE; i++) {
        List_Append(&newcl->msg_free_list, &newcl->msg_pool[i].entry);
    }
}

/**
*	@brief	Frees all client message packets, clears out the client message pool from ram,
*			and reinitializes the free message packet list.
**/
void SV_ShutdownClientSend(client_t *client) {
    SV_FreeAllMessagePackets(client);

    Z_Free(client->msg_pool);
    client->msg_pool = NULL;

    List_Init(&client->msg_free_list);
}

