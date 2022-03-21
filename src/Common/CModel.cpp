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
// cmodel.c -- model loading

#include "Shared/Shared.h"
#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CModel.h"
#include "Common/Common.h"
#include "Common/CVar.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

mtexinfo_t nulltexinfo;

static mleaf_t      nullleaf;

static int          floodvalid;
static int          checkcount;

static cvar_t       *map_noareas;
static cvar_t       *map_allsolid_bug;

static void    FloodAreaConnections(cm_t *cm);

/*
==================
CM_FreeMap
==================
*/
void CM_FreeMap(cm_t *cm)
{
    if (cm->floodnums) {
        Z_Free(cm->floodnums);
    }
    BSP_Free(cm->cache);

    memset(cm, 0, sizeof(*cm));
}

/*
==================
CM_LoadMap

Loads in the map and all submodels
==================
*/
qerror_t CM_LoadMap(cm_t *cm, const char *name)
{
    bsp_t *cache;

    qerror_t ret = BSP_Load(name, &cache);
    if (!cache) {
        return ret;
    }

    cm->cache = cache;
    cm->floodnums = (int*)Z_TagMallocz(sizeof(int) * cm->cache->numareas +      // CPP: Cast
                                 sizeof(qboolean) * (cm->cache->lastareaportal + 1), TAG_CMODEL);
    cm->portalopen = (qboolean *)(cm->floodnums + cm->cache->numareas);
    FloodAreaConnections(cm);

    return Q_ERR_SUCCESS;
}

mnode_t *CM_NodeNum(cm_t *cm, int number)
{
    if (!cm->cache) {
        return (mnode_t *)&nullleaf;
    }
    if (number == -1) {
        return (mnode_t *)cm->cache->leafs;   // special case for solid leaf
    }
    if (number < 0 || number >= cm->cache->numnodes) {
        Com_EPrintf("%s: bad number: %d\n", __func__, number);
        return (mnode_t *)&nullleaf;
    }
    return cm->cache->nodes + number;
}

mleaf_t *CM_LeafNum(cm_t *cm, int number)
{
    if (!cm->cache) {
        return &nullleaf;
    }
    if (number < 0 || number >= cm->cache->numleafs) {
        Com_EPrintf("%s: bad number: %d\n", __func__, number);
        return &nullleaf;
    }
    return cm->cache->leafs + number;
}

//=======================================================================

static CollisionPlane box_planes[12];
static mnode_t  box_nodes[6];
static mnode_t  *box_headnode;
static mbrush_t box_brush;
static mbrush_t *box_leafbrush;
static mbrushside_t box_brushsides[6];
static mleaf_t  box_leaf;
static mleaf_t  box_emptyleaf;

/*
===================
CM_InitBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
static void CM_InitBoxHull(void)
{
    box_headnode = &box_nodes[0];

    box_brush.numsides = 6;
    box_brush.firstbrushside = &box_brushsides[0];
    box_brush.contents = BrushContents::Monster;

    box_leaf.contents = BrushContents::Monster;
    box_leaf.firstleafbrush = &box_leafbrush;
    box_leaf.numleafbrushes = 1;

    box_leafbrush = &box_brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = i & 1;

        // brush sides
        mbrushside_t *brushSide = &box_brushsides[i];
        brushSide->plane = &box_planes[i * 2 + side];
        brushSide->texinfo = &nulltexinfo;

        // nodes
        mnode_t *node = &box_nodes[i];
        node->plane = &box_planes[i * 2];
        node->children[side] = (mnode_t *)&box_emptyleaf;
        if (i != 5)
            node->children[side ^ 1] = &box_nodes[i + 1];
        else
            node->children[side ^ 1] = (mnode_t *)&box_leaf;

        // planes
        CollisionPlane *plane = &box_planes[i * 2];
        plane->type = i >> 1;
        plane->signBits = 0;
        plane->normal = vec3_zero();
        plane->normal[i >> 1] = 1;

        plane = &box_planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = 0;
        plane->normal = vec3_zero();
        plane->normal[i >> 1] = -1;
    }
}


/*
===================
CM_HeadnodeForBox

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
mnode_t *CM_HeadnodeForBox(const vec3_t &mins, const vec3_t &maxs)
{
    box_planes[0].dist = maxs[0];
    box_planes[1].dist = -maxs[0];
    box_planes[2].dist = mins[0];
    box_planes[3].dist = -mins[0];
    box_planes[4].dist = maxs[1];
    box_planes[5].dist = -maxs[1];
    box_planes[6].dist = mins[1];
    box_planes[7].dist = -mins[1];
    box_planes[8].dist = maxs[2];
    box_planes[9].dist = -maxs[2];
    box_planes[10].dist = mins[2];
    box_planes[11].dist = -mins[2];

    return box_headnode;
}

// TODO: Implement.
mnode_t* CM_HeadnodeForOctagon(const vec3_t& mins, const vec3_t& maxs) {
    return nullptr;
}


mleaf_t *CM_PointLeaf(cm_t *cm, const vec3_t &p)
{
    if (!cm->cache) {
        return &nullleaf;       // server may call this without map loaded
    }
    return BSP_PointLeaf(cm->cache->nodes, p);
}

/*
=============
CM_BoxLeafnums

Fills in a list of all the leafs touched
=============
*/
static int      leaf_count, leaf_maxcount;
static mleaf_t  **leaf_list;
static const float    *leaf_mins, *leaf_maxs;
static mnode_t  *leaf_topnode;

static void CM_BoxLeafs_r(mnode_t *node)
{
    while (node->plane) {
        int32_t s = BoxOnPlaneSideFast(leaf_mins, leaf_maxs, node->plane);
        if (s == 1) {
            node = node->children[0];
        } else if (s == 2) {
            node = node->children[1];
        } else {
            // go down both
            if (!leaf_topnode) {
                leaf_topnode = node;
            }
            CM_BoxLeafs_r(node->children[0]);
            node = node->children[1];
        }
    }

    if (leaf_count < leaf_maxcount) {
        leaf_list[leaf_count++] = (mleaf_t *)node;
    }
}

static int CM_BoxLeafs_headnode(const vec3_t &mins, const vec3_t &maxs, mleaf_t **list, int listsize,
                                mnode_t *headNode, mnode_t **topnode)
{
    leaf_list = list;
    leaf_count = 0;
    leaf_maxcount = listsize;
    leaf_mins = mins;
    leaf_maxs = maxs;

    leaf_topnode = NULL;

    CM_BoxLeafs_r(headNode);

    if (topnode)
        *topnode = leaf_topnode;

    return leaf_count;
}

int CM_BoxLeafs(cm_t *cm, const vec3_t &mins, const vec3_t &maxs, mleaf_t **list, int listsize, mnode_t **topnode)
{
    if (!cm->cache)     // map not loaded
        return 0;
    return CM_BoxLeafs_headnode(mins, maxs, list,
                                listsize, cm->cache->nodes, topnode);
}



/*
==================
CM_PointContents

==================
*/
int CM_PointContents(const vec3_t &p, mnode_t *headNode)
{
    if (!headNode) {
        return 0;
    }

    mleaf_t *leaf = BSP_PointLeaf(headNode, p);

    return leaf->contents;
}

/*
==================
CM_TransformedPointContents

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
int CM_TransformedPointContents(const vec3_t &p, mnode_t *headNode, const vec3_t& origin, const vec3_t& angles)
{
    vec3_t temp = vec3_zero();
    vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();

    if (!headNode) {
        return 0;
    }

    // subtract origin offset
    vec3_t p_l = p - origin;

    vec3_t axis[3];
    // rotate start and end into the models frame of reference
    if (headNode != box_headnode && !(angles[0] == 0 && angles[1] == 0 && angles[2] == 0)) {
        AnglesToAxis(angles, axis);
        RotatePoint(p_l, axis);
    }

    return BSP_PointLeaf(headNode, p_l)->contents;
}


/*
===============================================================================

BOX TRACING

===============================================================================
*/

// 1/32 epsilon to keep floating point happy
//#define DIST_EPSILON    (0.03125)
//#define DIST_EPSILON    0.125
static constexpr float DIST_EPSILON = 1.0f / 32.0f;

static vec3_t   trace_start, trace_end;
static vec3_t   trace_mins, trace_maxs;
static vec3_t   trace_extents;

static TraceResult  *trace_trace;
static int      trace_contents;
static qboolean trace_ispoint;      // optimized case

/*
================
CM_ClipBoxToBrush
================
*/
static void CM_ClipBoxToBrush(const vec3_t &mins, const vec3_t &maxs, const vec3_t &p1, const vec3_t &p2,
                              TraceResult *trace, mbrush_t *brush)
{
    if (!brush->numsides)
        return;
    
    qboolean getout = false;
    qboolean startout = false;

    vec3_t offset = vec3_zero();
    float dist = 0.f;
    float fraction = 0.f;

    float enterFraction = -1.f;
    float leaveFraction = 1.f;

    CollisionPlane    *clipPlane = nullptr;
    mbrushside_t *leadside = nullptr;
    mbrushside_t *side = brush->firstbrushside;

    for (int32_t i = 0; i < brush->numsides; i++, side++) {
        CollisionPlane *plane = side->plane;

        // FIXME: special case for axial

        if (!trace_ispoint) {
            // general box case

            // push the plane out apropriately for mins/maxs

            // FIXME: use signBits into 8 way lookup for each mins/maxs
            for (int32_t j = 0; j < 3; j++) {
                if (plane->normal[j] < 0)
                    offset[j] = maxs[j];
                else
                    offset[j] = mins[j];
            }
            dist = vec3_dot(offset, plane->normal);
            dist = plane->dist - dist;
        } else {
            // special point case
            dist = plane->dist;
        }

        const float d1 = vec3_dot(p1, plane->normal) - dist;
        const float d2 = vec3_dot(p2, plane->normal) - dist;

        if (d2 > 0)
            getout = true; // endpoint is not in solid
        if (d1 > 0)
            startout = true;

        // if completely in front of face, no intersection
        if (d1 > 0 && d2 >= d1)
            return;

        if (d1 <= 0 && d2 <= 0)
            continue;

        // crosses face
        if (d1 > d2) {
            // enter
            fraction = (d1 - DIST_EPSILON) / (d1 - d2);
            if (fraction > enterFraction) {
                enterFraction = fraction;
                clipPlane = plane;
                leadside = side;
            }
        } else {
            // leave
            fraction = (d1 + DIST_EPSILON) / (d1 - d2);
            if (fraction < leaveFraction)
                leaveFraction = fraction;
        }
    }

    if (!startout) {
        // original point was inside brush
        trace->startSolid = true;
        if (!getout) {
            trace->allSolid = true;
            if (!map_allsolid_bug->integer) {
                // original Q2 didn't set these
                trace->fraction = 0;
                trace->contents = brush->contents;
            }
        }
        return;
    }
    if (enterFraction < leaveFraction) {
        if (enterFraction > -1 && enterFraction < trace->fraction) {
            if (enterFraction < 0)
                enterFraction = 0;
            trace->fraction = enterFraction;
            trace->plane = *clipPlane;
            trace->surface = &(leadside->texinfo->c);
            trace->contents = brush->contents;
        }
    }
}

/*
================
CM_TestBoxInBrush
================
*/
static void CM_TestBoxInBrush(const vec3_t &mins, const vec3_t &maxs, const vec3_t &p1,
                              TraceResult *trace, mbrush_t *brush)
{

    if (!brush->numsides)
        return;

    vec3_t offset = vec3_zero();

    mbrushside_t *brushSide = brush->firstbrushside;

    for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
        CollisionPlane *plane = brushSide->plane;

        // FIXME: special case for axial

        // general box case

        // push the plane out apropriately for mins/maxs

        // FIXME: use signBits into 8 way lookup for each mins/maxs
        for (int32_t j = 0; j < 3; j++) {
            if (plane->normal[j] < 0)
                offset[j] = maxs[j];
            else
                offset[j] = mins[j];
        }
        float dist = vec3_dot(offset, plane->normal);
        dist = plane->dist - dist;

        float d1 = vec3_dot(p1, plane->normal) - dist;

        // if completely in front of face, no intersection
        if (d1 > 0)
            return;

    }

    // inside this brush
    trace->startSolid = trace->allSolid = true;
    trace->fraction = 0;
    trace->contents = brush->contents;
}


/*
================
CM_TraceToLeaf
================
*/
static void CM_TraceToLeaf(mleaf_t *leaf)
{
    if (!(leaf->contents & trace_contents))
        return;

    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *b = *leafbrush;

        if (b->checkcount == checkcount)
            continue;   // Already checked this brush in another leaf
        
        b->checkcount = checkcount;

        if (!(b->contents & trace_contents))
            continue;
        
        CM_ClipBoxToBrush(trace_mins, trace_maxs, trace_start, trace_end, trace_trace, b);
        
        if (!trace_trace->fraction)
            return;
    }

}


/*
================
CM_TestInLeaf
================
*/
static void CM_TestInLeaf(mleaf_t *leaf)
{
    if (!(leaf->contents & trace_contents))
        return;
    
    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *b = *leafbrush;
        
        if (b->checkcount == checkcount)
            continue;   // Already checked this brush in another leaf
        
        b->checkcount = checkcount;

        if (!(b->contents & trace_contents))
            continue;
        
        CM_TestBoxInBrush(trace_mins, trace_maxs, trace_start, trace_trace, b);
        
        if (!trace_trace->fraction)
            return;
    }

}


/*
==================
CM_RecursiveHullCheck

==================
*/
static void CM_RecursiveHullCheck(mnode_t *node, float p1f, float p2f, const vec3_t &p1, const vec3_t &p2)
{
    CollisionPlane    *plane = nullptr;
    float       t1, t2, offset;
    float       frac, frac2;
    float       idist;
    vec3_t      mid;
    int         side;
    float       midf;

    if (trace_trace->fraction <= p1f)
        return;     // already hit something nearer

recheck:
    // if plane is NULL, we are in a leaf node
    plane = node->plane;
    if (!plane) {
        CM_TraceToLeaf((mleaf_t *)node);
        return;
    }

    //
    // find the point distances to the seperating plane
    // and the offset for the size of the box
    //
    if (plane->type < 3) {
        t1 = p1[plane->type] - plane->dist;
        t2 = p2[plane->type] - plane->dist;
        offset = trace_extents[plane->type];
    } else {
        t1 = PlaneDiff(p1, plane);
        t2 = PlaneDiff(p2, plane);
        if (trace_ispoint)
            offset = 0;
        else
            offset = 2048.f;
            //offset = fabs(trace_extents[0] * plane->normal[0]) +
            //         fabs(trace_extents[1] * plane->normal[1]) +
            //         fabs(trace_extents[2] * plane->normal[2]);
    }

    // see which sides we need to consider
    if (t1 >= offset + DIST_EPSILON && t2 >= offset + DIST_EPSILON) {
        node = node->children[0];
        goto recheck;
    }
    if (t1 < -offset - DIST_EPSILON && t2 < -offset - DIST_EPSILON) {
        node = node->children[1];
        goto recheck;
    }

    // put the crosspoint DIST_EPSILON pixels on the near side
    if (t1 < t2) {
        idist = 1.0 / (t1 - t2);
        side = 1;
        frac2 = (t1 + offset + DIST_EPSILON) * idist;
        frac = (t1 - offset + DIST_EPSILON) * idist;
    } else if (t1 > t2) {
        idist = 1.0 / (t1 - t2);
        side = 0;
        frac2 = (t1 - offset - DIST_EPSILON) * idist;
        frac = (t1 + offset + DIST_EPSILON) * idist;
    } else {
        side = 0;
        frac = 1;
        frac2 = 0;
    }

    // move up to the node
    frac = Clampf(frac);//clamp(frac, 0, 1);

    midf = p1f + (p2f - p1f) * frac;
    mid = vec3_mix(p1, p2, frac);//LerpVector(p1, p2, frac, mid);

    CM_RecursiveHullCheck(node->children[side], p1f, midf, p1, mid);

    // go past the node
    frac2 = Clampf(frac2);//clamp(frac2, 0, 1);

    midf = p1f + (p2f - p1f) * frac2;
    mid = vec3_mix(p1, p2, frac2); //LerpVector(p1, p2, frac2, mid);

    CM_RecursiveHullCheck(node->children[side ^ 1], midf, p2f, mid, p2);
}



//======================================================================

/*
==================
CM_BoxTrace
==================
*/
void CM_BoxTrace(TraceResult *trace, const vec3_t &start, const vec3_t &end,
                 const vec3_t &mins, const vec3_t &maxs,
                 mnode_t *headNode, int brushmask)
{
    checkcount++;       // for multi-check avoidance

    // fill in a default trace
    trace_trace = trace;
    memset(trace_trace, 0, sizeof(*trace_trace));
    trace_trace->fraction = 1;
    trace_trace->surface = &(nulltexinfo.c);

    if (!headNode) {
        return;
    }

    trace_contents = brushmask;
    trace_start = start; //VectorCopy(start, trace_start);
    trace_end = end; // VectorCopy(end, trace_end);
    trace_mins = mins; // VectorCopy(mins, trace_mins);
    trace_maxs = maxs; // VectorCopy(maxs, trace_maxs);

    //
    // check for position test special case
    //
    if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
        mleaf_t *leafs[1024];
        vec3_t c1 = start + mins;  //VectorAdd(start, mins, c1);
        vec3_t c2 = start + maxs; // VectorAdd(start, maxs, c2);
        for (int32_t i = 0; i < 3; i++) {
            c1[i] -= 1;
            c2[i] += 1;
        }

        int32_t numleafs = CM_BoxLeafs_headnode(c1, c2, leafs, Q_COUNTOF(leafs), headNode, NULL);
        for (int32_t i = 0; i < numleafs; i++) {
            CM_TestInLeaf(leafs[i]);
            if (trace_trace->allSolid)
                break;
        }
        trace_trace->endPosition = start; //VectorCopy(start, trace_trace->endPosition);
        return;
    }

    //
    // check for point special case
    //
    if (mins[0] == 0 && mins[1] == 0 && mins[2] == 0
        && maxs[0] == 0 && maxs[1] == 0 && maxs[2] == 0) {
            trace_ispoint = true;
            trace_extents = vec3_zero();// VectorClear(trace_extents);
    } else {
        trace_ispoint = false;
        trace_extents[0] = -mins[0] > maxs[0] ? -mins[0] : maxs[0];
        trace_extents[1] = -mins[1] > maxs[1] ? -mins[1] : maxs[1];
        trace_extents[2] = -mins[2] > maxs[2] ? -mins[2] : maxs[2];

        // PH: Q3 - FF Precision. Hopefully...
        trace_trace->offsets[0] = maxs;//VectorCopy(maxs, trace_trace->offsets[0]);

        trace_trace->offsets[1][0] = maxs[0];
        trace_trace->offsets[1][1] = mins[1];
        trace_trace->offsets[1][2] = mins[2];

        trace_trace->offsets[2][0] = mins[0];
        trace_trace->offsets[2][1] = maxs[1];
        trace_trace->offsets[2][2] = mins[2];

        trace_trace->offsets[3][0] = maxs[0];
        trace_trace->offsets[3][1] = maxs[1];
        trace_trace->offsets[3][2] = mins[2];

        trace_trace->offsets[4][0] = mins[0];
        trace_trace->offsets[4][1] = mins[1];
        trace_trace->offsets[4][2] = maxs[2];

        trace_trace->offsets[5][0] = maxs[0];
        trace_trace->offsets[5][1] = mins[1];
        trace_trace->offsets[5][2] = maxs[2];

        trace_trace->offsets[6][0] = mins[0];
        trace_trace->offsets[6][1] = maxs[1];
        trace_trace->offsets[6][2] = maxs[2];

        trace_trace->offsets[7] = maxs; // VectorCopy(maxs, trace_trace->offsets[7]);
//        trace_trace->offsets[7] = maxs0;
    }

    //
    // general sweeping through world
    //
    CM_RecursiveHullCheck(headNode, 0, 1, start, end);

    if (trace_trace->fraction == 1)
        trace_trace->endPosition = end;
    else
        trace_trace->endPosition = vec3_mix(start, end, trace_trace->fraction);
}


/*
==================
CM_TransformedBoxTrace

Handles offseting and rotation of the end points for moving and
rotating entities
==================
*/
void CM_TransformedBoxTrace(TraceResult *trace, const vec3_t &start, const vec3_t &end,
                            const vec3_t &mins, const vec3_t &maxs,
                            mnode_t *headNode, int brushmask,
                            const vec3_t &origin, const vec3_t &angles)
{
    vec3_t      axis[3];
    qboolean    rotated;

    // subtract origin offset
    vec3_t start_l = start - origin;
    vec3_t end_l = end - origin;

    // rotate start and end into the models frame of reference
    if (headNode != box_headnode &&
        (angles[0] || angles[1] || angles[2]))
        rotated = true;
    else
        rotated = false;
    
    if (rotated) {
        AnglesToAxis(angles, axis);
        RotatePoint(start_l, axis);
        RotatePoint(end_l, axis);
    }

    // sweep the box through the model
    CM_BoxTrace(trace, start_l, end_l, mins, maxs, headNode, brushmask);

    // rotate plane normal into the worlds frame of reference
    if (rotated && trace->fraction != 1.0) {
        TransposeAxis(axis);
        RotatePoint(trace->plane.normal, axis);
    }

    // FIXME: offset plane distance?
    trace->endPosition = vec3_mix(start, end, trace->fraction); // LerpVector(start, end, trace->fraction, trace->endPosition);
}

void CM_ClipEntity(TraceResult *dst, const TraceResult *src, struct entity_s *ent)
{
    dst->allSolid |= src->allSolid;
    dst->startSolid |= src->startSolid;
    if (src->fraction < dst->fraction) {
        dst->fraction = src->fraction;
        dst->endPosition = src->endPosition; //VectorCopy(src->endPosition, dst->endPosition);
        dst->plane = src->plane;
        dst->surface = src->surface;
        dst->contents |= src->contents;
        dst->ent = ent;
    }
}


/*
===============================================================================

AREAPORTALS

===============================================================================
*/

static void FloodArea_r(cm_t *cm, int number, int floodnum)
{
    int i;
    mareaportal_t *p;
    marea_t *area;

    area = &cm->cache->areas[number];
    if (area->floodvalid == floodvalid) {
        if (cm->floodnums[number] == floodnum)
            return;
        Com_Error(ErrorType::Drop, "FloodArea_r: reflooded");
    }

    cm->floodnums[number] = floodnum;
    area->floodvalid = floodvalid;
    p = area->firstareaportal;
    for (i = 0; i < area->numareaportals; i++, p++) {
        if (cm->portalopen[p->portalnum])
            FloodArea_r(cm, p->otherarea, floodnum);
    }
}

static void FloodAreaConnections(cm_t *cm)
{
    marea_t *area;

    // All current floods are now invalid
    floodvalid++;
    int32_t floodnum = 0;

    // Area 0 is not used
    for (int32_t i = 1; i < cm->cache->numareas; i++) {
        area = &cm->cache->areas[i];
        if (area->floodvalid == floodvalid)
            continue;       // already flooded into
        floodnum++;
        FloodArea_r(cm, i, floodnum);
    }
}

void CM_SetAreaPortalState(cm_t *cm, int portalnum, qboolean open)
{
    if (!cm->cache) {
        return;
    }

    if (portalnum < 0 || portalnum >= MAX_MAP_AREAPORTALS) {
        Com_EPrintf("%s: portalnum %d is out of range\n", __func__, portalnum);
        return;
    }

    // ignore areaportals not referenced by areas
    if (portalnum > cm->cache->lastareaportal) {
        Com_DPrintf("%s: portalnum %d is not in use\n", __func__, portalnum);
        return;
    }

    cm->portalopen[portalnum] = open;
    FloodAreaConnections(cm);
}

qboolean CM_AreasConnected(cm_t *cm, int area1, int area2)
{
    bsp_t *cache = cm->cache;

    if (!cache) {
        return false;
    }
    if (map_noareas->integer) {
        return true;
    }
    if (area1 < 1 || area2 < 1) {
        return false;
    }
    if (area1 >= cache->numareas || area2 >= cache->numareas) {
        Com_EPrintf("%s: area > numareas\n", __func__);
        return false;
    }
    if (cm->floodnums[area1] == cm->floodnums[area2]) {
        return true;
    }

    return false;
}


/*
=================
CM_WriteAreaBits

Writes a length byte followed by a bit vector of all the areas
that area in the same flood as the area parameter

This is used by the client refreshes to cull visibility
=================
*/
int CM_WriteAreaBits(cm_t *cm, byte *buffer, int area)
{
    bsp_t *cache = cm->cache;

    if (!cache) {
        return 0;
    }

    int32_t bytes = (cache->numareas + 7) >> 3;

    if (map_noareas->integer || !area) {
        // for debugging, send everything
        memset(buffer, 255, bytes);
    } else {
        memset(buffer, 0, bytes);

        int32_t floodnum = cm->floodnums[area];
        for (int32_t i = 0; i < cache->numareas; i++) {
            if (cm->floodnums[i] == floodnum) {
                Q_SetBit(buffer, i);
            }
        }
    }

    return bytes;
}

int CM_WritePortalBits(cm_t *cm, byte *buffer)
{
    int32_t     i, bytes, numportals;

    if (!cm->cache) {
        return 0;
    }

    numportals = min(cm->cache->lastareaportal + 1, MAX_MAP_PORTAL_BYTES << 3);

    bytes = (numportals + 7) >> 3;
    memset(buffer, 0, bytes);
    for (i = 0; i < numportals; i++) {
        if (cm->portalopen[i]) {
            Q_SetBit(buffer, i);
        }
    }

    return bytes;
}

void CM_SetPortalStates(cm_t *cm, byte *buffer, int bytes)
{
    if (!cm->cache) {
        return;
    }

    if (!bytes) {
        for (int32_t i = 0; i <= cm->cache->lastareaportal; i++) {
            cm->portalopen[i] = true;
        }
    } else {
        int32_t numPortals = min(cm->cache->lastareaportal + 1, bytes << 3);
        int32_t i = 0;
        for (i = 0; i < numPortals; i++) {
            cm->portalopen[i] = Q_IsBitSet(buffer, i);
        }
        for (; i <= cm->cache->lastareaportal; i++) {
            cm->portalopen[i] = true;
        }
    }

    FloodAreaConnections(cm);
}


/*
=============
CM_HeadnodeVisible

Returns true if any leaf under headNode has a cluster that
is potentially visible
=============
*/
qboolean CM_HeadnodeVisible(mnode_t *node, byte *visbits)
{
    while (node->plane) {
        if (CM_HeadnodeVisible(node->children[0], visbits))
            return true;
        node = node->children[1];
    }

    mleaf_t *leaf = (mleaf_t *)node;
    int32_t cluster = leaf->cluster;
    if (cluster == -1)
        return false;
    if (Q_IsBitSet(visbits, cluster))
        return true;
    return false;
}


/*
============
CM_FatPVS

The client will interpolate the view position,
so we can't use a single PVS point
===========
*/
byte *CM_FatPVS(cm_t *cm, byte *mask, const vec3_t &org, int vis)
{
    static byte    temp[VIS_MAX_BYTES];
    mleaf_t *leafs[64];
    int     clusters[64];
    uint_fast32_t *src, *dst;
    vec3_t  mins, maxs;

    if (!cm->cache) {   // map not loaded
        return (byte*)memset(mask, 0, VIS_MAX_BYTES); // CPP: Cast
    }
    if (!cm->cache->vis) {
        return (byte*)memset(mask, 0xff, VIS_MAX_BYTES); // CPP: Cast
    }

    for (int32_t i = 0; i < 3; i++) {
        mins[i] = org[i] - 8;
        maxs[i] = org[i] + 8;
    }

    int32_t count = CM_BoxLeafs(cm, mins, maxs, leafs, 64, NULL);
    if (count < 1)
        Com_Error(ErrorType::Drop, "CM_FatPVS: leaf count < 1");
    int32_t longs = VIS_FAST_LONGS(cm->cache);

    // convert leafs to clusters
    for (int32_t i = 0; i < count; i++) {
        clusters[i] = leafs[i]->cluster;
    }

    BSP_ClusterVis(cm->cache, mask, clusters[0], vis);

    // or in all the other leaf bits
    for (int32_t i = 1; i < count; i++) {
        for (int32_t j = 0; j < i; j++) {
            if (clusters[i] == clusters[j]) {
                goto nextleaf; // already have the cluster we want
            }
        }
        src = (uint_fast32_t *)BSP_ClusterVis(cm->cache, temp, clusters[i], vis);
        dst = (uint_fast32_t *)mask;
        for (int32_t j = 0; j < longs; j++) {
            *dst++ |= *src++;
        }

nextleaf:;
    }

    return mask;
}

/*
=============
CM_Init
=============
*/
void CM_Init(void)
{
    CM_InitBoxHull();

    nullleaf.cluster = -1;

    map_noareas = Cvar_Get("map_noareas", "0", 0);
    map_allsolid_bug = Cvar_Get("map_allsolid_bug", "1", 0);
}

