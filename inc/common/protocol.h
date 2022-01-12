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

#ifndef PROTOCOL_H
#define PROTOCOL_H

//
// Protocol Configuration.
//
// The maximum length of a message on the network: 32k.
constexpr uint32_t MAX_MSGLEN = 0x8000;

// Used to refer when during a connection challenge the protocols differ.
constexpr int32_t   PROTOCOL_VERSION_UNKNOWN = -1;

// The DEFAULT version is the minimum allowed for connecting.
constexpr uint32_t  PROTOCOL_VERSION_DEFAULT = 1340;
constexpr uint32_t  PROTOCOL_VERSION_POLYHEDRON     = 1340;

// Minimum required "MINOR" protocol version for this client to be compatible to.
constexpr uint32_t PROTOCOL_VERSION_POLYHEDRON_MINIMUM = 1337;

// The "FIRST" protocol version we ever had for Polyhedron.
constexpr uint32_t PROTOCOL_VERSION_POLYHEDRON_FIRST   = 1337;

// EXAMPLE of what an update would then resemble in our code. Ofc, We then also change
// the PROTOCOL_VERSION_POLYHEDRON_CURRENT to accomodate.
constexpr uint32_t PROTOCOL_VERSION_POLYHEDRON_FEATURE_UPDATE = 1341;

// Current actual protocol version that is in use.
constexpr uint32_t PROTOCOL_VERSION_POLYHEDRON_CURRENT = 1340;

// This is used to ensure that the protocols in use match up, and support each other.
qboolean static inline NAC_PROTOCOL_SUPPORTED(uint32_t x) {
    return x >= PROTOCOL_VERSION_POLYHEDRON_MINIMUM && x <= PROTOCOL_VERSION_POLYHEDRON_CURRENT;
}

//==============================================

//
// Protocol Configuration.
//
// Number of copies of EntityState to keep buffered.
constexpr int32_t UPDATE_BACKUP = 256;  // Must be Power Of Two. 
constexpr int32_t UPDATE_MASK = (UPDATE_BACKUP - 1);

// Allow a lot of command backups for very fast systems, used to be 64.
constexpr int32_t CMD_BACKUP = 512; 
constexpr int32_t CMD_MASK = (CMD_BACKUP - 1);


constexpr int32_t SVCMD_BITS = 5;
constexpr int32_t SVCMD_MASK = ((1 << SVCMD_BITS) - 1);

constexpr int32_t FRAMENUM_BITS = 27;
constexpr int32_t FRAMENUM_MASK = ((1 << FRAMENUM_BITS) - 1);

constexpr int32_t SUPPRESSCOUNT_BITS = 4;
constexpr int32_t SUPPRESSCOUNT_MASK = ((1 << SUPPRESSCOUNT_BITS) - 1);

constexpr int32_t MAX_PACKET_ENTITIES = 2048;
constexpr int32_t MAX_PARSE_ENTITIES = (MAX_PACKET_ENTITIES * UPDATE_BACKUP);
constexpr int32_t PARSE_ENTITIES_MASK = (MAX_PARSE_ENTITIES - 1);

constexpr int32_t MAX_PACKET_USERCMDS = 32;
constexpr int32_t MAX_PACKET_FRAMES = 4;

constexpr int32_t MAX_PACKET_STRINGCMDS = 8;
constexpr int32_t MAX_PACKET_USERINFOS = 8;

constexpr int32_t CS_BITMAP_BYTES = (ConfigStrings::MaxConfigStrings/ 8); // 260
constexpr int32_t CS_BITMAP_LONGS = (CS_BITMAP_BYTES / 4);

//==============================================

//
// Server to Client commands.
//
typedef enum {
    svc_bad,

    // these ops are known to the game dll
    //SVG_CMD_MUZZLEFLASH,
    //SVG_CMD_MUZZLEFLASH2,
    //SVG_CMD_TEMP_ENTITY,
    //SVG_CMD_LAYOUT,
    //SVG_CMD_INVENTORY,

    // the rest are private to the client and server
    svc_nop,
    svc_disconnect,
    svc_reconnect,
    svc_sound,                  // <see code>
    svc_print,                  // [byte] id [string] null terminated string
    svc_stufftext,              // [string] stuffed into client's console buffer
                                // should be \n terminated
    svc_serverdata,             // [long] protocol ...
    svc_configstring,           // [short] [string]
    svc_spawnbaseline,
    svc_centerprint,            // [string] to put in center of the screen
    svc_download,               // [short] size [size bytes]
    svc_playerinfo,             // variable
    svc_packetentities,         // [...]
    svc_deltapacketentities,    // [...]
    svc_frame,

    // r1q2 specific operations
    svc_zpacket,
    svc_zdownload,
    svc_gamestate, // q2pro specific, means svc_playerupdate in r1q2
    svc_setting,

    // This determines the maximum amount of types we can have.
    svc_num_types = 255
} svc_ops_t;

//==============================================

//
// Client to Server commands.
//
typedef enum {
    clc_bad,
    clc_nop,
    clc_move,               // [ClientMoveCommand]
    clc_userinfo,           // [userinfo string]
    clc_stringcmd,          // [string] message

    // q2pro specific operations
    clc_userinfo_delta
} clc_ops_t;

//==============================================

// PlayerState communication
#define PS_PM_TYPE				(1 << 0)
#define PS_PM_ORIGIN			(1 << 1)
#define PS_PM_VELOCITY			(1 << 2)
#define PS_PM_FLAGS				(1 << 3)
#define PS_PM_TIME				(1 << 4)
#define PS_PM_GRAVITY			(1 << 5)
#define PS_PM_VIEW_OFFSET		(1 << 6)
#define PS_PM_VIEW_ANGLES		(1 << 7)
#define PS_PM_DELTA_ANGLES		(1 << 8)
#define PS_PM_STEP_OFFSET		(1 << 9)

#define PS_KICKANGLES       (1 << 10)
#define PS_BLEND            (1 << 11)
#define PS_FOV              (1 << 12)
#define PS_WEAPONINDEX      (1 << 13)
#define PS_WEAPONFRAME      (1 << 14)
#define PS_RDFLAGS          (1 << 15)

#define PS_BITS             16
#define PS_MASK             ((1<<PS_BITS)-1)

// r1q2 protocol specific extra flags
#define EPS_GUNOFFSET       (1<<0)
#define EPS_GUNANGLES       (1<<1)
#define EPS_M_VELOCITY2     (1<<2)
#define EPS_M_ORIGIN2       (1<<3)
#define EPS_VIEWANGLE2      (1<<4)
#define EPS_STATS           (1<<5)

// q2pro protocol specific extra flags
#define EPS_CLIENTNUM       (1<<6)

#define EPS_BITS            7
#define EPS_MASK            ((1<<EPS_BITS)-1)

//==============================================

// user_cmd_t communication

// ms and light always sent, the others are optional
#define CM_ANGLE1   (1<<0)
#define CM_ANGLE2   (1<<1)
#define CM_ANGLE3   (1<<2)
#define CM_FORWARD  (1<<3)
#define CM_SIDE     (1<<4)
#define CM_UP       (1<<5)
#define CM_BUTTONS  (1<<6)
#define CM_IMPULSE  (1<<7)

// r1q2 button byte hacks
#define BUTTON_MASK     (BUTTON_ATTACK|BUTTON_USE|BUTTON_ANY)
#define BUTTON_FORWARD  4
#define BUTTON_SIDE     8
#define BUTTON_UP       16
#define BUTTON_ANGLE1   32
#define BUTTON_ANGLE2   64

//==============================================

// a sound without an ent or pos will be a local only sound
#define SND_VOLUME          (1<<0)  // a byte
#define SND_ATTENUATION     (1<<1)  // a byte
#define SND_POS             (1<<2)  // three coordinates
#define SND_ENT             (1<<3)  // a short 0-2: channel, 3-12: entity
#define SND_OFFSET          (1<<4)  // a byte, msec offset from frame start

#define DEFAULT_SOUND_PACKET_VOLUME    1.0
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0

//==============================================

// EntityState communication

// Try to pack the common update flags into the first byte
#define U_ORIGIN_X  (1<<0)         // was named: U_ORIGIN_X
#define U_ORIGIN_Y  (1<<1)         // was named: U_ORIGIN_Y
#define U_ANGLE_Y   (1<<2)         // was named: U_ANGLE_Y
#define U_ANGLE_Z   (1<<3)         // was named: U_ANGLE_Z
#define U_FRAME     (1<<4)          // frame is a byte
#define U_EVENT     (1<<5)
#define U_REMOVE    (1<<6)          // REMOVE this entity, don't add it
#define U_MOREBITS1 (1<<7)          // read one additional byte

// Second byte
#define U_NUMBER16  (1<<8)          // NUMBER8 is implicit if not set
#define U_ORIGIN_Z  (1<<9)         // was named: U_ORIGIN_Z
#define U_ANGLE_X   (1<<10)        // was named: U_ANGLE_X
#define U_MODEL     (1<<11)
#define U_RENDERFX8 (1<<12)        // fullbright, etc
//#define U_ANGLE16   (1<<13)
#define U_EFFECTS8  (1<<14)        // autorotate, trails, etc
#define U_MOREBITS2 (1<<15)        // read one additional byte

// Third byte
#define U_SKIN8         (1<<16)
#define U_FRAME16       (1<<17)     // frame is a short
#define U_RENDERFX16    (1<<18)     // 8 + 16 = 32
#define U_EFFECTS16     (1<<19)     // 8 + 16 = 32
#define U_MODEL2        (1<<20)     // weapons, flags, etc
#define U_MODEL3        (1<<21)
#define U_MODEL4        (1<<22)
#define U_MOREBITS3     (1<<23)     // read one additional byte

// fourth byte
#define U_OLDORIGIN     (1<<24)     // FIXME: get rid of this
#define U_SKIN16        (1<<25)
#define U_SOUND         (1<<26)
#define U_SOLID         (1<<27)

// ==============================================================

#define CLIENTNUM_NONE        (MAX_CLIENTS - 1)
#define CLIENTNUM_RESERVED    (MAX_CLIENTS - 1)

// a Solid::BoundingBox will never create this value
#define PACKED_BSP      31


// q2pro frame flags sent by the server
// only SUPPRESSCOUNT_BITS can be used
struct FrameFlags {
    // Server surpressed packets to client because rate limit was exceeded.
    static constexpr int32_t Suppressed = (1 << 0);
    // A few packets from client to server were dropped by the network.
    // Server recovered player's movement using backup commands.
    static constexpr int32_t ClientDrop = (1 << 1);
    // Many packets from client to server were dropped by the network.
    // Server ran out of backup commands and had to predict player's movement.
    static constexpr int32_t ClientPredict = (1 << 2);
    // Unused, reserved for future reasons perhaps.
    static constexpr int32_t Reserved = (1 << 3);

    // Packets from server to client were dropped by the network.
    static constexpr int32_t ServerDrop = (1 << 4);
    // Server sent an invalid delta compressed frame.
    static constexpr int32_t BadFrame = (1 << 5);
    // Server sent a delta compressed frame that is too old and
    // can't be recovered.
    static constexpr int32_t OldFrame = (1 << 6);
    // Server sent a delta compressed frame whose entities are too
    // old and can't be recovered.
    static constexpr int32_t OldEntity = (1 << 7);
    // Server sent an uncompressed frame. Typically occurs during
    // a heavy lag, when a lot of packets are dropped by the network.
    static constexpr int32_t NoDeltaFrame = (1 << 8);
};

#endif // PROTOCOL_H
