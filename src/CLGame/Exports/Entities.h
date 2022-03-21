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

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

// Base Entity.
#include "../Entities/Base/CLGBaseEntity.h"

// Entity List.
#include "../Entities/EntityList.h"

////-----------------------
//// Client base class entity.
////------------------------
//class CLGBaseEntity {
//public:
//    //! Constructor/Destructor.
//    CLGBaseEntity() = default;
//    virtual ~CLGBaseEntity() = default;
//
//    //! Sets the classname of this entity.
//    inline void SetClassname(const std::string& classname) {
//        this->classname = classname;
//    }
//
//    //! Get the classname.
//    inline const std::string& GetClassname() {
//        return classname;
//    }
//
//    // Sets the entity ID
//    inline void SetEntityID(int32_t id) {
//        entityID = id;
//    }
//
//    // Gets the entity ID
//    inline int32_t GetEntityID() {
//        return entityID;
//    }
//
//private:
//    // Entity ID.
//    int32_t entityID = 0;
//
//    // Classname.
//    std::string classname = "";
//};


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
    *   @brief  Emplaces, or spawn anew, an entity from the entity state.
    **/
    qboolean SpawnFromState(ClientEntity *clEntity, const EntityState &state);

    /**
    *   @brief Executed whenever an entity event is receieved.
    **/
    void Event(int32_t number) final;

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


//! Entity Parsing utilities.
private:
    /**
    *	@brief	Parses the BSP Entity string and places the results in the client
    *			entity dictionary.
    **/
    qboolean ParseEntityString(const char** data, ClientEntity* clEntity);

    /**
    *   @brief  Allocates the class entity determined by the classname key, and
    *           then does a precache before spawning the class entity.
    **/
    qboolean SpawnParsedClassEntity(ClientEntity* clEntity);

    /**
    *	@brief	Seeks through the type info system for a class registered under the classname string.
    *			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
    *			try and allocate it.
    *	@return	nullptr in case of failure, a valid pointer to a class entity otherwise.
    **/
    CLGBaseEntity *AllocateClassEntity(ClientEntity* clEntity, const std::string &classname);

//! Class Entity utilities.
private:
    EntityList entityList;

//! Entity Rendering utilities.
private:
    /**
	*   @brief  Gives the opportunity to adjust render effects where desired.
    **/
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

