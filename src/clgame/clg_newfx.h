/*
// LICENSE HERE.

//
// clgame/clg_newfx.h
//
*/

#ifndef __CLGAME_NEWFX_H__
#define __CLGAME_NEWFX_H__

#if USE_DLIGHTS
void CLG_Flashlight(int ent, vec3_t pos);
void CLG_ColorFlash(vec3_t pos, int ent, int intensity, float r, float g, float b);
#endif
void CLG_DebugTrail(vec3_t start, vec3_t end);
void CLG_SmokeTrail(vec3_t start, vec3_t end, int colorStart, int colorRun, int spacing);
void CLG_ForceWall(vec3_t start, vec3_t end, int color);
void CLG_GenericParticleEffect(vec3_t org, vec3_t dir, int color, int count, int numcolors, int dirspread, float alphavel);
void CLG_BubbleTrail2(vec3_t start, vec3_t end, int dist);
void CLG_Heatbeam(vec3_t start, vec3_t forward);
void CLG_ParticleSteamEffect(vec3_t org, vec3_t dir, int color, int count, int magnitude);
void CLG_ParticleSteamEffect2(cl_sustain_t* self);
void CLG_TrackerTrail(vec3_t start, vec3_t end, int particleColor);
void CLG_Tracker_Shell(vec3_t origin);
void CLG_MonsterPlasma_Shell(vec3_t origin);
void CLG_Widowbeamout(cl_sustain_t* self);
void CLG_Nukeblast(cl_sustain_t* self);
void CLG_Tracker_Explode(vec3_t  origin);
void CLG_TagTrail(vec3_t start, vec3_t end, int color);
void CLG_ColorExplosionParticles(vec3_t org, int color, int run);
void CLG_ParticleSmokeEffect(vec3_t org, vec3_t dir, int color, int count, int magnitude);
void CLG_BlasterParticles2(vec3_t org, vec3_t dir, unsigned int color);
void CLG_BlasterTrail2(vec3_t start, vec3_t end);
void CLG_IonripperTrail(vec3_t start, vec3_t ent);
void CLG_TrapParticles(r_entity_t* ent);
void CLG_ParticleEffect3(vec3_t org, vec3_t dir, int color, int count);
#endif // __CLGAME_NEWFX_H__