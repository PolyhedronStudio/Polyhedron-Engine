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
#include "Shared/Shared.h"
#include "Common/HalfFloat.h"
#include "Common/Msg.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"
#include "SharedGame/Protocol.h"

// Assertion.
#include <cassert>


/**
*
*   Message Buffer functionality.
*
**/
static constexpr int32_t MAX_MSG_STRING_CHARS = 2048;

/**
*   @brief Initializes a MessageBuffer with said data of given length.
**/
//void MSG_Init( MessageBuffer *msg, uint8_t *data, size_t length ) {
void MSG_Init(MessageBuffer* msg, uint8_t* data, size_t length) {
	memset( msg, 0, sizeof( *msg ) );
	msg->data = data;
	msg->maximumSize = length;
	msg->currentSize = 0;
	msg->isCompressed = false;
}

/**
*   @brief Clears the Messagebuffer.
**/
void MSG_Clear( MessageBuffer *msg ) {
	msg->currentSize = 0;
	msg->isCompressed = false;
}

/**
*   @brief  Tells the MessageBuffer that the wished for space has been written to.
* 
*   @return Pointer to the acquired space which to write in. nullptr on failure.
**/
void *MSG_GetSpace( MessageBuffer *msg, size_t length ) {
	void *ptr;

	assert( msg->currentSize + length <= msg->maximumSize);
	if( msg->currentSize + length > msg->maximumSize ) {
		Com_Error( ERR_FATAL, "MSG_GetSpace: overflowed" );
	}

	ptr = msg->data + msg->currentSize;
	msg->currentSize += length;
	return ptr;
}

/**
*   @brief  Writes given data to the MessageBuffer.
**/
void MSG_WriteData( MessageBuffer *msg, const void *data, size_t length ) {
#if 0
	uint32_t i;
	for( i = 0; i < length; i++ )
		MSG_WriteUint8( msg, ( (uint8_t *)data )[i] );
#else
	MSG_CopyData( msg, data, length );
#endif
}

/**
*   @brief Reads the data into the message buffer.
**/
void MSG_ReadData(MessageBuffer* msg, void* data, size_t length) {
    for (uint32_t i = 0; i < length; i++) {
		((uint8_t*)data)[i] = MSG_ReadUint8(msg);
    }
}

/**
*   @brief Copies an entire piece of data into the MessageBuffer.
**/
void MSG_CopyData( MessageBuffer *buf, const void *data, size_t length ) {
	memcpy( MSG_GetSpace( buf, length ), data, length );
}

/**
*   @brief Skips reading data in the message buffer, like flushing does.
**/
int32_t MSG_SkipData(MessageBuffer* msg, size_t length) {
    if (msg->readCount + length <= msg->currentSize) {
		msg->readCount += length;
		return 1;
    }
    return 0;
}



/**
*
*   Wire Types write functionality.
*
**/
/**
*   @brief Writes a signed 8 bit byte.
**/
void MSG_WriteInt8( MessageBuffer *msg, int32_t c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 1 );
	buf[0] = ( char )c;
}

/**
*   @brief Writes an unsigned 8 bit byte.
**/
void MSG_WriteUint8( MessageBuffer *msg, int32_t c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 1 );
	buf[0] = ( uint8_t )( c & 0xff );
}

/**
*   @brief Writes a signed 16 bit short.
**/
void MSG_WriteInt16( MessageBuffer *msg, int32_t c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 2 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
}

/**
*   @brief Writes an unsigned 16 bit short.
**/
void MSG_WriteUint16( MessageBuffer *msg, unsigned c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 2 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
}

/**
*   @brief Writes a 32 bit integer.
**/
void MSG_WriteInt32( MessageBuffer *msg, int32_t c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 4 );
	buf[0] = ( uint8_t )( c & 0xff );
	buf[1] = ( uint8_t )( ( c >> 8 ) & 0xff );
	buf[2] = ( uint8_t )( ( c >> 16 ) & 0xff );
	buf[3] = ( uint8_t )( c >> 24 );
}

/**
*   @brief Writes a 64 bit integer.
**/
void MSG_WriteInt64( MessageBuffer *msg, int64_t c ) {
	uint8_t *buf = ( uint8_t* )MSG_GetSpace( msg, 8 );
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
void MSG_WriteUintBase128( MessageBuffer *msg, uint64_t c ) {
	uint8_t buf[10];
	size_t len = 0;

	do {
		buf[len] = c & 0x7fU;
		if ( c >>= 7 ) {
			buf[len] |= 0x80U;
		}
		len++;
	} while( c );

	MSG_WriteData( msg, buf, len );
}

/**
*   @brief Writes a zic-zac encoded signed integer.
**/
void MSG_WriteIntBase128( MessageBuffer *msg, int64_t c ) {
	// use Zig-zag encoding for signed integers for more efficient storage
	uint64_t cc = (uint64_t)(c << 1) ^ (uint64_t)(c >> 63);
	MSG_WriteUintBase128( msg, cc );
}

/**
*   @brief Writes a full precision float. (Transfered over the wire as an int32_t).
**/
void MSG_WriteFloat( MessageBuffer *msg, float f ) {
	union {
		float f;
		int32_t l;
	} dat;

	dat.f = f;
	MSG_WriteInt32( msg, dat.l );
}

/**
*   @brief Writes a half float, lesser precision. (Transfered over the wire as an uint16_t)
**/
void MSG_WriteHalfFloat( MessageBuffer *msg, float f ) {
	MSG_WriteUint16( msg, float_to_half( f ) );
}

/**
*   @brief Writes a character string.
**/
void MSG_WriteString( MessageBuffer *msg, const char *s ) {
	if( !s ) {
		MSG_WriteData( msg, "", 1 );
	} else {
		int32_t l = strlen( s );
		if( l >= MAX_MSG_STRING_CHARS ) {
			Com_Printf( "MSG_WriteString: MAX_MSG_STRING_CHARS overflow" );
			MSG_WriteData( msg, "", 1 );
			return;
		}
		MSG_WriteData( msg, s, l + 1 );
	}
}



/**
*
*   Wire Types write functionality.
*
**/
/**
*   @brief Engages reading mode for the message buffer.
**/
void MSG_BeginReading(MessageBuffer* msg) { 
	msg->readCount = 0; 
}

/**
*   @return Signed 8 bit byte.
**/
int32_t MSG_ReadInt8(MessageBuffer* msg) {
    int32_t i = (signed char)msg->data[msg->readCount++];
    if (msg->readCount > msg->currentSize) {
		i = -1;
    }
    return i;
}

/**
*   @return Unsigned 8 bit byte.
**/
int32_t MSG_ReadUint8(MessageBuffer* msg) {
    msg->readCount++;
    if (msg->readCount > msg->currentSize) {
		return 0;
    }

    return (unsigned char)(msg->data[msg->readCount - 1]);
}

/**
*   @return Signed 16 bit short.
**/
int16_t MSG_ReadInt16(MessageBuffer* msg) {
    msg->readCount += 2;
    if (msg->readCount > msg->currentSize) {
		return -1;
    }
    return (int16_t)(msg->data[msg->readCount - 2] | (msg->data[msg->readCount - 1] << 8));
}

/**
*   @return Unsigned 16 bit short.
**/
uint16_t MSG_ReadUint16(MessageBuffer* msg) {
    msg->readCount += 2;
    if (msg->readCount > msg->currentSize) {
	return 0;
    }
    return (uint16_t)(msg->data[msg->readCount - 2] | (msg->data[msg->readCount - 1] << 8));
}

/**
*   @return 32 bit integer.
**/
int32_t MSG_ReadInt32(MessageBuffer* msg) {
    msg->readCount += 4;
    if (msg->readCount > msg->currentSize) {
		return -1;
    }

    return msg->data[msg->readCount - 4] 
		| (msg->data[msg->readCount - 3] << 8) 
		| (msg->data[msg->readCount - 2] << 16) 
		| (msg->data[msg->readCount - 1] << 24);
}

/**
*   @return 64 bit integer.
**/
int64_t MSG_ReadInt64( MessageBuffer *msg ) {
	msg->readCount += 8;
	if( msg->readCount > msg->currentSize ) {
		return -1;
	}

	return ( int64_t )msg->data[msg->readCount - 8]
		| ( ( int64_t )msg->data[msg->readCount - 7] << 8L )
		| ( ( int64_t )msg->data[msg->readCount - 6] << 16L )
		| ( ( int64_t )msg->data[msg->readCount - 5] << 24L )
		| ( ( int64_t )msg->data[msg->readCount - 4] << 32L )
		| ( ( int64_t )msg->data[msg->readCount - 3] << 40L )
		| ( ( int64_t )msg->data[msg->readCount - 2] << 48L )
		| ( ( int64_t )msg->data[msg->readCount - 1] << 56L );
}

/**
*   @return Base 128 decoded unsigned integer.
**/
uint64_t MSG_ReadUintBase128(MessageBuffer* msg) {
    size_t   len = 0;
    uint64_t i = 0;

    while (len < 10) {
	uint8_t c = MSG_ReadUint8(msg);
	i |= (c & 0x7fLL) << (7 * len);
	len++;
		if (!(c & 0x80)) {
			break;
		}
    }

    return i;
}

/**
*   @return Zig-Zac decoded integer.
**/
int64_t MSG_ReadIntBase128(MessageBuffer* msg) {
    // un-Zig-Zag our value back to a signed integer
    uint64_t c = MSG_ReadUintBase128(msg);
    return (int64_t)(c >> 1) ^ (-(int64_t)(c & 1));
}

/**
*   @return The full precision float.
**/
float MSG_ReadFloat(MessageBuffer* msg) {
    union {
		float f;
		int32_t   l;
    } dat;

    dat.l = MSG_ReadInt32(msg);
    if (msg->readCount > msg->currentSize) {
		dat.f = -1;
    }
    return dat.f;
}

/**
*   @return A half float, converted to float, keep in mind that half floats have less precision.
**/
float MSG_ReadHalfFloat(MessageBuffer* msg) { 
	return half_to_float(MSG_ReadUint16(msg)); 
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
static inline char* MSG_ReadString2(MessageBuffer* msg, bool linebreak = false) {
    int32_t		l, c;
    static char string[MAX_MSG_STRING_CHARS];

    l = 0;
    do {
		c = MSG_ReadUint8(msg);
		if (c == -1 || c == 0 || (linebreak && c == '\n')) {
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
char* MSG_ReadString(MessageBuffer* msg) { 
	return MSG_ReadString2(msg); 
}

/**
*   @return The part of the string data up till the first '\n'
**/
char* MSG_ReadStringLine(MessageBuffer* msg) { 
	return MSG_ReadString2(msg, true); 
}



/**
*
*   Field Handling. Encode/Decode.
*
**/
/**
*   @return Field bytes.
**/
static size_t MSG_FieldBytes( const MessageBufferField *field ) {
	if( field->bits == 0 ) {
		return sizeof( float );
	}
	return field->bits >> 3;
}

/**
*   @return True if the field is identical, false otherwise.
**/
static bool MSG_CompareField( const uint8_t *from, const uint8_t *to, const MessageBufferField *field ) {
	int32_t itv, ifv;
	bool btv, bfv;
	int64_t bitv, bifv;
	float ftv, ffv;

	switch( field->bits ) {
		case 0:
			ftv = *((float *)( to + field->offset ));
			ffv = *((float *)( from + field->offset ));
			return ftv != ffv;
		case 1:
			btv = *((bool *)( to + field->offset ));
			bfv = *((bool *)( from + field->offset ));
			return btv != bfv;
		case 8:
			itv = *((int8_t *)( to + field->offset ));
			ifv = *((int8_t *)( from + field->offset ));
			return itv != ifv;
		case 16:
			itv = *((int16_t *)( to + field->offset ));
			ifv = *((int16_t *)( from + field->offset ));
			return itv != ifv;
		case 32:
			itv = *((int32_t *)( to + field->offset ));
			ifv = *((int32_t *)( from + field->offset ));
			return itv != ifv;
		case 64:
			bitv = *((int64_t *)( to + field->offset ));
			bifv = *((int64_t *)( from + field->offset ));
			return bitv != bifv;
		default:
			Com_Error( ERR_FATAL, "MSG_CompareField: unknown field bits value %i", field->bits );
	}

	return false;
}

/**
*   @brief Writes a field to the MessageBuffer using the chosen encoding.
**/
static void MSG_WriteField( MessageBuffer *msg, const uint8_t *to, const MessageBufferField *field ) {
	switch( field->encoding ) {
	case WIRE_BOOL:
		// Why is this missing? lol.
		break;
	case WIRE_FIXED_INT8:
		MSG_WriteInt8( msg, *((int8_t *)( to + field->offset )) );
		break;
	case WIRE_FIXED_INT16:
		MSG_WriteInt16( msg, *((int16_t *)( to + field->offset )) );
		break;
	case WIRE_FIXED_INT32:
		MSG_WriteInt32( msg, *((int32_t *)( to + field->offset )) );
		break;
	case WIRE_FIXED_INT64:
		MSG_WriteInt64( msg, *((int64_t *)( to + field->offset )) );
		break;
	case WIRE_FLOAT:
		MSG_WriteFloat( msg, *((float *)( to + field->offset )) );
		break;
	case WIRE_HALF_FLOAT:
		MSG_WriteHalfFloat( msg, (*((float *)( to + field->offset ))) );
		break;
	case WIRE_ANGLE:
		MSG_WriteHalfFloat( msg, AngleMod( (*((float *)( to + field->offset ))) ) );
		break;
	case WIRE_BASE128:
		switch( field->bits ) {
		case 8:
			MSG_WriteInt8( msg, *((int8_t *)( to + field->offset )) );
			break;
		case 16:
			MSG_WriteIntBase128( msg, *((int16_t *)( to + field->offset )) );
			break;
		case 32:
			MSG_WriteIntBase128( msg, *((int32_t *)( to + field->offset )) );
			break;
		case 64:
			MSG_WriteIntBase128( msg, *((int64_t *)( to + field->offset )) );
			break;
		default:
			Com_Error( ERR_FATAL, "MSG_WriteField: unknown base128 field bits value %i", field->bits );
			break;
		}
		break;
	case WIRE_UBASE128:
		switch( field->bits ) {
		case 8:
			MSG_WriteUint8( msg, *((uint8_t *)( to + field->offset )) );
			break;
		case 16:
			MSG_WriteUintBase128( msg, *((uint16_t *)( to + field->offset )) );
			break;
		case 32:
			MSG_WriteUintBase128( msg, *((uint32_t *)( to + field->offset )) );
			break;
		case 64:
			MSG_WriteUintBase128( msg, *((uint64_t *)( to + field->offset )) );
			break;
		default:
			Com_Error( ERR_FATAL, "MSG_WriteField: unknown base128 field bits value %i", field->bits );
			break;
		}
		break;
	default:
		Com_Error( ERR_FATAL, "MSG_WriteField: unknown encoding type %i", field->encoding );
		break;
	}
}

/**
*   @brief Reads and decodes field from the MessageBuffer
**/
static void MSG_ReadField( MessageBuffer *msg, uint8_t *to, const MessageBufferField *field ) {
	switch( field->encoding ) {
	case WIRE_BOOL:
		*((bool *)( to + field->offset )) ^= true;
		break;
	case WIRE_FIXED_INT8:
		*((int8_t *)( to + field->offset )) = MSG_ReadInt8( msg );
		break;
	case WIRE_FIXED_INT16:
		*((int16_t *)( to + field->offset )) = MSG_ReadInt16( msg );
		break;
	case WIRE_FIXED_INT32:
		*((int32_t *)( to + field->offset )) = MSG_ReadInt32( msg );
		break;
	case WIRE_FIXED_INT64:
		*((int64_t *)( to + field->offset )) = MSG_ReadInt64( msg );
		break;
	case WIRE_FLOAT:
		*((float *)( to + field->offset )) = MSG_ReadFloat( msg );
		break;
	case WIRE_HALF_FLOAT:
		*((float *)( to + field->offset )) = MSG_ReadHalfFloat( msg );
		break;
	case WIRE_ANGLE:
		*((float *)( to + field->offset )) = MSG_ReadHalfFloat( msg );
		break;
	case WIRE_BASE128:
		switch( field->bits ) {
		case 8:
			*((int8_t *)( to + field->offset )) = MSG_ReadInt8( msg );
			break;
		case 16:
			*((int16_t *)( to + field->offset )) = MSG_ReadIntBase128( msg );
			break;
		case 32:
			*((int32_t *)( to + field->offset )) = MSG_ReadIntBase128( msg );
			break;
		case 64:
			*((int64_t *)( to + field->offset )) = MSG_ReadIntBase128( msg );
			break;
		default:
			Com_Error( ERR_FATAL, "MSG_WriteField: unknown base128 field bits value %i", field->bits );
			break;
		}
		break;
	case WIRE_UBASE128:
		switch( field->bits ) {
		case 8:
			*((uint8_t *)( to + field->offset )) = MSG_ReadUint8( msg );
			break;
		case 16:
			*((uint16_t *)( to + field->offset )) = MSG_ReadUintBase128( msg );
			break;
		case 32:
			*((uint32_t *)( to + field->offset )) = MSG_ReadUintBase128( msg );
			break;
		case 64:
			*((uint64_t *)( to + field->offset )) = MSG_ReadUintBase128( msg );
			break;
		default:
			Com_Error( ERR_FATAL, "MSG_WriteField: unknown base128 field bits value %i", field->bits );
			break;
		}
		break;
	default:
		Com_Error( ERR_FATAL, "MSG_WriteField: unknown encoding type %i", field->encoding );
		break;
	}
}

/**
*   @brief Writes the fieldmask.
**/
void MSG_WriteFieldMask( MessageBuffer *msg, const uint8_t *fieldMask, unsigned byteMask ) {
	size_t b;

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			MSG_WriteUint8( msg, fieldMask[b] );
		}
		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Reads the fieldmask.
**/
void MSG_ReadFieldMask( MessageBuffer *msg, uint8_t *fieldMask, size_t maskSize, unsigned byteMask ) {
	size_t b;

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			if( b >= maskSize ) {
				Com_Error( ERR_FATAL, "MSG_ReadFieldMask: i >= maskSize" );
			}
			fieldMask[b] = MSG_ReadUint8( msg );
		}
		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Compares arrays to each other, returns the bytemask.
**/
static unsigned MSG_CompareArrays( const void *from, const void *to, const MessageBufferField *field, uint8_t *elemMask, size_t maskSize, bool quick ) {
	size_t i;
	unsigned byteMask;
	const size_t bytes = MSG_FieldBytes( field );
	const size_t maxElems = field->count;
	const uint8_t *bfrom = static_cast<const uint8_t*>(from), *bto = static_cast<const uint8_t*>(to);

	byteMask = 0;
	for( i = 0; i < maxElems; i++ ) {
		size_t byte = i >> 3;

		if( elemMask != NULL && byte > maskSize ) {
			Com_Error( ERR_FATAL, "MSG_CompareArrays: byte > maskSize" );
		}
		if( byte > 32 ) {
			Com_Error( ERR_FATAL, "MSG_CompareArrays: byte > 32" );
		}

		if( MSG_CompareField( bfrom, bto, field ) ) {
			if( elemMask != NULL ) {
				elemMask[byte] |= (1 << (i & 7));
			}
			byteMask |= (1 << (byte & 7));
			if( quick ) {
				return byteMask;
			}
		}

		bfrom += bytes;
		bto += bytes;
	}

	return byteMask;
}

/**
*   @brief Writes array element fields.
**/
static void MSG_WriteArrayElems( MessageBuffer *msg, const void *to, const MessageBufferField *field, const uint8_t *elemMask, unsigned byteMask ) {
	size_t b;
	const size_t bytes = MSG_FieldBytes( field );
	const size_t maxElems = field->count;
	const uint8_t *bto = static_cast<const uint8_t*>(to);

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			unsigned fm = elemMask[b];
			size_t f = b << 3;

			while( fm ) {
				if( f >= maxElems )
					return;

				if( fm & 1 ) {
					MSG_WriteField( msg, bto + f * bytes, field );
				}
				f++;
				fm >>= 1;
			}
		}

		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Reads array element fields.
**/
static void MSG_ReadArrayElems( MessageBuffer *msg, void *to, const MessageBufferField *field, const uint8_t *elemMask, size_t maskSize, unsigned byteMask ) {
	size_t b;
	uint8_t *bto = static_cast<uint8_t*>(to);
	const size_t bytes = MSG_FieldBytes( field );
	const size_t maxElems = field->count;

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			unsigned fm;
			size_t fn = b << 3;

			if( b >= maskSize ) {
				Com_Error( ERR_FATAL, "MSG_ReadArrayElems: b >= maxSize" );
			}

			fm = elemMask[b];
			while( fm ) {
				assert( fn < maxElems );
				if( fn >= maxElems ) {
					Com_Error( ERR_FATAL, "MSG_ReadArrayElems: fn >= maxElems" );
				}

				if( fm & 1 ) {
					MSG_ReadField( msg, bto + fn * bytes, field );
				}

				fn++;
				fm >>= 1;
			}
		}

		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Writes the delta of two arrays into the MessageBuffer.
**/
static void MSG_WriteDeltaArray( MessageBuffer *msg, const void *from, const void *to, const MessageBufferField *field ) {
	unsigned byteMask;
	uint8_t elemMask[32] = { };
	const size_t numElems = field->count;

	assert( numElems < 256 );
	if( numElems > 256 ) {
		Com_Error( ERR_FATAL, "MSG_WriteDeltaArray: numFields == %" PRIu32, (unsigned)numElems );
	}

	byteMask = MSG_CompareArrays( from, to, field, elemMask, sizeof( elemMask ), false );

	if( numElems <= 8 ) {
		// we don't need the byteMask in case all field bits fit a single byte
		byteMask = 1;
	} else {
		MSG_WriteUintBase128( msg, byteMask );
	}

	MSG_WriteFieldMask( msg, elemMask, byteMask );

	MSG_WriteArrayElems( msg, to, field, elemMask, byteMask );
}

/**
*   @brief Reads the array deltas from MessageBuffer.
**/
static void MSG_ReadDeltaArray( MessageBuffer *msg, const void *from, void *to, const MessageBufferField *field ) {
	unsigned byteMask;
	uint8_t elemMask[32] = { };
	const size_t bytes = MSG_FieldBytes( field );
	const size_t maxElems = field->count;

	assert( maxElems < 256 );
	if( maxElems > 256 ) {
		Com_Error( ERR_FATAL, "MSG_ReadDeltaArray: numFields == %" PRIu32, (unsigned)maxElems );
	}

	// set everything to the state we are delta'ing from
	// we actually do this in MSG_ReadDeltaStruct
	// memcpy( (uint8_t *)to + field->offset, (uint8_t *)from + field->offset, bytes * maxElems );

	if( maxElems <= 8 ) {
		// we don't need the byteMask in case all field bits fit a single byte
		byteMask = 1;
	} else {
		byteMask = MSG_ReadUintBase128( msg );
	}

	MSG_ReadFieldMask( msg, elemMask, sizeof( elemMask ), byteMask );

	MSG_ReadArrayElems( msg, to, field, elemMask, sizeof( elemMask ), byteMask );
}

/**
*   @brief Compares struct fields.
**/
static unsigned MSG_CompareStructs( const void *from, const void *to, const MessageBufferField *fields, size_t numFields, uint8_t *fieldMask, size_t maskSize ) {
	size_t i;
	unsigned byteMask;

	byteMask = 0;
	for( i = 0; i < numFields; i++ ) {
		size_t byte = i >> 3;
		bool change;
		const MessageBufferField *f = &fields[i];

		if( fieldMask != NULL && byte > maskSize ) {
			Com_Error( ERR_FATAL, "MSG_CompareStructs: byte > maskSize" );
		}
		if( byte > 32 ) {
			Com_Error( ERR_FATAL, "MSG_CompareStructs: byte > 32" );
		}

		if( f->count > 1 ) {
			change = MSG_CompareArrays( from, to, f, NULL, 0, true ) != 0;
		} else {
			change = MSG_CompareField( static_cast<const uint8_t*>(from), static_cast<const uint8_t*>(to), f );
		}

		if( change ) {
			if( fieldMask != NULL ) {
				fieldMask[byte] |= (1 << (i & 7));
			}
			byteMask |= (1 << (byte & 7));
		}
	}

	return byteMask;
}

/**
*   @brief Writes struct fields.
**/
static void MSG_WriteStructFields(MessageBuffer* msg, const void* from, const void* to, const MessageBufferField* fields, size_t numFields, const uint8_t* fieldMask, uint32_t byteMask) {
	size_t b;

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			unsigned fm = fieldMask[b];
			size_t fn = b << 3;

			while( fm ) {
				if( fn >= numFields )
					return;
				
				if( fm & 1 ) {
					const MessageBufferField *f = &fields[fn];

					if( f->count > 1 ) {
						MSG_WriteDeltaArray( msg, from, to, f );
					} else {
						MSG_WriteField( msg, static_cast<const uint8_t*>(to), f );
					}
				}
				fn++;
				fm >>= 1;
			}
		}

		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Reads struct fields.
**/
static void MSG_ReadStructFields(MessageBuffer* msg, const void* from, void* to, const MessageBufferField* fields, size_t numFields, const uint8_t* fieldMask, size_t maskSize, uint32_t byteMask) {
	size_t b;

	b = 0;
	while( byteMask ) {
		if( byteMask & 1 ) {
			unsigned fm;
			size_t fn = b << 3;

			if( b >= maskSize ) {
				Com_Error( ERR_FATAL, "MSG_ReadStructFields: b >= maxSize" );
			}

			fm = fieldMask[b];
			while( fm ) {
				assert( fn < numFields );
				if( fn >= numFields ) {
					Com_Error( ERR_FATAL, "MSG_ReadStructFields: f >= numFields" );
				}

				if( fm & 1 ) {
					const MessageBufferField *f = &fields[fn];
					if( f->count > 1 ) {
						MSG_ReadDeltaArray( msg, from, to, f );
					} else {
						MSG_ReadField( msg, static_cast<uint8_t*>(to), f );
					}
				}

				fn++;
				fm >>= 1;
			}
		}

		b++;
		byteMask >>= 1;
	}
}

/**
*   @brief Writes delta struct fields.
**/
void MSG_WriteDeltaStruct( MessageBuffer *msg, const void *from, const void *to, const MessageBufferField *fields, size_t numFields ) {
	unsigned byteMask;
	uint8_t fieldMask[32] = { };

	assert( numFields < 256 );
	if( numFields > 256 ) {
		Com_Error( ERR_FATAL, "MSG_WriteDeltaStruct: numFields == %" PRIu32, (unsigned)numFields );
	}

	byteMask = MSG_CompareStructs( from, to, fields, numFields, fieldMask, sizeof( fieldMask ) );

	if( numFields <= 8 ) {
		// we don't need the byteMask in case all field bits fit a single byte
		byteMask = 1;
	} else {
		MSG_WriteUintBase128( msg, byteMask );
	}

	MSG_WriteFieldMask( msg, fieldMask, byteMask );

	MSG_WriteStructFields( msg, from, to, fields, numFields, fieldMask, byteMask );
}

/**
*   @brief Reads delta struct fields.
**/
void MSG_ReadDeltaStruct( MessageBuffer *msg, const void *from, void *to, size_t size, const MessageBufferField *fields, size_t numFields ) {
	unsigned byteMask;
	uint8_t fieldMask[32] = { };

	assert( numFields < 256 );
	if( numFields > 256 ) {
		Com_Error( ERR_FATAL, "MSG_ReadDeltaStruct: numFields == %" PRIu32, (unsigned)numFields );
	}

	// set everything to the state we are delta'ing from
	memcpy( to, from, size );

	if( numFields <= 8 ) {
		// we don't need the byteMask in case all field bits fit a single byte
		byteMask = 1;
	} else {
		byteMask = MSG_ReadUintBase128( msg );
	}

	MSG_ReadFieldMask( msg, fieldMask, sizeof( fieldMask ), byteMask );

	MSG_ReadStructFields( msg, from, to, fields, numFields, fieldMask, sizeof( fieldMask ), byteMask );
}



//==================================================
// DELTA ENTITIES
//==================================================

//#define ESOFS(x) offsetof(EntityState, x)
//
//static const MessageBufferField ent_state_fields[] = {
//    //{ ESOFS(events[0]), 32, 1, WIRE_UBASE128 },
//    //{ ESOFS(eventParms[0]), 32, 1, WIRE_BASE128 },
//
//    { ESOFS(origin[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin[2]), 0, 1, WIRE_FLOAT },
//
//    { ESOFS(angles[0]), 0, 1, WIRE_ANGLE },
//    { ESOFS(angles[1]), 0, 1, WIRE_ANGLE },
//    { ESOFS(angles[2]), 0, 1, WIRE_ANGLE },
//
//	{ ESOFS(oldOrigin[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(oldOrigin[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(oldOrigin[2]), 0, 1, WIRE_FLOAT },
//
////    { ESOFS(teleported), 1, 1, WIRE_BOOL },
//
////    { ESOFS(type), 32, 1, WIRE_UBASE128 },
//    { ESOFS(solid), 32, 1, WIRE_UBASE128 },
//    //{ ESOFS(frame), 32, 1, WIRE_UBASE128 },
//    { ESOFS(modelIndex), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(modelindex2), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(modelindex3), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(modelindex4), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(svflags), 32, 1, WIRE_UBASE128 },
//    { ESOFS(skinnum), 32, 1, WIRE_BASE128 },
//    { ESOFS(effects), 32, 1, WIRE_UBASE128 },
//    { ESOFS(ownerNum), 32, 1, WIRE_BASE128 },
//    { ESOFS(targetNum), 32, 1, WIRE_BASE128 },
//    { ESOFS(sound), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(modelindex2), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(attenuation), 0, 1, WIRE_HALF_FLOAT },
//    { ESOFS(counterNum), 32, 1, WIRE_BASE128 },
//    { ESOFS(bodyOwner), 32, 1, WIRE_UBASE128 },
//    { ESOFS(channel), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(events[1]), 32, 1, WIRE_UBASE128 },
//    { ESOFS(eventParms[1]), 32, 1, WIRE_BASE128 },
//    { ESOFS(weapon), 32, 1, WIRE_UBASE128 },
//    { ESOFS(firemode), 32, 1, WIRE_FIXED_INT8 },
//    { ESOFS(damage), 32, 1, WIRE_UBASE128 },
//    { ESOFS(range), 32, 1, WIRE_UBASE128 },
//    { ESOFS(team), 32, 1, WIRE_FIXED_INT8 },
//
//    { ESOFS(origin2[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin2[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin2[2]), 0, 1, WIRE_FLOAT },
//
//    { ESOFS(origin3[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin3[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(origin3[2]), 0, 1, WIRE_FLOAT },
//
//    { ESOFS(linearMovementTimeStamp), 32, 1, WIRE_UBASE128 },
//    { ESOFS(linearMovement), 1, 1, WIRE_BOOL },
//    { ESOFS(linearMovementDuration), 32, 1, WIRE_UBASE128 },
//    { ESOFS(linearMovementVelocity[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementVelocity[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementVelocity[2]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementBegin[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementBegin[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementBegin[2]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementEnd[0]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementEnd[1]), 0, 1, WIRE_FLOAT },
//    { ESOFS(linearMovementEnd[2]), 0, 1, WIRE_FLOAT },
//
//    { ESOFS(itemNum), 32, 1, WIRE_UBASE128 },
//
//    { ESOFS(angles[2]), 0, 1, WIRE_ANGLE },
//
//    { ESOFS(colorRGBA), 32, 1, WIRE_FIXED_INT32 },
//
//    { ESOFS(light), 32, 1, WIRE_FIXED_INT32 },
//};

/*
* MSG_WriteEntityNumber
*/
void MSG_WriteEntityNumber(MessageBuffer* msg, int32_t number, bool remove, uint32_t byteMask) {
    MSG_WriteIntBase128(msg, number * (remove ? -1 : 1));
    MSG_WriteUintBase128(msg, byteMask);
}

///*
//* MSG_WriteDeltaEntity
//*
//* Writes part of a packetentities message.
//* Can delta from either a baseline or a previous packet_entity
//*/
//void MSG_WriteDeltaEntity(MessageBuffer* msg, const EntityState* from, const EntityState* to, bool force) {
//    int32_t		       number;
//    unsigned	       byteMask;
//    uint8_t	       fieldMask[32] = { };
//    const MessageBufferField* fields = ent_state_fields;
//    int32_t		       numFields = sizeof(ent_state_fields) / sizeof(ent_state_fields[0]);
//
//    assert(numFields < 256);
//    if (numFields > 256) {
//		Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: numFields == %i", numFields);
//    }
//
//    if (!to) {
//		if (!from) {
//			Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: Unset base state");
//		}
//		number = from->number;
//    } else {
//		number = to->number;
//    }
//
//    if (!number) {
//		Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: Unset entity number");
//    } else if (number >= MAX_EDICTS) {
//		Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: Entity number >= MAX_EDICTS");
//    } else if (number < 0) {
//		Com_Error(ERR_FATAL, "MSG_WriteDeltaEntity: Invalid Entity number");
//    }
//
//    if (!to) {
//		// remove
//		MSG_WriteEntityNumber(msg, number, true, 0);
//		return;
//    }
//
//    byteMask = MSG_CompareStructs(from, to, fields, numFields, fieldMask, sizeof(fieldMask));
//    if (!byteMask && !force) {
//		// no changes
//		return;
//    }
//
//    MSG_WriteEntityNumber(msg, number, false, byteMask);
//
//    MSG_WriteFieldMask(msg, fieldMask, byteMask);
//
//    MSG_WriteStructFields(msg, from, to, fields, numFields, fieldMask, byteMask);
//}

/*
* MSG_ReadEntityNumber
*
* Returns the entity number and the remove bit
*/
int32_t MSG_ReadEntityNumber(MessageBuffer* msg, bool* remove, uint32_t* byteMask) {
    int32_t number;

    *remove = false;
    number = (int32_t)MSG_ReadIntBase128(msg);
    *byteMask = MSG_ReadUintBase128(msg);

    if (number < 0) {
		number *= -1;
		*remove = true;
    }

    return number;
}

///*
//* MSG_ReadDeltaEntity
//*
//* Can go from either a baseline or a previous packet_entity
//*/
//void MSG_ReadDeltaEntity(MessageBuffer* msg, const EntityState* from, EntityState* to, int32_t number, uint32_t byteMask) {
//    uint8_t fieldMask[32] = { };
//    const MessageBufferField* fields = ent_state_fields;
//    int32_t numFields = sizeof(ent_state_fields) / sizeof(ent_state_fields[0]);
//
//    // set everything to the state we are delta'ing from
//    *to = *from;
//    to->number = number;
//
//    MSG_ReadFieldMask(msg, fieldMask, sizeof(fieldMask), byteMask);
//
//    MSG_ReadStructFields(msg, from, to, fields, numFields, fieldMask, sizeof(fieldMask), byteMask);
//}