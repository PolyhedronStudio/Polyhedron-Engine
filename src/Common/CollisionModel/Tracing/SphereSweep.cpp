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
	return sphere_intersects_bbox3( bboxSym, transformedTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
}





///**
//*	Core Plane Methods
//**/
//static const float plane_distance( const CollisionPlane &p, const vec3_t &point ) {
//  //return vec3_dot( point - planeOrigin, p.normal );
//	return vec3_dot( point, p.normal );
//}
//static const bool sphere_intersects_plane_point( const sphere_t &sphere, const CollisionPlane &plane, vec3_t &point, float &radius ) {
//	float d = plane_distance( plane, sphere.origin + sphere.offset );
//	vec3_t proj = vec3_scale( plane.normal, d );
//	point = ( sphere.origin + sphere.offset ) - proj;
//	radius = sqrtf( Maxf( sphere.radius * sphere.radius - d * d, 0 ) );
//	return fabs( d ) <= sphere.radius; 
//}
//const float sphere_project_plane_point( const sphere_t &sphere, const vec3_t &tracePoint, const CollisionPlane &plane, vec3_t &point, float &radius ) {
//	float d = plane_distance( plane, sphere.origin + sphere.offset - tracePoint );
//	vec3_t proj = vec3_scale( plane.normal, d );
//	point = ( sphere.origin + sphere.offset ) - proj;
//	radius = sqrtf( Maxf( sphere.radius * sphere.radius - d * d, 0 ) );
//	//return fabs( d ) <= sphere.radius; 
//	return d;
//}

///**
//*	
//**/
//bool sphere_inside_brush( const sphere_t &sphere, const vec3_t planeOrigin, mbrush_t *brush ) {
//	mbrushside_t *side = brush->firstbrushside; //mbrushside_t *side = brush->firstbrushside + brush->numsides - 1;
//
//	for ( int32_t i = 0; i < brush->numsides; i++, side++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, side--) {
//		// Get us the plane for this brush side.
//		CollisionPlane *plane = side->plane;
//		CollisionPlane p = *plane;
//		if ( !sphere_inside_plane( sphere, planeOrigin, p ) ) {
//			return false;
//		}
//	}
//
//	return true;
//}

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

	// If true, we already hit something nearer.
	//if (traceContext.traceResult.fraction <= p1f) {
	if (traceContext.realFraction <= p1f) {
		return;
	}

	/**
	*	Collect needed trace sphere values.
	**/
	sphere_t traceSphere = traceContext.sphereTrace.sphere;//leafTestSphereHull.sphere;
	sphere_t transformedTraceSphere = traceContext.sphereTrace.transformedSphere;
	// The offsetted sphere origin.
	const vec3_t traceSphereOrigin = transformedTraceSphere.origin + transformedTraceSphere.offset; 	//const vec3_t traceSphereOrigin = traceContext.sphereTrace.transformedSphere.origin																					//	+ traceContext.sphereTrace.transformedSphere.offset;
	// Sphere Radius.
	const float traceSphereRadius = transformedTraceSphere.radius; 	//const float traceSphereRadius = traceContext.sphereTrace.transformedSphere.radius;
	// Epsilon Offset Sphere Radius.
	const float traceSphereRadiusEpsilon = traceSphereRadius + CM_RAD_EPSILON;

	/**
	*	
	**/
	CollisionPlane transformedPlane = *plane;
	float dist = transformedPlane.dist;
	//transformedPlane.dist += traceSphereRadius;


	/**
	*	Calculate start and end point, including the sphere's offset.
	**/
	vec3_t traceStart = p1;
	vec3_t traceEnd = p2;

	// Find the clostest point on the sphere to the plane.
	const vec3_t sphereOffset = transformedTraceSphere.offset;
	const float t = vec3_dot( transformedPlane.normal, sphereOffset );
	//const vec3_t sphereOffset = transformedTraceSphere.offset;
	//const float t = vec3_dot( transformedPlane.normal, sphereOffset );
	//if ( t > 0 ) {
	//	traceStart -= transformedTraceSphere.offset;
	//	traceEnd -= transformedTraceSphere.offset;
	//} else {
	//	traceStart += transformedTraceSphere.offset;
	//	traceEnd += transformedTraceSphere.offset;
	//}


	/**
	*
	**/
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;

	t1 = plane_distance( transformedPlane, traceStart, traceSphereRadius );
	t2 = plane_distance( transformedPlane, traceEnd, traceSphereRadius );

	// Exact hit points.
	vec3_t traceStartHitPoint = vec3_zero(), traceEndHitPoint = vec3_zero();
	// Exact penetrated radius.
	float traceStartHitRadius = 0.f, traceEndHitRadius = 0.f;
	// Exact distance traveled between start to end, until hitting the plane.
	float traceStartHitDistance = 0.f, traceEndHitDistance = 0.f;

	const bool startTraceIntersected = sphere_intersects_plane_point( traceStart, traceSphereRadius, transformedPlane, traceStartHitPoint, traceStartHitRadius, traceStartHitDistance );
	const bool endTraceIntersected = sphere_intersects_plane_point( traceEnd, traceSphereRadius, transformedPlane, traceEndHitPoint, traceEndHitRadius, traceEndHitDistance );

	/**
	*
	**/
	// Plane origin for distance calculation.
	const vec3_t planeOrigin = vec3_scale( transformedPlane.normal, transformedPlane.dist /*+ traceSphereRadius*/ );

	//
	// Start Trace.
	// Calculate sphere to plane distance.
	const float startTraceDistance = vec3_dot( transformedPlane.normal, traceStart - planeOrigin );
	// Dot between plane normal and sphere 'angle'.
	const float startAngle = vec3_dot( transformedPlane.normal, traceStart );
	// Calculate start points of sphere to plane.
	const float /*t0*/extraOffsetT1 = ( ( traceSphereRadius - traceStartHitRadius ) / startAngle );

	//
	// END Trace.
	// Calculate sphere to plane distance.
	const float endTraceDistance = vec3_dot( transformedPlane.normal, traceEnd - planeOrigin );
	// Dot between plane normal and sphere 'angle'.
	const float endAngle = vec3_dot( transformedPlane.normal, traceEnd );
	// Calculate start and end points of sphere to plane.
	const float /*t1*/extraOffsetT2 = ( ( -( traceSphereRadius - traceEndHitRadius ) ) / endAngle );


	//
	// Should be fine however..
	//
	offset = ( fabs( ( traceSphereOrigin.x + transformedPlane.normal.x ) 
					+ ( traceSphereOrigin.y + transformedPlane.normal.y )
					+ ( traceSphereOrigin.z + transformedPlane.normal.z )
						
					  
	+ ( transformedPlane.dist ) ) );
	offset = offset / sqrt( vec3_dot( transformedPlane.normal, transformedPlane.normal ) );

	//const float traceStartHitRadius
	//if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
	if ( t1 >= offset + extraOffsetT1 + CM_RAD_EPSILON && t2 >= offset + extraOffsetT2 + CM_RAD_EPSILON ) {
		// Get child node.
		node = node->children[0];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// Ensure we are hitting this bounding box before testing any further.
			if ( CM_TraceIntersectBounds( traceContext, leafNode->bounds ) ) {
				// TODO: First checking for any intersection might be a good thing to do here.
				traceContext.traceShape = TraceShape::Sphere;
				CM_TraceSphere_TraceThroughLeaf( traceContext, leafNode );
			}

			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }
	//if ( t1 < -offset /*- 1.f */&& t2 < -offset /*- 1.f */) {
	if ( t1 < -offset - extraOffsetT1 - CM_RAD_EPSILON && t2 < -offset - extraOffsetT2 - CM_RAD_EPSILON ) {
		// Get child node.
		node = node->children[1];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// Ensure we are hitting this bounding box before testing any further.
			if ( CM_TraceIntersectBounds( traceContext, leafNode->bounds ) ) {
				// TODO: First checking for any intersection might be a good thing to do here.
				traceContext.traceShape = TraceShape::Sphere;
				CM_TraceSphere_TraceThroughLeaf( traceContext, leafNode );
			}

			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }

	/**
	*
	**/
	const float _extraOffset = 0;    // Dp NOT put the crosspoint DIST_EPSILON pixels on the near side
    
	int32_t side = 0;
    float idist = 0.f;
    float fractionA = 0.f;
    float fractionB = 0.f;
	
	float radialOffset = offset + traceSphereRadius + traceStartHitRadius;
    if ( t1 < t2 ) {
        idist = 1.0f / ( t1 - t2 );
        side = 1;
        
		radialOffset = offset - traceEndHitRadius ;//traceSphereRadius +traceEndHitRadius;
        fractionB = ( t1 + radialOffset + DIST_EPSILON ) * idist;
		radialOffset = offset + traceStartHitRadius; //traceSphereRadius +traceStartHitRadius;
        fractionA = ( t1 - radialOffset + DIST_EPSILON ) * idist;

		//fractionB = (t1 + offset /*+ DIST_EPSILON*/) * idist;
        //fractionA = (t1 - offset /*+ DIST_EPSILON*/) * idist;
		
        //fractionB = ( t1 + offset + _extraOffset ) * idist;
        //fractionA = ( t1 - offset + _extraOffset ) * idist;
    } else if ( t1 > t2 ) {
        idist = 1.0f / ( t1 - t2 );
        side = 0;

		radialOffset = offset - traceEndHitRadius;
		fractionB = ( t1 - radialOffset - DIST_EPSILON ) * idist;
		radialOffset = offset + traceStartHitRadius;
		fractionA = ( t1 + radialOffset + DIST_EPSILON ) * idist;

		//fractionB = (t1 - offset /*- DIST_EPSILON*/) * idist;
		//fractionA = (t1 + offset /*+ DIST_EPSILON*/) * idist;

		//fractionB = ( t1 - offset - _extraOffset ) * idist;
        //fractionA = ( t1 + offset + _extraOffset ) * idist;
    } else {
        side = 0;
        fractionA = 1.f;
        fractionB = 0.f;
    }

	/**
	*	
	**/
	// Move up to the node in case we can potentially hit it.
	if ( p1f < traceContext.realFraction ) {
		fractionA = Clampf( fractionA, 0.f, 1.f );

		float midf = p1f + ( p2f - p1f ) * fractionA;
		vec3_t mid = vec3_mix( p1, p2, fractionA );

		mnode_t *childNode = node->children[side];
		if ( !childNode->plane ) {
			// Ensure we are hitting this bounding box before testing any further.
			if ( CM_TraceIntersectBounds( traceContext, ((mleaf_t*)childNode)->bounds ) ) {
				traceContext.traceShape = TraceShape::Sphere;
				CM_TraceSphere_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
			}
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
			// Ensure we are hitting this bounding box before testing any further.
			if ( CM_TraceIntersectBounds( traceContext, ((mleaf_t*)childNode)->bounds ) ) {
				traceContext.traceShape = TraceShape::Sphere;
				CM_TraceSphere_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
			}
		} else {
			CM_RecursiveSphereTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
		}
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
