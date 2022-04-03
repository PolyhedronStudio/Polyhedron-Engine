/***
*
*	License here.
*
*	@file
*
*	SizeBuffers are used for collecting data to read and send data
*   over the wire.
*
***/
#include "../Shared/Shared.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"

void SZ_TagInit(SizeBuffer *buf, void *data, size_t size, uint32_t tag)
{
    memset(buf, 0, sizeof(*buf));
    buf->data = (byte*)data; // CPP: Cast
    buf->maximumSize = size;
    buf->tag = tag;
    //buf->isCompressed = false;
}

void SZ_Init(SizeBuffer *buf, void *data, size_t size)
{
    memset(buf, 0, sizeof(*buf));
    buf->data = (byte*)data; // CPP: Cast
    buf->maximumSize = size;
    buf->allowOverflow = true;
    buf->allowUnderflow = true;
}

void SZ_Clear(SizeBuffer *buf)
{
    buf->currentSize = 0;
    buf->readCount = 0;
    buf->bitPosition = 0;
    buf->overflowed = false;
}

void *SZ_GetSpace(SizeBuffer *buf, size_t len)
{
    void    *data;

    if (buf->currentSize > buf->maximumSize) {
        Com_Error(ErrorType::Fatal,
                  "%s: %#x: already overflowed",
                  __func__, buf->tag);
    }

    if (len > buf->maximumSize - buf->currentSize) {
        if (len > buf->maximumSize) {
            Com_Error(ErrorType::Fatal,
                      "%s: %#x: %" PRIz " is > full buffer size %" PRIz "", // CPP: Cast
                      __func__, buf->tag, len, buf->maximumSize);
        }

        if (!buf->allowOverflow) {
            Com_Error(ErrorType::Fatal,
                      "%s: %#x: overflow without allowOverflow set",
                      __func__, buf->tag);
        }

        //Com_DPrintf("%s: %#x: overflow\n", __func__, buf->tag);
        SZ_Clear(buf);
        buf->overflowed = true;
    }

    data = buf->data + buf->currentSize;
    buf->currentSize += len;
    buf->bitPosition = buf->currentSize << 3;
    return data;
}

void SZ_WriteByte(SizeBuffer *sb, int c)
{
    byte    *buf;

    buf = (byte*)SZ_GetSpace(sb, 1); // CPP: Cast
    buf[0] = c;
}

void SZ_WriteShort(SizeBuffer *sb, int c)
{
    byte    *buf;

    buf = (byte*)SZ_GetSpace(sb, 2); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = c >> 8;
}

void SZ_WriteLong(SizeBuffer *sb, int c)
{
    byte    *buf;

    buf = (byte*)SZ_GetSpace(sb, 4); // CPP: Cast
    buf[0] = c & 0xff;
    buf[1] = (c >> 8) & 0xff;
    buf[2] = (c >> 16) & 0xff;
    buf[3] = c >> 24;
}
