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
    // Define as abstract class in our type system.
    //
    DefineAbstractClass("DeathmatchGamemode", DeathmatchGamemode);

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    // DeathMatch unique function implementations.
    virtual void PutClientInServer(Entity* ent) override;
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    virtual void ClientBegin(Entity* serverEntity) override;
    virtual void ClientBeginServerFrame(SVGBaseEntity* entity, ServerClient *client) override;
    virtual void ClientUserinfoChanged(Entity* ent, char* userinfo) override;

    virtual void RespawnClient(PlayerClient* ent) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(PlayerClient* clientEntity) override;

    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

private:
    virtual void RespawnSpectator(PlayerClient* ent);
};