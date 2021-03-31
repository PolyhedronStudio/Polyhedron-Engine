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

void ClipGibVelocity(edict_t* ent);

//
// Gibs.
//
void gib_think(edict_t* self);
void gib_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
void gib_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t& point);

void ThrowHead(edict_t* self, const char* gibname, int damage, int type);
void ThrowClientHead(edict_t* self, int damage);
void ThrowGib(edict_t* self, const char* gibname, int damage, int type);

//
// Explosions.
//
void BecomeExplosion1(edict_t* self);
void BecomeExplosion2(edict_t* self);

#endif // __SVGAME_PLAYER_WEAPONS_H__