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
    virtual void PlaceClientInWorld(Entity* ent) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(SVGBasePlayer* entity, ServerClient *client) override;
    virtual void ClientUserinfoChanged(Entity* ent, char* userinfo) override;

    virtual void RespawnClient(SVGBasePlayer* ent) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(SVGBasePlayer* clientEntity) override;

    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

private:
    virtual void RespawnSpectator(SVGBasePlayer* ent);
};