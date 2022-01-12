// LICENSE HERE.

//
// svgame/brushfuncs.c
//
//
// Brush utility functionality implementation.
//

// Include local game header.
#include "g_local.h"
#include "entities/base/SVGBaseEntity.h"
#include "brushfuncs.h"

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

//#define DOOR_START_OPEN     1
//#define DOOR_REVERSE        2
//#define DOOR_CRUSHER        4
//#define DOOR_NOMONSTER      8
//#define DOOR_TOGGLE         32
//#define DOOR_X_AXIS         64
//#define DOOR_Y_AXIS         128

//
// Support routines for movement (changes in origin using velocity)
//

//void Brush_Move_Done(Entity* ent)
//{
//    ent->classEntity->SetVelocity( vec3_zero() );
//    ent->moveInfo.OnEndFunction( ent );
//}
//
//void Brush_Move_Final(Entity* ent)
//{
//    if ( ent->moveInfo.remainingDistance == 0 ) 
//    {
//        Brush_Move_Done( ent );
//        return;
//    }
//
//    // Move only as far as to clear the remaining distance
//    ent->classEntity->SetVelocity( vec3_scale( ent->moveInfo.dir, ent->moveInfo.remainingDistance / FRAMETIME ) );
//
//    // FIXME This is a problem.
//    //ent->Think = Brush_Move_Done;
//    ent->classEntity->SetNextThinkTime( level.time + FRAMETIME );
//}
//
//void Brush_Move_Begin(Entity* ent)
//{
//    float frames;
//
//    // It's time to stop
//    if ( (ent->moveInfo.speed * FRAMETIME) >= ent->moveInfo.remainingDistance )
//    {
//        Brush_Move_Final(ent);
//        return;
//    }
//    
//    //VectorScale(ent->moveInfo.dir, ent->moveInfo.speed, ent->velocity);
//    ent->classEntity->SetVelocity( vec3_scale( ent->moveInfo.dir, ent->moveInfo.speed ) );
//
//    frames = floor( (ent->moveInfo.remainingDistance / ent->moveInfo.speed) / FRAMETIME );
//    ent->moveInfo.remainingDistance -= frames * ent->moveInfo.speed * FRAMETIME;
//    ent->classEntity->SetNextThinkTime( level.time + (frames * FRAMETIME) );
//    
//    // FIXME: Mayhaps Brush_Move_Begin et al should be moved to SVGBaseEntity
//    //ent->Think = Brush_Move_Final;
//}
//
//void Think_AccelMove(Entity* ent);
//
//void Brush_Move_Calc(Entity* ent, const vec3_t &dest, void(*func)(Entity*))
//{
//    ent->classEntity->SetVelocity( vec3_zero() );
//    ent->moveInfo.dir = dest - ent->classEntity->GetOrigin();
//    ent->moveInfo.remainingDistance = VectorNormalize( ent->moveInfo.dir );
//    ent->moveInfo.OnEndFunction = func;
//
//    if (ent->moveInfo.speed == ent->moveInfo.acceleration && ent->moveInfo.speed == ent->moveInfo.deceleration)
//    {
//        if ( level.currentEntity == 
//             ((ent->classEntity->GetFlags() & EntityFlags::TeamSlave) ? 
//             ent->classEntity->GetTeamMasterEntity() : ent->classEntity) ) 
//        {
//            Brush_Move_Begin(ent);
//        }
//        else 
//        {
//            ent->classEntity->SetNextThinkTime( level.time + FRAMETIME );
//            //ent->Think = Brush_Move_Begin;
//        }
//    }
//    else 
//    {
//        // accelerative
//        ent->moveInfo.currentSpeed = 0;
//        //ent->Think = Think_AccelMove;
//        ent->classEntity->SetNextThinkTime( level.time + FRAMETIME );
//    }
//}
//
//
////
//// Support routines for angular movement (changes in angle using angularVelocity)
////
//
//void Brush_AngleMove_Done(Entity* ent)
//{
//    ent->classEntity->SetAngularVelocity( vec3_zero() );
//    ent->moveInfo.OnEndFunction( ent );
//}
//
//void Brush_AngleMove_Final(Entity* ent)
//{
//    vec3_t move;
//
//    if ( ent->moveInfo.state == STATE_UP )
//    {
//        VectorSubtract( ent->moveInfo.endAngles, ent->state.angles, move );
//    }
//    else
//    {
//        VectorSubtract( ent->moveInfo.startAngles, ent->state.angles, move );
//    }
//
//    if ( VectorCompare( move, vec3_zero(); ) ) 
//    {
//        Brush_AngleMove_Done(ent);
//        return;
//    }
//
//    ent->classEntity->SetAngularVelocity( vec3_scale( move, 1.0f / FRAMETIME ) );
//    
//    //ent->Think = Brush_AngleMove_Done;
//    ent->classEntity->SetNextThinkTime( level.time + FRAMETIME );
//}
//
//void Brush_AngleMove_Begin(Entity* ent)
//{
//    //vec3_t  destdelta;
//    //float   len;
//    //float   traveltime;
//    //float   frames;
//
//    //// set destdelta to the vector needed to move
//    //if (ent->moveInfo.state == STATE_UP)
//    //    VectorSubtract(ent->moveInfo.endAngles, ent->state.angles, destdelta);
//    //else
//    //    VectorSubtract(ent->moveInfo.startAngles, ent->state.angles, destdelta);
//
//    //// calculate length of vector
//    //len = VectorLength(destdelta);
//
//    //// divide by speed to get time to reach dest
//    //traveltime = len / ent->moveInfo.speed;
//
//    //if (traveltime < FRAMETIME) {
//    //    Brush_AngleMove_Final(ent);
//    //    return;
//    //}
//
//    //frames = floor(traveltime / FRAMETIME);
//
//    //// scale the destdelta vector by the time spent traveling to get velocity
//    //VectorScale(destdelta, 1.0 / traveltime, ent->angularVelocity);
//
//    //// set nextThinkTime to trigger a Think when dest is reached
//    //ent->nextThinkTime = level.time + frames * FRAMETIME;
//    ////ent->Think = Brush_AngleMove_Final;
//}
//
//void Brush_AngleMove_Calc(Entity* ent, void(*func)(Entity*))
//{
//    //VectorClear(ent->angularVelocity);
//    //ent->moveInfo.OnEndFunction = func;
//    //if (level.currentEntity == ((ent->flags & EntityFlags::TeamSlave) ? ent->teamMasterPtr : ent)) {
//    //    Brush_AngleMove_Begin(ent);
//    //}
//    //else {
//    //    ent->nextThinkTime = level.time + FRAMETIME;
//    //    ent->Think = Brush_AngleMove_Begin;
//    //}
//}
//
//
///*
//==============
//Think_AccelMove
//
//The team has completed a frame of movement, so
//change the speed for the next frame
//==============
//*/
//#define AccelerationDistance(target, rate)  (target * ((target / rate) + 1) / 2)
//
//void plat_CalcAcceleratedMove(PushMoveInfo* moveinfo)
//{
//    float   accel_dist;
//    float   decel_dist;
//
//    moveinfo->moveSpeed = moveinfo->speed;
//
//    if (moveinfo->remainingDistance < moveinfo->acceleration) {
//        moveinfo->currentSpeed = moveinfo->remainingDistance;
//        return;
//    }
//
//    accel_dist = AccelerationDistance(moveinfo->speed, moveinfo->acceleration);
//    decel_dist = AccelerationDistance(moveinfo->speed, moveinfo->deceleration);
//
//    if ((moveinfo->remainingDistance - accel_dist - decel_dist) < 0) {
//        float   f;
//
//        f = (moveinfo->acceleration + moveinfo->deceleration) / (moveinfo->acceleration * moveinfo->deceleration);
//        moveinfo->moveSpeed = (-2 + std::sqrtf(4 - 4 * f * (-2 * moveinfo->remainingDistance))) / (2 * f);
//        decel_dist = AccelerationDistance(moveinfo->moveSpeed, moveinfo->deceleration);
//    }
//
//    moveinfo->deceleratedDistance = decel_dist;
//}
//
//void plat_Accelerate(PushMoveInfo* moveinfo)
//{
//    // are we decelerating?
//    if (moveinfo->remainingDistance <= moveinfo->deceleratedDistance) {
//        if (moveinfo->remainingDistance < moveinfo->deceleratedDistance) {
//            if (moveinfo->nextSpeed) {
//                moveinfo->currentSpeed = moveinfo->nextSpeed;
//                moveinfo->nextSpeed = 0;
//                return;
//            }
//            if (moveinfo->currentSpeed > moveinfo->deceleration)
//                moveinfo->currentSpeed -= moveinfo->deceleration;
//        }
//        return;
//    }
//
//    // are we at full speed and need to start decelerating during this move?
//    if (moveinfo->currentSpeed == moveinfo->moveSpeed)
//        if ((moveinfo->remainingDistance - moveinfo->currentSpeed) < moveinfo->deceleratedDistance) {
//            float   p1_distance;
//            float   p2_distance;
//            float   distance;
//
//            p1_distance = moveinfo->remainingDistance - moveinfo->deceleratedDistance;
//            p2_distance = moveinfo->moveSpeed * (1.0 - (p1_distance / moveinfo->moveSpeed));
//            distance = p1_distance + p2_distance;
//            moveinfo->currentSpeed = moveinfo->moveSpeed;
//            moveinfo->nextSpeed = moveinfo->moveSpeed - moveinfo->deceleration * (p2_distance / distance);
//            return;
//        }
//
//    // are we accelerating?
//    if (moveinfo->currentSpeed < moveinfo->speed) {
//        float   old_speed;
//        float   p1_distance;
//        float   p1_speed;
//        float   p2_distance;
//        float   distance;
//
//        old_speed = moveinfo->currentSpeed;
//
//        // figure simple acceleration up to moveSpeed
//        moveinfo->currentSpeed += moveinfo->acceleration;
//        if (moveinfo->currentSpeed > moveinfo->speed)
//            moveinfo->currentSpeed = moveinfo->speed;
//
//        // are we accelerating throughout this entire move?
//        if ((moveinfo->remainingDistance - moveinfo->currentSpeed) >= moveinfo->deceleratedDistance)
//            return;
//
//        // during this move we will accelrate from currentSpeed to moveSpeed
//        // and cross over the deceleratedDistance; figure the average speed for the
//        // entire move
//        p1_distance = moveinfo->remainingDistance - moveinfo->deceleratedDistance;
//        p1_speed = (old_speed + moveinfo->moveSpeed) / 2.0;
//        p2_distance = moveinfo->moveSpeed * (1.0 - (p1_distance / p1_speed));
//        distance = p1_distance + p2_distance;
//        moveinfo->currentSpeed = (p1_speed * (p1_distance / distance)) + (moveinfo->moveSpeed * (p2_distance / distance));
//        moveinfo->nextSpeed = moveinfo->moveSpeed - moveinfo->deceleration * (p2_distance / distance);
//        return;
//    }
//
//    // we are at constant velocity (moveSpeed)
//    return;
//}

void Think_AccelMove(Entity* ent)
{
    //ent->moveInfo.remainingDistance -= ent->moveInfo.currentSpeed;

    //if (ent->moveInfo.currentSpeed == 0)       // starting or Blocked
    //    plat_CalcAcceleratedMove(&ent->moveInfo);

    //plat_Accelerate(&ent->moveInfo);

    //// will the entire move complete on next frame?
    //if (ent->moveInfo.remainingDistance <= ent->moveInfo.currentSpeed) {
    //    Brush_Move_Final(ent);
    //    return;
    //}

    //VectorScale(ent->moveInfo.dir, ent->moveInfo.currentSpeed * 10, ent->velocity);
    //ent->nextThinkTime = level.time + FRAMETIME;
    ////ent->Think = Think_AccelMove;
}