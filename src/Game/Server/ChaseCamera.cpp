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
#include "ServerGameLocals.h"
#include "Entities.h"
#include "ChaseCamera.h"

// Entities.
//#include "Entities/Base/SVGBasePlayer.h"

// GameModes.
#include "Gamemodes/IGamemode.h"

// GameWorld.
#include "World/ServerGameWorld.h"

void SVG_UpdateChaseCam(SVGBasePlayer *ent)
{
    vec3_t o, ownerv, goal;
    Entity *targ;
    vec3_t forward, right;
    TraceResult trace;
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

    VectorCopy(targ->currentState.origin, ownerv);

    ownerv[2] += targ->gameEntity->GetViewHeight();

    VectorCopy(targ->client->aimAngles, angles);
    if (angles[vec3_t::Pitch] > 56)
        angles[vec3_t::Pitch] = 56;
    AngleVectors(angles, &forward, &right, NULL);
    VectorNormalize(forward);
    VectorMA(ownerv, -30, forward, o);

    if (o[2] < targ->currentState.origin[2] + 20) {
        o[2] = targ->currentState.origin[2] + 20;
	}

    // jump animation lifts
    if ( !ServerGameWorld::ValidateEntity( *targ->gameEntity->GetGroundEntityHandle() ) ) {
        o[2] += 16;
	}

    trace = gi.Trace(ownerv, vec3_zero(), vec3_zero(), o, targ, BrushContentsMask::Solid);

    VectorCopy(trace.endPosition, goal);

    VectorMA(goal, 2, forward, goal);

    // pad for floors and ceilings
    VectorCopy(goal, o);
    o[2] += 6;
    trace = gi.Trace(goal, vec3_zero(), vec3_zero(), o, targ, BrushContentsMask::Solid);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] -= 6;
    }

    VectorCopy(goal, o);
    o[2] -= 6;
    trace = gi.Trace(goal, vec3_zero(), vec3_zero(), o, targ, BrushContentsMask::Solid);
    if (trace.fraction < 1) {
        VectorCopy(trace.endPosition, goal);
        goal[2] += 6;
    }

    if (targ->gameEntity->GetDeadFlag())
        ent->GetClient()->playerState.pmove.type = EnginePlayerMoveType::Dead;
    else
        ent->GetClient()->playerState.pmove.type = EnginePlayerMoveType::Freeze;

    ent->SetOrigin(goal);

    for (i = 0; i < 3; i++)
        ent->GetClient()->playerState.pmove.deltaAngles[i] = targ->client->aimAngles[i] - ent->GetClient() ->respawn.commandViewAngles[i];

    if (targ->gameEntity->GetDeadFlag()) {
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

void SVG_ChaseNext(SVGBasePlayer *ent)
{
    Entity* e = nullptr;

    // Sanity check.
    if (!ent) {
	    return;
    }

    ServerClient* client = ent->GetClient();
    Entity*	  serverEntities = GetGameWorld()->GetPODEntities();

    if (!client->chaseTarget) {
        return;
    }

    int32_t i = client->chaseTarget - serverEntities;
    do {
        i++;
        if (i > maximumclients->value)
            i = 1;
        e = serverEntities + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.isSpectator)
            break;
    } while (e != client->chaseTarget);

    client->chaseTarget = e;
    client->updateChase = true;
}

void SVG_ChasePrev(SVGBasePlayer*ent)
{
    Entity *e = nullptr;
 
    // Sanity check.
    if (!ent) {
        return;
    }

    ServerClient* client = ent->GetClient();
    Entity* serverEntities = GetGameWorld()->GetPODEntities();

    // Sanity check.
    if (!client->chaseTarget) {
        return;
    }

    int32_t i = client->chaseTarget - serverEntities;
    do {
        i--;
        if (i < 1)
            i = maximumclients->value;
        e = serverEntities + i;
        if (!e->inUse)
            continue;
        if (!e->client->respawn.isSpectator)
            break;
    } while (e != client->chaseTarget);

    client->chaseTarget = e;
    client->updateChase = true;
}

void SVG_GetChaseTarget(SVGBasePlayer *ent)
{
    Entity *other = nullptr;
    // Sanity check.
    if (!ent) {
	    return;
    }

    ServerClient* client = ent->GetClient();
    Entity* serverEntities = GetGameWorld()->GetPODEntities();

    for (int32_t i = 1; i <= maximumclients->value; i++) {
        other = serverEntities + i;
        if (other->inUse && !other->client->respawn.isSpectator) {
            client->chaseTarget = other;
            client->updateChase = true;
            SVG_UpdateChaseCam(ent);
            return;
        }
    }
    SVG_CenterPrint(ent, "No other players to chase.");
}

