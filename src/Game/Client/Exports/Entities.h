/***
*
*	License here.
*
*	@file
*
*	Client Game Entities Interface Implementation.
* 
***/
#pragma once

// Base Entity.
#include "../Entities/Base/CLGBaseEntity.h"

// Entity List.
#include "../Entities/GameEntityList.h"



//---------------------------------------------------------------------
// Client Game Entities IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameEntities : public IClientGameExportEntities {
public:
    /**
    *   @brief  Parses and spawns the local class entities in the BSP Entity String.
    * 
    *   @details    When a class isn't locally registered, it'll automatically spawn
    *               a CLGBaseEntity instead which has all the default behaviors that
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
    qboolean UpdateFromState(PODEntity *clEntity, const EntityState &state) final;

    /**
    *   @brief  Executed whenever a server frame entity event is receieved.
    **/
    void ServerEntityEvent(int32_t number) final;

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
        return gameEntityList.GetGameEntities();
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
    qboolean SpawnParsedGameEntity(PODEntity* podEntity);

//! Game Entity utilities.
private:
    GameEntityList gameEntityList;

//! Entity Rendering utilities.
private:
    /**
	*   @brief  Gives the opportunity to adjust render effects where desired.
    **/
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

