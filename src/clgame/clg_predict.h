/*
// LICENSE HERE.

//
// clgame/clg_predict.h
//
*/

#ifndef __CLGAME_PREDICT_H__
#define __CLGAME_PREDICT_H__

void CLG_CheckPredictionError(ClientMoveCommand* moveCommand);
void CLG_PredictAngles(void);
void CLG_PredictMovement(unsigned int acknowledgedCommandIndex, unsigned int currentCommandIndex);

// WID: TODO: Another concern, clean up later.
void CLG_UpdateClientSoundSpecialEffects(PlayerMove* pm);
trace_t q_gameabi CLG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end);
int CLG_PointContents(const vec3_t& point);

#endif // __CLGAME_PREDICT_H__