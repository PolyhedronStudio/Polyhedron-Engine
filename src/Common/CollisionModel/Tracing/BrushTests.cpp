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



// TEMPORARILY, NEED TO ALLOW TRACETHROUGHLEAF to HAVE A 2 HULL VARIETY AND PREVENT TEMP VAR USAGE LIKE THIS
extern SphereHull boundsTestSphereHull;
extern CapsuleHull boundsTestCapsuleHull;


//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;
//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;
//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
extern CapsuleHull capsuleHull;
//! All round 'sphere hull' data, accessed in a few other CollisionModel files as extern.
extern SphereHull sphereHull;



/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TestCapsuleInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	//if ( !CM_TraceIntersectCapsule( traceContext, boundsTestCapsuleHull.capsule )) {
	//	return;
	//}
	// Our headnode type is capsule, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, traceContext.traceSphere, 0 ) ) {
		return;
	}

	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}
		// Calculate dist.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.offsetRadius;
		}

		
		// Find the closest point on the capsule to the plane
		//vec3_t startPoint = traceContext.start;
		//vec3_t endPoint = traceContext.end;
	

		// Find the closest point on the capsule to the plane
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint;
		//vec3_t endPoint;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			//endPoint = traceContext.end + traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			//endPoint = traceContext.end - traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - ( transformedPlane.dist - traceContext.traceSphere.radius );
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - ( transformedPlane.dist + traceContext.traceSphere.radius );

		// if completely in front of face, no intersection
		if ( d1 > 0 )
		{
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
    traceContext.traceResult.contents = brush->contents;
}
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TestSphereInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	//if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere )) {
	//	return;
	//}
	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere, CM_RAD_EPSILON ) ) {
		return;
	}

	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}
		// Calculate dist.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.radius;
		}

		
		// Find the closest point on the capsule to the plane
		//vec3_t startPoint = traceContext.start;
		//vec3_t endPoint = traceContext.end;
	

		// Find the closest point on the capsule to the plane
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint;
		//vec3_t endPoint;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			//endPoint = traceContext.end + traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			//endPoint = traceContext.end - traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - ( transformedPlane.dist - traceContext.traceSphere.radius );
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - ( transformedPlane.dist + traceContext.traceSphere.radius );

		// if completely in front of face, no intersection
		if ( d1 > 0 )
		{
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
    traceContext.traceResult.contents = brush->contents;
}

/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TestBoundingBoxInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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
			dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
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