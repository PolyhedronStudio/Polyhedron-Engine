// LICENSE HERE.

//
// effects.h
//
//
// Gibs, to turning things into explosions.
//
#ifndef __SVGAME_EFFECTS_H__
#define __SVGAME_EFFECTS_H__

//
// Forward declaration.
//
class SVGBaseEntity;

//
// Debris.
//
void SVG_ThrowDebris(SVGBaseEntity* self, const char* modelname, float speed, const vec3_t& origin);

//
// Explosions.
//
void SVG_BecomeExplosion1(SVGBaseEntity* self);
void SVG_BecomeExplosion2(SVGBaseEntity* self);

//
// Gibs.
//
void SVG_ThrowClientHead(PlayerClient* self, int damage);
void SVG_ThrowGib(SVGBaseEntity* self, const char* gibname, int damage, int type);

#endif // __SVGAME_PLAYER_WEAPONS_H__