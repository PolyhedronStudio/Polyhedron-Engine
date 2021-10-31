/*
// LICENSE HERE.

//
// clgame/clg_effects.h
//
*/

#ifndef __CLGAME_EFFECTS_H__
#define __CLGAME_EFFECTS_H__

void CLG_ClearEffects(void);
void CLG_EffectsInit(void);

cparticle_t* CLG_AllocParticle(void);
void CLG_AddParticles(void);
#if USE_DLIGHTS
cdlight_t* CLG_AllocDLight(int key);
void CLG_AddDLights(void);
void CLG_RunDLights(void);
#endif
#if USE_LIGHTSTYLES
void CLG_ClearLightStyles(void);
void CLG_AddLightStyles(void);
void CLG_RunLightStyles(void);
void CLG_SetLightStyle(int index, const char* s);
#endif

void CLG_MuzzleFlash(void);
void CLG_MuzzleFlash2(void);
void CLG_BigTeleportParticles(vec3_t org);
void CLG_BlasterTrail(vec3_t start, vec3_t end);
void CLG_BlasterParticles(vec3_t org, vec3_t dir);
void CLG_BloodParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void CLG_BubbleTrail(vec3_t start, vec3_t end);
void CLG_DiminishingTrail(vec3_t start, vec3_t end, cl_entity_t* old, int flags);
void CLG_ExplosionParticles(vec3_t org);
void CLG_ItemRespawnParticles(vec3_t org);
void CLG_ParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void CLG_ParticleEffect2(vec3_t org, vec3_t dir, int color, int count);
void CLG_ParticleEffectWaterSplash(vec3_t org, vec3_t dir, int color, int count);
void CLG_TeleportParticles(vec3_t org);
void CLG_TeleporterParticles(vec3_t org);

#endif // __CLGAME_EFFECTS_H__