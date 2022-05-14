/***
*
*	License here.
*
*	@file
*
*	Client/Server messaging API. (Partially borrowed from QFusion.)
*
***/
#pragma once

#include "Protocol.h"
#include "SizeBuffer.h"


//============================================================================

//enum WireType {
//    WIRE_BOOL,	// a of value of 'true' is represented by a single bit in the header
//
//    WIRE_FIXED_INT8,    // 8-bit integer
//    WIRE_FIXED_INT16,   // 16-bit integer
//    WIRE_FIXED_INT32,   // 32-bit integer
//    WIRE_FIXED_INT64,   // 64-bit integer
//
//    WIRE_FLOAT,         // 32-bit floating point value
//    WIRE_HALF_FLOAT,    // 16-bit floating point value
//
//    WIRE_ANGLE,         // 32-bit float angle value, normalized to [0..360], transmitted at half-precision
//
//    WIRE_BASE128,       // base-128 encoded unsigned integer
//    WIRE_UBASE128       // base-128 encoded signed integer
//};


//typedef struct {
//	uint8_t *data;
//	size_t maxsize;
//	size_t cursize;
//	size_t readcount;
//	bool compressed;
//} MessageBuffer;

//struct MessageBufferField {
//    //! The field offset in the POD structure.
//    int32_t     offset;
//    //! The bits. If 0, it'll treat it as sizeof(float)
//    int32_t     bits;
//    //! The count of how many there are, if more than 1, it handles it as an array.
//    int32_t     count;
//    //! The type of encoding to use "over the wire".
//    WireType    encoding;
//};

//============================================================================


// Remove later on, use this for now.
//using MessageBuffer = SizeBuffer;
//struct GameState { };


//============================================================================

/**
*
*   Message Flags.
*
**/
/**
*   Player State Messaging Flags.
**/
enum PlayerStateMessageFlags {
    //! Ignore View Angles.
    MSG_PS_IGNORE_VIEWANGLES = (1 << 0),
    //! Ignore Delta Angles.
    MSG_PS_IGNORE_DELTAANGLES = (1 << 1),
    //! Mutually exclusive with IGNORE_VIEWANGLES
    MSG_PS_IGNORE_PREDICTION = (1 << 2),
};

/**
*   Entity State Messaging Flags.
**/
enum EntityStateMessageFlags {
    //! Force it whether we got a state to delta from or not.
    MSG_ES_FORCE = (1 << 0),
    //! New entity.
    MSG_ES_NEWENTITY = (1 << 1),
    //! Tells whether the entity state that being parsed is for a firstperson perspective.
    MSG_ES_FIRSTPERSON = (1 << 2), // Helps optimizing packet data by not sending certain information to a client.
    //! Informs the parsing that we're dealing with a beam, so we should treat origins differently.
    MSG_ES_BEAMORIGIN = (1 << 3),
};

// Write message buffer.
extern SizeBuffer msg_write;
extern byte	  msg_write_buffer[MAX_MSGLEN];

// Read message buffer.
extern SizeBuffer msg_read;
extern byte	  msg_read_buffer[MAX_MSGLEN];

//! Extern null baseline states.
extern const EntityState        nullEntityState;
extern const PlayerState        nullPlayerState;
extern const ClientMoveCommand  nullUserCmd;



/**
*
*   Message Buffer functionality.
*
**/
/**
*   @details    Initialize default buffers, clearing allow overflow/underflow flags.
*
*               This is the only place where writing buffer is initialized. Writing buffer is
*               never allowed to overflow.
*
*               Reading buffer is reinitialized in many other places. Reinitializing will set
*               the allow underflow flag as appropriate.
**/
void MSG_Init(void);

static inline void* MSG_WriteData(const void* data, size_t len)
{
    return memcpy(SZ_GetSpace(&msg_write, len), data, len);
}

static inline void MSG_FlushTo(SizeBuffer* buf)
{
    SZ_Write(buf, msg_write.data, msg_write.currentSize);
    SZ_Clear(&msg_write);
}



/**
*
*   Wire Types write functionality.
*
**/
/**
*   @brief Initializes write buffer for a new write session.
**/
void MSG_BeginWriting(void);//void    MSG_WriteChar(int c);
/**
*   @brief Writes a signed 8 bit byte.
**/
void MSG_WriteInt8( int32_t c );//void    MSG_WriteByte(int c);
/**
*   @brief Writes an unsigned 8 bit byte.
**/
void MSG_WriteUint8( int32_t c );
/**
*   @brief Writes a signed 16 bit short.
**/
void MSG_WriteInt16( int32_t c );
/**
*   @brief Writes an unsigned 16 bit short.
**/
void MSG_WriteUint16( uint32_t c );
/**
*   @brief Writes a 32 bit integer.
**/
void MSG_WriteInt32( int32_t c );
/**
*   @brief Writes an unsigned 32 bit integer.
**/
void MSG_WriteUint32( uint32_t c );
/**
*   @brief Writes a 64 bit integer.
**/
void MSG_WriteInt64( int64_t c );
/**
*   @brief Writes an unsigned LEB 128(base 128 encoded) integer.
**/
void MSG_WriteUintBase128( uint64_t c );
/**
*   @brief Writes a zic-zac encoded signed integer.
**/
void MSG_WriteIntBase128( int64_t c );
/**
*   @brief Writes a half float, lesser precision. (Transfered over the wire as an uint16_t)
**/
void MSG_WriteHalfFloat( float f );
/**
*   @brief Writes a full precision float. (Transfered over the wire as an int32_t).
**/
void MSG_WriteFloat( float f );
/**
*   @brief Writes a character string.
**/
void MSG_WriteString( const char *s );
/**
*   @brief Writes a full precision vector 3, half float precision if halfFloat == true.
**/
void MSG_WriteVector3(const vec3_t& pos, bool halfFloat = false);
/**
*   @brief Writes a full precision vector 4, half float precision if halfFloat == true.
**/
void MSG_WriteVector4(const vec4_t& pos, bool halfFloat = false);
#if USE_CLIENT
    void MSG_WriteBits(int value, int bits);
#endif // USE_CLIENT.



/**
*
*   Wire Types write functionality.
*
**/
/**
*   @brief Resets read count and bitposition so we got a fresh start to read data with.
**/
void    MSG_BeginReading(void);
/**
*   @brief  After acquiring the pointer, increases readCount by length.
* 
*   @return A pointer to the current read position. If underflows are allowed, a nullptr.
**/
byte*   MSG_ReadData(size_t len);
/**
*   @return Signed 8 bit byte.
**/
int32_t MSG_ReadInt8();
/**
*   @return Unsigned 8 bit byte.
**/
int32_t MSG_ReadUint8();
/**
*   @return Signed 16 bit short.
**/
int16_t MSG_ReadInt16();
/**
*   @return Unsigned 16 bit short.
**/
uint16_t MSG_ReadUint16();
/**
*   @return 32 bit integer.
**/
int32_t MSG_ReadInt32();
/**
*   @return 32 bit unsigned integer.
**/
uint32_t MSG_ReadUint32();
/**
*   @return 64 bit integer.
**/
int64_t MSG_ReadInt64();
/**
*   @return Base 128 decoded unsigned integer.
**/
uint64_t MSG_ReadUintBase128();
/**
*   @return Zig-Zac decoded signed integer.
**/
int64_t MSG_ReadIntBase128();
/**
*   @return A half float, converted to float, keep in mind that half floats have less precision.
**/
float MSG_ReadHalfFloat();
/**
*   @return The full precision float.
**/
float MSG_ReadFloat();
/**
*   @return The full string until its end.
**/
char* MSG_ReadString();
/**
*   @return The part of the string data up till the first '\n'
**/
char* MSG_ReadStringLine();
/**
*   @brief Reads the string from message buffer, into the string buffer, up till length.
* 
*   @return Count of characters read.
**/
size_t MSG_ReadStringBuffer(char *dest, size_t size);
size_t MSG_ReadStringLineBuffer(char *dest, size_t size);

/**
*   @return Depending on halfFloat = true/false, the full floating point precision of the vector, half float otherwise.
**/
vec3_t MSG_ReadVector3(bool halfFloat = false);
/**
*   @return Depending on halfFloat = true/false, the full floating point precision of the vector, half float otherwise.
**/
vec4_t MSG_ReadVector4(bool halfFloat = false);



/**
*
*   EntityState Write/Read Functions.
*
**/
/**
*   @brief  Writes entity number, remove bit, and the byte mask to buffer.
**/
void MSG_WriteEntityNumber(int32_t number, bool remove, uint32_t byteMask);

/**
* @brief Reads out entity number of current message in the buffer.
*
* @param remove     Set to true in case a remove bit was send along the wire..
* @param byteMask   Mask containing all the bits of message data to read out.
*
* @return   The entity number.
**/
int32_t MSG_ReadEntityNumber(bool* remove, uint32_t* byteMask);

/**
*   @brief
**/
void    MSG_WriteDeltaEntityState(const EntityState* from, const EntityState* to, uint32_t entityStateMessageFlags);
/**
*   @brief
**/
void    MSG_ParseDeltaEntityState(const EntityState* from, EntityState* to, int32_t number, uint32_t byteMask, uint32_t entityStateFlags);



/**
*
*   PlayerState Write/Read Functions.
*
**/
int32_t MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t flags);
void    MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t extraflags);



/**
*
*   ClientMoveCommand Write/Read Functions.
*
**/
#if USE_CLIENT
    int  MSG_WriteDeltaClientMoveCommand(const ClientMoveCommand* from, const ClientMoveCommand* cmd);
#endif // USE_CLIENT
void    MSG_ReadDeltaClientMoveCommand(const ClientMoveCommand* from, ClientMoveCommand* cmd);

//int     MSG_ParseEntityBits(int* bits);
#if USE_CLIENT
#ifdef _DEBUG
    void    MSG_ShowDeltaPlayerstateBits(int32_t flags, int32_t extraflags);
    void    MSG_ShowDeltaUsercmdBits(int32_t bits);
    void    MSG_ShowDeltaEntityBits(uint32_t bits);

    const char* MSG_ServerCommandString(int32_t cmd);

    #define MSG_ShowSVC(cmd) Com_LPrintf(PrintType::Developer, "%3" PRIz ":%s\n", msg_read.readCount - 1, MSG_ServerCommandString(cmd))
#endif // _DEBUG
#endif // USE_CLIENT



/**
*
*   Bounding Box Encode/Decode.
*
**/
/**
*   @brief Packs a bounding box by encoding it in a 32 bit unsigned int.
*   
*   Mainly used for client side prediction, 
*       8*(bits 0-4) is x/y radius
*       8*(bits 5-9) is z down distance, 8(bits10-15) is z up
*       
*   LinkEntity sets this properly
* 
*   @param[in]  mins The mins of the bounding box to pack.
*   @param[in]  maxs The maxs of the bounding box to pack.
**/
static inline int MSG_PackBoundingBox32(const vec3_t &mins, const vec3_t &maxs)
{
    //// Assume that x/y are equal and symetric
    //int32_t XY = Clampi(maxs[0] / 8, 1, 31);

    //// Z is not symetric
    //int32_t ZDown = Clampi(- mins[2] / 8, 1, 31);

    //// And Z maxs can be negative...
    //int32_t ZUp = Clampi((maxs[2] + 32) / 8, 1, 63);

    //return (ZUp << 10) | (ZDown << 5) | XY;
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
    //// Unpack.
    //int32_t XY = 8 * (solid & 31);

    //int32_t ZDown = 8 * ((solid >> 5) & 31);
    //int32_t ZUp = 8 * ((solid >> 10) & 63) - 32;

    //// Set bbox values.
    //mins[0] = mins[1] = -XY;
    //maxs[0] = maxs[1] = XY;
    //mins[2] = -ZDown;
    //maxs[2] = ZUp;
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
//// Assume that x/y are equal and symetric
//int32_t XY = Clampi(maxs[0], 1, 255);

//// Z is not symetric (Boundingbox height.)
//int32_t ZDown = Clampi(-mins[2], 1, 255);

//// And z maxs can be negative...
//int32_t ZUp = Clampi(maxs[2] + 32768, 1, 65535);

//// Return packed bounding box.
//return (ZUp << 16) | (ZDown << 8) | XY;
//}

//
// UNUSED.
//
//static inline void MSG_UnpackBoundingBox16(int solid, vec3_t &mins, vec3_t &maxs)
//{
//// Unpack.
//int32_t XY = solid & 255;
//int32_t ZDown = (solid >> 8) & 255;
//int32_t ZUp = ((solid >> 16) & 65535) - 32768;

//// Store unpacked values.
//mins[0] = mins[1] = -XY;
//maxs[0] = maxs[1] = XY;
//mins[2] = -ZDown;
//maxs[2] = ZUp;
//}
