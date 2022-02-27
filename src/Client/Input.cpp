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
// cl.input.c  -- builds an intended movement command to send to the server

#include "Client.h"
#include "Client/GameModule.h"
#include "Shared/CLGame.h"
#include "SharedGame/SharedGame.h"
#include "system/Lirc.h"

static cvar_t    *cl_nodelta;
static cvar_t    *cl_maxpackets;
static cvar_t    *cl_packetdup;
#ifdef _DEBUG
static cvar_t    *cl_showpackets;
#endif
static cvar_t    *cl_instantpacket;
static cvar_t    *cl_batchcmds;

static cvar_t    *m_filter;
cvar_t    *m_accel;
cvar_t    *m_autosens;

cvar_t    *sensitivity;

cvar_t    *m_pitch;
cvar_t    *m_invert;
cvar_t    *m_yaw;
static cvar_t    *m_forward;
static cvar_t    *m_side;

/*
===============================================================================

INPUT SUBSYSTEM

===============================================================================
*/

typedef struct {
    qboolean    modified;
    inputAPI_t  api;
    int         old_dx;
    int         old_dy;
} in_state_t;

static in_state_t   input;

static cvar_t    *in_enable;
#if USE_DINPUT
static cvar_t    *in_direct;
#endif
static cvar_t    *in_grab;

const inputAPI_t* IN_GetAPI()
{
    return &input.api;
}

static qboolean IN_GetCurrentGrab(void)
{
    if (cls.active != ACT_ACTIVATED)
        return false;  // main window doesn't have focus

    if (r_config.flags & QVF_FULLSCREEN)
        return true;   // full screen

    if (cls.key_dest & (KEY_MENU | KEY_CONSOLE))
        return false;  // menu or console is up

    if (cls.connectionState != ClientConnectionState::Active && cls.connectionState != ClientConnectionState::Cinematic)
        return false;  // not connected

    if (in_grab->integer >= 2) {
        if (cls.demo.playback && !Key_IsDown(K_SHIFT))
            return false;  // playing a demo (and not using freelook)

        if (cl.frame.playerState.pmove.type == EnginePlayerMoveType::Freeze)
            return false;  // isSpectator mode
    }

    if (in_grab->integer >= 1) {
        // We don't want the mouse to fire a weapon etc in this mainmenu mode.
        if (CL_InBSPMenu()) {
            return false;
        } else {
            return true;   // regular playing mode
        }
    }

    return false;
}

/*
============
IN_Activate
============
*/
void IN_Activate(void)
{
    if (input.api.Grab) {
        input.api.Grab(IN_GetCurrentGrab());
    }
}

/*
============
IN_Restart_f
============
*/
static void IN_Restart_f(void)
{
    IN_Shutdown();
    IN_Init();
}

/*
============
IN_Frame
============
*/
void IN_Frame(void)
{
    if (input.modified) {
        IN_Restart_f();
        return;
    }

    if (input.api.GetEvents) {
        input.api.GetEvents();
    }
}

/*
================
IN_WarpMouse
================
*/
void IN_WarpMouse(int x, int y)
{
    if (input.api.Warp) {
        input.api.Warp(x, y);
    }
}

/*
================
IN_GetOldMouseDelta
================
*/
void IN_GetOldMouseDelta(int &x, int &y)
{
    if (input.api.Warp) {
        input.api.Warp(x, y);
    }
}


/*
============
IN_Shutdown
============
*/
void IN_Shutdown(void)
{
#if USE_DINPUT
    if (in_direct) {
        in_direct->changed = NULL;
    }
#endif
    if (in_grab) {
        in_grab->changed = NULL;
    }

    if (input.api.Shutdown) {
        input.api.Shutdown();
    }

    // Reset input.
    input = {};
}

static void in_changed_hard(cvar_t *self)
{
    input.modified = true;
}

static void in_changed_soft(cvar_t *self)
{
    IN_Activate();
}

/*
============
IN_Init
============
*/
void IN_Init(void)
{
    qboolean ret = false;

    in_enable = Cvar_Get("in_enable", "1", 0);
    in_enable->changed = in_changed_hard;
    if (!in_enable->integer) {
        Com_Printf("Mouse input disabled.\n");
        return;
    }

#if USE_DINPUT
    in_direct = Cvar_Get("in_direct", "1", 0);
    if (in_direct->integer) {
        DI_FillAPI(&input.api);
        ret = input.api.Init();
        if (!ret) {
            Cvar_Set("in_direct", "0");
        }
    }
#endif

    if (!ret) {
        VID_FillInputAPI(&input.api);
        ret = input.api.Init();
        if (!ret) {
            Cvar_Set("in_enable", "0");
            return;
        }
    }

#if USE_DINPUT
    in_direct->changed = in_changed_hard;
#endif

    in_grab = Cvar_Get("in_grab", "1", 0);
    in_grab->changed = in_changed_soft;

    IN_Activate();
}


/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition


Key_Event (int key, qboolean down, unsigned time);

+mlook src time

===============================================================================
*/


//==========================================================================

// MOUSE
qboolean CL_GetMouseMotion(int *deltaX, int *deltaY) {
    if (input.api.GetMotion)
        return input.api.GetMotion(deltaX, deltaY);
    else
        return false;
}

/*
=================
CL_UpdateCmd

Updates msec, angles and builds interpolated movement vector for local prediction.
Doesn't touch command forward/side/upMove, these are filled by CL_FinalizeCmd.
=================
*/
void CL_UpdateCmd(int msec)
{
    CL_GM_BuildFrameMoveCommand(msec);
}


static void m_autosens_changed(cvar_t* self)
{
    float fov;

    if (self->value > 90.0f && self->value <= 179.0f)
        fov = self->value;
    else
        fov = 90.0f;

    cl.autosens_x = 1.0f / fov;
    cl.autosens_y = 1.0f / CL_GM_CalcFOV(fov, 4, 3);
}

/*
============
CL_RegisterInput
============
*/
void CL_RegisterInput(void)
{
    Cmd_AddCommand("in_restart", IN_Restart_f);

    cl_nodelta = Cvar_Get("cl_nodelta", "0", 0);
    cl_maxpackets = Cvar_Get("cl_maxpackets",  std::to_string(BASE_FRAMERATE * 3).c_str(), 0);
    cl_packetdup = Cvar_Get("cl_packetdup", "1", 0);
#ifdef _DEBUG
    cl_showpackets = Cvar_Get("cl_showpackets", "0", 0);
#endif
    cl_instantpacket = Cvar_Get("cl_instantpacket", "1", 0);
    cl_batchcmds = Cvar_Get("cl_batchcmds", "1", 0);

    sensitivity = Cvar_Get("sensitivity", "3", CVAR_ARCHIVE);

    m_pitch = Cvar_Get("m_pitch", "0.15", CVAR_ARCHIVE);
    m_invert = Cvar_Get("m_invert", "0", CVAR_ARCHIVE);
    m_yaw = Cvar_Get("m_yaw", "0.15", 0);
    m_forward = Cvar_Get("m_forward", "1", 0);
    m_side = Cvar_Get("m_side", "1", 0);
    m_filter = Cvar_Get("m_filter", "0", 0);
    m_accel = Cvar_Get("m_accel", "0", 0);


    m_autosens = Cvar_Get("m_autosens", "0", 0);
    m_autosens->changed = m_autosens_changed;
    m_autosens_changed(m_autosens);
}

/*
=================
CL_FinalizeCmd

Builds the actual movement vector for sending to server. Assumes that msec
and angles are already set for this frame by CL_UpdateCmd.
=================
*/
void CL_FinalizeCmd(void)
{
    // command buffer ticks in sync with cl_maxfps
    if (cmd_buffer.waitCount > 0) {
        cmd_buffer.waitCount--;
    }
    if (cl_cmdbuf.waitCount > 0) {
        cl_cmdbuf.waitCount--;
    }

    CL_GM_FinalizeFrameMoveCommand();
}

static inline qboolean ready_to_send(void)
{
    unsigned msec;

    if (cl.sendPacketNow) {
        Com_DDPrintf("NET: if cl.sendPacketNow\n");
        return true;
    }
    if (cls.netChannel->message.currentSize || cls.netChannel->reliableAckPending) {
        Com_DDPrintf("NET: if cls.netChannel->message.currentSize[%i] || cls.netChannel->reliableAckPending[%i]\n", cls.netChannel->message.currentSize, cls.netChannel->reliableAckPending);
        return true;
    }
    if (!cl_maxpackets->integer) {
        Com_DDPrintf("NET: if !cl_maxpackets->integer - return true\n");
        return true;
    }

    // Used to be 10, but since we upgrade the base framerate.... 
    if (cl_maxpackets->integer < BASE_FRAMERATE) {
        Cvar_Set("cl_maxpackets", std::to_string(BASE_FRAMERATE).c_str());
        Com_DDPrintf("NET: cl_maxpackets->integer[%i] < BASE_FRAMERATE[%i]\n", cl_maxpackets->integer, BASE_FRAMERATE);
    }

    msec = 1000 / cl_maxpackets->integer;
    if (msec) {
        int oldmsec = msec;
        msec = 100 / (100 / msec);
        Com_DDPrintf("NET: if msec { oldmsec[%i], msec[%i] }\n", oldmsec, msec);
    }
    if (cls.realtime - cl.lastTransmitTime < msec) {
        Com_DDPrintf("cls.realtime[%i], cls.lastTransmitTime[%ui], msec[%i] return falses\n", cls.realtime, cl.lastTransmitTime, msec);
        return false;
    } else {
        Com_DDPrintf("cls.realtime[%i], cls.lastTransmitTime[%ui], msec[%i] return true;\n", cls.realtime, cl.lastTransmitTime, msec);
    }

    return true;
}

static inline qboolean ready_to_send_hacked(void)
{
    if (cl.currentClientCommandNumber - cl.lastTransmitCmdNumberReal > 2) {
        return true; // can't drop more than 2 clientUserCommands
    }

    return ready_to_send();
}

/*
=================
CL_SendUserCommand
=================
*/
static void CL_SendUserCommand(void)
{
    size_t currentSize q_unused, checksumIndex;
    ClientMoveCommand *cmd, *oldcmd;
    ClientUserCommandHistory*history;

    // archive this packet
    history = &cl.clientCommandHistory[cls.netChannel->outgoingSequence & CMD_MASK];
    history->commandNumber = cl.currentClientCommandNumber;
    history->timeSent = cls.realtime;    // for ping calculation
    history->timeReceived = 0;

    cl.lastTransmitCmdNumber = cl.currentClientCommandNumber;

    // see if we are ready to send this packet
    if (!ready_to_send_hacked()) {
        cls.netChannel->outgoingSequence++; // just drop the packet
        return;
    }

    cl.lastTransmitTime = cls.realtime;
    cl.lastTransmitCmdNumberReal = cl.currentClientCommandNumber;

    // begin a client move command
    MSG_WriteUint8(clc_move);//MSG_WriteByte(clc_move);

    // save the position for a checksum byte
    checksumIndex = 0;

    // let the server know what the last frame we
    // got was, so the next message can be delta compressed
    if (cl_nodelta->integer || !cl.frame.valid /*|| cls.demowaiting*/) {
        MSG_WriteInt32(-1);//MSG_WriteLong(-1);   // no compression
    } else {
        MSG_WriteInt32(cl.frame.number);//MSG_WriteLong(cl.frame.number);
    }

    // send this and the previous clientUserCommands in the message, so
    // if the last packet was dropped, it can be recovered
    cmd = &cl.clientUserCommands[(cl.currentClientCommandNumber - 2) & CMD_MASK];
    // Ugly hack, but otherwise the player can shoot if he opens the cnsole in mainmenu.
    if (CL_InBSPMenu())
        cmd->input.buttons = 0;
    MSG_WriteDeltaClientMoveCommand(NULL, cmd);
    oldcmd = cmd;

    cmd = &cl.clientUserCommands[(cl.currentClientCommandNumber - 1) & CMD_MASK];
    // Ugly hack, but otherwise the player can shoot if he opens the cnsole in mainmenu.
    if (CL_InBSPMenu())
        cmd->input.buttons = 0;
    MSG_WriteDeltaClientMoveCommand(oldcmd, cmd);
    oldcmd = cmd;

    cmd = &cl.clientUserCommands[cl.currentClientCommandNumber & CMD_MASK];
    // Ugly hack, but otherwise the player can shoot if he opens the cnsole in mainmenu.
    if (CL_InBSPMenu())
        cmd->input.buttons = 0;
    MSG_WriteDeltaClientMoveCommand(oldcmd, cmd);

    P_FRAMES++;

    //
    // deliver the message
    //
    currentSize = Netchan_Transmit(cls.netChannel, msg_write.currentSize, msg_write.data, 1);
#ifdef _DEBUG
    if (cl_showpackets->integer) {
        Com_Printf("%" PRIz " ", currentSize); // C++20: String concat fix.
    }
#endif

    SZ_Clear(&msg_write);
}

static void CL_SendKeepAlive(void)
{
    ClientUserCommandHistory *history;
    size_t currentSize q_unused;

    // archive this packet
    history = &cl.clientCommandHistory[cls.netChannel->outgoingSequence & CMD_MASK];
    history->commandNumber = cl.currentClientCommandNumber;
    history->timeSent = cls.realtime;    // for ping calculation
    history->timeReceived = 0;

    cl.lastTransmitTime = cls.realtime;
    cl.lastTransmitCmdNumber = cl.currentClientCommandNumber;
    cl.lastTransmitCmdNumberReal = cl.currentClientCommandNumber;

    currentSize = Netchan_Transmit(cls.netChannel, 0, NULL, 1);
#ifdef _DEBUG
    if (cl_showpackets->integer) {
        Com_Printf("%" PRIz " ", currentSize);
    }
#endif
}

static void CL_SendUserinfo(void)
{
    char userinfo[MAX_INFO_STRING];
    cvar_t *var;
    int i;

    if (!cls.userinfo_modified) {
        return;
    }

    if (cls.userinfo_modified == MAX_PACKET_USERINFOS) {
        size_t len = Cvar_BitInfo(userinfo, CVAR_USERINFO);
        Com_DDPrintf("%s: %u: full update\n", __func__, com_framenum);
        MSG_WriteUint8(clc_userinfo);//MSG_WriteByte(clc_userinfo);
        MSG_WriteData(userinfo, len + 1);
        MSG_FlushTo(&cls.netChannel->message);
    } else if (cls.serverProtocol == PROTOCOL_VERSION_POLYHEDRON) {
        Com_DDPrintf("%s: %u: %d updates\n", __func__, com_framenum,
                     cls.userinfo_modified);
        for (i = 0; i < cls.userinfo_modified; i++) {
            var = cls.userinfo_updates[i];
            MSG_WriteUint8(clc_userinfo_delta);//MSG_WriteByte(clc_userinfo_delta);
            MSG_WriteString(var->name);
            if (var->flags & CVAR_USERINFO) {
                MSG_WriteString(var->string);
            } else {
                // no longer in userinfo
                MSG_WriteString(NULL);
            }
        }
        MSG_FlushTo(&cls.netChannel->message);
    } else {
        Com_WPrintf("%s: update count is %d, should never happen.\n",
                    __func__, cls.userinfo_modified);
    }

    cls.userinfo_modified = 0;
}

void CL_SendCmd(void)
{
    if (cls.connectionState < ClientConnectionState::Connected) {
        return; // not talking to a server
    }

    // generate usercmds while playing a demo,
    // but do not send them
    if (!cls.netChannel) {
        return;
    }

    if (cls.connectionState != ClientConnectionState::Active || sv_paused->integer) {
        // send a userinfo update if needed
        CL_SendUserinfo();

        // just keepalive or update reliable
        if (Netchan_ShouldUpdate(cls.netChannel)) {
            CL_SendKeepAlive();
        }

        cl.sendPacketNow = false;
        return;
    }

    // are there any new usercmds to send after all?
    if (cl.lastTransmitCmdNumber == cl.currentClientCommandNumber) {
        return; // nothing to send
    }

    // send a userinfo update if needed
    CL_SendUserinfo();

    CL_SendUserCommand();

    cl.sendPacketNow = false;
}

