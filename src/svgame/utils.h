// LICENSE HERE.

//
// svgame/utils.h
//
// N&C SVGame: UTILS Header
// 
//
#ifndef __SVGAME_UTILS_H__
#define __SVGAME_UTILS_H__

qboolean SVG_KillBox(SVGBaseEntity* ent);

vec3_t SVG_ProjectSource(const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);
vec3_t SVG_PlayerProjectSource(ServersClient* client, const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);

vec3_t SVG_VelocityForDamage(int damage);

void UTIL_UseTargets(SVGBaseEntity* ent, SVGBaseEntity* activator);
void UTIL_SetMoveDir(vec3_t &angles, vec3_t &moveDirection);

void UTIL_TouchTriggers(SVGBaseEntity* ent);
void UTIL_TouchSolids(SVGBaseEntity* ent);

#endif // __SVGAME_UTILS_H__