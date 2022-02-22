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
class SVGBasePlayer;

void SVG_UpdateChaseCam(SVGBasePlayer* ent);
void SVG_ChaseNext(SVGBasePlayer* ent);
void SVG_ChasePrev(SVGBasePlayer* ent);
void SVG_GetChaseTarget(SVGBasePlayer* ent);

#endif