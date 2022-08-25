/***
*
*	License here.
*
*	@file
*
*	Gamemode Interface: Do not inherit, use DefaultGamemode instead to have most
*	functions implemented with a base code.
* 
***/
#pragma once



#include "IGamemode.h"

class DefaultGameMode : public IGamemode {
public:
    //! Constructor/Deconstructor.
    DefaultGameMode();
    virtual ~DefaultGameMode() override;

    /**
    *	Map related.
    **/
    /**
    *   @brief  
    **/
    virtual void OnLevelExit() override;


    /**
    *   @brief  
    **/
    virtual qboolean ClientConnect( PODEntity *podEntity, char *userinfo ) override;

    /**
    *   @brief  
    **/
    virtual void ClientBegin( PODEntity *podEntity ) override;

    /**
    *   @brief  
    **/
    virtual void ClientBeginLocalFrame( CLGBasePlayer *player, ServerClient *client ) override;

    /**
    *   @brief  
    **/
    virtual void ClientEndLocalFrame( CLGBasePlayer *player, ServerClient *client ) override;

    /**
    *   @brief  
    **/
    virtual void ClientThink( CLGBasePlayer *player, ServerClient *client, ClientMoveCommand *moveCommand ) override;

    /**
    *   @brief  
    **/
    virtual void ClientDisconnect( CLGBasePlayer *player, ServerClient *client ) override;

    /**
    *   @brief  
    **/
    virtual void ClientDeath( CLGBasePlayer *player) override;

    /**
    *   @brief  
    **/
    virtual void InitializePlayerPersistentData( ServerClient *client) override;

    /**
    *   @brief  
    **/
    virtual void InitializePlayerRespawnData(ServerClient *client) override;
};