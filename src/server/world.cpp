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
// world.c -- world query functions

#include "server.h"

/*
===============================================================================

ENTITY AREA CHECKING

FIXME: this use of "area" is different from the bsp file use
===============================================================================
*/

typedef struct areanode_s {
    int     axis;       // -1 = leaf node
    float   dist;
    struct areanode_s   *children[2];
    list_t  trigger_edicts;
    list_t  solid_edicts;
} areanode_t;

#define    AREA_DEPTH    4
#define    AREA_NODES    32

static areanode_t   sv_areanodes[AREA_NODES];
static int          sv_numareanodes;

static vec3_t    area_mins, area_maxs; // MATHLIB: No more float* pointers to local func arrays.
static entity_t  **area_list;
static int      area_count, area_maxcount;
static int      area_type;

/*
===============
SV_CreateAreaNode

Builds a uniformly subdivided tree for the given world size
===============
*/
static areanode_t *SV_CreateAreaNode(int depth, const vec3_t &mins, const vec3_t &maxs)
{
    areanode_t  *anode;
    vec3_t      size;
    vec3_t      mins1, maxs1, mins2, maxs2;

    anode = &sv_areanodes[sv_numareanodes];
    sv_numareanodes++;

    List_Init(&anode->trigger_edicts);
    List_Init(&anode->solid_edicts);

    if (depth == AREA_DEPTH) {
        anode->axis = -1;
        anode->children[0] = anode->children[1] = NULL;
        return anode;
    }

    VectorSubtract(maxs, mins, size);
    if (size[0] > size[1])
        anode->axis = 0;
    else
        anode->axis = 1;

    anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
    VectorCopy(mins, mins1);
    VectorCopy(mins, mins2);
    VectorCopy(maxs, maxs1);
    VectorCopy(maxs, maxs2);

    maxs1[anode->axis] = mins2[anode->axis] = anode->dist;

    anode->children[0] = SV_CreateAreaNode(depth + 1, mins2, maxs2);
    anode->children[1] = SV_CreateAreaNode(depth + 1, mins1, maxs1);

    return anode;
}

/*
===============
SV_ClearWorld

===============
*/
void SV_ClearWorld(void)
{
    mmodel_t *cm;
    entity_t *ent;
    int i;

    memset(sv_areanodes, 0, sizeof(sv_areanodes));
    sv_numareanodes = 0;

    if (sv.cm.cache) {
        cm = &sv.cm.cache->models[0];
        SV_CreateAreaNode(0, cm->mins, cm->maxs);
    }

    // make sure all entities are unlinked
    for (i = 0; i < ge->max_edicts; i++) {
        ent = EDICT_NUM(i);
        ent->area.prev = ent->area.next = NULL;
    }
}

/*
===============
SV_EntityIsVisible

Checks if edict is potentially visible from the given PVS row.
===============
*/
qboolean SV_EntityIsVisible(cm_t *cm, entity_t *ent, byte *mask)
{
    int i;

    if (ent->numClusters == -1) {
        // too many leafs for individual check, go by headNode
        return CM_HeadnodeVisible(CM_NodeNum(cm, ent->headNode), mask);
    }

    // check individual leafs
    for (i = 0; i < ent->numClusters; i++) {
        if (Q_IsBitSet(mask, ent->clusterNumbers[i])) {
            return true;
        }
    }

    return false;  // not visible
}

/*
===============
SV_LinkEdict

General purpose routine shared between game DLL and MVD code.
Links entity to PVS leafs.
===============
*/
void SV_LinkEntity(cm_t *cm, entity_t *ent)
{
    mleaf_t     *leafs[MAX_TOTAL_ENT_LEAFS];
    int         clusters[MAX_TOTAL_ENT_LEAFS];
    int         num_leafs;
    int         i, j;
    int         area;
    mnode_t     *topnode;

    // set the size
    VectorSubtract(ent->maxs, ent->mins, ent->size);

    // set the abs box
    if (ent->solid == Solid::BSP &&
        (ent->state.angles[0] || ent->state.angles[1] || ent->state.angles[2])) {
        // expand for rotation
        float   max, v;
        int     i;

        max = 0;
        for (i = 0; i < 3; i++) {
            v = std::fabsf(ent->mins[i]);
            if (v > max)
                max = v;
            v = std::fabsf(ent->maxs[i]);
            if (v > max)
                max = v;
        }
        for (i = 0; i < 3; i++) {
            ent->absMin[i] = ent->state.origin[i] - max;
            ent->absMax[i] = ent->state.origin[i] + max;
        }
    } else {
        // normal
        VectorAdd(ent->state.origin, ent->mins, ent->absMin);
        VectorAdd(ent->state.origin, ent->maxs, ent->absMax);
    }

    // because movement is clipped an epsilon away from an actual edge,
    // we must fully check even when bounding boxes don't quite touch
    ent->absMin[0] -= 1;
    ent->absMin[1] -= 1;
    ent->absMin[2] -= 1;
    ent->absMax[0] += 1;
    ent->absMax[1] += 1;
    ent->absMax[2] += 1;

// link to PVS leafs
    ent->numClusters = 0;
    ent->areaNumber = 0;
    ent->areaNumber2 = 0;

    //get all leafs, including solids
    num_leafs = CM_BoxLeafs(cm, ent->absMin, ent->absMax,
                            leafs, MAX_TOTAL_ENT_LEAFS, &topnode);

    // set areas
    for (i = 0; i < num_leafs; i++) {
        clusters[i] = CM_LeafCluster(leafs[i]);
        area = CM_LeafArea(leafs[i]);
        if (area) {
            // doors may legally straggle two areas,
            // but nothing should evern need more than that
            if (ent->areaNumber && ent->areaNumber != area) {
                if (ent->areaNumber2 && ent->areaNumber2 != area && sv.serverState == ServerState::Loading) {
                    Com_DPrintf("Object touching 3 areas at %f %f %f\n",
                                ent->absMin[0], ent->absMin[1], ent->absMin[2]);
                }
                ent->areaNumber2 = area;
            } else
                ent->areaNumber = area;
        }
    }

    if (num_leafs >= MAX_TOTAL_ENT_LEAFS) {
        // assume we missed some leafs, and mark by headNode
        ent->numClusters = -1;
        ent->headNode = CM_NumNode(cm, topnode);
    } else {
        ent->numClusters = 0;
        for (i = 0; i < num_leafs; i++) {
            if (clusters[i] == -1)
                continue;        // not a visible leaf
            for (j = 0; j < i; j++)
                if (clusters[j] == clusters[i])
                    break;
            if (j == i) {
                if (ent->numClusters == MAX_ENT_CLUSTERS) {
                    // assume we missed some leafs, and mark by headNode
                    ent->numClusters = -1;
                    ent->headNode = CM_NumNode(cm, topnode);
                    break;
                }

                ent->clusterNumbers[ent->numClusters++] = clusters[i];
            }
        }
    }
}

void PF_UnlinkEntity(entity_t *ent)
{
    if (!ent->area.prev)
        return;        // not linked in anywhere
    List_Remove(&ent->area);
    ent->area.prev = ent->area.next = NULL;
}

void PF_LinkEntity(entity_t *ent)
{
    areanode_t *node;
    server_entity_t *sent;
    int entnum;

    if (ent->area.prev)
        PF_UnlinkEntity(ent);     // unlink from old position

    if (ent == ge->edicts)
        return;        // don't add the world

    if (!ent->inUse) {
        Com_DPrintf("%s: entity %d is not in use\n", __func__, NUM_FOR_EDICT(ent));
        return;
    }

    if (!sv.cm.cache) {
        return;
    }

    entnum = NUM_FOR_EDICT(ent);
    sent = &sv.entities[entnum];

    // encode the size into the entity_state for client prediction
    switch (ent->solid) {
    case Solid::BoundingBox:
        if ((ent->serverFlags & EntityServerFlags::DeadMonster) || VectorCompare(ent->mins, ent->maxs)) {
            ent->state.solid = 0;
            sent->solid32 = 0;
        } else {
            ent->state.solid = MSG_PackSolid32(ent->mins, ent->maxs);
            sent->solid32 = MSG_PackSolid32(ent->mins, ent->maxs);
        }
        break;
    case Solid::BSP:
        ent->state.solid = PACKED_BSP;      // a Solid::BoundingBox will never create this value
        sent->solid32 = PACKED_BSP;     // FIXME: use 255?
        break;
    default:
        ent->state.solid = 0;
        sent->solid32 = 0;
        break;
    }

    SV_LinkEntity(&sv.cm, ent);

    // if first time, make sure oldOrigin is valid
    if (!ent->linkCount) {
        VectorCopy(ent->state.origin, ent->state.oldOrigin);
    }
    ent->linkCount++;

    if (ent->solid == Solid::Not)
        return;

// find the first node that the ent's box crosses
    node = sv_areanodes;
    while (1) {
        if (node->axis == -1)
            break;
        if (ent->absMin[node->axis] > node->dist)
            node = node->children[0];
        else if (ent->absMax[node->axis] < node->dist)
            node = node->children[1];
        else
            break;        // crosses the node
    }

    // link it in
    if (ent->solid == Solid::Trigger)
        List_Append(&node->trigger_edicts, &ent->area);
    else
        List_Append(&node->solid_edicts, &ent->area);
}


/*
====================
SV_AreaEntities_r

====================
*/
static void SV_AreaEntities_r(areanode_t *node)
{
    list_t      *start;
    entity_t     *check;

    // touch linked edicts
    if (area_type == AREA_SOLID)
        start = &node->solid_edicts;
    else
        start = &node->trigger_edicts;

    LIST_FOR_EACH(entity_t, check, start, area) {
        if (check->solid == Solid::Not)
            continue;        // deactivated
        if (check->absMin[0] > area_maxs[0]
            || check->absMin[1] > area_maxs[1]
            || check->absMin[2] > area_maxs[2]
            || check->absMax[0] < area_mins[0]
            || check->absMax[1] < area_mins[1]
            || check->absMax[2] < area_mins[2])
            continue;        // not touching

        if (area_count == area_maxcount) {
            Com_WPrintf("SV_AreaEntities: MAXCOUNT\n");
            return;
        }

        area_list[area_count] = check;
        area_count++;
    }

    if (node->axis == -1)
        return;        // terminal node

    // recurse down both sides
    if (area_maxs[node->axis] > node->dist)
        SV_AreaEntities_r(node->children[0]);
    if (area_mins[node->axis] < node->dist)
        SV_AreaEntities_r(node->children[1]);
}

/*
================
SV_AreaEntities
================
*/
int SV_AreaEntities(const vec3_t &mins, const vec3_t &maxs, entity_t **list,
                  int maxcount, int areatype)
{
    area_mins = mins;
    area_maxs = maxs;
    area_list = list;
    area_count = 0;
    area_maxcount = maxcount;
    area_type = areatype;

    SV_AreaEntities_r(sv_areanodes);

    return area_count;
}


//===========================================================================

/*
================
SV_HullForEntity

Returns a headNode that can be used for testing or clipping an
object of mins/maxs size.
================
*/
static mnode_t *SV_HullForEntity(entity_t *ent)
{
    if (ent->solid == Solid::BSP) {
        int i = ent->state.modelIndex - 1;

        // explicit hulls in the BSP model
        if (i <= 0 || i >= sv.cm.cache->nummodels)
            Com_Error(ERR_DROP, "%s: inline model %d out of range", __func__, i);

        return sv.cm.cache->models[i].headNode;
    }

    // create a temp hull from bounding box sizes
    return CM_HeadnodeForBox(ent->mins, ent->maxs);
}

/*
=============
SV_PointContents
=============
*/
int SV_PointContents(const vec3_t &p)
{
    entity_t     *touch[MAX_EDICTS], *hit;
    int         i, num;
    int         contents;

    if (!sv.cm.cache) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }

    // get base contents from world
    contents = CM_PointContents(p, sv.cm.cache->nodes);

    // or in contents from all the other entities
    num = SV_AreaEntities(p, p, touch, MAX_EDICTS, AREA_SOLID);

    for (i = 0; i < num; i++) {
        hit = touch[i];

        // might intersect, so do an exact clip
        contents |= CM_TransformedPointContents(p, SV_HullForEntity(hit),
                                                hit->state.origin, hit->state.angles);
    }

    return contents;
}

/*
====================
SV_ClipMoveToEntities

====================
*/
static void SV_ClipMoveToEntities(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end,
                                  entity_t *passedict, int contentmask, trace_t *tr)
{
    vec3_t      boxmins, boxmaxs;
    int         i, num;
    entity_t     *touchlist[MAX_EDICTS], *touch;
    trace_t     trace;

    // create the bounding box of the entire move
    for (i = 0; i < 3; i++) {
        if (end[i] > start[i]) {
            boxmins[i] = start[i] + mins[i] - 1;
            boxmaxs[i] = end[i] + maxs[i] + 1;
        } else {
            boxmins[i] = end[i] + mins[i] - 1;
            boxmaxs[i] = start[i] + maxs[i] + 1;
        }
    }

    num = SV_AreaEntities(boxmins, boxmaxs, touchlist, MAX_EDICTS, AREA_SOLID);

    // be careful, it is possible to have an entity in this
    // list removed before we get to it (killtriggered)
    for (i = 0; i < num; i++) {
        touch = touchlist[i];
        if (touch->solid == Solid::Not)
            continue;
        if (touch == passedict)
            continue;
        if (tr->allSolid)
            return;
        if (passedict) {
            if (touch->owner == passedict)
                continue;    // don't clip against own missiles
            if (passedict->owner == touch)
                continue;    // don't clip against owner
        }

        if (!(contentmask & CONTENTS_DEADMONSTER)
            && (touch->serverFlags & EntityServerFlags::DeadMonster))
            continue;

        // might intersect, so do an exact clip
        CM_TransformedBoxTrace(&trace, start, end, mins, maxs,
                               SV_HullForEntity(touch), contentmask,
                               touch->state.origin, touch->state.angles);

        CM_ClipEntity(tr, &trace, touch);
    }
}

/*
==================
SV_Trace

Moves the given mins/maxs volume through the world from start to end.
Passedict and edicts owned by passedict are explicitly not checked.
==================
*/
trace_t q_gameabi SV_Trace(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end,
                           entity_t *passedict, int contentmask)
{
    trace_t     trace;

    if (!sv.cm.cache) {
        Com_Error(ERR_DROP, "%s: no map loaded", __func__);
    }

    // work around game bugs
    if (++sv.tracecount > 10000) {
        Com_EPrintf("%s: runaway loop avoided\n", __func__);
        memset(&trace, 0, sizeof(trace));
        trace.fraction = 1;
        trace.ent = ge->edicts;
        VectorCopy(end, trace.endPosition);
        sv.tracecount = 0;
        return trace;
    }

    // clip to world
    CM_BoxTrace(&trace, start, end, mins, maxs, sv.cm.cache->nodes, contentmask);
    trace.ent = ge->edicts;
    if (trace.fraction == 0) {
        return trace;   // Blocked by the world
    }

    // clip to other solid entities
    SV_ClipMoveToEntities(start, mins, maxs, end, passedict, contentmask, &trace);
    return trace;
}

