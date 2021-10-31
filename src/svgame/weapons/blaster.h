// LICENSE HERE.

//
// svgame/weapons/blaster.h
//
//
// Contains blaster declarations.
//
#ifndef __SVGAME_WEAPONS_BLASTER_H__
#define __SVGAME_WEAPONS_BLASTER_H__

void Blaster_Fire(PlayerClient* ent, const vec3_t &g_offset, int damage, qboolean hyper, int effect);
void Weapon_Blaster_Fire(PlayerClient* ent);
void Weapon_Blaster(PlayerClient* ent);

#endif // __SVGAME_WEAPONS_BLASTER_H__