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
struct ServerCommand {
    //! A bad servercommand.
    static constexpr int32_t Bad = 0;

    // These ops are known to the game dll
    //ServerGameCommand::MuzzleFlash,
    //ServerGameCommand::MuzzleFlash2,
    //ServerGameCommand::TempEntity,
    //ServerGameCommand::Layout,
    //ServerGameCommand::Inventory,

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

//
// Client to Server commands.
//
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
#define BUTTON_MASK     (ButtonBits::Attack|ButtonBits::Use|ButtonBits::Any)
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
//#define EntityMessageBits::OriginX  (1<<0)          // was named: EntityMessageBits::OriginX
//#define EntityMessageBits::OriginY  (1<<1)          // was named: EntityMessageBits::OriginY
//#define EntityMessageBits::AngleY   (1<<2)          // was named: EntityMessageBits::AngleY
//#define EntityMessageBits::AngleZ   (1<<3)          // was named: EntityMessageBits::AngleZ
//#define EntityMessageBits::AnimationFrame     (1<<4)          // frame is a byte
//#define EntityMessageBits::EventID     (1<<5)
//#define EntityMessageBits::Remove    (1<<6)          // REMOVE this entity, don't add it
//#define EntityMessageBits::MoreBitsA (1<<7)          // read one additional byte
//
//// Second byte
//#define U_NUMBER16  (1<<8)          // NUMBER8 is implicit if not set
//#define EntityMessageBits::OriginZ  (1<<9)         // was named: EntityMessageBits::OriginZ
//#define EntityMessageBits::AngleX   (1<<10)        // was named: EntityMessageBits::AngleX
//#define EntityMessageBits::ModelIndex1     (1<<11)
//#define U_RENDERFX8 (1<<12)        // fullbright, etc
////#define U_ANGLE16   (1<<13)
//#define U_EFFECTS8  (1<<14)        // autorotate, trails, etc
//#define EntityMessageBits::MoreBitsB (1<<15)        // read one additional byte
//
//// Third byte
//#define U_SKIN8         (1<<16)
//#define U_FRAME16       (1<<17)     // frame is a short
//#define U_RENDERFX16    (1<<18)     // 8 + 16 = 32
//#define U_EFFECTS16     (1<<19)     // 8 + 16 = 32
//#define EntityMessageBits::ModelIndex2        (1<<20)     // weapons, flags, etc
//#define EntityMessageBits::ModelIndex3        (1<<21)
//#define EntityMessageBits::ModelIndex4        (1<<22)
//#define EntityMessageBits::MoreBitsC     (1<<23)     // read one additional byte
//
//// fourth byte
//#define EntityMessageBits::OldOrigin     (1<<24)     // FIXME: get rid of this
//#define U_SKIN16        (1<<25)
//#define EntityMessageBits::Sound         (1<<26)
//#define EntityMessageBits::Solid         (1<<27)
//#define U_ANIMATION_STARTTIME (1 << 28) 
//#define U_ANIMATION_INDEX (1 << 29)
//#define U_ANIMATION_FRAMERATE (1 << 30)

/**
*   @brief  Client Game Commands are a way for the client to tell the server what to do.
*           Currently it is not in utilized but can be used if needed.
*
*           Due to protocol limitations at the time of writing, the index starts at 13
*           and the limit is 32 extra custom types.
**/
//static constexpr uint32_t BIT_NUMBER = (1 << 0);
//static constexpr uint32_t BIT_REMOVE = (1 << 6);
//
//static constexpr uint32_t EntityMessageBits::OriginX = (1 << 2);
//static constexpr uint32_t EntityMessageBits::OriginY = (1 << 3);
//static constexpr uint32_t BIT_ORIGIN_Z = (1 << 4);
//
//static constexpr uint32_t BIT_ANGLE_X = (1 << 5);
//static constexpr uint32_t BIT_ANGLE_Y = (1 << 1);
//static constexpr uint32_t BIT_ANGLE_Z = (1 << 7);
//
//static constexpr uint32_t BIT_FRAME = (1 << 8);
//static constexpr uint32_t BIT_ANIMATION_INDEX = (1 << 9);
//static constexpr uint32_t BIT_ANIMATION_START_TIME = (1 << 10);
//static constexpr uint32_t BIT_ANIMATION_FRAMERATE = (1 << 11);
//
//static constexpr uint32_t BIT_MODEL = (1 << 12);
//static constexpr uint32_t BIT_MODEL2 = (1 << 13);
//static constexpr uint32_t BIT_MODEL3 = (1 << 14);
//static constexpr uint32_t BIT_MODEL4 = (1 << 15);
//
//static constexpr uint32_t BIT_EVENT_ID = (1 << 16);
//
//static constexpr uint32_t BIT_EFFECTS = (1 << 17);
//static constexpr uint32_t BIT_RENDER_EFFECTS = (1 << 18);
//
//static constexpr uint32_t BIT_SKIN = (1 << 19);
//static constexpr uint32_t BIT_OLD_ORIGIN = (1 << 20);
//
//static constexpr uint32_t BIT_SOUND = (1 << 21);
//static constexpr uint32_t BIT_SOLID = (1 << 22);
//#define EntityMessageBits::OriginX (1 << 0)  // was named: EntityMessageBits::OriginX
//#define EntityMessageBits::OriginY (1 << 1)  // was named: EntityMessageBits::OriginY
//#define EntityMessageBits::AngleY (1 << 2)   // was named: EntityMessageBits::AngleY
//#define EntityMessageBits::AngleZ (1 << 3)   // was named: EntityMessageBits::AngleZ
//#define EntityMessageBits::AnimationFrame (1 << 4)     // frame is a byte
//#define EntityMessageBits::EventID (1 << 5)
//#define EntityMessageBits::Remove (1 << 6)     // REMOVE this entity, don't add it
//#define EntityMessageBits::MoreBitsA (1 << 7)  // read one additional byte
//
//// Second byte
//#define U_NUMBER16 (1 << 8)  // NUMBER8 is implicit if not set
//#define EntityMessageBits::OriginZ (1 << 9)  // was named: EntityMessageBits::OriginZ
//#define EntityMessageBits::AngleX (1 << 10)  // was named: EntityMessageBits::AngleX
//#define EntityMessageBits::ModelIndex1 (1 << 11)
//#define U_RENDERFX8 (1 << 12)  // fullbright, etc
////#define U_ANGLE16   (1<<13)
//#define U_EFFECTS8 (1 << 14)   // autorotate, trails, etc
//#define EntityMessageBits::MoreBitsB (1 << 15)  // read one additional byte

struct EntityMessageBits {

    // TODO: Adjust net code to send data perhaps like the following lists instead, prioritized by that which changes less often:
    // Order of data being sent, per category.
    // Entity State         : Number, Remove
    // Entity Transform     : OriginX, OriginY, AngleX, AngleY, OriginZ, AngleZ, OldOrigin 
    // Entity Display       : AnimationFrame, Skin, ModelIndex #1, RenderEffects, EntityEffects, ModelIndex #2, ModelIndex #3, ModelIndex #4, 

    // First 8 bits.
    static constexpr int32_t OriginX        = (1 << 0); //! X Origin.
    static constexpr int32_t OriginY        = (1 << 1); //! Y Origin.
    static constexpr int32_t AngleX         = (1 << 2); //! X Angle.
    static constexpr int32_t AngleY         = (1 << 3); //! Y Angle.
    static constexpr int32_t AnimationFrame = (1 << 4); //! Animation frame, not used by skeletal models.
    static constexpr int32_t EventID        = (1 << 5); //! EventID for this state, reset each frame.
    //static constexpr int32_t Remove         = (1 << 6); //! Notifies the client to remove this entity.
    static constexpr int32_t MoreBitsA      = (1 << 7); //! Notifies the client that more bits are coming its way for this entity state.

    // Second 8 bits.
    //static constexpr int32_t Number         = (1 << 8);  //! The entity index number.
    static constexpr int32_t OriginZ        = (1 << 9);  //! Origin Z.
    static constexpr int32_t AngleZ         = (1 << 10); //! Angle Z.
    static constexpr int32_t ModelIndex     = (1 << 11); //! Model Index #1.
    static constexpr int32_t ModelIndex2    = (1 << 12); //! Model Index #2.
    static constexpr int32_t ModelIndex3    = (1 << 13); //! Model Index #3.
    static constexpr int32_t ModelIndex4    = (1 << 14); //! Model Index #4.
    static constexpr int32_t MoreBitsB      = (1 << 15); //! Notifies the client that more bits are coming its way for this entity state.

    // Third 8 bits.
    static constexpr int32_t RenderEffects          = (1 << 16); //! Render Effects.
    static constexpr int32_t EntityEffects          = (1 << 17); //! Entity Effects.
    static constexpr int32_t Skin                   = (1 << 17); //! Model Skin.
    static constexpr int32_t OldOrigin              = (1 << 19); //! Old origin.
    static constexpr int32_t Sound                  = (1 << 20); //! Sound
    static constexpr int32_t Solid                  = (1 << 21); //! The animation start time.
    static constexpr int32_t AnimationTimeStart     = (1 << 22); //! The animation starting frame.
    static constexpr int32_t MoreBitsC              = (1 << 23); //! Notifies the client that more bits are coming its way for this entity state.

    // Fourth 8 bits.
    static constexpr int32_t AnimationFrameStart    = (1 << 24); //! The animation ending frame.
    static constexpr int32_t AnimationFrameEnd      = (1 << 25); //! The frame time.
    static constexpr int32_t AnimationFrameTime     = (1 << 26); //! Reserved for future use.
    static constexpr int32_t Unused1                = (1 << 27); //! Reserved for future use.
    static constexpr int32_t Unused2                = (1 << 28); //! Reserved for future use.
    static constexpr int32_t Unused3                = (1 << 29); //! Reserved for future use.
    static constexpr int32_t Unused4                = (1 << 30); //! Reserved for future use.
    static constexpr int32_t Unused5                = (1 << 31); //! Reserved for future use.

};



//// Third byte
//#define U_SKIN8 (1 << 16)
//#define U_FRAME16 (1 << 17)	// frame is a short
//#define U_RENDERFX16 (1 << 18)	// 8 + 16 = 32
//#define U_EFFECTS16 (1 << 19)	// 8 + 16 = 32
//#define EntityMessageBits::ModelIndex2 (1 << 20)	// weapons, flags, etc
//#define EntityMessageBits::ModelIndex3 (1 << 21)
//#define EntityMessageBits::ModelIndex4 (1 << 22)
//#define EntityMessageBits::MoreBitsC (1 << 23)  // read one additional byte
//
//// fourth byte
//#define EntityMessageBits::OldOrigin (1 << 24)  // FIXME: get rid of this
//#define U_SKIN16 (1 << 25)
//#define EntityMessageBits::Sound (1 << 26)
//#define EntityMessageBits::Solid (1 << 27)
//#define EntityMessageBits::AnimationTimeStart (1 << 28)
//#define EntityMessageBits::AnimationFrameStart (1 << 29)
//#define EntityMessageBits::AnimationFrameEnd (1 << 30)
//#define EntityMessageBits::AnimationFrameTime (1 << 31)

// ==============================================================

#define CLIENTNUM_NONE        (MAX_CLIENTS - 1)
#define CLIENTNUM_RESERVED    (MAX_CLIENTS - 1)

// a Solid::BoundingBox will never create this value
#define PACKED_BBOX      31


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


//    // Set the "more bits" flags based on how deep this state update is.
//    if (bits & 0xff000000)
//        bits |= EntityMessageBits::MoreBitsC | EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
//    else if (bits & 0x00ff0000)
//        bits |= EntityMessageBits::MoreBitsB | EntityMessageBits::MoreBitsA;
//    else if (bits & 0x0000ff00)
//        bits |= EntityMessageBits::MoreBitsA;
//
//    // Write out the first 8 bits.
//    MSG_WriteUint8(bits & 255);
//
//    // Write out the next 24 bits if this is an update reaching to MoreBitsC
//    if (bits & 0xff000000) {
//        MSG_WriteUint8((bits >> 8) & 255);
//        MSG_WriteUint8((bits >> 16) & 255);
//        MSG_WriteUint8((bits >> 24) & 255);
//    }
//    // Write out the next 16 bits if this is an update reaching to MoreBitsB
//    else if (bits & 0x00ff0000) {
//        MSG_WriteUint8((bits >> 8) & 255);
//        MSG_WriteUint8((bits >> 16) & 255);
//    }
//    // Write out the next 8 bits if this is an update reaching to MoreBitsA
//    else if (bits & 0x0000ff00) {
//        MSG_WriteUint8((bits >> 8) & 255);
//    }
//
//    //----------
//    // Make sure the "REMOVE" bit is unset before writing out the Entity State Number.
//    int32_t entityNumber = to->number & ~(1U << 15);
//
//    // Write out the Entity State number.
//    MSG_WriteUint16(to->number);
//
//    // Write out the ModelIndex.
//    if (bits & EntityMessageBits::ModelIndex) { 
//        MSG_WriteUint8(to->modelIndex);
//    }
//    // Write out the ModelIndex2.
//    if (bits & EntityMessageBits::ModelIndex2) {
//	    MSG_WriteUint8(to->modelIndex2);
//    }
//    // Write out the ModelIndex3.
//    if (bits & EntityMessageBits::ModelIndex3) {
//	    MSG_WriteUint8(to->modelIndex3);
//    }
//    // Write out the ModelIndex4.
//    if (bits & EntityMessageBits::ModelIndex4) {
//	    MSG_WriteUint8(to->modelIndex4);
//    }
//
//    // Write out the AnimationFrame.
//    if (bits & EntityMessageBits::AnimationFrame) {
//	    MSG_WriteFloat(to->animationFrame);
//    }
//    
//    // Write out the Skin Number.
//    if (bits & EntityMessageBits::Skin) {
//	    MSG_WriteUint16(to->skinNumber);
//    }
//
//    // Write out the Entity Effects.
//    if (bits & EntityMessageBits::EntityEffects) {
//	    MSG_WriteLong(to->effects);
//    }
//
//    // Write out the Render Effects.
//    if (bits & EntityMessageBits::RenderEffects) {
//	    MSG_WriteLong(to->renderEffects);
//    }
//
//    // Write out the Origin X.
//    if (bits & EntityMessageBits::OriginX) {
//	    MSG_WriteFloat(to->origin[0]);
//    }
//    // Write out the Origin Y.
//    if (bits & EntityMessageBits::OriginY) {
//	    MSG_WriteFloat(to->origin[1]);
//    }
//    // Write out the Origin Z.
//    if (bits & EntityMessageBits::OriginZ) {
//	    MSG_WriteFloat(to->origin[2]);
//    }
//
//    // Write out the Angle X.
//    if (bits & EntityMessageBits::AngleX) {
//	    MSG_WriteFloat(to->angles[0]);
//    }
//    // Write out the Angle Y.
//    if (bits & EntityMessageBits::AngleY) {
//	    MSG_WriteFloat(to->angles[1]);
//    }
//    // Write out the Angle Z.
//    if (bits & EntityMessageBits::AngleZ) {
//	    MSG_WriteFloat(to->angles[2]);
//    }
//
//    // Write out the Old Origin.
//    if (bits & EntityMessageBits::OldOrigin) {
//        MSG_WriteFloat(to->oldOrigin[0]);
//        MSG_WriteFloat(to->oldOrigin[1]);
//        MSG_WriteFloat(to->oldOrigin[2]);
//    }
//
//    // Write out the Sound.
//    if (bits & EntityMessageBits::Sound) {
//	    MSG_WriteUint8(to->sound);
//    }
//
//    // Write out the Event ID.
//    if (bits & EntityMessageBits::EventID) {
//	    MSG_WriteUint8(to->eventID);
//    }
//
//    // Write out the Solid.
//    if (bits & EntityMessageBits::Solid) {
//        MSG_WriteLong(to->solid);
//    }
//
//    // Write out the Animation Start Time.
//    if (bits & EntityMessageBits::AnimationTimeStart) {
//	    MSG_WriteLong(to->animationStartTime);
//    }
//    // Write out the Animation Start Frame.
//    if (bits & EntityMessageBits::AnimationFrameStart) {
//	    MSG_WriteUint16(to->animationStartFrame);
//    }
//    // Write out the Animation End Frame.
//    if (bits & EntityMessageBits::AnimationFrameEnd) {
//	    MSG_WriteUint16(to->animationEndFrame);
//    }
//    // Write out the Animation Frame Time.
//    if (bits & EntityMessageBits::AnimationFrameTime) {
//    	MSG_WriteFloat(to->animationFramerate);
//    }
//}
