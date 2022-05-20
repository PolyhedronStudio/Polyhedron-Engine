/***
*
*	License here.
*
*	@file
*
*	Client Base Player Entity.
* 
***/
#pragma once

// Client Game GameEntity Interface.
#include "CLGBasePacketEntity.h"



/**
*   CLGBasePlayer
**/
class CLGBasePlayer : public CLGBasePacketEntity {
public:
    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    CLGBasePlayer(PODEntity *podEntity);
    virtual ~CLGBasePlayer() = default;

    // Runtime type information
	DefineGameClass( CLGBasePlayer, CLGBasePacketEntity);
	//DefineClass( CLGBasePacketEntity, IClientGameEntity);
	//DefineMapClass( "CLGBasePacketEntity", CLGBaseLocalEntity, IClientGameEntity);
	//DefineGameClass( CLGBasePacketEntity, IClientGameEntity);
//	DefineGameClass( "SVGBasePlayer", CLGBasePlayer, CLGBasePacketEntity);


    /**
    *
    *   ClientGame Entity Interface Functions.
    *
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() override;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() override;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() override;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() override;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.


	/***
    *
    * 
    *   OnEventCallbacks.
    *
    * 
    ***/
    /**
    *   @brief  Gets called right before the moment of deallocation happens.
    **/
    virtual void OnDeallocate() override;

    /***
    *
    * 
    *   ClientGame BaseEntity Functions.
    *
    * 
    ***/

    /**
    *   @brief  Updates the entity with the data of the newly passed EntityState object.
    **/
    virtual void UpdateFromState(const EntityState &state) override;


    /**
    *   @returen True if the entity is still in the current frame.
    **/
    //virtual const qboolean  IsInUse() final;

	/**
	*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
	**/
	virtual void PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) override;






    /**
    *   @brief Get: Entity Client
    **/
    virtual gclient_s* GetClient() override { return &dummyClient; };

protected:
	/**
	*	Temporary Dummy Client.
	**/
	ServerClient dummyClient;

private:

public:

};