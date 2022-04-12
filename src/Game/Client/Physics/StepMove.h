// LICENSE HERE.

//
// stepmove.h
//
//
//
#pragma once

class CLGBaseEntity;

void CLG_StepMove_CheckGround(IClientGameEntity* ent);
qboolean CLG_StepMove_CheckBottom(IClientGameEntity* ent);
qboolean CLG_StepMove_Walk(IClientGameEntity* ent, float yaw, float dist);