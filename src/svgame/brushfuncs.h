// LICENSE HERE.

//
// brushfuncs.h
//
//
// Contains basic weapons functionalities.
//
#ifndef __SVGAME_PLAYER_WEAPONS_H__
#define __SVGAME_PLAYER_WEAPONS_H__

// LICENSE HERE.

//
// svgame/brushfuncs.c
//
//
// Brush utility functionality implementation.
//

// Include local game header.
#include "g_local.h"

//=====================================================

//
//=========================================================
//
//  PLATS
//
//  movement options:
//
//  linear
//  smooth start, hard stop
//  smooth start, smooth stop
//
//  start
//  end
//  acceleration
//  speed
//  deceleration
//  begin sound
//  end sound
//  target fired when reaching end
//  wait at end
//
//  object characteristics that use move segments
//  ---------------------------------------------
//  movetype_push, or movetype_stop
//  action when touched
//  action when Blocked
//  action when used
//    disabled?
//  auto trigger spawning
//
//
//=========================================================
//

#define PLAT_LOW_TRIGGER    1

#define STATE_TOP           0
#define STATE_BOTTOM        1
#define STATE_UP            2
#define STATE_DOWN          3

//
// Support routines for movement (changes in origin using velocity)
//

void Brush_Move_Done(entity_t* ent);
void Brush_Move_Final(entity_t* ent);
void Brush_Move_Begin(entity_t* ent);
void Brush_Move_Calc(entity_t* ent, const vec3_t &dest, void(*func)(entity_t*));

//
// Support routines for angular movement (changes in angle using avelocity)
//
void Brush_AngleMove_Done(entity_t* ent);
void Brush_AngleMove_Final(entity_t* ent);
void Brush_AngleMove_Begin(entity_t* ent);
void Brush_AngleMove_Calc(entity_t* ent, void(*func)(entity_t*));

void plat_CalcAcceleratedMove(moveinfo_t* moveinfo);
void plat_Accelerate(moveinfo_t* moveinfo);
void Think_AccelMove(entity_t* ent);

#endif // __SVGAME_PLAYER_WEAPONS_H__