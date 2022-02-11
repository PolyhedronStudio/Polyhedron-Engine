/*
// LICENSE HERE.

//
// CoopGamemode.h
//
// Coop game mode to run, same as default mode but with coop rules.
//
*/
#include "IGamemode.h"
#include "DefaultGamemode.h"

class CoopGamemode : public DefaultGamemode {
public:
    //
    // Constructor/Deconstructor.
    //
    CoopGamemode();
    virtual ~CoopGamemode() override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    // Coop has its own Obituary madness.
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

    // Respawn clients in Coop mode.
    virtual void RespawnClient(PlayerClient* playerClient) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(PlayerClient* clientEntity) override;

    // Some information that should be persistant, like health,
    // is still stored in the edict structure, so it needs to
    // be mirrored out to the client structure before all the
    // edicts are wiped.
    virtual void SaveClientEntityData(void) override;
    // Fetch client data that was stored between previous entity wipe session.
    virtual void FetchClientEntityData(Entity* ent) override;

private:

};