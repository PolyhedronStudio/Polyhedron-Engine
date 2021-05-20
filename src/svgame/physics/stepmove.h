// LICENSE HERE.

//
// stepmove.h
//
//
//
#ifndef __SVGAME_PHYSICS_STEPMOVE_H__
#define __SVGAME_PHYSICS_STEPMOVE_H__

void SVG_StepMove_CheckGround(SVGBaseEntity* ent);
qboolean SVG_StepMove_CheckBottom(SVGBaseEntity* ent);
qboolean SVG_StepMove_Walk(SVGBaseEntity* ent, float yaw, float dist);

#endif // __SVGAME_PHYSICS_STEPMOVE_H__