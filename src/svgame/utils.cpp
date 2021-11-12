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
#include "entities.h"
#include "utils.h"
#include "entities/base/SVGBaseEntity.h"

vec3_t SVG_ProjectSource(const vec3_t &point, const vec3_t &distance, const vec3_t &forward, const vec3_t &right)
{
    return vec3_t{
        point[0] + forward[0] * distance[0] + right[0] * distance[1],
        point[1] + forward[1] * distance[0] + right[1] * distance[1],
        point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2]
    };
}

vec3_t SVG_PlayerProjectSource(ServersClient* client, const vec3_t& point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right)
{
    vec3_t  _distance = distance;;

    if (client->persistent.hand == LEFT_HANDED)
        _distance[1] *= -1;
    else if (client->persistent.hand == CENTER_HANDED)
        _distance[1] = 0;

    return SVG_ProjectSource(point, _distance, forward, right);
}

void Think_Delay(SVGBaseEntity *ent)
{
    //UTIL_UseTargets(ent, ent->GetActivator());
    //SVG_FreeEntity(ent->GetServerEntity());
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
UTIL_TouchTriggers

============
*/
void UTIL_TouchTriggers(SVGBaseEntity *ent)
{
    int         i, num;
    Entity* serverEntity = ent->GetServerEntity();
    Entity* touch[MAX_EDICTS], * hit;

    // dead things don't activate triggers!
    if ((ent->GetClient() || (ent->GetServerFlags() & EntityServerFlags::Monster)) && (ent->GetHealth() <= 0))
        return;

    num = gi.BoxEntities(ent->GetAbsoluteMin(), ent->GetAbsoluteMax(), touch
        , MAX_EDICTS, AREA_TRIGGERS);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0; i < num; i++) {
        hit = touch[i];
        if (!hit->inUse)
            continue;

        if (!hit->classEntity)
            continue;

        hit->classEntity->Touch(hit->classEntity, ent, NULL, NULL);
        //if (!hit->touch)
        //    continue;
        //hit->touch(hit, ent, NULL, NULL);
    }
    //// Dead things don't activate triggers!
    //if ((ent->GetClient() || (ent->GetServerFlags() & EntityServerFlags::Monster)) && (ent->GetHealth() <= 0))
    //    return;

    //// Fetch the boxed entities.
    //std::vector<SVGBaseEntity*> touched = SVG_BoxEntities(ent->GetAbsoluteMin(), ent->GetAbsoluteMax(), MAX_EDICTS, AREA_TRIGGERS);
    //int32_t size = touched.size();

    //// be careful, it is possible to have an entity in this
    //// list removed before we get to it (killtriggered)
    //for (auto& touchedEntity : touched) {
    //    if (!touchedEntity)
    //        continue;
    //    if (!touchedEntity->IsInUse())
    //        continue;

    //    touchedEntity->Touch(touchedEntity, ent, NULL, NULL);
    //}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void G_TouchSolids(SVGBaseEntity *ent)
{
    int         i, num;
    Entity* serverEntity = ent->GetServerEntity();
    Entity* touch[MAX_EDICTS], * hit;

    // dead things don't activate triggers!
    if ((ent->GetClient() || (ent->GetServerFlags() & EntityServerFlags::Monster)) && (ent->GetHealth() <= 0))
        return;

    num = gi.BoxEntities(ent->GetAbsoluteMin(), ent->GetAbsoluteMax(), touch
        , MAX_EDICTS, AREA_SOLID);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0; i < num; i++) {
        hit = touch[i];
        if (!hit->inUse)
            continue;

        if (!hit->classEntity)
            continue;

        ent->Touch(ent, hit->classEntity, NULL, NULL);
        //if (!hit->touch)
        //    continue;
        //hit->touch(hit, ent, NULL, NULL);
    }
    //// Fetch the boxed entities.
    //std::vector<SVGBaseEntity*> touched = SVG_BoxEntities(ent->GetAbsoluteMin(), ent->GetAbsoluteMax(), MAX_EDICTS, AREA_SOLID);

    //// be careful, it is possible to have an entity in this
    //// list removed before we get to it (killtriggered)
    //for (auto& touchedEntity : touched) {
    //    if (!touchedEntity)
    //        continue;
    //    if (!touchedEntity->IsInUse())
    //        continue;

    //    ent->Touch(touchedEntity, ent, NULL, NULL);

    //    if (!ent->IsInUse())
    //        break;
    //}

    //for (i = 0 ; i < num ; i++) {
    //    hit = touch[i];
    //    if (!hit->inUse)
    //        continue;
    //    if (ent->classEntity)
    //        ent->classEntity->Touch((hit->classEntity ? hit->classEntity : nullptr), (ent->classEntity ? ent->classEntity : nullptr), NULL, NULL);
    //    if (!ent->inUse)
    //        break;
    //}
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
qboolean SVG_KillBox(SVGBaseEntity *ent)
{
    SVGTrace tr;

    if (!ent)
        return false;

    while (1) {
        tr = SVG_Trace(ent->GetOrigin(), ent->GetMins(), ent->GetMaxs(), ent->GetOrigin(), NULL, CONTENTS_MASK_PLAYERSOLID);
        if (!tr.ent)
            break;

        // nail it
        SVG_InflictDamage(tr.ent, ent, ent, vec3_zero(), ent->GetOrigin(), vec3_zero(), 100000, 0, DamageFlags::IgnoreProtection, MeansOfDeath::TeleFrag);

        // if we didn't kill it, fail
        if (tr.ent->GetSolid())
            return false;
    }

    return true;        // all clear
}
