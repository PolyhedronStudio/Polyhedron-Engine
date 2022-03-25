// LICENSE HERE.

//
// stepmove.h
//
//
//
#pragma once

void CLG_StepMove_CheckGround(CLGBaseEntity* ent);
qboolean CLG_StepMove_CheckBottom(CLGBaseEntity* ent);
qboolean CLG_StepMove_Walk(CLGBaseEntity* ent, float yaw, float dist);