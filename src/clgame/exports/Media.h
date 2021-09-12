// License here.
// 
//
// ClientGameExportCore implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Media IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameMedia : public IClientGameExportMedia {
public:
    // Called when the client wants to know the name of a custom load state.
    virtual std::string GetLoadStateName(LoadState loadState) = 0;

    // This is called when the client starts, but also when the renderer has had
    // modified settings.
    //
    // It should register the basic screen media, 2D icons etc.
    virtual void LoadScreen() = 0;

    // This is called when the client spawns into a server,
    //
    // It should register world related media here, such as particles that are
    // used in-game, or view models, or sounds, etc.
    virtual void LoadWorld() = 0;

    // Called upon initialization of the renderer.
    virtual void Initialize() = 0;
};

