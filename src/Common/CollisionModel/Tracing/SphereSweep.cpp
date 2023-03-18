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
	sphere_t transformedTestSphere = traceContext.sphereTrace.sphere;

	//// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matInvTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
	}
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTestSphere, traceContext.isTransformedTrace );

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


//
CollisionPlane CM_TranslatePlane( CollisionPlane *plane, const glm::mat4 &translateMatrix );
const float sphere_plane_project( const vec3_t &spherePos, const CollisionPlane & p){
        return fabs( spherePos.x*p.normal.x + spherePos.y*p.normal.y + spherePos.z*p.normal.z + p.dist );
}

const bool sphere_plane_collision( const sphere_t & s, const CollisionPlane & p){
        return sphere_plane_project(s.origin + s.offset, p)/ sqrtf( p.normal.x * p.normal.x + p.normal.y * p.normal.y + p.normal.z * p.normal.z ) < s.radius;
}
//CollisionPlane p = transformedPlane;
//if ( sphere_plane_project( startPoint, p)/ sqrtf( p.normal.x * p.normal.x + p.normal.y * p.normal.y + p.normal.z * p.normal.z ) < testSphere.radius ) 
//{
//	return;
//}


/**
*   @brief	Performs a recursive 'Trace Bounds Sweep' through the 'BSP World Tree Node', when successfully landing in a leaf node, perform its corresponding node trace.
**/
void CM_RecursiveSphereTraceThroughTree( TraceContext &traceContext, mnode_t *node, const float p1f, const float p2f, const vec3_t &p1, const vec3_t &p2 ) {
    if ( !node ) {
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
	if ( traceContext.realFraction <= p1f ) {
	//if (traceContext.traceResult.fraction <= p1f) {
		return;
	}
//#endif
	
	// If the node has no plane, we are inside a Leaf Node.
	if ( !plane ) {
		mleaf_t *leafNode = (mleaf_t *)node;

		// Ensure it matches the content we trace for.
		if ( leafNode->contents & traceContext.contentsMask ) {
			// Test for intersection.
			//const bool traceIntersected = CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );

			// Seemed to work?.
			//bbox3_t bboxSym = bbox3_from_center_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );
			//const bool traceIntersected = bbox3_intersects_sphere( bboxSym, traceContext.traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
			//const bool traceIntersected = CM_TraceIntersectBounds( traceContext, leafNode->bounds );
			
			//const bool traceIntersected = CM_SphereSweep_SphereTraceIntersectLeaf( traceContext, leafNode );
			//if ( traceIntersected ) {
			//if ( CM_TraceIntersectBounds( traceContext, leafNode->bounds ) ) {
		
			// Test trace intersection.
			bbox3_t bboxSym = bbox3_from_center_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) );
			if ( bbox3_intersects_sphere( bboxSym, traceContext.sphereTrace.transformedSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 1.f, false ) ) {
				// Test trace intersection.
				//sphere_t nodeLeafSphere = sphere_from_size( bbox3_symmetrical( leafNode->bounds ), bbox3_center( leafNode->bounds ) ); //bbox3_center( epsilonSphereBounds ) );
				//sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, nodeLeafSphere, traceContext.isTransformedTrace );
				//if ( CM_TraceIntersectSphere( traceContext, nodeLeafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 1.f ) ) {

				//leafTraceSphereHull.sphere = traceContext.transformedTraceSphere;
				//CM_Trace_TraceBox_Through_Sphere_LeafShape( traceContext, leafNode );

				traceContext.traceShape = CMHullType::Sphere;
				CM_Trace_TraceSphere_ThroughLeaf( traceContext, leafNode );
			}
		}

		return;
	}

	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
	//CollisionPlane transformedPlane = *plane;
	// Make sure its normal is normalized properly for sphere testing.
	//transformedPlane.normal = vec3_normalize( transformedPlane.normal );

	//
	//	Get our sphere data.
	//
	sphere_t traceSphere = traceContext.sphereTrace.transformedSphere;

    //
    // Find the point distances to the seperating plane and the offset for the size of the box.
    //

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

	////
	//// Current:
	////
	//// Axial planes.
	////if ( transformedPlane.type < 3 ) {
	////	t1 = p1[ transformedPlane.type ] - dist;
	////	t2 = p2[ transformedPlane.type ] - dist;
	////	offset = traceContext.extents[ transformedPlane.type ];
	////// Non axial planes use dot product testing.
	////} else {
	//	t1 = vec3_dot( transformedPlane.normal, p1 ) - dist;
	//	t2 = vec3_dot( transformedPlane.normal, p2 ) - dist;
	//	if ( traceContext.isPoint ) {
	//		offset = 0;
	//	} else {
	//		offset = (fabs( traceContext.extents.x * transformedPlane.normal.x ) +
	//					fabs( traceContext.extents.y * transformedPlane.normal.y ) +
	//					fabs( traceContext.extents.z * transformedPlane.normal.z )) * 3;
	//	}
	////}

		// Copy the plane and transform if need be. (We do not want to transform the source data.)
		CollisionPlane transformedPlane = *plane;
		CollisionPlane translatedPlane = *plane;

		if ( !traceContext.isPoint ) {
			transformedPlane.dist += traceSphere.radius;
			translatedPlane.dist += traceSphere.radius;
		}
		if ( traceContext.isTransformedTrace ) {
			// Needed for the distance to use.
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
			// Needed for normal testing.
			translatedPlane = CM_TranslatePlane( &translatedPlane, traceContext.matTransform );
		}

		// Determine the plane distance.
		const float dist = plane->dist + traceSphere.radius;//transformedPlane.dist;

		// Determine the offset to the plane.
		const float offsetOriginX = traceSphere.origin.x + traceSphere.offset.x;
		const float offsetOriginY = traceSphere.origin.y + traceSphere.offset.y;
		const float offsetOriginZ = traceSphere.origin.z + traceSphere.offset.z;
		const float offset = ( traceContext.isPoint ? 0.f :
			//fabs(
			//	offsetOriginX * translatedPlane.normal.x +
			//	offsetOriginY * translatedPlane.normal.y +
			//	offsetOriginZ * translatedPlane.normal.z + ( translatedPlane.dist )
			//)
			sqrtf( fabs(
				offsetOriginX * plane->normal.x +
				offsetOriginY * plane->normal.y +
				offsetOriginZ * plane->normal.z + plane->dist + traceSphere.radius 
			) ) * 2.f
		);

		// Calculate start and end point for traceline.
		vec3_t startPoint = traceContext.start;
		vec3_t endPoint = traceContext.end;
		//vec3_t startPoint = ( traceContext.isTransformedTrace ? traceContext.start + traceSphere.origin : traceContext.start );
		//vec3_t endPoint = ( traceContext.isTransformedTrace ? traceContext.end + traceSphere.origin : traceContext.end );

		startPoint -= traceSphere.offset;
		endPoint -= traceSphere.offset;

		// Calculate trace line.
		//const float t1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		//const float t2 = vec3_dot( endPoint, transformedPlane.normal ) - dist;
		const float t1 = vec3_dot( transformedPlane.normal, startPoint ) - dist;
		const float t2 = vec3_dot( transformedPlane.normal, endPoint ) - dist;


		// See which sides we need to consider.
		//if ( t1 >= offset && t2 >= offset ) { 
		if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
			// Get child node.
			node = node->children[0];
			// Go back and recheck to see if we hit a leaf node, or else traverse even further down the BSP tree.
			goto recheck;
		}
		//if ( t1 < -offset && t2 < -offset ) { 
		if ( t1 < -offset - 1.f && t2 < -offset - 1.f ) {
			// Get child node.
			node = node->children[1];
			// Go back and recheck to see if we hit a leaf node, or else traverse even further down the BSP tree.
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
			fractionB = (t1 + offset + DIST_EPSILON ) * idist;
			fractionA = (t1 - offset + DIST_EPSILON ) * idist;
			//fractionB = (t1 + offset /* +  DIST_EPSILON */ ) * idist;
			//fractionA = (t1 - offset /* +  DIST_EPSILON */ ) * idist;
		} else if (t1 > t2) {
			idist = 1.0f / (t1 - t2);
			side = 0;
			fractionB = (t1 - offset - DIST_EPSILON) * idist;
			fractionA = (t1 + offset + DIST_EPSILON ) * idist;
			//fractionB = (t1 - offset /* - DIST_EPSILON*/) * idist;
			//fractionA = (t1 + offset /* + DIST_EPSILON*/) * idist;
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
			node = childNode;
			goto recheck;
			//leafTraceSphereHull.sphere = traceContext.traceSphere;
			//CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
			//CM_Trace_TraceBox_Through_Sphere_LeafShape( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, p1f, midf, p1, mid );
		}
		//node = node->children[ side ];
		//// Go back and recheck to see if we hit a leaf node, or else traverse even further down the BSP tree.
		//goto recheck;
	}

    // Go past the node.
	fractionB = Clampf( fractionB, 0.f, 1.f );
    const float midf = p1f + ( p2f - p1f ) * fractionB;

	if ( midf < traceContext.realFraction ) {
		const vec3_t mid = vec3_mix( p1, p2, fractionB );

		mnode_t *childNode = node->children[side ^ 1];
		if ( !childNode->plane ) {
			node = childNode;
			goto recheck;
		} else {
			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
		}

		//node = node->children[ side ^ 1 ];
		//// Go back and recheck to see if we hit a leaf node, or else traverse even further down the BSP tree.
		//goto recheck;
	}
}
















/**
*   @brief	Performs a recursive 'Trace Bounds Sweep' through the 'BSP World Tree Node', when successfully landing in a leaf node, perform its corresponding node trace.
**/
//void CM_RecursiveSphereTraceThroughTree( TraceContext &traceContext, mnode_t *node, const float p1f, const float p2f, const vec3_t &p1, const vec3_t &p2 ) {
//    if (!node) {
//		return;
//	}
//recheck:
//	// If plane is NULL, we are in a leaf node
//    CollisionPlane *plane = node->plane;
//
////#ifdef TRACE_PLANE_SIDE_FIX
////	// If true, we already hit something nearer.
////    if (traceContext.realFraction <= p1f) {
////        return;
////    }
////#else
////	// If true, we already hit something nearer.
//	if (traceContext.traceResult.fraction <= p1f) {
//		return;
//	}
////#endif
//
//	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
//	CollisionPlane transformedPlane = *plane;
//	//if ( traceContext.isTransformedTrace ) { TransformPlane }
//
//    //
//    // Find the point distances to the seperating plane and the offset for the size of the box.
//    //
//    float offset = 0.f;
//    float t1 = 0.f;
//    float t2 = 0.f;
//
//	// Axial planes.
//    if ( transformedPlane.type < 3 ) {
//        t1 = p1[ transformedPlane.type ] - transformedPlane.dist;
//        t2 = p2[ transformedPlane.type ] - transformedPlane.dist;
//        offset = traceContext.extents[ transformedPlane.type ];
//	// Non axial planes use dot product testing.
//    } else {
//        t1 = vec3_dot( transformedPlane.normal, p1 ) - transformedPlane.dist;
//        t2 = vec3_dot( transformedPlane.normal, p2 ) - transformedPlane.dist;
//        if ( traceContext.isPoint ) {
//			offset = 0;
//        } else {
//			offset = (fabs( traceContext.extents.x * transformedPlane.normal.x ) +
//                     fabs( traceContext.extents.y * transformedPlane.normal.y ) +
//                     fabs( traceContext.extents.z * transformedPlane.normal.z )) * 3;
//		}
//    }
//
//    // See which sides we need to consider.
//    if ( t1 >= offset && t2 >= offset ) { //if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
//		// Get child node.
//		node = node->children[0];
//		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
//		if ( !node->plane ) {
//			mleaf_t *leafNode = (mleaf_t *)node;
//			// TODO: First checking for any intersection might be a good thing to do here.
//			CM_TraceThroughLeaf( traceContext, leafNode );
//			return;
//		}
//		// Perform another check traversing even further down the BSP tree.
//		goto recheck;
//    }
//	if ( t1 < -offset && t2 < -offset ) { //if ( t1 < -offset /*- 1.f */&& t2 < -offset /*- 1.f */) {
//		// Get child node.
//		node = node->children[1];
//		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
//		if ( !node->plane ) {
//			mleaf_t *leafNode = (mleaf_t *)node;
//			// TODO: First checking for any intersection might be a good thing to do here.
//			CM_TraceThroughLeaf( traceContext, leafNode );
//			return;
//		}
//		// Perform another check traversing even further down the BSP tree.
//		goto recheck;
//    }
//
//    // Dp NOT put the crosspoint DIST_EPSILON pixels on the near side
//    int32_t side = 0;
//    float idist = 0.f;
//    float fractionA = 0.f;
//    float fractionB = 0.f;
//    if (t1 < t2) {
//        idist = 1.0f / (t1 - t2);
//        side = 1;
//        fractionB = (t1 + offset /*+ DIST_EPSILON*/) * idist;
//        fractionA = (t1 - offset /*+ DIST_EPSILON*/) * idist;
//    } else if (t1 > t2) {
//        idist = 1.0f / (t1 - t2);
//        side = 0;
//		fractionB = (t1 - offset /*- DIST_EPSILON*/) * idist;
//        fractionA = (t1 + offset /*+ DIST_EPSILON*/) * idist;
//    } else {
//        side = 0;
//        fractionA = 1.f;
//        fractionB = 0.f;
//    }
//
//	// Move up to the node in case we can potentially hit it.
//	if ( p1f < traceContext.realFraction ) {
//		fractionA = Clampf( fractionA, 0.f, 1.f );
//
//		float midf = p1f + ( p2f - p1f ) * fractionA;
//		vec3_t mid = vec3_mix( p1, p2, fractionA );
//
//		mnode_t *childNode = node->children[side];
//		if ( !childNode->plane ) {
//			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
//		} else {
//			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, p1f, midf, p1, mid );
//		}
//	}
//
//    // Go past the node.
//	fractionB = Clampf( fractionB, 0.f, 1.f );
//    const float midf = p1f + ( p2f - p1f ) * fractionB;
//
//	if ( midf < traceContext.realFraction ) {
//		const vec3_t mid = vec3_mix( p1, p2, fractionB );
//
//		mnode_t *childNode = node->children[side ^ 1];
//		if ( !childNode->plane ) {
//			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
//		} else {
//			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
//		}
//	}
//}
