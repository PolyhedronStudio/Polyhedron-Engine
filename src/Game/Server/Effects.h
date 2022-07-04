// LICENSE HERE.

//
// effects.h
//
//
// Gibs, to turning things into explosions.
//
#pragma once

//
// Forward declaration.
//
class IServerGameEntity;
class SVGBasePlayer;

//
// Explosions.
//
void SVG_BecomeExplosion1(IServerGameEntity* self);
void SVG_BecomeExplosion2(IServerGameEntity* self);

//
// Gibs.
//
void SVG_ThrowClientHead(SVGBasePlayer* self, int damage);
