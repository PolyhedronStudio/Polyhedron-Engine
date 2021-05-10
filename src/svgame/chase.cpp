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
    if (!ent->client->chaseTarget->inUse
        || ent->client->chaseTarget->client->respawn.spectator) {
        entity_t *old = ent->client->chaseTarget;
        ChaseNext(ent);
        if (ent->client->chaseTarget == old) {
            ent->client->chaseTarget = NULL;
            ent->client->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            return;
        }
    }

    targ = ent->client->chaseTarget;

    VectorCopy(targ->state.origin, ownerv);

    ownerv[2] += targ->viewHeight;

    VectorCopy(targ->client->aimAngles, angles);
    if (angles[vec3_t::Pitch] > 56)
        angles[vec3_t::Pitch] = 56;
    AngleVectors(angles, &forward, &right, NULL);
    VectorNormalize(forward);
    VectorMA(ownerv, -30, forward, o);

    if (o[2] < targ->state.origin[2] + 20)
        o[2] = targ->state.origin[2] + 20;

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
        ent->client->playerState.pmove.type = EnginePlayerMoveType::Dead;
    else
        ent->client->playerState.pmove.type = EnginePlayerMoveType::Freeze;

    VectorCopy(goal, ent->state.origin);
    for (i = 0 ; i < 3 ; i++)
        ent->client->playerState.pmove.deltaAngles[i] = targ->client->aimAngles[i] - ent->client->respawn.commandViewAngles[i];

    if (targ->deadFlag) {
        ent->client->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
        ent->client->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
        ent->client->playerState.pmove.viewAngles[vec3_t::Yaw] = targ->client->killerYaw;
    } else {
        VectorCopy(targ->client->aimAngles, ent->client->playerState.pmove.viewAngles);
        VectorCopy(targ->client->aimAngles, ent->client->aimAngles);
    }

    ent->viewHeight = 0;
    ent->client->playerState.pmove.flags |= PMF_NO_PREDICTION;
    gi.LinkEntity(ent);
}

void ChaseNext(entity_t *ent)
{
    int i;
    entity_t *e;

    if (!ent->client->chaseTarget)
        return;

    i = ent->client->chaseTarget - g_edicts;
    do {
        i++;
        if (i > maxClients->value)
            i = 1;
        e = g_edicts + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.spectator)
            break;
    } while (e != ent->client->chaseTarget);

    ent->client->chaseTarget = e;
    ent->client->updateChase = true;
}

void ChasePrev(entity_t *ent)
{
    int i;
    entity_t *e;

    if (!ent->client->chaseTarget)
        return;

    i = ent->client->chaseTarget - g_edicts;
    do {
        i--;
        if (i < 1)
            i = maxClients->value;
        e = g_edicts + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.spectator)
            break;
    } while (e != ent->client->chaseTarget);

    ent->client->chaseTarget = e;
    ent->client->updateChase = true;
}

void GetChaseTarget(entity_t *ent)
{
    int i;
    entity_t *other;

    for (i = 1; i <= maxClients->value; i++) {
        other = g_edicts + i;
        if (other->inUse && !other->client->respawn.spectator) {
            ent->client->chaseTarget = other;
            ent->client->updateChase = true;
            UpdateChaseCam(ent);
            return;
        }
    }
    gi.CenterPrintf(ent, "No other players to chase.");
}

