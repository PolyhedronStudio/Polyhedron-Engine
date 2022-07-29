// License here.
// 
//
// ClientGameExports implementation.
#pragma once

// Export Interfaces.
#include "../../Shared/Interfaces/IClientGameExports.h"

// Predeclare.
class ClientGameCore;
class ClientGameEntities;
class ClientGameMedia;
class ClientGameMovement;
class ClientGamePrediction;
class ClientGameScreen;
class ClientGameServerMessage;
class ClientGameView;

/****
*   @brief      Main client game exports interface implementation.
*               Contains accessor functions to acquire the other interfaces.
****/
class ClientGameExports : public IClientGameExports {
public:
    //! Constructor
    ClientGameExports();
    virtual ~ClientGameExports();
    
    /****
    * 
    *   General.
    * 
    ****/
    /**
    *   @brief  Calculates the FOV the client is running. (Important to have in order.)
    **/
    float ClientCalculateFieldOfView(float fieldOfViewX, float width, float height) final;

    /**
    *   @brief  Called when a demo is being seeked through.
    **/
    void DemoSeek() final;

#ifdef _DEBUG
    /**
    *   @brief  For debugging problems when out-of-date entity origin is referenced.
    **/
    void CheckEntityPresent(int32_t entityNumber, const std::string &what) final;
#endif


    /****
    * 
    *   Frame & State related
    * 
    ****/
	/**
	*   @brief  Called right after connecting to a (loopback-)server and succesfully 
	*			loaded up the BSP map data. This gives it a chance to initialize game objects.
	**/
	void ClientConnect() final;
    /**
    *   @brief  Called after all downloads are done. (Aka, a map has started.)
    *           Not used for demos.
    **/
    void ClientBegin() final;
    /**
    *   @brief  Called upon whenever a client disconnects, for whichever reason.
    *           Could be him quiting, or pinging out etc.
    **/
    void ClientClearState() final;
	/**
    *   @brief  Called each client frame. Handle per frame basis things here.
    **/
    void ClientFrame() final;
	/**
    *   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
    **/
    void ClientPacketEntityDeltaFrame() final;
	/**
	*   @brief  Gives Local Entities a chance to think. Called synchroniously to the server frames.
	**/
	void ClientLocalEntitiesFrame() final;
	/**
	*	@return	The GameEntity's hashed classname value, 0 if it has no GameEntity.
	**/
	uint32_t GetHashedGameEntityClassname(PODEntity *podEntity) final;
    /**
    *   @brief  Called for each prediction frame, so all entities can try and predict like the player does.
    **/
    void ClientPredictEntitiesFrame();
    /**
    *   @brief  Called when a disconnect even occures. Including those for Com_Error
    **/
    void ClientDisconnect() final;



    /****
    *
    *   Update Related.
    * 
    ****/
    /**
    *   @brief  Updates the origin. (Used by the engine for determining current audio position too.)
    **/
    void ClientUpdateOrigin() final;
    /**
    *   @brief  Called when there is a needed retransmit of user info variables.
    **/
    void ClientUpdateUserinfo(cvar_t* var, from_t from) final;


    /****
    * 
    *   Interface Accessors.
    * 
    ****/
    /**
    *   @return A pointer to the client game's core interface.
    **/
    IClientGameExportCore *GetCoreInterface() final;

    /**
    *   @return A pointer to the client game module's entities interface.
    **/
    IClientGameExportEntities *GetEntityInterface() final;

    /**
    *   @return A pointer to the client game module's media interface.
    **/
    IClientGameExportMedia *GetMediaInterface() final;

    /**
    *   @return A pointer to the client game module's movement interface.
    **/
    IClientGameExportMovement *GetMovementInterface() final;

    /**
    *   @return A pointer to the client game module's prediction interface.
    **/
    IClientGameExportPrediction *GetPredictionInterface() final;

    /**
    *   @return A pointer to the client game module's screen interface.
    **/
    IClientGameExportScreen *GetScreenInterface() final;

    /**
    *   @return A pointer to the client game module's servermessage interface.
    **/
    IClientGameExportServerMessage *GetServerMessageInterface() final;

    /**
    *   @return A pointer to the client game module's view interface.
    **/
    IClientGameExportView *GetViewInterface() final;

private:
    /**
    *   @brief  Utility function for ClientUpdateOrigin
    **/
    float LerpFieldOfView(float oldFieldOfView, float newFieldOfView, float lerp);

public:
    /***
    *
    *   Client Game Interfaces Pointers.
    *
    ***/
	ClientGameCore* core;
    ClientGameEntities* entities;
    ClientGameMedia* media;
    ClientGameMovement* movement;
    ClientGamePrediction* prediction;
    ClientGameScreen* screen;
    ClientGameServerMessage* serverMessage;
    ClientGameView* view;
};

