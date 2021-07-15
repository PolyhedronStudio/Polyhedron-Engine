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

    virtual qboolean GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) override;
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
   
    virtual void SpawnClientCorpse(SVGBaseEntity* ent) override;
    virtual void SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) override;
    virtual vec3_t CalculateDamageVelocity(int32_t damage) override;
    
    virtual qboolean ClientCanConnect(Entity* serverEntity, char* userInfo) override;
    virtual void ClientConnect(Entity* serverEntity) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(PlayerClient* ent) override;
    virtual void ClientDisconnect(PlayerClient* ent) override;
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

    virtual void PutClientInServer(PlayerClient* ent) override;

    virtual void SetCurrentMeansOfDeath(int32_t meansOfDeath) override;
    virtual const int32_t& GetCurrentMeansOfDeath() override;

private:

};

#endif // __SVGAME_GAMEMODES_DEFAULTGAMEMODE_H__