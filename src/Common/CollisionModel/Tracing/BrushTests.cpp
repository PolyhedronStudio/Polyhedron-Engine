/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Testing' in 'Leafs' related work.
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

#include "Common/CollisionModel/Tracing/BrushTests.h"



/**
*
*
*	Plane VS Sphere Methods: TODO: Move elsewhere.
*
*
**/
const float plane_distance( const CollisionPlane &p, const vec3_t &point, const float extraDistance );
const bool sphere_inside_plane( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius );
const bool sphere_outside_plane( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius );
const bool sphere_intersects_plane( const CollisionPlane &plane, const vec3_t &sphereOrigin, const float sphereRadius );
const float sphere_plane_project( const vec3_t &spherePos, const CollisionPlane & plane );
const float sphere_plane_collision_distance( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane );
const bool sphere_plane_collision( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane );
const bool sphere_intersects_plane_point( const vec3_t &sphereOrigin, const float sphereRadius, const CollisionPlane &plane, vec3_t &hitPoint, float &hitRadius, float &hitDistance );



/**
*
*
*	'TraceBox' Brush Tests:
*
*
**/
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TraceBox_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// Push the plane out appropriately for mins/maxs:
		// If completely in front of face, no intersection occured.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			dist = transformedPlane.dist - vec3_dot( traceContext.aabbTrace.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		}
		
		const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;

		if ( d1 > 0.0f ) {// 0 ) {
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
    traceContext.traceResult.contents = brush->contents;
}



/**
*
*
*	'TraceSphere' Brush Tests:
*
*
**/
/**
*   @brief Test whether the sphere(radius) when located at p1 is inside of the brush, or not.
**/
void CM_TraceSphere_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}

	//bbox3_t transformedLeafTestBounds = leaf->bounds;
	//// Transform bounds if we're dealing with a transformed trace.
	//if ( traceContext.isTransformedTrace ) {
	//	transformedLeafTestBounds = CM_Matrix_TransformBounds( traceContext.matTransform, transformedLeafTestBounds );
	//}

	//if  (!bbox3_intersects( traceContext.absoluteBounds, transformedLeafTestBounds ) ) {
	//	return;
	//}


	/**
	*	See if the sphere touches our absolute bounds at all.
	**/
	//sphere_t traceSphere = traceContext.sphereTrace.sphere;//leafTestSphereHull.sphere;
	//sphere_t transformedTraceSphere = traceSphere;
	//if ( traceContext.isTransformedTrace ) {
	//	transformedTraceSphere = CM_Matrix_TransformSphere( traceContext.matTransform, transformedTraceSphere );
	//	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTraceSphere, traceContext.isTransformedTrace );
	//}
	sphere_t traceSphere = traceContext.sphereTrace.sphere;//leafTestSphereHull.sphere;
	sphere_t transformedTraceSphere = traceContext.sphereTrace.transformedSphere;

	// Use the already pre-transformed Sphere to save on performance penalties.
	//sphere_t transformedTraceSphere = traceContext.sphereTrace.transformedSphere;//leafTraceSphereHull.sphere;
	//bbox3_t leafBounds = leaf->bounds;
	//if ( traceContext.isTransformedTrace ) {
	//	leafBounds = CM_Matrix_TransformBounds( traceContext.matTransform, leafBounds );
	//}
	//if ( !sphere_intersects_bbox3( leafBounds, transformedTraceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON, true ) ) {
	////	return;
	//}
	//if ( !sphere_intersects_bbox3( transformedLeafTestBounds, transformedTraceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON, true ) ) {
	//	return;
	//}

	/**
	*	Collect needed trace sphere values.
	**/
	// The offsetted sphere origin.
	const vec3_t traceSphereOrigin = transformedTraceSphere.origin + transformedTraceSphere.offset; 	//const vec3_t traceSphereOrigin = traceContext.sphereTrace.transformedSphere.origin
																						//	+ traceContext.sphereTrace.transformedSphere.offset;
	// Sphere Radius.
	const float traceSphereRadius = transformedTraceSphere.radius; 	//const float traceSphereRadius = traceContext.sphereTrace.transformedSphere.radius;
	
	// Sphere Radius.
	const float traceSphereRadiusEpsilon = transformedTraceSphere.radius + CM_RAD_EPSILON; 	//const float traceSphereRadius = traceContext.sphereTrace.transformedSphere.radius;

	/**
	*	Trace 'Sphere' through the brush' planes. Push the plane out appropriately for radius(mins/maxs)
	*	If completely in front of face, no intersection occured.
	**/
	// Stores an actual copy of the current brush' clipping plane that our trace wound up hitting.
    CollisionPlane clipPlane;
	// The leading brush side: the dominating brush side that was hit by our trace.
    mbrushside_t *leadSide = nullptr;
	// Current brush side being iterated.
	mbrushside_t *brushSide = brush->firstbrushside; //mbrushside_t *brushSide = brush->firstbrushside + brush->numsides - 1;
	// Iterate over all brush sides.
	for ( int32_t i = 0; i < brush->numsides; i++, brushSide++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, brushSide--) {
		/**
		*	Calculate (transformed-)plane, and the plane + sphere distance.
		**/
		// Get us the plane for this brush side.
		CollisionPlane *plane = brushSide->plane;
		// Create a copy of the plane.
		CollisionPlane transformedPlane = *plane;
		// Add our sphere radius to the plane distance.
		//transformedPlane.dist += traceSphereRadius;
		// Transform if need be.
		//transformedPlane.dist += transformedTraceSphere.radius;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// Distance to work with.
		//const float dist = transformedPlane.dist + transformedTraceSphere.radius;
		//transformedPlane.dist += transformedTraceSphere.radius;

		/**
		*	Calculate start and end point.
		**/
		// Start point of this trace.
		vec3_t traceStart = traceContext.start;
		// End point of this trace.
		vec3_t traceEnd = traceContext.end;

		//// Find the clostest point on the sphere to the plane.
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
		*	Project the sphere trace's start and end points properly.
		**/
		float d1 = plane_distance( transformedPlane, traceStart, traceSphereRadiusEpsilon );
		float d2 = plane_distance( transformedPlane, traceEnd, traceSphereRadiusEpsilon );

		/**
		*	Determine if, where and how much distance the sphere had IF intersecting with a plane.
		**/
		// Exact hit points.
		vec3_t traceStartHitPoint = vec3_zero(), traceEndHitPoint = vec3_zero();
		// Exact penetrated radius.
		float startHitPenetrationRadius = 0.f, endHitPenetrationRadius = 0.f;
		// Exact distance traveled between start to end, until hitting the plane.
		float startHitPenetrationDistance = 0.f, endHitPenetrationDistance = 0.f;
		// Calculate whether sphere vs plane intersectioned, and if so, at what specific point did it intersect at?
		const bool D1TraceIntersected = sphere_intersects_plane_point( traceStart, traceSphereRadius, transformedPlane, traceStartHitPoint, startHitPenetrationRadius, startHitPenetrationDistance );
		const bool D2TraceIntersected = sphere_intersects_plane_point( traceEnd, traceSphereRadius, transformedPlane, traceEndHitPoint, endHitPenetrationRadius, endHitPenetrationDistance );

		/**
		*	Calculate sphere to plane distances for end and start trace.
		**/
		// Plane origin for distance calculation.
		const vec3_t planeOrigin = vec3_scale( transformedPlane.normal, transformedPlane.dist );
		
		// Calculate sphere to plane distance (Start Trace).
		const float startTraceDistance = vec3_dot( transformedPlane.normal, traceStart - planeOrigin );
		// Dot between plane normal and sphere 'angle'.
		const float startAngle = vec3_dot( transformedPlane.normal, traceStart - traceStartHitPoint );
		// Calculate extra offset for the 'Start Trace' based on the spheroid's actual angle.
		const float /*t0*/extraOffsetT1 = ( ( traceSphereRadius - startHitPenetrationRadius ) / startAngle );
		//const float /*t0*/extraOffsetT1 = ( ( traceSphereRadius - traceStartHitRadius ) / startAngle );
		
		// Calculate sphere to plane distance (End Trace).
		const float endTraceDistance = vec3_dot( transformedPlane.normal, traceEnd - planeOrigin );
		// Dot between plane normal and sphere 'angle'.
		const float endAngle = vec3_dot( transformedPlane.normal, traceEnd - traceEndHitPoint );
		// Calculate extra offset for the 'End Trace' based on the spheroid's actual angle.
		const float /*t1*/extraOffsetT2 = ( ( ( traceSphereRadius - endHitPenetrationRadius ) ) / endAngle );

		// Add OR subtract the offsets depending on the sphere's 'origin + offset' closest to the plane.
		d1 += extraOffsetT1;
		d2 += extraOffsetT2;
		//const float t = vec3_dot( transformedPlane.normal, traceSphereOrigin );
		//if ( t > 0 ) {
		//	d1 += extraOffsetT1;
		//	d2 += extraOffsetT2;
		//} else {
		//	d1 -= extraOffsetT1;
		//	d2 -= extraOffsetT2;
		//}

		/**
		*	Determine whether the trace started and/or ended, inside of 'Solid' or 'Non Solid' brushwork.
		*	- Inside:		If the distance is negative(-) and greater(>) than the Radius. 
		*	- Outside:		If the distance is positive(+) and greater(>) than the Radius. 
		*	- Intersection: If the absolute distance(fabs( dist )) is less than or 
		*					equal(<=) to the Radius.
		**/
		// Absolute distances.
		float absD1 = fabs( d1 );
		float absD2 = fabs( d2 );

		// Radius' to test against.
		const float testRadius = traceSphereRadiusEpsilon;

		// Determine whether the 'Start Point' resides in a 'Non Solid', empty space type.
		//if ( !(-d1 > testRadius) && !(absD1 <= testRadius) )   {
		//	return;
		//}
		//if ( ( d1 > 0 && absD1 <= testRadius ) /*&& ( absD2 < testRadius + DIST_EPSILON || d2 <= d1 )*/ ) {
		//if ( (d1 < 0 && d1 < -testRadius) && !(absD1 <= testRadius)) {
		if ( !(d1 <= 0 && d1 < -testRadius) && !(d1 > 0 && d1 > testRadius) ) {//!(absD1 <= testRadius)) {
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
	traceContext.realFraction = 0.0f;
    traceContext.traceResult.contents = brush->contents;
}



/**
*
*
*	'TraceCapsule' Brush Tests:
*
*
**/
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TraceCapsule_TestInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
	if (!brush->numsides) {
		return;
	}
	///////////////////
	return;
	//if (traceContext.traceType != CMHullType::Sphere) {
	//	return;
	//}

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	bbox3_t radiusBounds = bbox3_from_center_radius( bbox3_radius( leaf->bounds ), bbox3_center( leaf->bounds ) );
	//bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
	//if ( !CM_TraceIntersectBounds( traceContext, radiusBounds ) ) {
	//	return;
	//}



	/**
	*	Ensure we are hitting this 'Leaf Sphere' before testing any further.
	**/
	// Get the sphere to trace with.
	sphere_t testSphere =traceContext.capsuleTrace.sphere;

	//if ( !CM_TraceIntersectSphere( traceContext, testSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON ) ) {
	//	return;
	//}


	/**
	*	Trace the 'Capsule Sphere' through each plane of the brush.
	**/
	// Sphere offset origin 
	const vec3_t sphereOffsetOrigin = testSphere.origin + testSphere.offset;


	/**
	*	Trace the 'Sphere' through each plane of the brush.
	**/
	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// Determine the plane distance.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + testSphere.offsetRadius;
		}

		// Find the closest point on the capsule to the plane
		const float t = vec3_dot( transformedPlane.normal, sphereOffsetOrigin );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint = traceContext.start;
		vec3_t endPoint = traceContext.end;
		if ( t > 0.0f ) {
			startPoint = traceContext.start - sphereOffsetOrigin;
		} else {
			startPoint = traceContext.start + sphereOffsetOrigin;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - dist;

		// if completely in front of face, no intersection
		if ( d1 > 0.0f )
		{
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
    traceContext.traceResult.contents = brush->contents;
}