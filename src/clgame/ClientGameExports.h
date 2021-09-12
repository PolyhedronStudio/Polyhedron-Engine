// License here.
// 
//
// ClientGameExports implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// MAIN interface to implement. It holds pointers to actual sub interfaces,
// which one of course has to implement as well.
//---------------------------------------------------------------------
class ClientGameExports : public IClientGameExports {
public:
    //---------------------------------------------------------------------
    // General.
    //---------------------------------------------------------------------
    // Calculates the FOV the client is running. (Important to have in order.)
    float CalculateClientFieldOfView(float fieldOfViewX, float width, float height) final;

    // Called upon whenever a client disconnects, for whichever reason.
    // Could be him quiting, or pinging out etc.
    void ClearClientState() final;

    // Updates the origin. (Used by the engine for determining current audio position too.)
    void UpdateClientOrigin();

    // Called when a demo is being seeked through.
    void DemoSeek();

    // Called after all downloads are done. (Aka, a map has started.)
    // Not used for demos.
    void ClientBegin();
    // Called each VALID client frame. Handle per VALID frame basis 
    // things here.
    void ClientDeltaFrame();
    // Called each client frame. Handle per frame basis things here.
    void ClientFrame();
    // Called when a disconnect even occures. Including those for Com_Error
    void ClientDisconnect();

    // Called when there is a needed retransmit of user info variables.
    void ClientUpdateUserinfo(cvar_t* var, from_t from);

private:
    // Utility function for CLG_UpdateOrigin
    float LerpClientFieldOfView(float oldFieldOfView, float newFieldOfView, float lerp);
};

