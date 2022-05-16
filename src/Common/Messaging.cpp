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
#include "../Shared/Shared.h"
#include "Common/HalfFloat.h"
#include "Common/Messaging.h"
#include "Common/Protocol.h"
#include "Common/SizeBuffer.h"
#include "Game/Shared/Protocol.h"

// Assertion.
#include <cassert>


/*
==============================================================================

            DEBUGGING STUFF

==============================================================================
*/

#ifdef _DEBUG

#define SHOWBITS(x) Com_LPrintf(PrintType::Developer, x " ")

#if USE_CLIENT
void MSG_ShowDeltaPlayerstateBits(int flags, int extraflags)
{
#define SP(b,s) if(flags&PS_##b) SHOWBITS(s)
#define SE(b,s) if(extraflags&EPS_##b) SHOWBITS(s)
    SP(PM_TYPE, "pmove.type");
    SP(PM_ORIGIN, "pmove.origin[x,y]");
    SE(M_ORIGIN2, "pmove.origin[z]");
    SP(PM_VELOCITY, "pmove.velocity[x,y]");
    SE(M_VELOCITY2, "pmove.velocity[z]");
    SP(PM_TIME, "pmove.time");
    SP(PM_FLAGS, "pmove.flags");
    SP(PM_GRAVITY, "pmove.gravity");
    SP(PM_DELTA_ANGLES, "pmove.deltaAngles");
    SP(PM_VIEW_OFFSET, "pmove.viewOffset");
    SP(PM_VIEW_ANGLES, "pmove.viewAngles");
    SP(KICKANGLES, "kickAngles");
    SP(WEAPONINDEX, "gunIndex");
    SP(GUNANIMATION_TIME_START, "gunAnimationStartTime");
    SP(GUNANIMATION_FRAME_START, "gunAnimationFrameStart");
    SP(GUNANIMATION_FRAME_END, "gunAnimationFrameEnd");
    SP(GUNANIMATION_FRAME_TIME, "gunAnimationFrameTime");
    SP(GUNANIMATION_LOOP_COUNT, "gunAnimationLoopCount");
    SP(GUNANIMATION_LOOP_FORCE, "gunAnimationForceLoop");
    SE(GUNOFFSET, "gunOffset");
    SE(GUNANGLES, "gunAngles");
    SP(BLEND, "blend");
    SP(FOV, "fov");
    SP(RDFLAGS, "rdflags");
    SE(STATS, "stats");
#undef SP
#undef SE
}

void MSG_ShowDeltaUsercmdBits(int bits)
{
    if (!bits) {
        SHOWBITS("<none>");
        return;
    }

#define S(b,s) if(bits&UserCommandBits::##b) SHOWBITS(s)
    S(AngleX, "angle.x");
    S(AngleY, "angle.y");
    S(AngleZ, "angle.z");
    S(Forward, "forward");
    S(Side, "side");
    S(Up, "up");
    S(Buttons, "buttons");
    S(Impulse, "msec");
#undef S
}

void MSG_ShowDeltaEntityBits(uint32_t byteMask)
{
#define S(b,s) if(byteMask&EntityMessageBits::##b) SHOWBITS(s)
    S(OriginX, "origin.x");
    S(OriginY, "origin.y");
    S(OriginZ, "origin.z");
    S(AngleX, "angles.x");
    S(AngleY, "angles.y");
    S(AngleZ, "angles.z");
    S(OldOrigin, "oldOrigin");
    S(EventID, "eventID");

    S(Sound, "sound");    
    S(Solid, "solid");
    S(AnimationFrame, "animationFrame");
    S(AnimationTimeStart, "animationTimeStart");
    S(AnimationIndex, "animationFrameStart");
    S(AnimationFrameEnd, "animationFrameEnd");
    S(AnimationFrameTime, "animationFrameTime");
    S(Skin, "skin");
    S(ModelIndex, "modelIndex");
    S(ModelIndex2, "modelIndex2");
    S(ModelIndex3, "modelIndex3");
    S(ModelIndex4, "modelIndex4");
    S(EntityEffects, "entityEffects");
    S(RenderEffects, "renderEffects");
#undef S
}

const char* MSG_ServerCommandString(int cmd)
{
    switch (cmd) {
    case -1: return "END OF MESSAGE";
    default: return "UNKNOWN COMMAND";
#define S(x) case ServerCommand::##x: return "ServerCommand::" #x;
            S(Bad)
            // TODO: Protocol todo: add a game callback for this...?
            //S(muzzleflash)
            //S(muzzleflash2)
            //S(temp_entity)
            //S(layout)
            //S(inventory)
            S(Padding)
            S(Disconnect)
            S(Reconnect)
            S(Sound)
            S(Print)
            S(StuffText)
            S(ServerData)
            S(ConfigString)
            S(SpawnBaseline)
            S(CenterPrint)
            S(Download)
            S(PlayerInfo)
            S(PacketEntities)
	        S(DeltaPacketEntities)
            S(Frame)
            S(ZPacket)
            S(ZDownload)
            S(GameState)
#undef S
    }
}
#endif // USE_CLIENT
#endif // _DEBUG

