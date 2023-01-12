/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Box Tracing API.
*
***/
#include "Shared/Shared.h"

#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CVar.h"
#include "Common/CollisionModel.h"
#include "Common/Common.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

#include "Common/CollisionModel/BoundingBoxHull.h"
#include "Common/CollisionModel/OctagonBoxHull.h"
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"


//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;
//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;

//
//	TODO: Remove these after having "mainstreamed/unified" our sorry, mixed up vector maths.
//	The implementations reside in /Common/CollisionModel.cpp
//
void CM_AngleVectors( const vec3_t &angles, vec3_t &forward, vec3_t &right, vec3_t &up );
void CM_AnglesToAxis( const vec3_t &angles, vec3_t axis[3]);
void CM_Matrix_TransformVector( vec3_t m[3], const vec3_t &v, vec3_t &out );



/**
*
*
*   BSPTree Traversal, Testing & Clipping.
*
*
**/
// Enables proper endPosition for non axial planes.
#define TRACEFIX
// Enables proper rotating by using matrix transforms.
#define ROTATEFIX


// 1/32 epsilon to keep floating point happy
static constexpr float DIST_EPSILON = 1.0f / 32.0f;
//#define DIST_EPSILON    0.125
// Fraction Epsilon
#ifdef TRACEFIX
//static constexpr float FRAC_EPSILON = FLT_EPSILON; // This FLT_EPSILON method works also, should be more precise.
static constexpr float FRAC_EPSILON = 1.0f / 1024.0f;
#endif

/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
static void CM_TestBoxInBrush(const vec3_t &mins, const vec3_t &maxs, const vec3_t &p1,  TraceResult *trace, mbrush_t *brush, mleaf_t *leaf) {

    if (!brush->numsides) {
        return;
    }

	// special test for axial
	// the first 6 brush planes are always axial
	if ( traceWork.startMins[ 0 ] > leaf->maxs[ 0 ]
		|| traceWork.startMins[ 1 ] > leaf->maxs[ 1 ]
		|| traceWork.startMins[ 2 ] > leaf->maxs[ 2 ]
		|| traceWork.startMaxs[ 0 ] < leaf->mins[ 0 ]
		|| traceWork.startMaxs[ 1 ] < leaf->mins[ 1 ] 
		|| traceWork.startMaxs[ 2 ] < leaf->mins[ 2 ] )
	{
		return;
	}

    vec3_t offset = vec3_zero();
    mbrushside_t *brushSide = brush->firstbrushside + 6;

    for (int32_t i = 6; i < brush->numsides; i++, brushSide++) {
        CollisionPlane *plane = brushSide->plane;
		
		// push the plane out appropriately for mins/maxs
		// if completely in front of face, no intersection
		float dist = plane->dist - vec3_dot( traceWork.offsets[ plane->signBits ], plane->normal );
		float d1 = vec3_dot( p1, plane->normal ) - dist;
		
		if ( d1 > 0 ) {
			return;
		}
    }

    // inside this brush
    traceWork.traceResult.startSolid = traceWork.traceResult.allSolid = true;
    traceWork.traceResult.fraction = 0;
    traceWork.traceResult.contents = brush->contents;
}
/**
*   @brief 
**/
static void CM_TestInLeaf( cm_t *cm, mleaf_t *leaf ) {
    if (!(leaf->contents & traceWork.contents)) {
        return;
    }
    
    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *b = *leafbrush;
        
        if (b->checkcount == cm->checkCount) {
            continue;   // Already checked this brush in another leaf
        }
        
        b->checkcount = cm->checkCount;

        if (!(b->contents & traceWork.contents)) {
            continue;
        }
        
        CM_TestBoxInBrush( traceWork.mins, traceWork.maxs, traceWork.start, &traceWork.traceResult, b, leaf );
        
        if (!traceWork.traceResult.fraction) {
            return;
        }
    }

}



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

#ifdef TRACEFIX
	float enterDistance = 0.f, move = 1.f;
#endif

    vec3_t offset = vec3_zero();

    CollisionPlane    *clipPlane = nullptr;
    mbrushside_t *leadSide = nullptr;
    mbrushside_t *side = brush->firstbrushside;

    for (int32_t i = 0; i < brush->numsides; i++, side++) {
        CollisionPlane *plane = side->plane;

		// General Box Case: Push the plane out for mins/maxs.
		//float dist = plane->dist - vec3_dot( traceWork.offsets[ plane->signBits ], plane->normal );
		//float d1 = vec3_dot( p1, plane->normal ) - dist;
		//float d2 = vec3_dot( p2, plane->normal ) - dist;

		// Special Point Case:
        // FIXME: special case for axial
		float d1 = 0.f;
		float d2 = 0.f;
		if( plane->type < 3 ) {
			d1 = traceWork.startMins[plane->type] - plane->dist;
			d2 = traceWork.endMins[plane->type] - plane->dist;
		} else {
			if (!traceWork.isPoint) {
				// general box case
				// push the plane out apropriately for mins/maxs
				dist = vec3_dot( traceWork.offsets[plane->signBits], plane->normal );
				dist = plane->dist - dist;
			} else {
				// special point case
				dist = plane->dist;
			}

		    d1 = DotProduct(p1, plane->normal) - dist;
	        d2 = DotProduct(p2, plane->normal) - dist;
		}

		// FIXME: special case for axial
		//float d1 = 0.f;
		//float d2 = 0.f;
		// push the plane out apropriately for mins/maxs
		//if( plane->type < 3 ) {
		//	d1 = traceWork.startMins[plane->type] - plane->dist;
		//	d2 = traceWork.endMins[plane->type] - plane->dist;
		//} else {
		//	switch( plane->signBits ) {
		//		case 0:
		//			d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
		//			break;
		//		case 1:
		//			d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
		//			break;
		//		case 2:
		//			d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
		//			break;
		//		case 3:
		//			d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMins[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMins[2] - plane->dist;
		//			break;
		//		case 4:
		//			d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
		//			break;
		//		case 5:
		//			d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMins[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMins[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
		//			break;
		//		case 6:
		//			d1 = plane->normal[0] * traceWork.startMins[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMins[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
		//			break;
		//		case 7:
		//			d1 = plane->normal[0] * traceWork.startMaxs[0] + plane->normal[1] * traceWork.startMaxs[1] + plane->normal[2] * traceWork.startMaxs[2] - plane->dist;
		//			d2 = plane->normal[0] * traceWork.endMaxs[0] + plane->normal[1] * traceWork.endMaxs[1] + plane->normal[2] * traceWork.endMaxs[2] - plane->dist;
		//			break;
		//		default:
		//			d1 = d2 = 0; // shut up compiler
		//			//assert( 0 );
		//			break;
		//	}
		//}

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

#ifdef TRACEFIX
		float f = d1 - d2;

		if ( f > 0 ) {
			f = d1 / f;

			if ( f > enterFractionA ) {
				enterDistance = d1;
				move = d1 - d2;
				enterFractionA = f;
				clipPlane = plane;
				leadSide = side;
			}
		} else if ( f < 0 ) {
			f = d1 / f;
			if ( f < leaveFraction ) {
				leaveFraction = f;
			}
		}
#else
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
#endif
	}

    if (startOut == false) {
        // Original point was inside brush.
        traceWork.traceResult.startSolid = true;

        // Set contents.
        traceWork.contents = brush->contents;

        if (getOut == false) {
			// TRACEFIX: DO WE NEED THIS ?
#ifndef TRACEFIX
            traceWork.realFraction = 0.f;
#endif
			// TRACEFIX: END OF COMMENT
            traceWork.traceResult.allSolid = true;
            traceWork.traceResult.fraction = 0.f;
        }
    }

	// TRACEFIX: DO WE NEED THIS IF STATEMENT?
	// Check if this reduces collision time range.
#ifndef TRACEFIX
    if (enterFractionA <= -1) {
        return;
    }

    if (enterFractionA > leaveFraction) {
        return;
    }
#endif
	// END OF COMMENT.

#ifdef TRACEFIX
	if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
		if ( enterFractionA > -1 && enterFractionA < traceWork.realFraction ) {
			if ( enterFractionA < 0 ) {
				enterFractionA = 0;
			}

			traceWork.realFraction = enterFractionA;
            traceWork.traceResult.plane = *clipPlane;
            traceWork.traceResult.surface = &(leadSide->texinfo->c);
            traceWork.traceResult.contents = brush->contents;
			traceWork.traceResult.fraction = ( enterDistance - DIST_EPSILON ) / move;
			if ( traceWork.traceResult.fraction < 0 ) {
				traceWork.traceResult.fraction = 0;
			}
		}
	}
#else
	// PREVIOUS:
    if (enterFractionA < traceWork.realFraction) {
        if (enterFractionB < traceWork.traceResult.fraction) {
            traceWork.realFraction = enterFractionA;
            traceWork.traceResult.plane = *clipPlane;
            traceWork.traceResult.surface = &(leadSide->texinfo->c);
            traceWork.traceResult.contents = brush->contents;
            traceWork.traceResult.fraction = enterFractionB;
        }
    }
#endif
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
static void CM_RecursiveHullCheck(mnode_t *node, float p1f, float p2f, const vec3_t &p1, const vec3_t &p2) {
recheck:
#ifdef TRACEFIX
    if (traceWork.realFraction <= p1f) {
        return;     // already hit something nearer
    }
#else
    if (traceWork.traceResult.fraction <= p1f) {
        return;     // already hit something nearer
    }
#endif

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
#ifdef TRACEFIX
    if (plane->type < 3) {
        t1 = p1[plane->type] - plane->dist;
        t2 = p2[plane->type] - plane->dist;
        offset = traceWork.extents[plane->type];
    } else {
        t1 = vec3_dot(plane->normal, p1) - plane->dist;
        t2 = vec3_dot(plane->normal, p2) - plane->dist;
        if (traceWork.isPoint) {
			offset = 0;
			traceWork.extents = vec3_zero();
        } else {
           //offset = 2048.f;
           offset =	fabs(traceWork.extents[0] * plane->normal[0]) +
					fabs(traceWork.extents[1] * plane->normal[1]) +
					fabs(traceWork.extents[2] * plane->normal[2]);
        }
    }
    // see which sides we need to consider
    if (t1 >= offset && t2 >= offset) {
        node = node->children[0];
        goto recheck;
    }
    if (t1 < -offset && t2 < -offset ) {
        node = node->children[1];
        goto recheck;
    }
#else
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
           //offset = 2048.f;
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
#endif

    // put the crosspoint DIST_EPSILON pixels on the near side
    int32_t side = 0;
    float idist = 0.f;
    float fractionA = 0.f;
    float fractionB = 0.f;
    if (t1 < t2) {
        idist = 1.0f / (t1 - t2);
        side = 1;
#ifdef TRACEFIX
        fractionB = (t1 + offset) * idist;
        fractionA = (t1 - offset) * idist;
#else
        fractionB = (t1 + offset + DIST_EPSILON) * idist;
        fractionA = (t1 - offset + DIST_EPSILON) * idist;
#endif
    } else if (t1 > t2) {
        idist = 1.0f / (t1 - t2);
        side = 0;
#ifdef TRACEFIX
		fractionB = (t1 - offset) * idist;
        fractionA = (t1 + offset) * idist;
#else
		fractionB = (t1 - offset - DIST_EPSILON) * idist;
        fractionA = (t1 + offset + DIST_EPSILON) * idist;
#endif
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
*   @brief	Operates as the main trace function, used by other trace functions after changing model frame of references
*			or other necessaties.
**/
const TraceResult CM_Trace( cm_t *cm, const vec3_t &start, const vec3_t &end, const vec3_t &mins, const vec3_t &maxs, mnode_t *headNode, int32_t brushMask ) {
	/**
	*	Determine whether we are tracing the world or not, increase checkCount for preventing
	*	testing planes multiple times as well as fill in default trace result to start with and/or
	*	return in case we got no head node to work from.
	**/
    // Determine whether we are tracing world or not.
    bool worldTrace = (headNode != boxHull.headNode && headNode != octagonHull.headNode);

    // For multi-check avoidance.
    collisionModel.checkCount++;

    // Reset and fill in a default trace.
    traceWork.traceResult = {
	#ifdef TRACEFIX
        .fraction = traceWork.realFraction = 1,
	#else
		.fraction = 1,
	#endif
        .surface = &(CM_GetNullTextureInfo()->c)
    };

    // Need a headNode to work with or bail out.
    if (!headNode) {
        return traceWork.traceResult;
    }

    // Prepare TraceWork for the current trace.
#ifndef TRACEFIX
	traceWork.realFraction = 1 + DIST_EPSILON;
#endif
    //traceWork.realFraction = 1;
	traceWork.checkCount = collisionModel.checkCount;
    traceWork.contents = brushMask;
    traceWork.start = start;
    traceWork.end   = end;
    traceWork.mins  = mins;
    traceWork.maxs  = maxs;
    
    // Build a bounding box of the entire move.
    ClearBounds(traceWork.absMins, traceWork.absMaxs);
	/**
	*	Calculate 'tw.offsets[signbits] = vector to apropriate corner from origin'.
	**/
	traceWork.maxOffset = traceWork.mins[ 0 ] + traceWork.maxs[ 1 ] + traceWork.maxs[ 2 ];
	
	//const vec3_t *bounds[2] = { &traceWork.mins, &traceWork.maxs };
	//for (int32_t i = 0; i < 8; i++) {
	//	for (int32_t j = 0; j < 3; j++) {
	//		traceWork.offsets[i][j] = *bounds[i >> j & 1][j];
	//	}
	//}

	traceWork.offsets[0][0] = traceWork.mins[0];
	traceWork.offsets[0][1] = traceWork.mins[1];
	traceWork.offsets[0][2] = traceWork.mins[2];

	traceWork.offsets[1][0] = traceWork.maxs[0];
	traceWork.offsets[1][1] = traceWork.mins[1];
	traceWork.offsets[1][2] = traceWork.mins[2];

	traceWork.offsets[2][0] = traceWork.mins[0];
	traceWork.offsets[2][1] = traceWork.maxs[1];
	traceWork.offsets[2][2] = traceWork.mins[2];

	traceWork.offsets[3][0] = traceWork.maxs[0];
	traceWork.offsets[3][1] = traceWork.maxs[1];
	traceWork.offsets[3][2] = traceWork.mins[2];

	traceWork.offsets[4][0] = traceWork.mins[0];
	traceWork.offsets[4][1] = traceWork.mins[1];
	traceWork.offsets[4][2] = traceWork.maxs[2];

	traceWork.offsets[5][0] = traceWork.maxs[0];
	traceWork.offsets[5][1] = traceWork.mins[1];
	traceWork.offsets[5][2] = traceWork.maxs[2];

	traceWork.offsets[6][0] = traceWork.mins[0];
	traceWork.offsets[6][1] = traceWork.maxs[1];
	traceWork.offsets[6][2] = traceWork.maxs[2];

	traceWork.offsets[7][0] = traceWork.maxs[0];
	traceWork.offsets[7][1] = traceWork.maxs[1];
	traceWork.offsets[7][2] = traceWork.maxs[2];

	/**
    *	Build a bounding box of the entire move.
	**/
	// Clear bounds first.
	ClearBounds(traceWork.absMins, traceWork.absMaxs);
    // Calculate startMins and add points to bounds.
    traceWork.startMins = traceWork.start + traceWork.mins;
    AddPointToBounds(traceWork.startMins, traceWork.absMins, traceWork.absMaxs);
    // Calculate startMaxs and add points to bounds.
    traceWork.startMaxs = traceWork.start + traceWork.maxs;
    AddPointToBounds(traceWork.startMaxs, traceWork.absMins, traceWork.absMaxs);
    // Calculate endMins and add points to bounds.
    traceWork.endMins = traceWork.end + traceWork.mins;
    AddPointToBounds(traceWork.endMins, traceWork.absMins, traceWork.absMaxs);
    // Calculate endMaxs and add points to bounds.
    traceWork.endMaxs = traceWork.end + traceWork.maxs;
    AddPointToBounds(traceWork.endMaxs, traceWork.absMins, traceWork.absMaxs);


    /**
    *	Check for position test special case
    **/
    if (start[0] == end[0] && start[1] == end[1] && start[2] == end[2]) {
        mleaf_t *leafs[1024];
        int32_t topNode = 0;

        if (worldTrace) {
			// Calculate c1 and c2 vectors.
			vec3_t c1 = traceWork.start + traceWork.mins + vec3_t{ -1.f, -1.f, -1.f };
			vec3_t c2 = traceWork.start + traceWork.maxs + vec3_t{ 1.f, 1.f, 1.f };            

			int32_t numleafs = CM_BoxLeafs_headnode(c1, c2, leafs, Q_COUNTOF(leafs), headNode, nullptr);
			for (int32_t i = 0; i < numleafs; i++) {
				CM_TestInLeaf( cm, leafs[i] );
				if (traceWork.traceResult.allSolid) {
					break;
				}
			}
		} else {
			if (BoundsOverlap(traceWork.start + traceWork.mins, traceWork.start + traceWork.maxs, traceWork.absMins, traceWork.absMaxs)) {
				if (headNode == octagonHull.headNode) {
					CM_TestInLeaf( cm, &octagonHull.leaf );
				} else {
					CM_TestInLeaf( cm, &boxHull.leaf );
				}
			}
		}

        traceWork.traceResult.endPosition = traceWork.start;

        return traceWork.traceResult;
	}
	
    /**
    *	Check for point special case
    **/
    //if (mins[0] == 0 && mins[1] == 0 && mins[2] == 0 && maxs[0] == 0 && maxs[1] == 0 && maxs[2] == 0) {
    if (vec3_equal(traceWork.mins, vec3_zero()) && vec3_equal(traceWork.maxs, vec3_zero())) {
        traceWork.isPoint = true;
        traceWork.extents = vec3_zero();
    } else {
        traceWork.isPoint = false;
        traceWork.extents[0] = -traceWork.mins[0] > traceWork.maxs[0] ? -traceWork.mins[0] : traceWork.maxs[0];
        traceWork.extents[1] = -traceWork.mins[1] > traceWork.maxs[1] ? -traceWork.mins[1] : traceWork.maxs[1];
        traceWork.extents[2] = -traceWork.mins[2] > traceWork.maxs[2] ? -traceWork.mins[2] : traceWork.maxs[2];
    }

    /**
    *	General sweeping through world
    **/
	if (worldTrace) {
		CM_RecursiveHullCheck(headNode, 0, 1, traceWork.start, traceWork.end);
	} else if (BoundsOverlap(traceWork.start + traceWork.mins, traceWork.start + traceWork.maxs, traceWork.absMins, traceWork.absMaxs)) {
		if (headNode == octagonHull.headNode) {
			CM_TraceToLeaf(&octagonHull.leaf);
		} else {
			CM_TraceToLeaf(&boxHull.leaf);
		}
	}

	/**
	*	Clamp fraction, and lerp trace endPosition using fraction if necessary.
	**/
#ifdef TRACEFIX
    traceWork.traceResult.fraction = Clampf(traceWork.traceResult.fraction, 0.f, 1.f);
#else
	traceWork.traceResult.fraction = Clampf(traceWork.traceResult.fraction, 0.f, 1.f);
#endif

    // Lerp end position if necessary.
    if (traceWork.traceResult.fraction == 1) {
        traceWork.traceResult.endPosition = end;
    } else {
        traceWork.traceResult.endPosition = vec3_mix(start, end, traceWork.traceResult.fraction);
    }

	return traceWork.traceResult;
}

/**
*   @brief  General box tracing routine.
**/
const TraceResult CM_BoxTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const vec3_t &mins, const vec3_t &maxs, mnode_t *headNode, int32_t brushMask ) {
	return CM_Trace( cm, start, end, mins, maxs, headNode, brushMask );
}

/**
*   @brief  Same as CM_TraceBox but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedBoxTrace( cm_t *cm, const vec3_t &start, const vec3_t &end, const vec3_t &mins, const vec3_t &maxs, mnode_t *headNode, int32_t brushMask, const vec3_t &origin, const vec3_t &angles ) {
    vec3_t      axis[3] = { vec3_zero(), vec3_zero(), vec3_zero()};
    qboolean    rotated = false;

    // Reset tracework.
    traceWork = {};

    // Calculate end and start l.
    vec3_t end_l = vec3_zero();
    vec3_t start_l = vec3_zero();

	/**
	*	Adjust so that mins and maxs are always symmetric, which avoids some complications 
	*	with plane expanding of rotated bmodels.
	**/
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

    
	/**
	*	Rotate start and end into the models frame of reference.
	**/
#ifndef CFG_CM_ALLOW_ROTATING_BOXES
	if ((headNode != boxHull.headNode && headNode != octagonHull.headNode) && (angles[0] || angles[1] || angles[2])) {
#else
	if ((angles[0] || angles[1] || angles[2])) {
#endif
        rotated = true;

#ifdef ROTATEFIX
		// Create angle matrix.
		CM_AnglesToAxis( angles, axis );
		// Rotate start and end into model's frame of reference.
		vec3_t temp = start_l;
		CM_Matrix_TransformVector( axis, temp, start_l);
		temp = end_l;
		CM_Matrix_TransformVector( axis, temp, end_l);
#else
        CM_AnglesToAxis(angles, axis);
        RotatePoint(start_l, axis);
        RotatePoint(end_l, axis);
#endif

    } else {
        rotated = false;
    }

    /**
	*	Sweep the box through the model.
	**/
    traceWork.traceResult = CM_Trace( cm, start_l, end_l, mins, maxs, headNode, brushMask );

    /**
	*	Rotate plane normal back into the worlds frame of reference.
	**/
    if ( rotated && traceWork.traceResult.fraction != 1.0 ) {
#ifdef ROTATEFIX
		// Create angle matrix.
		const vec3_t negatedAngles = vec3_negate( angles );
		CM_AnglesToAxis( negatedAngles, axis );
		// Rotate plane normal back into worlds frame of reference.
		vec3_t temp = traceWork.traceResult.plane.normal;
		CM_Matrix_TransformVector( axis, temp, traceWork.traceResult.plane.normal );
#else
		TransposeAxis(axis);
        RotatePoint(traceWork.traceResult.plane.normal, axis);
#endif
    }
    
	/**
	*	Clamp fraction, and lerp trace endPosition using fraction if necessary.
	**/
#ifdef TRACEFIX
	traceWork.traceResult.endPosition = vec3_mix(start, end, traceWork.traceResult.fraction); // LerpVector(start, end, traceWork.traceResult.fraction, traceWork.traceResult.endPosition);
#else
	traceWork.traceResult.fraction = Clampf(traceWork.traceResult.fraction, 0.0, 1.0);

	// FIXME: offset plane distance?
	if ( traceWork.traceResult.fraction == 1 ) {
		traceWork.traceResult.endPosition = end;
	} else {
		traceWork.traceResult.endPosition = vec3_mix(start, end, traceWork.traceResult.fraction); // LerpVector(start, end, traceWork.traceResult.fraction, traceWork.traceResult.endPosition);
	}
#endif

	// Return trace result.
	return traceWork.traceResult;
}

/**
*   @brief  Same as PointContents but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
int32_t CM_TransformedPointContents( cm_t *cm, const vec3_t &p, mnode_t *headNode, const vec3_t& origin, const vec3_t& angles ) {
    vec3_t temp = vec3_zero();
    vec3_t forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();

    if ( !headNode ) {
        return 0;
    }

    // subtract origin offset
    vec3_t p_l = vec3_zero();
    if ( headNode == octagonHull.headNode ) {
        p_l = ( p - origin ) - traceWork.cylinderOffset;
    } else {
        p_l = p - origin;
    } 

    vec3_t axis[3];
    // rotate start and end into the models frame of reference
#ifndef CFG_CM_ALLOW_ROTATING_BOXES
    if ( headNode != boxHull.headNode && headNode != octagonHull.headNode && ( angles[0] || angles[1] || angles[2] ) ) {
#else
	if ((angles[0] || angles[1] || angles[2])) {
#endif
		#ifdef ROTATEFIX
        CM_AnglesToAxis( angles, axis );
		vec3_t temp = p_l;
        CM_Matrix_TransformVector( axis, temp, p_l );
		#else
        AnglesToAxis( angles, axis );
        RotatePoint( p_l, axis );
		#endif
    }

    return CM_PointContents(cm, p_l, headNode);
}