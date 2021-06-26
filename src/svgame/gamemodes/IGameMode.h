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
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) = 0;
    virtual qboolean CanDamage(SVGBaseEntity * targ, SVGBaseEntity * inflictor) = 0;

private:

};

#endif // __SVGAME_GAMEMODES_IGAMEMODE_H__