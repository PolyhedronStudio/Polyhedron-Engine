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

#include "../entities/base/SVGBaseEntity.h"

#define STEPSIZE    18

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean SVG_StepMove_CheckBottom(SVGBaseEntity* ent)
{
    vec3_t  start, stop;
    SVGTrace trace;
    int32_t x, y;
    float   mid, bottom;

    vec3_t mins = ent->GetOrigin() - ent->GetMins(); //VectorAdd(ent->state.origin, ent->mins, mins);
    vec3_t maxs = ent->GetOrigin() - ent->GetMaxs(); //VectorAdd(ent->state.origin, ent->maxs, maxs);


                                                     // if all of the points under the corners are solid world, don't bother
                                                     // with the tougher checks
                                                     // the corners must be within 16 of the midpoint
    start[2] = mins[2] - 1;
    for (x = 0; x <= 1; x++)
        for (y = 0; y <= 1; y++) {
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
    trace = SVG_Trace(start, vec3_zero(), vec3_zero(), stop, ent, CONTENTS_MASK_MONSTERSOLID);

    if (trace.fraction == 1.0)
        return false;
    mid = bottom = trace.endPosition[2];

    // the corners must be within 16 of the midpoint
    for (x = 0; x <= 1; x++)
        for (y = 0; y <= 1; y++) {
            start[0] = stop[0] = x ? maxs[0] : mins[0];
            start[1] = stop[1] = y ? maxs[1] : mins[1];

            trace = SVG_Trace(start, vec3_zero(), vec3_zero(), stop, ent, CONTENTS_MASK_MONSTERSOLID);

            if (trace.fraction != 1.0 && trace.endPosition[2] > bottom)
                bottom = trace.endPosition[2];
            if (trace.fraction == 1.0 || mid - trace.endPosition[2] > STEPSIZE)
                return false;
        }

    c_yes++;
    return true;
}

void SVG_StepMove_CheckGround(SVGBaseEntity* ent)
{
    vec3_t      point;
    SVGTrace     trace;

    if (ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly))
        return;

    if (ent->GetVelocity().z > 100) {
        ent->SetGroundEntity(nullptr);
        return;
    }

    // if the hull point one-quarter unit down is solid the entity is on ground
    point = ent->GetOrigin() - vec3_t {
        0.f, 0.f, 0.25f 
    };

    trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), point, ent, CONTENTS_MASK_MONSTERSOLID);

    // check steepness
    if (trace.plane.normal[2] < 0.7 && !trace.startSolid) {
        ent->SetGroundEntity(nullptr);
        return;
    }

    //  ent->groundentity = trace.ent;
    //  ent->groundentity_linkcount = trace.ent->linkcount;
    //  if (!trace.startsolid && !trace.allsolid)
    //      VectorCopy (trace.endpos, ent->s.origin);
    if ((!trace.startSolid && !trace.allSolid) &&
        (trace.ent && trace.ent->GetServerEntity())) {
        ent->SetOrigin(trace.endPosition);
        ent->SetGroundEntity(trace.ent);
        ent->SetGroundEntityLinkCount(trace.ent->GetLinkCount());
        vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, 0 });
    }
}

/*
=============
SVG_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
qboolean SVG_MoveStep(SVGBaseEntity* ent, vec3_t move, qboolean relink)
{
    float       dz;
    SVGTrace    trace;
    int         i;
    float       stepsize;
    vec3_t      test;
    int         contents;

    // Try the move
    vec3_t oldOrigin = ent->GetOrigin(); // VectorCopy(ent->state.origin, oldorg);
    vec3_t newOrigin = oldOrigin + move;

    // flying monsters don't step up
    if (ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)) {
        // try one move with vertical motion, then one without
        for (i = 0; i < 2; i++) {
            newOrigin = ent->GetOrigin() + move;

            if (i == 0 && ent->GetEnemy()) {
                if (!ent->GetServerEntity()->goalEntityPtr)
                    ent->GetServerEntity()->goalEntityPtr = ent->GetEnemy()->GetServerEntity();
                dz = ent->GetServerEntity()->state.origin[2] - ent->GetServerEntity()->goalEntityPtr->state.origin[2];
                if (ent->GetServerEntity()->goalEntityPtr->client) {
                    if (dz > 40)
                        newOrigin.z -= 8;
                    if (!((ent->GetFlags() & EntityFlags::Swim) && (ent->GetWaterLevel() < 2)))
                        if (dz < 30)
                            newOrigin.z += 8;
                }
                else {
                    if (dz > 8)
                        newOrigin.z -= 8;
                    else if (dz > 0)
                        newOrigin.z -= dz;
                    else if (dz < -8)
                        newOrigin.z += 8;
                    else
                        newOrigin.z += dz;
                }
            }
            trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), newOrigin, ent, CONTENTS_MASK_MONSTERSOLID);

            // fly monsters don't enter water voluntarily
            if (ent->GetFlags() & EntityFlags::Fly) {
                if (!ent->GetWaterLevel()) {
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + ent->GetMins().z + 1;
                    contents = gi.PointContents(test);
                    if (contents & CONTENTS_MASK_LIQUID)
                        return false;
                }
            }

            // swim monsters don't exit water voluntarily
            if (ent->GetFlags() & EntityFlags::Swim) {
                if (ent->GetWaterLevel() < 2) {
                    test[0] = trace.endPosition[0];
                    test[1] = trace.endPosition[1];
                    test[2] = trace.endPosition[2] + ent->GetMins().z + 1;
                    contents = gi.PointContents(test);
                    if (!(contents & CONTENTS_MASK_LIQUID))
                        return false;
                }
            }

            if (trace.fraction == 1) {
                ent->SetOrigin(trace.endPosition);

                if (relink) {
                    ent->LinkEntity();
                    UTIL_TouchTriggers(ent);
                }
                return true;
            }

            if (!ent->GetEnemy())
                break;
        }

        return false;
    }

    // Push down from a step height above the wished position
    //    if (!(ent->monsterInfo.aiflags & AI_NOSTEP))
    stepsize = STEPSIZE;
    //else
    //    stepsize = 1;

    newOrigin[2] += stepsize;
    vec3_t end = newOrigin;
    end[2] -= stepsize * 2;

    trace = SVG_Trace(newOrigin, ent->GetMins(), ent->GetMaxs(), end, ent, CONTENTS_MASK_MONSTERSOLID);

    if (trace.allSolid)
        return false;

    if (trace.startSolid) {
        newOrigin[2] -= stepsize;
        trace = SVG_Trace(newOrigin, ent->GetMins(), ent->GetMaxs(), end, ent, CONTENTS_MASK_MONSTERSOLID);
        if (trace.allSolid || trace.startSolid)
            return false;
    }


    // don't go in to water
    if (ent->GetWaterLevel() == 0) {
        test[0] = trace.endPosition[0];
        test[1] = trace.endPosition[1];
        test[2] = trace.endPosition[2] + ent->GetMins().z + 1;
        contents = gi.PointContents(test);

        if (contents & CONTENTS_MASK_LIQUID)
            return false;
    }

    if (trace.fraction == 1) {
        // if monster had the ground pulled out, go ahead and fall
        if (ent->GetFlags() & EntityFlags::PartiallyOnGround) {
            ent->SetOrigin(ent->GetOrigin() + move);
            if (relink) {
                ent->LinkEntity();
                UTIL_TouchTriggers(ent);
            }
            ent->SetGroundEntity(nullptr);
            return true;
        }

        return false;       // walked off an edge
    }

    // check point traces down for dangling corners
    ent->SetOrigin(trace.endPosition);

    if (!SVG_StepMove_CheckBottom(ent)) {
        if (ent->GetFlags() & EntityFlags::PartiallyOnGround) {
            // entity had floor mostly pulled out from underneath it
            // and is trying to correct
            if (relink) {
                ent->LinkEntity();
                UTIL_TouchTriggers(ent);
            }
            return true;
        }
        ent->SetOrigin(oldOrigin);
        return false;
    }

    if (ent->GetFlags() & EntityFlags::PartiallyOnGround) {
        ent->SetFlags(ent->GetFlags() & ~EntityFlags::PartiallyOnGround);
    }
    ent->SetGroundEntity(trace.ent);
    ent->SetGroundEntityLinkCount(trace.ent->GetLinkCount());

    // the move is ok
    if (relink) {
        ent->LinkEntity();
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
static void SVG_CalculateYawAngle (Entity* ent)
{
    float   ideal;
    float   current;
    float   move;
    float   speed;

    current = AngleMod(ent->state.angles[vec3_t::Yaw]);

    if (ent->classEntity)
        ideal = ent->classEntity->GetIdealYawAngle();
    else
        ideal = 0.f;

    if (current == ideal)
        return;

    move = ideal - current;
    if (ent->classEntity)
        speed = ent->classEntity->GetYawSpeed();
    else
        speed = 0.f;

    if (ideal > current) {
        if (move >= 180)
            move = move - 360;
    }
    else {
        if (move <= -180)
            move = move + 360;
    }
    if (move > 0) {
        if (move > speed)
            move = speed;
    }
    else {
        if (move < -speed)
            move = -speed;
    }

    ent->state.angles[vec3_t::Yaw] = AngleMod(current + move);
}


/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean SV_StepDirection(SVGBaseEntity* ent, float yaw, float dist)
{
    vec3_t      move, oldOrigin;
    float       delta;

    ent->SetIdealYawAngle(yaw);
    SVG_CalculateYawAngle(ent->GetServerEntity());

    yaw = yaw * M_PI * 2 / 360;
    move[0] = std::cosf(yaw) * dist;
    move[1] = std::sinf(yaw) * dist;
    move[2] = 0;

    oldOrigin = ent->GetOrigin();

    //    VectorCopy(ent->state.origin, oldorigin);
    if (SVG_MoveStep(ent, move, false)) {
        delta = ent->GetServerEntity()->state.angles[vec3_t::Yaw] - ent->GetIdealYawAngle();
        if (delta > 45 && delta < 315) {
            // not turned far enough, so don't take the step
            ent->SetOrigin(oldOrigin);
        }
        ent->LinkEntity();
        UTIL_TouchTriggers(ent);
        return true;
    }
    ent->LinkEntity();
    UTIL_TouchTriggers(ent);
    return false;
}

/*
===============
SVG_WalkStepMove
===============
*/
qboolean SVG_StepMove_Walk(SVGBaseEntity* ent, float yaw, float dist)
{
    vec3_t  move;

    if (!ent->GetGroundEntity() && !(ent->GetFlags() & (EntityFlags::Fly | EntityFlags::Swim)))
        return false;

    yaw = yaw * M_PI * 2 / 360;

    move[0] = std::cosf(yaw) * dist;
    move[1] = std::sinf(yaw) * dist;
    move[2] = 0;

    return SVG_MoveStep(ent, move, true);
}
