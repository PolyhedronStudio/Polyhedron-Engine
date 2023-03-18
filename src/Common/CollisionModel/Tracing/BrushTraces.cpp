/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Tracing' through 'LeafBrushes' related work.
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
#include "Common/CollisionModel/Tracing/BrushTraces.h"



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
*   @brief Performs a 'Capsule Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceCapsuleThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
	if (!brush->numsides) {
		return;
	}

	if (traceContext.traceType != CMHullType::Capsule) {
	//	return;
	}


	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	if ( !CM_TraceIntersectBounds( traceContext, leafTraceBoxHull.leaf.bounds ) ) {
		return;
	}


	/**
	*	Ensure we are hitting this 'Leaf Sphere' before testing any further.
	**/
	// Get the sphere to trace with.
	sphere_t traceSphere = leafTraceCapsuleHull.sphere;

	if ( !CM_TraceIntersectSphere( traceContext, traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
		return;
	}


	/**
	*	Trace the 'Capsule Sphere' through each plane of the brush.
	**/
	// Sphere offset origin 
	const vec3_t sphereOffsetOrigin = traceSphere.origin + traceSphere.offset;

	// Keep score of if we started inside, and/or the trace exited the brush.
    bool getOut = false;
    bool startOut = false;

	// Fraction.
    float fraction = 0.f;

	// Enter and leave fractions.
    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

	// The distance traversed before entering brush.
	float enterDistance = 0.f;
	// Total move made.
	float move = 1.f;

	// The actual final clipping plane.
    CollisionPlane clipPlane;

	// Brush side.
    mbrushside_t *leadSide = nullptr;
	mbrushside_t *side = brush->firstbrushside; //mbrushside_t *side = brush->firstbrushside + brush->numsides - 1;

	for ( int32_t i = 0; i < brush->numsides; i++, side++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, side--) {

		// Get us the plane for this brush side.
		CollisionPlane *plane = side->plane;

		// Copy the plane and transform if need be. (We do not want to transform the source data.)
		CollisionPlane transformedPlane = *plane;


		// NEW METHOD:
		// Adjust distance to radius if not point tracing.
		if ( !traceContext.isPoint ) {
			transformedPlane.dist += traceSphere.radius;
		}
		// Transform if needed.
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}
		// Determine the plane distance.
		const float dist = transformedPlane.dist;


		// Find the closest point on the capsule to the plane
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		const float t = vec3_dot( transformedPlane.normal, sphereOffsetOrigin );

		//vec3_t startPoint = ( traceContext.start - traceSphere.origin ) + traceSphere.offset;
		//vec3_t endPoint = ( traceContext.end - traceSphere.origin ) + traceSphere.offset;	
		vec3_t startPoint = traceContext.start;
		vec3_t endPoint = traceContext.end;
		if ( t > 0 ) {
			startPoint = traceContext.start - sphereOffsetOrigin;
			endPoint = traceContext.end - sphereOffsetOrigin;
		} else {
			startPoint = traceContext.start + sphereOffsetOrigin;
			endPoint = traceContext.end + sphereOffsetOrigin;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( transformedPlane.normal, startPoint ) - dist;
		const float d2 = vec3_dot( transformedPlane.normal, endPoint ) - dist;

		// Exited the brush.	
		if ( d2 > 0 ) {
			getOut = true; // endpoint is not in solid
		}
		// Started outside.
		if ( d1 > 0 ) {
			startOut = true;
		}

		// If completely in front of face, no intersection occured.
if (d1 > 0 && (d2 >= DIST_EPSILON || d2 >= d1)) {
	return;
}

if (d1 <= 0 && d2 <= 0) {
	continue;
}

// Calculate the fraction, enter distance, and the total move made.
float f = d1 - d2;

//if ( f > 0 ) {
if (d1 > d2) {
	f = d1 / f;

	//f = ( d1 - DIST_EPSILON ) / f;
	if (f < 0) {
		f = 0;
	}

	if (f > enterFractionA) {
		enterDistance = d1;
		move = d1 - d2;
		enterFractionA = f;
		clipPlane = transformedPlane;
		leadSide = side;
	}
} else {//if ( f < 0 ) {
//} else {
	//f = d1 / f;
	//if ( f < leaveFraction ) {
	//	leaveFraction = f;
	//}
	f = d1 / f; //( d1 + DIST_EPSILON ) / f;
	if (f > 1) {
		f = 1;
	}
	if (f < leaveFraction) {
		leaveFraction = f;
	}
}
	}


	// We started inside of the brush:
	if (startOut == false) {
		// Original point was inside brush.
		traceContext.traceResult.startSolid = true;
		// Set the brush' contents.
		traceContext.traceResult.contents = brush->contents;
		// See if we got out:
		if (getOut == false) {
			// We didn't get out, set fractions to 0.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.fraction = 0.f;

			// All solid.
			traceContext.traceResult.allSolid = true;
		}
		// Exit.
		return;
	}

	// Check if this reduces collision time range.
	//if (enterFractionA <= -1) { return; }
	//if (enterFractionA > leaveFraction) { return; }

	if (enterFractionA - FRAC_EPSILON <= leaveFraction) {
		if (enterFractionA > -1 && enterFractionA < traceContext.realFraction) {
			if (enterFractionA < 0) {
				enterFractionA = 0;
			}

			traceContext.realFraction = enterFractionA;
			traceContext.traceResult.plane = clipPlane;
			traceContext.traceResult.surface = &(leadSide->texinfo->c);
			traceContext.traceResult.contents = brush->contents;
			traceContext.traceResult.fraction = (enterDistance - DIST_EPSILON) / move;
			if (traceContext.traceResult.fraction < 0) {
				traceContext.traceResult.fraction = 0;
			}
		}
	} // if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
}

/**
*   @brief Performs a 'Sphere Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
//
CollisionPlane CM_TranslatePlane( CollisionPlane *plane, const glm::mat4 &translateMatrix );

void CM_Trace_TraceSphere_Through_Brush(TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf) {
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
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}


	/**
	*	Ensure we are hitting this 'Leaf Sphere' before testing any further.
	**/
	//// We use the transformed trace sphere(if non transformed, equals regular tracesphere) 
	//// for tracing against the brush with.
	//sphere_t traceSphere = traceContext.transformedTraceSphere; // leafTraceSphereHull.sphere;
	//if ( !CM_TraceIntersectSphere( traceContext, traceContext.traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
	//	return;
	//}

	// We use the leaf trace sphere for testing against the brush with.
	sphere_t traceSphere = traceContext.sphereTrace.transformedSphere;//leafTraceSphereHull.sphere;
	sphere_t intersectSphere = traceSphere;
	//if ( traceContext.isTransformedTrace ) {
	//	intersectSphere = CM_Matrix_TransformSphere( traceContext.matTransform, intersectSphere );
	//	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, intersectSphere, traceContext.isTransformedTrace );
	//}
	traceSphere = intersectSphere;
	//if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, intersectSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON, true ) ) {

	//if ( !bbox3_intersects_sphere( leafIntersectBounds, intersectSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f, false ) ) {
	//	return;
	//}
	//if ( !CM_TraceIntersectSphere( traceContext, traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
	//	return;
	//}

	/**
	*	Trace the 'Sphere' through each plane of the brush.
	**/
	//traceSphere = traceContext.traceSphere;
	// Sphere offset origin 

	// Keep score of if we started inside, and/or the trace exited the brush.
    bool getOut = false;
    bool startOut = false;

	// Fraction.
    float fraction = 0.f;

	// Enter and leave fractions.
    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

	// The distance traversed before entering brush.
	float enterDistance = 0.f;
	// Total move made.
	float move = 1.f;

	// The actual final clipping plane.
    CollisionPlane clipPlane;

	// Brush side.
    mbrushside_t *leadSide = nullptr;
	mbrushside_t *side = brush->firstbrushside; //mbrushside_t *side = brush->firstbrushside + brush->numsides - 1;

	for ( int32_t i = 0; i < brush->numsides; i++, side++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, side--) {

		// Get us the plane for this brush side.
		CollisionPlane *plane = side->plane;

		// Copy the plane and transform if need be. (We do not want to transform the source data.)
		CollisionPlane transformedPlane = *plane;
		CollisionPlane translatedPlane = *plane;

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


		// Calculate trace lines from translated plane.
		//const float d1 = vec3_dot( startPoint, translatedPlane.normal ) - dist;
		//const float d2 = vec3_dot( endPoint, translatedPlane.normal ) - dist;
		const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - dist;

		// Exited the brush.	
		if ( d2 > 0 ) {
			getOut = true; // endpoint is not in solid
		}
		// Started outside.
		if ( d1 > 0 ) {
			startOut = true;
		}

		// If completely in front of face, no intersection occured.
		if ( d1 > 0 && ( /*d2 >= DIST_EPSILON ||*/ d2 >= d1 ) ) {
			return;
		}

		if ( d1 <= 0 && d2 <= 0 ) {
			continue;
		}

		// Calculate the fraction, enter distance, and the total move made.
		//float f = d1 - d2;

		float f = d1 - d2;
		if ( f > 0 ) {
			f = d1 / f;

			if ( f > enterFractionA ) {
				enterDistance = d1;
				move = d1 - d2;
				enterFractionA = f;
				clipPlane = transformedPlane;
				leadSide = side;
			}
		} else if ( f < 0 ) {
			f = d1 / f;
			if ( f < leaveFraction ) {
				leaveFraction = f;
			}
		}
	}


	// We started inside of the brush:
    if ( startOut == false ) {
        // Original point was inside brush.
        traceContext.traceResult.startSolid = true;
        // Set the brush' contents.
        traceContext.traceResult.contents = brush->contents;
		// See if we got out:
        if ( getOut == false ) {
			// We didn't get out, set fractions to 0.
            traceContext.realFraction = 0.f;
            traceContext.traceResult.fraction = 0.f;
        
			// All solid.
			traceContext.traceResult.allSolid = true;
        }
		// Exit.
		return;
    }

	// Check if this reduces collision time range.
    //if (enterFractionA <= -1) { return; }
    //if (enterFractionA > leaveFraction) { return; }

	if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
		if ( enterFractionA > -1 && enterFractionA < traceContext.realFraction ) {
			if ( enterFractionA < 0 ) {
				enterFractionA = 0;
			}

			traceContext.realFraction = enterFractionA;
			traceContext.traceResult.plane = clipPlane;
			traceContext.traceResult.surface = &(leadSide->texinfo->c);
			traceContext.traceResult.contents = brush->contents;
			traceContext.traceResult.fraction = ( enterDistance - DIST_EPSILON ) / move;
			if ( traceContext.traceResult.fraction < 0 ) {
				traceContext.traceResult.fraction = 0;
			}
		}
	} // if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
}

/**
*   @brief Performs a 'BoundingBox Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_Trace_TraceBox_Through_Brush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }

	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	////	return;
	//}

    bool getOut = false;
    bool startOut = false;

    float fraction = 0.f;

    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

	float enterDistance = 0.f;
	float move = 1.f;

    CollisionPlane clipPlane;
    mbrushside_t *leadSide = nullptr;
	mbrushside_t *side = brush->firstbrushside; //mbrushside_t *side = brush->firstbrushside + brush->numsides - 1;

	for ( int32_t i = 0; i < brush->numsides; i++, side++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, side--) {
		// Get us the plane for this brush side.
		CollisionPlane *plane = side->plane;

		// Copy the plane and transform if need be. (We do not want to transform the source data.)
		CollisionPlane transformedPlane = *plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// Determine the actual distance to, skip dot product in case of 'point tracing' and just use plane distance.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			dist = transformedPlane.dist - vec3_dot( traceContext.aabbTrace.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		}
		// Calculate trace line.
		const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;
		const float d2 = vec3_dot( traceContext.end, transformedPlane.normal ) - dist;

		// Exited the brush.
		if ( d2 > 0 ) {
			getOut = true; // End Point is not in solid.
		}
		// Started outside.
		if ( d1 > 0 ) {
			startOut = true;
		}

		// If completely in front of face, no intersection occured.
		//if ( d1 > 0 && ( d2 >= DIST_EPSILON || d2 >= d1 ) ) {
		if ( d1 > 0 && d2 >= d1 ) {
			return;
		}

		if ( d1 <= 0 && d2 <= 0 ) {
			continue;
		}

		// Calculate the fraction, enter distance, and the total move made.
		float f = d1 - d2;
		if ( f > 0 ) {
			f = d1 / f;

			if ( f > enterFractionA ) {
				enterDistance = d1;
				move = d1 - d2;
				enterFractionA = f;
				clipPlane = transformedPlane;
				leadSide = side;
			}
		} else if ( f < 0 ) {
			f = d1 / f;
			if ( f < leaveFraction ) {
				leaveFraction = f;
			}
		}
	}

	// We started inside of the brush:
    if ( startOut == false ) {
        // Original point was inside brush.
        traceContext.traceResult.startSolid = true;
        // Set the brush' contents.
        traceContext.traceResult.contents = brush->contents;
		// See if we got out:
        if ( getOut == false ) {
			// We didn't get out, set fractions to 0.
            traceContext.realFraction = 0.f;
            traceContext.traceResult.fraction = 0.f;
        
			// All solid.
			traceContext.traceResult.allSolid = true;
        }
		// Exit.
		return;
    }

	// Check if this reduces collision time range.
    //if (enterFractionA <= -1) { return; }
    //if (enterFractionA > leaveFraction) { return; }

	if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
		if ( enterFractionA > -1 && enterFractionA < traceContext.realFraction ) {
			if ( enterFractionA < 0 ) {
				enterFractionA = 0;
			}

			traceContext.realFraction = enterFractionA;
			traceContext.traceResult.plane = clipPlane;
			traceContext.traceResult.surface = &(leadSide->texinfo->c);
			traceContext.traceResult.contents = brush->contents;
			traceContext.traceResult.fraction = ( enterDistance - DIST_EPSILON ) / move;
			if ( traceContext.traceResult.fraction < 0 ) {
				traceContext.traceResult.fraction = 0;
			}
		}
	} // if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
}