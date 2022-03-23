/***
*
*	License here.
*
*	@file
*
*	SharedGame ClassEntity Interface.
* 
***/
#pragma once

struct ClientEntity;
enum from_t;
// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"


class ISharedGameEntity {
public:
    // Runtime type information
    DefineTopAbstractClass( ISharedGameEntity );

    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    virtual ~ISharedGameEntity() = default;


    /**
    *
    *   SharedGame ClassEntity Interface Functions.
    * 
    *   These MUST BE IMPLEMENTED by any eventual "Base" entity derived from this interface.
    *
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() = 0;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() = 0;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() = 0;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() = 0;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() = 0;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) = 0; // Called for each key:value when parsing the entity dictionary.


    /**
    *
    *   The following functions can be left empty and only demand a "stub" to be implemented for them.
    *
    **/


private:

};