#include "../clg_local.h"

#include "../clg_effects.h"
#include "../clg_entities.h"
#include "../clg_input.h"
#include "../clg_main.h"
#include "../clg_media.h"
#include "../clg_parse.h"
#include "../clg_predict.h"
#include "../clg_screen.h"
#include "../clg_tents.h"
#include "../clg_view.h"

#include "shared/interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Movement.h"

// WID: TODO: These gotta be relocated, and the entire implementation of certain things
// might have to if I think about it.
extern KeyBinding in_klook;
extern KeyBinding in_left, in_right, in_forward, in_back;
extern KeyBinding in_lookup, in_lookdown, in_moveleft, in_moveright;
extern KeyBinding in_strafe, in_speed, in_use, in_attack;
extern KeyBinding in_up, in_down;

extern int32_t in_impulse;
extern cvar_t* cl_run;

//---------------
// ClientGameMovement::BuildFrameMovementCommand
//
//---------------
void ClientGameMovement::BuildFrameMovementCommand(int32_t miliseconds) {
    cl->localmove = vec3_zero();

    if (sv_paused->integer) {
        return;
    }

    // Add to milliseconds of time to apply the move
    cl->clientMoveCommand.moveInput.msec += miliseconds;

    // Adjust viewAngles
    CLG_AdjustAngles(miliseconds);

    // Get basic movement from keyboard
    cl->localmove = CLG_BaseMove(cl->localmove);

    // Allow mice to add to the move
    CLG_MouseMove();

    // Add accumulated mouse forward/side movement
    cl->localmove[0] += cl->mousemove[0];
    cl->localmove[1] += cl->mousemove[1];

    // Clamp to server defined max speed
    cl->localmove = CLG_ClampSpeed(cl->localmove);

    CLG_ClampPitch();

    cl->clientMoveCommand.moveInput.viewAngles[0] = cl->viewAngles[0];
    cl->clientMoveCommand.moveInput.viewAngles[1] = cl->viewAngles[1];
    cl->clientMoveCommand.moveInput.viewAngles[2] = cl->viewAngles[2];
}

//---------------
// ClientGameMedia::FinalizeFrameMovementCommand
//
//---------------
void ClientGameMovement::FinalizeFrameMovementCommand() {
    if (clgi.GetClienState() != ClientConnectionState::Active) {
        return; // not talking to a server
    }

    if (sv_paused->integer) {
        return;
    }

    //
    // figure button bits
    //
    if (in_attack.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN))
        cl->clientMoveCommand.moveInput.buttons |= BUTTON_ATTACK;

    if (in_use.state & (BUTTON_STATE_HELD | BUTTON_STATE_DOWN))
        cl->clientMoveCommand.moveInput.buttons |= BUTTON_USE;

    // Undo the button_state_down for the next frame, it needs a repress for
    // that to be re-enabled.
    in_attack.state &= ~BUTTON_STATE_DOWN;
    in_use.state &= ~BUTTON_STATE_DOWN;

    // Whether to run or not, depends on whether auto-run is on or off.
    if (cl_run->value) {
        if (in_speed.state & BUTTON_STATE_HELD) {
            cl->clientMoveCommand.moveInput.buttons |= BUTTON_WALK;
        }
    }
    else {
        if (!(in_speed.state & BUTTON_STATE_HELD)) {
            cl->clientMoveCommand.moveInput.buttons |= BUTTON_WALK;
        }
    }

    // Always send in case any button was down at all in-game.
    if (clgi.Key_GetDest() == KEY_GAME && clgi.Key_AnyKeyDown()) {
        cl->clientMoveCommand.moveInput.buttons |= BUTTON_ANY;
    }

    if (cl->clientMoveCommand.moveInput.msec > 250) {
        cl->clientMoveCommand.moveInput.msec = 100;        // time was unreasonable
    }

    // Rebuild the movement vector
    vec3_t move = vec3_zero();

    // Get basic movement from keyboard
    move = CLG_BaseMove(move);

    // Add mouse forward/side movement
    move[0] += cl->mousemove[0];
    move[1] += cl->mousemove[1];

    // Clamp to server defined max speed
    move = CLG_ClampSpeed(move);

    // Store the movement vector
    cl->clientMoveCommand.moveInput.forwardMove = move[0];
    cl->clientMoveCommand.moveInput.rightMove = move[1];
    cl->clientMoveCommand.moveInput.upMove = move[2];

    // Clear all states
    cl->mousemove[0] = 0;
    cl->mousemove[1] = 0;

    cl->clientMoveCommand.moveInput.impulse = in_impulse;
    in_impulse = 0;

    // Save this command off for prediction
    cl->currentClientCommandNumber++;
    cl->clientUserCommands[cl->currentClientCommandNumber & CMD_MASK] = cl->clientMoveCommand;

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


    // Clear pending cmd
    cl->clientMoveCommand = {};
}
