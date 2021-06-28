/*
// LICENSE HERE.

//
// DefaultMode.h
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
    qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) override;
    qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    void SpawnTempDamageEntity(int type, const vec3_t& origin, const vec3_t& normal, int damage);

private:

};

#endif // __SVGAME_GAMEMODES_DEFAULTGAMEMODE_H__