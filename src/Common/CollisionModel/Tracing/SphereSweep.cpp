/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Sphere Sweep API.
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
#include "Common/CollisionModel/Tracing/SphereSweep.h"



/**
*	In-Place Hulls: Used during Leaf/Brush Tests and Leaf/Brush Traces when requiring to 
*	convert a said passed-in Leaf Node into a different Shape type Hull Leaf Node.
**/
//! For 'Box' leaf tracing.
extern BoxHull leafTraceBoxHull;
//! For 'Sphere' leaf tracing.
extern SphereHull leafTraceSphereHull;
//! For 'Capsule' leaf tracing.
extern CapsuleHull leafTraceCapsuleHull;



/**
*
*
*	'BSP World Tree' Sphere Sweeping:
*
*
**/
const bool CM_SphereSweep_SphereTraceIntersectLeaf( TraceContext &traceContext, mleaf_t *leafNode ) {
	//
	// First attempt:
	//

	////// Calculate leaf sphere.
	//sphere_t leafSphere = sphere_from_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );	//sphere_t testSphere = CM_SphereFromBounds( leafBounds, vec3_zero() );
	//leafSphere.offset = { 0.f, 0.f, 0.f };
	//if ( traceContext.isTransformedTrace ) {
	//	leafSphere.origin = { 0.f, 0.f, 0.f };
	////	CM_Matrix_TransformSphere( traceContext.matTransform, leafSphere );
	//}
	//sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, leafSphere, true );

	////// Test for intersection.
	//return CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );


	////
	//// Third Attempt:
	////
	//sphere_t transformedTestSphere = sphere_from_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );	//sphere_t testSphere = CM_SphereFromBounds( leafBounds, vec3_zero() );;

	//// Transform bounds if we're dealing with a transformed trace.
	//if ( traceContext.isTransformedTrace ) {
	//	glm::vec4 transformedOffset = traceContext.matInvTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	//	glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
	//	transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
	//}

	//// Calculate offset rotation.
	//sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTestSphere, traceContext.isTransformedTrace );

	//return bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );


	//
	// Fourth Attempt:
	//
	//sphere_t transformedTestSphere = sphere_from_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );	//sphere_t testSphere = CM_SphereFromBounds( leafBounds, vec3_zero() );;
	sphere_t transformedTestSphere = traceContext.traceSphere;

	//// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matInvTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
		sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTestSphere, traceContext.isTransformedTrace );
	
	}

	//// Calculate offset rotation.
	bbox3_t bboxSym = bbox3_from_center_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );
	return bbox3_intersects_sphere( bboxSym, transformedTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );




	////
	//// Second Attempt:
	////
	// Leaf bounds.
	//const bbox3_t leafNodeBounds = leafNode->bounds;

	//// Test sphere.
	//sphere_t transformedTestSphere = traceContext.traceSphere;

	//// Transform bounds if we're dealing with a transformed trace.
	//if ( traceContext.isTransformedTrace ) {
	//	//glm::vec4 transformedOffset = traceContext.matTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	//	//glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
	//	//transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
	//}
	//// Calculate offset rotation.
	////sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTestSphere, traceContext.isTransformedTrace );

	//return bbox3_intersects_sphere( leafNodeBounds, transformedTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
}

/**
*   @brief	Performs a recursive 'Trace Bounds Sweep' through the 'BSP World Tree Node', when successfully landing in a leaf node, perform its corresponding node trace.
**/
void CM_RecursiveSphereTraceThroughTree( TraceContext &traceContext, mnode_t *node, const float p1f, const float p2f, const vec3_t &p1, const vec3_t &p2 ) {
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
	if (traceContext.traceResult.fraction <= p1f) {
		return;
	}
//#endif

	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
	CollisionPlane transformedPlane = *plane;
	// Make sure its normal is normalized properly for sphere testing.
	//transformedPlane.normal = vec3_normalize( transformedPlane.normal );

	//
	//	Get our sphere data.
	//
	sphere_t testSphere = traceContext.traceSphere;


	// Sphere offset origin 
	const vec3_t sphereOffsetOrigin = ( traceContext.isTransformedTrace ? testSphere.origin + testSphere.offset : testSphere.offset );

    //
    // Find the point distances to the seperating plane and the offset for the size of the box.
    //
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;
	const float dist = transformedPlane.dist + testSphere.offsetRadius;

	//
	// This almost worked, but we aim for the code below the nextb ig comment to work.
	//
	// Axial planes.
////	if ( transformedPlane.type < 3 ) {
////		t1 = p1[ transformedPlane.type ] - dist;
////		t2 = p2[ transformedPlane.type ] - dist;
////		offset = traceContext.extents[ transformedPlane.type ];
////// Non axial planes use dot product testing.
////	} else {
//		t1 = vec3_dot( transformedPlane.normal, p1 ) - dist;
//		t2 = vec3_dot( transformedPlane.normal, p2 ) - dist;
//		if ( traceContext.isPoint ) {
//			offset = 0;
//		} else {
//			offset = (fabs( traceContext.extents.x * transformedPlane.normal.x ) +
//						fabs( traceContext.extents.y * transformedPlane.normal.y ) +
//						fabs( traceContext.extents.z * transformedPlane.normal.z )) * 3;
//		}
////    }

	//
	// Current:
	//
	// Axial planes.
//	if ( transformedPlane.type < 3 ) {
//		t1 = p1[ transformedPlane.type ] - dist;
//		t2 = p2[ transformedPlane.type ] - dist;
//		offset = traceContext.extents[ transformedPlane.type ];
//// Non axial planes use dot product testing.
//	} else {
		t1 = vec3_dot( transformedPlane.normal, p1 ) - dist;
		t2 = vec3_dot( transformedPlane.normal, p2 ) - dist;
		if ( traceContext.isPoint ) {
			offset = 0;
		} else {
			offset = (fabs( traceContext.extents.x * transformedPlane.normal.x ) +
						fabs( traceContext.extents.y * transformedPlane.normal.y ) +
						fabs( traceContext.extents.z * transformedPlane.normal.z )) * 3;
		}
//    }


	//
	// Second Attempt:
	//
	//// Find the closest point on the capsule to the plane
	//const float t = vec3_dot( transformedPlane.normal, sphereOffsetOrigin );

	//// Calculate the actual sphere adjusted start(p1)/end(p2).
	////const vec3_t sphereP1 = ( t > 0.0f ? p1 - sphereOffsetOrigin : p1 + sphereOffsetOrigin );
	////const vec3_t sphereP2 = ( t > 0.0f ? p2 - sphereOffsetOrigin : p2 + sphereOffsetOrigin );
	//const vec3_t sphereP1 = p1;
	//const vec3_t sphereP2 = p2;
	//if ( transformedPlane.type < 3 ) {
	//	// Calculate start and end point.
	//	t1 = sphereP1[ transformedPlane.type ] - dist;
	//	t2 = sphereP2[ transformedPlane.type ] - dist;
	//	vec3_t sphereExtents = sphereOffsetOrigin + vec3_t{ 
	//		fabs( testSphere.origin.x + testSphere.offsetRadius ),
	//		fabs( testSphere.origin.y + testSphere.offsetRadius ),
	//		fabs( testSphere.origin.z + testSphere.offsetRadius )
	//	};
 //       offset = fabs( sphereExtents[ transformedPlane.type ] ) ;
	//	// Calculate sphere offset to plane.
	//	const float sphereExtentsX = testSphere.origin.x + testSphere.offsetRadius;
	//	const float sphereExtentsY = testSphere.origin.y + testSphere.offsetRadius;
	//	const float sphereExtentsZ = testSphere.origin.z + testSphere.offsetRadius;
	//	offset = ( fabs( sphereExtentsX * transformedPlane.normal.x ) +
	//				fabs( sphereExtentsY * transformedPlane.normal.y ) +
	//				fabs( sphereExtentsZ * transformedPlane.normal.z ) )/* * 3*/;
	//} else {
	//	// Calculate start and end point.
	//	t1 = vec3_dot( transformedPlane.normal, sphereP1 ) - dist;
	//	t2 = vec3_dot( transformedPlane.normal, sphereP2 ) - dist;

	//	// Calculate sphere offset to plane.
	//	const float sphereExtentsX = testSphere.origin.x + testSphere.offsetRadius;
	//	const float sphereExtentsY = testSphere.origin.y + testSphere.offsetRadius;
	//	const float sphereExtentsZ = testSphere.origin.z + testSphere.offsetRadius;
	//	offset = ( fabs( sphereExtentsX * transformedPlane.normal.x ) +
	//				fabs( sphereExtentsY * transformedPlane.normal.y ) +
	//				fabs( sphereExtentsZ * transformedPlane.normal.z ) )/* * 3*/;
	//}

    // See which sides we need to consider.
    if ( t1 >= offset && t2 >= offset ) { //if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
		// Get child node.
		node = node->children[0];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			
			// Test for intersection.
			//const bool traceIntersected = CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );
			
			
			//const bool traceIntersected = CM_SphereSweep_SphereTraceIntersectLeaf( traceContext, leafNode );
			//if ( traceIntersected ) {
				leafTraceSphereHull.sphere = traceContext.traceSphere;
				CM_TraceThroughLeaf( traceContext, leafNode );
				//CM_TraceBoxThroughSphere( traceContext, leafNode );
			//}
			
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
			// Test for intersection.
			//const bool traceIntersected = CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );
			
			
			//const bool traceIntersected = CM_SphereSweep_SphereTraceIntersectLeaf( traceContext, leafNode );
			//if ( traceIntersected ) {
				leafTraceSphereHull.sphere = traceContext.traceSphere;
				CM_TraceThroughLeaf( traceContext, leafNode );
				//CM_TraceBoxThroughSphere( traceContext, leafNode );
			//}
			
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
			leafTraceSphereHull.sphere = traceContext.traceSphere;
			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
			//CM_TraceBoxThroughSphere( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, p1f, midf, p1, mid );
		}
	}

    // Go past the node.
	fractionB = Clampf( fractionB, 0.f, 1.f );
    const float midf = p1f + ( p2f - p1f ) * fractionB;

	if ( midf < traceContext.realFraction ) {
		const vec3_t mid = vec3_mix( p1, p2, fractionB );

		mnode_t *childNode = node->children[side ^ 1];
		if ( !childNode->plane ) {
			leafTraceSphereHull.sphere = traceContext.traceSphere;
			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
			//CM_TraceBoxThroughSphere( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
		}
	}
}
