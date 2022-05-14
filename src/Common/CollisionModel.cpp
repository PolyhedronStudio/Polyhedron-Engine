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

#include <Shared/Shared.h>
#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CVar.h"
#include "Common/CollisionModel.h"
#include "Common/Common.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

/**
*   CollisionModel data.
**/
CollisionModel collisionModel;

/**
*   BoxLeaf 'Work'
**/
static struct BoxLeafsWork {
    //! Leaf count.
    int32_t leafCount = 0;
    //! Max leaf count.
    int32_t leafMaximumCount = 0;
    //! Leaf List.
    mleaf_t  **leafList = nullptr;
    //! Mins of Leaf.
    const float *leafMins = nullptr;
    //! Maxs of Leaf.
    const float *leafMaxs = nullptr;
    //! Top node of Leaf.
    mnode_t  *leafTopNode = nullptr;
} boxLeafsWork;

/**
*   Trace 'Work'
**/
static struct TraceWork {
    //! Pointer where to store trace results.
	TraceResult  traceResult = { 
	
	};

    //! Real Fraction used for testing.
    float realFraction = 0.f;

    //! Brush/Surface contents.
    int32_t contents = 0;
    //! Trace checkCount.
    int32_t checkCount = 0;

    //! Point or Box Trace.
    qboolean isPoint = false; // Optimized case.

    //! Offset for octagon cylinder clipping.
    vec3_t  cylinderOffset = vec3_zero();

    //! Mins/Maxs.
    vec3_t  mins = vec3_zero();
    vec3_t  maxs = vec3_zero();
    //! Absolute Mins/Maxs.
    vec3_t absMins = vec3_zero();
    vec3_t absMaxs = vec3_zero();

    //! Start/End.
    vec3_t  start   = vec3_zero();
    vec3_t  end     = vec3_zero();

    //! Start/End - Mins/Maxs
    vec3_t startMins    = vec3_zero();
    vec3_t startMaxs    = vec3_zero();
    vec3_t endMins      = vec3_zero();
    vec3_t endMaxs      = vec3_zero();

    //! Extents.
    vec3_t  extents = vec3_zero();
} traceWork;

static void    FloodAreaConnections(cm_t *cm);


/**
*   @brief  Frees the map and all of its "submodels".
**/
void CM_FreeMap(cm_t *cm)
{
    if (cm->floodnums) {
        Z_Free(cm->floodnums);
    }
    BSP_Free(cm->cache);

    memset(cm, 0, sizeof(*cm));
}

/**
*   @brief  Loads in the BSP map and its "submodels".
**/
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

/**
*   @return Pointer to a valid node matching 'number'.
**/
mnode_t *CM_NodeNum(cm_t *cm, int number)
{
    if (!cm->cache) {
        return (mnode_t *)&collisionModel.nullLeaf;
    }
    if (number == -1) {
        return (mnode_t *)cm->cache->leafs;   // special case for solid leaf
    }
    if (number < 0 || number >= cm->cache->numnodes) {
        Com_EPrintf("%s: bad number: %d\n", __func__, number);
        return (mnode_t *)&collisionModel.nullLeaf;
    }
    return cm->cache->nodes + number;
}

/**
*   @return Pointer to a valid leaf matching 'number'.
**/
mleaf_t *CM_LeafNum(cm_t *cm, int number)
{
    if (!cm->cache) {
        return &collisionModel.nullLeaf;
    }
    if (number < 0 || number >= cm->cache->numleafs) {
        Com_EPrintf("%s: bad number: %d\n", __func__, number);
        return &collisionModel.nullLeaf;
    }
    return cm->cache->leafs + number;
}



/***
*
*
*   General 'Box' Collision Shape
*
*
***/
static struct BoxHull {
    CollisionPlane planes[12];
    mnode_t  nodes[6];
    mnode_t  *headNode;
    mbrush_t brush;
    mbrush_t *leafBrush;
    mbrushside_t brushSides[6];
    mleaf_t  leaf;
    mleaf_t  emptyLeaf;
} boxHull;

/**
*   @brief  Set up the planes and nodes so that the six floats of a bounding box
*           can just be stored out and get a proper clipping hull structure.
**/
static void CM_InitBoxHull(void)
{
    boxHull.headNode = &boxHull.nodes[0];

    boxHull.brush.numsides = 6;
    boxHull.brush.firstbrushside = &boxHull.brushSides[0];
    boxHull.brush.contents = BrushContents::Monster;

    boxHull.leaf.contents = BrushContents::Monster;
    boxHull.leaf.firstleafbrush = &boxHull.leafBrush;
    boxHull.leaf.numleafbrushes = 1;

    boxHull.leafBrush = &boxHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &boxHull.brushSides[i];
        brushSide->plane = &boxHull.planes[i * 2 + side];
        brushSide->texinfo = &collisionModel.nullTextureInfo;

        // Setup Box Nodes.
        mnode_t *node = &boxHull.nodes[i];
        node->plane = &boxHull.planes[i * 2];
        node->children[side] = (mnode_t *)&boxHull.emptyLeaf;
        if (i != 5) {
            node->children[side ^ 1] = &boxHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&boxHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &boxHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &boxHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }
}

/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t *CM_HeadnodeForBox(const vec3_t &mins, const vec3_t &maxs)
{
    boxHull.planes[0].dist = maxs[0];
    boxHull.planes[1].dist = -maxs[0];
    boxHull.planes[2].dist = mins[0];
    boxHull.planes[3].dist = -mins[0];
    boxHull.planes[4].dist = maxs[1];
    boxHull.planes[5].dist = -maxs[1];
    boxHull.planes[6].dist = mins[1];
    boxHull.planes[7].dist = -mins[1];
    boxHull.planes[8].dist = maxs[2];
    boxHull.planes[9].dist = -maxs[2];
    boxHull.planes[10].dist = mins[2];
    boxHull.planes[11].dist = -mins[2];

    return boxHull.headNode;
}


/***
*
*
*   General 'Octagon' Collision Shape
*
*
***/
static struct OctagonHull {
    CollisionPlane planes[20];
    mnode_t  nodes[10];
    mnode_t  *headNode;
    mbrush_t brush;
    mbrush_t *leafBrush;
    mbrushside_t brushSides[10];
    mleaf_t  leaf;
    mleaf_t  emptyLeaf;
} octagonHull = {};

/**
*   @brief  Set up the planes and nodes so that the 10 floats of an octagon box
*           can just be stored out and get a proper clipping hull structure.
**/
static void CM_InitOctagonBoxHull(void)
{
    octagonHull.headNode = &octagonHull.nodes[0];

    octagonHull.brush.numsides = 10;
    octagonHull.brush.firstbrushside = &octagonHull.brushSides[0];
    octagonHull.brush.contents = BrushContents::Monster;

    octagonHull.leaf.firstleafbrush = &octagonHull.leafBrush;
    octagonHull.leaf.numleafbrushes = 1;
    octagonHull.leaf.contents = BrushContents::Monster;

    octagonHull.leafBrush = &octagonHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        // Determine side.
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &octagonHull.brushSides[i];
        brushSide->plane = &octagonHull.planes[i * 2 + side];
        brushSide->texinfo = &collisionModel.nullTextureInfo;

        // Setup Box Nodes.
        mnode_t *node = &octagonHull.nodes[i];
        node->plane = &octagonHull.planes[i * 2];
        node->children[side] = (mnode_t *)&octagonHull.emptyLeaf;
        node->children[side ^ 1] = &octagonHull.nodes[i + 1];

        // Plane A.
        CollisionPlane *plane = &octagonHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;

        // Plane B.
        plane = &octagonHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
    }

    const vec3_t oct_dirs[4] = { { 1, 1, 0 }, { -1, 1, 0 }, { -1, -1, 0 }, { 1, -1, 0 } };

    for (int32_t i = 6; i < 10; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &octagonHull.brushSides[i];
        brushSide->plane = &octagonHull.planes[i * 2 + side];
        brushSide->texinfo = &collisionModel.nullTextureInfo;

        // Setup Box Nodes.
        mnode_t *node = &octagonHull.nodes[i];
        node->plane = &octagonHull.planes[i * 2];
        node->children[side] = (mnode_t *)&octagonHull.emptyLeaf;
        if (i != 9) {
            node->children[side ^ 1] = &octagonHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&octagonHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &octagonHull.planes[i * 2];
        plane->type = 3;// + (i >> 1);
        plane->normal = oct_dirs[i - 6];
        //SetPlaneType(plane);
        SetPlaneSignbits(plane);

        // Plane B.
        plane = &octagonHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->normal = oct_dirs[(i - 6)];
        //plane->signBits = (1 << (i >> 1)); //SetPlaneSignbits(plane);
        //SetPlaneType(plane);
        SetPlaneSignbits(plane);
    }
}

/**
*   @brief  Utility function to complement CM_HeadnodeForOctagon with.
**/
static inline float CalculateOctagonPlaneDist(CollisionPlane &plane, const vec3_t &mins, const vec3_t &maxs, bool negate = false) {
    if (negate == true) {
        return vec3_dot(plane.normal, {(plane.signBits & 1) ? -mins[0] : -maxs[0], (plane.signBits & 2) ? -mins[1] : -maxs[1], (plane.signBits & 4) ? -mins[2] : -maxs[2]});//-d;//d;
    } else {
        return vec3_dot(plane.normal, {(plane.signBits & 1) ? mins[0] : maxs[0], (plane.signBits & 2) ? mins[1] : maxs[1], (plane.signBits & 4) ? mins[2] : maxs[2]});//-d;//d;
    }
}

/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t* CM_HeadnodeForOctagon(const vec3_t& mins, const vec3_t& maxs) {
	// Calculate half size for mins and maxs.
    const vec3_t size[2] = {
        vec3_scale(mins, 0.5f), // Not sure why but this --> mins - offset, // was somehow not working well.
        vec3_scale(maxs, 0.5f), // Not sure why but this --> maxs - offset, // was somehow not working well.
    };

    // Calculate and store cylinder offset.
    traceWork.cylinderOffset = vec3_scale(mins + maxs, 0.5f);;

    // Setup the box distances.
    octagonHull.planes[0].dist = maxs[0];
    octagonHull.planes[1].dist = -maxs[0];
    octagonHull.planes[2].dist = mins[0];
    octagonHull.planes[3].dist = -mins[0];
    octagonHull.planes[4].dist = maxs[1];
    octagonHull.planes[5].dist = -maxs[1];
    octagonHull.planes[6].dist = mins[1];
    octagonHull.planes[7].dist = -mins[1];
    octagonHull.planes[8].dist = maxs[2];
    octagonHull.planes[9].dist = -maxs[2];
    octagonHull.planes[10].dist = mins[2];
    octagonHull.planes[11].dist = -mins[2];

    // Calculate actual up to scale normals for the non axial planes.
	const float a = maxs[0]; // Half-X
	const float b = maxs[1]; // Half-Y
	float dist = sqrt( a * a + b * b ); // Hypothenuse

	float cosa = a / dist;
	float sina = b / dist;

	// Calculate and set distances for each non axial plane.
    octagonHull.planes[12].normal   = vec3_t{cosa, sina, 0.f};
    octagonHull.planes[12].dist     = CalculateOctagonPlaneDist(octagonHull.planes[12], size[0], size[1]);
    octagonHull.planes[13].normal   = vec3_t{cosa, sina, 0.f};
    octagonHull.planes[13].dist     = CalculateOctagonPlaneDist(octagonHull.planes[13], size[0], size[1], true);

    octagonHull.planes[14].normal   = vec3_t{-cosa, sina, 0.f};
    octagonHull.planes[14].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[14], size[0], size[1], true);
    octagonHull.planes[15].normal   = vec3_t{-cosa, sina, 0.f};
    octagonHull.planes[15].dist     = CalculateOctagonPlaneDist(octagonHull.planes[15], size[0], size[1]);

    octagonHull.planes[16].normal   = vec3_t{-cosa, -sina, 0.f};
    octagonHull.planes[16].dist     = CalculateOctagonPlaneDist(octagonHull.planes[16], size[0], size[1]);
    octagonHull.planes[17].normal   = vec3_t{-cosa, -sina, 0.f};
    octagonHull.planes[17].dist     = CalculateOctagonPlaneDist(octagonHull.planes[17], size[0], size[1], true);

    octagonHull.planes[18].normal   = vec3_t{cosa, -sina, 0.f};
    octagonHull.planes[18].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[18], size[0], size[1], true);
    octagonHull.planes[19].normal   = vec3_t{cosa, -sina, 0.f};
    octagonHull.planes[19].dist     = CalculateOctagonPlaneDist(octagonHull.planes[19], size[0], size[1]);

    // Cheers, we made it to this point, enjoy the new octagonHull :)
    return octagonHull.headNode;
}

/**
*   @return Pointer to the leaf matching vec3 'p'. Nullptr if it is called without a map loaded.
**/
mleaf_t *CM_PointLeaf(cm_t *cm, const vec3_t &p) {
    if (!cm->cache) {
        return &collisionModel.nullLeaf;       // server may call this without map loaded
    }
    return BSP_PointLeaf(cm->cache->nodes, p);
}

/**
*   @brief  Fills in a list of all the leafs touched
**/
static void CM_BoxLeafs_r(mnode_t *node)
{
    while (node->plane) {
        int32_t s = BoxOnPlaneSideFast(boxLeafsWork.leafMins, boxLeafsWork.leafMaxs, node->plane);
        if (s == 1) {
            node = node->children[0];
        } else if (s == 2) {
            node = node->children[1];
        } else {
            // go down both
            if (!boxLeafsWork.leafTopNode) {
                boxLeafsWork.leafTopNode = node;
            }
            CM_BoxLeafs_r(node->children[0]);
            node = node->children[1];
        }
    }

    if (boxLeafsWork.leafCount < boxLeafsWork.leafMaximumCount) {
        boxLeafsWork.leafList[boxLeafsWork.leafCount++] = (mleaf_t *)node;
    }
}

/**
*   @brief  
**/
static int CM_BoxLeafs_headnode(const vec3_t &mins, const vec3_t &maxs, mleaf_t **list, int listsize,
                                mnode_t *headNode, mnode_t **topnode)
{
    boxLeafsWork.leafList   = list;
    boxLeafsWork.leafCount  = 0;
    boxLeafsWork.leafMaximumCount = listsize;
    boxLeafsWork.leafMins   = mins;
    boxLeafsWork.leafMaxs   = maxs;

    boxLeafsWork.leafTopNode = nullptr;

    CM_BoxLeafs_r(headNode);

    if (topnode) {
        *topnode = boxLeafsWork.leafTopNode;
    }

    return boxLeafsWork.leafCount;
}

/**
*   @brief  
**/
int CM_BoxLeafs(cm_t *cm, const vec3_t &mins, const vec3_t &maxs, mleaf_t **list, int listsize, mnode_t **topnode)
{
    if (!cm->cache)     // map not loaded
        return 0;
    return CM_BoxLeafs_headnode(mins, maxs, list,
                                listsize, cm->cache->nodes, topnode);
}


/**
*
*
*   Box Tracing
*
*
**/

// 1/32 epsilon to keep floating point happy
//#define DIST_EPSILON    (0.03125)
//#define DIST_EPSILON    0.125
static constexpr float DIST_EPSILON = 1.0f / 32.0f;



/**
*   @brief Clips the box to the brush if needed.
**/
static void CM_ClipBoxToBrush(const vec3_t &mins, const vec3_t &maxs, const vec3_t &p1, const vec3_t &p2,
                              TraceResult *trace, mbrush_t *brush)
{
    if (!brush->numsides) {
        return;
    }
    
    bool getOut = false;
    bool startOut = false;

    float dist = 0.f;
    float fraction = 0.f;

    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

    vec3_t offset = vec3_zero();

    CollisionPlane    *clipPlane = nullptr;
    mbrushside_t *leadSide = nullptr;
    mbrushside_t *side = brush->firstbrushside;

    for (int32_t i = 0; i < brush->numsides; i++, side++) {
        CollisionPlane *plane = side->plane;

        float d1 = 0.f;
        float d2 = 0.f;

        // FIXME: special case for axial
		// push the plane out apropriately for mins/maxs
		if( plane->type < 3 ) {
			d1 = traceWork.startMins[plane->type] - plane->dist;
			d2 = traceWork.endMins[plane->type] - plane->dist;
		} else {
			switch( plane->signBits ) {
				case 0:
					d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
					break;
				case 1:
					d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
					break;
				case 2:
					d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
					break;
				case 3:
					d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
					break;
				case 4:
					d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
					break;
				case 5:
					d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
					break;
				case 6:
					d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
					break;
				case 7:
					d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
					d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
					break;
				default:
					d1 = d2 = 0; // shut up compiler
					//assert( 0 );
					break;
			}
		}

        //if (!traceWork.isPoint) {
        //    // general box case

        //    // push the plane out apropriately for mins/maxs

        //    // FIXME: use signBits into 8 way lookup for each mins/maxs
        //    for (int32_t j = 0; j < 3; j++) {
        //        if (plane->normal[j] < 0)
        //            offset[j] = maxs[j];
        //        else
        //            offset[j] = mins[j];
        //    }
        //    dist = vec3_dot(offset, plane->normal);
        //    dist = plane->dist - dist;
        //} else {
        //    // special point case
        //    dist = plane->dist;
        //}

        //const float d1 = vec3_dot(p1, plane->normal) - dist;
        //const float d2 = vec3_dot(p2, plane->normal) - dist;

        if (d2 > 0) {
            getOut = true; // endpoint is not in solid
        }
        if (d1 > 0) {
            startOut = true;
        }

        // if completely in front of face, no intersection
        if (d1 > 0 && d2 >= d1) {
            return;
        }

        if (d1 <= 0 && d2 <= 0) {
            continue;
        }

        // Crosses faces.
        float f = d1 - d2;

        if (f > 0) { // Enter.
            f = d1 / f;
            if (f > enterFractionA) {
                enterFractionA = f;
                clipPlane = plane;
                leadSide = side;
                enterFractionB = (d1 - DIST_EPSILON) / (d1 - d2);
            }
        } else if (f < 0) { // Leave.
            f = d1 / f;
            if (f < leaveFraction) {
                leaveFraction = f;
            }
        }
    }

    if (startOut == false) {
        // Original point was inside brush.
        traceWork.traceResult.startSolid = true;

        // Set contents.
        traceWork.contents = brush->contents;

        if (getOut == false) {
            traceWork.realFraction = 0.f;
            traceWork.traceResult.allSolid = true;
            traceWork.traceResult.fraction = 0.f;
        }
    }

    if (enterFractionA <= -1) {
        return;
    }

    if (enterFractionA > leaveFraction) {
        return;
    }

    // Check if this reduces collision time range.
    if (enterFractionA < traceWork.realFraction) {
        if (enterFractionB < traceWork.traceResult.fraction) {
            traceWork.realFraction = enterFractionA;
            traceWork.traceResult.plane = *clipPlane;
            traceWork.traceResult.surface = &(leadSide->texinfo->c);
            traceWork.traceResult.contents = brush->contents;
            traceWork.traceResult.fraction = enterFractionB;
        }
    }
    //    // crosses face
    //    if (d1 > d2) {
    //        // enter
    //        fraction = (d1 - DIST_EPSILON) / (d1 - d2);
    //        if (fraction > enterFractionA) {
    //            enterFractionA = fraction;
    //            clipPlane = plane;
    //            leadSide = side;
    //        }
    //    } else {
    //        // leave
    //        fraction = (d1 + DIST_EPSILON) / (d1 - d2);
    //        if (fraction < leaveFraction)
    //            leaveFraction = fraction;
    //    }
    //}

    //if (!startOut) {
    //    // original point was inside brush
    //    traceWork.traceResult.startSolid = true;
    //    if (!getOut) {
    //        traceWork.traceResult.allSolid = true;
    //        if (!collisionModel.map_allsolid_bug->integer) {
    //            // original Q2 didn't set these
    //            traceWork.traceResult.fraction = 0;
    //            traceWork.traceResult.contents = brush->contents;
    //        }
    //    }
    //    return;
    //}
    //if (enterFractionA < leaveFraction) {
    //    if (enterFractionA > -1 && enterFractionA < traceWork.traceResult.fraction) {
    //        if (enterFractionA < 0) {
    //            enterFractionA = 0;
    //        }
    //        traceWork.traceResult.fraction = enterFractionA;
    //        traceWork.traceResult.plane = *clipPlane;
    //        traceWork.traceResult.surface = &(leadSide->texinfo->c);
    //        traceWork.traceResult.contents = brush->contents;
    //    }
    //}
}


/**
*   @brief Test whether trace is inside a brush box or not.
**/
static void CM_TestBoxInBrush(const vec3_t &mins, const vec3_t &maxs, const vec3_t &p1,  TraceResult *trace, mbrush_t *brush) {

    if (!brush->numsides) {
        return;
    }

    vec3_t offset = vec3_zero();

    mbrushside_t *brushSide = brush->firstbrushside;

    for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
        CollisionPlane *plane = brushSide->plane;

		if( plane->type < 3 ) {
			if( traceWork.startMins[plane->type] > plane->dist ) {
				return;
			}
		} else {
			switch( plane->signBits ) {
				case 0:
					if( plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] > plane->dist ) {
						return;
					}
					break;
				case 1:
					if( plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] > plane->dist ) {
						return;
					}
					break;
				case 2:
					if( plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] > plane->dist ) {
						return;
					}
					break;
				case 3:
					if( plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] > plane->dist ) {
						return;
					}
					break;
				case 4:
					if( plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] > plane->dist ) {
						return;
					}
					break;
				case 5:
					if( plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] > plane->dist ) {
						return;
					}
					break;
				case 6:
					if( plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] > plane->dist ) {
						return;
					}
					break;
				case 7:
					if( plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] > plane->dist ) {
						return;
					}
					break;
				default:
					//assert( 0 );
					return;
			}
        }


        // FIXME: special case for axial

        // general box case

        // push the plane out apropriately for mins/maxs

        // FIXME: use signBits into 8 way lookup for each mins/maxs
        //for (int32_t j = 0; j < 3; j++) {
        //    if (plane->normal[j] < 0)
        //        offset[j] = maxs[j];
        //    else
        //        offset[j] = mins[j];
        //}
        //float dist = vec3_dot(offset, plane->normal);
        //dist = plane->dist - dist;

        //float d1 = vec3_dot(p1, plane->normal) - dist;

        //// if completely in front of face, no intersection
        //if (d1 > 0) {
        //    return;
        //}

    }

    // inside this brush
    traceWork.traceResult.startSolid = traceWork.traceResult.allSolid = true;
    traceWork.traceResult.fraction = 0;
    traceWork.traceResult.contents = brush->contents;
}


/**
*   @brief 
**/
static void CM_TraceToLeaf(mleaf_t *leaf) {
    if (!(leaf->contents & traceWork.contents)) {
        return;
    }

    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *b = *leafbrush;

        if (b->checkcount == collisionModel.checkCount) {
            continue;   // Already checked this brush in another leaf
        }
        
        b->checkcount = collisionModel.checkCount;

        if (!(b->contents & traceWork.contents)) {
            continue;
        }
        
        CM_ClipBoxToBrush(traceWork.mins, traceWork.maxs, traceWork.start, traceWork.end, &traceWork.traceResult, b);
        
        if (!traceWork.traceResult.fraction) {
            return;
        }
    }

}

/**
*   @brief 
**/
static void CM_TestInLeaf(mleaf_t *leaf)
{
    if (!(leaf->contents & traceWork.contents)) {
        return;
    }
    
    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *b = *leafbrush;
        
        if (b->checkcount == collisionModel.checkCount) {
            continue;   // Already checked this brush in another leaf
        }
        
        b->checkcount = collisionModel.checkCount;

        if (!(b->contents & traceWork.contents)) {
            continue;
        }
        
        CM_TestBoxInBrush(traceWork.mins, traceWork.maxs, traceWork.start, &traceWork.traceResult, b);
        
        if (!traceWork.traceResult.fraction) {
            return;
        }
    }

}

/**
*   @brief 
**/
static void CM_RecursiveHullCheck(mnode_t *node, float p1f, float p2f, const vec3_t &p1, const vec3_t &p2) {

recheck:
    if (traceWork.traceResult.fraction <= p1f) {
        return;     // already hit something nearer
    }

    // If plane is NULL, we are in a leaf node
    CollisionPlane *plane = node->plane;
    if (!plane) {
        CM_TraceToLeaf((mleaf_t *)node);
        return;
    }

    //
    // find the point distances to the seperating plane
    // and the offset for the size of the box
    //
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;
    if (plane->type < 3) {
        t1 = p1[plane->type] - plane->dist;
        t2 = p2[plane->type] - plane->dist;
        offset = traceWork.extents[plane->type];
    } else {
        t1 = PlaneDiff(p1, plane);
        t2 = PlaneDiff(p2, plane);
        if (traceWork.isPoint) {
            offset = 0;
            traceWork.extents = vec3_zero();
        } else {
           offset = 2048.f;
           offset = fabs(traceWork.extents[0] * plane->normal[0]) +
                     fabs(traceWork.extents[1] * plane->normal[1]) +
                     fabs(traceWork.extents[2] * plane->normal[2]);
        }
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
    int32_t side = 0;
    float idist = 0.f;
    float fractionA = 0.f;
    float fractionB = 0.f;
    if (t1 < t2) {
        idist = 1.0f / (t1 - t2);
        side = 1;
        fractionB = (t1 + offset + DIST_EPSILON) * idist;
        fractionA = (t1 - offset + DIST_EPSILON) * idist;
    } else if (t1 > t2) {
        idist = 1.0f / (t1 - t2);
        side = 0;
        fractionB = (t1 - offset - DIST_EPSILON) * idist;
        fractionA = (t1 + offset + DIST_EPSILON) * idist;
    } else {
        side = 0;
        fractionA = 1.f;
        fractionB = 0.f;
    }

    // Move up to the node
    fractionA = Clampf(fractionA, 0.f, 1.f);

    float midf = p1f + (p2f - p1f) * fractionA;
    vec3_t mid = vec3_mix(p1, p2, fractionA);

    CM_RecursiveHullCheck(node->children[side], p1f, midf, p1, mid);

    // Go past the node
    fractionB = Clampf(fractionB, 0.f, 1.f);

    midf = p1f + (p2f - p1f) * fractionB;
    mid = vec3_mix(p1, p2, fractionB);

    CM_RecursiveHullCheck(node->children[side ^ 1], midf, p2f, mid, p2);
}



/**
*
*
* 
*   Box Tracing & Entity Clipping.
*
* 
*
**/
/**
*   @brief  Check for what brush contents reside at vec3 'p' inside given node list.
**/
int CM_PointContents(const vec3_t &p, mnode_t *headNode)
{
    if (!headNode) {
        return 0;
    }

    mleaf_t *leaf = BSP_PointLeaf(headNode, p);

    return leaf->contents;
}

/**
*   @brief  Same as PointContents but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
int CM_TransformedPointContents(const vec3_t &p, mnode_t *headNode, const vec3_t& origin, const vec3_t& angles)
{
    vec3_t temp = vec3_zero();
    vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();

    if (!headNode) {
        return 0;
    }

    // subtract origin offset
    vec3_t p_l = vec3_zero();
    if (headNode == octagonHull.headNode) {
        p_l = (p - origin) - traceWork.cylinderOffset;
    } else {
        p_l = p - origin;
    } 

    vec3_t axis[3];
    // rotate start and end into the models frame of reference
#ifndef CFG_CM_ALLOW_ROTATING_BOXES
    if (headNode != boxHull.headNode && headNode != octagonHull.headNode && (angles[0] || angles[1] || angles[2])) {
#else
	if ((angles[0] || angles[1] || angles[2])) {
#endif
        AnglesToAxis(angles, axis);
        RotatePoint(p_l, axis);
    }

    return CM_PointContents(p_l, headNode);
}

/**
*   @brief Executes a box trace.
**/
const TraceResult CM_BoxTrace(const vec3_t &start, const vec3_t &end, const vec3_t &mins, const vec3_t &maxs, mnode_t *headNode, int32_t brushMask) {
    // Determine whether we are tracing world or not.
    bool worldTrace = (headNode != boxHull.headNode && headNode != octagonHull.headNode);

    // For multi-check avoidance.
    collisionModel.checkCount++;

    // Reset and fill in a default trace.
    traceWork.traceResult = {
        .fraction = 1,
        .surface = &(collisionModel.nullTextureInfo.c)
    };

    // Need a headNode to work with or bail out.
    if (!headNode) {
        return traceWork.traceResult;
    }

    // Prepare TraceWork for the current trace.
    traceWork.realFraction = 1 + DIST_EPSILON;
    traceWork.checkCount = collisionModel.checkCount;
    traceWork.contents = brushMask;
    traceWork.start = start;
    traceWork.end   = end;
    traceWork.mins  = mins;
    traceWork.maxs  = maxs;
    
    // Build a bounding box of the entire move.
    ClearBounds(traceWork.absMins, traceWork.absMaxs);

    // Calculate startMins and add points to bounds.
    traceWork.startMins = start + traceWork.mins;
    AddPointToBounds(traceWork.startMins, traceWork.absMins, traceWork.absMaxs);

    // Calculate startMaxs and add points to bounds.
    traceWork.startMaxs = start + traceWork.maxs;
    AddPointToBounds(traceWork.startMaxs, traceWork.absMins, traceWork.absMaxs);

    // Calculate endMins and add points to bounds.
    traceWork.endMins = end + traceWork.mins;
    AddPointToBounds(traceWork.endMins, traceWork.absMins, traceWork.absMaxs);

    // Calculate endMaxs and add points to bounds.
    traceWork.endMaxs = end + traceWork.maxs;
    AddPointToBounds(traceWork.endMaxs, traceWork.absMins, traceWork.absMaxs);


    //
    // check for position test special case
    //
    if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
        mleaf_t *leafs[1024];
        int32_t topNode = 0;

        if (worldTrace) {
			// Calculate c1 and c2 vectors.
			vec3_t c1 = start + mins + vec3_t{ -1.f, -1.f, -1.f };
			vec3_t c2 = start + maxs + vec3_t{ 1.f, 1.f, 1.f };            

			int32_t numleafs = CM_BoxLeafs_headnode(c1, c2, leafs, Q_COUNTOF(leafs), headNode, nullptr);
			for (int32_t i = 0; i < numleafs; i++) {
				CM_TestInLeaf(leafs[i]);
				if (traceWork.traceResult.allSolid) {
					break;
				}
			}
		} else {
			if (BoundsOverlap(start + mins, start + maxs, traceWork.absMins, traceWork.absMaxs)) {
				if (headNode == octagonHull.headNode) {
					CM_TestInLeaf(&octagonHull.leaf);
				} else {
					CM_TestInLeaf(&boxHull.leaf);
				}
			}
		}

        traceWork.traceResult.endPosition = start;

        return traceWork.traceResult;
	}
	
    //
    // check for point special case
    //
    //if (mins[0] == 0 && mins[1] == 0 && mins[2] == 0 && maxs[0] == 0 && maxs[1] == 0 && maxs[2] == 0) {
    if (vec3_equal(mins, vec3_zero()) && vec3_equal(maxs, vec3_zero())) {
        traceWork.isPoint = true;
        traceWork.extents = vec3_zero();
    } else {
        traceWork.isPoint = false;
        traceWork.extents[0] = -mins[0] > maxs[0] ? -mins[0] : maxs[0];
        traceWork.extents[1] = -mins[1] > maxs[1] ? -mins[1] : maxs[1];
        traceWork.extents[2] = -mins[2] > maxs[2] ? -mins[2] : maxs[2];
    }

    //
    // general sweeping through world
    //
	if (worldTrace) {
		CM_RecursiveHullCheck(headNode, 0, 1, start, end);
	} else if (BoundsOverlap(start + mins, start + maxs, traceWork.absMins, traceWork.absMaxs)) {
		if (headNode == octagonHull.headNode) {
			CM_TraceToLeaf(&octagonHull.leaf);
		} else {
			CM_TraceToLeaf(&boxHull.leaf);
		}
	}

    // Clamp.
    traceWork.traceResult.fraction = Clampf(traceWork.traceResult.fraction, 0.f, 1.f);

    // Lerp end position if necessary.
    if (traceWork.traceResult.fraction == 1) {
        traceWork.traceResult.endPosition = end;
    } else {
        traceWork.traceResult.endPosition = vec3_mix(start, end, traceWork.traceResult.fraction);
    }

	return traceWork.traceResult;
}

/**
*   @brief  Same as CM_TraceBox but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedBoxTrace(const vec3_t &start, const vec3_t &end, const vec3_t &mins, const vec3_t &maxs, mnode_t *headNode, int32_t brushMask, const vec3_t &origin, const vec3_t &angles)
{
    vec3_t      axis[3] = { vec3_zero(), vec3_zero(), vec3_zero()};
    qboolean    rotated = false;

    // Reset tracework.
    traceWork = {};

    // Calculate end and start l.
    vec3_t end_l = vec3_zero();
    vec3_t start_l = vec3_zero();

    if (headNode == octagonHull.headNode) {
        // Octagon Cylinder offset.
        start_l = start - traceWork.cylinderOffset;
        end_l   = end - traceWork.cylinderOffset;
    } else {
        start_l = start;
        end_l   = end;
    }

    // Substract Origin offset.
    start_l -= origin;
    end_l   -= origin;

    // Rotate start and end into the models frame of reference.
#ifndef CFG_CM_ALLOW_ROTATING_BOXES
	if ((headNode != boxHull.headNode && headNode != octagonHull.headNode) && (angles[0] || angles[1] || angles[2])) {
#else
	if ((angles[0] || angles[1] || angles[2])) {
#endif
        rotated = true;

        AnglesToAxis(angles, axis);
        RotatePoint(start_l, axis);
        RotatePoint(end_l, axis);
    } else {
        rotated = false;
    }

    // Sweep the box through the model.
    CM_BoxTrace(start_l, end_l, mins, maxs, headNode, brushMask);

    // Rotate plane normal back into the worlds frame of reference.
    if (rotated && traceWork.traceResult.fraction != 1.0) {
        TransposeAxis(axis);
        RotatePoint(traceWork.traceResult.plane.normal, axis);
    }

    
	// Clamp fraction just to be sure.
	traceWork.traceResult.fraction = Clampf(traceWork.traceResult.fraction, 0.0, 1.0);

	// FIXME: offset plane distance?
    traceWork.traceResult.endPosition = vec3_mix(start, end, traceWork.traceResult.fraction); // LerpVector(start, end, traceWork.traceResult.fraction, traceWork.traceResult.endPosition);

	// Return trace result.
	return traceWork.traceResult;
}


/**
*   @brief  Clips the source trace result against given entity.
**/
void CM_ClipEntity(TraceResult *dst, const TraceResult *src, struct PODEntity *ent)
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



/**
*
*
* 
*   AREAPORTALS
*
* 
*
**/
/**
*   @brief
**/
static void FloodArea_r(cm_t *cm, int number, int floodnum)
{
    int i;
    mareaportal_t *p;
    marea_t *area;

    area = &cm->cache->areas[number];
    if (area->floodvalid == collisionModel.floodValid) {
        if (cm->floodnums[number] == floodnum) {
            return;
        }
        Com_Error(ErrorType::Drop, "FloodArea_r: reflooded");
    }

    cm->floodnums[number] = floodnum;
    area->floodvalid = collisionModel.floodValid;
    p = area->firstareaportal;
    for (i = 0; i < area->numareaportals; i++, p++) {
        if (cm->portalopen[p->portalnum]) {
            FloodArea_r(cm, p->otherarea, floodnum);
        }
    }
}

/**
*   @brief
**/
static void FloodAreaConnections(cm_t *cm)
{
    marea_t *area;

    // All current floods are now invalid
    collisionModel.floodValid++;
    int32_t floodnum = 0;

    // Area 0 is not used
    for (int32_t i = 1; i < cm->cache->numareas; i++) {
        area = &cm->cache->areas[i];
        if (area->floodvalid == collisionModel.floodValid) {
            continue;       // already flooded into
        }
        floodnum++;
        FloodArea_r(cm, i, floodnum);
    }
}

/**
*   @brief
**/
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

/**
*   @brief
**/
qboolean CM_AreasConnected(cm_t *cm, int area1, int area2)
{
    bsp_t *cache = cm->cache;

    if (!cache) {
        return false;
    }
    if (collisionModel.map_noareas->integer) {
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

/**
*   @brief  Writes a length byte followed by a bit vector of all the areas
*           that area in the same flood as the area parameter
*
*           This is used by the client refreshes to cull visibility
**/
int CM_WriteAreaBits(cm_t *cm, byte *buffer, int area)
{
    bsp_t *cache = cm->cache;

    if (!cache) {
        return 0;
    }

    int32_t bytes = (cache->numareas + 7) >> 3;

    if (collisionModel.map_noareas->integer || !area) {
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

/**
*   @brief
**/
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

/**
*   @brief
**/
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

/**
*   @return True if any leaf under headNode has a cluster that is potentially visible
**/
qboolean CM_HeadnodeVisible(mnode_t *node, byte *visbits)
{
    while (node->plane) {
        if (CM_HeadnodeVisible(node->children[0], visbits)) {
            return true;
        }
        node = node->children[1];
    }

    mleaf_t *leaf = (mleaf_t *)node;
    int32_t cluster = leaf->cluster;
    if (cluster == -1) {
        return false;
    }
    if (Q_IsBitSet(visbits, cluster)) {
        return true;
    }
    return false;
}

/**
*   @brief  The client will interpolate the view position, so we can't use a single PVS point
**/
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
    if (count < 1) {
        Com_Error(ErrorType::Drop, "CM_FatPVS: leaf count < 1");
    }
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


/**
*   @brief 
**/
void CM_Init(void)
{
    CM_InitBoxHull();
    CM_InitOctagonBoxHull();

    collisionModel.nullLeaf.cluster = -1;

    collisionModel.map_noareas = Cvar_Get("map_noareas", "0", 0);
    collisionModel.map_allsolid_bug = Cvar_Get("map_allsolid_bug", "1", 0);
}

