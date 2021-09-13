// License here.
// 
//
// ClientGameMedia implementation.
#pragma once

#include "shared/interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Media IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameMedia : public IClientGameExportMedia {
public:
    // Called when the client wants to know the name of a custom load state.
    std::string GetLoadStateName(LoadState loadState) final;

    // This is called when the client starts, but also when the renderer has had
    // modified settings.
    //
    // It should register the basic screen media, 2D icons etc.
    void LoadScreen() final;

    // This is called when the client spawns into a server,
    //
    // It should register world related media here, such as particles that are
    // used in-game, or view models, or sounds, etc.
    void LoadWorld() final;

    // Called upon initialization of the renderer.
    void Initialize() final;

    // This is called when the client stops the renderer.
    // Use this to unload remaining data.
    void Shutdown() final;
};

