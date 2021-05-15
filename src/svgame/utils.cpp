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
// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "utils.h"


vec3_t SVG_ProjectSource(const vec3_t &point, const vec3_t &distance, const vec3_t &forward, const vec3_t &right)
{
    return vec3_t{
        point[0] + forward[0] * distance[0] + right[0] * distance[1],
        point[1] + forward[1] * distance[0] + right[1] * distance[1],
        point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2]
    };
}

vec3_t SVG_PlayerProjectSource(GameClient* client, const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right)
{
    vec3_t  _distance = distance;;

    if (client->persistent.hand == LEFT_HANDED)
        _distance[1] *= -1;
    else if (client->persistent.hand == CENTER_HANDED)
        _distance[1] = 0;

    return SVG_ProjectSource(point, _distance, forward, right);
}


vec3_t SVG_VelocityForDamage(int damage)
{
    // Pick random velocities.
    vec3_t v = {
        v[0] = 100.0 * crandom(),
        v[1] = 100.0 * crandom(),
        v[2] = 200.0 + 100.0 * random()
    };

    // Scale velocities.
    if (damage < 50)
        VectorScale(v, 0.7, v);
    else
        VectorScale(v, 1.2, v);

    // Return.
    return v;
}

void Think_Delay(Entity *ent)
{
    UTIL_UseTargets(ent, ent->activator);
    SVG_FreeEntity(ent);
}

/*
==============================
UTIL_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetName in all entities that
match (string)self.target and call their .use function

==============================
*/
void UTIL_UseTargets(Entity *ent, Entity *activator)
{
    Entity     *t;

//
// check for a delay
//
    if (ent->delay) {
        // create a temp object to fire at a later time
        t = SVG_Spawn();
        t->className = "DelayedUse";
        t->nextThink = level.time + ent->delay;
        t->Think = Think_Delay;
        t->activator = activator;
        if (!activator)
            gi.DPrintf("Think_Delay with no activator\n");
        t->message = ent->message;
        t->target = ent->target;
        t->killTarget = ent->killTarget;
        return;
    }


//
// print the message
//
    if ((ent->message) && !(activator->serverFlags & EntityServerFlags::Monster)) {
        gi.CenterPrintf(activator, "%s", ent->message);
        if (ent->noiseIndex)
            gi.Sound(activator, CHAN_AUTO, ent->noiseIndex, 1, ATTN_NORM, 0);
        else
            gi.Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
    }

//
// kill killtargets
//
    if (ent->killTarget) {
        t = NULL;
        while ((t = SVG_Find(t, FOFS(targetName), ent->killTarget))) {
            SVG_FreeEntity(t);
            if (!ent->inUse) {
                gi.DPrintf("entity was removed while using killtargets\n");
                return;
            }
        }
    }

//
// fire targets
//
    if (ent->target) {
        t = NULL;
        while ((t = SVG_Find(t, FOFS(targetName), ent->target))) {
            // doors fire area portals in a specific way
            if (!Q_stricmp(t->className, "func_areaportal") &&
                (!Q_stricmp(ent->className, "func_door") || !Q_stricmp(ent->className, "func_door_rotating")))
                continue;

            if (t == ent) {
                gi.DPrintf("WARNING: Entity used itself.\n");
            } else {
                if (t->Use)
                    t->Use(t, ent, activator);
            }
            if (!ent->inUse) {
                gi.DPrintf("entity was removed while using targets\n");
                return;
            }
        }
    }
}


vec3_t VEC_UP       = {0, -1, 0};
vec3_t MOVEDIR_UP   = {0, 0, 1};
vec3_t VEC_DOWN     = {0, -2, 0};
vec3_t MOVEDIR_DOWN = {0, 0, -1};

void UTIL_SetMoveDir(vec3_t &angles, vec3_t &moveDirection)
{
    if (VectorCompare(angles, VEC_UP)) {
        VectorCopy(MOVEDIR_UP, moveDirection);
    } else if (VectorCompare(angles, VEC_DOWN)) {
        VectorCopy(MOVEDIR_DOWN, moveDirection);
    } else {
        AngleVectors(angles, &moveDirection, NULL, NULL);
    }

    VectorClear(angles);
}

/*
============
G_TouchTriggers

============
*/
void UTIL_TouchTriggers(Entity *ent)
{
    int         i, num;
    Entity     *touch[MAX_EDICTS], *hit;

    // dead things don't activate triggers!
    if ((ent->client || (ent->serverFlags & EntityServerFlags::Monster)) && (ent->health <= 0))
        return;

    num = gi.BoxEntities(ent->absMin, ent->absMax, touch
                       , MAX_EDICTS, AREA_TRIGGERS);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0 ; i < num ; i++) {
        hit = touch[i];
        if (!hit->inUse)
            continue;
        if (!hit->Touch)
            continue;
        hit->Touch(hit, ent, NULL, NULL);
    }
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void    G_TouchSolids(Entity *ent)
{
    int         i, num;
    Entity     *touch[MAX_EDICTS], *hit;

    num = gi.BoxEntities(ent->absMin, ent->absMax, touch
                       , MAX_EDICTS, AREA_SOLID);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0 ; i < num ; i++) {
        hit = touch[i];
        if (!hit->inUse)
            continue;
        if (ent->Touch)
            ent->Touch(hit, ent, NULL, NULL);
        if (!ent->inUse)
            break;
    }
}




/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
SVG_KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean SVG_KillBox(Entity *ent)
{
    trace_t     tr;

    while (1) {
        tr = gi.Trace(ent->state.origin, ent->mins, ent->maxs, ent->state.origin, NULL, CONTENTS_MASK_PLAYERSOLID);
        if (!tr.ent)
            break;

        // nail it
        SVG_Damage(tr.ent, ent, ent, vec3_origin, ent->state.origin, vec3_origin, 100000, 0, DamageFlags::IgnoreProtection, MeansOfDeath::TeleFrag);

        // if we didn't kill it, fail
        if (tr.ent->solid)
            return false;
    }

    return true;        // all clear
}
