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

// Entity List.
#include "../Entities/GameEntityList.h"

// World.
#include "../World/ClientGameWorld.h"

//---------------------------------------------------------------------
// Client Game Entities IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameEntities : public IClientGameExportEntities {
public:
    /**
    *   @brief  Parses and spawns the local class entities in the BSP Entity String.
    * 
    *   @details    When a class isn't locally registered, it'll automatically spawn
    *               a CLGBasePacketEntity instead which has all the default behaviors that
    *               you'd expect for it to be functional.
    * 
    *   @return True on success.
    **/
    qboolean SpawnFromBSPString(const char* entities) final;

    /**
    *   @brief  When the client receives state updates it calls into this function so we can update
    *           the game entity belonging to the server side entity(defined by state.number).
    * 
    *           If the hashed classname differs, we allocate a new one instead. Also we ensure to 
    *           always update its PODEntity pointer to the appropriate new one instead.
    * 
    *   @return True on success, false in case of trouble. (Should never happen, and if it does,
    *           well... file an issue lmao.)
    **/
    qboolean UpdateGameEntityFromState(PODEntity *clEntity, const EntityState &state) final;

    /**
    *   @brief  Executed whenever a server frame entity event is receieved.
    **/
    void PacketEntityEvent(int32_t number) final;

    /**
    *   @brief  Executed whenever a local client entity event is set.
    **/
    void LocalEntityEvent(int32_t number) final;

    /**
    *   @brief  Parse the server frame for server entities to add to our client view.
    *           Also applies special rendering effects to them where desired.
    **/
    void AddPacketEntities() final;

    /**
    *   @brief  Add the view weapon render entity to the screen. Can also be used for
    *           other scenarios where a depth hack is required.
    **/
    void AddViewEntities() final;

    /**
    *   @brief  Runs the client game module's entity logic for a single frame.
    **/
    void RunFrame();


    inline CLGEntityVector &GetGameEntities() {
		ClientGameWorld *gameWorld = GetGameWorld();
		if (gameWorld) {
			return gameWorld->GetGameEntities();
		} else {
			// This should never happen but... yeah...
			
			//return nullptr;
			return nullGameEntities;
		}
    }

//! Entity Parsing utilities.
private:
    /**
    *	@brief	Parses the BSP Entity string and places the results in the client
    *			entity dictionary.
    **/
    qboolean ParseEntityString(const char** data, EntityDictionary &parsedKeyValues);// PODEntity* podEntity);

    /**
    *   @brief  Allocates the game entity determined by the classname key, and
    *           then does a precache before spawning the game entity.
    **/
    qboolean CreateGameEntityFromDictionary(PODEntity* podEntity, EntityDictionary &dictionary);

//! Game Entity utilities.
private:
    GameEntityList gameEntityList;
	GameEntityVector nullGameEntities;

//! Entity Rendering utilities.
private:
    /**
	*   @brief  Gives the opportunity to adjust render effects where desired.
    **/
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

