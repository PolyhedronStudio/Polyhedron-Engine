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

#ifndef SIZEBUF_H
#define SIZEBUF_H

#define SZ_MSG_WRITE        MakeRawLong('w', 'r', 'i', 't')
#define SZ_MSG_READ         MakeRawLong('r', 'e', 'a', 'd')
#define SZ_NC_SEND_OLD      MakeRawLong('n', 'c', '1', 's')
#define SZ_NC_SEND_NEW      MakeRawLong('n', 'c', '2', 's')
#define SZ_NC_SEND_FRG      MakeRawLong('n', 'c', '2', 'f')
#define SZ_NC_FRG_IN        MakeRawLong('n', 'c', '2', 'i')
#define SZ_NC_FRG_OUT       MakeRawLong('n', 'c', '2', 'o')

struct SizeBuffer {
    uint32_t    tag;
    qboolean    allowOverflow;
    qboolean    allowUnderflow;
    qboolean    overflowed;     // set to true if the buffer size failed
    byte        *data;
    size_t      maximumSize;
    size_t      currentSize;
    size_t      readCount;
    size_t      bitPosition;
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

#endif // SIZEBUF_H
