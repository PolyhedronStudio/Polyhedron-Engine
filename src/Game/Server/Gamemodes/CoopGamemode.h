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

class CoopGameMode : public DefaultGameMode {
public:
    //
    // Constructor/Deconstructor.
    //
    CoopGameMode();
    virtual ~CoopGameMode() override;

    //
    // Server Related.
    //
    virtual qboolean CanSaveGame(qboolean isDedicatedServer) override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual qboolean CanDamage(IServerGameEntity* targ, IServerGameEntity* inflictor) override;
    // Coop has its own Obituary madness.
    virtual void ClientUpdateObituary(IServerGameEntity* self, IServerGameEntity* inflictor, IServerGameEntity* attacker) override;

    // Respawn clients in Coop mode.
    virtual void RespawnClient(SVGBasePlayer* playerClient) override;
    virtual void RespawnAllClients() override;
    virtual void ClientDeath(SVGBasePlayer* player) override;

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

private:

};