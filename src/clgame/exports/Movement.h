// License here.
// 
//
// ClientGameMovement implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Movement IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameMovement : public IClientGameExportMovement {
public:
    // Called when the movement command needs to be build for the given
    // client networking frame.
    void BuildFrameMovementCommand(int32_t miliseconds) final;
    // Finished off building the actual movement vector before sending it
    // to server.
    void FinalizeFrameMovementCommand() final;
};

