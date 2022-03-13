#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Input.h"
#include "../Main.h"
#include "../Media.h"
#include "../Predict.h"
#include "../Screen.h"
#include "../TemporaryEntities.h"
#include "../View.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Movement.h"

// WID: TODO: These gotta be relocated, and the entire implementation of certain things
// might have to if I think about it.
extern KeyBinding in_klook;
extern KeyBinding in_left, in_right, in_forward, in_back;
extern KeyBinding in_lookup, in_lookdown, in_moveleft, in_moveright;
extern KeyBinding in_strafe, in_speed, in_use, in_reload, in_primary_fire, in_secondary_fire;
extern KeyBinding in_up, in_down;

extern int32_t in_impulse;
extern cvar_t* cl_run;

//---------------
// ClientGameMovement::BuildFrameMovementCommand
//
//---------------
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
    CLG_AdjustAngles(miliseconds);

    // Get basic movement from keyboard
    cl->localMove = CLG_BaseMove(cl->localMove);

    // Allow mice to add to the move
    CLG_MouseMove();

    // Add accumulated mouse forward/side movement
    cl->localMove[0] += cl->mouseMove[0];
    cl->localMove[1] += cl->mouseMove[1];

    // Clamp to server defined max speed
    cl->localMove = CLG_ClampSpeed(cl->localMove);

    // Clamp the pitch.
    CLG_ClampPitch();

    // Assign view angles to move command user input.
    cl->moveCommand.input.viewAngles[0] = cl->viewAngles[0];
    cl->moveCommand.input.viewAngles[1] = cl->viewAngles[1];
    cl->moveCommand.input.viewAngles[2] = cl->viewAngles[2];
}

//---------------
// ClientGameMedia::FinalizeFrameMovementCommand
//
//---------------
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
    if (in_primary_fire.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN)) {
        cl->moveCommand.input.buttons |= ButtonBits::PrimaryFire;
    }

    // Secondary Fire Button Bits.
    if (in_secondary_fire.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN)) {
        cl->moveCommand.input.buttons |= ButtonBits::SecondaryFire;
    }

    // Reload Button Bits.
    if (in_use.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN)) {
        cl->moveCommand.input.buttons |= ButtonBits::Use;
    }

    // Use Button Bits.
    if (in_use.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN)) {
        cl->moveCommand.input.buttons |= ButtonBits::Use;
    }

    // Undo the button_state_down for the next frame, it needs a repress for
    // that to be re-enabled.
    in_primary_fire.state   &= ~BUTTON_STATE_DOWN;
    in_secondary_fire.state &= ~BUTTON_STATE_DOWN;
    in_reload.state         &= ~BUTTON_STATE_DOWN;
    in_use.state            &= ~BUTTON_STATE_DOWN;

    // Whether to run or not, depends on whether auto-run is on or off.
    if (cl_run->value) {
        if (in_speed.state & BUTTON_STATE_HELD) {
            cl->moveCommand.input.buttons |= ButtonBits::Walk;
        }
    }
    else {
        if (!(in_speed.state & BUTTON_STATE_HELD)) {
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
    move = CLG_BaseMove(move);

    // Add mouse forward/side movement
    move[0] += cl->mouseMove[0];
    move[1] += cl->mouseMove[1];

    // Clamp to server defined max speed
    move = CLG_ClampSpeed(move);

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
    CLG_KeyClear(&in_right);
    CLG_KeyClear(&in_left);

    CLG_KeyClear(&in_moveright);
    CLG_KeyClear(&in_moveleft);

    CLG_KeyClear(&in_up);
    CLG_KeyClear(&in_down);

    CLG_KeyClear(&in_forward);
    CLG_KeyClear(&in_back);

    CLG_KeyClear(&in_lookup);
    CLG_KeyClear(&in_lookdown);


    // Clear the pending cmd for the next frame.
    cl->moveCommand = {};
}
