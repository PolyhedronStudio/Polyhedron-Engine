/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Contains all 'Tracing' through 'Leafs' related work.
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


#include "Common/CollisionModel/Tracing/BrushTraces.h"



/**
*	Temporary Hulls: Adjusted on the fly for each trace, storing the required data for
*	the 'entity headnodes' passed to the trace.
**/
//! All round 'box hull' data.
extern BoxHull boxHull;
//! All round 'octagon hull' data.
extern OctagonHull octagonHull;
//! All round 'capsule hull' data.
extern CapsuleHull capsuleHull;
//! All round 'sphere hull' data.
extern SphereHull sphereHull;


/**
*	In-Place Hulls: Used during Leaf/Brush Tests and Leaf/Brush Traces when requiring to 
*	convert a said passed-in Leaf Node into a different Shape type Hull Leaf Node.
**/
//! For 'Box' leaf tracing.
BoxHull leafTraceBoxHull;
//! For 'Sphere' leaf tracing.
SphereHull leafTraceSphereHull;
//! For 'Capsule' leaf tracing.
CapsuleHull leafTraceCapsuleHull;


/**
*	Implementations are at the bottom of this file.
**/
void CM_TraceThroughVerticalCylinder( TraceContext &traceContext, const vec3_t &origin, const float radius, const float halfHeight, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
void CM_Trace_TraceBox_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );
void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const vec3_t &offset, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
void CM_TraceAABBThroughSphere( TraceContext &traceContext, const vec3_t &sphereOrigin, const vec3_t &sphereOffset, const float sphereRadius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 );
void CM_Trace_TraceSphere_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf );


/**
*   @brief	Trace through 'World Leaf'.
**/
void CM_Test_World_ThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
    if ( !( leaf->contents & traceContext.contentsMask ) ) {
        return;
    }

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}

    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *brush = *leafbrush;

		// Skip if we checked this brush already in another leaf.
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
         
		// Update the brush' check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip if the brushmask doesn't match up.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		// Trace: 'TraceBox' -> 'Sphere 'Leaf Brush' 
		if ( traceContext.traceShape == CMHullType::Sphere ) {
			CM_Trace_TraceSphere_Through_Brush( traceContext, brush, leaf );
		// Trace: 'TraceBox' -> 'Box Brush' 
		// Trace: 'TraceBox' -> 'World Brush' 
		} else {
			CM_Trace_TraceBox_Through_Brush( traceContext, brush, leaf );
		}

		// Return if we didn't collide.
		//if ( traceContext.traceResult.allSolid ) {
		if ( !traceContext.traceResult.fraction ) {
			traceContext.realFraction = 0.f;
		    return;
		}
    }
}

/**
*   @brief	Trace 'TraceBox' through Leaf.
**/
void CM_Trace_TraceBox_ThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
    if ( !( leaf->contents & traceContext.contentsMask ) ) {
        return;
    }

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}


	/**
	*
	**/
	// Trace: 'TraceBox' -> 'Sphere 'Leaf Brush' 
	if ( traceContext.headNodeType == CMHullType::Sphere ) {
		CM_Trace_TraceBox_Through_Sphere_LeafShape( traceContext, leaf );
		return;
	}

	/**
	*	Perform a TraceBox through brush trace for all leaf brushes.
	**/

    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *brush = *leafbrush;

		// Skip if we checked this brush already in another leaf.
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
         
		// Update the brush' check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip if the brushmask doesn't match up.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }

		// Trace: 'TraceBox' -> 'Box Brush' 
		// Trace: 'TraceBox' -> 'World Brush' 
		CM_Trace_TraceBox_Through_Brush( traceContext, brush, leaf );

		// Return if we didn't collide.
		//if ( traceContext.traceResult.allSolid ) {
		if ( !traceContext.traceResult.fraction ) {
			traceContext.realFraction = 0.f;
		    return;
		}
    }
}

/**
*   @brief	Trace 'TraceSphere' through Leaf.
**/
void CM_Trace_TraceSphere_ThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
    if ( !( leaf->contents & traceContext.contentsMask ) ) {
        return;
    }

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}


	/**
	*
	**/
	// Trace: 'TraceSphere' -> 'Sphere 'Leaf Brush' 
	if ( traceContext.headNodeType == CMHullType::Sphere ) {
		CM_Trace_TraceSphere_Through_Sphere_LeafShape( traceContext, leaf );
		return;
	}

	/**
	*	Perform a TraceBox through brush trace for all leaf brushes.
	**/
    // Trace line against all brushes in the leaf
    mbrush_t **leafbrush = leaf->firstleafbrush;

    for (int32_t k = 0; k < leaf->numleafbrushes; k++, leafbrush++) {
        mbrush_t *brush = *leafbrush;

		// Skip if we checked this brush already in another leaf.
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
         
		// Update the brush' check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip if the brushmask doesn't match up.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		/**
		*	Determine what Leaf Trace to use.
		**/
		// Trace: 'TraceSphere' -> 'Box Brush' 
		// Trace: 'TraceSphere' -> 'World Brush' 
		CM_Trace_TraceSphere_Through_Brush( traceContext, brush, leaf );

		// Return if we didn't collide.
		//if ( traceContext.traceResult.allSolid ) {
		if ( !traceContext.traceResult.fraction ) {
			traceContext.realFraction = 0.f;
		    return;
		}
    }
}


/**
*
*
*	SPHERE Tracing
*
*
**/
static void _sphere_calculate_offset_rotation( const vec3_t origin, sphere_t &sphere, const bool isTransformed  ) {
	glm::vec3 vTranslate = phvec_to_glmvec3( origin );
	const glm::mat4 &matTransform = glm::translate( ph_mat_identity(), vTranslate );
	const glm::mat4 &matInvTransform = glm::inverse( matTransform );

		glm::vec4 vOffset = phvec_to_glmvec4( sphere.offset, 1 );
		//const float t = sphere.halfHeight - sphere.radius;
		const float t = sphere.halfHeight - sphere.offsetRadius;
		vOffset = matTransform * glm::vec4( t, t, t, 1.f ) * matInvTransform;
		glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
		sphere.offset = glmvec3_to_phvec( v3Offset );
}

/**
*   @brief 
**/
void CM_SetTraceBox( TraceContext &traceContext, const bbox3_t &traceBox, const bool boundsPointCase = false );
void CM_SetTraceSphere( TraceContext &traceContext, const bbox3_t &sphereBounds, const glm::mat4 &matTransform );
void CM_Trace_TraceBox_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf ) {
	// TODO: ENABLE POINT TRACE AND USE THE ALREADY DETERMINED CONSTANT
	const bool boundsPointCase = vec3_equal( traceContext.aabbTrace.bounds.mins, vec3_zero() ) && vec3_equal( traceContext.aabbTrace.bounds.maxs, vec3_zero() );
	//traceContext.isPoint = boundsPointCase;

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}


	/**
	*	Calculate the Leaf's sphere to 'Test' against.
	**/
	sphere_t leafSphere = leaf->sphereShape;
	sphere_t transformedLeafSphere = leaf->sphereShape;

	// Transform the sphere.
	if ( traceContext.isTransformedTrace ) {
		transformedLeafSphere = CM_Matrix_TransformSphere( traceContext.matTransform, leafSphere );
	}
	// Calculate offset rotation.
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedLeafSphere, traceContext.isTransformedTrace );

	// First perform a general AABB vs Sphere test.
	if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, transformedLeafSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON, true ) ) {
		return;
	}

	
	//const vec3_t aabbOrigin				 = 
	const vec3_t aabbTraceStart			 = traceContext.start;
	const vec3_t aabbTraceEnd			 = traceContext.end;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = transformedLeafSphere.origin + transformedLeafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Total test radius. ( Seems to work. )
	//const float testRadius = ( traceSphere.radius + CM_RAD_EPSILON ) + ( leafSphere.radius + CM_RAD_EPSILON );
	const float testRadius = ( traceContext.sphereTrace.transformedSphere.radius + CM_RAD_EPSILON ) + ( transformedLeafSphere.radius + CM_RAD_EPSILON );
	//const float testRadius = ( leafSphere.radius + CM_RAD_EPSILON );

	// Now perform sphere trace.
	CM_TraceAABBThroughSphere( traceContext, leafSphereOffsetOrigin, transformedLeafSphere.offset, testRadius, aabbTraceStart, aabbTraceEnd, leaf->contents );


	//// Calculate the needed data to test against.
	//// Leaf Sphere.
	//const vec3_t leafSphereOffsetOrigin = transformedLeafSphere.origin + transformedLeafSphere.offset;
	//const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	//const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;

	//// Radius to test against.
	//const float testRadius = flt_square( leafSphere.radius );
	//const float testRadiusEpsilon = flt_square( leafSphere.radius + DIST_EPSILON );


	///**
	//*	Go over each point of our transformed AABB and Test if it resides within the transformed leaf sphere.
	//**/
	//// Go over each point of our transformed AABB and see if it resides within the transformed leaf sphere.
	//static constexpr int32_t BBOX3_MAX_POINTS = 8;

	//// Closest distance length to the center of the sphere.
	//float closestDistance = FLT_MAX;
	//// Stores the resulting closest distance point from the trace to the center of the sphere.
	//vec3_t closestDistancePoint = vec3_zero();
	//// Stores the directional normal between the center of the sphere and the closest distance point.
	//vec3_t closestDistanceNormal = vec3_zero();

	//// Iterate over all 8 bounding box offset points.
	//for ( int32_t i = 0; i < BBOX3_MAX_POINTS; i++ ) {
	//	// Get the offset point to test against.
	//	const vec3_t offsetPoint = ( traceContext.isTransformedTrace ? traceContext.aabbTrace.transformedOffsets[ i ] : traceContext.aabbTrace.offsets[ i ] );
	//	//const vec3_t offsetPoint = traceContext.aabbTrace.transformedOffsets[ i ];
	//	//const vec3_t offsetPoint = traceContext.aabbTrace.offsets[ i ];

	//	// Calculate the box's total start, and end offset points.
	//	const vec3_t boxOffsetStartPoint = traceContext.start + offsetPoint;
	//	const vec3_t boxOffsetEndPoint	 = traceContext.end + offsetPoint;

	//	// Stores the resulting normal between the offset and the sphere's center.
	//	vec3_t distanceNormal = vec3_zero();

	//	// Test the start trace point's distance.
	//	const float startDistance = vec3_distance_direction( leafSphereStart, boxOffsetStartPoint, distanceNormal );

	//	if ( ( startDistance < closestDistance ) && ( startDistance < testRadius ) && !( startDistance >= testRadiusEpsilon ) ) {
	//		closestDistance = startDistance;
	//		closestDistanceNormal = distanceNormal;
	//		closestDistancePoint = boxOffsetStartPoint;
	//	}

	//	// Test the end trace point's distance.
	//	const float endDistance = vec3_distance_direction( leafSphereEnd, boxOffsetEndPoint, distanceNormal );
	//	if ( ( endDistance < closestDistance ) && ( endDistance < testRadius ) && !( endDistance >= testRadiusEpsilon ) ) {
	//		closestDistance = endDistance;
	//		closestDistanceNormal = distanceNormal;
	//		closestDistancePoint = boxOffsetEndPoint;
	//	}
	//}

	///**
	//*	Test the 'Start'/'Top' point.
	//**/
	//// Test the 'Start'/'Top' point.
	////if ( vec3_length_squared( pointTop ) < testRadius ) {
	//if ( closestDistance < testRadius ) {
	////if ( vec3_length_squared( pointTop ) < testRadius ) {
	//	//traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
	//	//traceContext.realFraction = 0.f;
	//	//traceContext.traceResult.fraction = 0.f;
	//	//traceContext.traceResult.contents = leaf->contents;
	//	const vec3_t boxOffsetStartPoint = traceContext.start + closestDistancePoint;
	//	const vec3_t boxOffsetEndPoint	 = traceContext.end + closestDistancePoint;

	//	// Origin
	//	const vec3_t sphereOrg = transformedLeafSphere.origin + transformedLeafSphere.offset;
	//	const vec3_t sphereOffset = transformedLeafSphere.offset;

	//	CM_TraceThroughSphere( traceContext, 
	//						  sphereOrg, 
	//						  sphereOffset, 
	//						  testRadiusEpsilon, 
	//						  boxOffsetStartPoint, 
	//						  boxOffsetEndPoint, 
	//						  leaf->contents );
	//	return;
	//}
}

/**
*   @brief 
**/
//void CM_TraceSphereLeafThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
void CM_Trace_TraceSphere_Through_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf ) {
	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}


	/**
	*	Calculate the Leaf's sphere to 'Trace' against.
	**/
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		//leafBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}

	// Create the test sphere.
	sphere_t leafSphere = sphere_from_size( bbox3_symmetrical( leafBounds ), bbox3_center( leafBounds ) );
	leafSphere.offset = { 0.f, 0.f, 0.f };

	// Calculate offset rotation.
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, leafSphere, traceContext.isTransformedTrace );


	/**
	*	Ensure we are hitting this 'Leaf Sphere' before Testing any further.
	**/
	// TODO: We should transform it here, just once, because right now CM_TraceIntersectSphere
	// does so too on its own.
	if ( !CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
		return;
	}
	// Spheres are always transformed when tested and traced against, so
	// transform the sphere if needed.
	//if ( !traceContext.isTransformedTrace ) {
	//	leafSphere = CM_Matrix_TransformSphere( traceContext.matInvTransform, leafSphere );
	//}


	/**
	*	Trace through the possible intersection two spheres.
	**/
	//// Trace Sphere.
	//sphere_t traceSphere = traceContext.transformedTraceSphere;

	//const vec3_t traceSphereOffsetOrigin = traceSphere.origin + traceSphere.offset;
	//const vec3_t traceSphereStart		 = ( traceContext.start - traceSphere.origin ) + traceSphere.offset; //traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	//const vec3_t traceSphereEnd			 = ( traceContext.end - traceSphere.origin ) + traceSphere.offset; //traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	//// Leaf Sphere.
	//const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	//const vec3_t leafSphereStart		= ( traceContext.start - leafSphere.origin ) + leafSphere.offset; //leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	//const vec3_t leafSphereEnd			= ( traceContext.end - leafSphere.origin ) + leafSphere.offset; //leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Trace Sphere.
	sphere_t traceSphere = traceContext.sphereTrace.transformedSphere;

	const vec3_t traceSphereOffsetOrigin = traceSphere.origin + traceSphere.offset;
	const vec3_t traceSphereStart		 = traceContext.start + traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	const vec3_t traceSphereEnd			 = traceContext.end + traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Total test radius. ( Seems to work. )
	//const float testRadius = ( traceSphere.radius + CM_RAD_EPSILON ) + ( leafSphere.radius + CM_RAD_EPSILON );
	const float testRadius = ( traceSphere.radius + CM_RAD_EPSILON ) + ( leafSphere.radius + CM_RAD_EPSILON );

	// Now perform sphere trace.
	CM_TraceThroughSphere( traceContext, traceSphereOffsetOrigin, traceSphere.offset, testRadius, leafSphereStart, leafSphereEnd, leaf->contents );
	//CM_TraceThroughSphere( traceContext, leafSphereOffsetOrigin, leafSphere.offset, testRadius, traceSphereStart, traceSphereEnd, leaf->contents );
}



/**
*
*
*	CAPSULE Tracing
*
*
**/
/**
*   @brief 
**/
void CM_TraceCapsuleThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	return;
}

/**
*   @brief 
**/

void CM_TraceBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	return;
}




/***
*
*
*	Shape specific support Trace functions:
*
*
***/
/**
*	@brief	Traces the AABB offset points through the sphere checking for any intersections.
**/
void CM_TraceAABBThroughSphere( TraceContext &traceContext, const vec3_t &sphereOrigin, const vec3_t &sphereOffset, const float sphereRadius, const vec3_t &start, const vec3_t &end, const int32_t leafContents ) {
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

		// Calculate the box's total start, and end offset points.
		const vec3_t boxOffsetStartPoint = traceContext.start + offsetPoint;
		const vec3_t boxOffsetEndPoint	 = traceContext.end + offsetPoint;

		// if inside the sphere
		const vec3_t startOffset = start + offsetPoint;
		l1 = vec3_length_squared( startOffset - sphereOrigin );

		// See if it is closer than the previous length.
		if ( l1 < closestStartLength ) {
			closestStartLength = l1;
			//startOffsetPoint = offsetPoint;
			if ( closestStartLength < closestLength ) {
				startOffsetPoint = offsetPoint;
				closestOffset = offsetPoint;
				closestLength = closestStartLength;
			}
		}

		if ( l1 < flt_square( sphereRadius ) ) {
			// Increase the count of start trace offset points residing fully within the sphere.
			startInPointCount += 1;
			//// Original point was inside brush.
			//traceContext.traceResult.startSolid = true;
			//// Set the brush' contents.
			//traceContext.traceResult.contents = leafContents;
			// test for allsolid
			const vec3_t endOffset = end + offsetPoint;
			l1 = vec3_length_squared( endOffset - sphereOrigin );
		
			// See if it is closer than the previous length.
			if ( l1 < closestEndLength ) {
				closestEndLength = l1;
				//endOffsetPoint = offsetPoint;
				if ( closestEndLength < closestLength ) {
					closestOffset = offsetPoint;
					closestLength = closestEndLength;
					endOffsetPoint = offsetPoint;
				}
			}

			if ( l1 < flt_square( sphereRadius ) ) {
				// Increase the count of end trace offset points residing fully within the sphere.
				endInPointCount += 1;
				//// We didn't get out, set fractions to 0.
				//traceContext.realFraction = 0.f;
				//traceContext.traceResult.fraction = 0.f;

				//// All solid.
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

	l1 = CM_DistanceFromLineSquared( sphereOrigin, offsetStart, offsetEnd, dir );


	v1 = offsetEnd - sphereOrigin;
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
	v1 = offsetStart - sphereOrigin;

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
			dir = intersection - sphereOrigin;
			scale = 1 / ( sphereRadius );
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

	// no intersection at all
	/*
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius;// + CM_RAD_EPSILON;
	const float squaredRadiusRadEpsilon = flt_square( radiusRadEpsilon );

	//// Calculate 2D coordinates.
	//const vec3_t origin2D	= vec3_xy( origin );
	//const vec3_t start2D	= vec3_xy( start );
	//const vec3_t end2D		= vec3_xy( end );

	// Test for being between cylinder 'lower' AND 'upper' bounds.
//	if ( start.y <= origin.y + halfHeight && start.y >= origin.y - halfHeight ) {
		// Prepare testing for whether we are inside the sphere.
		const vec3_t inDirection = start - origin;
		float inLength = vec3_length_squared( inDirection );

		// See if we started inside:
		if ( inLength < squaredRadius ) {
			// We started solid at least.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.fraction = 0.f;
			traceContext.traceResult.startSolid = true;

			// See if we got out:
			const vec3_t outDirection = end - origin;
			float outLength = vec3_length_squared( outDirection );

			if ( outLength < squaredRadius ) {
				// We did NOT get out. All Solid.
				traceContext.traceResult.allSolid = true;
			}

			// Exit.
			return;
		}
	//}

	// Line length.
	//const vec3_t lineDirection = end - start;
	//const vec3_t lineLength = vec3_normalize( lineDirection );
	float lineLength = 0;
	const vec3_t lineDirection = vec3_normalize_length( end - start, lineLength );

	// Get distance from line squared.
	const float lineLengthA = CM_DistanceFromLineSquared( origin, start, end, lineDirection );
	const vec3_t vA = end - origin;
	const float lineLengthB = vec3_length_squared( vA );

	// If the end point is at least an epsilon away, and no intersection with the cylinder occured, return.
	if ( lineLengthA >= squaredRadius && lineLengthB > squaredRadiusDistEpsilon ) {
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


	#if 0
	float b = 2.0f * ( v1[ 0 ] * lineDirection[ 0 ] + v1[ 1 ] * lineDirection[ 1 ] );
	// squaredRadiusRadEpsilon = ( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );
	float c = v1[ 0 ] * v1[ 0 ] + v1[ 1 ] * v1[ 1 ] - squaredRadiusRadEpsilon; 

	float d = b * b - 4.0f * c; // * a;
	#else
	const float b = 2.0f * ( lineDirection[ 0 ] * v1[ 0 ] + lineDirection[ 1 ] * v1[ 1 ] + lineDirection[ 2 ] * v1[ 2 ] );
	// squaredRadiusRadEpsilon = ( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );
	const float c = v1[ 0 ] * v1[ 0 ] + v1[ 1 ] * v1[ 1 ] + v1[ 2 ] * v1[ 2 ] - squaredRadiusRadEpsilon; //( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );

	const float d = b * b - 4.0f * c; // * a;
	#endif
	if ( d > 0 ) {
		float sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		float fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		//
		if ( fraction < 0 ) {
			fraction = 0;
		} else {
			fraction /= lineLength;
		}

		if ( fraction < traceContext.traceResult.fraction ) {
		//if ( fraction < traceContext.realFraction ) {
			
			const vec3_t intersectDirection = end - start; //VectorSubtract( end, start, dir );
			const vec3_t intersection = vec3_fmaf( start, fraction, intersectDirection );//VectorMA( start, fraction, dir, intersection );

			// If the intersection is between the cylinder's 'lower' AND 'upper' bound.
			//if ( intersection.z <= origin.z + halfHeight && intersection.z >= origin.z - halfHeight ) {
			{
				// Set resulting fraction.
				traceContext.traceResult.fraction = fraction; //tw->trace.fraction = fraction;
				traceContext.realFraction = fraction;

				//VectorSubtract( intersection, origin, dir );
				//dir[ 2 ] = 0;
				const vec3_t intersectDirection = /*vec3_xy( *_/ intersection - origin;// );


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
				
				//Com_WPrintf( fmt::format("CM_TraceThroughSphere set traceResult: dist({}), normal({}, {}, {})\n",
				//			traceContext.traceResult.plane.dist,
				//			traceContext.traceResult.plane.normal.x,
				//			traceContext.traceResult.plane.normal.y,
				//			traceContext.traceResult.plane.normal.z ).c_str()
				//);
			}
		}
	}
	else if ( d == 0 )
	{
		//t[0] = (- b ) / 2 * a;
		// slide along the sphere.
	}

	// no intersection at all
*/
}

/**
*	@brief	Get the first intersection of the ray with the sphere.
**/
void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const vec3_t &offset, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
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

	// no intersection at all
	/*
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius;// + CM_RAD_EPSILON;
	const float squaredRadiusRadEpsilon = flt_square( radiusRadEpsilon );

	//// Calculate 2D coordinates.
	//const vec3_t origin2D	= vec3_xy( origin );
	//const vec3_t start2D	= vec3_xy( start );
	//const vec3_t end2D		= vec3_xy( end );

	// Test for being between cylinder 'lower' AND 'upper' bounds.
//	if ( start.y <= origin.y + halfHeight && start.y >= origin.y - halfHeight ) {
		// Prepare testing for whether we are inside the sphere.
		const vec3_t inDirection = start - origin;
		float inLength = vec3_length_squared( inDirection );

		// See if we started inside:
		if ( inLength < squaredRadius ) {
			// We started solid at least.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.fraction = 0.f;
			traceContext.traceResult.startSolid = true;

			// See if we got out:
			const vec3_t outDirection = end - origin;
			float outLength = vec3_length_squared( outDirection );

			if ( outLength < squaredRadius ) {
				// We did NOT get out. All Solid.
				traceContext.traceResult.allSolid = true;
			}

			// Exit.
			return;
		}
	//}

	// Line length.
	//const vec3_t lineDirection = end - start;
	//const vec3_t lineLength = vec3_normalize( lineDirection );
	float lineLength = 0;
	const vec3_t lineDirection = vec3_normalize_length( end - start, lineLength );

	// Get distance from line squared.
	const float lineLengthA = CM_DistanceFromLineSquared( origin, start, end, lineDirection );
	const vec3_t vA = end - origin;
	const float lineLengthB = vec3_length_squared( vA );

	// If the end point is at least an epsilon away, and no intersection with the cylinder occured, return.
	if ( lineLengthA >= squaredRadius && lineLengthB > squaredRadiusDistEpsilon ) {
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


	#if 0
	float b = 2.0f * ( v1[ 0 ] * lineDirection[ 0 ] + v1[ 1 ] * lineDirection[ 1 ] );
	// squaredRadiusRadEpsilon = ( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );
	float c = v1[ 0 ] * v1[ 0 ] + v1[ 1 ] * v1[ 1 ] - squaredRadiusRadEpsilon; 

	float d = b * b - 4.0f * c; // * a;
	#else
	const float b = 2.0f * ( lineDirection[ 0 ] * v1[ 0 ] + lineDirection[ 1 ] * v1[ 1 ] + lineDirection[ 2 ] * v1[ 2 ] );
	// squaredRadiusRadEpsilon = ( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );
	const float c = v1[ 0 ] * v1[ 0 ] + v1[ 1 ] * v1[ 1 ] + v1[ 2 ] * v1[ 2 ] - squaredRadiusRadEpsilon; //( radius + RADIUS_EPSILON ) * ( radius + RADIUS_EPSILON );

	const float d = b * b - 4.0f * c; // * a;
	#endif
	if ( d > 0 ) {
		float sqrtd = sqrtf( d );
		// = (- b + sqrtd) * 0.5f;// / (2.0f * a);
		float fraction = ( -b - sqrtd ) * 0.5f; // / (2.0f * a);

		//
		if ( fraction < 0 ) {
			fraction = 0;
		} else {
			fraction /= lineLength;
		}

		if ( fraction < traceContext.traceResult.fraction ) {
		//if ( fraction < traceContext.realFraction ) {
			
			const vec3_t intersectDirection = end - start; //VectorSubtract( end, start, dir );
			const vec3_t intersection = vec3_fmaf( start, fraction, intersectDirection );//VectorMA( start, fraction, dir, intersection );

			// If the intersection is between the cylinder's 'lower' AND 'upper' bound.
			//if ( intersection.z <= origin.z + halfHeight && intersection.z >= origin.z - halfHeight ) {
			{
				// Set resulting fraction.
				traceContext.traceResult.fraction = fraction; //tw->trace.fraction = fraction;
				traceContext.realFraction = fraction;

				//VectorSubtract( intersection, origin, dir );
				//dir[ 2 ] = 0;
				const vec3_t intersectDirection = /*vec3_xy( *_/ intersection - origin;// );


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
				
				//Com_WPrintf( fmt::format("CM_TraceThroughSphere set traceResult: dist({}), normal({}, {}, {})\n",
				//			traceContext.traceResult.plane.dist,
				//			traceContext.traceResult.plane.normal.x,
				//			traceContext.traceResult.plane.normal.y,
				//			traceContext.traceResult.plane.normal.z ).c_str()
				//);
			}
		}
	}
	else if ( d == 0 )
	{
		//t[0] = (- b ) / 2 * a;
		// slide along the sphere.
	}

	// no intersection at all
*/
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