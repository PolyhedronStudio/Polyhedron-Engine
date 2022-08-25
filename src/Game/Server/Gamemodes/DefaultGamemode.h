/***
*
*	License here.
*
*	@file
*
*	Gamemode Interface: Do not inherit, use DefaultGamemode instead to have most
*	functions implemented with a base code.
* 
***/
#pragma once



#include "IGamemode.h"
 
class DefaultGameMode : public IGamemode {
public:
    //! Constructor/Deconstructor.
    DefaultGameMode();
    virtual ~DefaultGameMode() = default;

    /***
    * Server Related.
    ***/
    virtual qboolean CanSaveGame(qboolean isDedicatedServer) override;


    /***
    * Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    ***/
    virtual void OnLevelExit() override;


    /***
    * Combat GameRules checks.
    ***/
    virtual qboolean IsDeadEntity( SVGBaseEntity *entity ) override;
    virtual qboolean GetEntityTeamName( SVGBaseEntity* ent, std::string &teamName ) override;
    virtual qboolean OnSameTeam( IServerGameEntity* ent1, IServerGameEntity* ent2 ) override;
    virtual qboolean CanDamage( IServerGameEntity* targ, IServerGameEntity* inflictor ) override;
    virtual GameEntityVector FindBaseEnitiesWithinRadius( const vec3_t& origin, float radius, uint32_t excludeSolidFlags ) override;


    /***
    * Combat GameMode Actions.
    ***/    
    virtual void EntityKilled( IServerGameEntity* target, IServerGameEntity* inflictor, IServerGameEntity* attacker, int32_t damage, vec3_t point ) override;
    virtual void InflictDamage( IServerGameEntity* target, IServerGameEntity* inflictor, IServerGameEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t dflags, int32_t mod ) override;
    virtual void InflictRadiusDamage( IServerGameEntity* inflictor, IServerGameEntity* attacker, float damage, IServerGameEntity* ignore, float radius, int32_t mod ) override;
    virtual void SetCurrentMeansOfDeath( int32_t meansOfDeath ) override;
    virtual const int32_t& GetCurrentMeansOfDeath() override;


    /***
    * Random Gameplay Functions.
    ***/
    virtual void SpawnClientCorpse( SVGBaseEntity* ent ) override;
    virtual void SpawnTempDamageEntity( int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage ) override;
    virtual vec3_t CalculateDamageVelocity( int32_t damage ) override;
    

    /***
    * Client Hooks.
    ***/
    virtual qboolean ClientConnect( PODEntity* podEntity, char *userinfo ) override;
    virtual void ClientBegin( PODEntity* podEntity ) override;
    virtual void ClientBeginServerFrame( SVGBasePlayer* player, ServerClient *client ) override;
    virtual void ClientEndServerFrame( SVGBasePlayer *player, ServerClient *client ) override;
    virtual void ClientDisconnect( SVGBasePlayer* player, ServerClient *client ) override;
    virtual void ClientUserinfoChanged( PODEntity* ent, char *userinfo ) override;
    virtual void ClientUpdateObituary( IServerGameEntity* self, IServerGameEntity* inflictor, IServerGameEntity* attacker ) override;

protected:
	/**
	*	@brief	Wrapper PM_Trace to interscept and adjust tracing needs if desired.
	**/
	static TraceResult PM_Trace( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end );

public:
    virtual void ClientThink( SVGBasePlayer *player, ServerClient *client, ClientMoveCommand *moveCommand ) override;


    /***
    * Client Related Functions.
    ***/ 
    virtual void InitializePlayerPersistentData( ServerClient* client ) override;
    virtual void InitializePlayerRespawnData( ServerClient *client ) override;

    virtual void SelectPlayerSpawnPoint( SVGBasePlayer* player, vec3_t& origin, vec3_t& angles ) override;
    virtual void PlacePlayerInGame( SVGBasePlayer* player ) override;
    virtual void RespawnClient( SVGBasePlayer* player ) override;
    virtual void RespawnAllClients() override;

    virtual void ClientDeath( SVGBasePlayer *player ) override;


    /**
    *   @brief Stores player entity data in the client's persistent structure..
    * 
    *   @details    When switching a gamemap, information that should be persistant, like health,
    *               is still stored in the entity. This method mirrors it out to the client structure
    *               before all entities are wiped.
    **/
    virtual void StorePlayerPersistentData(void) override;

    /**
    *   @brief Restores player persistent data from the client struct by assigning it to the player entity.
    **/
    virtual void RestorePlayerPersistentData( SVGBaseEntity* player, ServerClient* client ) override;

protected:
    /**
    *   @brief  Sets a client's button, oldButton, and latched button bits.
    **/
    virtual void SetClientButtonBits( ServerClient *client, ClientMoveCommand *moveCommand );

    /**
    *   @brief  Sets client into intermission mode by setting movetype to freeze
    *           and positioning the client at the intermission point.
    **/
    virtual void StartClientIntermission( SVGBasePlayer* player, ServerClient* client );
};