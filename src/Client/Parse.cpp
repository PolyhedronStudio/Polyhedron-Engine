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
// cl_parse.c  -- parse a message received from the server

#include "Client.h"
#include "GameModule.h"
#include "Sound/Sound.h"
#include "../Shared/CLGame.h"

// N&C: Cheesy hack, we need to actually make this extern in a header.
extern IClientGameExports* cge;

/*
=====================================================================

  DELTA FRAME PARSING

=====================================================================
*/

static inline void CL_ParseDeltaEntity(ServerFrame  *svFrame, int32_t newEntityNumber, EntityState  *oldEntityState, uint32_t byteMask) {
    EntityState *entityState = nullptr;

    // suck up to MAX_WIRED_POD_ENTITIES for servers that don't cap at MAX_PACKET_ENTITIES
    if (svFrame->numEntities >= MAX_WIRED_POD_ENTITIES) {
        Com_Error(ErrorType::Drop, "%s: MAX_WIRED_POD_ENTITIES exceeded", __func__);
    }

    entityState = &cl.entityStates[cl.numEntityStates & PARSE_ENTITIES_MASK];
    cl.numEntityStates++;
    svFrame->numEntities++;

#ifdef _DEBUG
    if (cl_shownet->integer > 2 && byteMask) {
        MSG_ShowDeltaEntityBits(byteMask);
        Com_LPrintf(PrintType::Developer, "\n");
    }
#endif

    MSG_ParseDeltaEntityState(oldEntityState, entityState, newEntityNumber, byteMask, cl.entityStateFlags);

    // Shuffle previous origin to old
    if (!(byteMask & EntityMessageBits::OldOrigin) && !(entityState->renderEffects & RenderEffects::Beam))
        entityState->oldOrigin = oldEntityState->origin;//VectorCopy(old->origin, state->oldOrigin);
}

static void CL_ParsePacketEntities(ServerFrame *oldframe, ServerFrame *frame) {
    int32_t            newnum = 0;
    uint32_t        byteMask = 0;
    EntityState    *oldstate = nullptr;
    int32_t         oldindex = 0, oldnum = 0;
    int32_t i = 0;

    frame->firstEntity = cl.numEntityStates;
    frame->numEntities = 0;

    // delta from the entities present in oldframe
    oldindex = 0;
    oldstate = NULL;
    if (!oldframe) {
        oldnum = 99999;
    } else {
        if (oldindex >= oldframe->numEntities) {
            oldnum = 99999;
        } else {
            i = oldframe->firstEntity + oldindex;
            oldstate = &cl.entityStates[i & PARSE_ENTITIES_MASK];
            oldnum = oldstate->number;
        }
    }

    while (1) {
        bool removeEntity = false;

        // Read out entity number, whether to remove it or not, and its byteMask.
        newnum = MSG_ReadEntityNumber(&removeEntity, &byteMask);

        if (newnum < 0 || newnum >= MAX_PACKET_ENTITIES) {
            Com_Error(ErrorType::Drop, "%s: bad number: %d", __func__, newnum);
        }

        if (msg_read.readCount > msg_read.currentSize) {
            Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);
        }

        if (!newnum) {
            break;
        }

        while (oldnum < newnum) {
            // one or more entities from the old packet are unchanged
            SHOWNET(3, "   unchanged: %i\n", oldnum);
            CL_ParseDeltaEntity(frame, oldnum, oldstate, 0);

            oldindex++;

            if (oldindex >= oldframe->numEntities) {
                oldnum = 99999;
            } else {
                i = oldframe->firstEntity + oldindex;
                oldstate = &cl.entityStates[i & PARSE_ENTITIES_MASK];
                oldnum = oldstate->number;
            }
        }

        //if (bits & EntityMessageBits::Remove) {
	    if (removeEntity) {
            // the entity present in oldframe is not in the current frame
            SHOWNET(2, "   remove: %i\n", newnum);
            if (oldnum != newnum) {
                Com_DPrintf("EntityMessageBits::Remove: oldnum != newnum\n");
            }
            if (!oldframe) {
                Com_Error(ErrorType::Drop, "%s: EntityMessageBits::Remove with NULL oldframe", __func__);
            }

            oldindex++;

            if (oldindex >= oldframe->numEntities) {
                oldnum = 99999;
            } else {
                i = oldframe->firstEntity + oldindex;
                oldstate = &cl.entityStates[i & PARSE_ENTITIES_MASK];
                oldnum = oldstate->number;
            }
            continue;
        }

        if (oldnum == newnum) {
            // delta from previous state
            SHOWNET(2, "   delta: %i ", newnum);
            CL_ParseDeltaEntity(frame, newnum, oldstate, byteMask);
            if (!byteMask) {
                SHOWNET(2, "\n");
            }

            oldindex++;

            if (oldindex >= oldframe->numEntities) {
                oldnum = 99999;
            } else {
                i = oldframe->firstEntity + oldindex;
                oldstate = &cl.entityStates[i & PARSE_ENTITIES_MASK];
                oldnum = oldstate->number;
            }
            continue;
        }

        if (oldnum > newnum) {
            // delta from baseline
            SHOWNET(2, "   baseline: %i ", newnum);
            CL_ParseDeltaEntity(frame, newnum, &cl.entityBaselines[newnum], byteMask);
            if (!byteMask) {
                SHOWNET(2, "\n");
            }
            continue;
        }

    }

    // any remaining entities in the old frame are copied over
    while (oldnum != 99999) {
        // one or more entities from the old packet are unchanged
        SHOWNET(3, "   unchanged: %i\n", oldnum);
        CL_ParseDeltaEntity(frame, oldnum, oldstate, 0);

        oldindex++;

        if (oldindex >= oldframe->numEntities) {
            oldnum = 99999;
        } else {
            i = oldframe->firstEntity + oldindex;
            oldstate = &cl.entityStates[i & PARSE_ENTITIES_MASK];
            oldnum = oldstate->number;
        }
    }
}

static void CL_ParseFrame(int extrabits)
{
    uint32_t bits, extraflags;
    int     currentframe, deltaframe,
            delta, suppressed;
    ServerFrame  frame, *oldframe;
    PlayerState  *from;
    int     length;

    memset(&frame, 0, sizeof(frame));

    cl.frameFlags = 0;

    extraflags = 0;
    bits = static_cast<uint32_t>(MSG_ReadInt32());//MSG_ReadLong();

    currentframe = bits & FRAMENUM_MASK;
    delta = bits >> FRAMENUM_BITS;

    if (delta == 31) {
        deltaframe = -1;
    } else {
        deltaframe = currentframe - delta;
    }

    bits = MSG_ReadUint8();//MSG_ReadByte();

    suppressed = bits & SUPPRESSCOUNT_MASK;
    if (suppressed & FrameFlags::ClientPredict) {
        // CLIENTDROP is implied, don't draw both
        suppressed &= ~FrameFlags::ClientDrop;
    }
    cl.frameFlags |= suppressed;

    extraflags = (extrabits << 4) | (bits >> SUPPRESSCOUNT_BITS);


    frame.number = currentframe;
    frame.delta = deltaframe;

    if (cls.netChannel && cls.netChannel->deltaFramePacketDrops) {
        cl.frameFlags |= FrameFlags::ServerDrop;
    }

    // if the frame is delta compressed from data that we no longer have
    // available, we must suck up the rest of the frame, but not use it, then
    // ask for a non-compressed message
    if (deltaframe > 0) {
        oldframe = &cl.frames[deltaframe & UPDATE_MASK];
        from = &oldframe->playerState;
        if (deltaframe == currentframe) {
            // old servers may cause this on map change
            Com_DPrintf("%s: delta from current frame\n", __func__);
            cl.frameFlags |= FrameFlags::BadFrame;
        } else if (oldframe->number != deltaframe) {
            // the frame that the server did the delta from
            // is too old, so we can't reconstruct it properly.
            Com_DPrintf("%s: delta frame was never received or too old\n", __func__);
            cl.frameFlags |= FrameFlags::OldFrame;
        } else if (!oldframe->valid) {
            // should never happen
            Com_DPrintf("%s: delta from invalid frame\n", __func__);
            cl.frameFlags |= FrameFlags::BadFrame;
        } else if (cl.numEntityStates - oldframe->firstEntity >
                   MAX_PARSE_ENTITIES - MAX_PACKET_ENTITIES) {
            Com_DPrintf("%s: delta entities too old\n", __func__);
            cl.frameFlags |= FrameFlags::OldEntity;
        } else {
            frame.valid = true; // valid delta parse
        }
        if (!frame.valid && cl.frame.valid && cls.demo.playback) {
            Com_DPrintf("%s: recovering broken demo\n", __func__);
            oldframe = &cl.frame;
            from = &oldframe->playerState;
            frame.valid = true;
        }
    } else {
        oldframe = NULL;
        from = NULL;
        frame.valid = true; // uncompressed frame
        cl.frameFlags |= FrameFlags::NoDeltaFrame;
    }

    // read areaBits
    length = MSG_ReadUint8();//MSG_ReadByte();
    if (length) {
        if (length < 0 || msg_read.readCount + length > msg_read.currentSize) {
            Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);
        }
        if (length > sizeof(frame.areaBits)) {
            Com_Error(ErrorType::Drop, "%s: invalid areaBits length", __func__);
        }
        memcpy(frame.areaBits, msg_read.data + msg_read.readCount, length);
        msg_read.readCount += length;
        frame.areaBytes = length;
    } else {
        frame.areaBytes = 0;
    }

    // MSG: !! TODO: Look at demo code and see if we can remove NETCHAN_OLD.
    //if (cls.serverProtocol <= PROTOCOL_VERSION_DEFAULT) {
    //    if (MSG_ReadByte() != ServerCommand::PlayerInfo) {
    //        Com_Error(ErrorType::Drop, "%s: not playerinfo", __func__);
    //    }
    //}

    SHOWNET(2, "%3" PRIz ":playerinfo\n", msg_read.readCount - 1);

    // parse playerstate
    //bits = MSG_ReadUint16();
    MSG_ParseDeltaPlayerstate(from, &frame.playerState, extraflags);
#ifdef _DEBUG
    if (cl_shownet->integer > 2 && (bits || extraflags)) {
        MSG_ShowDeltaPlayerstateBits(bits, extraflags);
        Com_LPrintf(PrintType::Developer, "\n");
    }
#endif
    // parse clientNumber
    if (extraflags & EPS_CLIENTNUM) {
        frame.clientNumber = MSG_ReadUint8();//MSG_ReadByte();
    } else if (oldframe) {
        frame.clientNumber = oldframe->clientNumber;
    }

    SHOWNET(2, "%3" PRIz ":packetentities\n", msg_read.readCount - 1);

    CL_ParsePacketEntities(oldframe, &frame);

    // save the frame off in the backup array for later delta comparisons
    cl.frames[currentframe & UPDATE_MASK] = frame;

#ifdef _DEBUG
    if (cl_shownet->integer > 2) {
        int rtt = 0;
        if (cls.netChannel) {
            int seq = cls.netChannel->incomingAcknowledged & CMD_MASK;
            rtt = cls.realtime - cl.clientCommandHistory[seq].timeSent;
        }
        Com_LPrintf(PrintType::Developer, "%3" PRIz ":frame:%d  delta:%d  rtt:%d\n",   // CPP: String concat.
                    msg_read.readCount - 1, frame.number, frame.delta, rtt);
    }
#endif

    if (!frame.valid) {
        cl.frame.valid = false;
        return; // do not change anything
    }

    if (!frame.playerState.fov) {
        // fail out early to prevent spurious errors later
        Com_Error(ErrorType::Drop, "%s: bad fov", __func__);
    }

    if (cls.connectionState < ClientConnectionState::Precached) {
        return;
	}

    cl.oldframe = cl.frame;
    cl.frame = frame;

    cls.demo.frames_read++;

    if (!cls.demo.seeking) {
        CL_DeltaFrame();
	}
}

/*
=====================================================================

  SERVER CONNECTING MESSAGES

=====================================================================
*/

static void CL_ParseConfigstring(int index)
{
    size_t  len, maxlen;
    char    *s;

    if (index < 0 || index >= ConfigStrings::MaxConfigStrings) {
        Com_Error(ErrorType::Drop, "%s: bad index: %d", __func__, index);
    }

    s = cl.configstrings[index];
    maxlen = CS_SIZE(index);
    len = MSG_ReadStringBuffer(s, maxlen);//MSG_ReadString(s, maxlen);

    SHOWNET(2, "    %d \"%s\"\n", index, s);

    if (len >= maxlen) {
        Com_WPrintf(
            "%s: index %d overflowed: %" PRIz " > %" PRIz "\n",
            __func__, index, len, maxlen - 1);
    }

    if (cls.demo.seeking) {
        Q_SetBit(cl.dcs, index);
        return;
    }

    if (cls.demo.recording && cls.demo.paused) {
        Q_SetBit(cl.dcs, index);
    }

    // do something apropriate
    CL_UpdateConfigstring(index);
}

static void CL_ParseBaseline(int32_t index, uint32_t byteMask)
{
    if (index < 1 || index >= MAX_PACKET_ENTITIES) {
        Com_Error(ErrorType::Drop, "%s: bad index: %d", __func__, index);
		return;
    }
#ifdef _DEBUG
    if (cl_shownet->integer > 2) {
        MSG_ShowDeltaEntityBits(byteMask);
        Com_LPrintf(PrintType::Developer, "\n");
    }
#endif
    MSG_ParseDeltaEntityState(NULL, &cl.entityBaselines[index], index, byteMask, cl.entityStateFlags);
}

// instead of wasting space for ServerCommand::ConfigString and ServerCommand::SpawnBaseline
// bytes, entire game state is compressed into a single stream.
static void CL_ParseGamestate(void)
{
    while (msg_read.readCount < msg_read.currentSize) {
        int16_t index = MSG_ReadInt16();//MSG_ReadShort();
        if (index == ConfigStrings::MaxConfigStrings) {
            break;
        }
        CL_ParseConfigstring(index);
    }

    while (msg_read.readCount < msg_read.currentSize) {
        //index = MSG_ParseEntityBits(&byteMask);
        bool remove = false;
		uint32_t byteMask = 0;
		const int32_t index = MSG_ReadEntityNumber(&remove, &byteMask);
        if (!index) {
            break;
        }
        CL_ParseBaseline(index, byteMask);
    }
}

static void CL_ParseServerData(void)
{
    char    levelname[MAX_QPATH];
    int     i, protocol, attractloop q_unused;
    size_t  len;

    Cbuf_Execute(&cl_cmdbuf);          // make sure any stuffed commands are done

    // wipe the ClientState struct
    CL_ClearState();

    // parse protocol version number
    protocol = MSG_ReadInt32();//MSG_ReadLong();
    cl.serverCount = MSG_ReadInt32();//MSG_ReadLong();
    attractloop = MSG_ReadUint8(); //MSG_ReadByte();

    Com_DPrintf("Serverdata packet received "
                "(protocol=%d, serverCount=%d, attractloop=%d)\n",
                protocol, cl.serverCount, attractloop);

    // check protocol
    if (cls.serverProtocol != protocol) {
        if (!cls.demo.playback) {
            Com_Error(ErrorType::Drop, "Requested protocol version %d, but server returned %d.",
                      cls.serverProtocol, protocol);
        }/* else {
            Com_Error(ErrorType::Drop, "Demo uses unsupported protocol version %d", protocol);
        }*/

        cls.serverProtocol = protocol;
    }

    // game directory
    len = MSG_ReadStringBuffer(cl.gamedir, sizeof(cl.gamedir));//MSG_ReadString(cl.gamedir, sizeof(cl.gamedir));
    if (len >= sizeof(cl.gamedir)) {
        Com_Error(ErrorType::Drop, "Oversize gamedir string");
    }

    // never allow demos to change gamedir
    // do not change gamedir if connected to local sever either,
    // as it was already done by SV_InitGame, and changing it
    // here will not work since server is now running
    if (!cls.demo.playback && !sv_running->integer) {
        // pretend it has been set by user, so that 'changed' hook
        // gets called and filesystem is restarted
        Cvar_UserSet("game", cl.gamedir);

        // protect it from modifications while we are connected
        fs_game->flags |= CVAR_ROM;
    }

    // parse player entity number
    cl.clientNumber = MSG_ReadInt16();//MSG_ReadShort();

    // get the full level name
    MSG_ReadStringBuffer(levelname, sizeof(levelname));//MSG_ReadString(levelname, sizeof(levelname));

    // setup default server state
    cl.serverState = ServerState::Game;

    // MSG: !! Removed: PROTOCOL_VERSION_POLYHEDRON
    //if (cls.serverProtocol != PROTOCOL_VERSION_POLYHEDRON) {
    i = MSG_ReadInt16();//MSG_ReadShort();
    if (!POLYHEDRON_PROTOCOL_SUPPORTED(protocol)) {
        Com_Error(ErrorType::Drop,
                    "Polyhedron server reports unsupported protocol version %d.\n"
                    "Current server/client version is %d.", protocol, PROTOCOL_VERSION_POLYHEDRON_CURRENT);
    }
        
    Com_DPrintf("Using minor Polyhedron protocol version %d\n", protocol);
    cls.protocolVersion = protocol;
            
    // Parse N&C server state.
    i = MSG_ReadUint8();//MSG_ReadByte();
    Com_DPrintf("Polyhedron server state %d\n", i);
    cl.serverState = i;


    cl.entityStateFlags = (cl.entityStateFlags | MSG_ES_BEAMORIGIN);


    if (cl.clientNumber == -1) {
        SCR_PlayCinematic(levelname);
    } else {
        // seperate the printfs so the server message can have a color
        Con_Printf(
            "\n\n"
            "\35\36\36\36\36\36\36\36\36\36\36\36"
            "\36\36\36\36\36\36\36\36\36\36\36\36"
            "\36\36\36\36\36\36\36\36\36\36\36\37"
            "\n\n");

        Com_SetColor(COLOR_ALT);
        Com_Printf("%s\n", levelname);
        Com_SetColor(COLOR_NONE);

        // make sure clientNumber is in range
        if (cl.clientNumber < 0 || cl.clientNumber >= MAX_CLIENTS) {
            cl.clientNumber = CLIENTNUM_NONE;
        }
    }
}

/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

static void CL_ParseStartSoundPacket(void)
{
    int flags, channel, entity;

    flags = MSG_ReadUint8();//MSG_ReadByte();
    if ((flags & (SoundCommandBits::Entity | SoundCommandBits::Position)) == 0)
        Com_Error(ErrorType::Drop, "%s: neither SoundCommandBits::Entity nor SoundCommandBits::Position set", __func__);

    snd.index = MSG_ReadUint8();//MSG_ReadByte();
    if (snd.index == -1)
        Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);

    if (flags & SoundCommandBits::Volume)
        snd.volume = MSG_ReadUint8() / 255.0f;//MSG_ReadByte() / 255.0f;
    else
        snd.volume = DEFAULT_SOUND_PACKET_VOLUME;

    if (flags & SoundCommandBits::Attenuation)
        snd.attenuation = MSG_ReadUint8() / 64.0f; //MSG_ReadByte() / 64.0f;
    else
        snd.attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

    if (flags & SoundCommandBits::Offset)
        snd.timeofs = MSG_ReadUint8() / 1000.0f; //MSG_ReadByte() / 1000.0f;
    else
        snd.timeofs = 0;

    if (flags & SoundCommandBits::Entity) {
        // entity relative
        channel = MSG_ReadUint16();//MSG_ReadShort();
        entity = channel >> 3;
        if (entity < 0 || entity >= MAX_WIRED_POD_ENTITIES)
            Com_Error(ErrorType::Drop, "%s: bad entity: %d", __func__, entity);
        snd.entity = entity;
        snd.channel = channel & 7;
    } else {
        snd.entity = 0;
        snd.channel = 0;
    }

    // positioned in space
    if (flags & SoundCommandBits::Position)
        snd.pos = MSG_ReadVector3(false);

    snd.flags = flags;

    SHOWNET(2, "    %s\n", cl.configstrings[ConfigStrings::Sounds+ snd.index]);
}

static void CL_ParseReconnect(void)
{
    if (cls.demo.playback) {
        Com_Error(ErrorType::Disconnect, "Server disconnected");
    }

    Com_Printf("Server disconnected, reconnecting\n");

    // free netchan now to prevent `disconnect'
    // message from being sent to server
    if (cls.netChannel) {
        Netchan_Close(cls.netChannel);
        cls.netChannel = NULL;
    }

    CL_Disconnect(ErrorType::Reconnect);

    cls.connectionState = ClientConnectionState::Challenging;
    cls.timeOfInitialConnect -= CONNECT_FAST;
    cls.connect_count = 0;

    CL_CheckForResend();
}

#if USE_AUTOREPLY
static void CL_CheckForVersion(const char *s)
{
    char *p;

    // CPP: WARNING: Cast from const char * to char*
    p = (char*)strstr(s, ": ");
    if (!p) {
        return;
    }

    if (strncmp(p + 2, "!version", 8)) {
        return;
    }

    if (cl.replyTime && cls.realtime - cl.replyTime < 120000) {
        return;
    }

    cl.replyTime = cls.realtime;
    cl.replyDelta = 1024 + (rand() & 1023);
}
#endif

// attempt to scan out an IP address in dotted-quad notation and
// add it into circular array of recent addresses
void CL_CheckForIP(const char *s)
{
    unsigned b1, b2, b3, b4, port;
    NetAdr *a;
    char *p;

    while (*s) {
        if (sscanf(s, "%3u.%3u.%3u.%3u", &b1, &b2, &b3, &b4) == 4 &&
            b1 < 256 && b2 < 256 && b3 < 256 && b4 < 256) {
            // CPP: WARNING: Cast from const char* to char*...
            p = (char*)strchr(s, ':');
            if (p) {
                port = strtoul(p + 1, NULL, 10);
                if (port < 1024 || port > 65535) {
                    break; // privileged or invalid port
                }
            } else {
                port = PORT_SERVER;
            }

            a = &cls.recent_addr[cls.recent_head++ & RECENT_MASK];
            a->type = NA_IP;
            a->ip.u8[0] = b1;
            a->ip.u8[1] = b2;
            a->ip.u8[2] = b3;
            a->ip.u8[3] = b4;
            a->port = BigShort(port);
            break;
        }

        s++;
    }
}

void CL_ClientUpdateUserinfo(cvar_t* var, from_t from) {
    // Call in straight to the GM module.
    CL_GM_ClientUpdateUserInfo(var, from);
}

static void CL_ParseStuffText(void)
{
    char s[MAX_STRING_CHARS];

    MSG_ReadStringBuffer(s, sizeof(s));//MSG_ReadString(s, sizeof(s));
    SHOWNET(2, "    \"%s\"\n", s);
    Cbuf_AddText(&cl_cmdbuf, s);
}

static void CL_ParseDownload(int cmd)
{
    int size, percent, compressed;
    byte *data;

    if (!cls.download.temp[0]) {
        Com_Error(ErrorType::Drop, "%s: no download requested", __func__);
    }

    // read the data
    size = MSG_ReadInt16();//MSG_ReadShort();
    percent = MSG_ReadUint8();//MSG_ReadByte();
    if (size == -1) {
        CL_HandleDownload(NULL, size, percent, false);
        return;
    }

    // read optional uncompressed packet size
    if (cmd == ServerCommand::ZDownload) {
        // MSG: !! ZLIB PKZ Download: Kept here in case this was part of Q2PRO protocol as well (This was related to challenging connect packets.)
        //if (cls.serverProtocol == PROTOCOL_VERSION_R1Q2) {
        //    compressed = MSG_ReadShort();
        //} else {
            compressed = -1;
        //}
    } else {
        compressed = 0;
    }

    if (size < 0) {
        Com_Error(ErrorType::Drop, "%s: bad size: %d", __func__, size);
    }

    if (msg_read.readCount + size > msg_read.currentSize) {
        Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);
    }

    data = msg_read.data + msg_read.readCount;
    msg_read.readCount += size;

    CL_HandleDownload(data, size, percent, compressed);
}

static void CL_ParseZPacket(void)
{
#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    SizeBuffer   temp;
    byte        buffer[MAX_MSGLEN];
    int         inlen, outlen;

    if (msg_read.data != msg_read_buffer) {
        Com_Error(ErrorType::Drop, "%s: recursively entered", __func__);
    }

    inlen = MSG_ReadInt16();//MSG_ReadWord();
    outlen = MSG_ReadInt16();//MSG_ReadWord();

    if (inlen == -1 || outlen == -1 || msg_read.readCount + inlen > msg_read.currentSize) {
        Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);
    }

    if (outlen > MAX_MSGLEN) {
        Com_Error(ErrorType::Drop, "%s: invalid output length", __func__);
    }

    inflateReset(&cls.z);

    cls.z.next_in = msg_read.data + msg_read.readCount;
    cls.z.avail_in = (uInt)inlen;
    cls.z.next_out = buffer;
    cls.z.avail_out = (uInt)outlen;
    if (inflate(&cls.z, Z_FINISH) != Z_STREAM_END) {
        Com_Error(ErrorType::Drop, "%s: inflate() failed: %s", __func__, cls.z.msg);
    }

    msg_read.readCount += inlen;

    temp = msg_read;
    SZ_Init(&msg_read, buffer, outlen);
    msg_read.currentSize = outlen;

    CL_ParseServerMessage();

    msg_read = temp;
#else
    Com_Error(ErrorType::Drop, "Compressed server packet received, "
              "but no zlib support linked in.");
#endif // USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
}

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage(void)
{
    int         cmd, extrabits;
    size_t      readCount;
    int         index;

#ifdef _DEBUG
    if (cl_shownet->integer == 1) {
        Com_LPrintf(PrintType::Developer, "%" PRIz " ", msg_read.currentSize); // CPP: String concat.
    } else if (cl_shownet->integer > 1) {
        Com_LPrintf(PrintType::Developer, "------------------\n");
    }
#endif

//
// parse the message
//
    // WATISDEZE: Inform game module about beginning of message parsing.
    CL_GM_StartServerMessage();

    while (1) {
        if (msg_read.readCount > msg_read.currentSize) {
            Com_Error(ErrorType::Drop, "%s: read past end of server message", __func__);
        }

        readCount = msg_read.readCount;

        if ((cmd = MSG_ReadUint8()) == -1) {//if ((cmd = MSG_ReadByte()) == -1) {
            SHOWNET(1, "%3" PRIz ":END OF MESSAGE\n", msg_read.readCount - 1);
            break;
        }

        extrabits = cmd >> SVCMD_BITS;
        cmd &= SVCMD_MASK;

#ifdef _DEBUG
        if (cl_shownet->integer > 1) {
            MSG_ShowSVC(cmd);
        }
#endif

        // other commands
        switch (cmd) {
        default:
            // WATISDEZE: Call the game module server message parsing in case the engine didn't catch any.
			if (CL_GM_ParseServerMessage (cmd)) {
				break;
            } else {
                goto badbyte;
            }
badbyte:
            Com_Error(ErrorType::Drop, "%s: illegible server message: %d", __func__, cmd);
            break;

        case ServerCommand::Padding:
            break;

        case ServerCommand::Disconnect:
            Com_Error(ErrorType::Disconnect, "Server disconnected");
            break;

        case ServerCommand::Reconnect:
            CL_ParseReconnect();
            return;

        case ServerCommand::StuffText:
            CL_ParseStuffText();
            break;

        case ServerCommand::ServerData:
            CL_ParseServerData();
            continue;

        case ServerCommand::ConfigString:
            index = MSG_ReadInt16();//MSG_ReadShort();
            CL_ParseConfigstring(index);
            break;

        case ServerCommand::Sound:
            CL_ParseStartSoundPacket();
            S_ParseStartSound();
            break;

        case ServerCommand::SpawnBaseline: {
            //index = MSG_ParseEntityBits(&bits);
            uint32_t byteMask = 0;
            bool removeEntity = false;
            index = MSG_ReadEntityNumber(&removeEntity, &byteMask);
            CL_ParseBaseline(index, byteMask);
            break;
        }

        case ServerCommand::Download:
            CL_ParseDownload(cmd);
            continue;

        case ServerCommand::Frame:
            CL_ParseFrame(extrabits);
            continue;

        case ServerCommand::ZPacket:
            CL_ParseZPacket();
            continue;

        case ServerCommand::ZDownload:
            CL_ParseDownload(cmd);
            continue;

        case ServerCommand::GameState:
            CL_ParseGamestate();
            continue;
        }

        // if recording demos, copy off protocol invariant stuff
        if (cls.demo.recording && !cls.demo.paused) {
            size_t len = msg_read.readCount - readCount;

            // it is very easy to overflow standard 1390 bytes
            // demo frame with modern servers... attempt to preserve
            // reliable messages at least, assuming they come first
            if (cls.demo.buffer.currentSize + len < cls.demo.buffer.maximumSize) {
                SZ_Write(&cls.demo.buffer, msg_read.data + readCount, len);
            } else {
                cls.demo.others_dropped++;
            }
        }

        // WATISDEZE: Inform game module about beginning of message parsing.
        CL_GM_EndServerMessage();
    }
}

/*
=====================
CL_SeekDemoMessage

A variant of ParseServerMessage that skips over non-important action messages,
used for seeking in demos.
=====================
*/
void CL_SeekDemoMessage(void)
{
    int         cmd, extrabits;
    int         index;

#ifdef _DEBUG
    if (cl_shownet->integer == 1) {
        Com_LPrintf(PrintType::Developer, "%" PRIz " ", msg_read.currentSize); // CPP: String concat.
    } else if (cl_shownet->integer > 1) {
        Com_LPrintf(PrintType::Developer, "------------------\n");
    }
#endif

//
// parse the message
//
    while (1) {
        if (msg_read.readCount > msg_read.currentSize) {
            Com_Error(ErrorType::Drop, "%s: read past end of server message", __func__);
        }

        if ((cmd = MSG_ReadUint8()) == -1) {//if ((cmd = MSG_ReadByte()) == -1) {
            SHOWNET(1, "%3" PRIz ":END OF MESSAGE\n", msg_read.readCount - 1);
            break;
        }

        extrabits = cmd >> SVCMD_BITS;
        cmd &= SVCMD_MASK;

#ifdef _DEBUG
        if (cl_shownet->integer > 1) {
            MSG_ShowSVC(cmd);
        }
#endif

        // other commands
        switch (cmd) {
        default:
            // Give the CGModule a chance to handle the command.
            if (!CL_GM_SeekDemoMessage(cmd)) {
                Com_Error(ErrorType::Drop, "%s: illegible server message: %d", __func__, cmd);
            }
            break;

        case ServerCommand::Padding:
            break;

        case ServerCommand::Disconnect:
        case ServerCommand::Reconnect:
            Com_Error(ErrorType::Disconnect, "Server disconnected");
            break;

        case ServerCommand::Print:
            MSG_ReadUint8();//MSG_ReadByte();
            // fall thorugh

        case ServerCommand::CenterPrint:
        case ServerCommand::StuffText:
            MSG_ReadStringBuffer(nullptr, 0);//MSG_ReadString(NULL, 0);
            break;

        case ServerCommand::ConfigString:
            index = MSG_ReadInt16();//MSG_ReadShort();
            CL_ParseConfigstring(index);
            break;

        case ServerCommand::Sound:
            CL_ParseStartSoundPacket();
            break;

        // WatIsDeze: Movd to CGModule.
        //case ServerGameCommand::TempEntityEvent:
        //    CL_ParseTEntPacket();
        //    break;

        //case ServerGameCommand::MuzzleFlash:
        //case ServerGameCommand::MuzzleFlash2:
        //    CL_ParseMuzzleFlashPacket(0);
        //    break;

        case ServerCommand::Frame:
            CL_ParseFrame(extrabits);
            continue;

        // N&C: Moved to CGModule.
        //case ServerGameCommand::Inventory:
        //    CL_ParseInventory();
        //    break;

        //case ServerGameCommand::Layout:
        //    CL_ParseLayout();
        //    break;

        }
    }
}
