/***
*
*	License here.
*
*	@file
*
*	Client Game MonsterTestDummy.
* 
***/
#pragma once

class MonsterTestDummy : public CLGBaseEntity {
public:
    //! Constructor/Deconstructor.
    MonsterTestDummy(Entity* svEntity);
    virtual ~MonsterTestDummy() = default;

    DefineMapClass("monster_testdummy", MonsterTestDummy, CLGBaseEntity);


    /**
    *
    *   Client Class Entity Interface Functions.
    *
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() final;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() final;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() final;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() final;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() final;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) final; // Called for each key:value when parsing the entity dictionary.



    /***
    *
    *   Client Class Entity Functions.
    *
    ***/


    /***
    *
    *   OnEventCallbacks.
    *
    ***/
    /**
    *   @brief  Gets called right before the moment of deallocation happens.
    **/
    virtual void OnDeallocate() final;

};