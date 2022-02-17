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
// Explosions.
//
void SVG_BecomeExplosion1(SVGBaseEntity* self);
void SVG_BecomeExplosion2(SVGBaseEntity* self);

//
// Gibs.
//
void SVG_ThrowClientHead(SVGBasePlayer* self, int damage);

#endif // __SVGAME_PLAYER_WEAPONS_H__