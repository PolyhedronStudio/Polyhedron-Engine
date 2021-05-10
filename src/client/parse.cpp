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

#include "client.h"
#include "client/gamemodule.h"
#include "shared/clgame.h"

// N&C: Cheesy hack, we need to actually make this extern in a header.
extern ClientGameExport* cge;

/*
=====================================================================

  DELTA FRAME PARSING

=====================================================================
*/

static inline void CL_ParseDeltaEntity(ServerFrame  *frame,
                                       int             newnum,
                                       EntityState  *old,
                                       int             bits)
{
    EntityState    *state;

    // suck up to MAX_EDICTS for servers that don't cap at MAX_PACKET_ENTITIES
    if (frame->numEntities >= MAX_EDICTS) {
        Com_Error(ERR_DROP, "%s: MAX_EDICTS exceeded", __func__);
    }

    state = &cl.entityStates[cl.numEntityStates & PARSE_ENTITIES_MASK];
    cl.numEntityStates++;
    frame->numEntities++;

#ifdef _DEBUG
    if (cl_shownet->integer > 2 && bits) {
        MSG_ShowDeltaEntityBits(bits);
        Com_LPrintf(PRINT_DEVELOPER, "\n");
    }
#endif

    MSG_ParseDeltaEntity(old, state, newnum, bits, cl.esFlags);

    // shuffle previous origin to old
    if (!(bits & U_OLDORIGIN) && !(state->renderfx & RenderEffects::Beam))
        VectorCopy(old->origin, state->oldOrigin);
}

static void CL_ParsePacketEntities(ServerFrame *oldframe,
                                   ServerFrame *frame)
{
    int            newnum;
    int            bits;
    EntityState    *oldstate;
    int            oldindex, oldnum;
    int i;

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
        newnum = MSG_ParseEntityBits(&bits);
        if (newnum < 0 || newnum >= MAX_EDICTS) {
            Com_Error(ERR_DROP, "%s: bad number: %d", __func__, newnum);
        }

        if (msg_read.readcount > msg_read.cursize) {
            Com_Error(ERR_DROP, "%s: read past end of message", __func__);
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

        if (bits & U_REMOVE) {
            // the entity present in oldframe is not in the current frame
            SHOWNET(2, "   remove: %i\n", newnum);
            if (oldnum != newnum) {
                Com_DPrintf("U_REMOVE: oldnum != newnum\n");
            }
            if (!oldframe) {
                Com_Error(ERR_DROP, "%s: U_REMOVE with NULL oldframe", __func__);
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
            CL_ParseDeltaEntity(frame, newnum, oldstate, bits);
            if (!bits) {
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
            CL_ParseDeltaEntity(frame, newnum, &cl.entityBaselines[newnum], bits);
            if (!bits) {
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
    bits = MSG_ReadLong();

    currentframe = bits & FRAMENUM_MASK;
    delta = bits >> FRAMENUM_BITS;

    if (delta == 31) {
        deltaframe = -1;
    } else {
        deltaframe = currentframe - delta;
    }

    bits = MSG_ReadByte();

    suppressed = bits & SUPPRESSCOUNT_MASK;
    if (suppressed & FF_CLIENTPRED) {
        // CLIENTDROP is implied, don't draw both
        suppressed &= ~FF_CLIENTDROP;
    }
    cl.frameFlags |= suppressed;

    extraflags = (extrabits << 4) | (bits >> SUPPRESSCOUNT_BITS);


    frame.number = currentframe;
    frame.delta = deltaframe;

    if (cls.netchan && cls.netchan->dropped) {
        cl.frameFlags |= FF_SERVERDROP;
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
            cl.frameFlags |= FF_BADFRAME;
        } else if (oldframe->number != deltaframe) {
            // the frame that the server did the delta from
            // is too old, so we can't reconstruct it properly.
            Com_DPrintf("%s: delta frame was never received or too old\n", __func__);
            cl.frameFlags |= FF_OLDFRAME;
        } else if (!oldframe->valid) {
            // should never happen
            Com_DPrintf("%s: delta from invalid frame\n", __func__);
            cl.frameFlags |= FF_BADFRAME;
        } else if (cl.numEntityStates - oldframe->firstEntity >
                   MAX_PARSE_ENTITIES - MAX_PACKET_ENTITIES) {
            Com_DPrintf("%s: delta entities too old\n", __func__);
            cl.frameFlags |= FF_OLDENT;
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
        cl.frameFlags |= FF_NODELTA;
    }

    // read areaBits
    length = MSG_ReadByte();
    if (length) {
        if (length < 0 || msg_read.readcount + length > msg_read.cursize) {
            Com_Error(ERR_DROP, "%s: read past end of message", __func__);
        }
        if (length > sizeof(frame.areaBits)) {
            Com_Error(ERR_DROP, "%s: invalid areaBits length", __func__);
        }
        memcpy(frame.areaBits, msg_read.data + msg_read.readcount, length);
        msg_read.readcount += length;
        frame.areaBytes = length;
    } else {
        frame.areaBytes = 0;
    }

    // MSG: !! TODO: Look at demo code and see if we can remove NETCHAN_OLD.
    //if (cls.serverProtocol <= PROTOCOL_VERSION_DEFAULT) {
    //    if (MSG_ReadByte() != svc_playerinfo) {
    //        Com_Error(ERR_DROP, "%s: not playerinfo", __func__);
    //    }
    //}

    SHOWNET(2, "%3" PRIz ":playerinfo\n", msg_read.readcount - 1);

    // parse playerstate
    bits = MSG_ReadShort();
    MSG_ParseDeltaPlayerstate(from, &frame.playerState, bits, extraflags);
#ifdef _DEBUG
    if (cl_shownet->integer > 2 && (bits || extraflags)) {
        MSG_ShowDeltaPlayerstateBits(bits, extraflags);
        Com_LPrintf(PRINT_DEVELOPER, "\n");
    }
#endif
    // parse clientNumber
    if (extraflags & EPS_CLIENTNUM) {
        frame.clientNumber = MSG_ReadByte();
    } else if (oldframe) {
        frame.clientNumber = oldframe->clientNumber;
    }

    SHOWNET(2, "%3" PRIz ":packetentities\n", msg_read.readcount - 1);

    CL_ParsePacketEntities(oldframe, &frame);

    // save the frame off in the backup array for later delta comparisons
    cl.frames[currentframe & UPDATE_MASK] = frame;

#ifdef _DEBUG
    if (cl_shownet->integer > 2) {
        int rtt = 0;
        if (cls.netchan) {
            int seq = cls.netchan->incomingAcknowledged & CMD_MASK;
            rtt = cls.realtime - cl.clientCommandHistory[seq].timeSent;
        }
        Com_LPrintf(PRINT_DEVELOPER, "%3" PRIz ":frame:%d  delta:%d  rtt:%d\n",   // CPP: String concat.
                    msg_read.readcount - 1, frame.number, frame.delta, rtt);
    }
#endif

    if (!frame.valid) {
        cl.frame.valid = false;
        return; // do not change anything
    }

    if (!frame.playerState.fov) {
        // fail out early to prevent spurious errors later
        Com_Error(ERR_DROP, "%s: bad fov", __func__);
    }

    if (cls.connectionState < ClientConnectionState::Precached)
        return;

    cl.oldframe = cl.frame;
    cl.frame = frame;

    cls.demo.frames_read++;

    if (!cls.demo.seeking)
        CL_DeltaFrame();
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
        Com_Error(ERR_DROP, "%s: bad index: %d", __func__, index);
    }

    s = cl.configstrings[index];
    maxlen = CS_SIZE(index);
    len = MSG_ReadString(s, maxlen);

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

static void CL_ParseBaseline(int index, int bits)
{
    if (index < 1 || index >= MAX_EDICTS) {
        Com_Error(ERR_DROP, "%s: bad index: %d", __func__, index);
    }
#ifdef _DEBUG
    if (cl_shownet->integer > 2) {
        MSG_ShowDeltaEntityBits(bits);
        Com_LPrintf(PRINT_DEVELOPER, "\n");
    }
#endif
    MSG_ParseDeltaEntity(NULL, &cl.entityBaselines[index], index, bits, cl.esFlags);
}

// instead of wasting space for svc_configstring and svc_spawnbaseline
// bytes, entire game state is compressed into a single stream.
static void CL_ParseGamestate(void)
{
    int        index, bits;

    while (msg_read.readcount < msg_read.cursize) {
        index = MSG_ReadShort();
        if (index == ConfigStrings::MaxConfigStrings) {
            break;
        }
        CL_ParseConfigstring(index);
    }

    while (msg_read.readcount < msg_read.cursize) {
        index = MSG_ParseEntityBits(&bits);
        if (!index) {
            break;
        }
        CL_ParseBaseline(index, bits);
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
    protocol = MSG_ReadLong();
    cl.serverCount = MSG_ReadLong();
    attractloop = MSG_ReadByte();

    Com_DPrintf("Serverdata packet received "
                "(protocol=%d, serverCount=%d, attractloop=%d)\n",
                protocol, cl.serverCount, attractloop);

    // check protocol
    if (cls.serverProtocol != protocol) {
        if (!cls.demo.playback) {
            Com_Error(ERR_DROP, "Requested protocol version %d, but server returned %d.",
                      cls.serverProtocol, protocol);
        }/* else {
            Com_Error(ERR_DROP, "Demo uses unsupported protocol version %d", protocol);
        }*/

        cls.serverProtocol = protocol;
    }

    // game directory
    len = MSG_ReadString(cl.gamedir, sizeof(cl.gamedir));
    if (len >= sizeof(cl.gamedir)) {
        Com_Error(ERR_DROP, "Oversize gamedir string");
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
    cl.clientNumber = MSG_ReadShort();

    // get the full level name
    MSG_ReadString(levelname, sizeof(levelname));

    // setup default server state
    cl.serverState = ServerState::Game;

    // MSG: !! Removed: PROTOCOL_VERSION_NAC
    //if (cls.serverProtocol == PROTOCOL_VERSION_NAC) {
    i = MSG_ReadShort();
    if (!NAC_PROTOCOL_SUPPORTED(i)) {
        Com_Error(ERR_DROP,
                    "NaC server reports unsupported protocol version %d.\n"
                    "Current client version is %d.", i, PROTOCOL_VERSION_NAC_CURRENT);
    }
    Com_DPrintf("Using minor NaC protocol version %d\n", i);
    cls.protocolVersion = i;
    
    // Parse N&C server state.
    i = MSG_ReadByte();
    Com_DPrintf("NaC server state %d\n", i);
    cl.serverState = i;


    //cl.esFlags = (EntityStateMessageFlags)(cl.esFlags | MSG_ES_UMASK); // CPP: IMPROVE: cl.esFlags |= MSG_ES_UMASK;
    cl.esFlags = (EntityStateMessageFlags)(cl.esFlags | MSG_ES_BEAMORIGIN); // CPP: IMPROVE: cl.esFlags |= MSG_ES_BEAMORIGIN;


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

    flags = MSG_ReadByte();
    if ((flags & (SND_ENT | SND_POS)) == 0)
        Com_Error(ERR_DROP, "%s: neither SND_ENT nor SND_POS set", __func__);

    snd.index = MSG_ReadByte();
    if (snd.index == -1)
        Com_Error(ERR_DROP, "%s: read past end of message", __func__);

    if (flags & SND_VOLUME)
        snd.volume = MSG_ReadByte() / 255.0f;
    else
        snd.volume = DEFAULT_SOUND_PACKET_VOLUME;

    if (flags & SND_ATTENUATION)
        snd.attenuation = MSG_ReadByte() / 64.0f;
    else
        snd.attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

    if (flags & SND_OFFSET)
        snd.timeofs = MSG_ReadByte() / 1000.0f;
    else
        snd.timeofs = 0;

    if (flags & SND_ENT) {
        // entity relative
        channel = MSG_ReadShort();
        entity = channel >> 3;
        if (entity < 0 || entity >= MAX_EDICTS)
            Com_Error(ERR_DROP, "%s: bad entity: %d", __func__, entity);
        snd.entity = entity;
        snd.channel = channel & 7;
    } else {
        snd.entity = 0;
        snd.channel = 0;
    }

    // positioned in space
    if (flags & SND_POS)
        snd.pos = MSG_ReadPosition();

    snd.flags = flags;

    SHOWNET(2, "    %s\n", cl.configstrings[ConfigStrings::Sounds+ snd.index]);
}

static void CL_ParseReconnect(void)
{
    if (cls.demo.playback) {
        Com_Error(ERR_DISCONNECT, "Server disconnected");
    }

    Com_Printf("Server disconnected, reconnecting\n");

    // free netchan now to prevent `disconnect'
    // message from being sent to server
    if (cls.netchan) {
        Netchan_Close(cls.netchan);
        cls.netchan = NULL;
    }

    CL_Disconnect(ERR_RECONNECT);

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
    netadr_t *a;
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

static void CL_ParseStuffText(void)
{
    char s[MAX_STRING_CHARS];

    MSG_ReadString(s, sizeof(s));
    SHOWNET(2, "    \"%s\"\n", s);
    Cbuf_AddText(&cl_cmdbuf, s);
}

static void CL_ParseDownload(int cmd)
{
    int size, percent, compressed;
    byte *data;

    if (!cls.download.temp[0]) {
        Com_Error(ERR_DROP, "%s: no download requested", __func__);
    }

    // read the data
    size = MSG_ReadShort();
    percent = MSG_ReadByte();
    if (size == -1) {
        CL_HandleDownload(NULL, size, percent, false);
        return;
    }

    // read optional uncompressed packet size
    if (cmd == svc_zdownload) {
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
        Com_Error(ERR_DROP, "%s: bad size: %d", __func__, size);
    }

    if (msg_read.readcount + size > msg_read.cursize) {
        Com_Error(ERR_DROP, "%s: read past end of message", __func__);
    }

    data = msg_read.data + msg_read.readcount;
    msg_read.readcount += size;

    CL_HandleDownload(data, size, percent, compressed);
}

static void CL_ParseZPacket(void)
{
#if USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
    sizebuf_t   temp;
    byte        buffer[MAX_MSGLEN];
    int         inlen, outlen;

    if (msg_read.data != msg_read_buffer) {
        Com_Error(ERR_DROP, "%s: recursively entered", __func__);
    }

    inlen = MSG_ReadWord();
    outlen = MSG_ReadWord();

    if (inlen == -1 || outlen == -1 || msg_read.readcount + inlen > msg_read.cursize) {
        Com_Error(ERR_DROP, "%s: read past end of message", __func__);
    }

    if (outlen > MAX_MSGLEN) {
        Com_Error(ERR_DROP, "%s: invalid output length", __func__);
    }

    inflateReset(&cls.z);

    cls.z.next_in = msg_read.data + msg_read.readcount;
    cls.z.avail_in = (uInt)inlen;
    cls.z.next_out = buffer;
    cls.z.avail_out = (uInt)outlen;
    if (inflate(&cls.z, Z_FINISH) != Z_STREAM_END) {
        Com_Error(ERR_DROP, "%s: inflate() failed: %s", __func__, cls.z.msg);
    }

    msg_read.readcount += inlen;

    temp = msg_read;
    SZ_Init(&msg_read, buffer, outlen);
    msg_read.cursize = outlen;

    CL_ParseServerMessage();

    msg_read = temp;
#else
    Com_Error(ERR_DROP, "Compressed server packet received, "
              "but no zlib support linked in.");
#endif // USE_ZLIB_PACKET_COMPRESSION // MSG: !! Changed from USE_ZLIB
}

static void CL_ParseSetting(void)
{
    int index q_unused;
    int value q_unused;

    index = MSG_ReadLong();
    value = MSG_ReadLong();

//    switch (index) {
//    case SVS_SOME_SETTING:
//          .....
//        break;
//    default:
//        break;
//    }
}

/*
=====================
CL_ParseServerMessage
=====================
*/
void CL_ParseServerMessage(void)
{
    int         cmd, extrabits;
    size_t      readcount;
    int         index, bits;

#ifdef _DEBUG
    if (cl_shownet->integer == 1) {
        Com_LPrintf(PRINT_DEVELOPER, "%" PRIz " ", msg_read.cursize); // CPP: String concat.
    } else if (cl_shownet->integer > 1) {
        Com_LPrintf(PRINT_DEVELOPER, "------------------\n");
    }
#endif

//
// parse the message
//
    // WATISDEZE: Inform game module about beginning of message parsing.
    CL_GM_StartServerMessage();

    while (1) {
        if (msg_read.readcount > msg_read.cursize) {
            Com_Error(ERR_DROP, "%s: read past end of server message", __func__);
        }

        readcount = msg_read.readcount;

        if ((cmd = MSG_ReadByte()) == -1) {
            SHOWNET(1, "%3" PRIz ":END OF MESSAGE\n", msg_read.readcount - 1);
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
            Com_Error(ERR_DROP, "%s: illegible server message: %d", __func__, cmd);
            break;

        case svc_nop:
            break;

        case svc_disconnect:
            Com_Error(ERR_DISCONNECT, "Server disconnected");
            break;

        case svc_reconnect:
            CL_ParseReconnect();
            return;

        case svc_stufftext:
            CL_ParseStuffText();
            break;

        case svc_serverdata:
            CL_ParseServerData();
            continue;

        case svc_configstring:
            index = MSG_ReadShort();
            CL_ParseConfigstring(index);
            break;

        case svc_sound:
            CL_ParseStartSoundPacket();
            S_ParseStartSound();
            break;

        case svc_spawnbaseline:
            index = MSG_ParseEntityBits(&bits);
            CL_ParseBaseline(index, bits);
            break;

        case svc_download:
            CL_ParseDownload(cmd);
            continue;

        case svc_frame:
            CL_ParseFrame(extrabits);
            continue;

        case svc_zpacket:
            CL_ParseZPacket();
            continue;

        case svc_zdownload:
            CL_ParseDownload(cmd);
            continue;

        case svc_gamestate:
            CL_ParseGamestate();
            continue;

        case svc_setting:
            CL_ParseSetting();
            continue;
        }

        // if recording demos, copy off protocol invariant stuff
        if (cls.demo.recording && !cls.demo.paused) {
            size_t len = msg_read.readcount - readcount;

            // it is very easy to overflow standard 1390 bytes
            // demo frame with modern servers... attempt to preserve
            // reliable messages at least, assuming they come first
            if (cls.demo.buffer.cursize + len < cls.demo.buffer.maxsize) {
                SZ_Write(&cls.demo.buffer, msg_read.data + readcount, len);
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
        Com_LPrintf(PRINT_DEVELOPER, "%" PRIz " ", msg_read.cursize); // CPP: String concat.
    } else if (cl_shownet->integer > 1) {
        Com_LPrintf(PRINT_DEVELOPER, "------------------\n");
    }
#endif

//
// parse the message
//
    while (1) {
        if (msg_read.readcount > msg_read.cursize) {
            Com_Error(ERR_DROP, "%s: read past end of server message", __func__);
        }

        if ((cmd = MSG_ReadByte()) == -1) {
            SHOWNET(1, "%3" PRIz ":END OF MESSAGE\n", msg_read.readcount - 1);
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
                Com_Error(ERR_DROP, "%s: illegible server message: %d", __func__, cmd);
            }
            break;

        case svc_nop:
            break;

        case svc_disconnect:
        case svc_reconnect:
            Com_Error(ERR_DISCONNECT, "Server disconnected");
            break;

        case svc_print:
            MSG_ReadByte();
            // fall thorugh

        case svc_centerprint:
        case svc_stufftext:
            MSG_ReadString(NULL, 0);
            break;

        case svc_configstring:
            index = MSG_ReadShort();
            CL_ParseConfigstring(index);
            break;

        case svc_sound:
            CL_ParseStartSoundPacket();
            break;

        // WatIsDeze: Movd to CGModule.
        //case svg_temp_entity:
        //    CL_ParseTEntPacket();
        //    break;

        //case svg_muzzleflash:
        //case svg_muzzleflash2:
        //    CL_ParseMuzzleFlashPacket(0);
        //    break;

        case svc_frame:
            CL_ParseFrame(extrabits);
            continue;

        // N&C: Moved to CGModule.
        //case svg_inventory:
        //    CL_ParseInventory();
        //    break;

        //case svg_layout:
        //    CL_ParseLayout();
        //    break;

        }
    }
}
