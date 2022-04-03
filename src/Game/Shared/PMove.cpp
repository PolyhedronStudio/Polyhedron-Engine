/***
*
*	License here.
*
*	@file
*
*	Implements the player movement logic for both client and server game modules.
*
*   The playerMoveLocals_t serves as a temporary structure which is used to store
*   data when processing the movement logic. The previous origin and velocity are
*   stored in there.
* 
*   At the start of pmove we initialize it, as well as certain pm-> variables.
* 
*   After that, we go state checking(on ladder, ducking, on-ground, in-water, etc)
*   and execute the right movement according to that.
* 
***/
#include "SharedGame.h"


/**
*   Pointer to the actual (client-/npc-)entity PlayerMove(PM) structure.
**/
static PlayerMove* pm;
cvar_t *developer;
/**
*   All of the locals will be zeroed before each player move, just to make damn sure 
*   we don't have any differences when running on the client or the server.
**/
static struct {
    vec3_t      origin;
    vec3_t      velocity;

    vec3_t      forward, right, up;
    vec3_t      forwardXY, rightXY;

    float       frameTime;

    vec3_t      previousOrigin;
    vec3_t      previousVelocity;
    qboolean    isClimbingLadder;

    // Ground trace results.
    TraceResult groundTrace;
} playerMoveLocals;

/**
*   PM_MINS and PM_MAXS are the default bounding box, scaled by PM_SCALE
*   in Pm_Init. They are referenced in a few other places e.g. to create effects
*   at a certain body position on the player model.
**/
const vec3_t PM_MINS = { -16.f, -16.f, -24.f };
const vec3_t PM_MAXS = { 16.f,  16.f,  32.f };

static const vec3_t PM_DEAD_MINS = { -16.f, -16.f, -24.f };
static const vec3_t PM_DEAD_MAXS = { 16.f,  16.f,  -4.f };

static const vec3_t PM_GIBLET_MINS = { -8.f, -8.f, -8.f };
static const vec3_t PM_GIBLET_MAXS = { 8.f,  8.f,  8.f };


/**
*
*
*   (Debug) Utility Functions.
*
*
**/

/**
*   Can be enabled on/off for client AND server indistinctively.
**/
#ifdef CGAME_INCLUDE
#define DEBUG_CLIENT_PMOVE 1
#if DEBUG_CLIENT_PMOVE == 1
// Client debug output.
static void CLGPM_Debug(const char* func, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);
    std::string str = "[CLG PM_Debug:";
    str += func;
    str += "] ";
    str += buffer;
    str += "\n";
    Com_DPrintf(str.c_str());

    va_end(args);
}
#define PM_Debug(...) CLGPM_Debug(__func__, __VA_ARGS__);
#else
static void CLGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    va_end(args);
    CLGPM_Debug
#define PM_Debug(...) CLGPM_Debug(__func__, __VA_ARGS__);
        //#define PM_Debug () void(0)
#endif // PMOVE_DEBUG
#else
#define DEBUG_SERVER_PMOVE 1
#if DEBUG_SERVER_PMOVE == 1
static void SVGPM_Debug(const char* func, const char* fmt, ...) {
    char buffer[MAX_STRING_CHARS];

    va_list args;
    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);
    std::string str = "[SVGPM_Debug:{";
    str += func;
    str += "}] ";
    str += buffer;
    str += "\n";
    Com_DPrintf(str.c_str());

    va_end(args);
}
#define PM_Debug(...) SVGPM_Debug(__func__, __VA_ARGS__);
#else
// Server debug output.
static void SVGPM_Debug(const char* func, const char* fmt, ...) {

    va_list args;
    va_start(args, fmt);

    va_end(args);
}
#define PM_Debug(...) SVGPM_Debug(__func__, __VA_ARGS__);
//#define PM_Debug (...) void(0)
#endif // PMOVE_DEBUG
#endif // CGAME_INCLUDE



/**
*   @brief  Slide off of the impacted plane.
**/
static vec3_t PM_ClipVelocity(const vec3_t in, const vec3_t normal, float bounce) {

    float backoff = vec3_dot(in, normal);

    if (backoff < 0.0f) {
        backoff *= bounce;
    } else {
        backoff /= bounce;
    }

    return in - vec3_scale(normal, backoff);
}

/**
*   @brief  Marks the specified entity as touched.
**/
static void PM_TouchEntity(struct entity_s* ent) {
    // Ensure it is valid.
    if (ent == NULL) {
        PM_Debug("ent = NULL");
        return;
    }

    // Only touch entity if we aren't at the maximum limit yet.
    if (pm->numTouchedEntities < PM_MAX_TOUCH_ENTS && ent) {
        pm->touchedEntities[pm->numTouchedEntities] = ent;
        pm->numTouchedEntities++;
    }
    else {
        // Developer print.
        PM_Debug("PM_MAX_TOUCH_ENTS(%i) amount of entities reached for this frame.", PM_MAX_TOUCH_ENTS);
    }
}



/**
*
* 
*	STEP SLIDE MOVE
*
* 
**/
/**
*   @brief  Check whether the player just stepped off of something, or not.
**/
static bool PM_CheckStep(const TraceResult * trace) {
    if (!trace->allSolid) {
        if (trace->ent && trace->plane.normal.z >= PM_STEP_NORMAL) {
            //if (trace->ent != pm->groundEntityPtr || trace->plane.dist != playerMoveLocals.groundTrace.plane.dist) {
                return true;

                // KEEP AROUND - //PM_Debug("PM_CheckStep: true");
            //}
        }
    }

    return false;
}

/**
*   @brief  Check whether the player just stepped off of something, or not.
**/
static void PM_StepDown(const TraceResult * trace) {
    // Set current origin to the trace end position.
    pm->state.origin = trace->endPosition;

    // Calculate step height.
    const float stepHeight = pm->state.origin.z - playerMoveLocals.previousOrigin.z;

    // Set step variable in case the absolute value of stepHeight is equal or heigher than PM_STEP_HEIGHT_MIN.
    if (fabsf(stepHeight) >= PM_STEP_HEIGHT_MIN) {
        pm->step = stepHeight;
        PM_Debug("Set pm->step to %f", pm->step);
    } else {
        PM_Debug("Did not set pm->step");
    }
}

/**
*   @brief  Adapted from Quake III, this function adjusts a trace so that if it starts inside of a wall,
*           it is adjusted so that the trace begins outside of the solid it impacts.
*           Returns the actual trace.
**/
const TraceResult PM_TraceCorrectAllSolid(const vec3_t & start, const vec3_t & mins, const vec3_t & maxs, const vec3_t & end) {
    // Disabled, for this we have no need. It seems to work fine at this moment without it.
    // Not getting stuck into rotating objects or what have we....
    // 
    // And otherwise, we got this solution below, which... is seemingly slow in certain cases...
#if 1
    return pm->Trace(start, mins, maxs, end);
#else
    const vec3_t offsets = { 0.f, 1.f, -1.f };

    // Jitter around
    for (int32_t i = 0; i < 3; i++) {
        for (int32_t j = 0; j < 3; j++) {
            for (int32_t k = 0; k < 3; k++) {
                // Calculate start.
                const vec3_t point = start + vec3_t{
                    offsets[i],
                    offsets[j],
                    offsets[k]
                };

                // Execute trace.
                const TraceResult trace = pm->Trace(point, mins, maxs, end);

                if (!trace.allSolid) {

                    if (i != 0 || j != 0 || k != 0) {
                        // KEEP AROUND - // PM_Debug("Fixed all-solid");
                    }

                    return trace;
                } else {
                    // KEEP AROUND - //PM_Debug("No fixed solids, aka !trace.AllSolid == FALSE");
                }
            }
        }
    }

    // KEEP AROUND - //PM_Debug("No good position");
    return pm->Trace(start, mins, maxs, end);
#endif
}

/**
*   @return True if `plane` is unique to `planes` and should be impacted, 
*           return false otherwise.
**/
static bool PM_ImpactPlane(vec3_t * planes, int32_t num_planes, const vec3_t & plane) {

    for (int32_t i = 0; i < num_planes; i++) {
        if (vec3_dot(plane, planes[i]) > 1.0f - PM_STOP_EPSILON) {
            return false;
        }
    }

    return true;
}

/**
*   @brief  Calculates a new origin, velocity, and contact entities based on the
*           movement command and world state. Returns true if not blocked.
**/
static constexpr float      MIN_STEP_NORMAL = 0.7;  // Can't step up onto very steep slopes.
static constexpr int32_t    MAX_CLIP_PLANES = 20;   // Maximum amount of planes to clip to.

static qboolean PM_StepSlideMove_(void)
{
    const int32_t numBumps = MAX_CLIP_PLANES - 2;
    vec3_t planes[MAX_CLIP_PLANES];
    int32_t bump;

    float timeRemaining = playerMoveLocals.frameTime;
    int32_t numPlanes = 0;

    // never turn against our ground plane
    if (pm->state.flags & PMF_ON_GROUND) {
        planes[numPlanes] = playerMoveLocals.groundTrace.plane.normal;
        numPlanes++;
    }

    vec3_t primal_velocity = pm->state.velocity;

    // or our original velocity
    planes[numPlanes] = vec3_normalize(pm->state.velocity);
    numPlanes++;

    for (bump = 0; bump < numBumps; bump++) {
        if (timeRemaining < (FLT_EPSILON - 1.0f)) { // out of time
            break;
        }

        // project desired destination
        vec3_t pos = vec3_fmaf(pm->state.origin, timeRemaining, pm->state.velocity);

        // trace to it
        const TraceResult trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

        // if the player is trapped in a solid, don't build up Z
        if (trace.allSolid) {
            pm->state.velocity.z = 0.0f;
            return true;
        }

        // if the trace succeeded, move some distance
        if (trace.fraction > (FLT_EPSILON - 1.0f)) {
            pm->state.origin = trace.endPosition;

            // if the trace didn't hit anything, we're done
            if (trace.fraction == 1.0f) {
                break;
            }

            // update the movement time remaining
            timeRemaining -= (timeRemaining * trace.fraction);
        }

        // store a reference to the entity for firing game events
        PM_TouchEntity(trace.ent);

        // record the impacted plane, or nudge velocity out along it
        if (PM_ImpactPlane(planes, numPlanes, trace.plane.normal)) {
            planes[numPlanes] = trace.plane.normal;
            numPlanes++;
        } else {
            // if we've seen this plane before, nudge our velocity out along it
            pm->state.velocity += trace.plane.normal;
            continue;
        }

        // and modify velocity, clipping to all impacted planes
        for (int32_t i = 0; i < numPlanes; i++) {
            vec3_t vel;

            // if velocity doesn't impact this plane, skip it
            if (vec3_dot(pm->state.velocity, planes[i]) > (FLT_EPSILON - 1.0f)) {
                continue;
            }

            // slide along the plane
            vel = PM_ClipVelocity(pm->state.velocity, planes[i], PM_CLIP_BOUNCE);

            // see if there is a second plane that the new move enters
            for (int32_t j = 0; j < numPlanes; j++) {
                vec3_t cross;

                if (j == i) {
                    continue;
                }

                // if the clipped velocity doesn't impact this plane, skip it
                if (vec3_dot(vel, planes[j]) > (FLT_EPSILON - 1.0f)) {
                    continue;
                }

                // we are now intersecting a second plane
                vel = PM_ClipVelocity(vel, planes[j], PM_CLIP_BOUNCE);

                // but if we clip against it without being deflected back, we're okay
                if (vec3_dot(vel, planes[i]) > (FLT_EPSILON - 1.0f)) {
                    continue;
                }

                // we must now slide along the crease (cross product of the planes)
                cross = vec3_cross(planes[i], planes[j]);
                cross = vec3_normalize(cross);

                const float scale = vec3_dot(cross, pm->state.velocity);
                vel = vec3_scale(cross, scale);

                // see if there is a third plane the the new move enters
                for (int32_t k = 0; k < numPlanes; k++) {

                    if (k == i || k == j) {
                        continue;
                    }

                    if (vec3_dot(vel, planes[k]) > (FLT_EPSILON - 1.0f)) {
                        continue;
                    }

                    // stop dead at a triple plane interaction
                    pm->state.velocity = vec3_zero();
                    return true;
                }
            }

            // if we have fixed all interactions, try another move
            pm->state.velocity = vel;
            break;
        }
    }

    return bump == 0;
}

/**
*   @brief  Executes the stepslide movement.
**/
static void PM_StepSlideMove(void)
{
    // Store pre-move parameters
    const vec3_t org0 = pm->state.origin;
    const vec3_t vel0 = pm->state.velocity;

    // Attempt to move; if nothing blocks us, we're done
    PM_StepSlideMove_();

    // Attempt to step down to remain on ground
    if ((pm->state.flags & PMF_ON_GROUND) && pm->moveCommand.input.upMove <= 0) {
        const vec3_t down = vec3_fmaf(pm->state.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
        const TraceResult downTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);

        if (PM_CheckStep(&downTrace)) {
            PM_StepDown(&downTrace);
        }
    }

    // If we are blocked, we will try to step over the obstacle.
    const vec3_t org1 = pm->state.origin;
    const vec3_t vel1 = pm->state.velocity;

    const vec3_t up = vec3_fmaf(org0, PM_STEP_HEIGHT, vec3_up());
    const TraceResult upTrace = PM_TraceCorrectAllSolid(org0, pm->mins, pm->maxs, up);

    if (!upTrace.allSolid) {
        // Step from the higher position, with the original velocity
        pm->state.origin = upTrace.endPosition;
        pm->state.velocity = vel0;

        PM_StepSlideMove_();

        // Settle to the new ground, keeping the step if and only if it was successful
        const vec3_t down = vec3_fmaf(pm->state.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
        const TraceResult downTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);

        if (PM_CheckStep(&downTrace)) {
            // Quake2 trick jump secret sauce
            if ((pm->state.flags & PMF_ON_GROUND) || vel0.z < PM_SPEED_UP) {
                PM_StepDown(&downTrace);
            } else {
                pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
            }

            return;
        }
    }

    // Save end results.
    pm->state.origin = org1;
    pm->state.velocity = vel1;
}



/**
*
* 
*	MOVEMENT CONDITION CHECKS/HANDLING
*
* 
**/
/**
*   @return True if the player will be eligible for trick jumping should they
*           impact the ground on this frame, false otherwise.
**/
static qboolean PM_CheckTrickJump(void) {
    // False in the following conditions.
    if (pm->groundEntityPtr) { return false; }
    if (playerMoveLocals.previousVelocity.z < PM_SPEED_UP) { return false; }
    if (pm->moveCommand.input.upMove < 1) { return false; }
    if (pm->state.flags & PMF_JUMP_HELD) { return false; }
    if (pm->state.flags & PMF_TIME_MASK) { return false; }

    // True otherwise :)
    return true;
}

/**
*   @brief  Check for jumpingand trick jumping.
*   @return True if a jump occurs, false otherwise.
**/
static qboolean PM_CheckJump(void) {
    // KEEP AROUND - //PM_Debug("PM_CheckJump");

    // Before allowing a new jump:
    // 1. Wait for landing damage to subside.
    if (pm->state.flags & PMF_TIME_LAND) {
        PM_Debug("PM_CheckJump - PMF_TIME_LAND");
        return false;
    }

    // 2. Wait for jump key to be released
    if (pm->state.flags & PMF_JUMP_HELD) {
        PM_Debug("PM_CheckJump - PMF_JUMP_HELD");
        return false;
    }

    // 3. Check if, they didn't ask to jump.
    if (pm->moveCommand.input.upMove < 1) {
        PM_Debug("PM_CheckJump - UPMOVE < 1");
        return false;
    }

    // We're ok to go, prepare for jumping.
    float jump = PM_SPEED_JUMP;

    // Factor in water level, modulate jump force based on that.
    if (pm->waterLevel > WaterLevel::Feet) {
        jump *= PM_SPEED_JUMP_MOD_WATER;
    }

    // Add in the trick jump if eligible
    if (pm->state.flags & PMF_TIME_TRICK_JUMP) {
        jump += PM_SPEED_TRICK_JUMP;

        pm->state.flags &= ~PMF_TIME_TRICK_JUMP;
        pm->state.time = 0;

        // KEEP AROUND - //PM_Debug("Trick jump: %i", pm->moveCommand.input.upMove);
    } else {
        // KEEP AROUND - //PM_Debug("Jump: %i", pm->moveCommand.input.upMove);
    }

    if (pm->state.velocity.z < 0.0f) {
        pm->state.velocity.z = jump;
    } else {
        pm->state.velocity.z += jump;
    }

    // indicate that jump is currently held
    pm->state.flags |= (PMF_JUMPED | PMF_JUMP_HELD);

    // clear the ground indicators
    pm->state.flags &= ~PMF_ON_GROUND;
    pm->groundEntityPtr = NULL;

    // we can trick jump soon
    pm->state.flags |= PMF_TIME_TRICK_START;
    pm->state.time = 100;

    return true;
}

/**
*   @brief  Sets the wished for values to crouch: pm->mins, pm->maxs, and pm->viewHeight
**/
static void PM_CheckDuck(void) {
    // Any state after dead, can be checked for here.
    if (pm->state.type >= EnginePlayerMoveType::Dead) {
        if (pm->state.type == EnginePlayerMoveType::Gib) {
            pm->state.viewOffset.z = 0.0f;
        } else {
            pm->state.viewOffset.z = -16.0f;
        }
        // Other states go here :)
    } else {

        const qboolean is_ducking = pm->state.flags & PMF_DUCKED;
        const qboolean wants_ducking = (pm->moveCommand.input.upMove < 0) && !(playerMoveLocals.isClimbingLadder);

        if (!is_ducking && wants_ducking) {
            pm->state.flags |= PMF_DUCKED;
        } else if (is_ducking && !wants_ducking) {
            const TraceResult trace = pm->Trace(pm->state.origin, pm->mins, pm->maxs, pm->state.origin);

            if (!trace.allSolid && !trace.startSolid) {
                pm->state.flags &= ~PMF_DUCKED;
            }
        }

        // For either case, ducked, or not ducked. We first change the target 
        // view height.
        // 
        // This doesn't reflect the actual bbox, since in real life your eyes 
        // aren't at the top of your skull either.
        // 
        // Considering games == faking effects to get a nice real feel... Here we go.
        const float height = pm->maxs.z - pm->mins.z;

        if (pm->state.flags & PMF_DUCKED) {
            // A nice view height value.
            const float targetViewHeight = pm->mins.z + height * 0.5f;

            // Go down. This gets LERP-ed.
            if (pm->state.viewOffset.z > targetViewHeight) {
                pm->state.viewOffset.z -= playerMoveLocals.frameTime * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.viewOffset.z < targetViewHeight) {
                pm->state.viewOffset.z = targetViewHeight;
            }

            // Change the actual bounding box to reflect ducking
            pm->maxs.z = pm->maxs.z + pm->mins.z * 0.66f;
        }
        else {
            // A nice view height value.
            const float targetViewHeight = pm->mins.z + (height - 6) * 0.9f;

            // LERP it.
            if (pm->state.viewOffset.z < targetViewHeight) { // go up
                pm->state.viewOffset.z += playerMoveLocals.frameTime * PM_SPEED_DUCK_STAND;
            }

            if (pm->state.viewOffset.z > targetViewHeight) {
                pm->state.viewOffset.z = targetViewHeight;
            }

            // No need to change the bounding box, it has already been initialized at the start of the frame.
        }
    }

    pm->state.viewOffset = pm->state.viewOffset;
}


/**
*   @brief  Check for isClimbingLadder interaction.
*   @return True if the player is on a isClimbingLadder.
**/
static qboolean PM_CheckLadder(void) {
    // If any time mask flag is set, return.
    if (pm->state.flags & PMF_TIME_MASK) {
        return false;
    }

    // Calculate a trace for determining whether there is a isClimbingLadder in front of us.
    const vec3_t pos = vec3_fmaf(pm->state.origin, 1, playerMoveLocals.forwardXY);
    const TraceResult trace = pm->Trace(pm->state.origin, pm->mins, pm->maxs, pos);

    // Found one, engage isClimbingLadder state.
    if ((trace.fraction < 1.0f) && (trace.contents & BrushContents::Ladder)) {
        // Add isClimbingLadder flag.
        pm->state.flags |= PMF_ON_LADDER;

        // No ground entity, obviously.
        pm->groundEntityPtr = NULL;

        // Remove ducked and possible ON_GROUND flags.
        pm->state.flags &= ~(PMF_ON_GROUND | PMF_DUCKED);

        return true;
    }

    return false;
}

/**
*   Checks for water exit.The player may exit the water when they can
*   see a usable step out of the water.
*
*   @return True if a water jump has occurred, false otherwise.
**/
static qboolean PM_CheckWaterJump(void) {
    if (pm->state.flags & PMF_TIME_WATER_JUMP) {
        return false;
    }

    if (pm->waterLevel != WaterLevel::Waist) {
        return false;
    }

    if (pm->moveCommand.input.upMove < 1 && pm->moveCommand.input.forwardMove < 1) {
        return false;
    }

    vec3_t pos = vec3_fmaf(pm->state.origin, 16.f, playerMoveLocals.forward);
    TraceResult trace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

    if ((trace.fraction < 1.0f) && (trace.contents & BrushContentsMask::Solid)) {

        pos.z += PM_STEP_HEIGHT + pm->maxs.z - pm->mins.z;

        trace = PM_TraceCorrectAllSolid(pos, pm->mins, pm->maxs, pos);

        if (trace.startSolid) {
            PM_Debug("Can't exit water: Blocked");
            return false;
        }

        vec3_t position2 = {
            pos.x,
            pos.y,
            pm->state.origin.z
        };

        trace = PM_TraceCorrectAllSolid(pos, pm->mins, pm->maxs, position2);

        if (!(trace.ent && trace.plane.normal.z >= PM_STEP_NORMAL)) {
            PM_Debug("Can't exit water: not a step\n");
            return false;
        }

        // Set up water velocity.
        pm->state.velocity.z = PM_SPEED_WATER_JUMP;

        // Set up the time state, JUMP == HELD, WATER JUMPING == ACTIVE
        pm->state.flags |= PMF_TIME_WATER_JUMP | PMF_JUMP_HELD;
        pm->state.time = 2000; // 2 seconds.

        return true;
    }

    return false;
}

/**
*   @brief  Checks for water interaction, accounting for player ducking, etc.
**/
static void PM_CheckWater(void) {
    // When checking for water level we first reset all to defaults for this frame.
    pm->waterLevel = WaterLevel::None;
    pm->waterType = 0;

    // Create the position for testing.
    vec3_t contentPosition = {
        pm->state.origin.x,
        pm->state.origin.y,
        // Pick the mins bounding box Z, PM_GROUND_DIST and add it to our current Z to use for testing.
        // (This should give us about the feet pos)
        pm->state.origin.z + pm->mins.z + PM_GROUND_DIST
    };

    // Perform the actual test.
    int32_t contents = pm->PointContents(contentPosition);

    // Are we in liquid? Hallelujah!
    if (contents & BrushContentsMask::Liquid) {
        // Watertype is whichever contents type we are in with at least our feet.
        pm->waterType = contents;
        pm->waterLevel = WaterLevel::Feet;

        contentPosition.z = pm->state.origin.z;

        contents = pm->PointContents(contentPosition);

        if (contents & BrushContentsMask::Liquid) {

            pm->waterType |= contents;
            pm->waterLevel = WaterLevel::Waist;

            contentPosition.z = pm->state.origin.z + pm->state.viewOffset.z + 1.0f;

            contents = pm->PointContents(contentPosition);

            if (contents & BrushContentsMask::Liquid) {
                pm->waterType |= contents;
                pm->waterLevel = WaterLevel::Under;

                pm->state.flags |= PMF_UNDER_WATER;
            }
        }
    }
}

/**
*   @brief  Checks for ground interaction, enabling trick jumpingand dealing with landings.
**/
static void PM_CheckGround(void) {
    // If we jumped, or been pushed, do not attempt to seek ground
    if (pm->state.flags & (PMF_JUMPED | PMF_TIME_PUSHED | PMF_ON_LADDER)) {
        return;
    }

    // Seek ground eagerly in case the player wishes to trick jump
    const qboolean trick_jump = PM_CheckTrickJump();
    vec3_t pos;

    if (trick_jump) {
        pos = vec3_fmaf(pm->state.origin, playerMoveLocals.frameTime, pm->state.velocity);
        pos.z -= PM_GROUND_DIST_TRICK;
    } else {
        pos = pm->state.origin;
        pos.z -= PM_GROUND_DIST;
    }

    // Seek the ground
    TraceResult trace = playerMoveLocals.groundTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos);

    // If we hit an upward facing plane, make it our ground
    if (trace.ent && trace.plane.normal.z >= PM_STEP_NORMAL) {

        // If we had no ground, then handle landing events
        if (!pm->groundEntityPtr) {

            // Any landing terminates the water jump
            if (pm->state.flags & PMF_TIME_WATER_JUMP) {
                pm->state.flags &= ~PMF_TIME_WATER_JUMP;
                pm->state.time = 0;
            }

            // Hard landings disable jumping briefly
            if (playerMoveLocals.previousVelocity.z <= PM_SPEED_LAND) {
                pm->state.flags |= PMF_TIME_LAND;
                pm->state.time = 1;

                if (playerMoveLocals.previousVelocity.z <= PM_SPEED_FALL) {
                    pm->state.time = 16;

                    if (playerMoveLocals.previousVelocity.z <= PM_SPEED_FALL_FAR) {
                        pm->state.time = 256;
                    }
                }
            } else {
                // Soft landings with upward momentum grant trick jumps
                if (trick_jump) {
                    pm->state.flags |= PMF_TIME_TRICK_JUMP;
                    pm->state.time = 32;
                }
            }
        }

        // Save a reference to the ground
        pm->state.flags |= PMF_ON_GROUND;
        pm->groundEntityPtr = trace.ent;

        // Sink down to it if not trick jumping
        if (!(pm->state.flags & PMF_TIME_TRICK_JUMP)) {
            pm->state.origin = trace.endPosition;
            pm->state.velocity = PM_ClipVelocity(pm->state.velocity, trace.plane.normal, PM_CLIP_BOUNCE);
        }
    } else {
        pm->state.flags &= ~PMF_ON_GROUND;
        pm->groundEntityPtr = NULL;
    }

    // Always touch the entity, even if we couldn't stand on it
    PM_TouchEntity(trace.ent);
}



/**
*
* 
*	PHYSICS - ACCELERATION/FRICTION/GRAVITY
*
* 
**/
/**
*   @brief  Handles friction against user intentions, and based on contents.
**/
static void PM_Friction(void) {
    vec3_t vel = pm->state.velocity;

    if (pm->state.flags & PMF_ON_GROUND) {
        vel.z = 0.0;
    }

    const float speed = vec3_length(vel);

    if (speed < 1.0f) {
        pm->state.velocity.x = pm->state.velocity.y = 0.0f;
        return;
    }

    const float control = Maxf(PM_SPEED_STOP, speed);

    float friction = 0.0;

    // SPECTATOR friction
    if (pm->state.type == PlayerMoveType::Spectator|| pm->state.type == PlayerMoveType::Noclip) {
        friction = PM_FRICT_SPECTATOR;
        // LADDER friction
    } else if (pm->state.flags & PMF_ON_LADDER) {
        friction = PM_FRICT_LADDER;
        // WATER friction.
    } else if (pm->waterLevel > WaterLevel::Feet) {
        friction = PM_FRICT_WATER;
        // GROUND friction.
    } else if (pm->state.flags & PMF_ON_GROUND) {
        if (playerMoveLocals.groundTrace.surface && (playerMoveLocals.groundTrace.surface->flags & SurfaceFlags::Slick)) {
            friction = PM_FRICT_GROUND_SLICK;
        } else {
            friction = PM_FRICT_GROUND;
        }
        // OTHER friction
    } else {
        friction = PM_FRICT_AIR;
    }

    // scale the velocity, taking care to not reverse direction
    const float scale = Maxf(0.0, speed - (friction * control * playerMoveLocals.frameTime)) / speed;

    pm->state.velocity = vec3_scale(pm->state.velocity, scale);
}

/**
*   @brief  Returns the newly user intended velocity
**/
static void PM_Accelerate(const vec3_t & dir, float speed, float acceleration) {
    const float currentSpeed = vec3_dot(pm->state.velocity, dir);
    const float add_speed = speed - currentSpeed;

    if (add_speed <= 0.0f) {
        return;
    }

    float accel_speed = acceleration * playerMoveLocals.frameTime * speed;

    if (accel_speed > add_speed) {
        accel_speed = add_speed;
    }

    pm->state.velocity = vec3_fmaf(pm->state.velocity, accel_speed, dir);
}

/**
*   @brief  Applies gravity to the current movement.
**/
static void PM_Gravity(void) {
    float gravity = pm->state.gravity;

    if (pm->waterLevel > WaterLevel::Waist) {
        gravity *= PM_GRAVITY_WATER;
    }

    pm->state.velocity.z -= gravity * playerMoveLocals.frameTime;
}

/**
*   @brief  Applies external force currents, such as water currents or conveyor belts.
**/
static void PM_ApplyCurrents(void) {
    // Start off with 0 currents.
    vec3_t current = vec3_zero();

    // add water currents
    if (pm->waterLevel) {
        if (pm->waterType & BrushContents::Current0) {
            current.x += 1.0;
        }
        if (pm->waterType & BrushContents::Current90) {
            current.y += 1.0;
        }
        if (pm->waterType & BrushContents::Current180) {
            current.x -= 1.0;
        }
        if (pm->waterType & BrushContents::Current270) {
            current.y -= 1.0;
        }
        if (pm->waterType & BrushContents::CurrentUp) {
            current.z += 1.0;
        }
        if (pm->waterType & BrushContents::CurrentDown) {
            current.z -= 1.0;
        }
    }

    // add conveyer belt velocities
    if (pm->groundEntityPtr) {
        if (playerMoveLocals.groundTrace.contents & BrushContents::Current0) {
            current.x += 1.0;
        }
        if (playerMoveLocals.groundTrace.contents & BrushContents::Current90) {
            current.y += 1.0;
        }
        if (playerMoveLocals.groundTrace.contents & BrushContents::Current180) {
            current.x -= 1.0;
        }
        if (playerMoveLocals.groundTrace.contents & BrushContents::Current270) {
            current.y -= 1.0;
        }
        if (playerMoveLocals.groundTrace.contents & BrushContents::CurrentUp) {
            current.z += 1.0;
        }
        if (playerMoveLocals.groundTrace.contents & BrushContents::CurrentDown) {
            current.z -= 1.0;
        }
    }

    if (!vec3_equal(current, vec3_zero())) {
        current = vec3_normalize(current);
    }

    pm->state.velocity = vec3_fmaf(pm->state.velocity, PM_SPEED_CURRENT, current);
}



/**
*
* 
*	PLAYER MOVEMENT STYLE IMPLEMENTATIONS
*
* 
**/
/**
*   @brief  Called when the player is climbing a isClimbingLadder.
**/
static void PM_LadderMove(void) {
    //PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_ApplyCurrents();

    // user intentions in X/Y
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->moveCommand.input.forwardMove, playerMoveLocals.forwardXY);
    vel = vec3_fmaf(vel, pm->moveCommand.input.rightMove, playerMoveLocals.rightXY);

    const float s = PM_SPEED_LADDER * 0.125f;

    // limit horizontal speed when on a isClimbingLadder
    vel.x = Clampf(vel.x, -s, s);
    vel.y = Clampf(vel.y, -s, s);
    vel.z = 0.f;

    // handle Z intentions differently
    if (std::fabsf(pm->state.velocity.z) < PM_SPEED_LADDER) {

        if ((pm->viewAngles.x <= -15.0f) && (pm->moveCommand.input.forwardMove > 0)) {
            vel.z = PM_SPEED_LADDER;
        }
        else if ((pm->viewAngles.x >= 15.0f) && (pm->moveCommand.input.forwardMove > 0)) {
            vel.z = -PM_SPEED_LADDER;
        }
        else if (pm->moveCommand.input.upMove > 0) {
            vel.z = PM_SPEED_LADDER;
        }
        else if (pm->moveCommand.input.upMove < 0) {
            vel.z = -PM_SPEED_LADDER;
        }
        else {
            vel.z = 0.0;
        }
    }

    if (pm->moveCommand.input.upMove > 0) { // avoid jumps when exiting ladders
        pm->state.flags |= PMF_JUMP_HELD;
    }

    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0, PM_SPEED_LADDER);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    PM_Accelerate(dir, speed, PM_ACCEL_LADDER);

    PM_StepSlideMove();
}

/**
*   @brief  Called when the player is jumping out of the water to a solid.
**/
static void PM_WaterJumpMove(void) {
    //PM_Debug("%s\n", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_Gravity();

    // check for a usable spot directly in front of us
    const vec3_t pos = vec3_fmaf(pm->state.origin, 30.f, playerMoveLocals.forwardXY);

    // if we've reached a usable spot, clamp the jump to avoid launching
    if (PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, pos).fraction == 1.0f) {
        pm->state.velocity.z = Clampf(pm->state.velocity.z, 0.f, PM_SPEED_JUMP);
    }

    // if we're falling back down, clear the timer to regain control
    if (pm->state.velocity.z <= 0.0f) {
        pm->state.flags &= ~PMF_TIME_MASK;
        pm->state.time = 0;
    }

    PM_StepSlideMove();
}

/**
*   @brief  Called for movements where player is in the water
**/
static void PM_WaterMove(void) {

    if (PM_CheckWaterJump()) {
        PM_WaterJumpMove();
        return;
    }

    //PM_Debug("%s\n", Vec3ToString(pm->state.origin));

    // Apply friction, slowing rapidly when first entering the water
	float speed = vec3_length(pm->state.velocity);

	for (int32_t i = speed / PM_SPEED_WATER; i >= 0; i--) {
		PM_Friction();
	}

    // And sink if idle
    if (!pm->moveCommand.input.forwardMove && !pm->moveCommand.input.rightMove && !pm->moveCommand.input.upMove) {
        if (pm->state.velocity.z > PM_SPEED_WATER_SINK) {
            PM_Gravity();
        }
    }

    // Apply currents.
    PM_ApplyCurrents();

    // user intentions on X/Y/Z
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->moveCommand.input.forwardMove, playerMoveLocals.forward);
    vel = vec3_fmaf(vel, pm->moveCommand.input.rightMove, playerMoveLocals.right);

    // add explicit Z
    vel.z += pm->moveCommand.input.upMove;

    // disable water skiing
    if (pm->waterLevel == WaterLevel::Waist) {
        vec3_t view = pm->state.origin + pm->state.viewOffset;
        view.z -= 4.0;

        if (!(pm->PointContents(view) & BrushContentsMask::Liquid)) {
            pm->state.velocity.z = Minf(pm->state.velocity.z, 0.0);
            vel.z = Minf(vel.z, 0.0);
        }
    }

    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0, PM_SPEED_WATER);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0;
    }

    PM_Accelerate(dir, speed, PM_ACCEL_WATER);

    if (pm->moveCommand.input.upMove > 0) {
        PM_StepSlideMove_();
    }
    else {
        PM_StepSlideMove();
    }
}

/**
*   @brief  Called for movements where player is in air.
**/
static void PM_AirMove(void) {

    PM_Debug("{%s}", Vec3ToString(pm->state.origin));

    PM_Friction();

    PM_Gravity();

    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->moveCommand.input.forwardMove, playerMoveLocals.forwardXY);
    vel = vec3_fmaf(vel, pm->moveCommand.input.rightMove, playerMoveLocals.rightXY);
    vel.z = 0.f;

    float max_speed = PM_SPEED_AIR;

    // Accounting for walk modulus
    if (pm->moveCommand.input.buttons & ButtonBits::Walk) {
        max_speed *= PM_SPEED_MOD_WALK;
    }

    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.f, max_speed);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.f;
    }

    float acceleration = PM_ACCEL_AIR;

    if (pm->state.flags & PMF_DUCKED) {
        acceleration *= PM_ACCEL_AIR_MOD_DUCKED;
    }

    PM_Accelerate(dir, speed, acceleration);

    PM_StepSlideMove();
}

/**
*   @brief  Called for movements where player is on ground, regardless of water level.
**/
static void PM_WalkMove(void) {
    PM_Debug("{%s}", Vec3ToString(pm->state.origin));

    // Check for beginning of a jump
    if (PM_CheckJump()) {
        PM_AirMove();
        return;
    }

    PM_Friction();

    PM_ApplyCurrents();

    // Project the desired movement into the X/Y plane
    const vec3_t forward = vec3_normalize(PM_ClipVelocity(playerMoveLocals.forwardXY, playerMoveLocals.groundTrace.plane.normal, PM_CLIP_BOUNCE));
    const vec3_t right = vec3_normalize(PM_ClipVelocity(playerMoveLocals.rightXY, playerMoveLocals.groundTrace.plane.normal, PM_CLIP_BOUNCE));

    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->moveCommand.input.forwardMove, forward);
    vel = vec3_fmaf(vel, pm->moveCommand.input.rightMove, right);

    float max_speed;

    // Clamp to max speed
    if (pm->waterLevel > WaterLevel::Feet) {
        max_speed = PM_SPEED_WATER;
    }
    else if (pm->state.flags & PMF_DUCKED) {
        max_speed = PM_SPEED_DUCKED;
    }
    else {
        max_speed = PM_SPEED_RUN;
    }

    // Accounting for walk modulus
    if (pm->moveCommand.input.buttons & ButtonBits::Walk) {
        max_speed *= PM_SPEED_MOD_WALK;
    }

    // Clamp the speed to min/max speed
    float speed;
    const vec3_t dir = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0f, max_speed);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0f;
    }

    // Accelerate based on slickness of ground surface
    const float acceleration = (playerMoveLocals.groundTrace.surface->flags & SurfaceFlags::Slick) ? PM_ACCEL_GROUND_SLICK : PM_ACCEL_GROUND;

    PM_Accelerate(dir, speed, acceleration);

    // Determine the speed after acceleration
    speed = vec3_length(pm->state.velocity);

    // Clip to the ground
    pm->state.velocity = PM_ClipVelocity(pm->state.velocity, playerMoveLocals.groundTrace.plane.normal, PM_CLIP_BOUNCE);

    // And now scale by the speed to avoid slowing down on slopes
    pm->state.velocity = vec3_normalize(pm->state.velocity);
    pm->state.velocity = vec3_scale(pm->state.velocity, speed);

    // And finally, step if moving in X/Y
    if (pm->state.velocity.x || pm->state.velocity.y) {
        PM_StepSlideMove();
    }
}

/**
*   @brief  Handles special isSpectator movement.
**/
static void PM_SpectatorMove(void) {
    //PM_Debug("%s", Vec3ToString(pm->state.origin));

    PM_Friction();

    // User intentions on X/Y/Z
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf(vel, pm->moveCommand.input.forwardMove, playerMoveLocals.forward);
    vel = vec3_fmaf(vel, pm->moveCommand.input.rightMove, playerMoveLocals.right);

    // Add explicit Z
    vel.z += pm->moveCommand.input.upMove;

    float speed;
    vel = vec3_normalize_length(vel, speed);
    speed = Clampf(speed, 0.0f, PM_SPEED_SPECTATOR);

    if (speed < PM_STOP_EPSILON) {
        speed = 0.0f;
    }

    // Accelerate
    PM_Accelerate(vel, speed, PM_ACCEL_SPECTATOR);

    // Do the move
    PM_StepSlideMove();
}

/**
*   @brief  Handles special noclip movement.
**/
static void PM_NoclipMove() {
    PM_Friction();

    // User intentions on X/Y/Z
    vec3_t vel = vec3_zero();
    vel = vec3_fmaf( vel, pm->moveCommand.input.forwardMove, playerMoveLocals.forward );
    vel = vec3_fmaf( vel, pm->moveCommand.input.rightMove, playerMoveLocals.right );

    // Add explicit Z
    vel.z += pm->moveCommand.input.upMove;

    float speed;
    vel = vec3_normalize_length( vel, speed );
    speed = Clampf( speed, 0.0f, PM_SPEED_SPECTATOR );

    if ( speed < PM_STOP_EPSILON ) {
        speed = 0.0f;
    }

    // Accelerate
    PM_Accelerate( vel, speed, PM_ACCEL_SPECTATOR );

    // Do the move
    pm->state.origin += vec3_scale( pm->state.velocity, playerMoveLocals.frameTime );
}

/**
*   @brief  Freeze Player Movement.
**/
static void PM_FreezeMove(void) {
    //PM_Debug("%s", Vec3ToString(pm->state.origin));
         
    // Wait wut? An empty functions?
    //
    // What else did you expect to find here?
    //
    // The miracles of the universe? Try the vkpt folder for that. :D <3
}

/**
*   @brief  Initializes the current set PMove pointer for another frame iteration.
**/
static void PM_Init(PlayerMove * pmove) {
    // Store pmove ptr.
    pm = pmove;

    // Set the default bounding box
    if (pm->state.type >= EnginePlayerMoveType::Dead) {
        if (pm->state.type == EnginePlayerMoveType::Gib) {
            pm->mins = PM_GIBLET_MINS;
            pm->maxs = PM_GIBLET_MAXS;
        }
        else {
            pm->mins = vec3_scale(PM_DEAD_MINS, PM_SCALE);
            pm->maxs = vec3_scale(PM_DEAD_MAXS, PM_SCALE);
        }
    }
    else {
        pm->mins = vec3_scale(PM_MINS, PM_SCALE);
        pm->maxs = vec3_scale(PM_MAXS, PM_SCALE);
    }

    // Clear out previous PM iteration results
    pm->viewAngles = vec3_zero();

    // This is important too.
    pm->numTouchedEntities = 0;
    
    // Reset water states.
    pm->waterType = 0;
    pm->waterLevel = 0;

    // Reset the flags, and step, values. These are set on a per frame basis.
    pm->state.flags &= ~(PMF_ON_GROUND | PMF_ON_LADDER);
    pm->state.flags &= ~(PMF_JUMPED | PMF_UNDER_WATER);
    pm->step = 0.0f;

    // Jump "held" also, in case its key was released.
    if (pm->moveCommand.input.upMove < 1) {
        pm->state.flags &= ~PMF_JUMP_HELD;
    }

    // Decrement the movement timer, used for "dropping" the player, landing after jumps, 
    // or falling off a ledge/slope, by the duration of the command.
    if (pm->state.time) {
        if (pm->moveCommand.input.msec >= pm->state.time) { // clear the timer and timed flags
            pm->state.flags &= ~PMF_TIME_MASK;
            pm->state.time = 0;
        }
        else { // or just decrement the timer
            pm->state.time -= pm->moveCommand.input.msec;
        }
    }
}

/**
*   @brief  Clamp angles with deltas. Ensure they pitch doesn't exceed 90 or 270
**/
static void PM_ClampAngles(void) {
    // Copy the command angles into the outgoing state
    // Do we need this check per se?
    //if (pm->state.flags & PMF_TIME_TELEPORT) {
	   // pm->viewAngles[vec3_t::PYR::Yaw] = pm->moveCommand.input.viewAngles[vec3_t::PYR::Yaw] + pm->state.deltaAngles[vec3_t::PYR::Yaw];
	   // pm->viewAngles[vec3_t::PYR::Pitch] = 0;
	   // pm->viewAngles[vec3_t::PYR::Roll] = 0;
    //} else {
	    pm->viewAngles = pm->moveCommand.input.viewAngles + pm->state.deltaAngles;
    //}

    // Clamp pitch to prevent the player from looking up or down more than 90º
    if (pm->viewAngles.x > 90.0f && pm->viewAngles.x < 270.0f) {
        pm->viewAngles.x = 90.0f;
    }
    else if (pm->viewAngles.x <= 360.0f && pm->viewAngles.x >= 270.0f) {
        pm->viewAngles.x -= 360.0f;
    }
}

/**
*   @brief  Caculate the view step value that we are at when stepping up and down a ledge/slope/stairs.
**/
static void PM_CheckViewStep(void) {
    // Add the step offset we've made on this frame
    if (pm->step) {
        pm->state.stepOffset += pm->step;
    }

    // Calculate change to the step offset
    if (pm->state.stepOffset) {
        // Calculate the step speed to interpolate at.
        const float step_speed = playerMoveLocals.frameTime * (PM_SPEED_STEP * (Maxf(1.f, fabsf(pm->state.stepOffset) / PM_STEP_HEIGHT)));

        // Are we interpolating upwards, or downwards?
        if (pm->state.stepOffset > 0) {
            pm->state.stepOffset = Maxf(0.f, pm->state.stepOffset - step_speed);
        } else {
            pm->state.stepOffset = Minf(0.f, pm->state.stepOffset + step_speed);
        }
    }
}

/**
*   @brief  Resets the current local pmove values, and stores the required data for
*           reverting an invalid pmove command.
**/
static void PM_InitLocal() {
    // Clear all PM local vars
    playerMoveLocals = {};

    // Increase frame time based on seconds.
    playerMoveLocals.frameTime = pm->moveCommand.input.msec * 0.001f;

    // Save in case we get stuck and wish to undo this move.
    playerMoveLocals.previousOrigin = pm->state.origin;
    playerMoveLocals.previousVelocity = pm->state.velocity;

    // Calculate the directional vectors for this move, and in the XY Plane.
    vec3_vectors(pm->viewAngles, &playerMoveLocals.forward, &playerMoveLocals.right, &playerMoveLocals.up);
    vec3_vectors(vec3_t{ 0.f, pm->viewAngles.y, 0.f }, &playerMoveLocals.forwardXY, &playerMoveLocals.rightXY, NULL);
}

/**
*   @brief  Actual Player Move function. Called by both, the ServerGame and ClientGame modules.
**/
void PMove(PlayerMove * pmove)
{
    // Initialize the PMove.
    PM_Init(pmove);

    // Ensure angles are clamped.
    PM_ClampAngles();

    // Initialize the locals.
    PM_InitLocal();

    // Special PM_FREEZE(Player Movement is frozen, idle, not happening!) treatment.
    if (pm->state.type == EnginePlayerMoveType::Freeze) {
        PM_FreezeMove();
        return;
    }

    // Special PM_SPECTATOR(Spectator Movement, viewing a match, etc) treatment.
    if (pm->state.type == PlayerMoveType::Spectator) {
        PM_SpectatorMove();
        return;
    }

    if (pm->state.type == PlayerMoveType::Noclip) {
        PM_NoclipMove();
        return;
    }

    // Erase input direction values in case we are dead, or something alike.
    if (pm->state.type >= EnginePlayerMoveType::Dead) {
        pm->moveCommand.input.forwardMove = 0;
        pm->moveCommand.input.rightMove = 0;
        pm->moveCommand.input.upMove = 0;
    }

    // Check for Ladders.
    PM_CheckLadder();

    // Set mins, maxs, and viewHeight
    PM_CheckDuck();

    // Check for water.
    PM_CheckWater();

    // Check for ground.
    PM_CheckGround();

    if (pm->state.flags & PMF_TIME_TELEPORT) {
        // pause in place briefly
    } else if (pm->state.flags & PMF_TIME_WATER_JUMP) {
        PM_WaterJumpMove();
    } else if (pm->state.flags & PMF_ON_LADDER) {
        PM_LadderMove();
    } else if (pm->state.flags & PMF_ON_GROUND) {
        PM_WalkMove();
        // KEEP AROUND - //PM_Debug("Onground");
    } else if (pm->waterLevel > WaterLevel::Feet) {
        PM_WaterMove();
    } else {
        PM_AirMove();
        // KEEP AROUND - //PM_Debug("Airmove");
    }

    // Check for ground at new spot.
    PM_CheckGround();

    // Check for water at new spot.
    PM_CheckWater();

    // Check for view step changes, if so, interpolate.
    PM_CheckViewStep();
}