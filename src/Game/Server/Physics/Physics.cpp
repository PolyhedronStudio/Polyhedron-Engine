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

// Core.
#include "../ServerGameLocals.h"
#include "../Entities.h"
#include "../Utilities.h"

// Step Move physics.
#include "StepMove.h"

// World.
#include "../World/Gameworld.h"
/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest. It is set for steping or walking objects

doors, plats, etc are Solid::BSP, and MoveType::Push
bonus items are Solid::Trigger touch, and MoveType::Toss
corpses are Solid::Not and MoveType::Toss
crates are Solid::BoundingBox and MoveType::Toss
walking monsters are SOLID_SLIDEBOX and MoveType::Step
flying/floating monsters are SOLID_SLIDEBOX and MoveType::Fly

solid_edge items only clip against bsp models.

*/

void SVG_PhysicsEntityWPrint(const std::string &functionName, const std::string &functionSector, const std::string& message) {
    // Only continue if developer warnings for physics are enabled.
    extern cvar_t* dev_show_physwarnings;
    if (!dev_show_physwarnings->integer) {
        return;
    }

    // Show warning.
    std::string warning = "Physics warning occured in: ";
    warning += functionName;
    warning += " ";
    warning += functionSector;
    warning += " ";
    warning += message;
    warning += "\n";
    gi.DPrintf(warning.c_str());
 //   // Write the index, programmers may look at that thing first
 //   std::string errorString = "";
 //   if (ent->GetPODEntity()) {
	//errorString += "entity (index " + std::to_string(ent->GetNumber());
 //   } else {
	//errorString += "entity has no ServerEntity ";
 //   }

 //   // Write the targetname as well, if it exists
 //   if (!ent->GetTargetName().empty()) {
	//errorString += ", name '" + ent->GetTargetName() + "'";
 //   }

 //   // Write down the C++ class name too
 //   errorString += ", class '";
 //   errorString += ent->GetTypeInfo()->classname;
 //   errorString += "'";

 //   // Close it off and state what's actually going on
 //   errorString += ") has a nullptr think callback \n";
 //   //
 //   gi.Error(errorString.c_str());
}

//===============
// SVG_TestEntityPosition
//
//===============
SVGBaseEntity *SVG_TestEntityPosition(IServerGameEntity *ent)
{
    SVGTrace trace;
    int32_t clipMask = 0;

    if (ent->GetClipMask()) {
	    clipMask = ent->GetClipMask();
    } else {
        clipMask = BrushContentsMask::Solid;
    }

    trace = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), ent->GetOrigin(), ent, clipMask);

    if (trace.startSolid) {
	    return dynamic_cast<SVGBaseEntity*>(game.world->GetWorldspawnClassEntity());
    }

    return nullptr;
}

//===============
// SVG_BoundVelocity
//
// Keeps an entity its velocity within max boundaries. (-sv_maxvelocity, sv_maxvelocity)
//===============
void SVG_BoundVelocity(IServerGameEntity *ent)
{
    vec3_t velocity = ent->GetVelocity();

    ent->SetVelocity(vec3_t {
        Clampf(velocity.x, -sv_maxvelocity->value, sv_maxvelocity->value),
        Clampf(velocity.y, -sv_maxvelocity->value, sv_maxvelocity->value),
        Clampf(velocity.z, -sv_maxvelocity->value, sv_maxvelocity->value)
    });
}

//===============
// SVG_RunThink
//
// Runs entity thinking code for this frame if necessary
//===============
qboolean SVG_RunThink(IServerGameEntity *ent)
{
    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "nullptr entity!\n");
        return false;
    }

    // Fetch think time.
    const float thinkTime = ent->GetNextThinkTime();

    if (thinkTime <= 0) {
        return true;
    } else if (thinkTime > level.time + 0.001) {// Used to be: if (thinkTime > level.time + 0.001)
        return true;
    }

    // Reset think time before thinking.
    ent->SetNextThinkTime(0);

#if _DEBUG
    if ( !ent->HasThinkCallback() ) {
        // Write the index, programmers may look at that thing first
        std::string errorString = "";
        if (ent->GetPODEntity()) {
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
        errorString += ent->GetTypeInfo()->classname;
        errorString += "'";

        // Close it off and state what's actually going on
        errorString += ") has a nullptr think callback \n";
    //    
        gi.Error( errorString.c_str() );

        // Return true.
        return true;
    }
#endif

    // Last but not least, let the entity execute its think behavior callback.
    ent->Think();


    return false;
}

//===============
// SVG_Impact
//
// Two entities have touched, so run their touch functions
//===============
void SVG_Impact(IServerGameEntity *entityA, SVGTrace *trace)
{
    IServerGameEntity* entityB = nullptr;
//  CollisionPlane    backplane;

    // Return in case there is no entity to to test with (invalid pointer.)
    if (!entityA) {
        gi.DPrintf("Warning: Tried to call SVG_Impact with a nullptr entity!\n");
    }

    // If the impact came from a trace, set entityB to this ent.
    if (trace && trace->ent) {
        entityB = trace->ent;
    }

    // Execute touch functionality for entityA if its solid is not a Solid::Not.
    if (entityA->GetSolid() != Solid::Not) {
        //e1->DispatchTouchCallback(e1, e2, &trace->plane, trace->surface);
        entityA->DispatchTouchCallback(entityA, entityB, &trace->plane, trace->surface);
    }

    // Execute touch functionality for entityB exists, and is not a Solid::Not.
    if (entityB != nullptr && entityB->GetSolid() != Solid::Not) {
        //e2->DispatchTouchCallback(e2, e1, NULL, NULL);
        entityB->DispatchTouchCallback(entityB, entityA, nullptr, nullptr);
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
#define MAX_CLIP_PLANES 20
int SVG_FlyMove(IServerGameEntity *ent, float time, int mask)
{
    IServerGameEntity     *hit;
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

    numbumps = MAX_CLIP_PLANES - 2;

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
void SVG_AddGravity(IServerGameEntity *ent)
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
SVGTrace SVG_PushEntity(IServerGameEntity *ent, vec3_t push)
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
        mask = BrushContentsMask::Solid;

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
    SGEntityHandle ent;
    vec3_t  origin;
    vec3_t  angles;
#if USE_SMOOTH_DELTA_ANGLES
    int     deltaYaw;
#endif
} pushed_t;
pushed_t    pushed[MAX_EDICTS], *pushed_p;

IServerGameEntity *obstacle = nullptr;


//
//===============
// SVG_Push
//
// Objects need to be moved back on a failed push,
// otherwise riders would continue to slide.
//===============
//
qboolean SVG_Push(SGEntityHandle &entityHandle, vec3_t move, vec3_t amove)
{
    int e;
    IServerGameEntity* check = NULL;
    IServerGameEntity* block = NULL;
    pushed_t    *p = NULL;
    vec3_t      org, org2, move2, forward, right, up;

    // Assign handle to base entity.
    IServerGameEntity* pusher = *entityHandle;

    // Ensure it is a valid entity.
    if (!pusher) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "called with an invalid entity handle!\n");
	    return false;
    }

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
    IServerGameEntity** classEntities = game.world->GetClassEntities();
    for (e = 1; e < globals.numberOfEntities; e++) {
        // Fetch the base entity and ensure it is valid.
        //check = g_baseEntities[e];
	    SGEntityHandle checkHandle = classEntities[e];

        if (!checkHandle) {
    		SVG_PhysicsEntityWPrint(__func__, "[solid entity loop]", "got an invalid entity handle!\n");
		    continue;
	    }

        // Acquire base entity pointer.
        IServerGameEntity *check = *checkHandle;

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

        if (!check->GetPODEntity()->area.prev)
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
	        // Fetch pusher's base entity.
            IServerGameEntity* pusherEntity = *p->ent;

            // Ensure we are dealing with a valid pusher entity.
            if (!pusherEntity) {
    		    SVG_PhysicsEntityWPrint(__func__, "[move back loop]", "got an invalid entity handle!\n");
                continue;
            }

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
    for (p = pushed_p - 1; p >= pushed; p--) {
        // Fetch pusher's base entity.
        IServerGameEntity* pusherEntity = *p->ent;

        // Ensure we are dealing with a valid pusher entity.
	    if (!pusherEntity) {
		    SVG_PhysicsEntityWPrint(__func__, "[was moved loop] ", "got an invalid entity handle!\n");
            continue;
	    }

	    UTIL_TouchTriggers(*p->ent);
    }

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
void SVG_Physics_Pusher(SGEntityHandle &entityHandle)
{
    vec3_t      move, amove;
    IServerGameEntity     *part = nullptr, *mv = nullptr;

    // Assign handle to base entity.
    IServerGameEntity* ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
    	SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

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
            partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z) 
        {
            // object is moving
            move = vec3_scale(part->GetVelocity(), FRAMETIME);
            amove = vec3_scale(part->GetAngularVelocity(), FRAMETIME);

            SGEntityHandle partHandle(part);
            if (!SVG_Push(partHandle, move, amove))
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
        if (obstacle) {
            part->DispatchBlockedCallback(obstacle);
        }

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

/**
*   @brief  Executes MoveType::None behavior for said entity. Meaning it
*           only gets a chance to think.
**/
void SVG_Physics_None(SGEntityHandle& entityHandle) {
    // Assign handle to base entity.
    IServerGameEntity* ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Run think method.
    SVG_RunThink(ent);
}

/**
*   @brief  Executes MoveType::Noclip, behavior on said entity meaning it
*           does not clip to world, or respond to other entities.
**/
void SVG_Physics_Noclip(SGEntityHandle &entityHandle) {
    // Assign handle to base entity.
    IServerGameEntity* ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // regular thinking
    if (!SVG_RunThink(ent))
        return;
    if (!ent->IsInUse())
        return;

    ent->SetAngles(vec3_fmaf(ent->GetAngles(), FRAMETIME, ent->GetAngularVelocity()));
    ent->SetOrigin(vec3_fmaf(ent->GetOrigin(), FRAMETIME, ent->GetVelocity()));

    ent->LinkEntity();
}


//=============================================================================
//
//	TOSS / BOUNCE
//
//=============================================================================
/**
*   @brief  Executes MoveType::Toss, TossSlide, Bounce and Fly movement behaviors on
*           said entity.
**/
void SVG_Physics_Toss(SGEntityHandle& entityHandle) {
    // Assign handle to base entity.
    IServerGameEntity* ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
    	SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

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
    if (*ent->GetGroundEntity()) {
        if (!ent->GetGroundEntity()->IsInUse()) {
            ent->SetGroundEntity(nullptr);
        }
    }

    // If onground, return without moving
    if (*ent->GetGroundEntity() && ent->GetMoveType() != MoveType::TossSlide) {
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
	        if (ent->GetVelocity().z < 60.f || (ent->GetMoveType() != MoveType::Bounce)) {
                ent->SetGroundEntity(trace.ent);
                ent->SetGroundEntityLinkCount(trace.ent->GetLinkCount());
                ent->SetVelocity(vec3_zero());
                ent->SetAngularVelocity(vec3_zero());
            }
        }

        //ent->DispatchTouchCallback(ent, trace.ent, &trace.plane, trace.surface);
    }

    // Check for water transition, first fetch the OLD contents mask.
    qboolean wasInWater = (ent->GetWaterType() & BrushContentsMask::Liquid);
    // Set new watertype based on gi.PointContents test result.
    ent->SetWaterType(gi.PointContents(ent->GetOrigin()));
    qboolean isInWater = ent->GetWaterType() & BrushContentsMask::Liquid;

    // Store waterlevel.
    if (isInWater)
        ent->SetWaterLevel(1);
    else
        ent->SetWaterLevel(0);

    // Determine what sound to play.
    if (!wasInWater && isInWater)
        gi.PositionedSound(oldOrigin, game.world->GetServerEntities(), SoundChannel::Auto, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
    else if (wasInWater && !isInWater)
        gi.PositionedSound(ent->GetOrigin(), game.world->GetServerEntities(), SoundChannel::Auto, gi.SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

    // Move teamslaves
    for (IServerGameEntity *slave = ent->GetTeamChainEntity(); slave; slave = slave->GetTeamChainEntity()) {
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
void SVG_AddRotationalFriction(SGEntityHandle entityHandle) { 
    // Assign handle to base entity.
    IServerGameEntity *ent = *entityHandle;

    // Ensure it is a valid entity.
    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

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
void SVG_Physics_Step(SGEntityHandle &entityHandle)
{
    // Stores whether to play a "surface hit" sound.
    qboolean    hitSound = false;

    // Check if handle is valid.    
    IServerGameEntity *ent = *entityHandle;

    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Retrieve ground entity.
    IServerGameEntity* groundEntity = *ent->GetGroundEntity();

    // If we have no ground entity.
    if (!groundEntity) {
        // Ensure we check if we aren't on one in this frame already.
        SVG_StepMove_CheckGround(ent);
    }

    //if (!groundEntity) {
    //    return;
    //}

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
        int32_t mask = BrushContentsMask::Solid;

        // In case of a monster, monstersolid.
        if (ent->GetServerFlags() & EntityServerFlags::Monster)
            mask = BrushContentsMask::MonsterSolid;
        
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
void SVG_RunEntity(SGEntityHandle &entityHandle)
{
    IServerGameEntity *ent = *entityHandle;

    if (!ent) {
	    SVG_PhysicsEntityWPrint(__func__, "[start of]", "got an invalid entity handle!\n");
        return;
    }

    // Execute the proper physics that belong to its movetype.
    int32_t moveType = ent->GetMoveType();

    switch (moveType) {
        case MoveType::Push:
        case MoveType::Stop:
	        SVG_Physics_Pusher(entityHandle);
        break;
        case MoveType::None:
	        SVG_Physics_None(entityHandle);
        break;
        case MoveType::NoClip:
        case MoveType::Spectator:
	        SVG_Physics_Noclip(entityHandle);
        break;
        case MoveType::Step:
            SVG_Physics_Step(entityHandle);
        break;
        case MoveType::Toss:
	    case MoveType::TossSlide:
        case MoveType::Bounce:
        case MoveType::Fly:
        case MoveType::FlyMissile:
	        SVG_Physics_Toss(entityHandle);
        break;
    default:
        gi.Error("SV_Physics: bad moveType %i", ent->GetMoveType());
    }
}
