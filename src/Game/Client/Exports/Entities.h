/***
*
*	License here.
*
*	@file
*
*	Client Game Entities Interface Implementation.
*
*	The client knows about 2 specific type of game entities, those that come from the
*	server also known as Packet Entities, and those that are specifically local only 
*	to the client, known as Local Entities. For further information about these look
*	them up in the Entities/Base folder.
* 
***/
#pragma once

// Base Packet Entity.
#include "../Entities/Base/CLGBasePacketEntity.h"

// Base Local Entity.
#include "../Entities/Base/CLGBaseLocalEntity.h"

// World.
#include "../World/ClientGameWorld.h"

//---------------------------------------------------------------------
// Client Game Entities IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameEntities : public IClientGameExportEntities {
public:
    //! Destructor.
    virtual ~ClientGameEntities()  = default;



	/**
	*
	*
	*	Client Local BSP Entities.
    *
	*
	**/
    /**
    *   @brief  Parses and spawns the local class entities in the BSP Entity String.
    * 
    *   @details    When a class isn't locally registered, it'll automatically spawn
    *               a CLGBasePacketEntity instead which has all the default behaviors that
    *               you'd expect for it to be functional.
    * 
    *   @return True on success.
    **/
    qboolean PrepareBSPEntities( const char *mapName, const char* entities ) final;



	/**
	*
	*
	*	'Game' Entities.
    *
	*
	**/
    /**
    *   @brief  When the client receives state updates it calls into this function so we can update
    *           the game entity belonging to the server side entity(defined by state.number).
    * 
    *           If the hashed classname differs, we allocate a new one instead. Also we ensure to 
    *           always update its PODEntity pointer to the appropriate new one instead.
    * 
    *   @return True on success, false in case of trouble.
    **/
    qboolean UpdateGameEntityFromState( PODEntity *clEntity, const EntityState *state ) final;
	/**
	*   @brief  Gives local entities a chance to think. These are called "synchroniously" to the server frames.
	*	@return	The GameEntity's hashed classname value, 0 if it has no GameEntity.
	**/
	uint32_t GetHashedGameEntityClassname(PODEntity *podEntity); 


	/**
	*
	*
	*	Entity Events.
    *
	*
	**/
	/**
    *   @brief  Executed whenever a server frame entity event is receieved.
    **/
    void PacketEntityEvent( int32_t number ) final;
    /**
    *   @brief  Executed whenever a local client entity event is set.
    **/
    void LocalEntityEvent( int32_t number ) final;



	/**
	*
	*
	*	Refresh & View entities.
    *
	*
	**/
    /**
    *   @brief  Prepares all parsed server entities, as well as local entities for rendering
	*			of the current frame.
    **/
    void PrepareRefreshEntities() final;
	/**
	*	@brief	Adds all 'view' entities to screen, the place to hook in entities that require a depth hack.
	**/
    void AddViewEntities() final;



	/**
	*
	*
	*	Run Frames for Packet Entities (Deltaframe), and Local Entities.
    *	(Prediction is placeholder for now.)
	*
	**/
	/**
	*   @brief  Called each VALID client frame. Handle per VALID frame basis things here.
	**/
    void RunPacketEntitiesDeltaFrame();
	/**
	*   @brief  Gives Local Entities a chance to think. Called synchroniously to the server frames.
	**/
	void RunLocalEntitiesFrame();
	/**
	*   @brief  Called for each prediction frame, so all entities can try and predict like the player does.
	**/
	void RunPackEntitiesPredictionFrame();



	/**
	*
	*
	*	Other.
    *	
	*
	**/
	/**
	*	@brief	Returns a pointer to the actual client game POD Entities array residing in the ClientGame's world.
	**/
	PODEntity *GetClientPODEntities() final;
	/**
	*	@return	Pointer to the game entities, if the GameWorld is not yet created, we return a fake placeholder nullGameEntities.
	**/
	inline GameEntityVector &GetGameEntities() {
		ClientGameWorld *gameWorld = GetGameWorld();
		if (gameWorld) {
			return gameWorld->GetGameEntities();
		} else {
			// This should never happen but... yeah...
			
			//return nullptr;
			return nullGameEntities;
		}
    }



//! Game Entity utilities.
private:
	GameEntityVector nullGameEntities;



//! Entity Rendering utilities.
private:
    /**
	*   @brief  Gives the opportunity to adjust render effects where desired.
    **/
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

