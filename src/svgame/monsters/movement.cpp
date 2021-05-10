/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// m_move.c -- monster movement

#include "../g_local.h"      // SVGame funcs.
#include "../utils.h"        // Util funcs.

#define STEPSIZE    18

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom(entity_t *ent)
{
    vec3_t  mins, maxs, start, stop;
    trace_t trace;
    int     x, y;
    float   mid, bottom;

    VectorAdd(ent->state.origin, ent->mins, mins);
    VectorAdd(ent->state.origin, ent->maxs, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0 ; x <= 1 ; x++)
        for (y = 0 ; y <= 1 ; y++) {
            start[0] = x ? maxs[0] : mins[0];
            start[1] = y ? maxs[1] : mins[1];
            if (gi.PointContents(start) != CONTENTS_SOLID)
                goto realcheck;
        }

    c_yes++;
    return true;        // we got out easy

realcheck:
    c_no++;
//
// check it for real...
//
    start[2] = mins[2];

// the midpoint must be within 16 of the bottom
    start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
    start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
    stop[2] = start[2] - 2 * STEPSIZE;
    trace = gi.Trace(start, vec3_origin, vec3_origin, stop, ent, CONTENTS_MASK_MONSTERSOLID);

    if (trace.fraction == 1.0)
        return false;
    mid = bottom = trace.endPosition[2];

// the corners must be within 16 of the midpoint
    for (x = 0 ; x <= 1 ; x++)
        for (y = 0 ; y <= 1 ; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = gi.Trace(start, vec3_origin, vec3_origin, stop, ent, CONTENTS_MASK_MONSTERSOLID);

            if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
                bottom = trace.endPosition[2];
            if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
                return false;
        }

    c_yes++;
    return true;
}


/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
qboolean SV_movestep(entity_t *ent, vec3_t move, qboolean relink)
{
    float       dz;
    vec3_t      oldorg, neworg, end;
    trace_t     trace;
    int         i;
    float       stepsize;
    vec3_t      test;
    int         contents;

// try the move
    VectorCopy(ent->state.origin, oldorg);
    VectorAdd(ent->state.origin, move, neworg);

// flying monsters don't step up
    if (ent->flags & (EntityFlags::Swim | EntityFlags::Fly)) {
        // try one move with vertical motion, then one without
        for (i = 0 ; i < 2 ; i++) {
            VectorAdd(ent->state.origin, move, neworg);
            if (i == 0 && ent->enemy) {
                if (!ent->goalEntityPtr)
                    ent->goalEntityPtr = ent->enemy;
                dz = ent->state.origin[2] - ent->goalEntityPtr->state.origin[2];
                if (ent->goalEntityPtr->client) {
                    if (dz > 40)
                        neworg[2] -= 8;
                    if (!((ent->flags & EntityFlags::Swim) && (ent->waterLevel < 2)))
                        if (dz < 30)
                            neworg[2] += 8;
                } else {
                    if (dz > 8)
                        neworg[2] -= 8;
                    else if (dz > 0)
                        neworg[2] -= dz;
                    else if (dz < -8)
                        neworg[2] += 8;
                    else
                        neworg[2] += dz;
                }
            }
            trace = gi.Trace(ent->state.origin, ent->mins, ent->maxs, neworg, ent, CONTENTS_MASK_MONSTERSOLID);

            // fly monsters don't enter water voluntarily
            if (ent->flags & EntityFlags::Fly) {
                if (!ent->waterLevel) {
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + ent->mins[2] + 1;
                    contents = gi.PointContents(test);
                    if (contents & CONTENTS_MASK_LIQUID)
                        return false;
                }
            }

            // swim monsters don't exit water voluntarily
            if (ent->flags & EntityFlags::Swim) {
                if (ent->waterLevel < 2) {
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + ent->mins[2] + 1;
                    contents = gi.PointContents(test);
                    if (!(contents & CONTENTS_MASK_LIQUID))
                        return false;
                }
            }

            if (trace.fraction == 1) {
                VectorCopy(trace.endPosition, ent->state.origin);
                if (relink) {
                    gi.LinkEntity(ent);
                    UTIL_TouchTriggers(ent);
                }
                return true;
            }

            if (!ent->enemy)
                break;
        }

        return false;
    }

// push down from a step height above the wished position
    if (!(ent->monsterInfo.aiflags & AI_NOSTEP))
        stepsize = STEPSIZE;
    else
        stepsize = 1;

    neworg[2] += stepsize;
    VectorCopy(neworg, end);
    end[2] -= stepsize * 2;

    trace = gi.Trace(neworg, ent->mins, ent->maxs, end, ent, CONTENTS_MASK_MONSTERSOLID);

    if (trace.allSolid)
        return false;

    if (trace.startSolid) {
        neworg[2] -= stepsize;
        trace = gi.Trace(neworg, ent->mins, ent->maxs, end, ent, CONTENTS_MASK_MONSTERSOLID);
        if (trace.allSolid || trace.startSolid)
            return false;
    }


    // don't go in to water
    if (ent->waterLevel == 0) {
        test[0] = trace.endPosition[0];
        test[1] = trace.endPosition[1];
        test[2] = trace.endPosition[2] + ent->mins[2] + 1;
        contents = gi.PointContents(test);

        if (contents & CONTENTS_MASK_LIQUID)
            return false;
    }

    if (trace.fraction == 1) {
        // if monster had the ground pulled out, go ahead and fall
        if (ent->flags & EntityFlags::PartiallyOnGround) {
            VectorAdd(ent->state.origin, move, ent->state.origin);
            if (relink) {
                gi.LinkEntity(ent);
                UTIL_TouchTriggers(ent);
            }
            ent->groundEntityPtr = NULL;
            return true;
        }

        return false;       // walked off an edge
    }

// check point traces down for dangling corners
    VectorCopy(trace.endPosition, ent->state.origin);

    if (!M_CheckBottom(ent)) {
        if (ent->flags & EntityFlags::PartiallyOnGround) {
            // entity had floor mostly pulled out from underneath it
            // and is trying to correct
            if (relink) {
                gi.LinkEntity(ent);
                UTIL_TouchTriggers(ent);
            }
            return true;
        }
        VectorCopy(oldorg, ent->state.origin);
        return false;
    }

    if (ent->flags & EntityFlags::PartiallyOnGround) {
        ent->flags &= ~EntityFlags::PartiallyOnGround;
    }
    ent->groundEntityPtr = trace.ent;
    ent->groundEntityLinkCount = trace.ent->linkCount;

// the move is ok
    if (relink) {
        gi.LinkEntity(ent);
        UTIL_TouchTriggers(ent);
    }
    return true;
}


//============================================================================

/*
===============
M_ChangeYaw

===============
*/
void M_ChangeYaw(entity_t *ent)
{
    float   ideal;
    float   current;
    float   move;
    float   speed;

    current = anglemod(ent->state.angles[vec3_t::Yaw]);
    ideal = ent->idealYaw;

    if (current == ideal)
        return;

    move = ideal - current;
    speed = ent->yawSpeed;
    if (ideal > current) {
        if (move >= 180)
            move = move - 360;
    } else {
        if (move <= -180)
            move = move + 360;
    }
    if (move > 0) {
        if (move > speed)
            move = speed;
    } else {
        if (move < -speed)
            move = -speed;
    }

    ent->state.angles[vec3_t::Yaw] = anglemod(current + move);
}


/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean SV_StepDirection(entity_t *ent, float yaw, float dist)
{
    vec3_t      move, oldorigin;
    float       delta;

    ent->idealYaw = yaw;
    M_ChangeYaw(ent);

    yaw = yaw * M_PI * 2 / 360;
    move[0] = std::cosf(yaw) * dist;
    move[1] = std::sinf(yaw) * dist;
    move[2] = 0;

    VectorCopy(ent->state.origin, oldorigin);
    if (SV_movestep(ent, move, false)) {
        delta = ent->state.angles[vec3_t::Yaw] - ent->idealYaw;
        if (delta > 45 && delta < 315) {
            // not turned far enough, so don't take the step
            VectorCopy(oldorigin, ent->state.origin);
        }
        gi.LinkEntity(ent);
        UTIL_TouchTriggers(ent);
        return true;
    }
    gi.LinkEntity(ent);
    UTIL_TouchTriggers(ent);
    return false;
}

/*
======================
SV_FixCheckBottom

======================
*/
void SV_FixCheckBottom(entity_t *ent)
{
    ent->flags |= EntityFlags::PartiallyOnGround;
}



/*
================
SV_NewChaseDir

================
*/
#define DI_NODIR    -1
void SV_NewChaseDir(entity_t *actor, entity_t *enemy, float dist)
{
    float   deltax, deltay;
    float   d[3];
    float   tdir, olddir, turnaround;

    //FIXME: how did we get here with no enemy
    if (!enemy)
        return;

    olddir = anglemod((int)(actor->idealYaw / 45) * 45);
    turnaround = anglemod(olddir - 180);

    deltax = enemy->state.origin[0] - actor->state.origin[0];
    deltay = enemy->state.origin[1] - actor->state.origin[1];
    if (deltax > 10)
        d[1] = 0;
    else if (deltax < -10)
        d[1] = 180;
    else
        d[1] = DI_NODIR;
    if (deltay < -10)
        d[2] = 270;
    else if (deltay > 10)
        d[2] = 90;
    else
        d[2] = DI_NODIR;

// try direct route
    if (d[1] != DI_NODIR && d[2] != DI_NODIR) {
        if (d[1] == 0)
            tdir = d[2] == 90 ? 45 : 315;
        else
            tdir = d[2] == 90 ? 135 : 215;

        if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
            return;
    }

// try other directions
    if (((rand() & 3) & 1) ||  fabsf(deltay) > fabsf(deltax)) {
        tdir = d[1];
        d[1] = d[2];
        d[2] = tdir;
    }

    if (d[1] != DI_NODIR && d[1] != turnaround
        && SV_StepDirection(actor, d[1], dist))
        return;

    if (d[2] != DI_NODIR && d[2] != turnaround
        && SV_StepDirection(actor, d[2], dist))
        return;

    /* there is no direct path to the player, so pick another direction */

    if (olddir != DI_NODIR && SV_StepDirection(actor, olddir, dist))
        return;

    if (rand() & 1) { /*randomly determine direction of search*/
        for (tdir = 0 ; tdir <= 315 ; tdir += 45)
            if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
                return;
    } else {
        for (tdir = 315 ; tdir >= 0 ; tdir -= 45)
            if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
                return;
    }

    if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist))
        return;

    actor->idealYaw = olddir;      // can't move

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

    if (!M_CheckBottom(actor))
        SV_FixCheckBottom(actor);
}

/*
======================
SV_CloseEnough

======================
*/
qboolean SV_CloseEnough(entity_t *ent, entity_t *goal, float dist)
{
    int     i;

    for (i = 0 ; i < 3 ; i++) {
        if (goal->absMin[i] > ent->absMax[i] + dist)
            return false;
        if (goal->absMax[i] < ent->absMin[i] - dist)
            return false;
    }
    return true;
}


/*
======================
M_MoveToGoal
======================
*/
void M_MoveToGoal(entity_t *ent, float dist)
{
    entity_t     *goal;

    goal = ent->goalEntityPtr;

    if (!ent->groundEntityPtr && !(ent->flags & (EntityFlags::Fly | EntityFlags::Swim)))
        return;

// if the next step hits the enemy, return immediately
    if (ent->enemy &&  SV_CloseEnough(ent, ent->enemy, dist))
        return;

// bump around...
    if ((rand() & 3) == 1 || !SV_StepDirection(ent, ent->idealYaw, dist)) {
        if (ent->inUse)
            SV_NewChaseDir(ent, goal, dist);
    }
}


/*
===============
M_walkmove
===============
*/
qboolean M_walkmove(entity_t *ent, float yaw, float dist)
{
    vec3_t  move;

    if (!ent->groundEntityPtr && !(ent->flags & (EntityFlags::Fly | EntityFlags::Swim)))
        return false;

    yaw = yaw * M_PI * 2 / 360;

    move[0] = std::cosf(yaw) * dist;
    move[1] = std::sinf(yaw) * dist;
    move[2] = 0;

    return SV_movestep(ent, move, true);
}
