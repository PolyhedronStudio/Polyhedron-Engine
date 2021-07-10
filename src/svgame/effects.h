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
// Misc.
//
void ClipGibVelocity(Entity* ent);

//
// Gibs.
//
void gib_think(Entity* self);
void gib_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf);
void gib_die(Entity* self, Entity* inflictor, Entity* attacker, int damage, const vec3_t& point);

void ThrowHead(Entity* self, const char* gibname, int damage, int type);
void ThrowClientHead(Entity* self, int damage);
void ThrowGib(SVGBaseEntity* self, const char* gibname, int damage, int type);

//
// Explosions.
//
void BecomeExplosion1(SVGBaseEntity* self);
void BecomeExplosion2(SVGBaseEntity* self);

#endif // __SVGAME_PLAYER_WEAPONS_H__