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

#ifndef NET_H
#define NET_H

#include "common/fifo.h"

// net.h -- quake's interface to the networking layer

#define PORT_ANY            -1
#define PORT_MASTER         27900
#define PORT_SERVER         27910

#define MIN_PACKETLEN                   512     // don't allow smaller packets
#define MAX_PACKETLEN                   4096    // max length of a single packet
#define PACKET_HEADER                   10      // two ints and a short (worst case)
#define MAX_PACKETLEN_DEFAULT           1400    // default quake2 limit
#define MAX_PACKETLEN_WRITABLE          (MAX_PACKETLEN - PACKET_HEADER)
#define MAX_PACKETLEN_WRITABLE_DEFAULT  (MAX_PACKETLEN_DEFAULT - PACKET_HEADER)

// portable network error codes
#define NET_OK       0  // success
#define NET_ERROR   -1  // failure (NET_ErrorString returns error message)
#define NET_AGAIN   -2  // operation would block, try again
#define NET_CLOSED  -3  // peer has closed connection

typedef int neterr_t;

#ifdef _WIN32
typedef intptr_t qsocket_t;
#else
typedef int qsocket_t;
#endif

struct ioentry_t {
#ifdef _WIN32
    qsocket_t fd;
#endif
    qboolean inUse: 1;
    qboolean canread: 1;
    qboolean canwrite: 1;
    qboolean canexcept: 1;
    qboolean wantread: 1;
    qboolean wantwrite: 1;
    qboolean wantexcept: 1;
};

enum NetAddressType {
    NA_UNSPECIFIED,
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IP6
};

enum NetSource {
    NS_CLIENT,
    NS_SERVER,
    NS_COUNT
};

enum NetFlag {
    NET_NONE    = 0,
    NET_CLIENT  = (1 << 0),
    NET_SERVER  = (1 << 1)
};

// Also include in shared.h but not every file that includes net.h has that included, so we do it here, again.
#include <array>
union NetAdrIP {
    std::array<uint8_t, 16> u8;
    std::array<uint16_t, 8> u16;
    std::array<uint32_t, 4> u32;
    std::array<uint64_t, 2> u64;
};

struct NetAdr {
    NetAddressType type;
    NetAdrIP ip;
    uint16_t port;
    uint32_t scope_id;  // IPv6 crap
};

struct NetState {
    static constexpr int32_t Disconnected = 0;  // No socket opened
    static constexpr int32_t Connecting = 1;    // Connect() not yet completed
    static constexpr int32_t Connected = 2;     // May transmit data
    static constexpr int32_t Closed = 3;        // Peer has preformed orderly shutdown
    static constexpr int32_t Broken = 4;        // Fatal error has been signaled
};

static inline qboolean NET_IsEqualAdr(const NetAdr *a, const NetAdr *b)
{
    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case NA_LOOPBACK:
        return true;
    case NA_IP:
    case NA_BROADCAST:
        if (a->ip.u32[0] == b->ip.u32[0] && a->port == b->port) {
            return true;
        }
        return false;
    case NA_IP6:
        if (memcmp(a->ip.u8.data(), b->ip.u8.data(), 16) == 0 && a->port == b->port) {
            return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

static inline qboolean NET_IsEqualBaseAdr(const NetAdr *a, const NetAdr *b)
{
    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case NA_LOOPBACK:
        return true;
    case NA_IP:
    case NA_BROADCAST:
        if (a->ip.u32[0] == b->ip.u32[0]) {
            return true;
        }
        return false;
    case NA_IP6:
        if (a->ip.u8 == b->ip.u8) {
            return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

static inline qboolean NET_IsEqualBaseAdrMask(const NetAdr *a,
                                              const NetAdr *b,
                                              const NetAdr *m)
{
    if (a->type != b->type) {
        return false;
    }

    switch (a->type) {
    case NA_IP:
        return !((a->ip.u32[0] ^ b->ip.u32[0]) & m->ip.u32[0]);
    case NA_IP6:
#if (defined __amd64__) || (defined _M_AMD64)
        return !(((a->ip.u64[0] ^ b->ip.u64[0]) & m->ip.u64[0]) |
                 ((a->ip.u64[1] ^ b->ip.u64[1]) & m->ip.u64[1]));
#else
        return !(((a->ip.u32[0] ^ b->ip.u32[0]) & m->ip.u32[0]) |
                 ((a->ip.u32[1] ^ b->ip.u32[1]) & m->ip.u32[1]) |
                 ((a->ip.u32[2] ^ b->ip.u32[2]) & m->ip.u32[2]) |
                 ((a->ip.u32[3] ^ b->ip.u32[3]) & m->ip.u32[3]));
#endif
    default:
        break;
    }

    return false;
}

static inline qboolean NET_IsLanAddress(const NetAdr *adr)
{
    switch (adr->type) {
    case NA_LOOPBACK:
        return true;
    case NA_IP:
    case NA_BROADCAST:
        if (adr->ip.u8[0] == 127 || adr->ip.u8[0] == 10) {
            return true;
        }
        if (adr->ip.u16[0] == MakeRawShort(192, 168) ||
            adr->ip.u16[0] == MakeRawShort(172,  16)) {
            return true;
        }
        return false;
    case NA_IP6:
        if (adr->ip.u8[0] == 0xfe && (adr->ip.u8[1] & 0xc0) == 0x80) {
            return true;
        }
        return false;
    default:
        break;
    }

    return false;
}

static inline qboolean NET_IsLocalAddress(const NetAdr *adr)
{
#if USE_CLIENT && USE_SERVER
    if (adr->type == NA_LOOPBACK)
        return true;
#endif
    return false;
}

void        NET_Init(void);
void        NET_Shutdown(void);
void        NET_Config(NetFlag flag);
void        NET_UpdateStats(void);

qboolean    NET_GetAddress(NetSource sock, NetAdr *adr);
void        NET_GetPackets(NetSource sock, void (*packet_cb)(void));
qboolean    NET_SendPacket(NetSource sock, const void *data,
                           size_t len, const NetAdr *to);

const char *NET_AdrToString(const NetAdr *a);
qboolean    NET_StringToAdr(const char *s, NetAdr *a, int default_port);
qboolean    NET_StringPairToAdr(const char *host, const char *port, NetAdr *a);

char        *NET_BaseAdrToString(const NetAdr *a);
#define     NET_StringToBaseAdr(s, a)   NET_StringPairToAdr(s, NULL, a)

const char  *NET_ErrorString(void);

neterr_t    NET_Listen(qboolean listen);

ioentry_t   *NET_AddFd(qsocket_t fd);
void        NET_RemoveFd(qsocket_t fd);
int         NET_Sleep(int msec);

extern cvar_t       *net_ip;
extern cvar_t       *net_port;

extern NetAdr     net_from;

#endif // NET_H
