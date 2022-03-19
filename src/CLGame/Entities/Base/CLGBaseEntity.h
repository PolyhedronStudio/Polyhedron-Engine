/***
*
*	License here.
*
*	@file
*
*	Client Game BaseEntity.
* 
***/
#pragma once

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"



class CLGBaseEntity {
public:
    //! Constructor/Destructor.
    CLGBaseEntity() = default;
    virtual ~CLGBaseEntity() = default;

    //! Sets the classname of this entity.
    inline virtual void SetClassname(const std::string& classname) {
        this->classname = classname;
    }

    //! Get the classname.
    inline const std::string& GetClassname() {
        return classname;
    }

    /**
    *   @brief
    **/
    inline void SetClientEntity(ClientEntity* clEntity) {
        clientEntity = clEntity;
    }
    /**
    *   @brief
    **/
    inline ClientEntity* GetClientEntity() {
        return clientEntity;
    }

    /**
    *   @brief
    **/

    /**
    *   @brief
    **/

    /**
    *   @brief  Only sets the entity ID if it has a valid clientEntity pointer to set it on.
    **/
    inline void SetEntityID(int32_t id) {
        if (clientEntity) {
            clientEntity->id = id;
        }
    }
    /**
    *   @return Returns the clientEntity id when its a valid pointer. World(0), otherwise.
    **/
    inline int32_t GetEntityID() {
        if (clientEntity) {
            return clientEntity->id;
        }

        return 0; // None, or world for that matter.
    }

private:
    // Classname.
    std::string classname = "";

    //! Pointer to the client entity which owns this class entity.
    ClientEntity    *clientEntity = nullptr;
};