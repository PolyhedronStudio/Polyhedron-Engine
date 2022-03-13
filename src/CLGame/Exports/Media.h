// License here.
// 
//
// ClientGameMedia implementation.
#pragma once

#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Media IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameMedia : public IClientGameExportMedia {
public:
    //! Destructor.
    virtual ~ClientGameMedia() = default;

    /**
    *   @brief Called upon initialization of the renderer.
    **/
    void Initialize() final;

    /**
    *   @brief Called when the client stops the renderer.
    * 
    *   @details    Used to unload remaining data.
    **/
    void Shutdown() final;

    /**
    *   @brief Called when the client wants to acquire the name of a load state.
    **/
    std::string GetLoadStateName(LoadState loadState) final;

    /**
    *   @brief  This is called when the client spawns into a server,
    *   
    *   @details    Used to register world related media (particles, view models, sounds).
    **/
    void LoadWorld() final;

private:
    /**
    *   @brief Loads up the view(-weapon) models of a client.
    **/
    void LoadViewModels();

    /**
    *   @brief Load client models media here.
    **/
    void LoadModels();
    /**
    *   @brief Load client image media here.
    **/
    void LoadImages();
    /**
    *   Load client sound media here.
    **/
    void LoadSounds();
};

