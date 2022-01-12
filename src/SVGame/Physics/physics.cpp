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

#include "../g_local.h"
#include "../utils.h"
#include "stepmove.h"

#include "../entities/base/SVGBaseEntity.h"

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


//
//===============
// SVG_TestEntityPosition
//
//===============
//
SVGBaseEntity *SVG_TestEntityPosition(SVGBaseEntity *ent)
{
    SVGTrace trace;
    int     mask;

    if (ent->GetClipMask())
        mask = ent->GetClipMask();
    else
        mask = CONTENTS_MASK_SOLID;
    trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), ent->GetOrigin(), ent, mask);

    if (trace.startSolid)
        return g_baseEntities[0];

    return NULL;
}



//
//===============
// SVG_BoundVelocity
//
// Keeps an entity its velocity within max boundaries. (-sv_maxvelocity, sv_maxvelocity)
//===============
//
void SVG_BoundVelocity(SVGBaseEntity *ent)
{
    vec3_t velocity = ent->GetVelocity();

    ent->SetVelocity(vec3_t {
        Clampf(velocity.x, -sv_maxvelocity->value, sv_maxvelocity->value),
        Clampf(velocity.y, -sv_maxvelocity->value, sv_maxvelocity->value),
        Clampf(velocity.z, -sv_maxvelocity->value, sv_maxvelocity->value)
    });
}


//
//===============
// SVG_RunThink
//
// Runs entity thinking code for this frame if necessary
//===============
//
qboolean SVG_RunThink(SVGBaseEntity *ent)
{
    float   thinktime;

    thinktime = ent->GetNextThinkTime();
    if (thinktime <= 0)
        return true;
    if (thinktime > level.time + 0.001)
        return true;

    ent->SetNextThinkTime(0);

    #if _DEBUG
    if ( !ent->HasThinkCallback() ) {
        // Write the index, programmers may look at that thing first
        std::string errorString = "";
        if (ent->GetServerEntity()) {
            errorString += "entity (index " + std::to_string(ent->GetNumber());
        } else {
            errorString += "entity has no ServerEntity ";
        }

        // Write the targetname as well, if it exists
        if ( !ent->GetTargetName().empty() ) {
            errorString += ", name '" + ent->GetTargetName() + "'";
        }

        // Write down the C++ class name too
        errorString += ", class '";
        errorString += ent->GetTypeInfo()->className;
        errorString += "'";

        // Close it off and state what's actually going on
        errorString += ") has a nullptr think callback \n";
    //    
        gi.Error( errorString.c_str() );
    }
    #endif

    ent->Think();


    return false;
}

//
//===============
// SVG_Impact
//
// Two entities have touched, so run their touch functions
//===============
//
void SVG_Impact(SVGBaseEntity *e1, SVGTrace *trace)
{
    SVGBaseEntity *e2 = nullptr;
//  cplane_t    backplane;

    if (trace) {
        e2 = trace->ent;
    }

    if (e1->GetSolid() != Solid::Not) {
        //e1->Touch(e1, e2, &trace->plane, trace->surface);
        e1->Touch(e1, e2, &trace->plane, trace->surface);
    }

    if (e2 != nullptr && e2->GetSolid() != Solid::Not) {
        //e2->Touch(e2, e1, NULL, NULL);
        e2->Touch(e2, e1, NULL, NULL);
    }
}


//
//===============
// ClipVelocity
//
// Slide off of the impacting object returns new velocity.
//===============
//
static vec3_t ClipVelocity(const vec3_t in, const vec3_t normal, float bounce) {

    float backoff = vec3_dot(in, normal);

    if (backoff < 0.0f) {
        backoff *= bounce;
    } else {
        backoff /= bounce;
    }

    return in - vec3_scale(normal, backoff);
}


//static int ClipVelocity(const vec3_t &in, const vec3_t &normal, vec3_t &out, float overbounce)
//{
//    float   backoff;
//    float   change;
//    int     i, Blocked;
//
//    Blocked = 0;
//    if (normal[2] > 0)
//        Blocked |= 1;       // floor
//    if (!normal[2])
//        Blocked |= 2;       // step
//
//    backoff = DotProduct(in, normal) * overbounce;
//
//    for (i = 0 ; i < 3 ; i++) {
//        change = normal[i] * backoff;
//        out[i] = in[i] - change;
//        if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
//            out[i] = 0;
//    }
//
//    return Blocked;
//}


//
//===============
// SVG_FlyMove
//
// The basic solid body movement clip that slides along multiple planes
// Returns the clipflags if the velocity was modified(hit something solid)
// 1 = floor
// 2 = wall / step
// 4 = dead stop
//===============
//
#define MAX_CLIP_PLANES 5
int SVG_FlyMove(SVGBaseEntity *ent, float time, int mask)
{
    SVGBaseEntity     *hit;
    int         bumpcount, numbumps;
    vec3_t      dir;
    float       d;
    int         numplanes;
    vec3_t      planes[MAX_CLIP_PLANES];
    vec3_t      primal_velocity, original_velocity, new_velocity;
    int         i, j;
    SVGTrace     trace;
    vec3_t      end;
    float       time_left;
    int         Blocked;

    numbumps = 4;

    Blocked = 0;
    original_velocity = ent->GetVelocity();
    primal_velocity = ent->GetVelocity();

    numplanes = 0;

    time_left = time;

    ent->SetGroundEntity(nullptr);
    for (bumpcount = 0 ; bumpcount < numbumps ; bumpcount++) {
        //for (i = 0 ; i < 3 ; i++)
        //    end[i] = ent->state.origin[i] + time_left * ent->velocity[i];
        end = ent->GetOrigin() + vec3_t{ time_left, time_left, time_left } * ent->GetVelocity();

        trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), end, ent, mask);

        if (trace.allSolid) {
            // entity is trapped in another solid
            ent->SetVelocity(vec3_zero());
            return 3;
        }

        if (trace.fraction > 0) {
            // actually covered some distance
            ent->SetOrigin(trace.endPosition);
            original_velocity = ent->GetVelocity();
            numplanes = 0;
        }

        if (trace.fraction == 1)
            break;     // moved the entire distance

        hit = trace.ent;

        if (trace.plane.normal[2] > 0.7) {
            Blocked |= 1;       // floor
            if (hit->GetSolid() == Solid::BSP) {
                ent->SetGroundEntity(hit);
                ent->SetGroundEntityLinkCount(hit->GetLinkCount());
            }
        }
        if (!trace.plane.normal[2]) {
            Blocked |= 2;       // step
        }

//
// run the impact function
//
        SVG_Impact(ent, &trace);
        if (!ent->IsInUse())
            break;      // removed by the impact function


        time_left -= time_left * trace.fraction;

        // cliped to another plane
        if (numplanes >= MAX_CLIP_PLANES) {
            // this shouldn't really happen
            ent->SetVelocity(vec3_zero());
            return 3;
        }

        planes[numplanes] = trace.plane.normal;
        numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
        for (i = 0 ; i < numplanes ; i++) {
            new_velocity = ClipVelocity(original_velocity, planes[i], 1);

            for (j = 0 ; j < numplanes ; j++)
                if ((j != i) && !vec3_equal(planes[i], planes[j])) {
                    if (vec3_dot(new_velocity, planes[j]) < 0)
                        break;  // not ok
                }
            if (j == numplanes)
                break;
        }

        if (i != numplanes) {
            // go along this plane
            ent->SetVelocity(new_velocity);
        } else {
            // go along the crease
            if (numplanes != 2) {
//              gi.DPrintf ("clip velocity, numplanes == %i\n",numplanes);
                ent->SetVelocity(vec3_zero());
                return 7;
            }

            dir = vec3_cross(planes[0], planes[1]);
            d = vec3_dot(dir, ent->GetVelocity());
            ent->SetVelocity(vec3_scale(dir, d));
        }

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
        if (vec3_dot(ent->GetVelocity(), primal_velocity) <= 0) {
            ent->SetVelocity(vec3_zero());
            return Blocked;
        }
    }

    return Blocked;
}


//
//===============
// SVG_AddGravity
//
//===============
//
void SVG_AddGravity(SVGBaseEntity *ent)
{
    // Fetch velocity.
    vec3_t velocity = ent->GetVelocity();

    // Apply gravity.
    velocity.z -= ent->GetGravity() * sv_gravity->value * FRAMETIME;

    // Apply new velocity to entity.
    ent->SetVelocity(velocity);
}

//
//=============================================================================
//
//	PUSHMOVE
//
//=============================================================================
//

//
//===============
// SVG_PushEntity
//
// Does not change the entities velocity at all
//===============
//
SVGTrace SVG_PushEntity(SVGBaseEntity *ent, vec3_t push)
{
    SVGTrace trace;
    int     mask;

    // Calculate start for push.
    vec3_t start = ent->GetOrigin();

    // Calculate end for push.
    vec3_t end = start + push;

retry:
    if (ent->GetClipMask())
        mask = ent->GetClipMask();
    else
        mask = CONTENTS_MASK_SOLID;

    trace = SVG_Trace(start, ent->GetMins(), ent->GetMaxs(), end, ent, mask);

    ent->SetOrigin(trace.endPosition);
    ent->LinkEntity();

    if (trace.fraction != 1.0) {
        SVG_Impact(ent, &trace);

        // if the pushed entity went away and the pusher is still there
        if (!trace.ent->IsInUse() && ent->IsInUse()) {
            // move the pusher back and try again
            ent->SetOrigin(start);
            ent->LinkEntity();
            goto retry;
        }
    }

    if (ent->IsInUse())
        UTIL_TouchTriggers(ent);

    return trace;
}


typedef struct {
    SVGBaseEntity *ent;
    vec3_t  origin;
    vec3_t  angles;
#if USE_SMOOTH_DELTA_ANGLES
    int     deltaYaw;
#endif
} pushed_t;
pushed_t    pushed[MAX_EDICTS], *pushed_p;

SVGBaseEntity *obstacle;


//
//===============
// SVG_Push
//
// Objects need to be moved back on a failed push,
// otherwise riders would continue to slide.
//===============
//
qboolean SVG_Push(SVGBaseEntity *pusher, vec3_t move, vec3_t amove)
{
    int e;
    SVGBaseEntity* check = NULL;
    SVGBaseEntity* block = NULL;
    pushed_t    *p = NULL;
    vec3_t      org, org2, move2, forward, right, up;

    // Find the bounding box
    vec3_t mins = pusher->GetAbsoluteMin() + move;
    vec3_t maxs = pusher->GetAbsoluteMax() + move;

    // We need this for pushing things later
    VectorSubtract(vec3_zero(), amove, org);
    AngleVectors(org, &forward, &right, &up);

    // Save the pusher's original position
    pushed_p->ent = pusher;
    pushed_p->origin = pusher->GetOrigin(); // VectorCopy(pusher->state.origin, pushed_p->origin);
    pushed_p->angles = pusher->GetAngles();

#if USE_SMOOTH_DELTA_ANGLES
    if (pusher->GetClient()) {
        pushed_p->deltaYaw = pusher->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
    }
#endif
    pushed_p++;

// move the pusher to it's final position
    pusher->SetOrigin(pusher->GetOrigin() + move);
    pusher->SetAngles(pusher->GetAngles() + amove);
    pusher->LinkEntity();

// see if any solid entities are inside the final position
    for (e = 1; e < globals.numberOfEntities; e++) {
        // Fetch the base entity and ensure it is valid.
        check = g_baseEntities[e];

        if (!check)
            continue;

        // Fetch its properties to work with.
        qboolean isInUse = check->IsInUse();
        int32_t moveType = check->GetMoveType();
        vec3_t absMin = check->GetAbsoluteMin();
        vec3_t absMax = check->GetAbsoluteMax();

        if (!isInUse)
            continue;
        if (moveType == MoveType::Push
            || moveType == MoveType::Stop
            || moveType == MoveType::None
            || moveType == MoveType::NoClip
            || moveType == MoveType::Spectator)
            continue;

        if (!check->GetServerEntity()->area.prev)
            continue;       // not linked in anywhere

        // if the entity is standing on the pusher, it will definitely be moved
        if (check->GetGroundEntity() != pusher) {
            // see if the ent needs to be tested
            if (absMin[0] >= maxs[0]
                || absMin[1] >= maxs[1]
                || absMin[2] >= maxs[2]
                || absMax[0] <= mins[0]
                || absMax[1] <= mins[1]
                || absMax[2] <= mins[2])
                continue;

            // see if the ent's bbox is inside the pusher's final position
            if (!SVG_TestEntityPosition(check))
                continue;
            
        }

        if ((pusher->GetMoveType() == MoveType::Push) || (check->GetGroundEntity() == pusher)) {
            // move this entity
            pushed_p->ent = check;
            pushed_p->origin = check->GetOrigin();  //VectorCopy(check->state.origin, pushed_p->origin);
            pushed_p->angles = check->GetAngles(); //VectorCopy(check->state.angles, pushed_p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (check->GetClient())
                pushed_p->deltaYaw = check->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw];
#endif
            pushed_p++;

            // try moving the contacted entity
            check->SetOrigin(check->GetOrigin() + move);
#if USE_SMOOTH_DELTA_ANGLES
            if (check->GetClient()) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
                check->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += amove[vec3_t::Yaw];
            }
#endif

            // figure movement due to the pusher's amove
            org = check->GetOrigin() - pusher->GetOrigin(); //VectorSubtract(check->state.origin, pusher->state.origin, org);
            org2[0] = vec3_dot(org, forward);
            org2[1] = -vec3_dot(org, right);
            org2[2] = vec3_dot(org, up);
            move2 = org2 - org;
            check->SetOrigin(check->GetOrigin() + move2);//VectorAdd(check->state.origin, move2, check->state.origin);

            // may have pushed them off an edge
            if (check->GetGroundEntity() != pusher)
                check->SetGroundEntity(nullptr);

            block = SVG_TestEntityPosition(check);
            if (!block) {
                // pushed ok
                check->LinkEntity();
                // impact?
                continue;
            }

            // if it is ok to leave in the old position, do it
            // this is only relevent for riding entities, not pushed
            // FIXME: this doesn't acount for rotation
            check->SetOrigin(check->GetOrigin() - move);//check->state.origin -= move;
            block = SVG_TestEntityPosition(check);
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
            p->ent->SetOrigin(p->origin);
            p->ent->SetAngles(p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (p->ent->GetClient()) {
                p->ent->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = p->deltaYaw;
            }
#endif
            p->ent->LinkEntity();
        }
        return false;
    }

//FIXME: is there a better way to handle this?
    // see if anything we moved has touched a trigger
    for (p = pushed_p - 1 ; p >= pushed ; p--)
        UTIL_TouchTriggers(p->ent);

    return true;
}


//
//===============
// SVG_Physics_Pusher
//
// Bmodel objects don't interact with each other, but
// push all box objects
//===============
//
void SVG_Physics_Pusher(SVGBaseEntity *ent)
{
    vec3_t      move, amove;
    SVGBaseEntity     *part, *mv;

    // if not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave)
        return;

    // make sure all team slaves can move before commiting
    // any moves or calling any Think functions
    // if the move is Blocked, all moved objects will be backed out
//retry:
    pushed_p = pushed;
    for (part = ent ; part ; part = part->GetTeamChainEntity()) {
        // Fetch pusher part, its Velocity.
        vec3_t partVelocity = part->GetVelocity();

        // Fetch pusher part, its Angular Velocity.
        vec3_t partAngularVelocity = part->GetAngularVelocity();

        if (partVelocity.x || partVelocity.y || partVelocity.z ||
            partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z
            ) {
            // object is moving
            move = vec3_scale(part->GetVelocity(), FRAMETIME);
            amove = vec3_scale(part->GetAngularVelocity(), FRAMETIME);

            if (!SVG_Push(part, move, amove))
                break;  // move was Blocked
        }
    }
    if (pushed_p > &pushed[MAX_EDICTS])
        gi.Error("pushed_p > &pushed[MAX_EDICTS], memory corrupted");

    if (part) {
        // the move failed, bump all nextThinkTime times and back out moves
        for (mv = ent ; mv ; mv = mv->GetTeamChainEntity()) {
            if (mv->GetNextThinkTime() > 0)
                mv->SetNextThinkTime(mv->GetNextThinkTime() + FRAMETIME);
        }

        // if the pusher has a "Blocked" function, call it
        // otherwise, just stay in place until the obstacle is gone
        //if (obstacle) {
            part->Blocked(obstacle);
        //}

        //if (part->Blocked)
        //    part->Blocked(part, obstacle);
#if 0
        // if the pushed entity went away and the pusher is still there
        if (!obstacle->inUse && part->inUse)
            goto retry;
#endif
    } else {
        // the move succeeded, so call all Think functions
        for (part = ent ; part ; part = part->GetTeamChainEntity()) {
            SVG_RunThink(part);
        }
    }
}

//==================================================================

//
//===============
// SVG_Physics_None
//
// Non moving objects can only Think
//===============
//
void SVG_Physics_None(SVGBaseEntity *ent)
{
    SVG_RunThink(ent);
}

//
//===============
// SVG_Physics_Noclip
//
// A moving object that doesn't obey physics
//===============
//
void SVG_Physics_Noclip(SVGBaseEntity *ent)
{
// regular thinking
    if (!SVG_RunThink(ent))
        return;
    if (!ent->IsInUse())
        return;

    ent->SetAngles(vec3_fmaf(ent->GetAngles(), FRAMETIME, ent->GetAngularVelocity()));
    ent->SetOrigin(vec3_fmaf(ent->GetOrigin(), FRAMETIME, ent->GetVelocity()));

    ent->LinkEntity();
}

//
//=============================================================================
//
//	TOSS / BOUNCE
//
//=============================================================================
//

//
//===============
// SVG_Physics_Toss
//
// Toss, bounce, and fly movement.  When onground, do nothing.
//===============
//
void SVG_Physics_Toss(SVGBaseEntity *ent)
{
    // Regular thinking
    SVG_RunThink(ent);
    
    if (!ent->IsInUse())
        return;

    // If not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave)
        return;

    // IF we're moving up, we know we're not on-ground, that's for sure :)
    if (ent->GetVelocity().z > 0) {
        ent->SetGroundEntity(nullptr);
    }

    // Check for the groundEntity going away
    if (ent->GetGroundEntity()) {
        if (!ent->GetGroundEntity()->IsInUse()) {
            ent->SetGroundEntity(nullptr);
        }
    }

    // If onground, return without moving
    if (ent->GetGroundEntity()) {
        return;
    }

    // Store ent->state.origin as the old origin
    vec3_t oldOrigin = ent->GetOrigin();

    // Bound velocity within limits of sv_maxvelocity
    SVG_BoundVelocity(ent);

    // Add gravity
    if (ent->GetMoveType() != MoveType::Fly
        && ent->GetMoveType() != MoveType::FlyMissile)
        SVG_AddGravity(ent);

    // Move angles
    ent->SetAngles(vec3_fmaf(ent->GetAngles(), FRAMETIME, ent->GetAngularVelocity()));

    // Move origin
    vec3_t move = vec3_scale(ent->GetVelocity(), FRAMETIME);
    SVGTrace trace = SVG_PushEntity(ent, move);
    if (!ent->IsInUse())
        return;

    if (trace.fraction < 1) {
        float backOff = 1.f;

        // More backOff if bouncing movetype.
        if (ent->GetMoveType() == MoveType::Bounce) {
            backOff = 1.5f;
        }
        
        // Clip new velocity.
        ent->SetVelocity(ClipVelocity(ent->GetVelocity(), trace.plane.normal, backOff));

        // Stop if on ground
        if (trace.plane.normal[2] > 0.7f) {
            if (ent->GetVelocity().z < 60.f || ent->GetMoveType() != MoveType::Bounce) {
                ent->SetGroundEntity(trace.ent);
                ent->SetGroundEntityLinkCount(trace.ent->GetLinkCount());
                ent->SetVelocity(vec3_zero());
                ent->SetAngularVelocity(vec3_zero());
            }
        }

        //ent->Touch(ent, trace.ent, &trace.plane, trace.surface);
    }

    // Check for water transition, first fetch the OLD contents mask.
    qboolean wasInWater = (ent->GetWaterType() & CONTENTS_MASK_LIQUID);
    // Set new watertype based on gi.PointContents test result.
    ent->SetWaterType(gi.PointContents(ent->GetOrigin()));
    qboolean isInWater = ent->GetWaterType() & CONTENTS_MASK_LIQUID;

    // Store waterlevel.
    if (isInWater)
        ent->SetWaterLevel(1);
    else
        ent->SetWaterLevel(0);

    // Determine what sound to play.
    if (!wasInWater && isInWater)
        gi.PositionedSound(oldOrigin, g_entities, CHAN_AUTO, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
    else if (wasInWater && !isInWater)
        gi.PositionedSound(ent->GetOrigin(), g_entities, CHAN_AUTO, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

    // Move teamslaves
    for (SVGBaseEntity *slave = ent->GetTeamChainEntity(); slave; slave = slave->GetTeamChainEntity()) {
        // Set origin and link them in.
        slave->SetOrigin(ent->GetOrigin());
        slave->LinkEntity();
    }
}

//
//=============================================================================
//
//	STEPPING MOVEMENT
//
//=============================================================================
//

//FIXME: hacked in for E3 demo
constexpr float STEPMOVE_STOPSPEED = 100.f;
constexpr float STEPMOVE_FRICTION = 6.f;
constexpr float STEPMOVE_WATERFRICTION = 1.f;

//
//===============
// SVG_AddRotationalFriction
//
// Does what it says it does.
//===============
//
void SVG_AddRotationalFriction(SVGBaseEntity *ent)
{ 
    // Acquire the rotational velocity first.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    // Set angles in proper direction.
    ent->SetAngles(vec3_fmaf(ent->GetAngles(), FRAMETIME, angularVelocity));

    // Calculate adjustment to apply.
    float adjustment = FRAMETIME * STEPMOVE_STOPSPEED * STEPMOVE_FRICTION;

    // Apply adjustments.
    angularVelocity = ent->GetAngularVelocity();
    for (int32_t n = 0; n < 3; n++) {
        if (angularVelocity[n] > 0) {
            angularVelocity[n] -= adjustment;
            if (angularVelocity[n] < 0)
                angularVelocity[n] = 0;
        } else {
            angularVelocity[n] += adjustment;
            if (angularVelocity[n] > 0)
                angularVelocity[n] = 0;
        }
    }

    // Last but not least, set the new angularVelocity.
    ent->SetAngularVelocity(angularVelocity);
}

//
//===============
// SVG_Physics_Step
//
// Monsters freefall when they don't have a ground entity, otherwise
// all movement is done with discrete steps.
//
// This is also used for objects that have become still on the ground, but
// will fall if the floor is pulled out from under them.
// FIXME: is this true ?
//===============
//
void SVG_Physics_Step(SVGBaseEntity *ent)
{
    // Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;

    // If we have no ground entity.
    if (!ent->GetGroundEntity()) {
        // Ensure we check if we aren't on one in this frame already.
        SVG_StepMove_CheckGround(ent);
    }

    // Fetch ground entity pointer. (This can be the newly found entity ofc.)
    SVGBaseEntity* groundEntity = ent->GetGroundEntity();

    // Store whether we had a ground entity at all.
    qboolean wasOnGround = (groundEntity ? true : false);

    // Bound our velocity within sv_maxvelocity limits.
    SVG_BoundVelocity(ent);

    // Check for angular velocities. If found, add rotational friction.
    vec3_t angularVelocity = ent->GetAngularVelocity();

    if (angularVelocity.x || angularVelocity.y || angularVelocity.z)
        SVG_AddRotationalFriction(ent);

    // Re-ensure we fetched its latest angular velocity.
    angularVelocity = ent->GetAngularVelocity();

    // Add gravity except for: 
    // - Flying monsters
    // - Swimming monsters who are in the water
    if (!wasOnGround) {
        // If it is not a flying monster, we are done.
        if (!(ent->GetFlags() & EntityFlags::Fly)) {
            // In case the swim mosnter is not in water...
            if (!((ent->GetFlags() & EntityFlags::Swim) && (ent->GetWaterLevel() > 2))) {
                // Determine whether to play a "hit sound".
                if (ent->GetVelocity().z < sv_gravity->value * -0.1) {
                    hitSound = true;
                }

                // Add gravity in case the monster is not in water, it can't fly, so it falls.
                if (ent->GetWaterLevel() == 0) {
                    SVG_AddGravity(ent);
                }
            }
        }
    }

    // Friction for flying monsters that have been given vertical velocity
    if ((ent->GetFlags() & EntityFlags::Fly) && (ent->GetVelocity().z != 0)) {
        float speed = std::fabsf(ent->GetVelocity().z);
        float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
        float friction = STEPMOVE_FRICTION / 3;
        float newSpeed = speed - (FRAMETIME * control * friction);
        if (newSpeed < 0)
            newSpeed = 0;
        newSpeed /= speed;
        vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
    }

    // Friction for flying monsters that have been given vertical velocity
    if ((ent->GetFlags() & EntityFlags::Swim) && (ent->GetVelocity().z != 0)) {
        float speed = std::fabsf(ent->GetVelocity().z);
        float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
        float newSpeed = speed - (FRAMETIME * control * STEPMOVE_WATERFRICTION * ent->GetWaterLevel());
        if (newSpeed < 0)
            newSpeed = 0;
        newSpeed /= speed;
        vec3_t velocity = ent->GetVelocity();
        ent->SetVelocity({ velocity.x, velocity.y, velocity.z * newSpeed });
    }

    // In case we have velocity, execute movement logic.
    if (ent->GetVelocity().z || ent->GetVelocity().y || ent->GetVelocity().x) {
        // apply friction
        // let dead monsters who aren't completely onground slide
        if ((wasOnGround) || (ent->GetFlags() & (EntityFlags::Swim | EntityFlags::Fly)))
            if (!(ent->GetHealth() <= 0.0)) {
                vec3_t vel = ent->GetVelocity();
                float speed = std::sqrtf(vel[0] * vel[0] + vel[1] * vel[1]);
                if (speed) {
                    float friction = STEPMOVE_FRICTION;
                    float control = speed < STEPMOVE_STOPSPEED ? STEPMOVE_STOPSPEED : speed;
                    float newSpeed = speed - FRAMETIME * control * friction;

                    if (newSpeed < 0)
                        newSpeed = 0;
                    newSpeed /= speed;

                    vel[0] *= newSpeed;
                    vel[1] *= newSpeed;

                    // Set the velocity.
                    ent->SetVelocity(vel);
                }
            }

        // Default mask is solid.
        int32_t mask = CONTENTS_MASK_SOLID;

        // In case of a monster, monstersolid.
        if (ent->GetServerFlags() & EntityServerFlags::Monster)
            mask = CONTENTS_MASK_MONSTERSOLID;
        
        // Execute "FlyMove", essentially also our water move.
        SVG_FlyMove(ent, FRAMETIME, mask);

        // Link entity back for collision, and execute a check for touching triggers.
        ent->LinkEntity();
        UTIL_TouchTriggers(ent);

        // Can't continue if this entity wasn't in use.
        if (!ent->IsInUse())
            return;

        // Check for whether to play a land sound.
        if (ent->GetGroundEntity()) {
            if (!wasOnGround) {
                if (hitSound) {
                    SVG_Sound(ent, 0, gi.SoundIndex("world/land.wav"), 1, 1, 0);
                }
            }
        }
    }

    // Last but not least, give the entity a chance to think for this frame.
    SVG_RunThink(ent);
}

//============================================================================

//
//===============
// SVG_RunEntity
//
// Determines what kind of run/thinking method to execute.
//===============
//
void SVG_RunEntity(SVGBaseEntity *ent)
{
    //if (ent->PreThink)
    //    ent->PreThink(ent);
    int32_t moveType = ent->GetMoveType();

    switch (moveType) {
    case MoveType::Push:
    case MoveType::Stop:
        SVG_Physics_Pusher(ent);
        break;
    case MoveType::None:
        SVG_Physics_None(ent);
        break;
    case MoveType::NoClip:
    case MoveType::Spectator:
        SVG_Physics_Noclip(ent);
        break;
    case MoveType::Step:
        SVG_Physics_Step(ent);
        break;
    case MoveType::Toss:
    case MoveType::Bounce:
    case MoveType::Fly:
    case MoveType::FlyMissile:
        SVG_Physics_Toss(ent);
        break;
    default:
        gi.Error("SV_Physics: bad moveType %i", ent->GetMoveType());
    }
}
