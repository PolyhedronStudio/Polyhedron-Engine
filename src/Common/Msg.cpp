/***
*
*	License here.
*
*	@file
*
*	Client/Server messaging API. (Partially borrowed from QFusion.)
* 
*	Handles byte ordering and avoids alignment errors.
*
***/
#include "../Shared/Shared.h"
#include "Common/HalfFloat.h"
#include "Common/Msg.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"
#include "Game/Shared/Protocol.h"

// Assertion.
#include <cassert>


/**
*
*   Message Buffer functionality.
*
**/
static constexpr int32_t MAX_MSG_STRING_CHARS = MAX_NET_STRING;

//! Write Message SizeBuffer.
SizeBuffer   msg_write;
//! Actual Write Message buffer.
byte        msg_write_buffer[MAX_MSGLEN];


//! Read Message SizeBuffer.
SizeBuffer  msg_read;
//! Actual Read Message buffer.
byte        msg_read_buffer[MAX_MSGLEN];


//! null states to "delta from".
const EntityState       nullEntityState = {};
const PlayerState       nullPlayerState = {};
const ClientMoveCommand nullUserCmd = {};



/**
*
*   Wire Types write functionality.
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
void MSG_Init(void) {
    SZ_TagInit(&msg_read, msg_read_buffer, MAX_MSGLEN, SZ_MSG_READ);
    SZ_TagInit(&msg_write, msg_write_buffer, MAX_MSGLEN, SZ_MSG_WRITE);
}


/**
*   @brief Initializes write buffer for a new write session.
**/
void MSG_BeginWriting(void) {
    msg_write.currentSize = 0;
    msg_write.bitPosition = 0;
    msg_write.overflowed = false;
}

/**
*   @brief Writes a signed 8 bit byte.
**/
void MSG_WriteInt8( int32_t c ) {
    uint8_t* buf = ( uint8_t* )SZ_GetSpace( &msg_write, 1 );
	buf[0] = ( char )c;
}

/**
*   @brief Writes an unsigned 8 bit byte.
**/
void MSG_WriteUint8( int32_t c ) {
	uint8_t *buf = ( uint8_t* )SZ_GetSpace( &msg_write, 1 );
	buf[0] = ( uint8_t )( c & 0xff );
}

/**
*   @brief Writes a signed 16 bit short.
**/
void MSG_WriteInt16( int32_t c ) {
	uint8_t *buf = ( uint8_t* )SZ_GetSpace( &msg_write, 2 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
}

/**
*   @brief Writes an unsigned 16 bit short.
**/
void MSG_WriteUint16( uint32_t c ) {
	uint8_t *buf = ( uint8_t* )SZ_GetSpace( &msg_write, 2 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
}

/**
*   @brief Writes a 32 bit integer.
**/
void MSG_WriteInt32( int32_t c ) {
	uint8_t *buf = ( uint8_t* )SZ_GetSpace( &msg_write, 4 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
	buf[2] = ( uint8_t )( ( c >> 16 ) & 0xff );
	buf[3] = ( uint8_t )( c >> 24 );
}

/**
*   @brief Writes a 64 bit integer.
**/
void MSG_WriteInt64( int64_t c ) {
	uint8_t *buf = ( uint8_t* )SZ_GetSpace( &msg_write, 8 );
	buf[0] = ( uint8_t )( c & 0xffL );
	buf[1] = ( uint8_t )( ( c >> 8L ) & 0xffL );
	buf[2] = ( uint8_t )( ( c >> 16L ) & 0xffL );
	buf[3] = ( uint8_t )( ( c >> 24L ) & 0xffL );
	buf[4] = ( uint8_t )( ( c >> 32L ) & 0xffL );
	buf[5] = ( uint8_t )( ( c >> 40L ) & 0xffL );
	buf[6] = ( uint8_t )( ( c >> 48L ) & 0xffL );
	buf[7] = ( uint8_t )( c >> 56L );
}

/**
*   @brief Writes an unsigned LEB 128(base 128 encoded) integer.
**/
void MSG_WriteUintBase128( uint64_t c ) {
	uint8_t buf[10];
	size_t len = 0;

	do {
		buf[len] = c & 0x7fU;
		if ( c >>= 7 ) {
			buf[len] |= 0x80U;
		}
		len++;
	} while( c );

	MSG_WriteData( buf, len );
}

/**
*   @brief Writes a zic-zac encoded signed integer.
**/
void MSG_WriteIntBase128( int64_t c ) {
	// use Zig-zag encoding for signed integers for more efficient storage
	uint64_t cc = (uint64_t)(c << 1) ^ (uint64_t)(c >> 63);
	MSG_WriteUintBase128( cc );
}

/**
*   @brief Writes a full precision float. (Transfered over the wire as an int32_t).
**/
void MSG_WriteFloat( float f ) {
	union {
		float f;
		int32_t l;
	} dat;

	dat.f = f;
	MSG_WriteInt32( dat.l );
}

/**
*   @brief Writes a half float, lesser precision. (Transfered over the wire as an uint16_t)
**/
void MSG_WriteHalfFloat( float f ) {
	MSG_WriteUint16( float_to_half( f ) );
}

/**
*   @brief Writes a character string.
**/
void MSG_WriteString( const char *s ) {
	if( !s ) {
		MSG_WriteUint8(0); //MSG_WriteData( "", 1 );
	} else {
		int32_t l = strlen( s );
		if( l >= MAX_MSG_STRING_CHARS ) {
			Com_Printf( "MSG_WriteString: MAX_MSG_STRING_CHARS overflow" );
			MSG_WriteUint8(0);//MSG_WriteData( "", 1 );
			return;
		}
		MSG_WriteData( s, l + 1 );
	}
}

/**
*   @brief Writes a full precision vector 3, half float precision if halfFloat == true.
**/
void MSG_WriteVector3(const vec3_t& pos, bool halfFloat)
{
    if (!halfFloat) {
	    MSG_WriteFloat(pos.x);
	    MSG_WriteFloat(pos.y);
    	MSG_WriteFloat(pos.z);
    } else {
	    MSG_WriteHalfFloat(pos.x);
	    MSG_WriteHalfFloat(pos.y);
	    MSG_WriteHalfFloat(pos.z);
    }
}

/**
*   @brief Writes a full precision vector 4, half float precision if halfFloat == true.
**/
void MSG_WriteVector4(const vec4_t& pos, bool halfFloat)
{
    if (!halfFloat) {
	    MSG_WriteFloat(pos.x);
	    MSG_WriteFloat(pos.y);
    	MSG_WriteFloat(pos.z);
        MSG_WriteFloat(pos.w);
    } else {
	    MSG_WriteHalfFloat(pos.x);
	    MSG_WriteHalfFloat(pos.y);
	    MSG_WriteHalfFloat(pos.z);
        MSG_WriteHalfFloat(pos.w);
    }
}



/**
*
*   Wire Types Read Functionality.
*
**/
/**
*   @brief Resets read count and bitposition so we got a fresh start to read data with.
**/
void MSG_BeginReading(void)
{
    msg_read.readCount = 0;
    msg_read.bitPosition = 0;
}

/**
*   @brief  After acquiring the pointer, increases readCount by length.
* 
*   @return A pointer to the current read position. If underflows are allowed, a nullptr.
**/
byte* MSG_ReadData(size_t len)
{
    byte* buf = msg_read.data + msg_read.readCount;

    msg_read.readCount += len;
    msg_read.bitPosition = msg_read.readCount << 3;

    if (msg_read.readCount > msg_read.currentSize) {
        if (!msg_read.allowUnderflow) {
            Com_Error(ErrorType::Drop, "%s: read past end of message", __func__);
        }
        return NULL;
    }

    return buf;
}

/**
*   @return Signed 8 bit byte.
**/
int32_t MSG_ReadInt8() {
    uint8_t *buf = MSG_ReadData(1);

    if (!buf) {
        return -1;
    }

    return static_cast<signed char>(buf[0]);
}

/**
*   @return Unsigned 8 bit byte.
**/
int32_t MSG_ReadUint8() {
    uint8_t *buf = MSG_ReadData(1);

    if (!buf) {
        return -1; // TODO: Used to be 0 for QFusion but that won't budge for us now does it?   
    }

    return static_cast<unsigned char>(buf[0]);
}

/**
*   @return Signed 16 bit short.
**/
int16_t MSG_ReadInt16() {
    uint8_t *buf = MSG_ReadData(2);

    if (!buf) {
        return -1;
    }

    return (int16_t)(buf[0] | (buf[1] << 8));
//    return (int16_t)(buf[1] | (buf[0] << 8));
}

/**
*   @return Unsigned 16 bit short.
**/
uint16_t MSG_ReadUint16() {
    uint8_t *buf = MSG_ReadData(2);

    if (!buf) {
        return -1; // TODO: This used to be 0, ...
    }

    return (uint16_t)(buf[0] | (buf[1] << 8));
    //return (uint16_t)(buf[1] | (buf[0] << 8));
}

/**
*   @return 32 bit integer.
**/
int32_t MSG_ReadInt32() {
    uint8_t *buf = MSG_ReadData(4);

    if (!buf) {
        return -1;
    }

    return buf[0] 
		| (buf[1] << 8) 
		| (buf[2] << 16) 
		| (buf[3] << 24);
    //return buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
}

/**
*   @return 64 bit integer.
**/
int64_t MSG_ReadInt64() {
    uint8_t *buf = MSG_ReadData(8);

    if (!buf) {
        return -1;
    }

	//return ( int64_t )buf[7]
	//	| ( ( int64_t )buf[6] << 8L )
	//	| ( ( int64_t )buf[5] << 16L )
	//	| ( ( int64_t )buf[4] << 24L )
	//	| ( ( int64_t )buf[3] << 32L )
	//	| ( ( int64_t )buf[2] << 40L )
	//	| ( ( int64_t )buf[1] << 48L )
	//	| ( ( int64_t )buf[0] << 56L );
	return ( int64_t )buf[0]
		| ( ( int64_t )buf[1] << 8L )
		| ( ( int64_t )buf[2] << 16L )
		| ( ( int64_t )buf[3] << 24L )
		| ( ( int64_t )buf[4] << 32L )
		| ( ( int64_t )buf[5] << 40L )
		| ( ( int64_t )buf[6] << 48L )
		| ( ( int64_t )buf[7] << 56L );
}

/**
*   @return Base 128 decoded unsigned integer.
**/
uint64_t MSG_ReadUintBase128() {
    size_t   len = 0;
    uint64_t i = 0;

    while (len < 10) {
    	uint8_t c = MSG_ReadUint8();
    	i |= (c & 0x7fLL) << (7 * len);
    	len++;
	
        if (!(c & 0x80)) {
			break;
		}
    }

    return i;
}

/**
*   @return Zig-Zac decoded signed integer.
**/
int64_t MSG_ReadIntBase128() {
    // un-Zig-Zag our value back to a signed integer
    uint64_t c = MSG_ReadUintBase128();
    return (int64_t)(c >> 1) ^ (-(int64_t)(c & 1));
}

/**
*   @return The full precision float.
**/
float MSG_ReadFloat() {
    union {
		float f;
		int32_t   l;
    } dat;

    dat.l = MSG_ReadInt32();
    if (msg_read.readCount > msg_read.currentSize) {
		dat.f = -1;
    }
    return dat.f;
}

/**
*   @return A half float, converted to float, keep in mind that half floats have less precision.
**/
float MSG_ReadHalfFloat() { 
	return half_to_float(MSG_ReadUint16()); 
}

/**
*   @brief Reads the vector as a 'direction' into the message buffer.
**/
//void MSG_ReadDir(MessageBuffer* msg, vec3_t &dir) { 
//	ByteToDir(MSG_ReadUint8(msg), dir); 
//}

/**
*   @brief Helper function for MSG_ReadString and MSG_ReadStringLine.
**/
static inline char* MSG_ReadString2(bool stopAtLinebreak) {
    int32_t		l, c;
    static char string[MAX_MSG_STRING_CHARS];

    l = 0;
    do {
		c = MSG_ReadUint8();
		if (c == -1 || c == 0 || (stopAtLinebreak && c == '\n')) {
			break;
		}

		string[l] = c;
		l++;
    } while ( (uint32_t)l < sizeof(string) - 1 );

    string[l] = 0;

    return string;
}

/**
*   @return The full string until its end.
**/
char* MSG_ReadString() { 
	return MSG_ReadString2(false); 
}

/**
*   @return The part of the string data up till the first '\n'
**/
char* MSG_ReadStringLine() { 
	return MSG_ReadString2(true); 
}

/**
*   @brief Reads the string from message buffer, into the string buffer, up till length.
* 
*   @return Count of characters read.
**/
size_t MSG_ReadStringBuffer(char* dest, size_t size) {
    int32_t c = 0;
    size_t len = 0;

    while (1) {
        c = MSG_ReadUint8();
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
size_t MSG_ReadStringLineBuffer(char *dest, size_t size)
{
    int     c;
    size_t  len = 0;

    while (1) {
        c = MSG_ReadUint8();
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

/**
*   @return Depending on halfFloat = true/false, the full floating point precision of the vector, half float otherwise.
**/
vec3_t MSG_ReadVector3(bool halfFloat) {
    if (!halfFloat) {
        return vec3_t{
            MSG_ReadFloat(),
            MSG_ReadFloat(),
            MSG_ReadFloat()
        };
    } else {
        return vec3_t{
            MSG_ReadHalfFloat(),
            MSG_ReadHalfFloat(),
            MSG_ReadHalfFloat()
        };
    }
}

/**
*   @return Depending on halfFloat = true/false, the full floating point precision of the vector, half float otherwise.
**/
vec4_t MSG_ReadVector4(bool halfFloat) {
    if (!halfFloat) {
        return vec4_t{
            MSG_ReadFloat(),
            MSG_ReadFloat(),
            MSG_ReadFloat(),
            MSG_ReadFloat()
        };
    } else {
        return vec4_t{
            MSG_ReadHalfFloat(),
            MSG_ReadHalfFloat(),
            MSG_ReadHalfFloat(),
            MSG_ReadHalfFloat()
        };
    }
}



/**
*
*   Delta Player State Read/Write.
*
**/
/**
*   @brief  Parses the delta packets of player states.
**/
void MSG_ParseDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t extraFlags) {

    // Sanity check. 
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: no state to delta to, 'to' == nullptr", __func__);
    }

    // Clear to old value before doing any delta parsing.
    if (!from) {
	    *to = {};
    } else if (to != from) {
        *to = *from;
    }

    // Read the flag bits.
    uint32_t flags = static_cast<uint32_t>(MSG_ReadInt32());

    //
    // Parse movement related state data.
    // 
    // PM Type
    if (flags & PS_PM_TYPE) { 
        to->pmove.type = MSG_ReadUint8();
    }

    // Origin X Y.
    if (flags & PS_PM_ORIGIN) {
        to->pmove.origin[0] = MSG_ReadFloat();
        to->pmove.origin[1] = MSG_ReadFloat();
    }

    // Origin Z.
    if (extraFlags & EPS_M_ORIGIN2) {
        to->pmove.origin[2] = MSG_ReadFloat();
    }

    // Velocity X Y.
    if (flags & PS_PM_VELOCITY) {
        to->pmove.velocity[0] = MSG_ReadHalfFloat();
        to->pmove.velocity[1] = MSG_ReadHalfFloat();
    }

    // Velocity Z.
    if (extraFlags & EPS_M_VELOCITY2) {
        to->pmove.velocity[2] = MSG_ReadHalfFloat();
    }

    // PM Time.
    if (flags & PS_PM_TIME) { 
        to->pmove.time = MSG_ReadUint16();
    }

    // PM Flags.
    if (flags & PS_PM_FLAGS) { 
        to->pmove.flags = MSG_ReadUint16();
    }

    // PM Gravity.
    if (flags & PS_PM_GRAVITY) {
        to->pmove.gravity = MSG_ReadUint16();
    }

    // PM Delta Angles.
    if (flags & PS_PM_DELTA_ANGLES) {
	    to->pmove.deltaAngles = MSG_ReadVector3(true);
    }

    // View Offset.
    if (flags & PS_PM_VIEW_OFFSET) {
	    to->pmove.viewOffset = MSG_ReadVector3(true);
    }

    // Step offset.
    if (flags & PS_PM_STEP_OFFSET) {
	    to->pmove.stepOffset = MSG_ReadHalfFloat();
    }

    // View Angles X Y Z.
    if (flags & PS_PM_VIEW_ANGLES) {
	    to->pmove.viewAngles = MSG_ReadVector3(true);
    }

    //
    // Parse the rest of the player state data.
    //
    // Kick Angles.
    if (flags & PS_KICKANGLES) {
        to->kickAngles = MSG_ReadVector3(true);
    }

    // Weapon Index.
    if (flags & PS_WEAPONINDEX) {
        to->gunIndex = MSG_ReadUint8();
    }

    if (flags & PS_GUNANIMATION_TIME_START) {
	    to->gunAnimationStartTime = MSG_ReadInt32();
    }
    if (flags & PS_GUNANIMATION_FRAME_START) {
	    to->gunAnimationStartFrame = MSG_ReadUint16();
    }
    if (flags & PS_GUNANIMATION_FRAME_END) {
	    to->gunAnimationEndFrame = MSG_ReadUint16();
    }
    if (flags & PS_GUNANIMATION_FRAME_TIME) {
	    to->gunAnimationFrametime = MSG_ReadHalfFloat();
    }
    if (flags & PS_GUNANIMATION_LOOP_COUNT) {
	    to->gunAnimationLoopCount = MSG_ReadUint8(); 
    }
    if (flags & PS_GUNANIMATION_LOOP_FORCE) {
    	to->gunAnimationForceLoop = MSG_ReadUint8();
    }

    // Gun Offset.
    if (extraFlags & EPS_GUNOFFSET) {
        to->gunOffset = MSG_ReadVector3(true);
    }

    // Gun Angles.
    if (extraFlags & EPS_GUNANGLES) {
        to->gunAngles = MSG_ReadVector3(true);
    }

    // Blend.
    if (flags & PS_BLEND) {
        to->blend[0] = MSG_ReadHalfFloat();
        to->blend[1] = MSG_ReadHalfFloat();
        to->blend[2] = MSG_ReadHalfFloat();
        to->blend[3] = MSG_ReadHalfFloat();
    }

    // FOV.
    if (flags & PS_FOV) {
        to->fov = MSG_ReadHalfFloat();
    }

    // RDFlags.
    if (flags & PS_RDFLAGS) {
        to->rdflags = MSG_ReadInt32();
    }

    // Parse Stats.
    if (extraFlags & EPS_STATS) {
        int32_t statbits = MSG_ReadInt32();
        for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
            if (statbits & (1 << i)) {
                to->stats[i] = MSG_ReadInt16();
            }
        }
    }
}

/**
*   @brief Writes the delta player state.
**/
int MSG_WriteDeltaPlayerstate(const PlayerState* from, PlayerState* to, uint32_t playerStateMessageFlags)
{
    // Sanity check.
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: NULL", __func__);
    }

    // Derive 'from' from nullPlayerState if we ain't got one.
    if (!from) {
        from = &nullPlayerState;
    }

    //
    // Determine what needs to be sent
    //
    uint32_t playerStateFlags = 0;
    uint32_t entityStateFlags = 0;

    if (to->pmove.type != from->pmove.type) {
        playerStateFlags |= PS_PM_TYPE;
    }

    if (!EqualEpsilonf(to->pmove.origin[0], from->pmove.origin[0]) ||
        !EqualEpsilonf(to->pmove.origin[1], from->pmove.origin[1])) {
        playerStateFlags |= PS_PM_ORIGIN;
    }

    if (!EqualEpsilonf(to->pmove.origin[2], from->pmove.origin[2])) {
        entityStateFlags |= EPS_M_ORIGIN2;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_PREDICTION)) {
        if (!EqualEpsilonf(to->pmove.velocity[0], from->pmove.velocity[0]) ||
            !EqualEpsilonf(to->pmove.velocity[1], from->pmove.velocity[1])) {
            playerStateFlags |= PS_PM_VELOCITY;
        }

        if (!EqualEpsilonf(to->pmove.velocity[2], from->pmove.velocity[2])) { 
            entityStateFlags |= EPS_M_VELOCITY2;
        }

        if (to->pmove.time != from->pmove.time) {
            playerStateFlags |= PS_PM_TIME;
        }

        if (to->pmove.flags != from->pmove.flags) {
            playerStateFlags |= PS_PM_FLAGS;
        }

        if (to->pmove.gravity != from->pmove.gravity) {
            playerStateFlags |= PS_PM_GRAVITY;
        }
    } else {
        // Save previous state.
	    to->pmove.velocity = from->pmove.velocity;
        to->pmove.time = from->pmove.time;
        to->pmove.flags = from->pmove.flags;
        to->pmove.gravity = from->pmove.gravity;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_DELTAANGLES)) {
        if (!EqualEpsilonf(to->pmove.deltaAngles.x, from->pmove.deltaAngles.x) ||
            !EqualEpsilonf(to->pmove.deltaAngles.y, from->pmove.deltaAngles.y) ||
            !EqualEpsilonf(to->pmove.deltaAngles.z, from->pmove.deltaAngles.z))
            playerStateFlags |= PS_PM_DELTA_ANGLES;
    } else {
        // Save previous state.
	    to->pmove.deltaAngles = from->pmove.deltaAngles;
    }

    if (!EqualEpsilonf(to->pmove.viewOffset.x, from->pmove.viewOffset.x) ||
        !EqualEpsilonf(to->pmove.viewOffset.y, from->pmove.viewOffset.y) ||
        !EqualEpsilonf(to->pmove.viewOffset.z, from->pmove.viewOffset.z)) {
        playerStateFlags |= PS_PM_VIEW_OFFSET;
    }

    if (!(playerStateMessageFlags & MSG_PS_IGNORE_VIEWANGLES)) {
        if (!EqualEpsilonf(to->pmove.viewAngles.x, from->pmove.viewAngles.x) ||
            !EqualEpsilonf(to->pmove.viewAngles.y, from->pmove.viewAngles.y) ||
            !EqualEpsilonf(to->pmove.viewAngles.z, from->pmove.viewAngles.z)) {
            playerStateFlags |= PS_PM_VIEW_ANGLES;
        }
    } else {
        // save previous state
        to->pmove.viewAngles[0] = from->pmove.viewAngles[0];
        to->pmove.viewAngles[1] = from->pmove.viewAngles[1];
        to->pmove.viewAngles[2] = from->pmove.viewAngles[2];
    }

    if (from->kickAngles[0] != to->kickAngles[0] ||
        from->kickAngles[1] != to->kickAngles[1] ||
        from->kickAngles[2] != to->kickAngles[2]) {
        playerStateFlags |= PS_KICKANGLES;
    }

    if (from->blend[0] != to->blend[0] ||
        from->blend[1] != to->blend[1] ||
        from->blend[2] != to->blend[2] ||
        from->blend[3] != to->blend[3]) {
        playerStateFlags |= PS_BLEND;
    }

    if (from->pmove.stepOffset != to->pmove.stepOffset) {
	    playerStateFlags |= PS_PM_STEP_OFFSET;
    }

    if (from->fov != to->fov) {
	    playerStateFlags |= PS_FOV;
    }

    if (to->rdflags != from->rdflags) {
	    playerStateFlags |= PS_RDFLAGS;
    }

    if (to->gunIndex != from->gunIndex) {
	    playerStateFlags |= PS_WEAPONINDEX;
    }

    if (to->gunAnimationStartTime != from->gunAnimationStartTime) {
	    playerStateFlags |= PS_GUNANIMATION_TIME_START;
    }
    if (to->gunAnimationStartFrame != from->gunAnimationStartFrame) {
        playerStateFlags |= PS_GUNANIMATION_FRAME_START;
    }
    if (to->gunAnimationEndFrame != from->gunAnimationEndFrame) {
	    playerStateFlags |= PS_GUNANIMATION_FRAME_END;
    }
    if (!EqualEpsilonf(to->gunAnimationFrametime, from->gunAnimationFrametime)) {
        playerStateFlags |= PS_GUNANIMATION_FRAME_TIME;
    }
    if (to->gunAnimationLoopCount != from->gunAnimationLoopCount) {
	    playerStateFlags |= PS_GUNANIMATION_LOOP_COUNT;
    }
    if (to->gunAnimationForceLoop != from->gunAnimationForceLoop) {
	    playerStateFlags |= PS_GUNANIMATION_LOOP_FORCE;
    }


    if (from->gunOffset[0] != to->gunOffset[0] ||
        from->gunOffset[1] != to->gunOffset[1] ||
        from->gunOffset[2] != to->gunOffset[2]) {
        entityStateFlags |= EPS_GUNOFFSET;
    }

    if (from->gunAngles[0] != to->gunAngles[0] ||
        from->gunAngles[1] != to->gunAngles[1] ||
        from->gunAngles[2] != to->gunAngles[2]) {
        entityStateFlags |= EPS_GUNANGLES;
    }

    int32_t statbits = 0;
    for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
	    if (to->stats[i] != from->stats[i]) {
	        statbits |= 1 << i;
        }
    }

    if (statbits) {
	    entityStateFlags |= EPS_STATS;
    }

    //
    // write it
    //
    MSG_WriteInt32(playerStateFlags);

    //
    // write the PlayerMoveState
    //
    if (playerStateFlags & PS_PM_TYPE) {
        MSG_WriteUint8(to->pmove.type);
    }

    if (playerStateFlags & PS_PM_ORIGIN) {
        MSG_WriteFloat(to->pmove.origin[0]);
        MSG_WriteFloat(to->pmove.origin[1]);
    }

    if (entityStateFlags & EPS_M_ORIGIN2) {
	    MSG_WriteFloat(to->pmove.origin[2]);
    }

    if (playerStateFlags & PS_PM_VELOCITY) {
        MSG_WriteHalfFloat(to->pmove.velocity[0]);
        MSG_WriteHalfFloat(to->pmove.velocity[1]);
    }

    if (entityStateFlags & EPS_M_VELOCITY2) {
	    MSG_WriteHalfFloat(to->pmove.velocity[2]);
    }

    if (playerStateFlags & PS_PM_TIME) {
	    MSG_WriteUint16(to->pmove.time);
    }

    if (playerStateFlags & PS_PM_FLAGS) {
	    MSG_WriteUint16(to->pmove.flags);
    }

    if (playerStateFlags & PS_PM_GRAVITY) {
	    MSG_WriteUint16(to->pmove.gravity);
    }

    if (playerStateFlags & PS_PM_DELTA_ANGLES) {
        MSG_WriteVector3(to->pmove.deltaAngles, true);
    }

    //
    // write the rest of the PlayerState
    //
    if (playerStateFlags & PS_PM_VIEW_OFFSET) {
        MSG_WriteVector3(to->pmove.viewOffset, true);
    }

    if (playerStateFlags & PS_PM_STEP_OFFSET) {
        MSG_WriteHalfFloat(to->pmove.stepOffset);
    }

    if (playerStateFlags & PS_PM_VIEW_ANGLES) {
        MSG_WriteVector3(to->pmove.viewAngles, true);
    }

    if (playerStateFlags & PS_KICKANGLES) {
        MSG_WriteVector3(to->kickAngles, true);
    }

    if (playerStateFlags & PS_WEAPONINDEX) {
	    MSG_WriteUint8(to->gunIndex);
    }

    if (playerStateFlags & PS_GUNANIMATION_TIME_START) {
	    MSG_WriteInt32(to->gunAnimationStartTime);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_START) {
	    MSG_WriteUint16(to->gunAnimationStartFrame);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_END) {
	    MSG_WriteUint16(to->gunAnimationEndFrame);
    }
    if (playerStateFlags & PS_GUNANIMATION_FRAME_TIME) {
	    MSG_WriteHalfFloat(to->gunAnimationFrametime);
    }
    if (playerStateFlags & PS_GUNANIMATION_LOOP_COUNT) {
	    MSG_WriteUint8(to->gunAnimationLoopCount);
    }
    if (playerStateFlags & PS_GUNANIMATION_LOOP_FORCE) {
	    MSG_WriteUint8(to->gunAnimationForceLoop);
    }

    if (entityStateFlags & EPS_GUNOFFSET) {
        MSG_WriteVector3(to->gunOffset, true);
    }

    if (entityStateFlags & EPS_GUNANGLES) {
        MSG_WriteVector3(to->gunAngles, true);
    }

    if (playerStateFlags & PS_BLEND) {
        MSG_WriteVector4(to->blend, true);
    }
  
    if (playerStateFlags & PS_FOV) {
	    MSG_WriteHalfFloat(to->fov);
    }

    if (playerStateFlags & PS_RDFLAGS) {
        MSG_WriteInt32(to->rdflags);
    }

    // Send stats
    if (entityStateFlags & EPS_STATS) {
        MSG_WriteInt32(statbits);
	    for (int32_t i = 0; i < MAX_PLAYERSTATS; i++) {
	        if (statbits & (1 << i)) {
    		    MSG_WriteInt16(to->stats[i]);
	        }
	    }
    }

    return entityStateFlags;
}



/**
*
*   Delta Entity State Read/Write.
*
**/
/**
*   @brief  Writes entity number, remove bit, and the byte mask to buffer.
**/
void MSG_WriteEntityNumber(int32_t number, bool remove, uint32_t byteMask) {
    MSG_WriteIntBase128(number * (remove ? -1 : 1));
    MSG_WriteUintBase128(byteMask);
}

/**
* @brief Reads out entity number of current message in the buffer.
*
* @param remove     Set to true in case a remove bit was send along the wire..
* @param byteMask   Mask containing all the bits of message data to read out.
*
* @return   The entity number.
**/
int32_t MSG_ReadEntityNumber(bool* remove, uint32_t* byteMask) {
    int32_t number;

    *remove = false;
    number = (int32_t)MSG_ReadIntBase128();
    *byteMask = MSG_ReadUintBase128();

    if (number < 0) {
		number *= -1;
		*remove = true;
    }

    return number;
}

/**
*   @brief Writes the delta values of the entity state.
**/
void MSG_WriteDeltaEntity(const EntityState* from, const EntityState* to, uint32_t entityStateMessageFlags) {
    // Check whether to remove this entity if it isn't going anywhere.
    if (!to) {
        // If it never came from anywhere, we got no data to work with, error out.
        if (!from) {
            Com_Error(ErrorType::Drop, "%s: NULL", __func__);
        }

        // Invalid number? Error out.
        if (from->number < 1 || from->number >= MAX_EDICTS) {
            Com_Error(ErrorType::Drop, "%s: bad number: %d", __func__, from->number);
        }

        // Write out entity number with a remove bit set.
        MSG_WriteEntityNumber(from->number, true, 0);

        // We're done, remove entity.
        return;
    }

    // Entity number sanity check.
    if (to->number < 1 || to->number >= MAX_EDICTS) {
        Com_Error(ErrorType::Drop, "%s: bad number: %d", __func__, to->number);
    }

    // Update from null entity state in case there is nothing to delta from.
    if (!from) {
        from = &nullEntityState;
    }

    // Set byteMask to 0.
    uint32_t byteMask = 0;

    if (!(entityStateMessageFlags & MSG_ES_FIRSTPERSON)) {
	    if (!EqualEpsilonf(to->origin[0], from->origin[0])) {
	        byteMask |= EntityMessageBits::OriginX;
	    }
	    if (!EqualEpsilonf(to->origin[1], from->origin[1])) {
		    byteMask |= EntityMessageBits::OriginY;
        }
	    if (!EqualEpsilonf(to->origin[2], from->origin[2])) { 
            byteMask |= EntityMessageBits::OriginZ;
        }

        // N&C: Full float precision.
        if (!EqualEpsilonf(to->angles[0], from->angles[0])) {
            byteMask |= EntityMessageBits::AngleX;
        }
	    if (!EqualEpsilonf(to->angles[1], from->angles[1])) {
	        byteMask |= EntityMessageBits::AngleY;
	    }
	    if (!EqualEpsilonf(to->angles[2], from->angles[2])) {
		    byteMask |= EntityMessageBits::AngleZ;
	    }

        if (entityStateMessageFlags & MSG_ES_NEWENTITY) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->origin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->origin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->origin[2]))
            {
                byteMask |= EntityMessageBits::OldOrigin;
            }
        }
    }

    if (to->skinNumber != from->skinNumber) {
        byteMask |= EntityMessageBits::Skin;
    }

    if (!EqualEpsilonf(to->animationFrame, from->animationFrame)) {
        byteMask |= EntityMessageBits::AnimationFrame;
    }

    if (to->effects != from->effects) {
	    byteMask |= EntityMessageBits::EntityEffects;
    }

    if (to->renderEffects != from->renderEffects) {
	    byteMask |= EntityMessageBits::RenderEffects;
    }

    if (to->hashedClassname != from->hashedClassname) {
        byteMask |= EntityMessageBits::HashedClassname;
    }

    if (to->solid != from->solid) {
	    byteMask |= EntityMessageBits::Solid;
    }

    // Event is not delta compressed, it's bit is set when eventID is non 0.
    if (to->eventID) {
	    byteMask |= EntityMessageBits::EventID;
    }

    if (to->modelIndex != from->modelIndex) {
	    byteMask |= EntityMessageBits::ModelIndex;
    }
    if (to->modelIndex2 != from->modelIndex2) {
	    byteMask |= EntityMessageBits::ModelIndex2;
    }
    if (to->modelIndex3 != from->modelIndex3) {
	    byteMask |= EntityMessageBits::ModelIndex3;
    }
    if (to->modelIndex4 != from->modelIndex4) {
	    byteMask |= EntityMessageBits::ModelIndex4;
    }

    if (to->sound != from->sound) {
	    byteMask |= EntityMessageBits::Sound;
    }

    if (to->renderEffects & RenderEffects::FrameLerp) {
        byteMask |= EntityMessageBits::OldOrigin;
    }
    else if (to->renderEffects & RenderEffects::Beam) {
        if (entityStateMessageFlags & MSG_ES_BEAMORIGIN) {
            if (!EqualEpsilonf(to->oldOrigin[0], from->oldOrigin[0]) ||
                !EqualEpsilonf(to->oldOrigin[1], from->oldOrigin[1]) ||
                !EqualEpsilonf(to->oldOrigin[2], from->oldOrigin[2])) 
            {
                byteMask |= EntityMessageBits::OldOrigin;
            }
        } else {
            byteMask |= EntityMessageBits::OldOrigin;
        }
    }

    if (to->animationStartTime != from->animationStartTime) { 
        byteMask |= EntityMessageBits::AnimationTimeStart;
    }
    if (to->animationStartFrame != from->animationStartFrame) {
	    byteMask |= EntityMessageBits::AnimationFrameStart;
    }
    if (to->animationEndFrame != from->animationEndFrame) {
	    byteMask |= EntityMessageBits::AnimationFrameEnd;
    }
    if (to->animationFramerate != from->animationFramerate) {
	    byteMask |= EntityMessageBits::AnimationFrameTime;
    }
    
    //
    // write the message
    //
    if (!byteMask && !(entityStateMessageFlags & MSG_ES_FORCE))
        return;     // nothing to send!

    //----------

    // Set the "more byteMask" flags based on how deep this state update is.
    //if (byteMask & 0xff000000)
    //    byteMask |= EntityMessageBits::MoreBitsC | EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    //else if (byteMask & 0x00ff0000)
    //    byteMask |= EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
    //else if (byteMask & 0x0000ff00)
    //    byteMask |= EntityMessageBits::MoreBitsA;

    // Write out the first 8 byteMask.
    //MSG_WriteUint8(byteMask & 255);

    //// Write out the next 24 byteMask if this is an update reaching to MoreBitsC
    //if (byteMask & 0xff000000) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //    MSG_WriteUint8((byteMask >> 16) & 255);
    //    MSG_WriteUint8((byteMask >> 24) & 255);
    //}
    //// Write out the next 16 byteMask if this is an update reaching to MoreBitsB
    //else if (byteMask & 0x00ff0000) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //    MSG_WriteUint8((byteMask >> 16) & 255);
    //}
    //// Write out the next 8 byteMask if this is an update reaching to MoreBitsA
    //else if (byteMask & 0x0000ff00) {
    //    MSG_WriteUint8((byteMask >> 8) & 255);
    //}
    //MSG_WriteInt32(byteMask);


    ////----------
    //// Make sure the "REMOVE" bit is unset before writing out the Entity State Number.
    //int32_t entityNumber = to->number & ~(1U << 15);

    //// Write out the Entity State number.
    //MSG_WriteInt16(to->number);
    MSG_WriteEntityNumber(to->number, false, byteMask);

    // Write out the ModelIndex.
    if (byteMask & EntityMessageBits::ModelIndex) { 
        MSG_WriteUint8(to->modelIndex);
    }
    // Write out the ModelIndex2.
    if (byteMask & EntityMessageBits::ModelIndex2) {
	    MSG_WriteUint8(to->modelIndex2);
    }
    // Write out the ModelIndex3.
    if (byteMask & EntityMessageBits::ModelIndex3) {
	    MSG_WriteUint8(to->modelIndex3);
    }
    // Write out the ModelIndex4.
    if (byteMask & EntityMessageBits::ModelIndex4) {
	    MSG_WriteUint8(to->modelIndex4);
    }

    // Write out the AnimationFrame.
    if (byteMask & EntityMessageBits::AnimationFrame) {
	    MSG_WriteHalfFloat(to->animationFrame);
    }
    
    // Write out the Skin Number.
    if (byteMask & EntityMessageBits::Skin) {
	    MSG_WriteInt16(to->skinNumber);
    }

    // Write out the Entity Effects.
    if (byteMask & EntityMessageBits::EntityEffects) {
	    MSG_WriteInt32(to->effects);
    }

    // Write out the Render Effects.
    if (byteMask & EntityMessageBits::RenderEffects) {
	    MSG_WriteInt32(to->renderEffects);
    }

    // Write out the Render Effects.
    if (byteMask & EntityMessageBits::HashedClassname) {
	    MSG_WriteInt32(to->hashedClassname);
    }

    // Write out the Origin X.
    if (byteMask & EntityMessageBits::OriginX) {
	    MSG_WriteFloat(to->origin[0]);
    }
    // Write out the Origin Y.
    if (byteMask & EntityMessageBits::OriginY) {
	    MSG_WriteFloat(to->origin[1]);
    }
    // Write out the Origin Z.
    if (byteMask & EntityMessageBits::OriginZ) {
	    MSG_WriteFloat(to->origin[2]);
    }

    // Write out the Angle X.
    if (byteMask & EntityMessageBits::AngleX) {
	    MSG_WriteHalfFloat(to->angles[0]);
    }
    // Write out the Angle Y.
    if (byteMask & EntityMessageBits::AngleY) {
	    MSG_WriteHalfFloat(to->angles[1]);
    }
    // Write out the Angle Z.
    if (byteMask & EntityMessageBits::AngleZ) {
	    MSG_WriteHalfFloat(to->angles[2]);
    }

    // Write out the Old Origin.
    if (byteMask & EntityMessageBits::OldOrigin) {
        MSG_WriteVector3(to->oldOrigin);
    }

    // Write out the Sound.
    if (byteMask & EntityMessageBits::Sound) {
	    MSG_WriteUint8(to->sound);
    }

    // Write out the Event ID.
    if (byteMask & EntityMessageBits::EventID) {
	    MSG_WriteUint8(to->eventID);
    }

    // Write out the Solid.
    if (byteMask & EntityMessageBits::Solid) {
        MSG_WriteInt32(to->solid);
    }

    // Write out the Animation Start Time.
    if (byteMask & EntityMessageBits::AnimationTimeStart) {
	    MSG_WriteInt32(to->animationStartTime);
    }
    // Write out the Animation Start Frame.
    if (byteMask & EntityMessageBits::AnimationFrameStart) {
	    MSG_WriteUint16(to->animationStartFrame);
    }
    // Write out the Animation End Frame.
    if (byteMask & EntityMessageBits::AnimationFrameEnd) {
	    MSG_WriteUint16(to->animationEndFrame);
    }
    // Write out the Animation Frame Time.
    if (byteMask & EntityMessageBits::AnimationFrameTime) {
    	MSG_WriteHalfFloat(to->animationFramerate);
    }
}

/**
*   @brief Reads the delta entity state, can go from either a baseline or a previous packet Entity State.
**/
void MSG_ParseDeltaEntity(const EntityState* from, EntityState* to, int32_t number, uint32_t byteMask, uint32_t entityStateFlags) {
    // Sanity checks.
    if (!to) {
        Com_Error(ErrorType::Drop, "%s: NULL", __func__);
    }

    // Ensure the number is valid.
    if (number < 1 || number >= MAX_EDICTS) {
        Com_Error(ErrorType::Drop, "%s: bad entity number: %d", __func__, number);
    }

    // Ensure our 'to' state is cleared out in case of not having a 'from'.
    // Otherwise, assign 'from' to 'to' so we already got the unchanged values.
    if (!from) {
	    *to = {};
    } else if (to != from) {
        *to = *from;
    }

    // Store number.
    to->number = number;

    // Reset eventID.
    to->eventID = 0;

    // If no byteMask were received, we got no business here, return.
    if (!byteMask) {
        return;
    }

    // Model Indexes.
    if (byteMask & EntityMessageBits::ModelIndex) {
        to->modelIndex = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex2) {
        to->modelIndex2 = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex3) {
        to->modelIndex3 = MSG_ReadUint8();
    }
    if (byteMask & EntityMessageBits::ModelIndex4) {
        to->modelIndex4 = MSG_ReadUint8();
    }

    // Frame.
    if (byteMask & EntityMessageBits::AnimationFrame)
        to->animationFrame = MSG_ReadHalfFloat();

    // Skinnum.
    if (byteMask & EntityMessageBits::Skin) { 
        to->skinNumber = MSG_ReadUint16();
    }

    // Effects.
    if (byteMask & EntityMessageBits::EntityEffects) {
	    to->effects = MSG_ReadInt32();
    }

    // RenderFX.
    if (byteMask & EntityMessageBits::RenderEffects) {
	    to->renderEffects = MSG_ReadInt32();
    }

    // HashedClassname.
    if (byteMask & EntityMessageBits::HashedClassname) {
	    to->hashedClassname = MSG_ReadInt32();
    }

    // Origin.
    if (byteMask & EntityMessageBits::OriginX) {
        to->origin[0] = MSG_ReadFloat();
    }
    if (byteMask & EntityMessageBits::OriginY) {
        to->origin[1] = MSG_ReadFloat();
    }
    if (byteMask & EntityMessageBits::OriginZ) {
        to->origin[2] = MSG_ReadFloat();
    }

    // Angle.
    if (byteMask & EntityMessageBits::AngleX) {
	    to->angles[0] = MSG_ReadHalfFloat();
    }
    if (byteMask & EntityMessageBits::AngleY) {
    	to->angles[1] = MSG_ReadHalfFloat();
    }
    if (byteMask & EntityMessageBits::AngleZ) {
	    to->angles[2] = MSG_ReadHalfFloat();
    }

    // Old Origin.
    if (byteMask & EntityMessageBits::OldOrigin) {
        to->oldOrigin = MSG_ReadVector3();
    }

    // Sound.
    if (byteMask & EntityMessageBits::Sound) {
        to->sound = MSG_ReadUint8();
    }

    // Event.
    if (byteMask & EntityMessageBits::EventID) {
        to->eventID = MSG_ReadUint8();
    }

    // Solid.
    if (byteMask & EntityMessageBits::Solid) {
        to->solid = MSG_ReadInt32();
    }

    if (byteMask & EntityMessageBits::AnimationTimeStart) {
	    to->animationStartTime = MSG_ReadInt32();
    }
    if (byteMask & EntityMessageBits::AnimationFrameStart) {
	    to->animationStartFrame = MSG_ReadUint16();
    }
    if (byteMask & EntityMessageBits::AnimationFrameEnd) {
	    to->animationEndFrame = MSG_ReadUint16();
    }
    if (byteMask & EntityMessageBits::AnimationFrameTime) {
    	to->animationFramerate = MSG_ReadHalfFloat();
    }
}



/**
*
*   ClientMoveCommand Read/Write.
*
**/
#if USE_CLIENT
/**
*   @brief Write a client's delta move command.
**/
int MSG_WriteDeltaClientMoveCommand(const ClientMoveCommand* from, const ClientMoveCommand* cmd)
{
    // Send a null message in case we had none.
    if (!from) {
        from = &nullUserCmd;
    }

    //
    // send the movement message
    //
    int32_t bits = 0;

    if (cmd->input.viewAngles[0] != from->input.viewAngles[0]) {
        bits |= UserCommandBits::AngleX;
    }
    if (cmd->input.viewAngles[1] != from->input.viewAngles[1]) {
        bits |= UserCommandBits::AngleY;
    }
    if (cmd->input.viewAngles[2] != from->input.viewAngles[2]) {
        bits |= UserCommandBits::AngleZ;
    }
    if (cmd->input.forwardMove != from->input.forwardMove) {
        bits |= UserCommandBits::Forward;
    }
    if (cmd->input.rightMove != from->input.rightMove) {
        bits |= UserCommandBits::Side;
    }
    if (cmd->input.upMove != from->input.upMove) {
        bits |= UserCommandBits::Up;
    }
    if (cmd->input.buttons != from->input.buttons) {
        bits |= UserCommandBits::Buttons;
    }
    if (cmd->input.impulse != from->input.impulse) {
        bits |= UserCommandBits::Impulse;
    }

    // Write out the changed bits.
    MSG_WriteUint8(bits);

    if (bits & UserCommandBits::AngleX) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[0]);
    }
    if (bits & UserCommandBits::AngleY) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[1]);
    }
    if (bits & UserCommandBits::AngleZ) {
        MSG_WriteHalfFloat(cmd->input.viewAngles[2]);
    }

    if (bits & UserCommandBits::Forward) {
        MSG_WriteInt16(cmd->input.forwardMove);
    }
    if (bits & UserCommandBits::Side) {
        MSG_WriteInt16(cmd->input.rightMove);
    }
    if (bits & UserCommandBits::Up) {
        MSG_WriteInt16(cmd->input.upMove);
    }

    if (bits & UserCommandBits::Buttons) {
        MSG_WriteUint8(cmd->input.buttons);
    }

    if (bits & UserCommandBits::Impulse) {
        MSG_WriteUint8(cmd->input.impulse);
    }

    MSG_WriteUint8(cmd->input.msec);
    MSG_WriteUint8(cmd->input.lightLevel);

    // (Returned bits isn't used anywhere, but might as well keep it around.)
    return bits;
}

#endif // USE_CLIENT

/**
*   @brief Read a client's delta move command.
**/
void MSG_ReadDeltaClientMoveCommand(const ClientMoveCommand* from, ClientMoveCommand* to)
{
    int bits;

    if (from) {
        *to = *from;
    } else {
        *to = {};
    }

    bits = MSG_ReadUint8();

    // Read current angles.
    if (bits & UserCommandBits::AngleX) {
        to->input.viewAngles[0] = MSG_ReadHalfFloat();
    }
    if (bits & UserCommandBits::AngleY) {
        to->input.viewAngles[1] = MSG_ReadHalfFloat();
    }
    if (bits & UserCommandBits::AngleZ) {
        to->input.viewAngles[2] = MSG_ReadHalfFloat();
    }

    // Read movement.
    if (bits & UserCommandBits::Forward) {
        to->input.forwardMove = MSG_ReadInt16();
    }
    if (bits & UserCommandBits::Side) {
        to->input.rightMove = MSG_ReadInt16();
    }
    if (bits & UserCommandBits::Up) {
        to->input.upMove = MSG_ReadInt16();
    }

    // Read buttons.
    if (bits & UserCommandBits::Buttons) {
        to->input.buttons = MSG_ReadUint8();
    }

    if (bits & UserCommandBits::Impulse) {
        to->input.impulse = MSG_ReadUint8();
    }

    // Read time to run command.
    to->input.msec = MSG_ReadUint8();

    // Read the light level.
    to->input.lightLevel = MSG_ReadUint8();
}


/*
==============================================================================

            DEBUGGING STUFF

==============================================================================
*/

#ifdef _DEBUG

#define SHOWBITS(x) Com_LPrintf(PrintType::Developer, x " ")

#if USE_CLIENT
void MSG_ShowDeltaPlayerstateBits(int flags, int extraflags)
{
#define SP(b,s) if(flags&PS_##b) SHOWBITS(s)
#define SE(b,s) if(extraflags&EPS_##b) SHOWBITS(s)
    SP(PM_TYPE, "pmove.type");
    SP(PM_ORIGIN, "pmove.origin[x,y]");
    SE(M_ORIGIN2, "pmove.origin[z]");
    SP(PM_VELOCITY, "pmove.velocity[x,y]");
    SE(M_VELOCITY2, "pmove.velocity[z]");
    SP(PM_TIME, "pmove.time");
    SP(PM_FLAGS, "pmove.flags");
    SP(PM_GRAVITY, "pmove.gravity");
    SP(PM_DELTA_ANGLES, "pmove.deltaAngles");
    SP(PM_VIEW_OFFSET, "pmove.viewOffset");
    SP(PM_VIEW_ANGLES, "pmove.viewAngles");
    SP(KICKANGLES, "kickAngles");
    SP(WEAPONINDEX, "gunIndex");
    SP(GUNANIMATION_TIME_START, "gunAnimationStartTime");
    SP(GUNANIMATION_FRAME_START, "gunAnimationFrameStart");
    SP(GUNANIMATION_FRAME_END, "gunAnimationFrameEnd");
    SP(GUNANIMATION_FRAME_TIME, "gunAnimationFrameTime");
    SP(GUNANIMATION_LOOP_COUNT, "gunAnimationLoopCount");
    SP(GUNANIMATION_LOOP_FORCE, "gunAnimationForceLoop");
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

#define S(b,s) if(bits&UserCommandBits::##b) SHOWBITS(s)
    S(AngleX, "angle.x");
    S(AngleY, "angle.y");
    S(AngleZ, "angle.z");
    S(Forward, "forward");
    S(Side, "side");
    S(Up, "up");
    S(Buttons, "buttons");
    S(Impulse, "msec");
#undef S
}

void MSG_ShowDeltaEntityBits(uint32_t byteMask)
{
#define S(b,s) if(byteMask&EntityMessageBits::##b) SHOWBITS(s)
    S(OriginX, "origin.x");
    S(OriginY, "origin.y");
    S(OriginZ, "origin.z");
    S(AngleX, "angles.x");
    S(AngleY, "angles.y");
    S(AngleZ, "angles.z");
    S(OldOrigin, "oldOrigin");
    S(EventID, "eventID");

    S(Sound, "sound");    
    S(Solid, "solid");
    S(AnimationFrame, "animationFrame");
    S(AnimationTimeStart, "animationTimeStart");
    S(AnimationFrameStart, "animationFrameStart");
    S(AnimationFrameEnd, "animationFrameEnd");
    S(AnimationFrameTime, "animationFrameTime");
    S(Skin, "skin");
    S(ModelIndex, "modelIndex");
    S(ModelIndex2, "modelIndex2");
    S(ModelIndex3, "modelIndex3");
    S(ModelIndex4, "modelIndex4");
    S(EntityEffects, "entityEffects");
    S(RenderEffects, "renderEffects");
#undef S
}

const char* MSG_ServerCommandString(int cmd)
{
    switch (cmd) {
    case -1: return "END OF MESSAGE";
    default: return "UNKNOWN COMMAND";
#define S(x) case ServerCommand::##x: return "ServerCommand::" #x;
            S(Bad)
            // TODO: Protocol todo: add a game callback for this...?
            //S(muzzleflash)
            //S(muzzleflash2)
            //S(temp_entity)
            //S(layout)
            //S(inventory)
            S(Padding)
            S(Disconnect)
            S(Reconnect)
            S(Sound)
            S(Print)
            S(StuffText)
            S(ServerData)
            S(ConfigString)
            S(SpawnBaseline)
            S(CenterPrint)
            S(Download)
            S(PlayerInfo)
            S(PacketEntities)
	        S(DeltaPacketEntities)
            S(Frame)
            S(ZPacket)
            S(ZDownload)
            S(GameState)
#undef S
    }
}
#endif // USE_CLIENT
#endif // _DEBUG

