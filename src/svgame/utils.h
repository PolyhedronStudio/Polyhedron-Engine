// LICENSE HERE.

//
// svgame/utils.h
//
// N&C SVGame: UTILS Header
// 
//
#ifndef __SVGAME_UTILS_H__
#define __SVGAME_UTILS_H__

qboolean    KillBox(edict_t* ent);

vec3_t  G_ProjectSource(const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);
vec3_t  P_ProjectSource(gclient_t* client, const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);

vec3_t VelocityForDamage(int damage);

void    UTIL_UseTargets(edict_t* ent, edict_t* activator);
void    UTIL_SetMoveDir(vec3_t& angles, vec3_t& movedir);

void    UTIL_TouchTriggers(edict_t* ent);
void    UTIL_TouchSolids(edict_t* ent);

char* vtos(const vec3_t& v, qboolean rounded = true);

float vectoyaw(const vec3_t& vec);
void vectoangles(const vec3_t& vec, vec3_t& angles);

#endif // __SVGAME_UTILS_H__