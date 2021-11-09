/*
// LICENSE HERE.

//
// DeathMatchGameMode.h
//
// Default game mode to run, allows for all sorts of stuff.
//
*/
#ifndef __SVGAME_GAMEMODES_DEATHMATCHGAMEMODE_H__
#define __SVGAME_GAMEMODES_DEATHMATCHGAMEMODE_H__

#include "IGameMode.h"
#include "DefaultGameMode.h"

class DeathMatchGameMode : public DefaultGameMode {
public:
    // Constructor/Deconstructor.
    DeathMatchGameMode();
    virtual ~DeathMatchGameMode() override;

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    // MP has its own damage regulation logic.
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    // MP Does special "ClientBegin" rules for clients.
    virtual void ClientBegin(Entity* serverEntity) override;
    // DeathMatch has its own Obituary madness.
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

private:

};

#endif // __SVGAME_GAMEMODES_DEATHMATCHGAMEMODE_H__