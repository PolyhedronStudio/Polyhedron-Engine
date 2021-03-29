// LICENSE HERE.

//
// svgame/brushfuncs.c
//
//
// Brush utility functionality implementation.
//

// Include local game header.
#include "g_local.h"
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
//  action when blocked
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

void Brush_Move_Done(edict_t* ent)
{
    VectorClear(ent->velocity);
    ent->moveinfo.endfunc(ent);
}

void Brush_Move_Final(edict_t* ent)
{
    if (ent->moveinfo.remaining_distance == 0) {
        Brush_Move_Done(ent);
        return;
    }

    VectorScale(ent->moveinfo.dir, ent->moveinfo.remaining_distance / FRAMETIME, ent->velocity);

    ent->think = Brush_Move_Done;
    ent->nextthink = level.time + FRAMETIME;
}

void Brush_Move_Begin(edict_t* ent)
{
    float   frames;

    if ((ent->moveinfo.speed * FRAMETIME) >= ent->moveinfo.remaining_distance) {
        Brush_Move_Final(ent);
        return;
    }
    VectorScale(ent->moveinfo.dir, ent->moveinfo.speed, ent->velocity);
    frames = floor((ent->moveinfo.remaining_distance / ent->moveinfo.speed) / FRAMETIME);
    ent->moveinfo.remaining_distance -= frames * ent->moveinfo.speed * FRAMETIME;
    ent->nextthink = level.time + (frames * FRAMETIME);
    ent->think = Brush_Move_Final;
}

void Think_AccelMove(edict_t* ent);

void Brush_Move_Calc(edict_t* ent, vec3_t dest, void(*func)(edict_t*))
{
    VectorClear(ent->velocity);
    VectorSubtract(dest, ent->s.origin, ent->moveinfo.dir);
    ent->moveinfo.remaining_distance = VectorNormalize(ent->moveinfo.dir);
    ent->moveinfo.endfunc = func;

    if (ent->moveinfo.speed == ent->moveinfo.accel && ent->moveinfo.speed == ent->moveinfo.decel) {
        if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent)) {
            Brush_Move_Begin(ent);
        }
        else {
            ent->nextthink = level.time + FRAMETIME;
            ent->think = Brush_Move_Begin;
        }
    }
    else {
        // accelerative
        ent->moveinfo.current_speed = 0;
        ent->think = Think_AccelMove;
        ent->nextthink = level.time + FRAMETIME;
    }
}


//
// Support routines for angular movement (changes in angle using avelocity)
//

void Brush_AngleMove_Done(edict_t* ent)
{
    VectorClear(ent->avelocity);
    ent->moveinfo.endfunc(ent);
}

void Brush_AngleMove_Final(edict_t* ent)
{
    vec3_t  move;

    if (ent->moveinfo.state == STATE_UP)
        VectorSubtract(ent->moveinfo.end_angles, ent->s.angles, move);
    else
        VectorSubtract(ent->moveinfo.start_angles, ent->s.angles, move);

    if (VectorCompare(move, vec3_origin)) {
        Brush_AngleMove_Done(ent);
        return;
    }

    VectorScale(move, 1.0 / FRAMETIME, ent->avelocity);

    ent->think = Brush_AngleMove_Done;
    ent->nextthink = level.time + FRAMETIME;
}

void Brush_AngleMove_Begin(edict_t* ent)
{
    vec3_t  destdelta;
    float   len;
    float   traveltime;
    float   frames;

    // set destdelta to the vector needed to move
    if (ent->moveinfo.state == STATE_UP)
        VectorSubtract(ent->moveinfo.end_angles, ent->s.angles, destdelta);
    else
        VectorSubtract(ent->moveinfo.start_angles, ent->s.angles, destdelta);

    // calculate length of vector
    len = VectorLength(destdelta);

    // divide by speed to get time to reach dest
    traveltime = len / ent->moveinfo.speed;

    if (traveltime < FRAMETIME) {
        Brush_AngleMove_Final(ent);
        return;
    }

    frames = floor(traveltime / FRAMETIME);

    // scale the destdelta vector by the time spent traveling to get velocity
    VectorScale(destdelta, 1.0 / traveltime, ent->avelocity);

    // set nextthink to trigger a think when dest is reached
    ent->nextthink = level.time + frames * FRAMETIME;
    ent->think = Brush_AngleMove_Final;
}

void Brush_AngleMove_Calc(edict_t* ent, void(*func)(edict_t*))
{
    VectorClear(ent->avelocity);
    ent->moveinfo.endfunc = func;
    if (level.current_entity == ((ent->flags & FL_TEAMSLAVE) ? ent->teammaster : ent)) {
        Brush_AngleMove_Begin(ent);
    }
    else {
        ent->nextthink = level.time + FRAMETIME;
        ent->think = Brush_AngleMove_Begin;
    }
}


/*
==============
Think_AccelMove

The team has completed a frame of movement, so
change the speed for the next frame
==============
*/
#define AccelerationDistance(target, rate)  (target * ((target / rate) + 1) / 2)

void plat_CalcAcceleratedMove(moveinfo_t* moveinfo)
{
    float   accel_dist;
    float   decel_dist;

    moveinfo->move_speed = moveinfo->speed;

    if (moveinfo->remaining_distance < moveinfo->accel) {
        moveinfo->current_speed = moveinfo->remaining_distance;
        return;
    }

    accel_dist = AccelerationDistance(moveinfo->speed, moveinfo->accel);
    decel_dist = AccelerationDistance(moveinfo->speed, moveinfo->decel);

    if ((moveinfo->remaining_distance - accel_dist - decel_dist) < 0) {
        float   f;

        f = (moveinfo->accel + moveinfo->decel) / (moveinfo->accel * moveinfo->decel);
        moveinfo->move_speed = (-2 + std::sqrtf(4 - 4 * f * (-2 * moveinfo->remaining_distance))) / (2 * f);
        decel_dist = AccelerationDistance(moveinfo->move_speed, moveinfo->decel);
    }

    moveinfo->decel_distance = decel_dist;
}

void plat_Accelerate(moveinfo_t* moveinfo)
{
    // are we decelerating?
    if (moveinfo->remaining_distance <= moveinfo->decel_distance) {
        if (moveinfo->remaining_distance < moveinfo->decel_distance) {
            if (moveinfo->next_speed) {
                moveinfo->current_speed = moveinfo->next_speed;
                moveinfo->next_speed = 0;
                return;
            }
            if (moveinfo->current_speed > moveinfo->decel)
                moveinfo->current_speed -= moveinfo->decel;
        }
        return;
    }

    // are we at full speed and need to start decelerating during this move?
    if (moveinfo->current_speed == moveinfo->move_speed)
        if ((moveinfo->remaining_distance - moveinfo->current_speed) < moveinfo->decel_distance) {
            float   p1_distance;
            float   p2_distance;
            float   distance;

            p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
            p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / moveinfo->move_speed));
            distance = p1_distance + p2_distance;
            moveinfo->current_speed = moveinfo->move_speed;
            moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
            return;
        }

    // are we accelerating?
    if (moveinfo->current_speed < moveinfo->speed) {
        float   old_speed;
        float   p1_distance;
        float   p1_speed;
        float   p2_distance;
        float   distance;

        old_speed = moveinfo->current_speed;

        // figure simple acceleration up to move_speed
        moveinfo->current_speed += moveinfo->accel;
        if (moveinfo->current_speed > moveinfo->speed)
            moveinfo->current_speed = moveinfo->speed;

        // are we accelerating throughout this entire move?
        if ((moveinfo->remaining_distance - moveinfo->current_speed) >= moveinfo->decel_distance)
            return;

        // during this move we will accelrate from current_speed to move_speed
        // and cross over the decel_distance; figure the average speed for the
        // entire move
        p1_distance = moveinfo->remaining_distance - moveinfo->decel_distance;
        p1_speed = (old_speed + moveinfo->move_speed) / 2.0;
        p2_distance = moveinfo->move_speed * (1.0 - (p1_distance / p1_speed));
        distance = p1_distance + p2_distance;
        moveinfo->current_speed = (p1_speed * (p1_distance / distance)) + (moveinfo->move_speed * (p2_distance / distance));
        moveinfo->next_speed = moveinfo->move_speed - moveinfo->decel * (p2_distance / distance);
        return;
    }

    // we are at constant velocity (move_speed)
    return;
}

void Think_AccelMove(edict_t* ent)
{
    ent->moveinfo.remaining_distance -= ent->moveinfo.current_speed;

    if (ent->moveinfo.current_speed == 0)       // starting or blocked
        plat_CalcAcceleratedMove(&ent->moveinfo);

    plat_Accelerate(&ent->moveinfo);

    // will the entire move complete on next frame?
    if (ent->moveinfo.remaining_distance <= ent->moveinfo.current_speed) {
        Brush_Move_Final(ent);
        return;
    }

    VectorScale(ent->moveinfo.dir, ent->moveinfo.current_speed * 10, ent->velocity);
    ent->nextthink = level.time + FRAMETIME;
    ent->think = Think_AccelMove;
}