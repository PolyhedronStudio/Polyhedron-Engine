/*
// LICENSE HERE.

//
// clgame/clg_predict.h
//
*/

#ifndef __CLGAME_PREDICT_H__
#define __CLGAME_PREDICT_H__

void CLG_CheckPredictionError(ClientUserCommand* clientUserCommand);
void CLG_PredictAngles(void);
void CLG_PredictMovement(unsigned int acknowledgedCommandIndex, unsigned int currentCommandIndex);

#endif // __CLGAME_PREDICT_H__