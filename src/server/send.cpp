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

#include "server.h"

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
        MSG_WriteByte(svc_print);
        MSG_WriteByte(PRINT_HIGH);
        MSG_WriteData(outputbuf, len);
        MSG_WriteByte(0);
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
        client->sendTime = svs.realtime;
        client->sendDelta = 0;
        return;
    }

    if (client->connectionState == ConnectionState::Spawned)
        client->messageSizes[client->frameNumber % SERVER_MESSAGES_TICKRATE] = size;

    client->sendTime = svs.realtime;
    client->sendDelta = size * 1000 / client->rate;
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/


/*
=================
SV_ClientPrintf

Sends text across to be displayed if the level passes.
NOT archived in MVD stream.
=================
*/
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

    MSG_WriteByte(svc_print);
    MSG_WriteByte(level);
    MSG_WriteData(string, len + 1);

    SV_ClientAddMessage(client, MSG_RELIABLE | MSG_CLEAR);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients.
NOT archived in MVD stream.
=================
*/
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

    MSG_WriteByte(svc_print);
    MSG_WriteByte(level);
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

    MSG_WriteByte(svc_stufftext);
    MSG_WriteData(string, len + 1);

    SV_ClientAddMessage(client, MSG_RELIABLE | MSG_CLEAR);
}

/*
=================
SV_BroadcastCommand

Sends command to all active clients.
NOT archived in MVD stream.
=================
*/
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

    MSG_WriteByte(svc_stufftext);
    MSG_WriteData(string, len + 1);

    FOR_EACH_CLIENT(client) {
        SV_ClientAddMessage(client, MSG_RELIABLE);
    }

    SZ_Clear(&msg_write);
}


/*
=================
SV_Multicast

Sends the contents of the write buffer to a subset of the clients,
then clears the write buffer.

Archived in MVD stream.

MultiCast::All    same as broadcast (origin can be NULL)
MultiCast::PVS    send to clients potentially visible from org
MultiCast::PHS    send to clients potentially hearable from org
=================
*/
void SV_Multicast(const vec3_t &origin, int32_t to)
{
    client_t    *client;
    static byte mask[VIS_MAX_BYTES];
    mleaf_t     *leaf1, *leaf2;
    int         leafnum q_unused;
    int         flags;
    vec3_t      org;

    if (!sv.cm.cache) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }

    flags = 0;

    switch (to) {
    case MultiCast::All_R:
        flags |= MSG_RELIABLE;
        // intentional fallthrough
    case MultiCast::All:
        leaf1 = NULL;
        leafnum = 0;
        break;
    case MultiCast::PHS_R:
        flags |= MSG_RELIABLE;
        // intentional fallthrough
    case MultiCast::PHS:
        leaf1 = CM_PointLeaf(&sv.cm, origin); // MATHLIB: !! Or do *origin??
        leafnum = leaf1 - sv.cm.cache->leafs;
        BSP_ClusterVis(sv.cm.cache, mask, leaf1->cluster, DVIS_PHS);
        break;
    case MultiCast::PVS_R:
        flags |= MSG_RELIABLE;
        // intentional fallthrough
    case MultiCast::PVS:
        leaf1 = CM_PointLeaf(&sv.cm, origin); // MATHLIB: !! Or do *origin??
        leafnum = leaf1 - sv.cm.cache->leafs;
        BSP_ClusterVis(sv.cm.cache, mask, leaf1->cluster, DVIS_PVS2);
        break;
    default:
        Com_Error(ERR_DROP, "SV_Multicast: bad to: %i", to);
    }

    // send the data to all relevent clients
    FOR_EACH_CLIENT(client) {
        if (client->connectionState < ConnectionState::Primed) {
            continue;
        }
        // do not send unreliables to connecting clients
        if (!(flags & MSG_RELIABLE) && (client->connectionState != ConnectionState::Spawned ||
                                        client->download.bytes || client->nodata)) {
            continue;
        }

        if (leaf1) {
            // find the client's PVS
#if 0
            PlayerState *ps = &client->edict->client->playerState;
            // N&C: FF Precision.
            VectorAdd(ps->viewOffset, ps->pmove.origin, orig);
            //VectorMA(ps->viewOffset, 0.125f, ps->pmove.origin, org);
#else
            // FIXME: for some strange reason, game code assumes the server
            // uses entity origin for PVS/PHS culling, not the view origin
            org = client->edict->state.origin; // VectorCopy(client->edict->state.origin, org);
#endif
            leaf2 = CM_PointLeaf(&sv.cm, org);
            if (!CM_AreasConnected(&sv.cm, leaf1->area, leaf2->area))
                continue;
            if (leaf2->cluster == -1)
                continue;
            if (!Q_IsBitSet(mask, leaf2->cluster))
                continue;
        }

        SV_ClientAddMessage(client, flags);
    }

    // clear the buffer
    SZ_Clear(&msg_write);
}

static qboolean compress_message(client_t *client, int flags)
{
#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    byte    buffer[MAX_MSGLEN];

    if (!(flags & MSG_COMPRESS))
        return false;

    if (!client->has_zlib)
        return false;

    // compress only sufficiently large layouts
    if (msg_write.currentSize < client->netchan->maximumPacketLength / 2)
        return false;

    deflateReset(&svs.z);
    svs.z.next_in = msg_write.data;
    svs.z.avail_in = (uInt)msg_write.currentSize;
    svs.z.next_out = buffer + 5;
    svs.z.avail_out = (uInt)(MAX_MSGLEN - 5);

    if (deflate(&svs.z, Z_FINISH) != Z_STREAM_END)
        return false;

    buffer[0] = svc_zpacket;
    buffer[1] = svs.z.total_out & 255;
    buffer[2] = (svs.z.total_out >> 8) & 255;
    buffer[3] = msg_write.currentSize & 255;
    buffer[4] = (msg_write.currentSize >> 8) & 255;

    SV_DPrintf(0, "%s: comp: %lu into %lu\n",
               client->name, svs.z.total_in, svs.z.total_out + 5);

    if (svs.z.total_out + 5 > msg_write.currentSize)
        return false;

    client->AddMessage(client, buffer, svs.z.total_out + 5,
                       (flags & MSG_RELIABLE) ? true : false);
    return true;
#else
    return false;
#endif
}

/*
=======================
SV_ClientAddMessage

Adds contents of the current write buffer to client's message list.
Does NOT clean the buffer for multicast delivery purpose,
unless told otherwise.
=======================
*/
void SV_ClientAddMessage(client_t *client, int flags)
{
    SV_DPrintf(1, "Added %sreliable message to %s: %" PRIz " bytes\n",
               (flags & MSG_RELIABLE) ? "" : "un", client->name, msg_write.currentSize);

    if (!msg_write.currentSize) {
        return;
    }

    if (compress_message(client, flags)) {
        goto clear;
    }

    client->AddMessage(client, msg_write.data, msg_write.currentSize,
                       (flags & MSG_RELIABLE) ? true : false);

clear:
    if (flags & MSG_CLEAR) {
        SZ_Clear(&msg_write);
    }
}

/*
===============================================================================

FRAME UPDATES - COMMON

===============================================================================
*/

static inline void free_msg_packet(client_t *client, MessagePacket *msg)
{
    List_Remove(&msg->entry);

    if (msg->currentSize > MSG_TRESHOLD) {
        if (msg->currentSize > client->msg_dynamic_bytes) {
            Com_Error(ERR_FATAL, "%s: bad packet size", __func__);
        }
        client->msg_dynamic_bytes -= msg->currentSize;
        Z_Free(msg);
    } else {
        List_Insert(&client->msg_free_list, &msg->entry);
    }
}

#define FOR_EACH_MSG_SAFE(list) \
    LIST_FOR_EACH_SAFE(MessagePacket, msg, next, list, entry)
#define MSG_FIRST(list) \
    LIST_FIRST(MessagePacket, list, entry)

static void free_all_messages(client_t *client)
{
    MessagePacket *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        free_msg_packet(client, msg);
    }
    FOR_EACH_MSG_SAFE(&client->msg_reliable_list) {
        free_msg_packet(client, msg);
    }
    client->msg_unreliable_bytes = 0;
    client->msg_dynamic_bytes = 0;
}

static void add_msg_packet(client_t    *client,
                           byte        *data,
                           size_t      len,
                           qboolean    reliable)
{
    MessagePacket    *msg;

    if (!client->msg_pool) {
        return; // already dropped
    }

    if (len > MSG_TRESHOLD) {
        if (len > MAX_MSGLEN) {
            Com_Error(ERR_FATAL, "%s: oversize packet", __func__);
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

    memcpy(msg->data, data, len);
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
        free_all_messages(client);
        SV_DropClient(client, "reliable queue overflowed");
    }
}

// check if this entity is present in current client frame
static qboolean check_entity(client_t *client, int entnum)
{
    ClientFrame *frame;
    unsigned i, j;

    frame = &client->frames[client->frameNumber & UPDATE_MASK];

    for (i = 0; i < frame->num_entities; i++) {
        j = (frame->first_entity + i) % svs.num_entities;
        if (svs.entities[j].number == entnum) {
            return true;
        }
    }

    return false;
}

// sounds reliative to entities are handled specially
static void emit_snd(client_t *client, MessagePacket *msg)
{
    int flags, entnum;

    entnum = msg->sendchan >> 3;
    flags = msg->flags;

    // check if position needs to be explicitly sent
    if (!(flags & SND_POS) && !check_entity(client, entnum)) {
        SV_DPrintf(0, "Forcing position on entity %d for %s\n",
                   entnum, client->name);
        flags |= SND_POS;   // entity is not present in frame
    }

    MSG_WriteByte(svc_sound);
    MSG_WriteByte(flags);
    MSG_WriteByte(msg->index);

    if (flags & SND_VOLUME)
        MSG_WriteByte(msg->volume);
    if (flags & SND_ATTENUATION)
        MSG_WriteByte(msg->attenuation);
    if (flags & SND_OFFSET)
        MSG_WriteByte(msg->timeofs);

    MSG_WriteShort(msg->sendchan);

    if (flags & SND_POS) {
        MSG_WriteVector3(msg->pos);
    }
}

static inline void write_snd(client_t *client, MessagePacket *msg, size_t maximumSize)
{
    // if this msg fits, write it
    if (msg_write.currentSize + MAX_SOUND_PACKET <= maximumSize) {
        emit_snd(client, msg);
    }
    List_Remove(&msg->entry);
    List_Insert(&client->msg_free_list, &msg->entry);
}

static inline void write_msg(client_t *client, MessagePacket *msg, size_t maximumSize)
{
    // if this msg fits, write it
    if (msg_write.currentSize + msg->currentSize <= maximumSize) {
        MSG_WriteData(msg->data, msg->currentSize);
    }
    free_msg_packet(client, msg);
}

static inline void write_unreliables(client_t *client, size_t maximumSize)
{
    MessagePacket    *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        if (msg->currentSize) {
            write_msg(client, msg, maximumSize);
        } else {
            write_snd(client, msg, maximumSize);
        }
    }
}

/*
===============================================================================

FRAME UPDATES 

===============================================================================
*/

static void SV_AddMessage(client_t *client, byte *data,
                            size_t len, qboolean reliable)
{
//    if (reliable) {
        // don't packetize, netchan level will do fragmentation as needed
        SZ_Write(&client->netchan->message, data, len);
    //} else {
    //    // still have to packetize, relative sounds need special processing
    //    add_msg_packet(client, data, len, false);
    //}
}

static void SV_WriteDatagram(client_t *client)
{
    size_t currentSize;

    // send over all the relevant EntityState
    // and the PlayerState
    client->WriteFrame(client);

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
        write_unreliables(client, msg_write.maximumSize);
    }

#ifdef _DEBUG
    if (sv_pad_packets->integer) {
        size_t pad = msg_write.currentSize + sv_pad_packets->integer;

        if (pad > msg_write.maximumSize) {
            pad = msg_write.maximumSize;
        }
        for (; pad > 0; pad--) {
            MSG_WriteByte(svc_nop);
        }
    }
#endif

    // send the datagram
    currentSize = Netchan_Transmit(client->netchan,
                                        msg_write.currentSize,
                                        msg_write.data,
                                        client->numpackets);

    // record the size for rate estimation
    SV_CalcSendTime(client, currentSize);

    // clear the write buffer
    SZ_Clear(&msg_write);
}


/*
===============================================================================

COMMON STUFF

===============================================================================
*/

static void finish_frame(client_t *client)
{
    MessagePacket *msg, *next;

    FOR_EACH_MSG_SAFE(&client->msg_unreliable_list) {
        free_msg_packet(client, msg);
    }
    client->msg_unreliable_bytes = 0;
}

/*
=======================
SV_SendClientMessages

Called each game frame, sends svc_frame messages to spawned clients only.
Clients in earlier connection state are handled in SV_SendAsyncPackets.
=======================
*/
void SV_SendClientMessages(void)
{
    client_t    *client;
    size_t      currentSize;

    // send a message to each connected client
    FOR_EACH_CLIENT(client) {
        if (client->connectionState != ConnectionState::Spawned || client->download.bytes || client->nodata)
            goto finish;

        if (!SV_CLIENTSYNC(client))
            continue;

        // if the reliable message overflowed,
        // drop the client (should never happen)
        if (client->netchan->message.overflowed) {
            SZ_Clear(&client->netchan->message);
            SV_DropClient(client, "reliable message overflowed");
            goto finish;
        }

        // don't overrun bandwidth
        if (SV_RateDrop(client))
            goto advance;

        // don't write any frame data until all fragments are sent
        if (client->netchan->fragmentPending) {
            client->frameFlags |= FrameFlags::Suppressed;
            currentSize = Netchan_TransmitNextFragment(client->netchan);
            SV_CalcSendTime(client, currentSize);
            goto advance;
        }

        // build the new frame and write it
        SV_BuildClientFrame(client);
        client->WriteDatagram(client);

advance:
        // advance for next frame
        client->frameNumber++;

finish:
        // clear all unreliable messages still left
        finish_frame(client);
    }
}

static void write_pending_download(client_t *client)
{
    SizeBuffer   *buf;
    size_t      remaining;
    int         chunk, percent;

    if (!client->download.bytes)
        return;

    if (!client->download.isPending)
        return;

    if (client->netchan->reliableLength)
        return;

    buf = &client->netchan->message;
    if (buf->currentSize > client->netchan->maximumPacketLength)
        return;

    remaining = client->netchan->maximumPacketLength - buf->currentSize;
    if (remaining <= 4)
        return;

    chunk = client->download.fileSize - client->download.bytesSent;
    if (chunk > remaining - 4)
        chunk = remaining - 4;

    client->download.isPending = false;
    client->download.bytesSent += chunk;

    percent = client->download.bytesSent * 100 / client->download.fileSize;

    SZ_WriteByte(buf, client->download.command);
    SZ_WriteShort(buf, chunk);
    SZ_WriteByte(buf, percent);
    SZ_Write(buf, client->download.bytes + client->download.bytesSent - chunk, chunk);

    if (client->download.bytesSent == client->download.fileSize) {
        SV_CloseDownload(client);
    }
}

/*
==================
SV_SendAsyncPackets

If the client is just connecting, it is pointless to wait another 100ms
before sending next command and/or reliable acknowledge, send it as soon
as client rate limit allows.

For spawned clients, this is not used, as we are forced to send svc_frame
packets synchronously with game DLL ticks.
==================
*/
void SV_SendAsyncPackets(void)
{
    qboolean    retransmit;
    client_t    *client;
    NetChannel   *netchan;
    size_t      currentSize;

    FOR_EACH_CLIENT(client) {
        // don't overrun bandwidth
        if (svs.realtime - client->sendTime < client->sendDelta) {
            continue;
        }

        netchan = client->netchan;

        // make sure all fragments are transmitted first
        if (netchan->fragmentPending) {
            currentSize = Netchan_TransmitNextFragment(netchan);
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
        write_pending_download(client);

        if (netchan->message.currentSize || netchan->reliableAckPending ||
            netchan->reliableLength || retransmit) {
            currentSize = Netchan_Transmit(netchan, 0, NULL, 1);
            SV_DPrintf(0, "%s: send: %" PRIz "\n", client->name, currentSize);
calctime:
            SV_CalcSendTime(client, currentSize);
        }
    }
}

void SV_InitClientSend(client_t *newcl)
{
    int i;

    List_Init(&newcl->msg_free_list);
    List_Init(&newcl->msg_unreliable_list);
    List_Init(&newcl->msg_reliable_list);

    newcl->msg_pool = (MessagePacket*)SV_Malloc(sizeof(MessagePacket) * MSG_POOLSIZE); // CPP: Cast
    for (i = 0; i < MSG_POOLSIZE; i++) {
        List_Append(&newcl->msg_free_list, &newcl->msg_pool[i].entry);
    }

    // setup protocol
//    if (newcl->netchan->type == NETCHAN_NEW) {
        newcl->AddMessage = SV_AddMessage;
        newcl->WriteDatagram = SV_WriteDatagram;
    //} else {
    //    newcl->AddMessage = add_message_old;
    //    newcl->WriteDatagram = write_datagram_old;
    //}
}

void SV_ShutdownClientSend(client_t *client)
{
    free_all_messages(client);

    Z_Free(client->msg_pool);
    client->msg_pool = NULL;

    List_Init(&client->msg_free_list);
}

