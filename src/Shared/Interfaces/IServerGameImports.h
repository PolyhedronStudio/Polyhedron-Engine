/***
*
*	License here.
*
*	@file
*
*	ServerGame Imports Interface.
* 
***/
#pragma once

// Predeclerations.
//struct PlayerMove;


/**
*
*   Core ServerGame Imports Interface.
* 
**/
class IServerGameImportCore {
public:
    //! Destructor.
    virtual ~IServerGameImportCore() = default;
    

	///**
	//*   @brief  Initializes the ServerGame module.
	//*/
	//virtual void Initialize() = 0;

	///**
	//*   @brief  Shuts down the ServerGame module.
	//*/
	//virtual void Shutdown() = 0;
};


/**
*
*   Media ServerGame Imports Interface.
* 
**/
class IServerGameImportMedia {
public:
    //! Destructor.
    virtual ~IServerGameImportMedia() = default;
    

	/**
	*   @brief	'Precaches' the specified image file by adding it to the configstrings.
	*	@return	The configstrings image identifier index.
	**/
	virtual void PrecacheSound( const std::string &filename ) = 0;
	/**
	*   @brief	'Precaches' the specified model file by adding it to the configstrings.
	*	@return	The configstrings model identifier index.
	**/
	virtual void PrecacheModel( const std::string &filename ) = 0;
	/**
	*   @brief	'Precaches' the specified sound file by adding it to the configstrings.
	*	@return	The configstrings sound identifier index.
	**/
	virtual void PrecacheSound( const std::string &filename ) = 0;


	/**
	*   @brief  Shuts down the ServerGame module.
	*/
	virtual void Shutdown() = 0;
};


/**
*
*   Entities ServerGame Imports Interface.
* 
**/
class IServerGameImportEntities {
public:
    //! Destructor.
    virtual ~IServerGameImportEntities() = default;
    

	/**
	*	@brief	'Links' the entity as a possible subject for collision.
	*			(in other words: enables traces to test against this entity.
	*			
	*			If it is NOT linked in, it will NOT be sent to a client OR
	*			be tested by traces.
	*
	*			This means that if an entity its size, position, or solidity
	*			has changed that the entity needs to be relinked in order to
	*			prevent odd behaviors.
	**/
    virtual void LinkEntity( Entity *entity ) = 0;
	/**
	*	@brief	'Unlinks' the entity for collision testing. Does not stop the
	*			entity from being rendered at all.
	**/
    virtual void UnlinkEntity( Entity *entity ) = 0;     // call before removing an interactive edict
	/**
	*   @brief	Generates a list of entities that have their bounding box residing in,
	*			or intersecting with the mins/maxs box.
	*	@param	maxcount	Maximum number of entities that the passed list can contain.
	*	@param	**list		A pointer to an array of maxcount size to store all positive tested entities in.
	*	@return	The number of entities contained inside the list.
	**/
	virtual int32_t BoxEntities( const vec3_t &mins, const vec3_t &maxs, Entity **list, int maxcount, int areatype ) = 0;
	/**
	*   @brief	Checks if the entity bounding boxes intersect, if they do, they touch.
	*	@return	True if the bounding boxes intersect. False otherwise.
	**/
	virtual int32_t ContactEntities( Entity *entityA, Entity* entityB ) = 0;
};


/**
*
*   Messaging ServerGame Imports Interface.
* 
**/
class IServerGameImportMessaging {
public:
    //! Destructor.
    virtual ~IServerGameImportMessaging() = default;
    

	/**
	*	@brief	Sends the current message contents to all connected clients that
	*			have any notion of the PHS/PVS that 'origin' takes place at.
	**/
    virtual void Multicast( const vec3_t origin, const int32_t toFlags ) = 0;
	/**
	*	@brief	Sends the current message to the connected entity client.
	*			Optionable reliability.
	**/
	virtual void Unicast( Entity *entity, bool sendAsReliable ) = 0;
};


/**
*
*   Main ServerGame Imports Interface.
* 
**/
class IServerGameImports {
public:
    //! Default destructor.
    virtual ~IServerGameImports() = default;


    /***
    *
    *   Interface Accessors.
    *
    ***/
    /**
    *   @return A pointer to the server game's core interface.
    **/
    virtual IServerGameImportCore *GetCoreInterface() = 0;

	/**
    *   @return A pointer to the server game module's entity interface.
    **/
    virtual IServerGameImportEntities *GetCollisionModelInterface() = 0;

	/**
    *   @return A pointer to the server game module's entity interface.
    **/
    virtual IServerGameImportEntities *GetEntityInterface() = 0;

    /**
    *   @return A pointer to the server game module's media interface.
    **/
    virtual IServerGameImportMedia *GetMediaInterface() = 0;

	/**
    *   @return A pointer to the server game module's message interface.
    **/
    virtual IServerGameImportSound *GetMessageInterface() = 0;

	/**
    *   @return A pointer to the server game module's sound interface.
    **/
    virtual IServerGameImportSound *GetSoundInterface() = 0;
    


    /***
    *
    *   General.
    *
    ***/
    /**
    *   @brief  Processes the ServerGame logic for a single frame.
    **/
    virtual void RunFrame() = 0;

    /**
    *   @brief  Called when an "sv <command>" command is issued throughout the server console.
    *           argc and argv can be used to get the rest of the parameters.
    **/
    virtual void ServerCommand() = 0;
};

