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

#include "Common/CollisionModel/Tracing/LeafTests.h"
#include "Common/CollisionModel/Tracing/LeafTraces.h"
#include "Common/CollisionModel/Tracing/BrushTests.h"



/**
*	In-Place Hulls: Used during Leaf/Brush Tests and Leaf/Brush Traces when requiring to 
*	convert a said passed-in Leaf Node into a different Shape type Hull Leaf Node.
**/
//! For 'Box' leaf testing.
BoxHull leafTestBoxHull;
//! For 'Sphere' leaf testing.
SphereHull leafTestSphereHull;
//! For 'Ca[si;e' leaf testing.
CapsuleHull leafTestCapsuleHull;


/**
*   @brief	Test in 'World Leaf'.
**/
void CM_Test_World_InLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
	if ( !(leaf->contents & traceContext.contentsMask ) ) {
		return;
	}

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}

	/**
	*	Perform a TraceBox/TraceSphere in brush test depending on what trace type we're dealing with.
	**/
    mbrush_t **leafBrush = leaf->firstleafbrush;

    for ( int32_t k = 0; k < leaf->numleafbrushes; k++, leafBrush++ ) {
        mbrush_t *brush = *leafBrush;
		
		// Skip the brush('continue') if we've already checked this brush in another leaf
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
        
		// Assign current check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip the brush('continue') if the contents mask does not match to that of our trace.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		/**
		*	Determine what Leaf Test to use.
		**/
		// Trace: 'TraceSphere' -> 'Box Brush'
		// Trace: 'TraceSphere' -> 'World Brush'
		if ( traceContext.traceShape == CMHullType::Sphere ) {
			CM_Test_TraceSphere_In_Brush( traceContext, brush, leaf  );
		// Trace: 'TraceBox' -> 'Box Brush' 
		// Trace: 'TraceBox' -> 'World Brush' 
		} else {
			CM_Test_TraceBox_In_Brush( traceContext, brush, leaf );
		}
        
		// Exit in case of achieving an 'allSolid' trace state.
		if ( traceContext.traceResult.allSolid ) { //if ( !traceContext.traceResult.fraction ) {
			return;
		}
    }
}

/**
*   @brief	Test 'TraceBox' in Leaf.
**/
void CM_Test_TraceBox_InLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
	// Trace: 'TraceBox' -> 'Sphere Leaf Brush' 
	if ( traceContext.headNodeType == CMHullType::Sphere ) {
		CM_Test_TraceBox_In_Sphere_LeafShape( traceContext, leaf );
		return;
	}


	/**
	*	Perform a TraceBox in brush test for all leaf brushes.
	**/
    // Trace line against all brushes in the leaf
    mbrush_t **leafBrush = leaf->firstleafbrush;

    for ( int32_t k = 0; k < leaf->numleafbrushes; k++, leafBrush++ ) {
        mbrush_t *brush = *leafBrush;
        
		// Skip the brush('continue') if we've already checked this brush in another leaf
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
        
		// Assign current check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip the brush('continue') if the contents mask does not match to that of our trace.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		// Trace: 'TraceBox' -> 'Box Brush' 
		// Trace: 'TraceBox' -> 'World Brush' 
		CM_Test_TraceBox_In_Brush( traceContext, brush, leaf );
        
		// Exit in case of achieving an 'allSolid' trace state.
		if ( traceContext.traceResult.allSolid ) { //if ( !traceContext.traceResult.fraction ) {
			return;
		}
    }
}

/**
*   @brief	Test 'TraceSphere' in Leaf.
**/
void CM_Test_TraceSphere_InLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
	if ( !(leaf->contents & traceContext.contentsMask ) ) {
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
		CM_Test_TraceSphere_In_Sphere_LeafShape( traceContext, leaf );
		return;
	}

	/**
	*	Perform a TraceSphere in brush test for all leaf brushes.
	**/
    // Trace line against all brushes in the leaf
    mbrush_t **leafBrush = leaf->firstleafbrush;

    for ( int32_t k = 0; k < leaf->numleafbrushes; k++, leafBrush++ ) {
        mbrush_t *brush = *leafBrush;
        
		// Skip the brush('continue') if we've already checked this brush in another leaf
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   
        }
        
		// Assign current check count.
        brush->checkcount = traceContext.collisionModel->checkCount;

		// Skip the brush('continue') if the contents mask does not match to that of our trace.
        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		// Trace: 'TraceSphere' -> 'Box Brush' 
		// Trace: 'TraceSphere' -> 'World Brush' 
		CM_Test_TraceSphere_In_Brush( traceContext, brush, leaf );

		// Exit in case of achieving an 'allSolid' trace state.
		if ( traceContext.traceResult.allSolid ) { //if ( !traceContext.traceResult.fraction ) {
			return;
		}
    }
}



/**
*
*
*	SPHERE Testing
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
*	@brief	Calculates the AABBTrace bounds needed to trace, by expanding our bounds by CM_BOUNDS_EPSILON offset and transform 
*			the bounds by our inverse matrix when dealing with a transformedTrace, after which it finishes by calculating its 
*			symmetrical extents.
**/
const TraceContext::AABBTrace CM_AABB_CalculateTraceShape( TraceContext &traceContext, const bbox3_t &bounds, const bool boundsPointCase = false, 
													   const bool isTransformedTrace = false, const glm::mat4 &matTransform = ph_mat_identity(), const glm::mat4 &matInvTransform = ph_mat_identity() );
/**
*	@brief	Calculates a new sphere shape to trace with for the trace context.
*	@param	isTransformed			When true, the transformedSphere is transformed by the trace context's transform matrix. If false
*									however, it will be identical to the non transformed sphere itself.
*	@param	expandRadiusEpsilon		Expand by radius epsilon, for use if sphereBounds isn't epsilonOffset yet.
**/
const TraceContext::SphereTrace CM_Sphere_CalculateTraceShape( TraceContext &traceContext, const bbox3_t &sphereBounds, const bool boundsPointCase = false, const bool expandRadiusEpsilon = false, 
														 const bool isTransformedTrace = false, const glm::mat4 &matTransform = ph_mat_identity(), const glm::mat4 &matInvTransform = ph_mat_identity() );
/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_Test_TraceBox_In_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf ) {
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
	if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, transformedLeafSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, 0.f, true ) ) {
		return;
	}

	// Calculate the needed data to test against.
	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;

	// Radius to test against.
	const float testRadius = flt_square( leafSphere.radius );
	const float testRadiusEpsilon = flt_square( leafSphere.radius + DIST_EPSILON );
	

	/**
	*	Go over each point of our transformed AABB and Test if it resides within the transformed leaf sphere.
	**/
	// Go over each point of our transformed AABB and see if it resides within the transformed leaf sphere.
	static constexpr int32_t BBOX3_MAX_POINTS = 8;

	// Closest distance length to the center of the sphere.
	float closestDistance = FLT_MAX;
	// Stores the resulting closest distance point from the trace to the center of the sphere.
	vec3_t closestDistancePoint = vec3_zero();
	// Stores the directional normal between the center of the sphere and the closest distance point.
	vec3_t closestDistanceNormal = vec3_zero();

	// Iterate over all 8 bounding box offset points.
	for ( int32_t i = 0; i < BBOX3_MAX_POINTS; i++ ) {
		// Get the offset point to test against.
		const vec3_t offsetPoint = ( traceContext.isTransformedTrace ? traceContext.aabbTrace.transformedOffsets[ i ] : traceContext.aabbTrace.offsets[ i ] );
		//const vec3_t offsetPoint = traceContext.aabbTrace.transformedOffsets[ i ];
		//const vec3_t offsetPoint = traceContext.aabbTrace.offsets[ i ];

		// Calculate the box's total start, and end offset points.
		const vec3_t boxOffsetStartPoint = traceContext.start + offsetPoint;
		const vec3_t boxOffsetEndPoint	 = traceContext.end + offsetPoint;

		// Stores the resulting normal between the offset and the sphere's center.
		vec3_t distanceNormal = vec3_zero();

		// Test the start trace point's distance.
		const float startDistance = vec3_distance_direction( leafSphereStart, boxOffsetStartPoint, distanceNormal );

		if ( ( startDistance < closestDistance ) && ( startDistance < testRadius ) && !( startDistance >= testRadiusEpsilon ) ) {
			closestDistance = startDistance;
			closestDistanceNormal = distanceNormal;
			closestDistancePoint = boxOffsetStartPoint;
		}

		// Test the end trace point's distance.
		const float endDistance = vec3_distance_direction( leafSphereEnd, boxOffsetEndPoint, distanceNormal );
		if ( ( endDistance < closestDistance ) && ( endDistance < testRadius ) && !( endDistance >= testRadiusEpsilon ) ) {
			closestDistance = endDistance;
			closestDistanceNormal = distanceNormal;
			closestDistancePoint = boxOffsetEndPoint;
		}
	}

	/**
	*	Test the 'Start'/'Top' point.
	**/
	// Test the 'Start'/'Top' point.
	//if ( vec3_length_squared( pointTop ) < testRadius ) {
	if ( closestDistance < testRadius ) {
	//if ( vec3_length_squared( pointTop ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.realFraction = 0.f;
		traceContext.traceResult.fraction = 0.f;
		traceContext.traceResult.contents = leaf->contents;
		return;
	}

	//// Go over each point of our transformed AABB and see if it resides within the transformed leaf sphere.
	//static constexpr int32_t BBOX3_MAX_POINTS = 8;

	//// Find closest point to the sphere center.
	//for ( int32_t i = 0; i < BBOX3_MAX_POINTS; i++ ) {
	//	// Get the offset point to test against.
	//	//const vec3_t offsetPoint = traceContext.aabbTrace.transformedOffsets[ i ];

	//	// Calculate the start and end points for our transformed bounds to test against the sphere with.
	//	const vec3_t boxOffsetStartPoint = traceContext.start + offsetPoint;
	//	const vec3_t boxOffsetEndPoint	 = traceContext.end + offsetPoint;

	//	/**
	//	*	Test the 'Start'/'Top' point.
	//	**/
	//	// Stores the top test normal result.
	//	vec3_t topDirectionNormal;
	//	// Test the 'Start'/'Top' point.
	//	//if ( vec3_length_squared( pointTop ) < testRadius ) {
	//	if ( vec3_distance_direction( leafSphereOffsetOrigin, boxOffsetStartPoint, topDirectionNormal ) < testRadius ) {
	//	//if ( vec3_length_squared( pointTop ) < testRadius ) {
	//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
	//		traceContext.realFraction = 0.f;
	//		traceContext.traceResult.fraction = 0.f;
	//		traceContext.traceResult.contents = leaf->contents;
	//		return;
	//	}

	//	/**
	//	*	Test the 'End'/'Bottom' point.
	//	**/
	//	// Stores the bottom test normal result.
	//	vec3_t bottomDirectionNormal;
	//	// Test the 'End'/'Bottom' point.
	//	if ( vec3_distance_direction( leafSphereOffsetOrigin, boxOffsetEndPoint, bottomDirectionNormal ) < testRadius ) {
	//	//if ( vec3_length_squared( pointBottom ) < testRadius ) {
	//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
	//		traceContext.realFraction = 0.f;
	//		traceContext.traceResult.fraction = 0.f;
	//		traceContext.traceResult.contents = leaf->contents;
	//		return;
	//	}
	//}
}

/**
*   @brief	Performs a 'Sphere' hull test on the 'Sphere' leaf.
**/
void CM_Test_TraceSphere_In_Sphere_LeafShape( TraceContext &traceContext, mleaf_t *leaf ) {
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
	if ( !CM_TraceIntersectSphere( traceContext, leafSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0 ) ) {
		return;
	}

	// Spheres are always transformed when tested and traced against, so
	// transform the sphere if needed.
	//if ( !traceContext.isTransformedTrace ) {
	//	leafSphere = CM_Matrix_TransformSphere( traceContext.matInvTransform, leafSphere );
	//}


	/**
	*	Test whether the two spheres intersect/overlap.
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


	/**
	*	Test
	**/
	// Total test radius. (Seems to work)
	//const float testRadius = /*flt_square*/( traceSphere.radius + leafSphere.radius );
	const float testRadius = flt_square( traceSphere.radius + CM_RAD_EPSILON + leafSphere.radius + CM_RAD_EPSILON);

	// Top point.
	//const vec3_t pointTop = traceSphereOffsetOrigin - leafSphereStart;
	const vec3_t pointTop = leafSphereOffsetOrigin - traceSphereStart;

	if ( vec3_length_squared( pointTop ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.realFraction = 0.f;
		traceContext.traceResult.fraction = 0.f;
		traceContext.traceResult.contents = leaf->contents;
	}

	// Bottom point.
	//const vec3_t pointBottom = traceSphereOffsetOrigin - leafSphereEnd; //traceSphereOffsetOrigin - traceSphereBottom;
	const vec3_t pointBottom = leafSphereOffsetOrigin - traceSphereEnd; //traceSphereOffsetOrigin - traceSphereBottom;
	if ( vec3_length_squared( pointBottom ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.realFraction = 0.f;
		traceContext.traceResult.fraction = 0.f;
		traceContext.traceResult.contents = leaf->contents;
	}
}



/**
*
*
*	CAPSULE Testing
*
*
**/
/**
*   @brief	Exchanges the tracing type with 'Capsule' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	return;
//	/**
//	*	Ensure we are hitting this bounding box before testing any further.
//	**/
//	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
//		return;
//	}
//
//
//	/**
//	*	Calculate the 'Leaf Sphere' to test the trace with.
//	**/
//	// Get bounds to apply for creating a sphere with.
//	bbox3_t testSphereBounds = leaf->bounds;
//	// Inverse Transform it if need be.
//	if ( traceContext.isTransformedTrace ) {
//	//	testSphereBounds = CM_Matrix_TransformBounds( traceContext.matTransform, testSphereBounds );
////		testSphere = CM_Matrix_TransformSphere( traceContext.matTransform, testSphere );
//	}
//	// Create Sphere from Bounds.
//	sphere_t testSphere = sphere_from_size( bbox3_symmetrical( testSphereBounds ), bbox3_center( testSphereBounds ));
//	testSphere.offset = { 0.f, 0.f, 0.f };
//
//
//	/**
//	*	Determine what piece of the capsule we possibly might've hit: 'Top Sphere', 'Middle Cylinder', or 'Bottom Sphere'.
//	**/
//	// Sphere for the intersection tests.
//	sphere_t intersectTestSphere = testSphere;
//
//	// Calculate cylinder and see if we hit it.
//	sphere_t cylinderTestSphere = intersectTestSphere;
//	cylinderTestSphere.origin = vec3_xy( cylinderTestSphere.origin );
//	cylinderTestSphere.offset = { 0.f, 0.f, 0.f };
//	bbox3_t noZAbsoluteBounds = bbox3_expand( { 
//		vec3_xy( traceContext.absoluteBounds.mins ),
//		vec3_xy( traceContext.absoluteBounds.maxs ),
//	}, FLT_EPSILON );
//	const bool hitCylinder		= bbox3_intersects_sphere( noZAbsoluteBounds, cylinderTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
//	if ( !hitCylinder ) {
//		return;
//	} else {
//		// Skip this one, it has no Z.
//		//intersectTestSphere = testSphere;
//	}
//
//	// Calculate offset for top sphere, see if we hit it.
//	const float t = intersectTestSphere.halfHeight - intersectTestSphere.offsetRadius;
//	sphere_t topTestSphere = intersectTestSphere;
//	testSphere.offset = { 0.f, 0.f, -t };
//	const bool hitTopSphere		= bbox3_intersects_sphere( traceContext.absoluteBounds, topTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
//	
//	// Calculate offset for bottom sphere, see if we hit it.
//	sphere_t bottomTestSphere = intersectTestSphere;
//	testSphere.offset = { 0.f, 0.f, t };
//	const bool hitBottomSphere	= bbox3_intersects_sphere( traceContext.absoluteBounds, bottomTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
//	
//	if ( !hitTopSphere && !hitBottomSphere ) {
//		return;
//	} else {
//		if (hitTopSphere) {
//			intersectTestSphere = topTestSphere;
//		} else if ( hitBottomSphere ) {
//			intersectTestSphere = bottomTestSphere;
//		}
//	}
//
//
//	/**
//	*	Adjust our traceContext to prepare for 'Sphere' in 'Leaf Testing' the 
//	*	leaf-bounds matching BoxHull.
//	**/
//	// Create and store BoxHull.
//	//bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
//	leafTestBoxHull = CM_NewBoundingBoxHull( 
//		bbox3_from_center_size( traceContext.size, bbox3_center( leaf->bounds ) ),
//		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
//	);
//
//	// "Abuse", the Sphere Test Hull.
//	leafTestCapsuleHull.sphere = intersectTestSphere;
//
//	// Change trace type.
//	traceContext.traceType = CMHullType::Capsule;
//	// Keep our traceSphere as is however.
//	//traceContext.traceSphere = traceSphere;
//
//	// Replace the head node we're working with.
//	traceContext.headNode = leafTestBoxHull.headNode;
//	traceContext.headNodeLeaf = &leafTestBoxHull.leaf;
//	traceContext.headNodeType = CMHullType::Capsule;
//
//	// Perform Test.
//	CM_TestInLeaf( traceContext, &leafTestBoxHull.leaf );
}

/**
*   @brief	Tests whether the traceContext traceType capsule against a temporary leaf capsule.
**/
void CM_TestCapsuleLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	return;

//	// Well here we go I guess.
//	vec3_t sphereTop = traceContext.start + traceContext.traceSphere.offset;
//	vec3_t sphereBottom = traceContext.start - traceContext.traceSphere.offset;
////	vec3_t sphereOffset = (traceContext.bounds.mins + traceContext.bounds.maxs) * 0.5f;
//
//	
//	// If the trace was transformed, and we are changing the leafs around, invert matrix it.
//	bbox3_t leafBounds = leaf->bounds;
//	if ( traceContext.isTransformedTrace ) {
//		CM_Matrix_TransformBounds( traceContext.matTransform, leafBounds );
//	}
//
//	// Switch trace type to capsule and replace the bounding box with the capsule for our leaf.
//	traceContext.traceType = CMHullType::Capsule;
//	// Use capsuleHull headnode.
//	//traceContext.headNode = capsuleHull.headNode;
//	// Capsule hull leaf node.
//	traceContext.headNodeLeaf = &capsuleHull.leaf;
//	traceContext.headNodeType = CMHullType::Capsule;
//	
//	//
//	CapsuleHull testCapsuleHull = CM_NewCapsuleHull( leafBounds, leaf->contents );
//	sphere_t leafSphere = testCapsuleHull.sphere;
//	vec3_t sphereOffset = leafSphere.offset;
//
//	// Sphere
//	//traceContext.traceSphere = CM_CalculateCMSphere( bbox3_size( leaf->bounds ), bbox3_center( leaf->bounds ) );//traceContext.extents );
//	//// Rotated sphere for capsule
//	//if ( traceContext.isTransformedTrace ) {
//	//	traceContext.traceSphere = CM_CalculateCMSphereFromBounds( traceContext.transformedBounds, bbox3_center( traceContext.transformedBounds ) );//traceContext.extents );
//	//// Regular capsule trace sphere.
//	//} else {
//	//	traceContext.traceSphere = CM_CalculateCMSphereFromBounds( traceContext.boundsEpsilonOffset, bbox3_center( traceContext.boundsEpsilonOffset ) );//bbox3_symmetrical( traceContext.bounds ) );
//	//}
//
//
//
//	//// Calculate sphere for leaf
//	//const bbox3_t leafSymmetricBounds = bbox3_from_center_size( 
//	//	bbox3_symmetrical( leaf->bounds ), vec3_zero()
//	//	//bbox3_size( leaf->bounds ), vec3_zero() 
//	//);
//	////CMSphere leafSphere = CM_CalculateCMSphere( leafSymmetricBounds );
//	//sphere_t leafSphere = CM_CalculateCMSphere( bbox3_size( leaf->bounds ) );
//
//	//float offsetZ = leafSphere.offset.z;
//
//	//// Test Radius: See if the spheres overlap
//	float testRadius = ( traceContext.traceSphere.radius + leafSphere.radius ) * ( traceContext.traceSphere.radius + leafSphere.radius );
//
//	//
//	// Top Sphere.
//	//
//	vec3_t pointA = sphereOffset + leafSphere.offset;//vec3_t{ 0.f, 0.f, offsetZ };
//	vec3_t temp = pointA - sphereTop;
//
//	if ( vec3_length_squared( temp ) < testRadius ) {
//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
//		traceContext.traceResult.fraction = 0.f;
//	}
//	temp = pointA - sphereBottom;
//
//	if ( vec3_length_squared( temp ) < testRadius ) {
//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
//		traceContext.traceResult.fraction = 0.f;
//	}
//
//	//
//	// Bottom Sphere.
//	//
//	vec3_t pointB = sphereOffset - leafSphere.offset;//- vec3_t{ 0.f, 0.f, offsetZ };
//	temp = pointB - sphereTop;
//
//	if ( vec3_length_squared( temp ) < testRadius ) {
//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
//		traceContext.traceResult.fraction = 0.f;
//	}
//	temp = pointB - sphereBottom;
//
//	if ( vec3_length_squared( temp ) < testRadius ) {
//		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
//		traceContext.traceResult.fraction = 0.f;
//	}
//
//	//
//	// If between cylinder up and lower bounds.
//	//
//	if ( ( sphereTop.z >= pointA.z && sphereTop.z <= pointB.z ) || ( sphereBottom.z >= pointA.z && sphereBottom.z <= pointB.z ) ) {
//		// 2d coordinates
//		sphereTop.z = pointA.z = 0;
//		// if the cylinders overlap
//		temp = sphereTop - pointA;
//		
//		if ( vec3_length_squared( temp ) < testRadius ) {
//			traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
//			traceContext.traceResult.fraction = 0.f;
//		}
//	}
}