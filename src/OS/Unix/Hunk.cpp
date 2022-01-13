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
#include "System/Hunk.h"
#include <sys/mman.h>
#include <errno.h>

void Hunk_Begin(memhunk_t *hunk, size_t maximumSize)
{
    void *buf;

    if (maximumSize > SIZE_MAX - 4095)
        Com_Error(ERR_FATAL, "%s: size > SIZE_MAX", __func__);

    // reserve a huge chunk of memory, but don't commit any yet
    hunk->currentSize = 0;
    hunk->maximumSize = (maximumSize + 4095) & ~4095;
    buf = mmap(NULL, hunk->maximumSize, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANON, -1, 0);
    if (buf == NULL || buf == (void *)-1)
        Com_Error(ERR_FATAL, "%s: unable to reserve %" PRIz " bytes: %s",
                  __func__, hunk->maximumSize, strerror(errno));
    hunk->base = buf;
    hunk->mapped = hunk->maximumSize;
}

void *Hunk_Alloc(memhunk_t *hunk, size_t size)
{
    void *buf;

    if (size > SIZE_MAX - 63)
        Com_Error(ERR_FATAL, "%s: size > SIZE_MAX", __func__);

    // round to cacheline
    size = (size + 63) & ~63;

    if (hunk->currentSize > hunk->maximumSize)
        Com_Error(ERR_FATAL, "%s: currentSize > maximumSize", __func__);

    if (size > hunk->maximumSize - hunk->currentSize)
        Com_Error(ERR_FATAL, "%s: couldn't allocate %" PRIz " bytes", __func__, size);

    buf = (byte *)hunk->base + hunk->currentSize;
    hunk->currentSize += size;
    return buf;
}

void Hunk_End(memhunk_t *hunk)
{
    size_t newsize;

    if (hunk->currentSize > hunk->maximumSize)
        Com_Error(ERR_FATAL, "%s: currentSize > maximumSize", __func__);

    newsize = (hunk->currentSize + 4095) & ~4095;

    if (newsize < hunk->maximumSize) {
#if (defined __linux__) && (defined _GNU_SOURCE)
        void *buf = mremap(hunk->base, hunk->maximumSize, newsize, 0);
#else
        void *unmap_base = (byte *)hunk->base + newsize;
        size_t unmap_len = hunk->maximumSize - newsize;
        void *buf = munmap(unmap_base, unmap_len) + (byte *)hunk->base;
#endif
        if (buf != hunk->base)
            Com_Error(ERR_FATAL, "%s: could not remap virtual block: %s",
                      __func__, strerror(errno));
    }

    hunk->mapped = newsize;
}

void Hunk_Free(memhunk_t *hunk)
{
    if (hunk->base && munmap(hunk->base, hunk->mapped))
        Com_Error(ERR_FATAL, "%s: munmap failed: %s",
                  __func__, strerror(errno));

    memset(hunk, 0, sizeof(*hunk));
}

