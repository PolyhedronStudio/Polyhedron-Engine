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
		if ( traceContext.headNodeType == CMHullType::Capsule ) {
			CM_TestCapsuleInBrush( traceContext, brush, leaf );
		// 'Sphere' brush test:
		} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
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

	// Well here we go I guess.
	vec3_t sphereTop = traceContext.start + traceContext.traceSphere.offset;
	vec3_t sphereBottom = traceContext.start - traceContext.traceSphere.offset;
//	vec3_t sphereOffset = (traceContext.bounds.mins + traceContext.bounds.maxs) * 0.5f;

	
	// If the trace was transformed, and we are changing the leafs around, invert matrix it.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matTransform, leafBounds );
	}

	// Switch trace type to capsule and replace the bounding box with the capsule for our leaf.
	traceContext.traceType = CMHullType::Capsule;
	// Use capsuleHull headnode.
	traceContext.headNode = capsuleHull.headNode;
	// Capsule hull leaf node.
	traceContext.headNodeLeaf = &capsuleHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;
	
	//
	CapsuleHull testCapsuleHull = CM_NewCapsuleHull( leafBounds, leaf->contents );
	sphere_t leafSphere = testCapsuleHull.sphere;
	vec3_t sphereOffset = leafSphere.offset;

	// Sphere
	//traceContext.traceSphere = CM_CalculateCMSphere( bbox3_size( leaf->bounds ), bbox3_center( leaf->bounds ) );//traceContext.extents );
	//// Rotated sphere for capsule
	//if ( traceContext.isTransformedTrace ) {
	//	traceContext.traceSphere = CM_CalculateCMSphereFromBounds( traceContext.transformedBounds, bbox3_center( traceContext.transformedBounds ) );//traceContext.extents );
	//// Regular capsule trace sphere.
	//} else {
	//	traceContext.traceSphere = CM_CalculateCMSphereFromBounds( traceContext.boundsEpsilonOffset, bbox3_center( traceContext.boundsEpsilonOffset ) );//bbox3_symmetrical( traceContext.bounds ) );
	//}



	//// Calculate sphere for leaf
	//const bbox3_t leafSymmetricBounds = bbox3_from_center_size( 
	//	bbox3_symmetrical( leaf->bounds ), vec3_zero()
	//	//bbox3_size( leaf->bounds ), vec3_zero() 
	//);
	////CMSphere leafSphere = CM_CalculateCMSphere( leafSymmetricBounds );
	//sphere_t leafSphere = CM_CalculateCMSphere( bbox3_size( leaf->bounds ) );

	//float offsetZ = leafSphere.offset.z;

	//// Test Radius: See if the spheres overlap
	float testRadius = ( traceContext.traceSphere.radius + leafSphere.radius ) * ( traceContext.traceSphere.radius + leafSphere.radius );

	//
	// Top Sphere.
	//
	vec3_t pointA = sphereOffset + leafSphere.offset;//vec3_t{ 0.f, 0.f, offsetZ };
	vec3_t temp = pointA - sphereTop;

	if ( vec3_length_squared( temp ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
	}
	temp = pointA - sphereBottom;

	if ( vec3_length_squared( temp ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
	}

	//
	// Bottom Sphere.
	//
	vec3_t pointB = sphereOffset - leafSphere.offset;//- vec3_t{ 0.f, 0.f, offsetZ };
	temp = pointB - sphereTop;

	if ( vec3_length_squared( temp ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
	}
	temp = pointB - sphereBottom;

	if ( vec3_length_squared( temp ) < testRadius ) {
		traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		traceContext.traceResult.fraction = 0.f;
	}

	//
	// If between cylinder up and lower bounds.
	//
	if ( ( sphereTop.z >= pointA.z && sphereTop.z <= pointB.z ) || ( sphereBottom.z >= pointA.z && sphereBottom.z <= pointB.z ) ) {
		// 2d coordinates
		sphereTop.z = pointA.z = 0;
		// if the cylinders overlap
		temp = sphereTop - pointA;
		
		if ( vec3_length_squared( temp ) < testRadius ) {
			traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
			traceContext.traceResult.fraction = 0.f;
		}
	}
}

/**
*   @brief	Exchanges the tracing type with 'Capsule' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
// TODO: Move elsewhere ofc.
sphere_t CM_CapsuleSphereFromBounds( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );
void CM_CalculateSphereOffsetRotation( TraceContext &traceContext, sphere_t &sphere );
const bool bbox3_intersects_sphere( const bbox3_t &boxA, const sphere_t &sphere, const float radiusDistEpsilon );

void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	sphere_t oldTraceSphere = traceContext.traceSphere;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;


//	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
//		return;
//	}

	/**
	*	Acquire the bounds, and in case of a transformed trace, exchange their 
	*	transformations by the inverse/transform matrices.
	**/
	// Get the 'Capsule' bounds, and transform by 'matTransform' in case of transformed traces.
	bbox3_t boxBounds = traceContext.transformedBounds;
	if ( traceContext.isTransformedTrace ) {
		//boxBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, boxBounds );
		boxBounds = traceContext.boundsEpsilonOffset;
	}
	// Get the 'Capsule' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		leafBounds = CM_Matrix_TransformBounds( traceContext.matTransform, leafBounds );
	}
	

	/**
	*	Create temporary Hulls for the Capsule and the Box.
	**/
	// Capsule:
	CapsuleHull capsuleHull = CM_NewCapsuleHull( 
		boxBounds, leaf->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
	// Calculate center offset.
	CM_CalculateSphereOffsetRotation( traceContext, capsuleHull.sphere );
	// Box:
	BoxHull boxHull = CM_NewBoundingBoxHull( 
		leafBounds, 
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);

	// Calculate offset.
	const float t = capsuleHull.sphere.halfHeight - capsuleHull.sphere.offsetRadius;
	capsuleHull.sphere.offset = { 0, 0, t };

	// Determine 
	float st = vec3_dot( traceContext.start, capsuleHull.sphere.origin );
	sphere_t testSphere = capsuleHull.sphere;

	if ( st > 0 ) {
		testSphere.offset = { 0, 0, t };
		//if ( bbox3_intersects_sphere( traceContext.absoluteBounds, testSphere, 0 ) ) {
		//	traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		//	traceContext.traceResult.fraction = 0.0f;
		//}
	} else {
		testSphere.offset = { 0, 0, -t };
		//if ( bbox3_intersects_sphere( traceContext.absoluteBounds, testSphere, 0 ) ) {
		//	traceContext.traceResult.startSolid = traceContext.traceResult.allSolid = true;
		//	traceContext.traceResult.fraction = 0.0f;
		//}
	}

	//} else {
	//	const float t = capsuleHull.sphere.halfHeight - capsuleHull.sphere.offsetRadius;
	//	capsuleHull.sphere.offset = { 0, 0, -t };
	//}
	//capsuleHull.sphere = testSphere;

	/**
	*	Adjust the trace context with the exchanged hulls and perform a Test in Leaf.
	**/
	traceContext.traceType = CMHullType::Capsule;
	traceContext.traceSphere = testSphere; //capsuleHull.sphere;
	traceContext.headNode = capsuleHull.headNode;
	traceContext.headNodeLeaf = &capsuleHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;

	// Store capsule hull thingy.
	boundsTestCapsuleHull = capsuleHull;

	// Perform testing.
	CM_TestInLeaf( traceContext, &boxHull.leaf );

	//// Revert back to old.
	//traceContext.traceType = oldTraceType;
	//traceContext.traceSphere = oldTraceSphere;
	//traceContext.headNodeType = oldHeadNodeType;
	//traceContext.headNode = oldHeadNode;
	//traceContext.headNodeLeaf = oldLeafNode;
}

/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
void CM_TestBoxLeafInSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;
	
//	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
//		return;
//	}

	/**
	*	Acquire the bounds, and in case of a transformed trace, exchange their 
	*	transformations by the inverse/transform matrices.
	**/
	// Get the 'Sphere' bounds, and transform by 'matTransform' in case of transformed traces.
	bbox3_t boxBounds = traceContext.transformedBounds;
	if ( traceContext.isTransformedTrace ) {
		//boxBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, boxBounds );
		boxBounds = traceContext.boundsEpsilonOffset;
	}
	// Get the 'Sphere' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		leafBounds = CM_Matrix_TransformBounds( traceContext.matTransform, leafBounds );
	}
	

	/**
	*	Create temporary Hulls for the Sphere and the Box.
	**/
	// Sphere:
	SphereHull sphereHull = CM_NewSphereHull( 
		boxBounds, leaf->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
	// Calculate center offset.
	CM_CalculateSphereOffsetRotation( traceContext, sphereHull.sphere );
	// Box:
	BoxHull boxHull = CM_NewBoundingBoxHull( 
		leafBounds, 
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);


	/**
	*	Adjust the trace context with the exchanged hulls and perform a Test in Leaf.
	**/
	traceContext.traceType = CMHullType::Sphere;
	traceContext.traceSphere = sphereHull.sphere;
	traceContext.headNode = sphereHull.headNode;
	traceContext.headNodeLeaf = &sphereHull.leaf;
	traceContext.headNodeType = CMHullType::Sphere;

	// Store sphere hull thingy.
	boundsTestSphereHull = sphereHull;

	// Perform testing.
	CM_TestInLeaf( traceContext, &boxHull.leaf );

	//// Revert back to old.
	//traceContext.traceType = oldTraceType;
	//traceContext.traceSphere = oldTraceSphere;
	//traceContext.headNodeType = oldHeadNodeType;
	//traceContext.headNode = oldHeadNode;
	//traceContext.headNodeLeaf = oldLeafNode;
}