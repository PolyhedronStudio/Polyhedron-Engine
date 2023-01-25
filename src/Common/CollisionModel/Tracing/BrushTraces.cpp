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



//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;
//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;
//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
extern CapsuleHull capsuleHull;
//! All round 'sphere hull' data, accessed in a few other CollisionModel files as extern.
extern SphereHull sphereHull;


// TEMPORARILY, NEED TO ALLOW TRACETHROUGHLEAF to HAVE A 2 HULL VARIETY AND PREVENT TEMP VAR USAGE LIKE THIS
extern SphereHull boundsTestSphereHull;
extern CapsuleHull boundsTestCapsuleHull;



/**
*   @brief Performs a 'Capsule Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceCapsuleThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }
	
	// Ensure we got a proper head node type set.
	if ( traceContext.headNodeType != CMHullType::Capsule ) {
	//	return;
	}

	// Our headnode type is capsule, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, traceContext.traceSphere, 0 ) ) {
		return;
	}

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
		//float dist = 0.f;
		//if ( traceContext.isPoint ) {
		//	dist = transformedPlane.dist;
		//} else {
		//	// Adjust the plane distance appropriately for radius.
		//	//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
		//	dist = transformedPlane.dist + traceContext.traceSphere.radius;
		//}


		//// Find the closest point on the capsule to the plane
		//float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		////float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		//vec3_t startPoint;
		//vec3_t endPoint;
		////float dist = 0.f;
		////if ( t > 0 )
		////{
		//
		//	startPoint = traceContext.start - traceContext.traceSphere.offset;
		//	endPoint = traceContext.end + traceContext.traceSphere.offset;
		////	dist = dist1;
		////}
		////else
		////{
		////	startPoint = traceContext.start + traceContext.traceSphere.offset;
		////	endPoint = traceContext.end - traceContext.traceSphere.offset;
		////	dist = dist2;
		////}

		//// Calculate trace line.
		//const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - dist;

		// Determine the actual distance to, skip dot product in case of 'point tracing' and just use plane distance.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.offsetRadius;
		}

		// Find the closest point on the capsule to the plane
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint;
		vec3_t endPoint;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			endPoint = traceContext.end + traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			endPoint = traceContext.end - traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - ( transformedPlane.dist - traceContext.traceSphere.radius );
		const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - ( transformedPlane.dist + traceContext.traceSphere.radius );

		// Exited the brush.	
		if ( d2 > 0 ) {
			getOut = true; // endpoint is not in solid
		}
		// Started outside.
		if ( d1 > 0 ) {
			startOut = true;
		}

		// If completely in front of face, no intersection occured.
		if ( d1 > 0 && ( d2 >= DIST_EPSILON || d2 >= d1 ) ) {
			return;
		}

		if ( d1 <= 0 && d2 <= 0 ) {
			continue;
		}

		// Calculate the fraction, enter distance, and the total move made.
		float f = d1 - d2;

		//if ( f > 0 ) {
		if ( d1 > d2 ) {
			f = d1 / f;

			//f = ( d1 - DIST_EPSILON ) / f;
			if ( f < 0 ) {
				f = 0;
			}

			if ( f > enterFractionA ) {
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
			if ( f > 1 ) {
				f = 1;
			}
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
*   @brief Performs a 'Sphere Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceSphereThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }

	if ( traceContext.traceType != CMHullType::Sphere) {
	//	return;
	}

	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere, CM_RAD_EPSILON ) ) {
		return;
	}

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
		//float dist = 0.f;
		//if ( traceContext.isPoint ) {
		//	dist = transformedPlane.dist;
		//} else {
		//	// Adjust the plane distance appropriately for radius.
		//	//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
		//	dist = transformedPlane.dist + traceContext.traceSphere.radius;
		//}


		//// Find the closest point on the capsule to the plane
		//float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		////float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		//vec3_t startPoint;
		//vec3_t endPoint;
		////float dist = 0.f;
		////if ( t > 0 )
		////{
		//
		//	startPoint = traceContext.start - traceContext.traceSphere.offset;
		//	endPoint = traceContext.end + traceContext.traceSphere.offset;
		////	dist = dist1;
		////}
		////else
		////{
		////	startPoint = traceContext.start + traceContext.traceSphere.offset;
		////	endPoint = traceContext.end - traceContext.traceSphere.offset;
		////	dist = dist2;
		////}

		//// Calculate trace line.
		//const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - dist;

		// Determine the actual distance to, skip dot product in case of 'point tracing' and just use plane distance.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.radius;
		}

		// Find the closest point on the capsule to the plane
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint;
		vec3_t endPoint;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			endPoint = traceContext.end + traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			endPoint = traceContext.end - traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - ( transformedPlane.dist - traceContext.traceSphere.radius );
		const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - ( transformedPlane.dist + traceContext.traceSphere.radius );

		// Exited the brush.	
		if ( d2 > 0 ) {
			getOut = true; // endpoint is not in solid
		}
		// Started outside.
		if ( d1 > 0 ) {
			startOut = true;
		}

		// If completely in front of face, no intersection occured.
		if ( d1 > 0 && ( d2 >= DIST_EPSILON || d2 >= d1 ) ) {
			return;
		}

		if ( d1 <= 0 && d2 <= 0 ) {
			continue;
		}

		// Calculate the fraction, enter distance, and the total move made.
		float f = d1 - d2;

		//if ( f > 0 ) {
		if ( d1 > d2 ) {
			f = d1 / f;

			//f = ( d1 - DIST_EPSILON ) / f;
			if ( f < 0 ) {
				f = 0;
			}

			if ( f > enterFractionA ) {
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
			if ( f > 1 ) {
				f = 1;
			}
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
void CM_TraceBoxThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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
			dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
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