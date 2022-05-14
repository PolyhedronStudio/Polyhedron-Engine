/***
*
*	License here.
*
*	@file
*
*	Message SizeBuffer Read/Write Functionality. (Some bits borrowed from QFusion.)
* 
*	Handles byte ordering and avoids alignment errors.
*
***/
#include "Shared/Shared.h"
#include "../HalfFloat.h"
#include "../Messaging.h"
#include "../Protocol.h"
#include "../SizeBuffer.h"

// TODO: Make this not needed, let the Game Modules supply an API for these needs.
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
//const EntityState       nullEntityState = {};
//const PlayerState       nullPlayerState = {};
//const ClientMoveCommand nullUserCmd = {};



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
*   @brief Writes a 32 bit integer.
**/
void MSG_WriteUint32( uint32_t c ) {
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
*   @return 32 bit unsigned integer.
**/
uint32_t MSG_ReadUint32() {
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
*   Delta Entity Number Read/Write.
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

	if (number > 4000) {
		Com_DPrintf("OH MIJN GOD WAS DIT DAH? LOL\n");
	}
    if (number < 0) {
		number *= -1;
		*remove = true;
    }

    return number;
}