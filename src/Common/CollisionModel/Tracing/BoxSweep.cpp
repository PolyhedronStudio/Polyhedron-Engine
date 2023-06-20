/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Box Sweep API.
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
#include "Common/CollisionModel/SphereHull.h"
#include "Common/CollisionModel/CapsuleHull.h"
#include "Common/CollisionModel/OctagonBoxHull.h"
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"

#include "Common/CollisionModel/Tracing/LeafTests.h"
#include "Common/CollisionModel/Tracing/LeafTraces.h"
#include "Common/CollisionModel/Tracing/BoxSweep.h"



/**
*
*
*	'BSP World Tree' Box Sweeping:
*
*
**/
/**
*   @brief	Performs a recursive 'Trace Bounds Sweep' through the 'BSP World Tree Node', when successfully landing in a leaf node, perform its corresponding node trace.
**/
void CM_RecursiveBoxTraceThroughTree( TraceContext &traceContext, mnode_t *node, const float p1f, const float p2f, const vec3_t &p1, const vec3_t &p2 ) {
    if (!node) {
		return;
	}
recheck:
	// If plane is NULL, we are in a leaf node
    CollisionPlane *plane = node->plane;

//#ifdef TRACE_PLANE_SIDE_FIX
//	// If true, we already hit something nearer.
//    if (traceContext.realFraction <= p1f) {
//        return;
//    }
//#else
//	// If true, we already hit something nearer.
	//if (traceContext.traceResult.fraction <= p1f) {
	if (traceContext.realFraction <= p1f) {
		return;
	}
//#endif

	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
	CollisionPlane transformedPlane = *plane;
	//if ( traceContext.isTransformedTrace ) { TransformPlane }

    //
    // Find the point distances to the seperating plane and the offset for the size of the box.
    //
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;

	// Axial planes.
    if ( transformedPlane.type < 3 ) {
        t1 = p1[ transformedPlane.type ] - transformedPlane.dist;
        t2 = p2[ transformedPlane.type ] - transformedPlane.dist;
        offset = traceContext.aabbTrace.extents[ transformedPlane.type ];
	// Non axial planes use dot product testing.
    } else {
        t1 = vec3_dot( transformedPlane.normal, p1 ) - transformedPlane.dist;
        t2 = vec3_dot( transformedPlane.normal, p2 ) - transformedPlane.dist;
        if ( traceContext.isPoint ) {
			offset = 0;
        } else {
			offset = (fabs( traceContext.aabbTrace.extents.x * transformedPlane.normal.x ) +
                     fabs( traceContext.aabbTrace.extents.y * transformedPlane.normal.y ) +
                     fabs( traceContext.aabbTrace.extents.z * transformedPlane.normal.z )) * 3;
		}
    }

    // See which sides we need to consider.
    if ( t1 >= offset && t2 >= offset ) { //if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
		// Get child node.
		node = node->children[0];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// TODO: First checking for any intersection might be a good thing to do here.
			CM_TraceBox_TraceThroughLeaf( traceContext, leafNode );
			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }
	if ( t1 < -offset && t2 < -offset ) { //if ( t1 < -offset /*- 1.f */&& t2 < -offset /*- 1.f */) {
		// Get child node.
		node = node->children[1];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// TODO: First checking for any intersection might be a good thing to do here.
			CM_TraceBox_TraceThroughLeaf( traceContext, leafNode );
			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }

    // Dp NOT put the crosspoint DIST_EPSILON pixels on the near side
    int32_t side = 0;
    float idist = 0.f;
    float fractionA = 0.f;
    float fractionB = 0.f;
    if (t1 < t2) {
        idist = 1.0f / (t1 - t2);
        side = 1;
        fractionB = (t1 + offset /*+ DIST_EPSILON*/) * idist;
        fractionA = (t1 - offset /*+ DIST_EPSILON*/) * idist;
    } else if (t1 > t2) {
        idist = 1.0f / (t1 - t2);
        side = 0;
		fractionB = (t1 - offset /*- DIST_EPSILON*/) * idist;
        fractionA = (t1 + offset /*+ DIST_EPSILON*/) * idist;
    } else {
        side = 0;
        fractionA = 1.f;
        fractionB = 0.f;
    }

	// Move up to the node in case we can potentially hit it.
	if ( p1f < traceContext.realFraction ) {
		fractionA = Clampf( fractionA, 0.f, 1.f );

		float midf = p1f + ( p2f - p1f ) * fractionA;
		vec3_t mid = vec3_mix( p1, p2, fractionA );

		mnode_t *childNode = node->children[side];
		if ( !childNode->plane ) {
			CM_TraceBox_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveBoxTraceThroughTree( traceContext, childNode, p1f, midf, p1, mid );
		}
	}

    // Go past the node.
	fractionB = Clampf( fractionB, 0.f, 1.f );
    const float midf = p1f + ( p2f - p1f ) * fractionB;

	if ( midf < traceContext.realFraction ) {
		const vec3_t mid = vec3_mix( p1, p2, fractionB );

		mnode_t *childNode = node->children[side ^ 1];
		if ( !childNode->plane ) {
			CM_TraceBox_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveBoxTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
		}
	}
}
