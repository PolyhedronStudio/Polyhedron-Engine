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

#ifndef MSG_H
#define MSG_H

#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"

//===========================================================================//
// (Antique) Q2-Pro MSG Style.
//===========================================================================//
//---------------
// A trick taken from Quetoo. Use an int32_t and a float in an union to
// simplify the networking of a float.
//---------------
typedef union {
    int32_t i;
    float f;
} msg_float;

//---------------
// Player state messaging flags.
//---------------
enum PlayerStateMessageFlags {
    MSG_PS_IGNORE_VIEWANGLES    = (1 << 0),
    MSG_PS_IGNORE_DELTAANGLES   = (1 << 1),
    MSG_PS_IGNORE_PREDICTION    = (1 << 2), // Mutually exclusive with IGNORE_VIEWANGLES
};

//---------------
// Entity state messaging flags.
//---------------
enum EntityStateMessageFlags {
    MSG_ES_FORCE = (1 << 0),
    MSG_ES_NEWENTITY = (1 << 1),
    MSG_ES_FIRSTPERSON = (1 << 2),
    MSG_ES_BEAMORIGIN = (1 << 3),
};

// Write message buffer.
extern SizeBuffer   msg_write;
extern byte         msg_write_buffer[MAX_MSGLEN];

// Read message buffer.
extern SizeBuffer   msg_read;
extern byte         msg_read_buffer[MAX_MSGLEN];

//! Extern null baseline states.
extern const EntityState        nullEntityState;
extern const PlayerState        nullPlayerState;
extern const ClientMoveCommand  nullUserCmd;

void    MSG_Init(void);

void    MSG_BeginWriting(void);
void	MSG_WriteChar(int32_t c);
void	MSG_WriteByte(int32_t c);
void	MSG_WriteShort(int32_t c);
void	MSG_WriteLong(int32_t c);
void    MSG_WriteFloat(float c);
void    MSG_WriteString(const char* s);
void    MSG_WriteVector3(const vec3_t& pos);
void    MSG_WriteDeltaEntity(const EntityState* from, const EntityState* to, EntityStateMessageFlags flags);
int     MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, PlayerStateMessageFlags flags);

#if USE_CLIENT
    void MSG_WriteBits(int32_t value, int32_t bits);
    int  MSG_WriteDeltaClientMoveCommand(const ClientMoveCommand* from, const ClientMoveCommand* cmd);
#endif // USE_CLIENT

static inline void* MSG_WriteData(const void* data, size_t len)
{
    return memcpy(SZ_GetSpace(&msg_write, len), data, len);
}

static inline void MSG_FlushTo(SizeBuffer* buf)
{
    SZ_Write(buf, msg_write.data, msg_write.currentSize);
    SZ_Clear(&msg_write);
}

void    MSG_BeginReading(void);
byte*   MSG_ReadData(size_t len);
int     MSG_ReadChar(void);
int     MSG_ReadByte(void);
int     MSG_ReadShort(void);
int     MSG_ReadWord(void);
int     MSG_ReadLong(void);
float   MSG_ReadFloat(void);
size_t  MSG_ReadString(char* dest, size_t size);
size_t  MSG_ReadStringLine(char* dest, size_t size);
void    MSG_ReadDeltaClientMoveCommand(const ClientMoveCommand* from, ClientMoveCommand* cmd);
int     MSG_ParseEntityBits(int32_t* bits);
void    MSG_ParseDeltaEntity(const EntityState* from, EntityState* to, int32_t number, int32_t bits, EntityStateMessageFlags flags);
#if USE_CLIENT
    vec3_t MSG_ReadVector3(void);
    vec3_t MSG_ReadVector3(void);

    void    MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, int32_t extraflags);

    #ifdef _DEBUG
        void    MSG_ShowDeltaPlayerstateBits(int32_t flags, int32_t extraflags);
        void    MSG_ShowDeltaUsercmdBits(int32_t bits);
        void    MSG_ShowDeltaEntityBits(int32_t bits);

        const char* MSG_ServerCommandString(int32_t cmd);

        #define MSG_ShowSVC(cmd) Com_LPrintf(PRINT_DEVELOPER, "%3" PRIz ":%s\n", msg_read.readCount - 1, MSG_ServerCommandString(cmd))
    #endif // _DEBUG
#endif // USE_CLIENT


//============================================================================

/**
*   @brief Packs a bounding box by encoding it in a 32 bit unsigned int.
*   
*   Mainly used for client side prediction, 
*       8*(bits 0-4) is x/y radius
*       8*(bits 5-9) is z down distance, 8(bits10-15) is z up
*       
*       LinkEntity sets this properly
* 
*   @param[in]  mins The mins of the bounding box to pack.
*   @param[in]  maxs The maxs of the bounding box to pack.
**/
static inline int MSG_PackBoundingBox32(const vec3_t &mins, const vec3_t &maxs)
{
    // Assume that x/y are equal and symetric
    int32_t XY = Clampi(maxs[0], 1, 255);

    // Z is not symetric (Boundingbox height.)
    int32_t ZDown = Clampi(-mins[2], 1, 255);

    // And z maxs can be negative...
    int32_t ZUp = Clampi(maxs[2] + 32768, 1, 65535);

    // Return packed bounding box.
    return (ZUp << 16) | (ZDown << 8) | XY;
}

/**
*   @brief Unpacks the bounding box into the given mins & maxs vectors.
*
*   @param[in] solid The actual integer to unpack.
*   @param[out/in]  mins The mins of the bounding box to set after unpacking.
*   @param[out/in]  maxs The maxs of the bounding box to set after unpacking.
**/
static inline void MSG_UnpackBoundingBox32(int32_t solid, vec3_t& mins, vec3_t& maxs)
{
    // Unpack.
    int32_t XY = solid & 255;
    int32_t ZDown = (solid >> 8) & 255;
    int32_t ZUp = ((solid >> 16) & 65535) - 32768;

    // Store unpacked values.
    mins[0] = mins[1] = -XY;
    maxs[0] = maxs[1] = XY;
    mins[2] = -ZDown;
    maxs[2] = ZUp;
}

//
// UNUSED
//
//static inline int MSG_PackBoundingBox16(const vec3_t &mins, const vec3_t &maxs)
//{
//    int x, zd, zu;
//
//    // assume that x/y are equal and symetric
//    x = maxs[0] / 8;
//    clamp(x, 1, 31);
//
//    // z is not symetric
//    zd = -mins[2] / 8;
//    clamp(zd, 1, 31);
//
//    // and z maxs can be negative...
//    zu = (maxs[2] + 32) / 8;
//    clamp(zu, 1, 63);
//
//    return (zu << 10) | (zd << 5) | x;
//}

//
// UNUSED.
//
//static inline void MSG_UnpackBoundingBox16(int solid, vec3_t &mins, vec3_t &maxs)
//{
//    int x, zd, zu;
//
//    x = 8 * (solid & 31);
//    zd = 8 * ((solid >> 5) & 31);
//    zu = 8 * ((solid >> 10) & 63) - 32;
//
//    mins[0] = mins[1] = -x;
//    maxs[0] = maxs[1] = x;
//    mins[2] = -zd;
//    maxs[2] = zu;
//}

#endif // MSG_H
