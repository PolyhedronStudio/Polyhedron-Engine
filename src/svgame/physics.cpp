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
// g_phys.c

#include "g_local.h"
#include "utils.h"

/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects

doors, plats, etc are Solid::BSP, and MoveType::Push
bonus items are Solid::Trigger touch, and MoveType::Toss
corpses are Solid::Not and MoveType::Toss
crates are Solid::BoundingBox and MoveType::Toss
walking monsters are SOLID_SLIDEBOX and MoveType::Step
flying/floating monsters are SOLID_SLIDEBOX and MoveType::Fly

solid_edge items only clip against bsp models.

*/


/*
============
SV_TestEntityPosition

============
*/
entity_t *SV_TestEntityPosition(entity_t *ent)
{
    trace_t trace;
    int     mask;

    if (ent->clipMask)
        mask = ent->clipMask;
    else
        mask = CONTENTS_MASK_SOLID;
    trace = gi.Trace(ent->state.origin, ent->mins, ent->maxs, ent->state.origin, ent, mask);

    if (trace.startSolid)
        return g_edicts;

    return NULL;
}


/*
================
SV_CheckVelocity
================
*/
void SV_CheckVelocity(entity_t *ent)
{
    int     i;

//
// bound velocity
//
    for (i = 0 ; i < 3 ; i++) {
        if (ent->velocity[i] > sv_maxvelocity->value)
            ent->velocity[i] = sv_maxvelocity->value;
        else if (ent->velocity[i] < -sv_maxvelocity->value)
            ent->velocity[i] = -sv_maxvelocity->value;
    }
}

/*
=============
SV_RunThink

Runs thinking code for this frame if necessary
=============
*/
qboolean SV_RunThink(entity_t *ent)
{
    float   thinktime;

    thinktime = ent->nextThink;
    if (thinktime <= 0)
        return true;
    if (thinktime > level.time + 0.001)
        return true;

    ent->nextThink = 0;
    if (!ent->Think)
        gi.Error("NULL ent->Think");
    ent->Think(ent);

    return false;
}

/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
void SV_Impact(entity_t *e1, trace_t *trace)
{
    entity_t     *e2;
//  cplane_t    backplane;

    e2 = trace->ent;

    if (e1->Touch && e1->solid != Solid::Not)
        e1->Touch(e1, e2, &trace->plane, trace->surface);

    if (e2->Touch && e2->solid != Solid::Not)
        e2->Touch(e2, e1, NULL, NULL);
}


/*
==================
ClipVelocity

Slide off of the impacting object
returns the Blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define STOP_EPSILON    0.1

int ClipVelocity(const vec3_t &in, const vec3_t &normal, vec3_t &out, float overbounce)
{
    float   backoff;
    float   change;
    int     i, Blocked;

    Blocked = 0;
    if (normal[2] > 0)
        Blocked |= 1;       // floor
    if (!normal[2])
        Blocked |= 2;       // step

    backoff = DotProduct(in, normal) * overbounce;

    for (i = 0 ; i < 3 ; i++) {
        change = normal[i] * backoff;
        out[i] = in[i] - change;
        if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
            out[i] = 0;
    }

    return Blocked;
}


/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/
#define MAX_CLIP_PLANES 5
int SV_FlyMove(entity_t *ent, float time, int mask)
{
    entity_t     *hit;
    int         bumpcount, numbumps;
    vec3_t      dir;
    float       d;
    int         numplanes;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      primal_velocity, original_velocity, new_velocity;
    int         i, j;
    trace_t     trace;
    vec3_t      end;
    float       time_left;
    int         Blocked;

    numbumps = 4;

    Blocked = 0;
    VectorCopy(ent->velocity, original_velocity);
    VectorCopy(ent->velocity, primal_velocity);
    numplanes = 0;

    time_left = time;

    ent->groundEntityPtr = NULL;
    for (bumpcount = 0 ; bumpcount < numbumps ; bumpcount++) {
        for (i = 0 ; i < 3 ; i++)
            end[i] = ent->state.origin[i] + time_left * ent->velocity[i];

        trace = gi.Trace(ent->state.origin, ent->mins, ent->maxs, end, ent, mask);

        if (trace.allSolid) {
            // entity is trapped in another solid
            VectorCopy(vec3_origin, ent->velocity);
            return 3;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            VectorCopy(trace.endPosition, ent->state.origin);
            VectorCopy(ent->velocity, original_velocity);
            numplanes = 0;
        }

        if (trace.fraction == 1)
            break;     // moved the entire distance

        hit = trace.ent;

        if (trace.plane.normal[2] > 0.7) {
            Blocked |= 1;       // floor
            if (hit->solid == Solid::BSP) {
                ent->groundEntityPtr = hit;
                ent->groundEntityLinkCount = hit->linkCount;
            }
        }
        if (!trace.plane.normal[2]) {
            Blocked |= 2;       // step
        }

//
// run the impact function
//
        SV_Impact(ent, &trace);
        if (!ent->inUse)
            break;      // removed by the impact function


        time_left -= time_left * trace.fraction;

        // cliped to another plane
        if (numplanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            VectorCopy(vec3_origin, ent->velocity);
            return 3;
        }

        VectorCopy(trace.plane.normal, planes[numplanes]);
        numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
        for (i = 0 ; i < numplanes ; i++) {
            ClipVelocity(original_velocity, planes[i], new_velocity, 1);

            for (j = 0 ; j < numplanes ; j++)
                if ((j != i) && !VectorCompare(planes[i], planes[j])) {
                    if (DotProduct(new_velocity, planes[j]) < 0)
                        break;  // not ok
                }
            if (j == numplanes)
                break;
        }

        if (i != numplanes) {
            // go along this plane
            VectorCopy(new_velocity, ent->velocity);
        } else {
            // go along the crease
            if (numplanes != 2) {
//              gi.DPrintf ("clip velocity, numplanes == %i\n",numplanes);
                VectorCopy(vec3_origin, ent->velocity);
                return 7;
            }
            CrossProduct(planes[0], planes[1], dir);
            d = DotProduct(dir, ent->velocity);
            VectorScale(dir, d, ent->velocity);
        }

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
        if (DotProduct(ent->velocity, primal_velocity) <= 0) {
            VectorCopy(vec3_origin, ent->velocity);
            return Blocked;
        }
    }

    return Blocked;
}


/*
============
SV_AddGravity

============
*/
void SV_AddGravity(entity_t *ent)
{
    ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME;
}

/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
trace_t SV_PushEntity(entity_t *ent, vec3_t push)
{
    trace_t trace;
    vec3_t  start;
    vec3_t  end;
    int     mask;

    VectorCopy(ent->state.origin, start);
    VectorAdd(start, push, end);

retry:
    if (ent->clipMask)
        mask = ent->clipMask;
    else
        mask = CONTENTS_MASK_SOLID;

    trace = gi.Trace(start, ent->mins, ent->maxs, end, ent, mask);

    VectorCopy(trace.endPosition, ent->state.origin);
    gi.LinkEntity(ent);

    if (trace.fraction != 1.0) {
        SV_Impact(ent, &trace);

        // if the pushed entity went away and the pusher is still there
        if (!trace.ent->inUse && ent->inUse) {
            // move the pusher back and try again
            VectorCopy(start, ent->state.origin);
            gi.LinkEntity(ent);
            goto retry;
        }
    }

    if (ent->inUse)
        UTIL_TouchTriggers(ent);

    return trace;
}


typedef struct {
    entity_t *ent;
    vec3_t  origin;
    vec3_t  angles;
#if USE_SMOOTH_DELTA_ANGLES
    int     deltayaw;
#endif
} pushed_t;
pushed_t    pushed[MAX_EDICTS], *pushed_p;

entity_t *obstacle;

/*
============
SV_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
============
*/
qboolean SV_Push(entity_t *pusher, vec3_t move, vec3_t amove)
{
    int         i, e;
    entity_t     *check, *block;
    vec3_t      mins, maxs;
    pushed_t    *p;
    vec3_t      org, org2, move2, forward, right, up;

    // find the bounding box
    for (i = 0 ; i < 3 ; i++) {
        mins[i] = pusher->absMin[i] + move[i];
        maxs[i] = pusher->absMax[i] + move[i];
    }

// we need this for pushing things later
    VectorSubtract(vec3_origin, amove, org);
    AngleVectors(org, &forward, &right, &up);

// save the pusher's original position
    pushed_p->ent = pusher;
    VectorCopy(pusher->state.origin, pushed_p->origin);
    VectorCopy(pusher->state.angles, pushed_p->angles);
#if USE_SMOOTH_DELTA_ANGLES
    if (pusher->client)
        pushed_p->deltayaw = pusher->client->playerState.pmove.deltaAngles[vec3_t::Yaw];
#endif
    pushed_p++;

// move the pusher to it's final position
    VectorAdd(pusher->state.origin, move, pusher->state.origin);
    VectorAdd(pusher->state.angles, amove, pusher->state.angles);
    gi.LinkEntity(pusher);

// see if any solid entities are inside the final position
    check = g_edicts + 1;
    for (e = 1; e < globals.num_edicts; e++, check++) {
        if (!check->inUse)
            continue;
        if (check->moveType == MoveType::Push
            || check->moveType == MoveType::Stop
            || check->moveType == MoveType::None
            || check->moveType == MoveType::NoClip
            || check->moveType == MoveType::Spectator)
            continue;

        if (!check->area.prev)
            continue;       // not linked in anywhere

        // if the entity is standing on the pusher, it will definitely be moved
        if (check->groundEntityPtr != pusher) {
            // see if the ent needs to be tested
            if (check->absMin[0] >= maxs[0]
                || check->absMin[1] >= maxs[1]
                || check->absMin[2] >= maxs[2]
                || check->absMax[0] <= mins[0]
                || check->absMax[1] <= mins[1]
                || check->absMax[2] <= mins[2])
                continue;

            // see if the ent's bbox is inside the pusher's final position
            if (!SV_TestEntityPosition(check))
                continue;
            
        }

        if ((pusher->moveType == MoveType::Push) || (check->groundEntityPtr == pusher)) {
            // move this entity
            pushed_p->ent = check;
            VectorCopy(check->state.origin, pushed_p->origin);
            VectorCopy(check->state.angles, pushed_p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (check->client)
                pushed_p->deltayaw = check->client->playerState.pmove.deltaAngles[vec3_t::Yaw];
#endif
            pushed_p++;

            // try moving the contacted entity
            VectorAdd(check->state.origin, move, check->state.origin);
#if USE_SMOOTH_DELTA_ANGLES
            if (check->client) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
                check->client->playerState.pmove.deltaAngles[vec3_t::Yaw] += amove[vec3_t::Yaw];
            }
#endif

            // figure movement due to the pusher's amove
            VectorSubtract(check->state.origin, pusher->state.origin, org);
            org2[0] = DotProduct(org, forward);
            org2[1] = -DotProduct(org, right);
            org2[2] = DotProduct(org, up);
            VectorSubtract(org2, org, move2);
            VectorAdd(check->state.origin, move2, check->state.origin);

            // may have pushed them off an edge
            if (check->groundEntityPtr != pusher)
                check->groundEntityPtr = NULL;

            block = SV_TestEntityPosition(check);
            if (!block) {
                // pushed ok
                gi.LinkEntity(check);
                // impact?
                continue;
            }

            // if it is ok to leave in the old position, do it
            // this is only relevent for riding entities, not pushed
            // FIXME: this doesn't acount for rotation
            check->state.origin -= move;
            block = SV_TestEntityPosition(check);
            if (!block) {
                pushed_p--;
                continue;
            }
        }

        // save off the obstacle so we can call the block function
        obstacle = check;

        // move back any entities we already moved
        // go backwards, so if the same entity was pushed
        // twice, it goes back to the original position
        for (p = pushed_p - 1 ; p >= pushed ; p--) {
            p->ent->state.origin = p->origin;
            p->ent->state.angles = p->angles;
#if USE_SMOOTH_DELTA_ANGLES
            if (p->ent->client) {
                p->ent->client->playerState.pmove.deltaAngles[vec3_t::Yaw] = p->deltayaw;
            }
#endif
            gi.LinkEntity(p->ent);
        }
        return false;
    }

//FIXME: is there a better way to handle this?
    // see if anything we moved has touched a trigger
    for (p = pushed_p - 1 ; p >= pushed ; p--)
        UTIL_TouchTriggers(p->ent);

    return true;
}

/*
================
SV_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
void SV_Physics_Pusher(entity_t *ent)
{
    vec3_t      move, amove;
    entity_t     *part, *mv;

    // if not a team captain, so movement will be handled elsewhere
    if (ent->flags & EntityFlags::TeamSlave)
        return;

    // make sure all team slaves can move before commiting
    // any moves or calling any Think functions
    // if the move is Blocked, all moved objects will be backed out
//retry:
    pushed_p = pushed;
    for (part = ent ; part ; part = part->teamChainPtr) {
        if (part->velocity[0] || part->velocity[1] || part->velocity[2] ||
            part->avelocity[0] || part->avelocity[1] || part->avelocity[2]
           ) {
            // object is moving
            VectorScale(part->velocity, FRAMETIME, move);
            VectorScale(part->avelocity, FRAMETIME, amove);

            if (!SV_Push(part, move, amove))
                break;  // move was Blocked
        }
    }
    if (pushed_p > &pushed[MAX_EDICTS])
        gi.Error("pushed_p > &pushed[MAX_EDICTS], memory corrupted");

    if (part) {
        // the move failed, bump all nextThink times and back out moves
        for (mv = ent ; mv ; mv = mv->teamChainPtr) {
            if (mv->nextThink > 0)
                mv->nextThink += FRAMETIME;
        }

        // if the pusher has a "Blocked" function, call it
        // otherwise, just stay in place until the obstacle is gone
        if (part->Blocked)
            part->Blocked(part, obstacle);
#if 0
        // if the pushed entity went away and the pusher is still there
        if (!obstacle->inUse && part->inUse)
            goto retry;
#endif
    } else {
        // the move succeeded, so call all Think functions
        for (part = ent ; part ; part = part->teamChainPtr) {
            SV_RunThink(part);
        }
    }
}

//==================================================================

/*
=============
SV_Physics_None

Non moving objects can only Think
=============
*/
void SV_Physics_None(entity_t *ent)
{
// regular thinking
    SV_RunThink(ent);
}

/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void SV_Physics_Noclip(entity_t *ent)
{
// regular thinking
    if (!SV_RunThink(ent))
        return;
    if (!ent->inUse)
        return;

    ent->state.angles = vec3_fmaf(ent->state.angles, FRAMETIME, ent->avelocity);
    ent->state.origin = vec3_fmaf(ent->state.origin, FRAMETIME, ent->velocity);

    gi.LinkEntity(ent);
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void SV_Physics_Toss(entity_t *ent)
{
    trace_t     trace;
    vec3_t      move;
    float       backoff;
    entity_t     *slave;
    qboolean    wasInWater;
    qboolean    isInWater;
    vec3_t      oldOrigin;

    // Regular thinking
    SV_RunThink(ent);
    if (!ent->inUse)
        return;

    // If not a team captain, so movement will be handled elsewhere
    if (ent->flags & EntityFlags::TeamSlave)
        return;

    if (ent->velocity[2] > 0)
        ent->groundEntityPtr = NULL;

    // Check for the groundentity going away
    if (ent->groundEntityPtr)
        if (!ent->groundEntityPtr->inUse)
            ent->groundEntityPtr = NULL;

    // If onground, return without moving
    if (ent->groundEntityPtr)
        return;

    // Store ent->state.origin as the old origin
    oldOrigin = ent->state.origin;

    SV_CheckVelocity(ent);

    // Add gravity
    if (ent->moveType != MoveType::Fly
        && ent->moveType != MoveType::FlyMissile)
        SV_AddGravity(ent);

    // Move angles
    ent->state.angles = vec3_fmaf(ent->state.angles, FRAMETIME, ent->avelocity);

    // Move origin
    move = vec3_scale(ent->velocity, FRAMETIME);
    trace = SV_PushEntity(ent, move);
    if (!ent->inUse)
        return;

    if (trace.fraction < 1) {
        if (ent->moveType == MoveType::Bounce)
            backoff = 1.5;
        else
            backoff = 1;

        ClipVelocity(ent->velocity, trace.plane.normal, ent->velocity, backoff);

        // stop if on ground
        if (trace.plane.normal[2] > 0.7) {
            if (ent->velocity[2] < 60 || ent->moveType != MoveType::Bounce) {
                ent->groundEntityPtr = trace.ent;
                ent->groundEntityLinkCount = trace.ent->linkCount;
                VectorCopy(vec3_origin, ent->velocity);
                VectorCopy(vec3_origin, ent->avelocity);
            }
        }

//      if (ent->Touch)
//          ent->Touch (ent, trace.ent, &trace.plane, trace.surface);
    }

    // Check for water transition
    wasInWater = (ent->waterType & CONTENTS_MASK_LIQUID);
    ent->waterType = gi.PointContents(ent->state.origin);
    isInWater = ent->waterType & CONTENTS_MASK_LIQUID;

    if (isInWater)
        ent->waterLevel = 1;
    else
        ent->waterLevel = 0;

    if (!wasInWater && isInWater)
        gi.PositionedSound(oldOrigin, g_edicts, CHAN_AUTO, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
    else if (wasInWater && !isInWater)
        gi.PositionedSound(ent->state.origin, g_edicts, CHAN_AUTO, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

    // Move teamslaves
    for (slave = ent->teamChainPtr; slave; slave = slave->teamChainPtr) {
        // Set origin and link them in.
        slave->state.origin = ent->state.origin;
        gi.LinkEntity(slave);
    }
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

//FIXME: hacked in for E3 demo
constexpr int32_t sv_stopspeed = 100;
constexpr int32_t sv_friction = 6;
constexpr int32_t sv_waterfriction = 1;

void SV_AddRotationalFriction(entity_t *ent)
{
    int     n;
    float   adjustment;

    ent->state.angles = vec3_fmaf(ent->state.angles, FRAMETIME, ent->avelocity);
    adjustment = FRAMETIME * sv_stopspeed * sv_friction;
    for (n = 0; n < 3; n++) {
        if (ent->avelocity[n] > 0) {
            ent->avelocity[n] -= adjustment;
            if (ent->avelocity[n] < 0)
                ent->avelocity[n] = 0;
        } else {
            ent->avelocity[n] += adjustment;
            if (ent->avelocity[n] > 0)
                ent->avelocity[n] = 0;
        }
    }
}

void SV_Physics_Step(entity_t *ent)
{
    qboolean    wasonground;
    qboolean    hitsound = false;
    float       *vel;
    float       speed, newspeed, control;
    float       friction;
    entity_t     *groundentity;
    int         mask;

    // airborn monsters should always check for ground
    if (!ent->groundEntityPtr)
        M_CheckGround(ent);

    groundentity = ent->groundEntityPtr;

    SV_CheckVelocity(ent);

    if (groundentity)
        wasonground = true;
    else
        wasonground = false;

    if (ent->avelocity[0] || ent->avelocity[1] || ent->avelocity[2])
        SV_AddRotationalFriction(ent);

    // add gravity except:
    //   flying monsters
    //   swimming monsters who are in the water
    if (! wasonground)
        if (!(ent->flags & EntityFlags::Fly))
            if (!((ent->flags & EntityFlags::Swim) && (ent->waterLevel > 2))) {
                if (ent->velocity[2] < sv_gravity->value * -0.1)
                    hitsound = true;
                if (ent->waterLevel == 0)
                    SV_AddGravity(ent);
            }

    // friction for flying monsters that have been given vertical velocity
    if ((ent->flags & EntityFlags::Fly) && (ent->velocity[2] != 0)) {
        speed = std::fabsf(ent->velocity[2]);
        control = speed < sv_stopspeed ? sv_stopspeed : speed;
        friction = sv_friction / 3;
        newspeed = speed - (FRAMETIME * control * friction);
        if (newspeed < 0)
            newspeed = 0;
        newspeed /= speed;
        ent->velocity[2] *= newspeed;
    }

    // friction for flying monsters that have been given vertical velocity
    if ((ent->flags & EntityFlags::Swim) && (ent->velocity[2] != 0)) {
        speed = std::fabsf(ent->velocity[2]);
        control = speed < sv_stopspeed ? sv_stopspeed : speed;
        newspeed = speed - (FRAMETIME * control * sv_waterfriction * ent->waterLevel);
        if (newspeed < 0)
            newspeed = 0;
        newspeed /= speed;
        ent->velocity[2] *= newspeed;
    }

    if (ent->velocity[2] || ent->velocity[1] || ent->velocity[0]) {
        // apply friction
        // let dead monsters who aren't completely onground slide
        if ((wasonground) || (ent->flags & (EntityFlags::Swim | EntityFlags::Fly)))
            if (!(ent->health <= 0.0 && !M_CheckBottom(ent))) {
                vel = ent->velocity;
                speed = std::sqrtf(vel[0] * vel[0] + vel[1] * vel[1]);
                if (speed) {
                    friction = sv_friction;

                    control = speed < sv_stopspeed ? sv_stopspeed : speed;
                    newspeed = speed - FRAMETIME * control * friction;

                    if (newspeed < 0)
                        newspeed = 0;
                    newspeed /= speed;

                    vel[0] *= newspeed;
                    vel[1] *= newspeed;
                }
            }

        if (ent->serverFlags & EntityServerFlags::Monster)
            mask = CONTENTS_MASK_MONSTERSOLID;
        else
            mask = CONTENTS_MASK_SOLID;
        SV_FlyMove(ent, FRAMETIME, mask);

        gi.LinkEntity(ent);
        UTIL_TouchTriggers(ent);
        if (!ent->inUse)
            return;

        if (ent->groundEntityPtr)
            if (!wasonground)
                if (hitsound)
                    gi.Sound(ent, 0, gi.SoundIndex("world/land.wav"), 1, 1, 0);
    }

// regular thinking
    SV_RunThink(ent);
}

//============================================================================
/*
================
G_RunEntity

================
*/
void G_RunEntity(entity_t *ent)
{
    if (ent->PreThink)
        ent->PreThink(ent);

    switch ((int)ent->moveType) {
    case MoveType::Push:
    case MoveType::Stop:
        SV_Physics_Pusher(ent);
        break;
    case MoveType::None:
        SV_Physics_None(ent);
        break;
    case MoveType::NoClip:
    case MoveType::Spectator:
        SV_Physics_Noclip(ent);
        break;
    case MoveType::Step:
        SV_Physics_Step(ent);
        break;
    case MoveType::Toss:
    case MoveType::Bounce:
    case MoveType::Fly:
    case MoveType::FlyMissile:
        SV_Physics_Toss(ent);
        break;
    default:
        gi.Error("SV_Physics: bad moveType %i", (int)ent->moveType);
    }
}
