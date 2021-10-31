/*
// LICENSE HERE.

//
// chasecamera.h
//
// The ChaseCamera does just what it says. It's used to chase a client.
// Think of spectator mode.
//
*/
#ifndef __SVGAME_CHASECAMERA_H__
#define __SVGAME_CHASECAMERA_H__

class SVGBaseEntity;
class PlayerClient;

void SVG_UpdateChaseCam(PlayerClient* ent);
void SVG_ChaseNext(PlayerClient* ent);
void SVG_ChasePrev(PlayerClient* ent);
void SVG_GetChaseTarget(PlayerClient* ent);

#endif