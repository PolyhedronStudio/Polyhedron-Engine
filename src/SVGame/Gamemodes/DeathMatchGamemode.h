/*
// LICENSE HERE.

//
// DeathmatchGamemode.h
//
// Deathmatch game mode.
//
*/
#pragma once

#include "IGamemode.h"
#include "DefaultGamemode.h"

class DeathmatchGamemode : public DefaultGamemode {
public:
    //
    // Constructor/Deconstructor.
    //
    DeathmatchGamemode();
    virtual ~DeathmatchGamemode() override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    // DeathMatch unique function implementations.
    virtual void PlacePlayerInGame(SVGBasePlayer* player) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) override;
    virtual void ClientUserinfoChanged(Entity* ent, char* userinfo) override;

    virtual void RespawnClient(SVGBasePlayer* player) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(SVGBasePlayer* player) override;

    virtual void ClientUpdateObituary(SVGBaseEntity* player, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

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