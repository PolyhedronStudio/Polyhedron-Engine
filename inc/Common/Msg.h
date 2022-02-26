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

#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"


//============================================================================

enum WireType {
    WIRE_BOOL,	// a of value of 'true' is represented by a single bit in the header

    WIRE_FIXED_INT8,    // 8-bit integer
    WIRE_FIXED_INT16,   // 16-bit integer
    WIRE_FIXED_INT32,   // 32-bit integer
    WIRE_FIXED_INT64,   // 64-bit integer

    WIRE_FLOAT,         // 32-bit floating point value
    WIRE_HALF_FLOAT,    // 16-bit floating point value

    WIRE_ANGLE,         // 32-bit float angle value, normalized to [0..360], transmitted at half-precision

    WIRE_BASE128,       // base-128 encoded unsigned integer
    WIRE_UBASE128       // base-128 encoded signed integer
};


//typedef struct {
//	uint8_t *data;
//	size_t maxsize;
//	size_t cursize;
//	size_t readcount;
//	bool compressed;
//} MessageBuffer;

struct MessageBufferField {
    //! The field offset in the POD structure.
    int32_t     offset;
    //! The bits. If 0, it'll treat it as sizeof(float)
    int32_t     bits;
    //! The count of how many there are, if more than 1, it handles it as an array.
    int32_t     count;
    //! The type of encoding to use "over the wire".
    WireType    encoding;
};

//============================================================================


// Remove later on, use this for now.
using MessageBuffer = SizeBuffer;
struct GameState { };


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



/**
*
*   Message Buffer functionality.
*
**/
/**
*   @brief Initializes a MessageBuffer with said data of given length.
**/
void MSG_Init( MessageBuffer *buf, uint8_t *data, size_t length );

/**
*   @brief Clears the Messagebuffer.
**/
void MSG_Clear( MessageBuffer *buf );

/**
*   @brief  Tells the MessageBuffer that the wished for space has been written to.
* 
*   @return Pointer to the acquired space which to write in. nullptr on failure.
**/
void *MSG_GetSpace( MessageBuffer *buf, size_t length );

/**
*   @brief  Writes given data to the MessageBuffer.
**/
void MSG_WriteData( MessageBuffer *msg, const void *data, size_t length );

/**
*   @brief Reads the data into the message buffer.
**/
void MSG_ReadData(MessageBuffer* sb, void* buffer, size_t length);

/**
*   @brief Copies an entire piece of data into the MessageBuffer.
**/
void MSG_CopyData( MessageBuffer *buf, const void *data, size_t length );

/**
*   @brief Skips reading data in the message buffer, like flushing does.
**/
int32_t MSG_SkipData( MessageBuffer *sb, size_t length );



/**
*
*   Wire Types write functionality.
*
**/
/**
*   @brief Writes a signed 8 bit byte.
**/
void MSG_WriteInt8( MessageBuffer *sb, int32_t c );

/**
*   @brief Writes an unsigned 8 bit byte.
**/
void MSG_WriteUint8( MessageBuffer *sb, int32_t c );

/**
*   @brief Writes a signed 16 bit short.
**/
void MSG_WriteInt16( MessageBuffer *sb, int32_t c );

/**
*   @brief Writes an unsigned 16 bit short.
**/
void MSG_WriteUint16( MessageBuffer *sb, uint32_t c );

/**
*   @brief Writes a 32 bit integer.
**/
void MSG_WriteInt32( MessageBuffer *sb, int32_t c );

/**
*   @brief Writes a 64 bit integer.
**/
void MSG_WriteInt64( MessageBuffer *sb, int64_t c );

/**
*   @brief Writes an unsigned LEB 128(base 128 encoded) integer.
**/
void MSG_WriteUintBase128( MessageBuffer *msg, uint64_t c );

/**
*   @brief Writes a zic-zac encoded signed integer.
**/
void MSG_WriteIntBase128( MessageBuffer *msg, int64_t c );

/**
*   @brief Writes a full precision float. (Transfered over the wire as an int32_t).
**/
void MSG_WriteFloat( MessageBuffer *sb, float f );

/**
*   @brief Writes a half float, lesser precision. (Transfered over the wire as an uint16_t)
**/
void MSG_WriteHalfFloat( MessageBuffer *sb, float f );

/**
*   @brief Writes a character string.
**/
void MSG_WriteString( MessageBuffer *sb, const char *s );

/**
*   @brief Writes a floating point angle converted to a 16 bit short. Lesser precision than full floats do.
**/
static inline void MSG_WriteAngle16(MessageBuffer* sb, float f) {
    //#define MSG_WriteAngle16( sb, f ) ( MSG_WriteInt16( ( sb ), FloatAngleToShort( ( f ) ) ) )
    MSG_WriteInt16(sb, FloatAngleToShort(f));
}



/**
*
*   Delta struct write functionality.
*
**/
/**
*   @brief Writes a delta user command.
**/
void MSG_WriteDeltaUsercmd( MessageBuffer *sb, const struct usercmd_s *from, struct usercmd_s *cmd );

/**
*   @brief Writes the entity number, remove state, and byte mask of the entity state.
**/
void MSG_WriteEntityNumber( MessageBuffer *msg, int32_t number, bool remove, uint32_t byteMask );

/**
*   @brief Writes the delta entity state.
**/
void MSG_WriteDeltaEntity( MessageBuffer *msg, const struct entity_state_s *from, const struct entity_state_s *to, bool force );

/**
*   @brief Writes the delta player state.
**/
void MSG_WriteDeltaPlayerState( MessageBuffer *msg, const PlayerState *ops, const PlayerState*ps );

/**
*   @brief Writes the delta of a game state.
**/
//void MSG_WriteDeltaGameState( MessageBuffer *msg, const GameState *from, const GameState *to );

/**
*   @brief Writes the delta of a given struct.
**/
void MSG_WriteDeltaStruct( MessageBuffer *msg, const void *from, const void *to, const MessageBufferField *fields, size_t numFields );



/**
*
*   Wire Types read functionality.
*
**/
/**
*   @brief Engages reading mode for the message buffer.
**/
void MSG_BeginReading( MessageBuffer *sb );
/**
*   @return Signed 8 bit byte.
**/
int32_t MSG_ReadInt8( MessageBuffer *msg );
/**
*   @return Unsigned 8 bit byte.
**/
int32_t MSG_ReadUint8( MessageBuffer *msg );
/**
*   @return Signed 16 bit short.
**/
int16_t MSG_ReadInt16( MessageBuffer *sb );
/**
*   @return Unsigned 16 bit short.
**/
uint16_t MSG_ReadUint16( MessageBuffer *sb );
/**
*   @return 32 bit integer.
**/
int32_t MSG_ReadInt32( MessageBuffer *sb );
/**
*   @return 64 bit integer.
**/
int64_t MSG_ReadInt64( MessageBuffer *sb );
/**
*   @return Base 128 decoded unsigned integer.
**/
uint64_t MSG_ReadUintBase128( MessageBuffer *msg );

/**
*   @return Zig-Zac decoded integer.
**/
int64_t MSG_ReadIntBase128( MessageBuffer *msg );

/**
*   @return The full precision float.
**/
float MSG_ReadFloat( MessageBuffer *sb );
/**
*   @return A half float, converted to float, keep in mind that half floats have less precision.
**/
float MSG_ReadHalfFloat( MessageBuffer *sb );
/**
*   @return The full string until its end.
**/
char *MSG_ReadString( MessageBuffer *sb );

/**
*   @return The part of the string data up till the first '\n'
**/
char *MSG_ReadStringLine( MessageBuffer *sb );

static inline float MSG_ReadAngle16(MessageBuffer* sb) { 
    //#define MSG_ReadAngle16( sb ) ( ShortToFloatAngle( MSG_ReadInt16( ( sb ) ) ) )
    return ShortToFloatAngle(MSG_ReadInt16(sb));
}

/**
*   @brief Reads the byte 'direction' into the vector.
**/
//void MSG_ReadDir(MessageBuffer* sb, vec3_t &dir);


/**
*
*   Delta struct read functionality.
*
**/
/**
*   @brief Reads in the entity number, the belonging byte mask, and whether to remove it or not.
**/
int32_t MSG_ReadEntityNumber(MessageBuffer* msg, bool* remove, uint32_t* byteMask);

/**
*   @brief Reads in the delta values of the entity states *from and *to into the message buffer.
**/
void MSG_ReadDeltaEntity(MessageBuffer* msg, const EntityState* from, EntityState* to, int32_t number, uint32_t byteMask);

/**
*   @brief Reads in the delta user command.
**/
void MSG_ReadDeltaUsercmd( MessageBuffer *sb, const struct usercmd_s *from, struct usercmd_s *cmd );

/**
*   @brief Reads in the delta values of the player states *from and *to into the message buffer.
**/
void MSG_ReadDeltaPlayerState(MessageBuffer* msg, const PlayerState* ops, PlayerState* ps);

/**
*   @brief Reads in the delta values of the game states *from and *to into the message buffer.
**/
void MSG_ReadDeltaGameState(MessageBuffer* msg, const GameState* from, GameState* to);

/**
*   @brief Reads in the delta values from the message buffer for the *from and *to structs.
**/
void MSG_ReadDeltaStruct( MessageBuffer *msg, const void *from, void *to, size_t size, const MessageBufferField *fields, size_t numFields );



/**
*   @brief Packs a bounding box by encoding it in a 32 bit unsigned int32_t.
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
static inline int32_t MSG_PackBoundingBox32(const vec3_t& mins, const vec3_t& maxs) {
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
static inline void MSG_UnpackBoundingBox32(int32_t solid, vec3_t& mins, vec3_t& maxs) {
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
//static inline int32_t MSG_PackBoundingBox16(const vec3_t &mins, const vec3_t &maxs)
//{
//    int32_t x, zd, zu;
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
//static inline void MSG_UnpackBoundingBox16(int32_t solid, vec3_t &mins, vec3_t &maxs)
//{
//    int32_t x, zd, zu;
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