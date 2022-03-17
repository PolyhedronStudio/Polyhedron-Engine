#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Main.h"
#include "../Media.h"
#include "../Predict.h"
#include "../TemporaryEntities.h"
#include "../View.h"

// Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"

// Movement.
#include "Movement.h"

#include "../Input/KeyBinding.h"

/**
*   Player Input Key Binds.
**/
KeyBinding ClientGameMovement::in_klook;
KeyBinding ClientGameMovement::in_left, ClientGameMovement::in_right, ClientGameMovement::in_forward, ClientGameMovement::in_back;
KeyBinding ClientGameMovement::in_lookup, ClientGameMovement::in_lookdown, ClientGameMovement::in_moveleft, ClientGameMovement::in_moveright;
KeyBinding ClientGameMovement::in_strafe, ClientGameMovement::in_speed, ClientGameMovement::in_use, ClientGameMovement::in_reload, ClientGameMovement::in_primary_fire, ClientGameMovement::in_secondary_fire;
KeyBinding ClientGameMovement::in_up, ClientGameMovement::in_down;

/**
*   Mouse Input Sampling Configuration CVars.
**/
cvar_t* ClientGameMovement::m_filter;
cvar_t* ClientGameMovement::m_accel;
cvar_t* ClientGameMovement::m_autosens;

cvar_t* ClientGameMovement::m_pitch;
cvar_t* ClientGameMovement::m_invert;
cvar_t* ClientGameMovement::m_yaw;
cvar_t* ClientGameMovement::m_forward;
cvar_t* ClientGameMovement::m_side;

/**
*   Movement Speed Sampling Configuration CVars.
**/
cvar_t* ClientGameMovement::cl_upspeed;
cvar_t* ClientGameMovement::cl_forwardspeed;
cvar_t* ClientGameMovement::cl_sidespeed;
cvar_t* ClientGameMovement::cl_yawspeed;
cvar_t* ClientGameMovement::cl_pitchspeed;
cvar_t* ClientGameMovement::cl_anglespeedkey;
cvar_t* ClientGameMovement::cl_instantpacket;

cvar_t* ClientGameMovement::cl_run; // WID: TODO: This is the new one, so it can be externed.

cvar_t* ClientGameMovement::sensitivity;

/**
*   This looks silly but.
*   TODO:   This is for mouse delta x/y sampling and is related to the user input
*           system APIs. Look into that for cleaning this out.
*/
ClientGameMovement::in_state_t ClientGameMovement::inputState;

//! Maintains state of whether to use mouse looking or not.
qboolean ClientGameMovement::in_mlooking;

//! The actual number supplied as an argument to the impulse cmd.
int32_t ClientGameMovement::in_impulse;



/**
*   @brief 
**/
void ClientGameMovement::BuildFrameMovementCommand(int32_t miliseconds) {
    // Reset for this frame.
    cl->localMove = vec3_zero();

    // In case of a pause, return. (There won't be any user input being fetched for movement.)
    if (sv_paused->integer) {
        return;
    }

    // Add to milliseconds of time to apply the move
    cl->moveCommand.input.msec += miliseconds;

    // Adjust viewAngles
    AdjustAngles(miliseconds);

    // Get basic movement from keyboard
    cl->localMove = BaseMove(cl->localMove);

    // Allow mice to add to the move
    MouseMove();

    // Add accumulated mouse forward/side movement
    cl->localMove[0] += cl->mouseMove[0];
    cl->localMove[1] += cl->mouseMove[1];

    // Clamp to server defined max speed
    cl->localMove = ClampSpeed(cl->localMove);

    // Clamp the pitch.
    ClampPitch();

    // Assign view angles to move command user input.
    cl->moveCommand.input.viewAngles[0] = cl->viewAngles[0];
    cl->moveCommand.input.viewAngles[1] = cl->viewAngles[1];
    cl->moveCommand.input.viewAngles[2] = cl->viewAngles[2];
}

/**
*   @brief 
**/
void ClientGameMovement::FinalizeFrameMovementCommand() {
    // Not talking to any server, so there is little use to proceed.
    if (clgi.GetClienState() != ClientConnectionState::Active) {
        return;
    }

    // Paused == Paused, nuff said.
    if (sv_paused->integer) {
        return;
    }

    // Primary Fire Button Bits.
    if (in_primary_fire.GetKeyStateBits() & (ButtonState::Held | ButtonState::Down)) {
        cl->moveCommand.input.buttons |= ButtonBits::PrimaryFire;
    }

    // Secondary Fire Button Bits.
    if (in_secondary_fire.GetKeyStateBits() & (ButtonState::Held | ButtonState::Down)) {
        cl->moveCommand.input.buttons |= ButtonBits::SecondaryFire;
    }

    // Reload Button Bits.
    if (in_use.GetKeyStateBits() & (ButtonState::Held | ButtonState::Down)) {
        cl->moveCommand.input.buttons |= ButtonBits::Use;
    }

    // Use Button Bits.
    if (in_use.GetKeyStateBits() & (ButtonState::Held | ButtonState::Down)) {
        cl->moveCommand.input.buttons |= ButtonBits::Use;
    }

    // Undo the ButtonState::Down for the next frame, it needs a repress for
    // that to be re-enabled.
    in_primary_fire.SetKeyStateBits(in_primary_fire.GetKeyStateBits() & ~ButtonState::Down);
    in_secondary_fire.SetKeyStateBits(in_secondary_fire.GetKeyStateBits() & ~ButtonState::Down);
    in_reload.SetKeyStateBits(in_reload.GetKeyStateBits() & ~ButtonState::Down);
    in_use.SetKeyStateBits(in_use.GetKeyStateBits() & ~ButtonState::Down);

    // Whether to run or not, depends on whether auto-run is on or off.
    if (cl_run->value) {
        if (in_speed.GetKeyStateBits() & ButtonState::Held) {
            cl->moveCommand.input.buttons |= ButtonBits::Walk;
        }
    }
    else {
        if (!(in_speed.GetKeyStateBits() & ButtonState::Held)) {
            cl->moveCommand.input.buttons |= ButtonBits::Walk;
        }
    }

    // Always send in case any button was down at all in-game.
    if (clgi.Key_GetDest() == KEY_GAME && clgi.Key_AnyKeyDown()) {
        cl->moveCommand.input.buttons |= ButtonBits::Any;
    }

    // Set a 'normal' time, since anything over 250 is unreasonable
    if (cl->moveCommand.input.msec > 250) {
        cl->moveCommand.input.msec = 100;
    }

    // Rebuild the movement vector
    vec3_t move = vec3_zero();

    // Get basic movement from keyboard
    move = BaseMove(move);

    // Add mouse forward/side movement
    move[0] += cl->mouseMove[0];
    move[1] += cl->mouseMove[1];

    // Clamp to server defined max speed
    move = ClampSpeed(move);

    // Store the movement vector
    cl->moveCommand.input.forwardMove = move[0];
    cl->moveCommand.input.rightMove = move[1];
    cl->moveCommand.input.upMove = move[2];

    // Clear all states
    cl->mouseMove[0] = 0;
    cl->mouseMove[1] = 0;

    // Assign current impulse, and reset the input for the next frame.
    cl->moveCommand.input.impulse = in_impulse;
    in_impulse = 0;

    // Save this command off for client move prediction.
    cl->currentClientCommandNumber++;
    cl->clientUserCommands[cl->currentClientCommandNumber & CMD_MASK] = cl->moveCommand;

    // Clear keys that need clearing for the next frame.
    in_right.ClearKeyState();
    in_left.ClearKeyState();

    in_moveright.ClearKeyState();
    in_moveleft.ClearKeyState();

    in_up.ClearKeyState();
    in_down.ClearKeyState();

    in_forward.ClearKeyState();
    in_back.ClearKeyState();

    in_lookup.ClearKeyState();
    in_lookdown.ClearKeyState();

    // Clear the pending cmd for the next frame.
    cl->moveCommand = {};
}

/**
*   @brief  Handles the mouse move based input adjustment.
**/
void ClientGameMovement::MouseMove() {
    int deltaX, deltaY;
    float motionX, motionY;
    float speed;

    if (!clgi.Mouse_GetMotion(&deltaX, &deltaY)) {
        return;
    }
    if (clgi.Key_GetDest() & (KEY_MENU | KEY_CONSOLE)) {
        return;
    }

    if (m_filter->integer) {
        motionX = (deltaX + inputState.oldDeltaX) * 0.5f;
        motionY = (deltaY + inputState.oldDeltaY) * 0.5f;
    }
    else {
        motionX = deltaX;
        motionY = deltaY;
    }

    inputState.oldDeltaX = deltaX;
    inputState.oldDeltaY = deltaY;

    if (!motionX && !motionY) {
        return;
    }

    clgi.Cvar_ClampValue(m_accel, 0, 1);

    speed = std::sqrtf(motionX * motionX + motionY * motionY);
    speed = sensitivity->value + speed * m_accel->value;

    motionX *= speed;
    motionY *= speed;

    if (m_autosens->integer) {
        motionX *= cl->fov_x * cl->autosens_x;
        motionY *= cl->fov_y * cl->autosens_y;
    }

    // Add mouse X/Y movement
    cl->viewAngles[vec3_t::Yaw] -= m_yaw->value * motionX;
    cl->viewAngles[vec3_t::Pitch] += m_pitch->value * motionY * (m_invert->integer ? -1.f : 1.f);
}

/**
*   @brief  Moves the local angle positions
**/
void ClientGameMovement::AdjustAngles(int32_t miliseconds)
{
    float speed;

    if (in_speed.GetKeyStateBits() & ButtonState::Held)
        speed = miliseconds * cl_anglespeedkey->value * 0.001f;
    else
        speed = miliseconds * 0.001f;

    cl->viewAngles[vec3_t::Yaw] -= speed * cl_yawspeed->value * in_right.GetStateFraction();
    cl->viewAngles[vec3_t::Yaw] += speed * cl_yawspeed->value * in_left.GetStateFraction();
    cl->viewAngles[vec3_t::Pitch] -= speed * cl_pitchspeed->value * in_lookup.GetStateFraction();
    cl->viewAngles[vec3_t::Pitch] += speed * cl_pitchspeed->value * in_lookdown.GetStateFraction();
}

/**
*   @brief  Build and return the intended movement vector
**/
vec3_t ClientGameMovement::BaseMove(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    if (in_strafe.GetKeyStateBits() & 1) {
        outMove[1] += cl_sidespeed->value * in_right.GetStateFraction();
        outMove[1] -= cl_sidespeed->value * in_left.GetStateFraction();
    }

    outMove[1] += cl_sidespeed->value * in_moveright.GetStateFraction();
    outMove[1] -= cl_sidespeed->value * in_moveleft.GetStateFraction();

    outMove[2] += cl_upspeed->value * in_up.GetStateFraction();
    outMove[2] -= cl_upspeed->value * in_down.GetStateFraction();

    if (!(in_klook.GetKeyStateBits() & 1)) {
        outMove[0] += cl_forwardspeed->value * in_forward.GetStateFraction();
        outMove[0] -= cl_forwardspeed->value * in_back.GetStateFraction();
    }

    return outMove;
}

/**
*   @brief  Returns the clamped movement speeds.
**/
vec3_t ClientGameMovement::ClampSpeed(const vec3_t& inMove)
{
    vec3_t outMove = inMove;

    // If movement ever starts feeling wrong after all, then move this code out of it.
#if 1
    float speed = cl_forwardspeed->value; // TODO: FIX PM_ //pmoveParams->maxspeed;

    outMove[0] = Clampf(outMove[0], -speed, speed);
    outMove[1] = Clampf(outMove[1], -speed, speed);
    outMove[2] = Clampf(outMove[2], -speed, speed);
#endif

    return outMove;
}

/**
*   @brief  Ensures the Pitch is clamped to prevent camera issues.
**/
void ClientGameMovement::ClampPitch(void)
{
    float pitch;

    pitch = cl->frame.playerState.pmove.deltaAngles[vec3_t::Pitch];
    if (pitch > 180)
        pitch -= 360;

    if (cl->viewAngles[vec3_t::Pitch] + pitch < -360)
        cl->viewAngles[vec3_t::Pitch] += 360; // wrapped
    if (cl->viewAngles[vec3_t::Pitch] + pitch > 360)
        cl->viewAngles[vec3_t::Pitch] -= 360; // wrapped

    if (cl->viewAngles[vec3_t::Pitch] + pitch > 89)
        cl->viewAngles[vec3_t::Pitch] = 89 - pitch;
    if (cl->viewAngles[vec3_t::Pitch] + pitch < -89)
        cl->viewAngles[vec3_t::Pitch] = -89 - pitch;
}

/**
*   @brief  Register input messages and binds them to a callback function.
*           Bindings are set in the config files, and/or the options menu.
* 
*           For more information, it still works like in q2pro.
**/
void ClientGameMovement::RegisterInput(void)
{
    // Register Input Key bind Commands.
    clgi.Cmd_AddCommand("centerview", IN_CenterView);
    clgi.Cmd_AddCommand("+moveup", IN_UpDown);
    clgi.Cmd_AddCommand("-moveup", IN_UpUp);
    clgi.Cmd_AddCommand("+movedown", IN_DownDown);
    clgi.Cmd_AddCommand("-movedown", IN_DownUp);
    clgi.Cmd_AddCommand("+left", IN_LeftDown);
    clgi.Cmd_AddCommand("-left", IN_LeftUp);
    clgi.Cmd_AddCommand("+right", IN_RightDown);
    clgi.Cmd_AddCommand("-right", IN_RightUp);
    clgi.Cmd_AddCommand("+forward", IN_ForwardDown);
    clgi.Cmd_AddCommand("-forward", IN_ForwardUp);
    clgi.Cmd_AddCommand("+back", IN_BackDown);
    clgi.Cmd_AddCommand("-back", IN_BackUp);
    clgi.Cmd_AddCommand("+lookup", IN_LookupDown);
    clgi.Cmd_AddCommand("-lookup", IN_LookupUp);
    clgi.Cmd_AddCommand("+lookdown", IN_LookdownDown);
    clgi.Cmd_AddCommand("-lookdown", IN_LookdownUp);
    clgi.Cmd_AddCommand("+strafe", IN_StrafeDown);
    clgi.Cmd_AddCommand("-strafe", IN_StrafeUp);
    clgi.Cmd_AddCommand("+moveleft", IN_MoveleftDown);
    clgi.Cmd_AddCommand("-moveleft", IN_MoveleftUp);
    clgi.Cmd_AddCommand("+moveright", IN_MoverightDown);
    clgi.Cmd_AddCommand("-moveright", IN_MoverightUp);
    clgi.Cmd_AddCommand("+speed", IN_SpeedDown);
    clgi.Cmd_AddCommand("-speed", IN_SpeedUp);
    clgi.Cmd_AddCommand("+primaryfire", IN_PrimaryFireDown);
    clgi.Cmd_AddCommand("-primaryfire", IN_PrimaryFireUp);
    clgi.Cmd_AddCommand("+secondaryfire", IN_SecondaryFireDown);
    clgi.Cmd_AddCommand("-secondaryfire", IN_SecondaryFireUp);
    clgi.Cmd_AddCommand("+reload", IN_ReloadDown);
    clgi.Cmd_AddCommand("-reload", IN_ReloadUp);
    clgi.Cmd_AddCommand("+use", IN_UseDown);
    clgi.Cmd_AddCommand("-use", IN_UseUp);
    clgi.Cmd_AddCommand("impulse", IN_Impulse);

    // Create Cvars.
    cl_upspeed = clgi.Cvar_Get("cl_upspeed", "300", 0);
    cl_forwardspeed = clgi.Cvar_Get("cl_forwardspeed", "300", 0);
    cl_sidespeed = clgi.Cvar_Get("cl_sidespeed", "300", 0);
    cl_yawspeed = clgi.Cvar_Get("cl_yawspeed", "0.140", 0);
    cl_pitchspeed = clgi.Cvar_Get("cl_pitchspeed", "0.150", CVAR_CHEAT);
    cl_anglespeedkey = clgi.Cvar_Get("cl_anglespeedkey", "1.5", CVAR_CHEAT);
    cl_run = clgi.Cvar_Get("cl_run", "1", CVAR_ARCHIVE);

    // Fetch CVars.
    cl_instantpacket = clgi.Cvar_Get("cl_instantpacket", "0", 0);

    m_filter = clgi.Cvar_Get("m_filter", "", 0);
    m_accel = clgi.Cvar_Get("m_accel", "", 0);
    m_autosens = clgi.Cvar_Get("m_autosens", "0", 0);

    m_pitch = clgi.Cvar_Get("m_pitch", "", 0);
    m_invert = clgi.Cvar_Get("m_invert", "", 0);
    m_yaw = clgi.Cvar_Get("m_yaw", "", 0);
    m_forward = clgi.Cvar_Get("m_forward", "", 0);
    m_side = clgi.Cvar_Get("m_side", "", 0);

    sensitivity = clgi.Cvar_Get("sensitivity", "0", 0);
}


/**
*
*   Key Input Sampler Functions.
* 
**/
void ClientGameMovement::IN_KLookDown(void) {
    in_klook.ProcessKeyDown();
}
void ClientGameMovement::IN_KLookUp(void) { 
    in_klook.ProcessKeyUp(); 
}

void ClientGameMovement::IN_UpDown(void) { 
    in_up.ProcessKeyDown(); 
}
void ClientGameMovement::IN_UpUp(void) { 
    in_up.ProcessKeyUp();
}

void ClientGameMovement::IN_DownDown(void) { 
    in_down.ProcessKeyDown();
}
void ClientGameMovement::IN_DownUp(void) { 
    in_down.ProcessKeyUp(); 
}

void ClientGameMovement::IN_LeftDown(void) {
    in_left.ProcessKeyDown();
}
void ClientGameMovement::IN_LeftUp(void) { 
    in_left.ProcessKeyUp();
}

void ClientGameMovement::IN_RightDown(void) { 
    in_right.ProcessKeyDown();
}
void ClientGameMovement::IN_RightUp(void) { 
    in_right.ProcessKeyUp();
}

void ClientGameMovement::IN_ForwardDown(void) { 
    in_forward.ProcessKeyDown();
}
void ClientGameMovement::IN_ForwardUp(void) { 
    in_forward.ProcessKeyUp();
}

void ClientGameMovement::IN_BackDown(void) { 
    in_back.ProcessKeyDown();
}
void ClientGameMovement::IN_BackUp(void) { 
    in_back.ProcessKeyUp();
}

void ClientGameMovement::IN_LookupDown(void) { 
    in_lookup.ProcessKeyDown();
}
void ClientGameMovement::IN_LookupUp(void) { 
    in_lookup.ProcessKeyUp();
}

void ClientGameMovement::IN_LookdownDown(void) { 
    in_lookdown.ProcessKeyDown();
}
void ClientGameMovement::IN_LookdownUp(void) { 
    in_lookdown.ProcessKeyUp();
}

void ClientGameMovement::IN_MoveleftDown(void) { 
    in_moveleft.ProcessKeyDown();
}
void ClientGameMovement::IN_MoveleftUp(void) { 
    in_moveleft.ProcessKeyUp();
}

void ClientGameMovement::IN_MoverightDown(void) { 
    in_moveright.ProcessKeyDown();
}
void ClientGameMovement::IN_MoverightUp(void) { 
    in_moveright.ProcessKeyUp();
}

void ClientGameMovement::IN_SpeedDown(void) { 
    in_speed.ProcessKeyDown();
}
void ClientGameMovement::IN_SpeedUp(void) { 
    in_speed.ProcessKeyUp();
}

void ClientGameMovement::IN_StrafeDown(void) { 
    in_strafe.ProcessKeyDown();
}
void ClientGameMovement::IN_StrafeUp(void) { 
    in_strafe.ProcessKeyUp();
}

/**
*   +primaryfire Button down/up.
**/
void ClientGameMovement::IN_PrimaryFireDown(void) {
    in_primary_fire.ProcessKeyDown();

    if (cl_instantpacket->integer && clgi.GetClienState() == ClientConnectionState::Active) {// && cls->netchan) {
        cl->sendPacketNow = true;
    }
}
void ClientGameMovement::IN_PrimaryFireUp(void) {
    in_primary_fire.ProcessKeyUp();
}

/**
*   +secondaryfire Button down/up.
**/
void ClientGameMovement::IN_SecondaryFireDown(void) {
    in_secondary_fire.ProcessKeyDown();

    if (cl_instantpacket->integer && clgi.GetClienState() == ClientConnectionState::Active) {// && cls->netchan) {
        cl->sendPacketNow = true;
    }
}

void ClientGameMovement::IN_SecondaryFireUp(void) {
    in_secondary_fire.ProcessKeyUp();
}

/**
*   +reload Button down/up.
**/
void ClientGameMovement::IN_ReloadDown(void) {
    in_reload.ProcessKeyDown();

    if (cl_instantpacket->integer && clgi.GetClienState() == ClientConnectionState::Active) {// && cls.netChannel) {
        cl->sendPacketNow = true;
    }
}
void ClientGameMovement::IN_ReloadUp(void) {
    in_reload.ProcessKeyUp();
}

/**
*   +use Button down/up.
**/
void ClientGameMovement::IN_UseDown(void) {
    in_use.ProcessKeyDown();

    if (cl_instantpacket->integer && clgi.GetClienState() == ClientConnectionState::Active) {// && cls.netChannel) {
        cl->sendPacketNow = true;
    }
}
void ClientGameMovement::IN_UseUp(void) {
    in_use.ProcessKeyUp();
}

/**
*   impulse
**/
void ClientGameMovement::IN_Impulse(void) {
    in_impulse = atoi(clgi.Cmd_Argv(1));
}

void ClientGameMovement::IN_CenterView(void) {
    cl->viewAngles.x = -cl->frame.playerState.pmove.deltaAngles[0];
}

void ClientGameMovement::IN_MLookDown(void) {
    in_mlooking = true;
}
void ClientGameMovement::IN_MLookUp(void) {
    in_mlooking = false;
}