/*
// LICENSE HERE.

//
// IGameMode.h
//
// GameMode interface class. Need a custom new gamemode? Implement this interface,
// and you did yourself a pleasure. :)
//
*/
#ifndef __SVGAME_GAMEMODES_IGAMEMODE_H__
#define __SVGAME_GAMEMODES_IGAMEMODE_H__

class SVGBaseEntity;

class IGameMode {
public:
    // Constructor/Deconstructor.
    IGameMode() {};
    virtual ~IGameMode() {};

    //
    // OG Gamerule replacement functions(vanilla q2 stuff).
    //
    // Returns true if these two entities are on a same team.
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) = 0;
    // Returns true if the target entity can be damaged by the inflictor enemy.
    virtual qboolean CanDamage(SVGBaseEntity * targ, SVGBaseEntity * inflictor) = 0;

    //
    // N&C Gamerule additions.
    //
    // Spawns a temporary entity for a client, this is best suited to be in game mode.
    // Allows for all modes to customize that when wished for.
    //
    // When implementing this interface, it is suggested to just take DefaultGameMode,
    // or base yours off of that anyhow.
    virtual void SpawnTempDamageEntity(int type, const vec3_t& origin, const vec3_t& normal, int damage) = 0;

private:

};

#endif // __SVGAME_GAMEMODES_IGAMEMODE_H__