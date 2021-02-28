// LICENSE HERE.

//
// misc.h
//
//
// Contains misc declarations.
//
#ifndef __SVGAME_MISC_H__
#define __SVGAME_MISC_H__

//
// Misc.
//
void VelocityForDamage(int damage, vec3_t v);
void ClipGibVelocity(edict_t* ent);

//
// Gibs.
//
void gib_think(edict_t* self);
void gib_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
void gib_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point);

void ThrowHead(edict_t* self, char* gibname, int damage, int type);
void ThrowClientHead(edict_t* self, int damage);
void ThrowGib(edict_t* self, char* gibname, int damage, int type);

//
// Explosions.
//
void BecomeExplosion1(edict_t* self);
void BecomeExplosion2(edict_t* self);

#endif // __SVGAME_PLAYER_WEAPONS_H__