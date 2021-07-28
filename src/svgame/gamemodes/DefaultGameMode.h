/*
// LICENSE HERE.

//
// DefaultGameMode.h
//
// Default game mode to run, allows for all sorts of stuff.
//
*/
#ifndef __SVGAME_GAMEMODES_DEFAULTGAMEMODE_H__
#define __SVGAME_GAMEMODES_DEFAULTGAMEMODE_H__

#include "IGameMode.h"

class DefaultGameMode : public IGameMode {
public:
    // Constructor/Deconstructor.
    DefaultGameMode();
    virtual ~DefaultGameMode() override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual void OnLevelExit() override;

    //
    // Combat GameRules checks.
    //
    virtual qboolean GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) override;
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    virtual BaseEntityVector FindBaseEnitiesWithinRadius(const vec3_t& origin, float radius, uint32_t excludeSolidFlags) override;

    //
    // Combat GameMode actions.
    //
    virtual void EntityKilled(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int32_t damage, vec3_t point) override;
    virtual void InflictDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t dflags, int32_t mod) override;
    virtual void SetCurrentMeansOfDeath(int32_t meansOfDeath) override;
    virtual const int32_t& GetCurrentMeansOfDeath() override;

    //
    // Random Gameplay Functions.
    //
    virtual void SpawnClientCorpse(SVGBaseEntity* ent) override;
    virtual void SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) override;
    virtual vec3_t CalculateDamageVelocity(int32_t damage) override;
    
    //
    // Client related callbacks.
    // 
    virtual qboolean ClientCanConnect(Entity* serverEntity, char* userInfo) override;
    virtual void ClientConnect(Entity* serverEntity) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(PlayerClient* ent) override;
    virtual void ClientDisconnect(PlayerClient* ent) override;
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

    //
    // Client related functions/utilities.
    // 
    virtual void PutClientInServer(PlayerClient* ent) override;

private:

};

#endif // __SVGAME_GAMEMODES_DEFAULTGAMEMODE_H__