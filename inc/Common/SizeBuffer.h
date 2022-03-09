/***
*
*	License here.
*
*	@file
*
*	SizeBuffers are used for collecting data to read and send data
*   over the net.
*
***/
#pragma once

#define SZ_MSG_WRITE        MakeRawLong('w', 'r', 'i', 't')
#define SZ_MSG_READ         MakeRawLong('r', 'e', 'a', 'd')
#define SZ_NC_SEND_NEW      MakeRawLong('n', 'c', 's', 'n')
#define SZ_NC_SEND_FRG      MakeRawLong('n', 'c', 's', 'f')
#define SZ_NC_FRG_IN        MakeRawLong('n', 'c', 'f', 'i')
#define SZ_NC_FRG_OUT       MakeRawLong('n', 'c', 'f', 'o')

/**
*   A buffer object used for reading and sending data over the net.
**/
struct SizeBuffer {
    //! Tag for ZMalloc.
    uint32_t    tag = 0;
    //! True if overflows are allowed.
    qboolean    allowOverflow = false;
    //! True if underflows are allowed.
    qboolean    allowUnderflow = false;
    //! Set to true if it overflowed the buffer size.
    qboolean    overflowed = false;     // set to true if the buffer size failed
    //! Pointer to buffer data.
    byte*       data = nullptr;
    //! Maximum buffer size.
    size_t      maximumSize = 0;
    //! Current buffer size.
    size_t      currentSize = 0;
    //! Count of bytes read so far.
    size_t      readCount = 0;
    //! Current bit position.
    size_t      bitPosition = 0;
    // Is it compressed?
    qboolean isCompressed = false;
};

void SZ_Init(SizeBuffer *buf, void *data, size_t size);
void SZ_TagInit(SizeBuffer *buf, void *data, size_t size, uint32_t tag);
void SZ_Clear(SizeBuffer *buf);
void *SZ_GetSpace(SizeBuffer *buf, size_t len);
void SZ_WriteByte(SizeBuffer *sb, int c);
void SZ_WriteShort(SizeBuffer *sb, int c);
void SZ_WriteLong(SizeBuffer *sb, int c);

static inline void *SZ_Write(SizeBuffer *buf, const void *data, size_t len)
{
    return memcpy(SZ_GetSpace(buf, len), data, len);
}
