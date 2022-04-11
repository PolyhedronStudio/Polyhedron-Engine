/***
*
*	License here.
*
*	@file
*
*	Client/Server Protocol.
*
***/
#pragma once

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
static inline qboolean POLYHEDRON_PROTOCOL_SUPPORTED(uint32_t x) {
    return x >= PROTOCOL_VERSION_POLYHEDRON_MINIMUM && x <= PROTOCOL_VERSION_POLYHEDRON_CURRENT;
}

//==============================================

/**
*   @brief  Protocol Configuration.
**/
//! Number of copies of EntityState to keep buffered.
constexpr int32_t UPDATE_BACKUP = 64;  // Must be Power Of Two. 
constexpr int32_t UPDATE_MASK = (UPDATE_BACKUP - 1);

//! Allow a lot of command backups for very fast systems, used to be 64.
constexpr int32_t CMD_BACKUP = 128; 
constexpr int32_t CMD_MASK = (CMD_BACKUP - 1);

//! Number of bits reserved for Server Commands.
constexpr int32_t SVCMD_BITS = 5;
constexpr int32_t SVCMD_MASK = ((1 << SVCMD_BITS) - 1);

//! Number of bits reserved for Frame Number.
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

/**
*   @brief  Contains the definitions of the bit flags for the Server to Client commands.
**/
struct ServerCommand {
    //! A bad servercommand.
    static constexpr int32_t Bad = 0;

    // the rest are private to the client and server
    static constexpr int32_t Padding = 1;
    static constexpr int32_t Disconnect = 2;
    static constexpr int32_t Reconnect = 3;
    static constexpr int32_t Sound = 4;    // <see code>
    static constexpr int32_t Print = 5;	 // [byte] id [string] null terminated string
    static constexpr int32_t StuffText = 6;	   // [string] stuffed into client's console buffer
                                                // should be \n terminated
    static constexpr int32_t ServerData = 7;     // [long] protocol ...
    static constexpr int32_t ConfigString = 8;    // [short] [string]
	static constexpr int32_t SpawnBaseline = 9;
	static constexpr int32_t CenterPrint = 10;  // [string] to put in center of the screen
	static constexpr int32_t Download = 11;    // [short] size [size bytes]
	static constexpr int32_t PlayerInfo = 12;  // variable
	static constexpr int32_t PacketEntities = 13;   // [...]
	static constexpr int32_t DeltaPacketEntities = 14;  // [...]
	static constexpr int32_t Frame = 15;

    // r1q2 specific operations
    static constexpr int32_t ZPacket = 16;
    static constexpr int32_t ZDownload = 17;
	static constexpr int32_t GameState = 18;	 // q2pro specific, means svc_playerupdate in r1q2

    // This determines the maximum amount of types we can have.
	static constexpr int32_t Maximum = 255;
};

//==============================================

/**
*   @brief  Contains the definitions of the bit flags for the Client to Server commands.
**/
struct ClientCommand {
    static constexpr int32_t Bad = 0;
    static constexpr int32_t Nop = 1;
    static constexpr int32_t Move = 2;
    static constexpr int32_t UserInfo = 3;
    static constexpr int32_t StringCommand = 4;
    static constexpr int32_t DeltaUserInfo = 5;
};


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
//#define PS_WEAPONFRAME      (1 << 14)
#define PS_RDFLAGS          (1 << 14)

// New Animation thingy.
#define PS_GUNANIMATION_TIME_START  (1 << 15)
#define PS_GUNANIMATION_FRAME_START (1 << 16)
#define PS_GUNANIMATION_FRAME_END   (1 << 17)
#define PS_GUNANIMATION_FRAME_TIME  (1 << 18)
#define PS_GUNANIMATION_LOOP_COUNT  (1 << 29)
#define PS_GUNANIMATION_LOOP_FORCE  (1 << 20)

#define PS_BITS             21
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

/**
*   @brief  Contains the definitions of the bit flags for the Client User Commands.
* 
*           MS is sent each and every frame.
**/
struct UserCommandBits {
    static constexpr uint32_t AngleX        = (1 << 0);
    static constexpr uint32_t AngleY        = (1 << 1);
    static constexpr uint32_t AngleZ        = (1 << 2);
    static constexpr uint32_t Forward       = (1 << 3);
    static constexpr uint32_t Side          = (1 << 4);
    static constexpr uint32_t Up            = (1 << 5);
    static constexpr uint32_t Buttons       = (1 << 6);
    static constexpr uint32_t Impulse       = (1 << 7);
};

//==============================================

/**
*   @brief  Contains the definitions of the bit flags for the Entity State.
* 
*           These are sent over the wire by MSG_WriteUintBase128. Although it's
*           limit is technically thus uint64_t, it is currently set to uint32_t.
**/
struct SoundCommandBits {
    // A sound without an entity or position will be a local only sound.
    static constexpr uint32_t Volume        = (1 << 0); // Gets transfered over the wire as a byte.
    static constexpr uint32_t Attenuation   = (1 << 1); // Gets transfered over the wire as a byte.
    static constexpr uint32_t Position      = (1 << 2); // Transfered over the wire as a full vec3_t.
    static constexpr uint32_t Entity        = (1 << 3); // Transfered over the wire as a short 0-2: channel, 3-12: entity
    static constexpr uint32_t Offset        = (1 << 4); // Msec offset from frame start, transfered over the wire as a byte.
};

/**
*   @brief Default volume and attenuation.
**/
static constexpr float DEFAULT_SOUND_PACKET_VOLUME = 1.0f;
static constexpr float DEFAULT_SOUND_PACKET_ATTENUATION = 1.0f;

//==============================================

/**
*   @brief  Contains the definitions of the bit flags for the Entity State.
* 
*           These are sent over the wire by MSG_WriteUintBase128. Although it's
*           limit is technically thus uint64_t, it is currently set to uint32_t.
**/
struct EntityMessageBits {
    //! 1st byte.
    static constexpr uint32_t OriginX				= (1 << 0); //! X Origin.
    static constexpr uint32_t OriginY				= (1 << 1); //! Y Origin.
    static constexpr uint32_t AngleX				= (1 << 2); //! X Angle.
    static constexpr uint32_t AngleY				= (1 << 3); //! Y Angle.
    static constexpr uint32_t OriginZ				= (1 << 4); //! Origin Z.
    static constexpr uint32_t AngleZ				= (1 << 5); //! Angle Z.
    static constexpr uint32_t OldOrigin				= (1 << 6); //! Old origin.
    static constexpr uint32_t EventID				= (1 << 7); //! EventID for this state, reset each frame.

    //! 2nd byte.
    static constexpr uint32_t Sound                 = (1 << 8);  //! Sound
    static constexpr uint32_t AnimationFrame        = (1 << 9);  //! Animation Frame for anything BUT skeletal models.
    static constexpr uint32_t AnimationTimeStart    = (1 << 10); //! The animation starting frame.
    static constexpr uint32_t AnimationFrameStart   = (1 << 11); //! Animation Start Frame.
    static constexpr uint32_t AnimationFrameEnd     = (1 << 12); //! Animation End Frame.
    static constexpr uint32_t AnimationFrameTime    = (1 << 13); //! Animation Speed
    static constexpr uint32_t Skin                  = (1 << 14); //! Model Skin.
	static constexpr uint32_t Solid                 = (1 << 15); //! The Entity Solid.
    
    //! 3rd byte.
    static constexpr uint32_t ModelIndex			= (1 << 16); //! Model Index #1.
    static constexpr uint32_t ModelIndex2			= (1 << 17); //! Model Index #2.
    static constexpr uint32_t ModelIndex3			= (1 << 18); //! Model Index #3.
    static constexpr uint32_t ModelIndex4			= (1 << 19); //! Model Index #4.
    static constexpr uint32_t EntityEffects			= (1 << 20); //! Entity Effects.
    static constexpr uint32_t RenderEffects			= (1 << 21); //! Render Effects.    
    static constexpr uint32_t HashedClassname		= (1 << 22); //! 32 bit Hashed version of an entity's classname string.
    static constexpr uint32_t Bounds				= (1 << 23); //! X/y Size and Z -Mins/+Maxs of an entity.

    //! 4th byte. Waiting to be used... some day :-)
    static constexpr uint32_t Unused3				= (1 << 24); //! A free bit.
    static constexpr uint32_t Unused4				= (1 << 25); //! A free bit.
    static constexpr uint32_t Unused5				= (1 << 26); //! A free bit.
    static constexpr uint32_t Unused6				= (1 << 27); //! A free bit.
    static constexpr uint32_t Unused7				= (1 << 28); //! A free bit.
    static constexpr uint32_t Unused8				= (1 << 29); //! A free bit.
    static constexpr uint32_t Unused9				= (1 << 30); //! A free bit.
    static constexpr uint32_t Unused10				= (1 << 31); //! A free bit.
};


// ==============================================================

static constexpr int32_t CLIENTNUM_NONE     = MAX_CLIENTS - 1;
static constexpr int32_t CLIENTNUM_RESERVED = MAX_CLIENTS - 1;

// a Solid::BoundingBox will never create this value
static constexpr uint32_t PACKED_BBOX   = 31;


// q2pro frame flags sent by the server
// only SUPPRESSCOUNT_BITS can be used
struct FrameFlags {
    // Server supressed packets to client because rate limit was exceeded.
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