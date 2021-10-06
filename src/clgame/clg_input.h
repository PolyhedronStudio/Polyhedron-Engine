/*
// LICENSE HERE.

//
// clgame/clg_input.h
//
*/

#ifndef __CLGAME_INPUT_H__
#define __CLGAME_INPUT_H__

void CLG_RegisterInput(void);

void CLG_KeyClear(KeyBinding* b);

void CLG_BuildFrameMoveCommand(int32_t miliseconds);
void CLG_FinalizeFrameMoveCommand(void);

vec3_t CLG_BaseMove(const vec3_t& inMove);
void CLG_AdjustAngles(int32_t miliseconds);
void CLG_MouseMove();
vec3_t CLG_ClampSpeed(const vec3_t& inMove);
void CLG_ClampPitch(void);

#endif // __CLGAME_INPUT_H__