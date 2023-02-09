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


// TODO: Move these elsewhere.
sphere_t bbox3_to_capsule( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );
void capsule_calculate_offset_rotation( TraceContext &traceContext, sphere_t &sphere, const bool isTransformed = false );

sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin );
sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin );
void sphere_calculate_offset_rotation( TraceContext &traceContext, sphere_t &sphere, const bool isTransformed );

const bbox3_t CM_CalculateSphereTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius );



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
*   @brief	Tests the traceContext headNode leaf against the passed in leaf,
*			picking a designated 'Shape Hull' test function based on the 
*			traceContext's traceType. (Capsule, Sphere, Box, etc).
**/
void CM_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	// Ensure the collision model is properly precached.
	if ( !traceContext.collisionModel || !traceContext.collisionModel->cache ) {
		return;
	}

	// Ensure leaf contents are valid.
	if ( !(leaf->contents & traceContext.contentsMask ) ) {
		return;
	}

    // Trace line against all brushes in the leaf
    mbrush_t **leafBrush = leaf->firstleafbrush;

    for ( int32_t k = 0; k < leaf->numleafbrushes; k++, leafBrush++ ) {
        mbrush_t *brush = *leafBrush;
        
        if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
            continue;   // Already checked this brush in another leaf
        }
        
        brush->checkcount = traceContext.collisionModel->checkCount;

        if ( !( brush->contents & traceContext.contentsMask ) ) {
            continue;
        }
        
		// Perform a specific hull type test based on our trace type.
		// 'Capsule' brush test:
		if ( traceContext.traceType == CMHullType::Capsule ) {
			CM_TestCapsuleInBrush( traceContext, brush, leaf );
		// 'Sphere' brush test:
		} else if ( traceContext.traceType == CMHullType::Sphere ) {
			CM_TestSphereInBrush( traceContext, brush, leaf );
		// Default to, 'Box' brush test:
		} else {
	        CM_TestBoundingBoxInBrush( traceContext, brush, leaf );
		}
        
		//if ( !traceContext.traceResult.fraction ) {
		if ( traceContext.traceResult.allSolid ) {
		    return;
		}

    }
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

/**
*   @brief	Exchanges the tracing type with 'Capsule' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}


	/**
	*	Calculate the 'Leaf Sphere' to test the trace with.
	**/
	// Get bounds to apply for creating a sphere with.
	bbox3_t testSphereBounds = leaf->bounds;
	// Inverse Transform it if need be.
	if ( traceContext.isTransformedTrace ) {
	//	testSphereBounds = CM_Matrix_TransformBounds( traceContext.matTransform, testSphereBounds );
//		testSphere = CM_Matrix_TransformSphere( traceContext.matTransform, testSphere );
	}
	// Create Sphere from Bounds.
	sphere_t testSphere = sphere_from_size( bbox3_symmetrical( testSphereBounds ), bbox3_center( testSphereBounds ));
	testSphere.offset = { 0.f, 0.f, 0.f };


	/**
	*	Determine what piece of the capsule we possibly might've hit: 'Top Sphere', 'Middle Cylinder', or 'Bottom Sphere'.
	**/
	// Sphere for the intersection tests.
	sphere_t intersectTestSphere = testSphere;

	// Calculate cylinder and see if we hit it.
	sphere_t cylinderTestSphere = intersectTestSphere;
	cylinderTestSphere.origin = vec3_xy( cylinderTestSphere.origin );
	cylinderTestSphere.offset = { 0.f, 0.f, 0.f };
	bbox3_t noZAbsoluteBounds = bbox3_expand( { 
		vec3_xy( traceContext.absoluteBounds.mins ),
		vec3_xy( traceContext.absoluteBounds.maxs ),
	}, FLT_EPSILON );
	const bool hitCylinder		= bbox3_intersects_sphere( noZAbsoluteBounds, cylinderTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
	if ( !hitCylinder ) {
		return;
	} else {
		// Skip this one, it has no Z.
		//intersectTestSphere = testSphere;
	}

	// Calculate offset for top sphere, see if we hit it.
	const float t = intersectTestSphere.halfHeight - intersectTestSphere.offsetRadius;
	sphere_t topTestSphere = intersectTestSphere;
	testSphere.offset = { 0.f, 0.f, -t };
	const bool hitTopSphere		= bbox3_intersects_sphere( traceContext.absoluteBounds, topTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
	
	// Calculate offset for bottom sphere, see if we hit it.
	sphere_t bottomTestSphere = intersectTestSphere;
	testSphere.offset = { 0.f, 0.f, t };
	const bool hitBottomSphere	= bbox3_intersects_sphere( traceContext.absoluteBounds, bottomTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f );
	
	if ( !hitTopSphere && !hitBottomSphere ) {
		return;
	} else {
		if (hitTopSphere) {
			intersectTestSphere = topTestSphere;
		} else if ( hitBottomSphere ) {
			intersectTestSphere = bottomTestSphere;
		}
	}


	/**
	*	Adjust our traceContext to prepare for 'Sphere' in 'Leaf Testing' the 
	*	leaf-bounds matching BoxHull.
	**/
	// Create and store BoxHull.
	//bbox3_t symmetricLeafBounds = bbox3_from_center_size( bbox3_symmetrical( leaf->bounds ), bbox3_center( leaf->bounds ) );
	leafTestBoxHull = CM_NewBoundingBoxHull( 
		bbox3_from_center_size( traceContext.size, bbox3_center( leaf->bounds ) ),
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);

	// "Abuse", the Sphere Test Hull.
	leafTestCapsuleHull.sphere = intersectTestSphere;

	// Change trace type.
	traceContext.traceType = CMHullType::Capsule;
	// Keep our traceSphere as is however.
	//traceContext.traceSphere = traceSphere;

	// Replace the head node we're working with.
	traceContext.headNode = leafTestBoxHull.headNode;
	traceContext.headNodeLeaf = &leafTestBoxHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;

	// Perform Test.
	CM_TestInLeaf( traceContext, &leafTestBoxHull.leaf );
}

/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_SetTraceBox( TraceContext &traceContext, const bbox3_t &traceBox, const bool boundsPointCase = false );
void CM_SetTraceSphere( TraceContext &traceContext, const bbox3_t &sphereBounds, const glm::mat4 &matTransform );
void CM_TestBoxLeafInSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;
	
	/**
	*	Calculate the 'Leaf Sphere' to test the trace with.
	**/
	const bbox3_t oldLeafBounds = oldLeafNode->bounds;
	const bbox3_t traceBounds = traceContext.bounds;
	const vec3_t traceTransformOrigin = glmvec4_to_phvec( traceContext.matTransform[3] );

	// Create the test sphere.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		//leafBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, leaf->bounds );
	}
	sphere_t traceSphere = sphere_from_size( bbox3_symmetrical( traceBounds ), vec3_zero() );	//sphere_t testSphere = CM_SphereFromBounds( leafBounds, vec3_zero() );
	traceSphere.offset = { 0.f, 0.f, 0.f };
	if ( traceContext.isTransformedTrace ) {
		//CM_Matrix_TransformSphere( traceContext.matTransform, traceSphere );
	}
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, traceSphere, true );


	/**
	*	Ensure we are hitting this 'Leaf Sphere' before Testing any further.
	**/
	if ( !CM_TraceIntersectSphere( traceContext, traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 0.f ) ) {
		return;
	}


	/**
	*	Test whether the trace box, when centered around leaf->bounds intersected our trace.
	**/
	// Create a centered at center of leaf->bounds box that is symmetrical to our transformed bounds.
	const bbox3_t inPlaceLeafBounds = bbox3_from_center_size( 
		bbox3_symmetrical( leafBounds ),//bbox3_symmetrical( leafBounds ),
		//traceTransformOrigin
		bbox3_center( leaf->bounds )
	);
	//// Ensure we actually HIT the box before creating its hull.
	//if ( !CM_TraceIntersectBounds( traceContext, inPlaceLeafBounds ) ) {
	//	return;
	//}

	// "Abuse", the Sphere Test Hull.
	leafTestSphereHull.sphere = traceSphere;

	// Create the 'In-Place' leaf box hull.
	leafTestBoxHull = CM_NewBoundingBoxHull( 
		// Bounds.
		inPlaceLeafBounds,
		// Ensure it keeps its contents.
		leaf->contents 
	);


	/**
	*	Prepare and perform the actual 'Leaf Test' itself.
	**/
	// Change trace type.
	traceContext.traceType = CMHullType::Sphere;
	// Keep our traceSphere as is however.
	//traceContext.traceSphere = traceSphere;

	// Replace the head node we're working with.
	traceContext.headNode = leafTestBoxHull.headNode;
	traceContext.headNodeLeaf = &leafTestBoxHull.leaf;
	traceContext.headNodeType = CMHullType::Sphere;
	
	// Perform Test.
	CM_TestInLeaf( traceContext, &leafTestBoxHull.leaf );
	//CM_TestInLeaf( traceContext, leaf );
}

/**
*   @brief	Performs a 'Sphere' hull test on the 'Sphere' leaf.
**/
void CM_TestSphereLeafInSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;
	

	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}


	/**
	*	Calculate the Leaf's sphere to 'Test' against.
	**/
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		//leafBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}

	// Create the test sphere.
	sphere_t leafSphere = sphere_from_size( bbox3_symmetrical( leafBounds ), vec3_zero() );
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
	if ( !traceContext.isTransformedTrace ) {
		leafSphere = CM_Matrix_TransformSphere( traceContext.matTransform, leafSphere );
	}


	/**
	*	Test whether the two spheres intersect/overlap.
	**/
	// Trace Sphere.
	sphere_t traceSphere = traceContext.traceSphere;

	const vec3_t traceSphereOffsetOrigin = traceContext.traceSphere.origin + traceContext.traceSphere.offset;
	const vec3_t traceSphereStart		 = traceContext.start - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	const vec3_t traceSphereEnd			 = traceContext.end - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start - leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end - leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;


	/**
	*	Test whether the two spheres intersect/overlap.
	**/
	// Total test radius.
	const float testRadius = flt_square( traceSphere.radius + leafSphere.radius );

	// Top point.
	const vec3_t pointTop = traceSphereOffsetOrigin - leafSphereStart; //leafSphereOffsetOrigin - traceSphereTop;

	if ( vec3_length_squared( pointTop ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
		traceContext.traceResult.contents = leaf->contents;
	}

	// Bottom point.
	const vec3_t pointBottom = traceSphereOffsetOrigin - leafSphereEnd; //traceSphereOffsetOrigin - traceSphereBottom;
	if ( vec3_length_squared( pointBottom ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
		traceContext.traceResult.contents = leaf->contents;
	}
}