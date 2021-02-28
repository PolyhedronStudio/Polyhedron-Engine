// LICENSE HERE.

//
// svgame/weapons/grenade.h
//
//
// Contains grenade declarations.
//
#ifndef __SVGAME_WEAPONS_GRENADE_H__
#define __SVGAME_WEAPONS_GRENADE_H__

#define GRENADE_TIMER       3.0
#define GRENADE_MINSPEED    400
#define GRENADE_MAXSPEED    800

void weapon_grenade_fire(edict_t* ent, qboolean held);
void Weapon_Grenade(edict_t* ent);

#endif // __SVGAME_WEAPONS_GRENADE_H__