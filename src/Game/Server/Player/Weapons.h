// LICENSE HERE.

//
// weapons.h
//
//
// Contains basic weapons functionalities.
//
#ifndef __SVGAME_PLAYER_WEAPONS_H__
#define __SVGAME_PLAYER_WEAPONS_H__

class SVGBaseEntity;
class SVGBasePlayer;

// Player project source.
vec3_t SVG_PlayerProjectSource(ServerClient* client, const vec3_t &point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);

#endif // __SVGAME_PLAYER_WEAPONS_H__