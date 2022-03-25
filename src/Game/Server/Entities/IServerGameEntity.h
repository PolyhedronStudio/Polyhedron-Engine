/***
*
*	License here.
*
*	@file
*
*	ServerGame Entity Interface.
* 
***/
#pragma once

// ServerGame Exports Interface. TODO: Obviously, still implement.
//#include "Shared/Interfaces/IServerGameExports.h"

// GameLocals.
#include "../ServerGameLocals.h"
// SharedGame Entity Interface.
#include "../../Shared/SharedGame.h"




/**
*   IClientGameEntity
**/
class IServerGameEntity : public ISharedGameEntity {
public:
    /**
    *
    * 
    *   Constructor/Destructor AND TypeInfo related.
    *
    * 
    **/
    //! Constructor/Destructor.
    virtual ~IServerGameEntity() = default;

    //! Runtime type information
    DefineAbstractClass( IServerGameEntity, ISharedGameEntity );

    

    /**
    *
    * 
    *   SharedGame ClassEntity Interface Functions.
    *
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
    *   ServerGame Class Entity Functions.
    * 
    *   These functions are all implemented in CLGBaseEntity, some are purposely left empty to act
    *   as a stub. Just because they are empty doesn't mean one can't fill them in and make them tick.
    *   It does require one to have a proper understanding of the matter before doing so. Breaking things
    *   by doing so is easy.
    *
    ***/



private:

};