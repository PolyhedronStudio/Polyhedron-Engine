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

#include "Shared/Shared.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"

void SZ_TagInit(SizeBuffer *buf, void *data, size_t size, uint32_t tag)
{
    memset(buf, 0, sizeof(*buf));
    buf->data = (byte*)data; // CPP: Cast
    buf->maximumSize = size;
    buf->tag = tag;
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
        Com_Error(ERR_FATAL,
                  "%s: %#x: already overflowed",
                  __func__, buf->tag);
    }

    if (len > buf->maximumSize - buf->currentSize) {
        if (len > buf->maximumSize) {
            Com_Error(ERR_FATAL,
                      "%s: %#x: %" PRIz " is > full buffer size %" PRIz "", // CPP: Cast
                      __func__, buf->tag, len, buf->maximumSize);
        }

        if (!buf->allowOverflow) {
            Com_Error(ERR_FATAL,
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
