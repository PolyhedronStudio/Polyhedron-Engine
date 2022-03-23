/***
*
*	License here.
*
*	@file
*
*	ClientGame Entity Interface.
* 
***/
#pragma once

// ClientGame Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

// SharedGame Entity Interface.
#include "SharedGame/Entities/ISharedGameEntity.h"




/**
*   IClientGameEntity
**/
class IClientGameEntity : public ISharedGameEntity {
public:
    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    virtual ~IClientGameEntity() = default;

    //! Runtime type information
    DefineAbstractClass( IClientGameEntity, ISharedGameEntity );

    

    /**
    *
    *   SharedGame ClassEntity Interface Functions.
    *
    **/
    ///**
    //*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    //**/
    //virtual void Precache() = 0;    // Precaches data.
    ///**
    //*   @brief  Called when it is time to spawn this entity.
    //**/
    //virtual void Spawn() = 0;       // Spawns the entity.
    ///**
    //*   @brief  Called when it is time to respawn this entity.
    //**/
    //virtual void Respawn() = 0;     // Respawns the entity.
    ///**
    //*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    //**/
    //virtual void PostSpawn() = 0;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    ///**
    //*   @brief  General entity thinking routine.
    //**/
    //virtual void Think() = 0;

    ///**
    //*   @brief  Act upon the parsed key and value.
    //**/
    //virtual void SpawnKey(const std::string& key, const std::string& value) = 0; // Called for each key:value when parsing the entity dictionary.



    /***
    *
    *   ClientGame Class Entity Functions.
    * 
    *   These functions are all implemented in CLGBaseEntity, some are purposely left empty to act
    *   as a stub. Just because they are empty doesn't mean one can't fill them in and make them tick.
    *   It does require one to have a proper understanding of the matter before doing so. Breaking things
    *   by doing so is easy.
    *
    ***/
    /**
    *   @brief  Updates the entity with the data of the newly passed EntityState object.
    **/
    virtual void UpdateFromState(const EntityState &state) = 0;

    /**
    *   @return A reference to the current state object.
    **/
    virtual const EntityState& GetCurrentEntityState() = 0;

    /**
    *   @brief  Sets the classname of this entity.
    **/
    virtual void SetClassname(const std::string& classname) = 0;

    /**
    *   @return A string containing the entity's classname.
    **/
    virtual const std::string GetClassname() = 0;
    /**
    *   @return An uint32_t containing the hashed classname string.
    **/
    virtual uint32_t GetHashedClassname() = 0;

    /**
    *   @brief  Sets a pointer referring to this class' client entity.
    **/
    virtual void SetClientEntity(ClientEntity* clEntity) = 0;

    /**
    *   @return The pointer referring to this class' client entity.
    **/
    virtual ClientEntity* GetClientEntity() = 0;


    /***
    *
    *   OnEventCallbacks.
    *
    ***/
    /**
    *   @brief  Gets called right before the moment of deallocation happens.
    **/
    virtual void OnDeallocate() = 0;

private:
    //! Pointer to the client entity which owns this class entity.
    ClientEntity *clientEntity = nullptr;

    //! Client Class Entities maintain their own states. (Get copied in from updates.)
    EntityState currentState = {};
    EntityState previousState = {};
};