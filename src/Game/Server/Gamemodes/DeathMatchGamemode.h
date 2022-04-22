/*
// LICENSE HERE.

//
// DeathmatchGameMode.h
//
// Deathmatch game mode.
//
*/
#pragma once

#include "IGameMode.h"
#include "DefaultGameMode.h"

class DeathmatchGameMode : public DefaultGameMode {
public:
    //
    // Constructor/Deconstructor.
    //
    DeathmatchGameMode();
    virtual ~DeathmatchGameMode() override;

    //
    // Server Related.
    //
    virtual qboolean CanSaveGame(qboolean isDedicatedServer) override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    // DeathMatch unique function implementations.
    virtual void PlacePlayerInGame(SVGBasePlayer* player) override;
    virtual qboolean CanDamage(IServerGameEntity* targ, IServerGameEntity* inflictor) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) override;
    virtual void ClientUserinfoChanged(Entity* ent, char* userinfo) override;

    virtual void RespawnClient(SVGBasePlayer* player) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(SVGBasePlayer* player) override;

    virtual void ClientUpdateObituary(IServerGameEntity* player, IServerGameEntity* inflictor, IServerGameEntity* attacker) override;

private:
    /**
    *   @brief  Respawn given player as a spectator.
    **/
    virtual void RespawnSpectator(SVGBasePlayer* player, ServerClient* client);

    /**
    *   @brief  Sets client into intermission mode like default game mode but
    *           also generates a scoreboard display.
    **/
    virtual void StartClientIntermission(SVGBasePlayer* player, ServerClient* client) override;
};