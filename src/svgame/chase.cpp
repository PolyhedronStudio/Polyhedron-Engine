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
#include "g_local.h"

void UpdateChaseCam(entity_t *ent)
{
    vec3_t o, ownerv, goal;
    entity_t *targ;
    vec3_t forward, right;
    trace_t trace;
    int i;
    vec3_t angles;

    // is our chase target gone?
    if (!ent->client->chase_target->inUse
        || ent->client->chase_target->client->resp.spectator) {
        entity_t *old = ent->client->chase_target;
        ChaseNext(ent);
        if (ent->client->chase_target == old) {
            ent->client->chase_target = NULL;
            ent->client->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            return;
        }
    }

    targ = ent->client->chase_target;

    VectorCopy(targ->s.origin, ownerv);

    ownerv[2] += targ->viewHeight;

    VectorCopy(targ->client->v_angle, angles);
    if (angles[vec3_t::Pitch] > 56)
        angles[vec3_t::Pitch] = 56;
    AngleVectors(angles, &forward, &right, NULL);
    VectorNormalize(forward);
    VectorMA(ownerv, -30, forward, o);

    if (o[2] < targ->s.origin[2] + 20)
        o[2] = targ->s.origin[2] + 20;

    // jump animation lifts
    if (!targ->groundEntityPtr)
        o[2] += 16;

    trace = gi.Trace(ownerv, vec3_origin, vec3_origin, o, targ, CONTENTS_MASK_SOLID);

    VectorCopy(trace.endPosition, goal);

    VectorMA(goal, 2, forward, goal);

    // pad for floors and ceilings
    VectorCopy(goal, o);
    o[2] += 6;
    trace = gi.Trace(goal, vec3_origin, vec3_origin, o, targ, CONTENTS_MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] -= 6;
    }

    VectorCopy(goal, o);
    o[2] -= 6;
    trace = gi.Trace(goal, vec3_origin, vec3_origin, o, targ, CONTENTS_MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] += 6;
    }

    if (targ->deadFlag)
        ent->client->playerState.pmove.type = PM_DEAD;
    else
        ent->client->playerState.pmove.type = PM_FREEZE;

    VectorCopy(goal, ent->s.origin);
    for (i = 0 ; i < 3 ; i++)
        ent->client->playerState.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

    if (targ->deadFlag) {
        ent->client->playerState.viewAngles[vec3_t::Roll] = 40;
        ent->client->playerState.viewAngles[vec3_t::Pitch] = -15;
        ent->client->playerState.viewAngles[vec3_t::Yaw] = targ->client->killer_yaw;
    } else {
        VectorCopy(targ->client->v_angle, ent->client->playerState.viewAngles);
        VectorCopy(targ->client->v_angle, ent->client->v_angle);
    }

    ent->viewHeight = 0;
    ent->client->playerState.pmove.flags |= PMF_NO_PREDICTION;
    gi.LinkEntity(ent);
}

void ChaseNext(entity_t *ent)
{
    int i;
    entity_t *e;

    if (!ent->client->chase_target)
        return;

    i = ent->client->chase_target - g_edicts;
    do {
        i++;
        if (i > maxclients->value)
            i = 1;
        e = g_edicts + i;
        if (!e->inUse)
            continue;
        if (!e->client->resp.spectator)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = true;
}

void ChasePrev(entity_t *ent)
{
    int i;
    entity_t *e;

    if (!ent->client->chase_target)
        return;

    i = ent->client->chase_target - g_edicts;
    do {
        i--;
        if (i < 1)
            i = maxclients->value;
        e = g_edicts + i;
        if (!e->inUse)
            continue;
        if (!e->client->resp.spectator)
            break;
    } while (e != ent->client->chase_target);

    ent->client->chase_target = e;
    ent->client->update_chase = true;
}

void GetChaseTarget(entity_t *ent)
{
    int i;
    entity_t *other;

    for (i = 1; i <= maxclients->value; i++) {
        other = g_edicts + i;
        if (other->inUse && !other->client->resp.spectator) {
            ent->client->chase_target = other;
            ent->client->update_chase = true;
            UpdateChaseCam(ent);
            return;
        }
    }
    gi.CenterPrintf(ent, "No other players to chase.");
}

