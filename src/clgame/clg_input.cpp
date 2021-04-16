
// LICENSE HERE.

//
// clg_input.c
//
//
// Handles the game specific related input areas.
//
#include "clg_local.h"
#include "clg_input.h"

//
//===============
// CLG_RegisterInput
// 
// Registered input messages and binds them to a callback function.
// Bindings are set in the config files, or the options menu.
// 
// For more information, it still works like in q2pro.
//===============
//
void CLG_RegisterInput(void)
{

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


static KeyBinding    in_klook;
static KeyBinding in_left, in_right, in_forward, in_back;
static KeyBinding in_lookup, in_lookdown, in_moveleft, in_moveright;
static KeyBinding in_strafe, in_speed, in_use, in_attack;
static KeyBinding in_up, in_down;

static int          in_impulse;
static qboolean     in_mlooking;

void KeyDown(KeyBinding* b)
{
    int k;
    const char* c; // C++20: STRING: Added const to char*

    c = Cmd_Argv(1);
    if (c[0])
        k = atoi(c);
    else
        k = -1;        // typed manually at the console for continuous down

    if (k == b->keys[0] || k == b->keys[1])
        return;        // repeating key

    if (!b->keys[0])
        b->keys[0] = k;
    else if (!b->keys[1])
        b->keys[1] = k;
    else {
        Com_WPrintf("Three keys down for a button!\n");
        return;
    }

    if (b->state & BUTTON_STATE_HELD)
        return;        // still down

    // save timestamp
    c = Cmd_Argv(2);
    b->downtime = atoi(c);
    if (!b->downtime) {
        b->downtime = com_eventTime - 100;
    }

    b->state |= BUTTON_STATE_HELD + BUTTON_STATE_DOWN;    // down + impulse down
}

void KeyUp(KeyBinding* b)
{
    int k;
    const char* c; // C++20: STRING: Added const to char*
    unsigned uptime;

    c = Cmd_Argv(1);
    if (c[0])
        k = atoi(c);
    else {
        // typed manually at the console, assume for unsticking, so clear all
        b->keys[0] = b->keys[1] = 0;
        b->state = 0;    // impulse up
        return;
    }

    if (b->keys[0] == k)
        b->keys[0] = 0;
    else if (b->keys[1] == k)
        b->keys[1] = 0;
    else
        return;        // key up without coresponding down (menu pass through)
    if (b->keys[0] || b->keys[1])
        return;        // some other key is still holding it down

    if (!(b->state & BUTTON_STATE_HELD))
        return;        // still up (this should not happen)

    // save timestamp
    c = Cmd_Argv(2);
    uptime = atoi(c);
    if (!uptime) {
        b->msec += 10;
    }
    else if (uptime > b->downtime) {
        b->msec += uptime - b->downtime;
    }

    b->state &= ~BUTTON_STATE_HELD;        // now up
}

static void KeyClear(KeyBinding* b)
{
    b->msec = 0;
    b->state &= ~BUTTON_STATE_DOWN;        // clear impulses
    if (b->state & BUTTON_STATE_HELD) {
        b->downtime = com_eventTime; // still down
    }
}

static void IN_KLookDown(void) { KeyDown(&in_klook); }
static void IN_KLookUp(void) { KeyUp(&in_klook); }
static void IN_UpDown(void) { KeyDown(&in_up); }
static void IN_UpUp(void) { KeyUp(&in_up); }
static void IN_DownDown(void) { KeyDown(&in_down); }
static void IN_DownUp(void) { KeyUp(&in_down); }
static void IN_LeftDown(void) { KeyDown(&in_left); }
static void IN_LeftUp(void) { KeyUp(&in_left); }
static void IN_RightDown(void) { KeyDown(&in_right); }
static void IN_RightUp(void) { KeyUp(&in_right); }
static void IN_ForwardDown(void) { KeyDown(&in_forward); }
static void IN_ForwardUp(void) { KeyUp(&in_forward); }
static void IN_BackDown(void) { KeyDown(&in_back); }
static void IN_BackUp(void) { KeyUp(&in_back); }
static void IN_LookupDown(void) { KeyDown(&in_lookup); }
static void IN_LookupUp(void) { KeyUp(&in_lookup); }
static void IN_LookdownDown(void) { KeyDown(&in_lookdown); }
static void IN_LookdownUp(void) { KeyUp(&in_lookdown); }
static void IN_MoveleftDown(void) { KeyDown(&in_moveleft); }
static void IN_MoveleftUp(void) { KeyUp(&in_moveleft); }
static void IN_MoverightDown(void) { KeyDown(&in_moveright); }
static void IN_MoverightUp(void) { KeyUp(&in_moveright); }
static void IN_SpeedDown(void) { KeyDown(&in_speed); }
static void IN_SpeedUp(void) { KeyUp(&in_speed); }
static void IN_StrafeDown(void) { KeyDown(&in_strafe); }
static void IN_StrafeUp(void) { KeyUp(&in_strafe); }

static void IN_AttackDown(void)
{
    KeyDown(&in_attack);

    if (cl_instantpacket->integer && cls.state == ca_active && cls.netchan) {
        cl.sendPacketNow = true;
    }
}

static void IN_AttackUp(void)
{
    KeyUp(&in_attack);
}

static void IN_UseDown(void)
{
    KeyDown(&in_use);

    if (cl_instantpacket->integer && cls.state == ca_active && cls.netchan) {
        cl.sendPacketNow = true;
    }
}

static void IN_UseUp(void)
{
    KeyUp(&in_use);
}

static void IN_Impulse(void)
{
    in_impulse = atoi(Cmd_Argv(1));
}

static void IN_CenterView(void)
{
    cl.viewAngles.x = -SHORT2ANGLE(cl.frame.playerState.pmove.delta_angles[0]);
}

static void IN_MLookDown(void)
{
    in_mlooking = true;
}

static void IN_MLookUp(void)
{
    in_mlooking = false;

    if (!freelook->integer && lookspring->integer)
        IN_CenterView();
}

/*
===============
CL_KeyState

Returns the fraction of the frame that the key was down
===============
*/
static float CL_KeyState(KeyBinding* key)
{
    unsigned msec = key->msec;
    float val;

    if (key->state & BUTTON_STATE_HELD) {
        // still down
        if (com_eventTime > key->downtime) {
            msec += com_eventTime - key->downtime;
        }
    }

    // special case for instant packet
    if (!cl.cmd.msec) {
        return (float)(key->state & BUTTON_STATE_HELD);
    }

    val = (float)msec / cl.cmd.msec;

    return Clampf(val, 0, 1);
}



//==========================================================================

float autosens_x;
float autosens_y;

/*
================
CL_MouseMove
================
*/
static void CL_MouseMove(void)
{
    int dx, dy;
    float mx, my;
    float speed;

    if (!input.api.GetMotion) {
        return;
    }
    if (cls.key_dest & (KEY_MENU | KEY_CONSOLE)) {
        return;
    }
    if (!input.api.GetMotion(&dx, &dy)) {
        return;
    }

    if (m_filter->integer) {
        mx = (dx + input.old_dx) * 0.5f;
        my = (dy + input.old_dy) * 0.5f;
    }
    else {
        mx = dx;
        my = dy;
    }

    input.old_dx = dx;
    input.old_dy = dy;

    if (!mx && !my) {
        return;
    }

    Cvar_ClampValue(m_accel, 0, 1);

    speed = std::sqrtf(mx * mx + my * my);
    speed = sensitivity->value + speed * m_accel->value;

    mx *= speed;
    my *= speed;

    if (m_autosens->integer) {
        mx *= cl.fov_x * autosens_x;
        my *= cl.fov_y * autosens_y;
    }

    // add mouse X/Y movement
    if ((in_strafe.state & 1) || (lookstrafe->integer && !in_mlooking)) {
        cl.mousemove[1] += m_side->value * mx;
    }
    else {
        cl.viewAngles[vec3_t::Yaw] -= m_yaw->value * mx;
    }

    if ((in_mlooking || freelook->integer) && !(in_strafe.state & 1)) {
        cl.viewAngles[vec3_t::Pitch] += m_pitch->value * my * (m_invert->integer ? -1.f : 1.f);
    }
    else {
        cl.mousemove[0] -= m_forward->value * my;
    }
}


/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
static void CL_AdjustAngles(int msec)
{
    float speed;

    if (in_speed.state & BUTTON_STATE_HELD)
        speed = msec * cl_anglespeedkey->value * 0.001f;
    else
        speed = msec * 0.001f;

    if (!(in_strafe.state & 1)) {
        cl.viewAngles[vec3_t::Yaw] -= speed * cl_yawspeed->value * CL_KeyState(&in_right);
        cl.viewAngles[vec3_t::Yaw] += speed * cl_yawspeed->value * CL_KeyState(&in_left);
    }
    if (in_klook.state & 1) {
        cl.viewAngles[vec3_t::Pitch] -= speed * cl_pitchspeed->value * CL_KeyState(&in_forward);
        cl.viewAngles[vec3_t::Pitch] += speed * cl_pitchspeed->value * CL_KeyState(&in_back);
    }

    cl.viewAngles[vec3_t::Pitch] -= speed * cl_pitchspeed->value * CL_KeyState(&in_lookup);
    cl.viewAngles[vec3_t::Pitch] += speed * cl_pitchspeed->value * CL_KeyState(&in_lookdown);
}

/*
================
CL_BaseMove

Build and return the intended movement vector
================
*/
static vec3_t CL_BaseMove(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    if (in_strafe.state & 1) {
        outMove[1] += cl_sidespeed->value * CL_KeyState(&in_right);
        outMove[1] -= cl_sidespeed->value * CL_KeyState(&in_left);
    }

    outMove[1] += cl_sidespeed->value * CL_KeyState(&in_moveright);
    outMove[1] -= cl_sidespeed->value * CL_KeyState(&in_moveleft);

    outMove[2] += cl_upspeed->value * CL_KeyState(&in_up);
    outMove[2] -= cl_upspeed->value * CL_KeyState(&in_down);

    if (!(in_klook.state & 1)) {
        outMove[0] += cl_forwardspeed->value * CL_KeyState(&in_forward);
        outMove[0] -= cl_forwardspeed->value * CL_KeyState(&in_back);
    }

    // Adjust for speed key / running
    if ((in_speed.state & 1) ^ cl_run->integer) {
        VectorScale(outMove, 2, outMove);
        cl.cmd.buttons |= BUTTON_WALK;
    }
    else {
        cl.cmd.buttons |= BUTTON_WALK;
    }

    return outMove;
}

/*
================
CL_ClampSpeed

Returns the clamped movement speeds.
================
*/
static vec3_t CL_ClampSpeed(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    if (!cge)
        return inMove;

    // N&C: This... can happen :P
    if (!cge->pmoveParams)
        return vec3_zero();

    float speed = cge->pmoveParams->maxspeed;

    clamp(outMove[0], -speed, speed);
    clamp(outMove[1], -speed, speed);
    clamp(outMove[2], -speed, speed);

    return outMove;
}

static void CL_ClampPitch(void)
{
    float pitch;

    pitch = SHORT2ANGLE(cl.frame.playerState.pmove.delta_angles[vec3_t::Pitch]);
    if (pitch > 180)
        pitch -= 360;

    if (cl.viewAngles[vec3_t::Pitch] + pitch < -360)
        cl.viewAngles[vec3_t::Pitch] += 360; // wrapped
    if (cl.viewAngles[vec3_t::Pitch] + pitch > 360)
        cl.viewAngles[vec3_t::Pitch] -= 360; // wrapped

    if (cl.viewAngles[vec3_t::Pitch] + pitch > 89)
        cl.viewAngles[vec3_t::Pitch] = 89 - pitch;
    if (cl.viewAngles[vec3_t::Pitch] + pitch < -89)
        cl.viewAngles[vec3_t::Pitch] = -89 - pitch;
}

/*
=================
CL_UpdateCmd

Updates msec, angles and builds interpolated movement vector for local prediction.
Doesn't touch command forward/side/upmove, these are filled by CL_FinalizeCmd.
=================
*/
void CL_UpdateCmd(int msec)
{
    VectorClear(cl.localmove);

    if (sv_paused->integer) {
        return;
    }

    // Add to milliseconds of time to apply the move
    cl.cmd.msec += msec;

    // Adjust viewAngles
    CL_AdjustAngles(msec);

    // Get basic movement from keyboard
    cl.localmove = CL_BaseMove(cl.localmove);

    // Allow mice to add to the move
    CL_MouseMove();

    // Add accumulated mouse forward/side movement
    cl.localmove[0] += cl.mousemove[0];
    cl.localmove[1] += cl.mousemove[1];

    // Clamp to server defined max speed
    cl.localmove = CL_ClampSpeed(cl.localmove);

    CL_ClampPitch();

    cl.cmd.angles[0] = ANGLE2SHORT(cl.viewAngles[0]);
    cl.cmd.angles[1] = ANGLE2SHORT(cl.viewAngles[1]);
    cl.cmd.angles[2] = ANGLE2SHORT(cl.viewAngles[2]);
}

static void m_autosens_changed(cvar_t* self)
{
    float fov;

    if (self->value > 90.0f && self->value <= 179.0f)
        fov = self->value;
    else
        fov = 90.0f;

    autosens_x = 1.0f / fov;
    autosens_y = 1.0f / CL_GM_CalcFOV(fov, 4, 3);
}

/*
============
CL_RegisterInput
============
*/
void CL_RegisterInput(void)
{
    Cmd_AddCommand("centerview", IN_CenterView);

    Cmd_AddCommand("+moveup", IN_UpDown);
    Cmd_AddCommand("-moveup", IN_UpUp);
    Cmd_AddCommand("+movedown", IN_DownDown);
    Cmd_AddCommand("-movedown", IN_DownUp);
    Cmd_AddCommand("+left", IN_LeftDown);
    Cmd_AddCommand("-left", IN_LeftUp);
    Cmd_AddCommand("+right", IN_RightDown);
    Cmd_AddCommand("-right", IN_RightUp);
    Cmd_AddCommand("+forward", IN_ForwardDown);
    Cmd_AddCommand("-forward", IN_ForwardUp);
    Cmd_AddCommand("+back", IN_BackDown);
    Cmd_AddCommand("-back", IN_BackUp);
    Cmd_AddCommand("+lookup", IN_LookupDown);
    Cmd_AddCommand("-lookup", IN_LookupUp);
    Cmd_AddCommand("+lookdown", IN_LookdownDown);
    Cmd_AddCommand("-lookdown", IN_LookdownUp);
    Cmd_AddCommand("+strafe", IN_StrafeDown);
    Cmd_AddCommand("-strafe", IN_StrafeUp);
    Cmd_AddCommand("+moveleft", IN_MoveleftDown);
    Cmd_AddCommand("-moveleft", IN_MoveleftUp);
    Cmd_AddCommand("+moveright", IN_MoverightDown);
    Cmd_AddCommand("-moveright", IN_MoverightUp);
    Cmd_AddCommand("+speed", IN_SpeedDown);
    Cmd_AddCommand("-speed", IN_SpeedUp);
    Cmd_AddCommand("+attack", IN_AttackDown);
    Cmd_AddCommand("-attack", IN_AttackUp);
    Cmd_AddCommand("+use", IN_UseDown);
    Cmd_AddCommand("-use", IN_UseUp);
    Cmd_AddCommand("impulse", IN_Impulse);
    Cmd_AddCommand("+klook", IN_KLookDown);
    Cmd_AddCommand("-klook", IN_KLookUp);
    Cmd_AddCommand("+mlook", IN_MLookDown);
    Cmd_AddCommand("-mlook", IN_MLookUp);

    Cmd_AddCommand("in_restart", IN_Restart_f);

    cl_nodelta = Cvar_Get("cl_nodelta", "0", 0);
    cl_maxpackets = Cvar_Get("cl_maxpackets", "30", 0);
    cl_fuzzhack = Cvar_Get("cl_fuzzhack", "0", 0);
    cl_packetdup = Cvar_Get("cl_packetdup", "1", 0);
#ifdef _DEBUG
    cl_showpackets = Cvar_Get("cl_showpackets", "0", 0);
#endif
    cl_instantpacket = Cvar_Get("cl_instantpacket", "1", 0);
    cl_batchcmds = Cvar_Get("cl_batchcmds", "1", 0);

    cl_upspeed = Cvar_Get("cl_upspeed", "200", 0);
    cl_forwardspeed = Cvar_Get("cl_forwardspeed", "200", 0);
    cl_sidespeed = Cvar_Get("cl_sidespeed", "200", 0);
    cl_yawspeed = Cvar_Get("cl_yawspeed", "140", 0);
    cl_pitchspeed = Cvar_Get("cl_pitchspeed", "150", CVAR_CHEAT);
    cl_anglespeedkey = Cvar_Get("cl_anglespeedkey", "1.5", CVAR_CHEAT);
    cl_run = Cvar_Get("cl_run", "1", CVAR_ARCHIVE);

    freelook = Cvar_Get("freelook", "1", CVAR_ARCHIVE);
    lookspring = Cvar_Get("lookspring", "0", CVAR_ARCHIVE);
    lookstrafe = Cvar_Get("lookstrafe", "0", CVAR_ARCHIVE);
    sensitivity = Cvar_Get("sensitivity", "3", CVAR_ARCHIVE);

    m_pitch = Cvar_Get("m_pitch", "0.022", CVAR_ARCHIVE);
    m_invert = Cvar_Get("m_invert", "0", CVAR_ARCHIVE);
    m_yaw = Cvar_Get("m_yaw", "0.022", 0);
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
    vec3_t move;

    // command buffer ticks in sync with cl_maxfps
    if (cmd_buffer.waitCount > 0) {
        cmd_buffer.waitCount--;
    }
    if (cl_cmdbuf.waitCount > 0) {
        cl_cmdbuf.waitCount--;
    }

    if (cls.state != ca_active) {
        return; // not talking to a server
    }

    if (sv_paused->integer) {
        return;
    }

    //
    // figure button bits
    //

    if (in_attack.state & 3)
        cl->cmd.buttons |= BUTTON_ATTACK;
    if (in_use.state & 3)
        cl->cmd.buttons |= BUTTON_USE;

    in_attack.state &= ~2;
    in_use.state &= ~2;

    if (cls.key_dest == KEY_GAME && Key_AnyKeyDown()) {
        cl->cmd.buttons |= BUTTON_ANY;
    }

    if (cl->cmd.msec > 250) {
        cl->cmd.msec = 100;        // time was unreasonable
    }

    // rebuild the movement vector
    VectorClear(move);

    // get basic movement from keyboard
    move = CL_BaseMove(move);

    // add mouse forward/side movement
    move[0] += cl->mousemove[0];
    move[1] += cl->mousemove[1];

    // clamp to server defined max speed
    move = CL_ClampSpeed(move);

    // store the movement vector
    cl->cmd.forwardmove = move[0];
    cl->cmd.sidemove = move[1];
    cl->cmd.upmove = move[2];

    // clear all states
    cl->mousemove[0] = 0;
    cl->mousemove[1] = 0;

    KeyClear(&in_right);
    KeyClear(&in_left);

    KeyClear(&in_moveright);
    KeyClear(&in_moveleft);

    KeyClear(&in_up);
    KeyClear(&in_down);

    KeyClear(&in_forward);
    KeyClear(&in_back);

    KeyClear(&in_lookup);
    KeyClear(&in_lookdown);

    cl->cmd.impulse = in_impulse;
    in_impulse = 0;

    // save this command off for prediction
    cl->cmdNumber++;
    cl->cmds[cl->cmdNumber & CMD_MASK] = cl->cmd;

    // clear pending cmd
    memset(&cl->cmd, 0, sizeof(cl->cmd));
}