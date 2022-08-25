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


/**
*	Predeclarations.
**/
class CLGBaseEntity;
class CLGBasePlayer;

//using GameEntityVector = std::vector<IServerGameEntity*>;

class IGamemode {
public:
	//! Constructor/Destructor.
    IGamemode() {};
    virtual ~IGamemode() = default;


	/***
	*
	*
	*	GameMode specific class checking. Best practice is to try and write code
    *	that does not depend on checking a game mode class type too much.
    *
    *	Instead try to facilitate the game mode itself instead where possible.
	*
	*
	***/
    /**
    *   @brief  Checks if this gamemode class is exactly the given class.
    *   @param  gamemodeClass A gamemode class which must inherint from IGamemode.
    *   @return True if the game mode class is the same class type or a derivate of gamemodeClass.
    **/
    template<typename gamemodeClass>
    bool IsClass() const {
	    return typeid(*this) == typeid(gamemodeClass);
    }
    
    /**
    *   @brief  Checks if this gamemode class is a subclass of another, or is the same class
    *   @param  gamemodeClass A gamemode class which must inherint from IGamemode.
    *   @return True if the game mode class is the same, or a derivate of gamemodeClass.
    **/
    template<typename gamemodeClass>
    bool IsSubclassOf() const {
	    return dynamic_cast<gamemodeClass>(*this) != nullptr;
    }


    /**
    *	Map related.
    **/
    /**
    *   @brief  
    **/
    virtual void OnLevelExit() = 0;


    /**
    *   @brief  
    **/
    virtual qboolean ClientConnect( PODEntity *podEntity, char *userinfo ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientBegin( PODEntity *podEntity ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientBeginLocalFrame( CLGBasePlayer *player, ServerClient *client ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientEndLocalFrame( CLGBasePlayer *player, ServerClient *client ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientThink( CLGBasePlayer *player, ServerClient *client, ClientMoveCommand *moveCommand ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientDisconnect( CLGBasePlayer *player, ServerClient *client ) = 0;

    /**
    *   @brief  
    **/
    virtual void ClientDeath( CLGBasePlayer *player) = 0;

    /**
    *   @brief  
    **/
    virtual void InitializePlayerPersistentData( ServerClient *client) = 0;

    /**
    *   @brief  
    **/
    virtual void InitializePlayerRespawnData(ServerClient *client) = 0;
};
