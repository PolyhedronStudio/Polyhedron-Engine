/***
*
*	License here.
*
*	@file
*
*	Client Game Media Interface Implementation.
* 
***/
#pragma once

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
    std::string GetLoadStateName(int32_t loadState) final;

    /**
    *   @brief  This is called when the client spawns into a server,
    *   
    *   @details    Used to register world related media (particles, view models, sounds).
    **/
    void LoadWorld() final;

    /**
    *   @brief  Loads up the view(-weapon) models of a client.
    **/
    void LoadViewModels();

    /**
    *   @brief  Load and set sky media, rotation and axis to their ConfigString settings.
    **/
    void LoadAndConfigureSky();

    /**
    *   @brief  Loads up the data for the given client. Here you can set the default
    *           models that it'll load, or totally disable clients from doing their
    *           own.
    *   
    *           Think about a mod where you have a class system, you can load the info
    *           here.
    **/
    void LoadClientInfo(ClientInfo* ci, const char* str);

    /**
    *   @brief  Breaks up playerskin into name (optional), model and skin components.
    *           If model or skin are found to be invalid, replaces them with sane defaults.
    **/
    void ParsePlayerSkin(char* name, char* model, char* skin, const char* s);


    /**
    *	@brief	These are here temporarily, they should probably move over to gameworld itself...
    */
    int32_t         numberOfEntities = 0;     // current number, <= maxEntities
    int32_t         maxEntities = 0;
private:
    /**
    *   @brief  Load client specific model media.
    **/
    void LoadModels();
    /**
    *   @brief  Load client specific image media.
    **/
    void LoadImages();
    /**
    *   @brief  Load client specific sound media here.
    **/
    void LoadSounds();
};

