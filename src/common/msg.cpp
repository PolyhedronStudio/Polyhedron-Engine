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

#include "shared/shared.h"
#include "common/huffman.h"
#include "common/msg.h"
#include "common/protocol.h"
#include "common/sizebuf.h"
#include "sharedgame/protocol.h"

//===========================================================================//
// Q3 MSG Style.
//===========================================================================//
static huffman_t    msgHuffmanBuffer;
static qboolean     isQ3MsgInitialized = false; // Stores whether we have initialized a message yet at all, (later on will be used for Mr Huffman)

int Q3MSGOldSize = 0;

void Q3MSG_initHuffman();

void Q3MSG_Init(msg_t* buf, byte* data, int32_t length) {
    if (!isQ3MsgInitialized) {
        Q3MSG_initHuffman();
    }
    std::memset(buf, 0, sizeof(*buf));
    buf->data = data;
    buf->maxsize = length;
}

void Q3MSG_InitOOB(msg_t* buf, byte* data, int32_t length) {
    if (!isQ3MsgInitialized) {
        Q3MSG_initHuffman();
    }
    std::memset(buf, 0, sizeof(*buf));

    buf->data = data;
    buf->maxsize = length;
    buf->oob = true;
}

void Q3MSG_Clear(msg_t* buf) {
    buf->cursize = 0;
    buf->overflowed = false;
    buf->bit = 0;					//<- in bits
}

void Q3MSG_Bitstream(msg_t* buf) {
    buf->oob = false;
}

void Q3MSG_BeginReading(msg_t* msg) {
    msg->readcount = 0;
    msg->bit = 0;
    msg->oob = false;
}

void Q3MSG_BeginReadingOOB(msg_t* msg) {
    msg->readcount = 0;
    msg->bit = 0;
    msg->oob = true;
}

void Q3MSG_Copy(msg_t* buf, byte* data, int length, msg_t* src) {
    if (length < src->cursize) {
        Com_Error(ERR_DROP, "Q3MSG_Copy: can't copy into a smaller msg_t buffer");
    }
    std::memcpy(buf, src, sizeof(msg_t));
    buf->data = data;
    std::memcpy(buf->data, src->data, src->cursize);
}

/*
=============================================================================

bit functions

=============================================================================
*/

int	overflows;

// negative bit values include signs
void MSG_WriteBits(msg_t* msg, int32_t value, int32_t bits) {
    int	i;
    //	FILE*	fp;

    Q3MSGOldSize += bits;

    // this isn't an exact overflow check, but close enough
    if (msg->maxsize - msg->cursize < 4) {
        msg->overflowed = true;
        return;
    }

    if (bits == 0 || bits < -31 || bits > 32) {
        Com_Error(ERR_DROP, "MSG_WriteBits: bad bits %i", bits);
    }

    // check for overflows
    if (bits != 32) {
        if (bits > 0) {
            if (value > ((1 << bits) - 1) || value < 0) {
                overflows++;
            }
        } else {
            int	r;

            r = 1 << (bits - 1);

            if (value > r - 1 || value < -r) {
                overflows++;
            }
        }
    }
    if (bits < 0) {
        bits = -bits;
    }
    if (msg->oob) {
        if (bits == 8) {
            msg->data[msg->cursize] = value;
            msg->cursize += 1;
            msg->bit += 8;
        } else if (bits == 16) {
            unsigned short* sp = (unsigned short*)&msg->data[msg->cursize];
            *sp = LittleShort(value);
            msg->cursize += 2;
            msg->bit += 16;
        } else if (bits == 32) {
            unsigned int* ip = (unsigned int*)&msg->data[msg->cursize];
            *ip = LittleLong(value);
            msg->cursize += 4;
            msg->bit += 8;
        } else {
            Com_Error(ERR_DROP, "can't read %d bits\n", bits);
        }
    } else {
        //		fp = fopen("c:\\netchan.bin", "a");
        value &= (0xffffffff >> (32 - bits));
        if (bits & 7) {
            int nbits;
            nbits = bits & 7;
            for (i = 0; i < nbits; i++) {
                Huff_putBit((value & 1), msg->data, &msg->bit);
                value = (value >> 1);
            }
            bits = bits - nbits;
        }
        if (bits) {
            for (i = 0; i < bits; i += 8) {
                //				fwrite(bp, 1, 1, fp);
                Huff_offsetTransmit(&msgHuffmanBuffer.compressor, (value & 0xff), msg->data, &msg->bit);
                value = (value >> 8);
            }
        }
        msg->cursize = (msg->bit >> 3) + 1;
        //		fclose(fp);
    }
}

int MSG_ReadBits(msg_t* msg, int bits) {
    int			value;
    int			get;
    qboolean	sgn;
    int			i, nbits;
    //	FILE*	fp;

    value = 0;

    if (bits < 0) {
        bits = -bits;
        sgn = true;
    } else {
        sgn = false;
    }

    if (msg->oob) {
        if (bits == 8) {
            value = msg->data[msg->readcount];
            msg->readcount += 1;
            msg->bit += 8;
        } else if (bits == 16) {
            unsigned short* sp = (unsigned short*)&msg->data[msg->readcount];
            value = LittleShort(*sp);
            msg->readcount += 2;
            msg->bit += 16;
        } else if (bits == 32) {
            unsigned int* ip = (unsigned int*)&msg->data[msg->readcount];
            value = LittleLong(*ip);
            msg->readcount += 4;
            msg->bit += 32;
        } else {
            Com_Error(ERR_DROP, "can't read %d bits\n", bits);
        }
    } else {
        nbits = 0;
        if (bits & 7) {
            nbits = bits & 7;
            for (i = 0; i < nbits; i++) {
                value |= (Huff_getBit(msg->data, &msg->bit) << i);
            }
            bits = bits - nbits;
        }
        if (bits) {
            //			fp = fopen("c:\\netchan.bin", "a");
            for (i = 0; i < bits; i += 8) {
                Huff_offsetReceive(msgHuffmanBuffer.decompressor.tree, &get, msg->data, &msg->bit);
                //				fwrite(&get, 1, 1, fp);
                value |= (get << (i + nbits));
            }
            //			fclose(fp);
        }
        msg->readcount = (msg->bit >> 3) + 1;
    }
    if (sgn) {
        if (value & (1 << (bits - 1))) {
            value |= -1 ^ ((1 << bits) - 1);
        }
    }

    return value;
}



//===========================================================================//
// (Antique) Q2-Pro MSG Style.
//===========================================================================//
/*
==============================================================================

            MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

sizebuf_t   msg_write;
byte        msg_write_buffer[MAX_MSGLEN];

sizebuf_t   msg_read;
byte        msg_read_buffer[MAX_MSGLEN];

const PackedEntity   nullEntityState = {};
const PlayerState    nullPlayerState = {};
const ClientUserCommand         nullUserCmd = {};

/*
=============
MSG_Init

Initialize default buffers, clearing allow overflow/underflow flags.

This is the only place where writing buffer is initialized. Writing buffer is
never allowed to overflow.

Reading buffer is reinitialized in many other places. Reinitializing will set
the allow underflow flag as appropriate.
=============
*/
void MSG_Init(void)
{
    SZ_TagInit(&msg_read, msg_read_buffer, MAX_MSGLEN, SZ_MSG_READ);
    SZ_TagInit(&msg_write, msg_write_buffer, MAX_MSGLEN, SZ_MSG_WRITE);
}


/*
==============================================================================

            WRITING

==============================================================================
*/

/*
=============
MSG_BeginWriting
=============
*/
void MSG_BeginWriting(void)
{
    msg_write.cursize = 0;
    msg_write.bitpos = 0;
    msg_write.overflowed = false;
}

//
//===============
// MSG_WriteChar
// 
//===============
//
void MSG_WriteChar(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < -128 || c > 127)
        Com_Error(ERR_FATAL, "MSG_WriteChar: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 1); // CPP: Cast
    buf[0] = c;
}

//
//===============
// MSG_WriteByte
// 
//===============
//
void MSG_WriteByte(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < 0 || c > 255)
        Com_Error(ERR_FATAL, "MSG_WriteByte: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 1); // CPP: Cast
    buf[0] = c;
}

//
//===============
// MSG_WriteShort
// 
//===============
//
void MSG_WriteShort(int c)
{
    byte* buf;

#ifdef PARANOID
    if (c < ((short)0x8000) || c >(short)0x7fff)
        Com_Error(ERR_FATAL, "MSG_WriteShort: range error");
#endif

    buf = (byte*)SZ_GetSpace(&msg_write, 2); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = c >> 8;
}

//
//===============
// MSG_WriteLong
// 
//===============
//
void MSG_WriteLong(int c)
{
    byte* buf;

    buf = (byte*)SZ_GetSpace(&msg_write, 4); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = (c >> 8) & 0xff;
    buf[2] = (c >> 16) & 0xff;
    buf[3] = c >> 24;
}

//
//===============
// MSG_WriteFloat
// 
// The idea is smart and taken from Quetoo, use an union for memory mapping.
// Write the float as an int32_t, use it after reading as a float.
//================
//
void MSG_WriteFloat(float c) {
    msg_float vec;
    vec.f = c;
    MSG_WriteLong(vec.i);
}

//
//===============
// MSG_WriteString
// 
//===============
//
void MSG_WriteString(const char* string)
{
    size_t length;

    if (!string) {
        MSG_WriteByte(0);
        return;
    }

    length = strlen(string);
    if (length >= MAX_NET_STRING) {
        Com_WPrintf("%s: overflow: %" PRIz " chars", __func__, length);
        MSG_WriteByte(0);
        return;
    }

    MSG_WriteData(string, length + 1);
}

//
//===============
// MSG_WriteVector3
// 
//===============
//
void MSG_WriteVector3(const vec3_t& pos)
{
    MSG_WriteFloat(pos[0]);
    MSG_WriteFloat(pos[1]);
    MSG_WriteFloat(pos[2]);
}

#if USE_CLIENT

//
//===============
// MSG_WriteDeltaUsercmd
// 
//===============
//
int MSG_WriteDeltaUsercmd(const ClientUserCommand* from, const ClientUserCommand* cmd)
{
    // Send a null message in case we had none.
    if (!from) {
        from = &nullUserCmd;
    }


    //
    // send the movement message
    //
    int32_t bits = 0;

    if (cmd->moveCommand.viewAngles[0] != from->moveCommand.viewAngles[0])
        bits |= CM_ANGLE1;
    if (cmd->moveCommand.viewAngles[1] != from->moveCommand.viewAngles[1])
        bits |= CM_ANGLE2;
    if (cmd->moveCommand.viewAngles[2] != from->moveCommand.viewAngles[2])
        bits |= CM_ANGLE3;
    if (cmd->moveCommand.forwardMove != from->moveCommand.forwardMove)
        bits |= CM_FORWARD;
    if (cmd->moveCommand.rightMove != from->moveCommand.rightMove)
        bits |= CM_SIDE;
    if (cmd->moveCommand.upMove != from->moveCommand.upMove)
        bits |= CM_UP;
    if (cmd->moveCommand.buttons != from->moveCommand.buttons)
        bits |= CM_BUTTONS;
    if (cmd->moveCommand.impulse != from->moveCommand.impulse)
        bits |= CM_IMPULSE;

    // Write out the changed bits.
    MSG_WriteByte(bits);

    if (bits & CM_ANGLE1)
        MSG_WriteFloat(cmd->moveCommand.viewAngles[0]);
    if (bits & CM_ANGLE2)
        MSG_WriteFloat(cmd->moveCommand.viewAngles[1]);
    if (bits & CM_ANGLE3)
        MSG_WriteFloat(cmd->moveCommand.viewAngles[2]);

    if (bits & CM_FORWARD)
        MSG_WriteShort(cmd->moveCommand.forwardMove);
    if (bits & CM_SIDE)
        MSG_WriteShort(cmd->moveCommand.rightMove);
    if (bits & CM_UP)
        MSG_WriteShort(cmd->moveCommand.upMove);

    if (bits & CM_BUTTONS)
        MSG_WriteByte(cmd->moveCommand.buttons);

    if (bits & CM_IMPULSE)
        MSG_WriteByte(cmd->moveCommand.impulse);

    MSG_WriteByte(cmd->moveCommand.msec);
    MSG_WriteByte(cmd->moveCommand.lightLevel);

    // (Returned bits isn't used anywhere, but might as well keep it around.)
    return bits;
}

#endif // USE_CLIENT

void MSG_PackEntity(PackedEntity* out, const EntityState* in)
{
    // allow 0 to accomodate empty entityBaselines
    if (in->number < 0 || in->number >= MAX_EDICTS)
        Com_Error(ERR_DROP, "%s: bad number: %d", __func__, in->number);

    // N&C: Full float precision.
    out->number = in->number;
    out->origin = in->origin;
    out->angles = in->angles;
    out->oldOrigin = in->oldOrigin;
    out->modelIndex = in->modelIndex;
    out->modelIndex2 = in->modelIndex2;
    out->modelIndex3 = in->modelIndex3;
    out->modelIndex4 = in->modelIndex4;
    out->skinNumber = in->skinNumber;
    out->effects = in->effects;
    out->renderEffects = in->renderEffects;
    out->solid = in->solid;
    out->frame = in->frame;
    out->sound = in->sound;
    out->eventID = in->eventID;
}

void MSG_WriteDeltaEntity(const PackedEntity* from,
    const PackedEntity* to,
    EntityStateMessageFlags          flags)
{
    uint32_t    bits, mask;

    if (!to) {
        if (!from)
            Com_Error(ERR_DROP, "%s: NULL", __func__);

        if (from->number < 1 || from->number >= MAX_EDICTS)
            Com_Error(ERR_DROP, "%s: bad number: %d", __func__, from->number);

        bits = U_REMOVE;
        if (from->number & 0xff00)
            bits |= U_NUMBER16 | U_MOREBITS1;

        MSG_WriteByte(bits & 255);
        if (bits & 0x0000ff00)
            MSG_WriteByte((bits >> 8) & 255);

        if (bits & U_NUMBER16)
            MSG_WriteShort(from->number);
        else
            MSG_WriteByte(from->number);

        return; // remove entity
    }

    if (to->number < 1 || to->number >= MAX_EDICTS)
        Com_Error(ERR_DROP, "%s: bad number: %d", __func__, to->number);

    if (!from)
        from = &nullEntityState;

    // send an update
    bits = 0;

    if (!(flags & MSG_ES_FIRSTPERSON)) {
        if (!EqualEpsilonf(to->origin[0], from->origin[0]))
            bits |= U_ORIGIN_X;
        if (!EqualEpsilonf(to->origin[1], from->origin[1]))
            bits |= U_ORIGIN_Y;
        if (!EqualEpsilonf(to->origin[2], from->origin[2]))
            bits |= U_ORIGIN_Z;

        // N&C: Full float precision.
        if (!EqualEpsilonf(to->angles[0], from->angles[0]))
            bits |= U_ANGLE_X;
        if (!EqualEpsilonf(to->angles[1], from->angles[1]))
            bits |= U_ANGLE_Y;
        if (!EqualEpsilonf(to->angles[2], from->angles[2]))
            bits |= U_ANGLE_Z;

        if (flags & MSG_ES_NEWENTITY) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->origin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->origin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->origin[2]))
                bits |= U_OLDORIGIN;
        }
    }
    
    mask = 0xffff8000;  // don't confuse old clients

    if (to->skinNumber != from->skinNumber) {
        if (to->skinNumber & mask)
            bits |= U_SKIN8 | U_SKIN16;
        else if (to->skinNumber & 0x0000ff00)
            bits |= U_SKIN16;
        else
            bits |= U_SKIN8;
    }

    if (to->frame != from->frame) {
        if (to->frame & 0xff00)
            bits |= U_FRAME16;
        else
            bits |= U_FRAME8;
    }

    if (to->effects != from->effects) {
        if (to->effects & mask)
            bits |= U_EFFECTS8 | U_EFFECTS16;
        else if (to->effects & 0x0000ff00)
            bits |= U_EFFECTS16;
        else
            bits |= U_EFFECTS8;
    }

    if (to->renderEffects != from->renderEffects) {
        if (to->renderEffects & mask)
            bits |= U_RENDERFX8 | U_RENDERFX16;
        else if (to->renderEffects & 0x0000ff00)
            bits |= U_RENDERFX16;
        else
            bits |= U_RENDERFX8;
    }

    if (to->solid != from->solid)
        bits |= U_SOLID;

    // event is not delta compressed, just 0 compressed
    if (to->eventID)
        bits |= U_EVENT;

    if (to->modelIndex != from->modelIndex)
        bits |= U_MODEL;
    if (to->modelIndex2 != from->modelIndex2)
        bits |= U_MODEL2;
    if (to->modelIndex3 != from->modelIndex3)
        bits |= U_MODEL3;
    if (to->modelIndex4 != from->modelIndex4)
        bits |= U_MODEL4;

    if (to->sound != from->sound)
        bits |= U_SOUND;

    if (to->renderEffects & RenderEffects::FrameLerp) {
        bits |= U_OLDORIGIN;
    }
    else if (to->renderEffects & RenderEffects::Beam) {
        if (flags & MSG_ES_BEAMORIGIN) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->oldOrigin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->oldOrigin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->oldOrigin[2]))
                bits |= U_OLDORIGIN;
        }
        else {
            bits |= U_OLDORIGIN;
        }
    }

    //
    // write the message
    //
    if (!bits && !(flags & MSG_ES_FORCE))
        return;     // nothing to send!

    //----------

    if (to->number & 0xff00)
        bits |= U_NUMBER16;     // number8 is implicit otherwise

    if (bits & 0xff000000)
        bits |= U_MOREBITS3 | U_MOREBITS2 | U_MOREBITS1;
    else if (bits & 0x00ff0000)
        bits |= U_MOREBITS2 | U_MOREBITS1;
    else if (bits & 0x0000ff00)
        bits |= U_MOREBITS1;

    MSG_WriteByte(bits & 255);

    if (bits & 0xff000000) {
        MSG_WriteByte((bits >> 8) & 255);
        MSG_WriteByte((bits >> 16) & 255);
        MSG_WriteByte((bits >> 24) & 255);
    }
    else if (bits & 0x00ff0000) {
        MSG_WriteByte((bits >> 8) & 255);
        MSG_WriteByte((bits >> 16) & 255);
    }
    else if (bits & 0x0000ff00) {
        MSG_WriteByte((bits >> 8) & 255);
    }

    //----------

    if (bits & U_NUMBER16)
        MSG_WriteShort(to->number);
    else
        MSG_WriteByte(to->number);

    if (bits & U_MODEL)
        MSG_WriteByte(to->modelIndex);
    if (bits & U_MODEL2)
        MSG_WriteByte(to->modelIndex2);
    if (bits & U_MODEL3)
        MSG_WriteByte(to->modelIndex3);
    if (bits & U_MODEL4)
        MSG_WriteByte(to->modelIndex4);

    if (bits & U_FRAME8)
        MSG_WriteByte(to->frame);
    else if (bits & U_FRAME16)
        MSG_WriteShort(to->frame);

    if ((bits & (U_SKIN8 | U_SKIN16)) == (U_SKIN8 | U_SKIN16))  //used for laser colors
        MSG_WriteLong(to->skinNumber);
    else if (bits & U_SKIN8)
        MSG_WriteByte(to->skinNumber);
    else if (bits & U_SKIN16)
        MSG_WriteShort(to->skinNumber);

    if ((bits & (U_EFFECTS8 | U_EFFECTS16)) == (U_EFFECTS8 | U_EFFECTS16))
        MSG_WriteLong(to->effects);
    else if (bits & U_EFFECTS8)
        MSG_WriteByte(to->effects);
    else if (bits & U_EFFECTS16)
        MSG_WriteShort(to->effects);

    if ((bits & (U_RENDERFX8 | U_RENDERFX16)) == (U_RENDERFX8 | U_RENDERFX16))
        MSG_WriteLong(to->renderEffects);
    else if (bits & U_RENDERFX8)
        MSG_WriteByte(to->renderEffects);
    else if (bits & U_RENDERFX16)
        MSG_WriteShort(to->renderEffects);

    // N&C: Full float precision.
    if (bits & U_ORIGIN_X)
        MSG_WriteFloat(to->origin[0]);
    if (bits & U_ORIGIN_Y)
        MSG_WriteFloat(to->origin[1]);
    if (bits & U_ORIGIN_Z)
        MSG_WriteFloat(to->origin[2]);

    // N&C: Full float precision.
    if (bits & U_ANGLE_X)
        MSG_WriteFloat(to->angles[0]);
    if (bits & U_ANGLE_Y)
        MSG_WriteFloat(to->angles[1]);
    if (bits & U_ANGLE_Z)
        MSG_WriteFloat(to->angles[2]);

    // N&C: Full float precision.
    if (bits & U_OLDORIGIN) {
        MSG_WriteFloat(to->oldOrigin[0]);
        MSG_WriteFloat(to->oldOrigin[1]);
        MSG_WriteFloat(to->oldOrigin[2]);
    }

    if (bits & U_SOUND)
        MSG_WriteByte(to->sound);
    if (bits & U_EVENT)
        MSG_WriteByte(to->eventID);
    if (bits & U_SOLID) {
        MSG_WriteLong(to->solid);
    }
}

int MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, msgPsFlags_t flags)
{
    int     i;
    int     pflags, eflags;
    int     statbits;

    if (!to)
        Com_Error(ERR_DROP, "%s: NULL", __func__);

    if (!from)
        from = &nullPlayerState;

    //
    // Determine what needs to be sent
    //
    pflags = 0;
    eflags = 0;

    if (to->pmove.type != from->pmove.type)
        pflags |= PS_PM_TYPE;

    if (!EqualEpsilonf(to->pmove.origin[0], from->pmove.origin[0]) ||
        !EqualEpsilonf(to->pmove.origin[1], from->pmove.origin[1]))
        pflags |= PS_PM_ORIGIN;

    if (!EqualEpsilonf(to->pmove.origin[2], from->pmove.origin[2]))
        eflags |= EPS_M_ORIGIN2;

    if (!(flags & MSG_PS_IGNORE_PREDICTION)) {
        if (!EqualEpsilonf(to->pmove.velocity[0], from->pmove.velocity[0]) ||
            !EqualEpsilonf(to->pmove.velocity[1], from->pmove.velocity[1]))
            pflags |= PS_PM_VELOCITY;

        if (!EqualEpsilonf(to->pmove.velocity[2], from->pmove.velocity[2]))
            eflags |= EPS_M_VELOCITY2;

        if (to->pmove.time != from->pmove.time)
            pflags |= PS_PM_TIME;

        if (to->pmove.flags != from->pmove.flags)
            pflags |= PS_PM_FLAGS;

        if (to->pmove.gravity != from->pmove.gravity)
            pflags |= PS_PM_GRAVITY;
    }
    else {
        // save previous state
        VectorCopy(from->pmove.velocity, to->pmove.velocity);
        to->pmove.time = from->pmove.time;
        to->pmove.flags = from->pmove.flags;
        to->pmove.gravity = from->pmove.gravity;
    }

    if (!(flags & MSG_PS_IGNORE_DELTAANGLES)) {
        if (!EqualEpsilonf(to->pmove.deltaAngles.x, from->pmove.deltaAngles.x) ||
            !EqualEpsilonf(to->pmove.deltaAngles.y, from->pmove.deltaAngles.y) ||
            !EqualEpsilonf(to->pmove.deltaAngles.z, from->pmove.deltaAngles.z))
            pflags |= PS_PM_DELTA_ANGLES;
    }
    else {
        // save previous state
        VectorCopy(from->pmove.deltaAngles, to->pmove.deltaAngles);
    }

    if (!EqualEpsilonf(to->pmove.viewOffset.x, from->pmove.viewOffset.x) ||
        !EqualEpsilonf(to->pmove.viewOffset.y, from->pmove.viewOffset.y) ||
        !EqualEpsilonf(to->pmove.viewOffset.z, from->pmove.viewOffset.z))
        pflags |= PS_PM_VIEW_OFFSET;

    if (!(flags & MSG_PS_IGNORE_VIEWANGLES)) {
        if (!EqualEpsilonf(to->pmove.viewAngles.x, from->pmove.viewAngles.x) ||
            !EqualEpsilonf(to->pmove.viewAngles.y, from->pmove.viewAngles.y) ||
            !EqualEpsilonf(to->pmove.viewAngles.z, from->pmove.viewAngles.z))
            pflags |= PS_PM_VIEW_ANGLES;
    }
    else {
        // save previous state
        to->pmove.viewAngles[0] = from->pmove.viewAngles[0];
        to->pmove.viewAngles[1] = from->pmove.viewAngles[1];
        to->pmove.viewAngles[2] = from->pmove.viewAngles[2];
    }

    if (from->kickAngles[0] != to->kickAngles[0] ||
        from->kickAngles[1] != to->kickAngles[1] ||
        from->kickAngles[2] != to->kickAngles[2])
        pflags |= PS_KICKANGLES;

    if (from->blend[0] != to->blend[0] ||
        from->blend[1] != to->blend[1] ||
        from->blend[2] != to->blend[2] ||
        from->blend[3] != to->blend[3])
        pflags |= PS_BLEND;

    if (from->pmove.stepOffset != to->pmove.stepOffset)
        pflags |= PS_PM_STEP_OFFSET;

    if (from->fov != to->fov)
        pflags |= PS_FOV;

    if (to->rdflags != from->rdflags)
        pflags |= PS_RDFLAGS;

    if (to->gunIndex != from->gunIndex)
        pflags |= PS_WEAPONINDEX;

    if (to->gunFrame != from->gunFrame)
        pflags |= PS_WEAPONFRAME;

    if (from->gunOffset[0] != to->gunOffset[0] ||
        from->gunOffset[1] != to->gunOffset[1] ||
        from->gunOffset[2] != to->gunOffset[2])
        eflags |= EPS_GUNOFFSET;

    if (from->gunAngles[0] != to->gunAngles[0] ||
        from->gunAngles[1] != to->gunAngles[1] ||
        from->gunAngles[2] != to->gunAngles[2])
        eflags |= EPS_GUNANGLES;

    statbits = 0;
    for (i = 0; i < MAX_STATS; i++)
        if (to->stats[i] != from->stats[i])
            statbits |= 1 << i;

    if (statbits)
        eflags |= EPS_STATS;

    //
    // write it
    //
    MSG_WriteShort(pflags);

    //
    // write the PlayerMoveState
    //
    if (pflags & PS_PM_TYPE)
        MSG_WriteByte(to->pmove.type);

    if (pflags & PS_PM_ORIGIN) {
        MSG_WriteFloat(to->pmove.origin[0]);
        MSG_WriteFloat(to->pmove.origin[1]);
    }

    if (eflags & EPS_M_ORIGIN2)
        MSG_WriteFloat(to->pmove.origin[2]);

    if (pflags & PS_PM_VELOCITY) {
        MSG_WriteFloat(to->pmove.velocity[0]);
        MSG_WriteFloat(to->pmove.velocity[1]);
    }

    if (eflags & EPS_M_VELOCITY2)
        MSG_WriteFloat(to->pmove.velocity[2]);

    if (pflags & PS_PM_TIME)
        MSG_WriteShort(to->pmove.time);

    if (pflags & PS_PM_FLAGS)
        MSG_WriteShort(to->pmove.flags);

    if (pflags & PS_PM_GRAVITY)
        MSG_WriteShort(to->pmove.gravity);

    if (pflags & PS_PM_DELTA_ANGLES) {
        MSG_WriteFloat(to->pmove.deltaAngles.x);
        MSG_WriteFloat(to->pmove.deltaAngles.y);
        MSG_WriteFloat(to->pmove.deltaAngles.z);
    }

    //
    // write the rest of the PlayerState
    //
    if (pflags & PS_PM_VIEW_OFFSET) {
        MSG_WriteFloat(to->pmove.viewOffset[0]);
        MSG_WriteFloat(to->pmove.viewOffset[1]);
        MSG_WriteFloat(to->pmove.viewOffset[2]);
    }

    if (pflags & PS_PM_STEP_OFFSET) {
        MSG_WriteFloat(to->pmove.stepOffset);
    }

    if (pflags & PS_PM_VIEW_ANGLES) {
        MSG_WriteFloat(to->pmove.viewAngles[0]);
        MSG_WriteFloat(to->pmove.viewAngles[1]);
        MSG_WriteFloat(to->pmove.viewAngles[2]);
    }

    if (pflags & PS_KICKANGLES) {
        MSG_WriteFloat(to->kickAngles[0]);
        MSG_WriteFloat(to->kickAngles[1]);
        MSG_WriteFloat(to->kickAngles[2]);
    }

    if (pflags & PS_WEAPONINDEX)
        MSG_WriteByte(to->gunIndex);

    if (pflags & PS_WEAPONFRAME)
        MSG_WriteLong(to->gunFrame);

    if (eflags & EPS_GUNOFFSET) {
        MSG_WriteFloat(to->gunOffset[0]);
        MSG_WriteFloat(to->gunOffset[1]);
        MSG_WriteFloat(to->gunOffset[2]);
    }

    if (eflags & EPS_GUNANGLES) {
        MSG_WriteFloat(to->gunAngles[0]);
        MSG_WriteFloat(to->gunAngles[1]);
        MSG_WriteFloat(to->gunAngles[2]);
    }

    if (pflags & PS_BLEND) {
        MSG_WriteFloat(to->blend[0]);
        MSG_WriteFloat(to->blend[1]);
        MSG_WriteFloat(to->blend[2]);
        MSG_WriteFloat(to->blend[3]);
    }

    if (pflags & PS_FOV)
        MSG_WriteFloat(to->fov);

    if (pflags & PS_RDFLAGS)
        MSG_WriteLong(to->rdflags);

    // send stats
    if (eflags & EPS_STATS) {
        MSG_WriteLong(statbits);
        for (i = 0; i < MAX_STATS; i++)
            if (statbits & (1 << i))
                MSG_WriteShort(to->stats[i]);
    }

    return eflags;
}

/*
==============================================================================

            READING

==============================================================================
*/

void MSG_BeginReading(void)
{
    msg_read.readcount = 0;
    msg_read.bitpos = 0;
}

byte* MSG_ReadData(size_t len)
{
    byte* buf = msg_read.data + msg_read.readcount;

    msg_read.readcount += len;
    msg_read.bitpos = msg_read.readcount << 3;

    if (msg_read.readcount > msg_read.cursize) {
        if (!msg_read.allowunderflow) {
            Com_Error(ERR_DROP, "%s: read past end of message", __func__);
        }
        return NULL;
    }

    return buf;
}

// returns -1 if no more characters are available
int MSG_ReadChar(void)
{
    byte* buf = MSG_ReadData(1);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (signed char)buf[0];
    }

    return c;
}

int MSG_ReadByte(void)
{
    byte* buf = MSG_ReadData(1);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (unsigned char)buf[0];
    }

    return c;
}

int MSG_ReadShort(void)
{
    byte* buf = MSG_ReadData(2);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (signed short)LittleShortMem(buf);
    }

    return c;
}

int MSG_ReadWord(void)
{
    byte* buf = MSG_ReadData(2);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = (unsigned short)LittleShortMem(buf);
    }

    return c;
}

int MSG_ReadLong(void)
{
    byte* buf = MSG_ReadData(4);
    int c;

    if (!buf) {
        c = -1;
    }
    else {
        c = LittleLongMem(buf);
    }

    return c;
}

//
//===============
// MSG_ReadFloat
// 
// The idea is smart and taken from Quetoo, use an union for memory mapping.
// Read the float as an int32_t, use the union struct trick to convert it to a float.
//================
//
float MSG_ReadFloat(void) {
    msg_float vec;
    vec.i = MSG_ReadLong();
    return vec.f;
}

size_t MSG_ReadString(char* dest, size_t size)
{
    int     c;
    size_t  len = 0;

    while (1) {
        c = MSG_ReadByte();
        if (c == -1 || c == 0) {
            break;
        }
        if (len + 1 < size) {
            *dest++ = c;
        }
        len++;
    }
    if (size) {
        *dest = 0;
    }

    return len;
}

size_t MSG_ReadStringLine(char* dest, size_t size)
{
    int     c;
    size_t  len = 0;

    while (1) {
        c = MSG_ReadByte();
        if (c == -1 || c == 0 || c == '\n') {
            break;
        }
        if (len + 1 < size) {
            *dest++ = c;
        }
        len++;
    }
    if (size) {
        *dest = 0;
    }

    return len;
}

vec3_t MSG_ReadVector3(void) {
    return vec3_t{
        MSG_ReadFloat(),
        MSG_ReadFloat(),
        MSG_ReadFloat()
    };
}

void MSG_ReadDeltaUsercmd(const ClientUserCommand* from, ClientUserCommand* to)
{
    int bits;

    if (from) {
        memcpy(to, from, sizeof(*to));
    }
    else {
        memset(to, 0, sizeof(*to));
    }

    bits = MSG_ReadByte();

    // read current angles
    if (bits & CM_ANGLE1)
        to->moveCommand.viewAngles[0] = MSG_ReadFloat();
    if (bits & CM_ANGLE2)
        to->moveCommand.viewAngles[1] = MSG_ReadFloat();
    if (bits & CM_ANGLE3)
        to->moveCommand.viewAngles[2] = MSG_ReadFloat();

    // read movement
    if (bits & CM_FORWARD)
        to->moveCommand.forwardMove = MSG_ReadShort();
    if (bits & CM_SIDE)
        to->moveCommand.rightMove = MSG_ReadShort();
    if (bits & CM_UP)
        to->moveCommand.upMove = MSG_ReadShort();

    // read buttons
    if (bits & CM_BUTTONS)
        to->moveCommand.buttons = MSG_ReadByte();

    if (bits & CM_IMPULSE)
        to->moveCommand.impulse = MSG_ReadByte();

    // read time to run command
    to->moveCommand.msec = MSG_ReadByte();

    // read the light level
    to->moveCommand.lightLevel = MSG_ReadByte();
}

#if USE_CLIENT

/*
=================
MSG_ParseEntityBits

Returns the entity number and the header bits
=================
*/
int MSG_ParseEntityBits(int* bits)
{
    int         b, total;
    int         number;

    total = MSG_ReadByte();
    if (total & U_MOREBITS1) {
        b = MSG_ReadByte();
        total |= b << 8;
    }
    if (total & U_MOREBITS2) {
        b = MSG_ReadByte();
        total |= b << 16;
    }
    if (total & U_MOREBITS3) {
        b = MSG_ReadByte();
        total |= b << 24;
    }

    if (total & U_NUMBER16)
        number = MSG_ReadShort();
    else
        number = MSG_ReadByte();

    *bits = total;

    return number;
}

/*
==================
MSG_ParseDeltaEntity

Can go from either a baseline or a previous packet_entity
==================
*/
void MSG_ParseDeltaEntity(const EntityState* from, EntityState* to, int number, int bits, EntityStateMessageFlags flags) {
    // Sanity checks.
    if (!to) {
        Com_Error(ERR_DROP, "%s: NULL", __func__);
    }

    if (number < 1 || number >= MAX_EDICTS) {
        Com_Error(ERR_DROP, "%s: bad entity number: %d", __func__, number);
    }

    // Set everything to the state we are delta'ing from
    if (!from) {
        memset(to, 0, sizeof(*to));
    } else if (to != from) {
        memcpy(to, from, sizeof(*to));
    }

    to->number = number;
    to->eventID = 0;

    if (!bits) {
        return;
    }

    // Model Indexes.
    if (bits & U_MODEL) {
        to->modelIndex = MSG_ReadByte();
    }
    if (bits & U_MODEL2) {
        to->modelIndex2 = MSG_ReadByte();
    }
    if (bits & U_MODEL3) {
        to->modelIndex3 = MSG_ReadByte();
    }
    if (bits & U_MODEL4) {
        to->modelIndex4 = MSG_ReadByte();
    }

    // Frame.
    if (bits & U_FRAME8)
        to->frame = MSG_ReadByte();
    if (bits & U_FRAME16)
        to->frame = MSG_ReadShort();

    // Skinnum.
    if ((bits & (U_SKIN8 | U_SKIN16)) == (U_SKIN8 | U_SKIN16))  //used for laser colors
        to->skinNumber = MSG_ReadLong();
    else if (bits & U_SKIN8)
        to->skinNumber = MSG_ReadByte();
    else if (bits & U_SKIN16)
        to->skinNumber = MSG_ReadWord();

    // Effects.
    if ((bits & (U_EFFECTS8 | U_EFFECTS16)) == (U_EFFECTS8 | U_EFFECTS16))
        to->effects = MSG_ReadLong();
    else if (bits & U_EFFECTS8)
        to->effects = MSG_ReadByte();
    else if (bits & U_EFFECTS16)
        to->effects = MSG_ReadWord();

    // RenderFX.
    if ((bits & (U_RENDERFX8 | U_RENDERFX16)) == (U_RENDERFX8 | U_RENDERFX16))
        to->renderEffects = MSG_ReadLong();
    else if (bits & U_RENDERFX8)
        to->renderEffects = MSG_ReadByte();
    else if (bits & U_RENDERFX16)
        to->renderEffects = MSG_ReadWord();

    // Origin.
    if (bits & U_ORIGIN_X)
        to->origin[0] = MSG_ReadFloat();
    if (bits & U_ORIGIN_Y) {
        to->origin[1] = MSG_ReadFloat();
    }
    if (bits & U_ORIGIN_Z) {
        to->origin[2] = MSG_ReadFloat();
    }

    // Angle.
    if (bits & U_ANGLE_X)
        to->angles[0] = MSG_ReadFloat();
    if (bits & U_ANGLE_Y)
        to->angles[1] = MSG_ReadFloat();
    if (bits & U_ANGLE_Z)
        to->angles[2] = MSG_ReadFloat();

    // Old Origin.
    if (bits & U_OLDORIGIN) {
        to->oldOrigin = MSG_ReadVector3(); // MSG: !! ReadPos
    }

    // Sound.
    if (bits & U_SOUND) {
        to->sound = MSG_ReadByte();
    }

    // Event.
    if (bits & U_EVENT) {
        to->eventID = MSG_ReadByte();
    }

    // Solid.
    if (bits & U_SOLID) {
        to->solid = MSG_ReadLong();
    }
}

/*
===================
MSG_ParseDeltaPlayerstate_Default
===================
*/
void MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, int flags, int extraflags) {
    int         i;
    int         statbits;

    if (!to) {
        Com_Error(ERR_DROP, "%s: NULL", __func__);
    }

    // clear to old value before delta parsing
    if (!from) {
        memset(to, 0, sizeof(*to));
    }
    else if (to != from) {
        memcpy(to, from, sizeof(*to));
    }

    //
    // parse the PlayerMoveState
    //
    // PM Type
    if (flags & PS_PM_TYPE)
        to->pmove.type = MSG_ReadByte(); // CPP: Cast

    // Origin X Y.
    if (flags & PS_PM_ORIGIN) {
        to->pmove.origin[0] = MSG_ReadFloat();
        to->pmove.origin[1] = MSG_ReadFloat();
    }

    // Origin Z.
    if (extraflags & EPS_M_ORIGIN2) {
        to->pmove.origin[2] = MSG_ReadFloat();
    }

    // Velocity X Y.
    if (flags & PS_PM_VELOCITY) {
        to->pmove.velocity[0] = MSG_ReadFloat();
        to->pmove.velocity[1] = MSG_ReadFloat();
    }

    // Velocity Z.
    if (extraflags & EPS_M_VELOCITY2) {
        to->pmove.velocity[2] = MSG_ReadFloat();
    }

    // PM Time.
    if (flags & PS_PM_TIME)
        to->pmove.time = MSG_ReadShort();

    // PM Flags.
    if (flags & PS_PM_FLAGS)
        to->pmove.flags = MSG_ReadShort();

    // PM Gravity.
    if (flags & PS_PM_GRAVITY)
        to->pmove.gravity = MSG_ReadShort();

    // PM Delta Angles.
    if (flags & PS_PM_DELTA_ANGLES) {
        to->pmove.deltaAngles.x = MSG_ReadFloat();
        to->pmove.deltaAngles.y = MSG_ReadFloat();
        to->pmove.deltaAngles.z = MSG_ReadFloat();
    }

    //
    // parse the rest of the PlayerState
    //
    // View Offset.
    if (flags & PS_PM_VIEW_OFFSET) {
        to->pmove.viewOffset.x = MSG_ReadFloat();
        to->pmove.viewOffset.y = MSG_ReadFloat();
        to->pmove.viewOffset.z = MSG_ReadFloat();
    }

    if (flags & PS_PM_STEP_OFFSET) {
        to->pmove.stepOffset = MSG_ReadFloat();
    }

    // View Angles X Y Z.
    if (flags & PS_PM_VIEW_ANGLES) {
        to->pmove.viewAngles.x = MSG_ReadFloat();
        to->pmove.viewAngles.y = MSG_ReadFloat();
        to->pmove.viewAngles.z = MSG_ReadFloat();
    }

    // Kick Angles.
    if (flags & PS_KICKANGLES) {
        to->kickAngles[0] = MSG_ReadFloat();
        to->kickAngles[1] = MSG_ReadFloat();
        to->kickAngles[2] = MSG_ReadFloat();
    }

    // Weapon Index.
    if (flags & PS_WEAPONINDEX) {
        to->gunIndex = MSG_ReadByte();
    }

    // Weapon Frame.
    if (flags & PS_WEAPONFRAME) {
        to->gunFrame = MSG_ReadLong();
    }

    // Gun Offset.
    if (extraflags & EPS_GUNOFFSET) {
        to->gunOffset[0] = MSG_ReadFloat();
        to->gunOffset[1] = MSG_ReadFloat();
        to->gunOffset[2] = MSG_ReadFloat();
    }

    // Gun Angles.
    if (extraflags & EPS_GUNANGLES) {
        to->gunAngles[0] = MSG_ReadFloat();
        to->gunAngles[1] = MSG_ReadFloat();
        to->gunAngles[2] = MSG_ReadFloat();
    }

    // Blend.
    if (flags & PS_BLEND) {
        to->blend[0] = MSG_ReadFloat();
        to->blend[1] = MSG_ReadFloat();
        to->blend[2] = MSG_ReadFloat();
        to->blend[3] = MSG_ReadFloat();
    }

    // FOV.
    if (flags & PS_FOV)
        to->fov = MSG_ReadFloat();

    // RDFlags.
    if (flags & PS_RDFLAGS)
        to->rdflags = MSG_ReadLong();

    // Parse Stats.
    if (extraflags & EPS_STATS) {
        statbits = MSG_ReadLong();
        for (i = 0; i < MAX_STATS; i++) {
            if (statbits & (1 << i)) {
                to->stats[i] = MSG_ReadShort();
            }
        }
    }

}

#endif // USE_CLIENT

int msg_hData[256] = {
250315,			// 0
41193,			// 1
6292,			// 2
7106,			// 3
3730,			// 4
3750,			// 5
6110,			// 6
23283,			// 7
33317,			// 8
6950,			// 9
7838,			// 10
9714,			// 11
9257,			// 12
17259,			// 13
3949,			// 14
1778,			// 15
8288,			// 16
1604,			// 17
1590,			// 18
1663,			// 19
1100,			// 20
1213,			// 21
1238,			// 22
1134,			// 23
1749,			// 24
1059,			// 25
1246,			// 26
1149,			// 27
1273,			// 28
4486,			// 29
2805,			// 30
3472,			// 31
21819,			// 32
1159,			// 33
1670,			// 34
1066,			// 35
1043,			// 36
1012,			// 37
1053,			// 38
1070,			// 39
1726,			// 40
888,			// 41
1180,			// 42
850,			// 43
960,			// 44
780,			// 45
1752,			// 46
3296,			// 47
10630,			// 48
4514,			// 49
5881,			// 50
2685,			// 51
4650,			// 52
3837,			// 53
2093,			// 54
1867,			// 55
2584,			// 56
1949,			// 57
1972,			// 58
940,			// 59
1134,			// 60
1788,			// 61
1670,			// 62
1206,			// 63
5719,			// 64
6128,			// 65
7222,			// 66
6654,			// 67
3710,			// 68
3795,			// 69
1492,			// 70
1524,			// 71
2215,			// 72
1140,			// 73
1355,			// 74
971,			// 75
2180,			// 76
1248,			// 77
1328,			// 78
1195,			// 79
1770,			// 80
1078,			// 81
1264,			// 82
1266,			// 83
1168,			// 84
965,			// 85
1155,			// 86
1186,			// 87
1347,			// 88
1228,			// 89
1529,			// 90
1600,			// 91
2617,			// 92
2048,			// 93
2546,			// 94
3275,			// 95
2410,			// 96
3585,			// 97
2504,			// 98
2800,			// 99
2675,			// 100
6146,			// 101
3663,			// 102
2840,			// 103
14253,			// 104
3164,			// 105
2221,			// 106
1687,			// 107
3208,			// 108
2739,			// 109
3512,			// 110
4796,			// 111
4091,			// 112
3515,			// 113
5288,			// 114
4016,			// 115
7937,			// 116
6031,			// 117
5360,			// 118
3924,			// 119
4892,			// 120
3743,			// 121
4566,			// 122
4807,			// 123
5852,			// 124
6400,			// 125
6225,			// 126
8291,			// 127
23243,			// 128
7838,			// 129
7073,			// 130
8935,			// 131
5437,			// 132
4483,			// 133
3641,			// 134
5256,			// 135
5312,			// 136
5328,			// 137
5370,			// 138
3492,			// 139
2458,			// 140
1694,			// 141
1821,			// 142
2121,			// 143
1916,			// 144
1149,			// 145
1516,			// 146
1367,			// 147
1236,			// 148
1029,			// 149
1258,			// 150
1104,			// 151
1245,			// 152
1006,			// 153
1149,			// 154
1025,			// 155
1241,			// 156
952,			// 157
1287,			// 158
997,			// 159
1713,			// 160
1009,			// 161
1187,			// 162
879,			// 163
1099,			// 164
929,			// 165
1078,			// 166
951,			// 167
1656,			// 168
930,			// 169
1153,			// 170
1030,			// 171
1262,			// 172
1062,			// 173
1214,			// 174
1060,			// 175
1621,			// 176
930,			// 177
1106,			// 178
912,			// 179
1034,			// 180
892,			// 181
1158,			// 182
990,			// 183
1175,			// 184
850,			// 185
1121,			// 186
903,			// 187
1087,			// 188
920,			// 189
1144,			// 190
1056,			// 191
3462,			// 192
2240,			// 193
4397,			// 194
12136,			// 195
7758,			// 196
1345,			// 197
1307,			// 198
3278,			// 199
1950,			// 200
886,			// 201
1023,			// 202
1112,			// 203
1077,			// 204
1042,			// 205
1061,			// 206
1071,			// 207
1484,			// 208
1001,			// 209
1096,			// 210
915,			// 211
1052,			// 212
995,			// 213
1070,			// 214
876,			// 215
1111,			// 216
851,			// 217
1059,			// 218
805,			// 219
1112,			// 220
923,			// 221
1103,			// 222
817,			// 223
1899,			// 224
1872,			// 225
976,			// 226
841,			// 227
1127,			// 228
956,			// 229
1159,			// 230
950,			// 231
7791,			// 232
954,			// 233
1289,			// 234
933,			// 235
1127,			// 236
3207,			// 237
1020,			// 238
927,			// 239
1355,			// 240
768,			// 241
1040,			// 242
745,			// 243
952,			// 244
805,			// 245
1073,			// 246
740,			// 247
1013,			// 248
805,			// 249
1008,			// 250
796,			// 251
996,			// 252
1057,			// 253
11457,			// 254
13504,			// 255
};

void Q3MSG_initHuffman() {
    int i, j;

    isQ3MsgInitialized = true;
    Huff_Init(&msgHuffmanBuffer);
    for (i = 0; i < 256; i++) {
        for (j = 0; j < msg_hData[i]; j++) {
            Huff_addRef(&msgHuffmanBuffer.compressor, (byte)i);			// Do update
            Huff_addRef(&msgHuffmanBuffer.decompressor, (byte)i);			// Do update
        }
    }
}

/*
==============================================================================

            DEBUGGING STUFF

==============================================================================
*/

#ifdef _DEBUG

#define SHOWBITS(x) Com_LPrintf(PRINT_DEVELOPER, x " ")

#if USE_CLIENT

void MSG_ShowDeltaPlayerstateBits(int flags, int extraflags)
{
#define SP(b,s) if(flags&PS_##b) SHOWBITS(s)
#define SE(b,s) if(extraflags&EPS_##b) SHOWBITS(s)
    SP(PM_TYPE, "pmove.type");
    SP(PM_ORIGIN, "pmove.origin[0,1]");
    SE(M_ORIGIN2, "pmove.origin[2]");
    SP(PM_VELOCITY, "pmove.velocity[0,1]");
    SE(M_VELOCITY2, "pmove.velocity[2]");
    SP(PM_TIME, "pmove.time");
    SP(PM_FLAGS, "pmove.flags");
    SP(PM_GRAVITY, "pmove.gravity");
    SP(PM_DELTA_ANGLES, "pmove.deltaAngles");
    SP(PM_VIEW_OFFSET, "pmove.viewOffset");
    SP(PM_VIEW_ANGLES, "pmove.viewAngles");
    SP(KICKANGLES, "kickAngles");
    SP(WEAPONINDEX, "gunIndex");
    SP(WEAPONFRAME, "gunFrame");
    SE(GUNOFFSET, "gunOffset");
    SE(GUNANGLES, "gunAngles");
    SP(BLEND, "blend");
    SP(FOV, "fov");
    SP(RDFLAGS, "rdflags");
    SE(STATS, "stats");
#undef SP
#undef SE
}

void MSG_ShowDeltaUsercmdBits(int bits)
{
    if (!bits) {
        SHOWBITS("<none>");
        return;
    }

#define S(b,s) if(bits&CM_##b) SHOWBITS(s)
    S(ANGLE1, "angle1");
    S(ANGLE2, "angle2");
    S(ANGLE3, "angle3");
    S(FORWARD, "forward");
    S(SIDE, "side");
    S(UP, "up");
    S(BUTTONS, "buttons");
    S(IMPULSE, "msec");
#undef S
}

#endif // USE_CLIENT

#if USE_CLIENT

void MSG_ShowDeltaEntityBits(int bits)
{
#define S(b,s) if(bits&U_##b) SHOWBITS(s)
    S(MODEL, "modelIndex");
    S(MODEL2, "modelIndex2");
    S(MODEL3, "modelIndex3");
    S(MODEL4, "modelIndex4");

    if (bits & U_FRAME8)
        SHOWBITS("frame8");
    if (bits & U_FRAME16)
        SHOWBITS("frame16");

    if ((bits & (U_SKIN8 | U_SKIN16)) == (U_SKIN8 | U_SKIN16))
        SHOWBITS("skinnum32");
    else if (bits & U_SKIN8)
        SHOWBITS("skinnum8");
    else if (bits & U_SKIN16)
        SHOWBITS("skinnum16");

    if ((bits & (U_EFFECTS8 | U_EFFECTS16)) == (U_EFFECTS8 | U_EFFECTS16))
        SHOWBITS("effects32");
    else if (bits & U_EFFECTS8)
        SHOWBITS("effects8");
    else if (bits & U_EFFECTS16)
        SHOWBITS("effects16");

    if ((bits & (U_RENDERFX8 | U_RENDERFX16)) == (U_RENDERFX8 | U_RENDERFX16))
        SHOWBITS("renderfx32");
    else if (bits & U_RENDERFX8)
        SHOWBITS("renderfx8");
    else if (bits & U_RENDERFX16)
        SHOWBITS("renderfx16");

    S(ORIGIN_X, "origin[0]");
    S(ORIGIN_Y, "origin[1]");
    S(ORIGIN_Z, "origin[2]");
    S(ANGLE_X, "angles[0]");
    S(ANGLE_Y, "angles[1]");
    S(ANGLE_Z, "angles[2]");
    S(OLDORIGIN, "oldOrigin");
    S(SOUND, "sound");
    S(EVENT, "event");
    S(SOLID, "solid");
#undef S
}

void MSG_ShowDeltaPlayerstateBits_Packet(int flags)
{
//#define S(b,s) if(flags&PPS_##b) SHOWBITS(s)
//    S(M_TYPE, "pmove.type");
//    S(M_ORIGIN, "pmove.origin[0,1]");
//    S(M_ORIGIN2, "pmove.origin[2]");
//    S(VIEWOFFSET, "viewOffset");
//    S(VIEWANGLES, "viewAngles[0,1]");
//    S(VIEWANGLE2, "viewAngles[2]");
//    S(KICKANGLES, "kickAngles");
//    S(WEAPONINDEX, "gunIndex");
//    S(WEAPONFRAME, "gunFrame");
//    S(GUNOFFSET, "gunOffset");
//    S(GUNANGLES, "gunAngles");
//    S(BLEND, "blend");
//    S(FOV, "fov");
//    S(RDFLAGS, "rdflags");
//    S(STATS, "stats");
//#undef S
}

const char* MSG_ServerCommandString(int cmd)
{
    switch (cmd) {
    case -1: return "END OF MESSAGE";
    default: return "UNKNOWN COMMAND";
#define S(x) case svc_##x: return "svc_" #x;
        S(bad)
            // N&C: Protocol todo: add a game callback for this...?
            //S(muzzleflash)
            //S(muzzleflash2)
            //S(temp_entity)
            //S(layout)
            //S(inventory)
            S(nop)
            S(disconnect)
            S(reconnect)
            S(sound)
            S(print)
            S(stufftext)
            S(serverdata)
            S(configstring)
            S(spawnbaseline)
            S(centerprint)
            S(download)
            S(playerinfo)
            S(packetentities)
            S(deltapacketentities)
            S(frame)
            S(zpacket)
            S(zdownload)
            S(gamestate)
#undef S
    }
}

#endif // USE_CLIENT

#endif // _DEBUG

