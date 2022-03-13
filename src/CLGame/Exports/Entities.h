// License here.
// 
//
// ClientGameEntities implementation.
#pragma once

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

//-----------------------
// Client base class entity.
//------------------------
class CLGBaseEntity {
public:
    //! Constructor/Destructor.
    CLGBaseEntity() = default;
    virtual ~CLGBaseEntity() = default;

    //! Sets the classname of this entity.
    inline void SetClassname(const std::string& classname) {
        this->classname = classname;
    }

    //! Get the classname.
    inline const std::string& GetClassname() {
        return classname;
    }

    // Sets the entity ID
    inline void SetEntityID(int32_t id) {
        entityID = id;
    }

    // Gets the entity ID
    inline int32_t GetEntityID() {
        return entityID;
    }

private:
    // Entity ID.
    int32_t entityID = 0;

    // Classname.
    std::string classname = "";
};


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
    qboolean SpawnClassEntities(const char* entities) final;

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
    * Add the view weapon render entity to the screen. Can also be used for
    * other scenarios where a depth hack is required.
    **/
    void AddViewEntities() final;


//! Entity Parsing utilities.
private:
    /**
    *	@brief	Parses the BSP Entity string and places the results in the server
    *			entity dictionary.
    **/
    qboolean ParseEntityString(const char** data, ClientEntity* clEntity);


//! Class Entity utilities.
private:
    //ClassEntityList 

//! Entity Rendering utilities.
private:
    /**
	*   @brief  Gives the opportunity to adjust render effects where desired.
    **/
	int32_t ApplyRenderEffects(int32_t renderEffects);
};

