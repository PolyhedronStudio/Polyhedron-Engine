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
#include "chasecamera.h"

#include "entities/base/SVGBaseEntity.h"
#include "entities/base/PlayerClient.h"

void SVG_UpdateChaseCam(PlayerClient *ent)
{
    vec3_t o, ownerv, goal;
    Entity *targ;
    vec3_t forward, right;
    trace_t trace;
    int i;
    vec3_t angles;

    // is our chase target gone?
    if (!ent->GetClient()->chaseTarget->inUse
        || ent->GetClient()->chaseTarget->client->respawn.isSpectator) {
        Entity *old = ent->GetClient()->chaseTarget;
        SVG_ChaseNext(ent);
        if (ent->GetClient()->chaseTarget == old) {
            ent->GetClient()->chaseTarget = NULL;
            ent->GetClient()->playerState.pmove.flags &= ~PMF_NO_PREDICTION;
            return;
        }
    }

    targ = ent->GetClient()->chaseTarget;

    VectorCopy(targ->state.origin, ownerv);

    ownerv[2] += targ->classEntity->GetViewHeight();

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

    trace = gi.Trace(ownerv, vec3_zero(), vec3_zero(), o, targ, CONTENTS_MASK_SOLID);

    VectorCopy(trace.endPosition, goal);

    VectorMA(goal, 2, forward, goal);

    // pad for floors and ceilings
    VectorCopy(goal, o);
    o[2] += 6;
    trace = gi.Trace(goal, vec3_zero(), vec3_zero(), o, targ, CONTENTS_MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] -= 6;
    }

    VectorCopy(goal, o);
    o[2] -= 6;
    trace = gi.Trace(goal, vec3_zero(), vec3_zero(), o, targ, CONTENTS_MASK_SOLID);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] += 6;
    }

    if (targ->classEntity->GetDeadFlag())
        ent->GetClient()->playerState.pmove.type = EnginePlayerMoveType::Dead;
    else
        ent->GetClient()->playerState.pmove.type = EnginePlayerMoveType::Freeze;

    ent->SetOrigin(goal);

    for (i = 0; i < 3; i++)
        ent->GetClient()->playerState.pmove.deltaAngles[i] = targ->client->aimAngles[i] - ent->GetClient() ->respawn.commandViewAngles[i];

    if (targ->classEntity->GetDeadFlag()) {
        ent->GetClient()->playerState.pmove.viewAngles[vec3_t::Roll] = 40;
        ent->GetClient()->playerState.pmove.viewAngles[vec3_t::Pitch] = -15;
        ent->GetClient()->playerState.pmove.viewAngles[vec3_t::Yaw] = targ->client->killerYaw;
    } else {
        ent->GetClient()->playerState.pmove.viewAngles = targ->client->aimAngles;
        ent->GetClient()->aimAngles = targ->client->aimAngles;
    }

    ent->SetViewHeight(0);
    ent->GetClient()->playerState.pmove.flags |= PMF_NO_PREDICTION;
    ent->LinkEntity();
}

void SVG_ChaseNext(PlayerClient *ent)
{
    int i;
    Entity *e;
    ServersClient* client = ent->GetClient();

    if (!client->chaseTarget)
        return;

    i = client->chaseTarget - g_entities;
    do {
        i++;
        if (i > maximumClients->value)
            i = 1;
        e = g_entities + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.isSpectator)
            break;
    } while (e != client->chaseTarget);

    client->chaseTarget = e;
    client->updateChase = true;
}

void SVG_ChasePrev(PlayerClient*ent)
{
    int i;
    Entity *e;
    ServersClient* client = ent->GetClient();

    if (!client->chaseTarget)
        return;

    i = client->chaseTarget - g_entities;
    do {
        i--;
        if (i < 1)
            i = maximumClients->value;
        e = g_entities + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.isSpectator)
            break;
    } while (e != client->chaseTarget);

    client->chaseTarget = e;
    client->updateChase = true;
}

void SVG_GetChaseTarget(PlayerClient *ent)
{
    int i;
    Entity *other;
    ServersClient* client = ent->GetClient();

    for (i = 1; i <= maximumClients->value; i++) {
        other = g_entities + i;
        if (other->inUse && !other->client->respawn.isSpectator) {
            client->chaseTarget = other;
            client->updateChase = true;
            SVG_UpdateChaseCam(ent);
            return;
        }
    }
    gi.CenterPrintf(ent->GetServerEntity(), "No other players to chase.");
}

