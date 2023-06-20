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
*
*
*	Plane VS Sphere Methods: TODO: Move elsewhere.
*
*
**/
const float plane_distance( const CollisionPlane &p, const vec3_t &point, const float extraDistance = 0.f );
const bool sphere_inside_plane( const CollisionPlane &plane, const vec3_t &traceSphereOrigin, const float sphereRadius );
const bool sphere_outside_plane( const CollisionPlane &plane, const vec3_t &traceSphereOrigin, const float sphereRadius );
const bool sphere_intersects_plane( const CollisionPlane &plane, const vec3_t &traceSphereOrigin, const float sphereRadius );
const float sphere_plane_project( const vec3_t &spherePos, const CollisionPlane & plane );
const float sphere_plane_collision_distance( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane );
const bool sphere_plane_collision( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane );
const bool sphere_intersects_plane_point( const vec3_t &traceSphereOrigin, const float sphereRadius, const CollisionPlane &plane, vec3_t &hitPoint, float &hitRadius, float &hitDistance );



/**
*
*
*	'TraceBox' Brush Traces:
*
*
**/
/**
*   @brief Performs a 'BoundingBox Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceBox_TraceThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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
	mbrushside_t *brushSide = brush->firstbrushside; //mbrushside_t *side = brush->firstbrushside + brush->numsides - 1;

	for ( int32_t i = 0; i < brush->numsides; i++, brushSide++ ) { //for (int32_t i = brush->numsides - 1; i >= 0; i--, side--) {
		// Get us the plane for this brush side.
		CollisionPlane *plane = brushSide->plane;

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
				leadSide = brushSide;
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
*
*
*	'TraceSphere' Brush Traces:
*
*
**/
CollisionPlane CM_TranslatePlane( CollisionPlane *plane, const glm::mat4 &translateMatrix );
const float sphere_plane_collision_distance( const vec3_t &point, const sphere_t &sphere, const CollisionPlane &plane );
const float sphere_plane_project( const vec3_t &spherePos, const CollisionPlane & plane );

/**
*   @brief Performs a 'Sphere Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceSphere_TraceThroughBrush(TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf) {
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
	//if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTraceSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON, true ) ) {
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
	// Keep track of whether out trace managed to exit the brush.
	bool getOut = false;
	// Keep track of whether our trace started outside of the brush.
    bool startOut = false;

	// Fractions.
    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

	// Move and enter distance.
	float enterDistance = 0.f;
	float move = 1.f;

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
		*	Trace the sphere through the brush, from 'start' to 'end' point.
		**/
		float d1 = plane_distance( transformedPlane, traceStart, traceSphereRadius );
		float d2 = plane_distance( transformedPlane, traceEnd, traceSphereRadius );

		// Exact hit points.
		vec3_t traceStartHitPoint = vec3_zero(), traceEndHitPoint = vec3_zero();
		// Exact penetrated radius.
		float traceStartHitRadius = 0.f;
		float traceEndHitRadius = 0.f;
		// Exact distance traveled between start to end, until hitting the plane.
		float traceStartHitDistance = 0.f;
		float traceEndHitDistance = 0.f;
		// Calculate whether sphere vs plane intersectioned, and if so, at what specific point did it intersect at?
		const bool startTraceIntersected = sphere_intersects_plane_point( traceStart, traceSphereRadius, transformedPlane, traceStartHitPoint, traceStartHitRadius, traceStartHitDistance );
		const bool endTraceIntersected = sphere_intersects_plane_point( traceEnd, traceSphereRadius, transformedPlane, traceEndHitPoint, traceEndHitRadius, traceEndHitDistance );

		/**
		*	Calculate sphere to plane distances for end and start trace.
		**/
		// Plane origin for distance calculation.
		const vec3_t planeOrigin = vec3_scale( transformedPlane.normal, transformedPlane.dist );
		//const vec3_t planeOrigin = vec3_scale( transformedPlane.normal, transformedPlane.dist + traceSphereRadiusEpsilon );
		// Calculate sphere to plane distance (Start Trace).
		const float startTraceDistance = vec3_dot( transformedPlane.normal, traceStart - planeOrigin );
		// Dot between plane normal and sphere 'angle'.
		const float startAngle = vec3_dot( transformedPlane.normal, traceStart );
		// Calculate extra offset for the 'Start Trace' based on the spheroid's actual angle.
		const float /*t0*/extraOffsetT1 = ( ( traceSphereRadius - traceStartHitRadius ) / startAngle );

		
		// Calculate sphere to plane distance (End Trace).
		const float endTraceDistance = vec3_dot( transformedPlane.normal, traceEnd - planeOrigin );
		// Dot between plane normal and sphere 'angle'.
		const float endAngle = vec3_dot( transformedPlane.normal, traceEnd );
		// Calculate extra offset for the 'End Trace' based on the spheroid's actual angle.
		const float /*t1*/extraOffsetT2 = ( ( -( traceSphereRadius - traceEndHitRadius ) ) / endAngle );


		//// - Inside:	If the distance is negative(-) and greater(>) than the Radius. 
		//// - Outside:	If the distance is positive(+) and greater(>) than the Radius. 
		//// - Intersection: If the absolute distance(fabs( dist )) is less than or 
		////					equal(<=) to the Radius.
		d1 += extraOffsetT1;
		d2 -= extraOffsetT2;
		//// Set hit distances.
		////const float traceStartCorrectRad = ( traceSphereRadius - traceStartHitRadius ) + CM_RAD_EPSILON;
		////const float traceEndCorrectRad = ( traceSphereRadius + traceEndHitRadius ) + CM_RAD_EPSILON;
		////d1 = traceStartHitDistance;// - traceStartCorrectRad;
		////d2 = traceEndHitDistance;// - traceEndCorrectRad;

		// Absolute distances.
		float absD1 = fabs( d1 );
		float absD2 = fabs( d2 );

		// Radius to test against.
		const float testRadius = traceSphereRadius;// traceSphereRadius;

		// Exited the brush.
		//if ( !( absD1 <= testRadius ) && d1 > 0 ) {
		//if ( d1 > testRadius && !(-d1 > 0 ) ) {
		if ( absD1 > testRadius ) {
			startOut = true;
		}
		// Started outside.
		//if ( d2 > testRadius && !(-d2 > 0 ) ) {
		if ( absD2 > testRadius ) {
			getOut = true; // End Point is not in solid.

		}

		// If completely in front of the brush' face/plane, the trace does NOT intersect with this brush side.
		//if ( d1 > testRadius && ( ( d2 >= testRadius + DIST_EPSILON ) && d2 >= d1 ) ) { 
		//if ( !( absD1 <= testRadius ) && d1 > 0 && ( !( absD2 <= testRadius ) && d2 >= d1 ) ) { 
		if ( !( absD1 <= testRadius ) && d1 > testRadius && ( !( absD2 <= testRadius + DIST_EPSILON ) && d2 >= d1 ) ) { 
		//if ( !( -d1 > 0 ) && d1 > testRadius && ( !( -d2 > 0 ) && ( d2 > testRadius ) ) ) { 
		//if ( d1 > testRadius && ( d2 >= d1 ) ) { 
			return;
		}
		// r’/cos(theta) — Ydown < (Y — projected_sphere_center) < r’/cos(theta) + Yup


		// If completely behind the brush' face/plane, the trace does NOT intersect with this brush side.
		if ( ( -d1 > testRadius ) && ( -d2 > testRadius ) && !( d1 > testRadius ) && !( d2 > testRadius ) //( -d2 <= -d1 ) 
			) {
			continue;
		}

	//	if (d1 > 0.f) {
	//		start_outside = true;
	//	}
	//	if (d2 > 0.f) {
	//		end_outside = true;
	//	}

	//	// if completely in front of plane, the trace does not intersect with the brush
	//	if (d1 > 0.f && d2 >= d1) {
	//		return;
	//	}

	//	// if completely behind plane, the trace does not intersect with this side
	//	if (d1 <= 0.f && d2 <= d1) {
	//		continue;
	//	}

	//	// the trace intersects this side
	//	const float d2d1_dist = (d1 - d2);

	//	if (d1 > d2) { // enter
	//		const float f = d1 / d2d1_dist;
	//		if (f > enter_fraction) {
	//			enter_fraction = f;
	//			plane = p;
	//			side = s;
	//			nudged_enter_fraction = (d1 - TRACE_EPSILON) / d2d1_dist;
	//		}
	//	} else { // leave
	//		const float f = d1 / d2d1_dist;
	//		if (f < leave_fraction) {
	//			leave_fraction = f;
	//		}
	//	}
	//}

	//// some sort of collision has occurred

	//if (!start_outside) { // original point was inside brush
	//	data->trace.start_solid = true;
	//	if (!end_outside) {
	//		data->trace.all_solid = true;
	//		data->trace.brush = brush;
	//		data->trace.contents = brush->contents;
	//		data->trace.fraction = 0.f;
	//		data->unnudged_fraction = 0.f;
	//	}
	//} else if (enter_fraction < leave_fraction) { // pierced brush
	//	if (enter_fraction > -1.f && enter_fraction < data->unnudged_fraction && nudged_enter_fraction < data->trace.fraction) {
	//		data->unnudged_fraction = enter_fraction;
	//		data->trace.fraction = nudged_enter_fraction;
	//		data->trace.brush = brush;
	//		data->trace.brush_side = side;
	//		data->trace.plane = plane;
	//		data->trace.contents = side->contents;
	//		data->trace.surface = side->surface;
	//		data->trace.material = side->material;
	//	}
	//}

		//if ( ( -d1 < testRadius ) && ( -d2 < testRadius ) 
		//	&& (absD1 < testRadius && absD2 < testRadius ) ) {
		//	continue;
		//}
		//// Exited the brush.
		//if ( d2 > 0 ) {
		//	getOut = true; // End Point is not in solid.
		//}
		//// Started outside.
		//if ( d1 > 0 ) {
		//	startOut = true;
		//}

		//// If completely in front of face, no intersection occured.
		////if ( d1 > 0 && ( d2 >= DIST_EPSILON || d2 >= d1 ) ) {
		//if ( d1 > 0 && d2 >= d1 ) {
		//	return;
		//}

		//if ( d1 <= 0 && d2 <= 0 ) {
		//	continue;
		//}

		//float f = d1 - d2;
		//if ( f > 0 ) {
		//	f = ( d1 ) / f;
		//	if ( f < 0 ) {
		//	//	f = 0;
		//	}

		//	if ( f > enterFractionA ) {
		//		enterDistance = ( d1 );
		//		move = d1 - d2;
		//		enterFractionA = f;
		//		clipPlane = transformedPlane;
		//		leadSide = brushSide;
		//	}
		//} else if ( f < 0 ) {
		//	f = ( d1 ) / f;
		//	if ( f > 1 ) {
		//	//	f = 1;
		//	}
		//	if ( f < leaveFraction ) {
		//		leaveFraction = f;
		//	}
		//}
		float distD1D2 = d1 - d2;
		float f = distD1D2;
		if ( d1 >= d2 ) {
			float f = ( d1 ) / distD1D2;
			if ( f < 0 ) {
				f = 0;
			}

			if ( f > enterFractionA ) {
				enterDistance = ( d1 - DIST_EPSILON ) ;
				move = d1 - d2;
				enterFractionA = f;
				clipPlane = transformedPlane;
				leadSide = brushSide;
			}
		} else if ( f < 0 ) {
			float f = ( d1  ) / (d1 - d2);
			//f = fabs(f);
			if ( f < 0 ) {
			//	f = 0;
			}
			if ( f > 1 ) {
				f = 1;
			}
			if ( f < leaveFraction ) {
				leaveFraction = f;
			}
		}

		//
		// WORKS: But, sinks to the huge ground.
		//
		// Calculate the fraction, enter distance, and the total move made.
		//float f = d1 - d2;
		//float absF = fabs( f );
		//if ( f > 0 ) {
		//	f = ( traceStartHitDistance ) / f;

		//	if ( f > enterFractionA ) {
		//		enterDistance = ( d1 );
		//		move = d1 - d2;
		//		enterFractionA = f;
		//		clipPlane = transformedPlane;
		//		leadSide = brushSide;
		//	}
		//} else if ( f < 0 ) {
		//	f = ( traceStartHitDistance ) / f;
		//	if ( f < leaveFraction ) {
		//		leaveFraction = f;
		//	}
		//}
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
 //   if ( enterFractionA <= -1 ) { 
	//	return;
	//}
 //   if ( enterFractionA - FRAC_EPSILON > leaveFraction ) { 
	//	return; 
	//}

//else if (enter_fraction < leave_fraction) { // pierced brush
	//	if (enter_fraction > -1.f && enter_fraction < data->unnudged_fraction 
		//&& nudged_enter_fraction < data->trace.fraction) {

	//if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
		//if ( enterFractionA > -1 && enterFractionA < traceContext.realFraction ) {
	
	// Calculate nudged fraction.
	const float nudgedFraction = ( enterDistance ) / move;
	const float unnudgedFraction = ( enterDistance ) / move;

	if ( enterFractionA < leaveFraction ) {
		if ( enterFractionA > -1.f && enterFractionA < traceContext.realFraction 
			&& nudgedFraction < traceContext.realFraction ) {//( enterDistance / move ) )

			if ( enterFractionA < 0 ) {
				enterFractionA = 0;
			}

			traceContext.realFraction = enterFractionA;
			traceContext.traceResult.plane = clipPlane;
			traceContext.traceResult.surface = &( leadSide->texinfo->c );
			traceContext.traceResult.contents = brush->contents;
			traceContext.traceResult.fraction = nudgedFraction;
			if ( traceContext.traceResult.fraction < 0 ) {
				traceContext.traceResult.fraction = 0;
			}
		}
	}
	//} // if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
}



/**
*
*
*	'TraceCapsule' Brush Traces:
*
*
**/
/**
*   @brief Performs a 'Capsule Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
void CM_TraceCapsule_TraceThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
	if (!brush->numsides) {
		return;
	}

	//if (traceContext.traceType != CMHullType::Capsule) {
	//	return;
	//}


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

	if ( !CM_TraceIntersectSphere( traceContext, traceSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON ) ) {
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
			traceContext.traceResult.fraction = (enterDistance - DIST_EPSILON) / move;
			if ( traceContext.traceResult.fraction < 0 ) {
				traceContext.traceResult.fraction = 0;
			}
		}
	}
}



/**
*
*
*	Shape Trace utility/support routines:
*
*
**/
/**
*	@brief	Traces the AABB offset points through the sphere checking for any intersections.
**/
void CM_TraceAABBThroughSphere( TraceContext &traceContext, const vec3_t &traceSphereOrigin, const vec3_t &sphereOffset, const float sphereRadius, const vec3_t &start, const vec3_t &end, const int32_t leafContents ) {
	float  l1, l2, length, scale, fraction;
	float  b, c, d, sqrtd;
	vec3_t v1, dir, intersection;


	/**
	*	Check each offset point for the start and end of the trace, to see whether they start inside the sphere,
	*	and/or exited the sphere.
	*
	*	While doing so we store the offsetPoint that is closest to the sphere origin(center).
	**/
	// Go over each point of our transformed AABB and see if it resides within the transformed leaf sphere.
	static constexpr int32_t BBOX3_MAX_POINTS = 8;

	// Stores the resulting length, direction, and offset points to the sphere.
	vec3_t closestOffset = vec3_zero();
	
	vec3_t startOffsetPoint = vec3_zero();
	vec3_t endOffsetPoint = vec3_zero();
	
	float closestLength = FLT_MAX;
	float closestStartLength = FLT_MAX;
	float closestEndLength = FLT_MAX;

	// Keep score of the count of points that lay on the inside of the sphere.
	int32_t endInPointCount = 0;
	int32_t startInPointCount = 0;

	// Iterate over all 8 bounding box offset points.
	for ( int32_t i = 0; i < BBOX3_MAX_POINTS; i++ ) {
		// Get the offset point to test against.
		//const vec3_t offsetPoint = traceContext.aabbTrace.transformedOffsets[ i ];
		//const vec3_t offsetPoint = traceContext.aabbTrace.offsets[ i ];
		const vec3_t offsetPoint = ( traceContext.isTransformedTrace ? traceContext.aabbTrace.transformedOffsets[ i ] : traceContext.aabbTrace.offsets[ i ] );

		// Test for startSolid by checking whether the startOffset trace point is inside the sphere.
		const vec3_t startOffset = start + offsetPoint;
		l1 = vec3_length_squared( startOffset - traceSphereOrigin );

		// See if it is closer than the previous length.
		if ( l1 < closestStartLength ) {
			closestStartLength = l1;
			startOffsetPoint = offsetPoint;
			if ( closestStartLength < closestLength ) {
				closestOffset = offsetPoint;
				closestLength = closestStartLength;
				startOffsetPoint = offsetPoint;
			}
		}

		if ( l1 < flt_square( sphereRadius ) ) {
			// Increase the count of start trace offset points residing fully within the sphere.
			startInPointCount += 1;

			// Test for allsolid by checking whether the endOffset trace point is inside the sphere.
			const vec3_t endOffset = end + offsetPoint;
			l1 = vec3_length_squared( endOffset - traceSphereOrigin );
		
			// See if it is closer than the previous length.
			if ( l1 < closestEndLength ) {
				closestEndLength = l1;
				endOffsetPoint = offsetPoint;
				if ( closestEndLength < closestLength ) {
					closestOffset = offsetPoint;
					closestLength = closestEndLength;
					endOffsetPoint = offsetPoint;
				}
			}

			if ( l1 < flt_square( sphereRadius + DIST_EPSILON ) ) {
				// Increase the count of end trace offset points residing fully within the sphere.
				endInPointCount += 1;
				// We didn't get out, set fractions to 0.
				//traceContext.realFraction = 0.f;
				//traceContext.traceResult.fraction = 0.f;

				// All solid.
				//traceContext.traceResult.allSolid = true;
			}

		}

	}

	/**
	*	Determine wether we startedSolid, and got allSolid. (Whether we started inside, and got out of the sphere,
	*	or whether we are fully inside of it. In either case, we return from escape this function.
	**/
	// If at least one point of the start trace is inside the sphere:
	if ( startInPointCount > 0 ) {
		// Original point was inside brush.
		traceContext.traceResult.startSolid = true;
		// Set the brush' contents.
		traceContext.traceResult.contents = leafContents;

		// If all end points reside within the sphere, we are: allSolid
		if ( endInPointCount >= BBOX3_MAX_POINTS ) {
			// We didn't get out, set fractions to 0.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.fraction = 0.f;

			// All solid.
			traceContext.traceResult.allSolid = true;
		}
		
		// Escape the trace.
		return;
	}


	/**
	*	Project the trace of the closest offset point onto the sphere.
	**/
	// Calculate the start and end offset trace points.
	const vec3_t offsetStart = start;// + closestOffset;
	const vec3_t offsetEnd = end;// + closestOffset;

	//
	dir = offsetEnd - offsetStart;
	dir = vec3_normalize_length( dir, length );

	l1 = CM_DistanceFromLineSquared( traceSphereOrigin, offsetStart, offsetEnd, dir );


	v1 = offsetEnd - traceSphereOrigin;
	l2 = vec3_length_squared( v1 );

	// if no intersection with the sphere and the end point is at least an epsilon away
	if ( l1 >= flt_square( sphereRadius ) && l2 > flt_square( sphereRadius + DIST_EPSILON ) ) {
		return;
	}

	/**
	*	We fully intersected: Now calculate the exact fraction, and normal, of the resulting intersection
	*	on the sphere.
	**/
	// We don't return anymore from here, so initialize clean plane.
	// Set plane normal to that of direction vector.
	//traceContext.traceResult.plane = CollisionPlane();

	//
	//  | origin - (start + t * dir) | = radius
	//  a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//  b = 2 * (dir[0] * (start[0] - origin[0]) + dir[1] * (start[1] - origin[1]) + dir[2] * (start[2] - origin[2]));
	//  c = (start[0] - origin[0])^2 + (start[1] - origin[1])^2 + (start[2] - origin[2])^2 - radius^2;
	//
	v1 = offsetStart - traceSphereOrigin;

	// Dir is normalized so a = 1
	//dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
	b = 2.0f * ( dir[ 0 ] * v1[ 0 ] + dir[ 1 ] * v1[ 1 ] + dir[ 2 ] * v1[ 2 ] );
	c = ( v1[ 0 ] * v1[ 0 ] ) + ( v1[ 1 ] * v1[ 1 ] ) + ( v1[ 2 ] * v1[ 2 ] ) - ( sphereRadius + CM_RAD_EPSILON ) * ( sphereRadius + CM_RAD_EPSILON );
	d = b * b - 4.0f * c; // * a;
	
	//const float a = (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	//d = b * b - 4.0f * a; // * a;

	if ( d > 0 ) {
		// Calculate Trace Hit fraction.
		//sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		// However, their comment states using a + in the formula instead?
		// The results seem far better either way.
		sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f; // / (2.0f * a);
		fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		if ( fraction < 0 ) {
			fraction = 0;
		} else {
			fraction /= length;
		}

		if ( fraction < traceContext.traceResult.fraction ) { //		if ( fraction < traceContext.traceResult.fraction) {
			traceContext.traceResult.fraction = fraction; //			traceContext.realFraction = traceContext.traceResult.fraction = fraction;
			//traceContext.realFraction = fraction;

			// Calculate plane hit normal.
			dir = offsetEnd - offsetStart;
			intersection = vec3_fmaf( offsetStart, fraction, dir );
			dir = intersection - traceSphereOrigin;
			scale = 1 / ( sphereRadius + CM_RAD_EPSILON );
			dir = vec3_scale( dir, scale ); 

			// Setup a new plane with the resulting normal.
			traceContext.traceResult.plane = CollisionPlane();
			traceContext.traceResult.plane.normal = dir;

			// Calculate plane distance, translate intersection if need be.
			glm::vec4 translatedIntersection = phvec_to_glmvec4( intersection );
			if ( traceContext.isTransformedTrace ) {
				translatedIntersection = traceContext.matTransform * translatedIntersection;// * traceContext.matInvTransform;
			}
			const vec3_t modelOriginIntersection = glmvec4_to_phvec( translatedIntersection ) ;

			// Derive our plane distance based from Model Origin Intersection.
			traceContext.traceResult.plane.dist = vec3_dot( traceContext.traceResult.plane.normal, modelOriginIntersection );
			
			// Update plane signbits and type.
			SetPlaneSignbits( &traceContext.traceResult.plane );
			SetPlaneType( &traceContext.traceResult.plane );

			// Set contents, take it from the leaf.
			traceContext.traceResult.contents = leafContents;
		}
	}
	else if ( d == 0 )
	{
		//t1 = (- b ) / 2;
		// slide along the sphere

		// Calculate Trace Hit fraction.
		//sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		// However, their comment states using a + in the formula instead?
		// The results seem far better either way.
		//sqrtd = sqrtf( d );
		//// = (- b + sqrtd) * 0.5f; // / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		//if ( fraction < 0 ) {
		//	fraction = 0;
		//} else {
		//	fraction /= length;
		//}

		//if ( fraction < traceContext.traceResult.fraction ) { //		if ( fraction < traceContext.traceResult.fraction) {
		//	traceContext.traceResult.fraction = fraction; //			traceContext.realFraction = traceContext.traceResult.fraction = fraction;
		//	//traceContext.realFraction = fraction;

		//	// Calculate plane hit normal.
		//	dir = end - start;
		//	intersection = vec3_fmaf( start, fraction, dir );
		//	dir = intersection - origin;
		//	scale = 1 / ( radius + CM_RAD_EPSILON );
		//	dir = vec3_scale( dir, scale );
		//	

		//	// Setup a new plane with the resulting normal.
		//	traceContext.traceResult.plane = CollisionPlane();
		//	traceContext.traceResult.plane.normal = dir;

		//	// Calculate plane distance, translate intersection if need be.
		//	glm::vec4 translatedIntersection = phvec_to_glmvec4( intersection );
		//	if ( traceContext.isTransformedTrace ) {
		//		translatedIntersection = traceContext.matTransform * translatedIntersection;// * traceContext.matInvTransform;
		//	}
		//	const vec3_t modelOriginIntersection = glmvec4_to_phvec( translatedIntersection ) ;

		//	// Derive our plane distance based from Model Origin Intersection.
		//	traceContext.traceResult.plane.dist = vec3_dot( traceContext.traceResult.plane.normal, modelOriginIntersection );
		//	
		//	// Update plane signbits and type.
		//	SetPlaneSignbits( &traceContext.traceResult.plane );
		//	SetPlaneType( &traceContext.traceResult.plane );

		//	// Set contents, take it from the leaf.
		//	traceContext.traceResult.contents = leafContents;
		//}
	}
}

/**
*	@brief	Get the first intersection of the ray with the sphere.
**/
void CM_TraceLineThroughSphere( TraceContext &traceContext, const vec3_t &origin, const vec3_t &offset, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
	float  l1, l2, length, scale, fraction;
	float  b, c, d, sqrtd;
	vec3_t v1, dir, intersection;

	// if inside the sphere
	dir = start - origin;
	l1 = vec3_length_squared( dir );

	if ( l1 < flt_square( radius ) ) {
		// Original point was inside brush.
		traceContext.traceResult.startSolid = true;
		// Set the brush' contents.
		traceContext.traceResult.contents = leafContents;
		// test for allsolid
		dir = end - origin;
		l1 = vec3_length_squared( dir );

		if ( l1 < flt_square( radius ) ) {
			// We didn't get out, set fractions to 0.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.fraction = 0.f;

			// All solid.
			traceContext.traceResult.allSolid = true;
		}

		return;
	}

	//
	dir = end - start;
	/*length = */
	dir = vec3_normalize_length( dir, length );
	//
	l1 = CM_DistanceFromLineSquared( origin, start, end, dir );
	v1 = end - origin;
	l2 = vec3_length_squared( v1 );

	// if no intersection with the sphere and the end point is at least an epsilon away
	if ( l1 >= flt_square( radius ) && l2 > flt_square( radius + DIST_EPSILON ) ) {
		return;
	}

	// We don't return anymore from here, so initialize clean plane.
	// Set plane normal to that of direction vector.
	//traceContext.traceResult.plane = CollisionPlane();

	//
	//  | origin - (start + t * dir) | = radius
	//  a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//  b = 2 * (dir[0] * (start[0] - origin[0]) + dir[1] * (start[1] - origin[1]) + dir[2] * (start[2] - origin[2]));
	//  c = (start[0] - origin[0])^2 + (start[1] - origin[1])^2 + (start[2] - origin[2])^2 - radius^2;
	//
	v1 = start - origin;

	// Dir is normalized so a = 1
	//dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2];
	b = 2.0f * ( dir[ 0 ] * v1[ 0 ] + dir[ 1 ] * v1[ 1 ] + dir[ 2 ] * v1[ 2 ] );
	c = ( v1[ 0 ] * v1[ 0 ] ) + ( v1[ 1 ] * v1[ 1 ] ) + ( v1[ 2 ] * v1[ 2 ] ) - ( radius + CM_RAD_EPSILON ) * ( radius + CM_RAD_EPSILON );
	d = b * b - 4.0f * c; // * a;
	
	//const float a = (dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	//d = b * b - 4.0f * a; // * a;

	if ( d > 0 ) {
		// Calculate Trace Hit fraction.
		//sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		// However, their comment states using a + in the formula instead?
		// The results seem far better either way.
		sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f; // / (2.0f * a);
		fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		if ( fraction < 0 ) {
			fraction = 0;
		} else {
			fraction /= length;
		}

		if ( fraction < traceContext.traceResult.fraction ) { //		if ( fraction < traceContext.traceResult.fraction) {
			traceContext.traceResult.fraction = fraction; //			traceContext.realFraction = traceContext.traceResult.fraction = fraction;
			//traceContext.realFraction = fraction;

			// Calculate plane hit normal.
			dir = end - start;
			intersection = vec3_fmaf( start, fraction, dir );
			dir = intersection - origin;
			scale = 1 / ( radius + CM_RAD_EPSILON );
			dir = vec3_scale( dir, scale ); 

			// Setup a new plane with the resulting normal.
			traceContext.traceResult.plane = CollisionPlane();
			traceContext.traceResult.plane.normal = dir;

			// Calculate plane distance, translate intersection if need be.
			glm::vec4 translatedIntersection = phvec_to_glmvec4( intersection );
			if ( traceContext.isTransformedTrace ) {
				translatedIntersection = traceContext.matTransform * translatedIntersection;// * traceContext.matInvTransform;
			}
			const vec3_t modelOriginIntersection = glmvec4_to_phvec( translatedIntersection ) ;

			// Derive our plane distance based from Model Origin Intersection.
			traceContext.traceResult.plane.dist = vec3_dot( traceContext.traceResult.plane.normal, modelOriginIntersection );
			
			// Update plane signbits and type.
			SetPlaneSignbits( &traceContext.traceResult.plane );
			SetPlaneType( &traceContext.traceResult.plane );

			// Set contents, take it from the leaf.
			traceContext.traceResult.contents = leafContents;
		}
	}
	else if ( d == 0 )
	{
		//t1 = (- b ) / 2;
		// slide along the sphere

		// Calculate Trace Hit fraction.
		//sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		// However, their comment states using a + in the formula instead?
		// The results seem far better either way.
		//sqrtd = sqrtf( d );
		//// = (- b + sqrtd) * 0.5f; // / (2.0f * a);
		//fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		//if ( fraction < 0 ) {
		//	fraction = 0;
		//} else {
		//	fraction /= length;
		//}

		//if ( fraction < traceContext.traceResult.fraction ) { //		if ( fraction < traceContext.traceResult.fraction) {
		//	traceContext.traceResult.fraction = fraction; //			traceContext.realFraction = traceContext.traceResult.fraction = fraction;
		//	//traceContext.realFraction = fraction;

		//	// Calculate plane hit normal.
		//	dir = end - start;
		//	intersection = vec3_fmaf( start, fraction, dir );
		//	dir = intersection - origin;
		//	scale = 1 / ( radius + CM_RAD_EPSILON );
		//	dir = vec3_scale( dir, scale );
		//	

		//	// Setup a new plane with the resulting normal.
		//	traceContext.traceResult.plane = CollisionPlane();
		//	traceContext.traceResult.plane.normal = dir;

		//	// Calculate plane distance, translate intersection if need be.
		//	glm::vec4 translatedIntersection = phvec_to_glmvec4( intersection );
		//	if ( traceContext.isTransformedTrace ) {
		//		translatedIntersection = traceContext.matTransform * translatedIntersection;// * traceContext.matInvTransform;
		//	}
		//	const vec3_t modelOriginIntersection = glmvec4_to_phvec( translatedIntersection ) ;

		//	// Derive our plane distance based from Model Origin Intersection.
		//	traceContext.traceResult.plane.dist = vec3_dot( traceContext.traceResult.plane.normal, modelOriginIntersection );
		//	
		//	// Update plane signbits and type.
		//	SetPlaneSignbits( &traceContext.traceResult.plane );
		//	SetPlaneType( &traceContext.traceResult.plane );

		//	// Set contents, take it from the leaf.
		//	traceContext.traceResult.contents = leafContents;
		//}
	}
}

/**
*	@brief	Get the first intersection of the ray with the cylinder, the cylinder extends 
*			'halfHeight' above and below the 'origin'.
**/
void CM_TraceThroughVerticalCylinder( TraceContext &traceContext, const vec3_t &origin, const float radius, const float halfHeight, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
	
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius + CM_RAD_EPSILON;
	const float squaredRadiusRadEpsilon = flt_square( radiusRadEpsilon );

	// Calculate 2D coordinates.
	const vec3_t origin2D	= vec3_xy( origin );
	const vec3_t start2D	= vec3_xy( start );
	const vec3_t end2D		= vec3_xy( end );

	// Test for being between cylinder 'lower' AND 'upper' bounds.
	if ( start.z <= origin.z + halfHeight && start.z >= origin.z - halfHeight ) {
		// Prepare testing for whether we are inside the cylinder.
		const vec3_t inDirection = start2D - origin2D;
		float inLength = vec3_length_squared( inDirection );

		// See if we started inside:
		if ( inLength < squaredRadius ) {
			// We started solid at least.
			traceContext.realFraction= 0.f;
			traceContext.traceResult.fraction = 0.f;
			traceContext.traceResult.startSolid = true;

			// See if we got out:
			const vec3_t outDirection = end2D - origin2D;
			float outLength = vec3_length_squared( outDirection );

			if ( outLength < squaredRadius ) {
				// We did NOT get out. All Solid.
				traceContext.traceResult.allSolid = true;
			}

			// Exit.
			return;
		}
	}

	// Line length.
	//const vec3_t lineDirection = end2D - start2D;
	//const vec3_t lineLength = vec3_normalize( lineDirection );
	float lineLength = 0;
	const vec3_t lineDirection = vec3_normalize_length( end2D - start2D, lineLength );

	// Get distance from line squared.
	const float lineLengthA = CM_DistanceFromLineSquared( origin2D, start2D, end2D, lineDirection );
	const vec3_t vA = end2D - origin2D;
	const float lineLengthB = vec3_length_squared( vA );

	// If the end point is at least an epsilon away, and no intersection with the cylinder occured, return.
	if ( lineLengthA >= squaredRadius && lineLengthB > squaredRadiusDistEpsilon ) {
		Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder exited at: if ( lineLengthA >= squaredRadius && lineLengthB > squaredRadiusDistEpsilon )\n").c_str() );
		return;
	}

	//
	//
	// (start[0] - origin[0] - t * dir[0]) ^ 2 + (start[1] - origin[1] - t * dir[1]) ^ 2 = radius ^ 2
	// (v1[0] + t * dir[0]) ^ 2 + (v1[1] + t * dir[1]) ^ 2 = radius ^ 2;
	// v1[0] ^ 2 + 2 * v1[0] * t * dir[0] + (t * dir[0]) ^ 2 +
	//                      v1[1] ^ 2 + 2 * v1[1] * t * dir[1] + (t * dir[1]) ^ 2 = radius ^ 2
	// t ^ 2 * (dir[0] ^ 2 + dir[1] ^ 2) + t * (2 * v1[0] * dir[0] + 2 * v1[1] * dir[1]) +
	//                      v1[0] ^ 2 + v1[1] ^ 2 - radius ^ 2 = 0
	//
	const vec3_t v1 = start - origin;//VectorSubtract( start, origin, v1 );
	// dir is normalized so we can use a = 1
	// * (dir[0] * dir[0] + dir[1] * dir[1]);
	//float b = 2.0f * ( v1[ 0 ] * dir[ 0 ] + v1[ 1 ] * dir[ 1 ] );
	float b = 2.0f * ( v1[ 0 ] * lineDirection[ 0 ] + v1[ 1 ] * lineDirection[ 1 ] );
	// squaredRadiusRadEpsilon = ( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );
	float c = v1[ 0 ] * v1[ 0 ] + v1[ 1 ] * v1[ 1 ] - squaredRadiusRadEpsilon; 

	float d = b * b - 4.0f * c; // * a;

	if ( d > 0 ) {
		float sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		float fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);
		Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder: d({}) > 0\n", d).c_str());

		//
		if ( fraction < 0 )
		{
			Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder: fraction({}) < 0\n", fraction ).c_str());
			fraction = 0;	
		} else {
			fraction /= lineLength;
			Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder: fraction({}) > 0\n", fraction ).c_str());
		}

		if ( fraction < traceContext.traceResult.fraction ) {
			Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder: fraction({}) < traceContext.traceResult.fraction({})\n", fraction, traceContext.traceResult.fraction ).c_str() );

			//if ( fraction < traceContext.realFraction ) {

			const vec3_t intersectDirection = end - start; //VectorSubtract( end, start, dir );
			const vec3_t intersection = vec3_fmaf( start, fraction, intersectDirection );//VectorMA( start, fraction, dir, intersection );

			// If the intersection is between the cylinder's 'lower' AND 'upper' bound.
			if ( intersection.z <= origin.z + halfHeight && intersection.z >= origin.z - halfHeight ) {
				// Set resulting fraction.
				traceContext.realFraction = fraction;
				traceContext.traceResult.fraction = fraction; //tw->trace.fraction = fraction;
				

				//VectorSubtract( intersection, origin, dir );
				//dir[ 2 ] = 0;
				vec3_t intersectDirection = intersection - origin;
				intersectDirection.z = 0.f;

				float intersectScale = 1.f / radiusRadEpsilon;//( radius + RADIUS_EPSILON );
				const vec3_t scaledIntersectDirection = vec3_scale( intersectDirection, intersectScale );//VectorScale( dir, scale, dir );

				// Set normal.
				//traceContext.traceResult.plane = CollisionPlane();
				traceContext.traceResult.plane.normal = scaledIntersectDirection; // VectorCopy( dir, tw->trace.plane.normal );
				
				// Model Origin offset intersection <-- It was never set in Daemon? Nowhere?
				//VectorAdd( tw->modelOrigin, intersection, intersection );
				// GLM:
				//glm::vec4 vIntersection = glm::vec4( phvec_to_glmvec3( intersection ), 1.0 );
				//if ( traceContext.isTransformedTrace ) {
				//	vIntersection = traceContext.matInvTransform * vIntersection;//traceContext.matTransform[3] + vIntersection;// * vIntersection;
				//} else {
				//	vIntersection = traceContext.matTransform[3] + vIntersection;
				//}
				//glm::vec3 v3Intersection = glm::vec3( vIntersection.x / vIntersection.w, vIntersection.y / vIntersection.w, vIntersection.z / vIntersection.w );
				// EOF GLM:
				const vec3_t modelOriginIntersection = intersection;//glmvec3_to_phvec( v3Intersection ); //intersection; //traceContext.modelOrigin + intersection;
				
				// Calculate plane distance.
				//tw->trace.plane.dist = DotProduct( tw->trace.plane.normal, intersection );
				traceContext.traceResult.plane.dist = vec3_dot( traceContext.traceResult.plane.normal, modelOriginIntersection );
				
				// Update plane signbits and type.
				SetPlaneSignbits( &traceContext.traceResult.plane );
				SetPlaneType( &traceContext.traceResult.plane );

				// Set contents, take it from the leaf.
				traceContext.traceResult.contents = leafContents;

				Com_WPrintf( fmt::format("CM_TraceThroughVerticalCylinder set traceResult: dist({}), normal({}, {}, {})\n",
							traceContext.traceResult.plane.dist,
							traceContext.traceResult.plane.normal.x,
							traceContext.traceResult.plane.normal.y,
							traceContext.traceResult.plane.normal.z ).c_str()
				);
			}
		}
	}
	else if ( d == 0 )
	{
		//t[0] = (- b ) / 2 * a;
		// slide along the cylinder
	}

	// no intersection at all
}