/*
// LICENSE HERE.

//
// clgame/clg_input.h
//
*/

#ifndef __CLGAME_INPUT_H__
#define __CLGAME_INPUT_H__

void CLG_RegisterInput(void);

void CLG_BuildFrameMoveCommand(int msec);
void CLG_FinalizeFrameMoveCommand(void);

#endif // __CLGAME_INPUT_H__