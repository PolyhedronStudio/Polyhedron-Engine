// LICENSE HERE.

//
// svgame/utils.h
//
// N&C SVGame: UTILS Header
// 
//
#ifndef __SVGAME_UTILS_H__
#define __SVGAME_UTILS_H__

qboolean    KillBox(Entity* ent);

vec3_t  G_ProjectSource(const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);
vec3_t  P_ProjectSource(GameClient* client, const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);

vec3_t VelocityForDamage(int damage);

void    UTIL_UseTargets(Entity* ent, Entity* activator);
void    UTIL_SetMoveDir(vec3_t& angles, vec3_t& moveDirection);

void    UTIL_TouchTriggers(Entity* ent);
void    UTIL_TouchSolids(Entity* ent);

float vectoyaw(const vec3_t& vec);

#endif // __SVGAME_UTILS_H__