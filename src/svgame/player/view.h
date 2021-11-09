// LICENSE HERE.

//
// view.h
//
//
//
#ifndef __SVGAME_PLAYER_VIEW_H__
#define __SVGAME_PLAYER_VIEW_H__

//
// Client view utility functions.
// 
// FIXME: Some of this stuff should move right into an actual PlayerClient class.
//
void SVG_Client_CheckFallingDamage(PlayerClient* ent);
void SVG_Client_ApplyDamageFeedback(PlayerClient* ent);

float SVG_Client_CalcRoll(const vec3_t& angles, const vec3_t& velocity);
void SVG_Client_CalculateViewOffset(PlayerClient* ent);
void SVG_Client_CalculateGunOffset(PlayerClient* ent);
void SVG_Client_CalculateBlend(PlayerClient* ent);

void SVG_Client_SetEvent(PlayerClient* ent);
void SVG_Client_SetEffects(PlayerClient* ent);
void SVG_Client_SetSound(PlayerClient* ent);
void SVG_Client_SetAnimationFrame(PlayerClient* ent);

void SVG_Client_CheckWorldEffects(PlayerClient* ent);

void SVG_ClientEndServerFrame(PlayerClient* ent);

#endif // __SVGAME_PLAYER_VIEW_H__