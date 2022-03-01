/*
// LICENSE HERE.

//
// DefaultGamemode.h
//
// Default game mode to run, allows for all sorts of stuff.
//
*/
#pragma once

#include "IGamemode.h"

class DefaultGamemode : public IGamemode {
public:
    //! Constructor/Deconstructor.
    DefaultGamemode();
    virtual ~DefaultGamemode() override;

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
    virtual qboolean IsDeadEntity(SVGBaseEntity *entity) override;
    virtual qboolean GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) override;
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    virtual ClassEntityVector FindBaseEnitiesWithinRadius(const vec3_t& origin, float radius, uint32_t excludeSolidFlags) override;

    /***
    * Combat Gamemode Actions.
    ***/    
    virtual void EntityKilled(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int32_t damage, vec3_t point) override;
    virtual void InflictDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t dflags, int32_t mod) override;
    virtual void InflictRadiusDamage(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, float damage, SVGBaseEntity* ignore, float radius, int32_t mod) override;
    virtual void SetCurrentMeansOfDeath(int32_t meansOfDeath) override;
    virtual const int32_t& GetCurrentMeansOfDeath() override;

    /***
    * Random Gameplay Functions.
    ***/
    virtual void SpawnClientCorpse(SVGBaseEntity* ent) override;
    virtual void SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) override;
    virtual vec3_t CalculateDamageVelocity(int32_t damage) override;
    
    /***
    * Client Hooks.
    ***/
    virtual qboolean ClientConnect(Entity* serverEntity, char *userinfo) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) override;
    virtual void ClientEndServerFrame(SVGBasePlayer *player, ServerClient *client) override;
    virtual void ClientDisconnect(SVGBasePlayer* player, ServerClient *client) override;
    virtual void ClientUserinfoChanged(Entity* ent, char *userinfo) override;
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

    /***
    * Client Related Functions.
    ***/ 

    virtual void InitializePlayerPersistentData(ServerClient* client) override;
    virtual void InitializePlayerRespawnData(ServerClient *client) override;

    virtual void SelectPlayerSpawnPoint(SVGBasePlayer* player, vec3_t& origin, vec3_t& angles) override;
    virtual void PlacePlayerInGame(SVGBasePlayer* player) override;
    virtual void RespawnClient(SVGBasePlayer* player) override;
    virtual void RespawnAllClients() override;

    virtual void ClientDeath(SVGBasePlayer *player) override;


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
    virtual void RestorePlayerPersistentData(SVGBaseEntity* player, ServerClient* client) override;

protected:
    /**
    *   @brief  Sets client into intermission mode by setting movetype to freeze
    *           and positioning the client at the intermission point.
    **/
    virtual void StartClientIntermission(SVGBasePlayer* player, ServerClient* client);
};