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
// Misc.
//

void ClipGibVelocity(entity_t* ent);

//
// Gibs.
//
void gib_think(entity_t* self);
void gib_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf);
void gib_die(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t& point);

void ThrowHead(entity_t* self, const char* gibname, int damage, int type);
void ThrowClientHead(entity_t* self, int damage);
void ThrowGib(entity_t* self, const char* gibname, int damage, int type);

//
// Explosions.
//
void BecomeExplosion1(entity_t* self);
void BecomeExplosion2(entity_t* self);

#endif // __SVGAME_PLAYER_WEAPONS_H__