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
    cl->clientUserCommand.moveCommand.msec += miliseconds;

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

    cl->clientUserCommand.moveCommand.viewAngles[0] = cl->viewAngles[0];
    cl->clientUserCommand.moveCommand.viewAngles[1] = cl->viewAngles[1];
    cl->clientUserCommand.moveCommand.viewAngles[2] = cl->viewAngles[2];
}

//---------------
// ClientGameMedia::FinalizeFrameMovementCommand
//
//---------------
void ClientGameMovement::FinalizeFrameMovementCommand() {

}
