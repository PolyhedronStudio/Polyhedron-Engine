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
*	In-Place Hulls: Used during Leaf/Brush Tests and Leaf/Brush Traces when requiring to 
*	convert a said passed-in Leaf Node into a different Shape type Hull Leaf Node.
**/
//! For 'Box' leaf testing.
extern BoxHull leafTestBoxHull;
//! For 'Sphere' leaf testing.
extern SphereHull leafTestSphereHull;
//! For 'Ca[si;e' leaf testing.
extern CapsuleHull leafTestCapsuleHull;



/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_TestCapsuleInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
	if (!brush->numsides) {
		return;
	}

	if (traceContext.traceType != CMHullType::Sphere) {
	//	return;
	}

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	bbox3_t radiusBounds = bbox3_from_center_radius( bbox3_radius( leaf->bounds ), bbox3_center( leaf->bounds ) );
	bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
	if ( !CM_TraceIntersectBounds( traceContext, radiusBounds ) ) {
		return;
	}



	/**
	*	Ensure we are hitting this 'Leaf Sphere' before testing any further.
	**/
	// Get the sphere to trace with.
	sphere_t testSphere = leafTestCapsuleHull.sphere;

	if ( !CM_TraceIntersectSphere( traceContext, testSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
		return;
	}


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
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
//! For 'Sphere' leaf testing.
extern SphereHull leafTestSphereHull;

//
CollisionPlane CM_TranslatePlane( CollisionPlane *plane, const glm::mat4 &translateMatrix );

void CM_Test_TraceSphere_In_Brush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
	if (!brush->numsides) {
		return;
	}

	if (traceContext.traceType != CMHullType::Sphere) {
	//	return;
	}

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	// We use the leaf test sphere for testing against the brush with.
	sphere_t testSphere = traceContext.sphereTrace.transformedSphere;//leafTestSphereHull.sphere;
	sphere_t intersectSphere = testSphere;
	//if ( traceContext.isTransformedTrace ) {
	//	intersectSphere = CM_Matrix_TransformSphere( traceContext.matTransform, intersectSphere );
	//	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, intersectSphere, traceContext.isTransformedTrace );
	//}
	testSphere = intersectSphere;
	//if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, intersectSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f, true ) ) {
	bbox3_t leafIntersectBounds = leafTestBoxHull.leaf.bounds;
	if ( traceContext.isTransformedTrace ) {
		// Get translation of matrix.
		glm::vec4 glmTransformOrigin = traceContext.matTransform[3];
		vec3_t transformOrigin = glmvec4_to_phvec( glmTransformOrigin );

		// Now we got the translation, recenter the bbox.
		vec3_t leafOrigin = transformOrigin + bbox3_center( leafIntersectBounds );

		// Readjust bounds to origin.
		leafIntersectBounds = bbox3_from_center_size( bbox3_size( leafIntersectBounds ), leafOrigin );
	}
	//if ( !bbox3_intersects_sphere( leafIntersectBounds, intersectSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f, false ) ) {
	//	return;
	//}
	//if ( !CM_TraceIntersectSphere( traceContext, testSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f ) ) {
	//	return;
	//}


	/**
	*	Trace the 'Sphere' through each plane of the brush.
	**/
	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		CollisionPlane translatedPlane = *brushSide->plane;

		// Adjust distance to radius if not point tracing.
		//if ( !traceContext.isPoint ) {
		//	transformedPlane.dist += intersectSphere.offsetRadius;
		//	translatedPlane.dist += intersectSphere.offsetRadius;
		//}
		// Transform if needed.
		if ( traceContext.isTransformedTrace ) {
			// Needed for the distance to use.
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
			// Needed for normal testing.
			translatedPlane = CM_TranslatePlane( &translatedPlane, traceContext.matTransform );
		}
		// Determine the plane distance.
		//const float dist = transformedPlane.dist;
		float dist = transformedPlane.dist;
		//if ( !traceContext.isPoint ) {
			dist += intersectSphere.radius;
		//}

		// Calculate start and end point for traceline.
		vec3_t startPoint = traceContext.start;
		vec3_t endPoint = traceContext.end;

		//vec3_t startPoint = traceContext.start - intersectSphere.origin;
		//vec3_t endPoint = traceContext.end - intersectSphere.origin;

		//vec3_t startPoint = traceContext.start + intersectSphere.origin;
		//vec3_t endPoint = traceContext.end + intersectSphere.origin;
		//if ( !traceContext.isTransformedTrace ) {
		//	startPoint = traceContext.start + intersectSphere.origin;
		//	endPoint = traceContext.end + intersectSphere.origin;
		//}


		// Find the clostest point on the sphere to the plane.
		const vec3_t offsetOrigin = intersectSphere.offset;
		const float t = vec3_dot( transformedPlane.normal, offsetOrigin );

		if ( t > 0 ) {
			startPoint -= intersectSphere.offset;
			endPoint -= intersectSphere.offset;
		} else {
			startPoint += intersectSphere.offset;
			endPoint += intersectSphere.offset;
		}


		// Calculate trace line from translated plane.
		//const float d1 = vec3_dot( startPoint, translatedPlane.normal ) - dist;
		//const float d2 = vec3_dot( endPoint, translatedPlane.normal ) - dist;
		// Calculate trace line from transformed plane.
		//const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;

		//const float d1 = vec3_dot( transformedPlane.normal, startPoint ) - dist;

		// if completely in front of face, no intersection
		if ( d1 > 0.0f ) {
			return;
		}
	}

    // inside this brush
    traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
    traceContext.traceResult.fraction = 0.0f;
	traceContext.realFraction= 0.0f;
    traceContext.traceResult.contents = brush->contents;
}

/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
void CM_Test_TraceBox_In_Brush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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