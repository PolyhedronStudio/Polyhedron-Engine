// LICENSE HERE.

//
// weapons.h
//
//
// Contains basic weapons functionalities.
//
#ifndef __SVGAME_PLAYER_WEAPONS_H__
#define __SVGAME_PLAYER_WEAPONS_H__

// These are extern, used in the weapons/* code files.
extern qboolean is_quad;
extern byte     is_silenced;

class SVGBaseEntity;
class SVGBasePlayer;

// Player project source.
vec3_t SVG_PlayerProjectSource(ServerClient* client, const vec3_t &point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);
void SVG_PlayerNoise(SVGBaseEntity* who, vec3_t where, int type);

void        NoAmmoWeaponChange(SVGBasePlayer* ent);
void        Drop_Weapon(SVGBasePlayer *ent, gitem_t* item);

#endif // __SVGAME_PLAYER_WEAPONS_H__