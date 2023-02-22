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


// TODO: Move these elsewhere.
sphere_t bbox3_to_capsule( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );
void capsule_calculate_offset_rotation( TraceContext &traceContext, sphere_t &sphere, const bool isTransformed = false );

sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin );
sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin );
void sphere_calculate_offset_rotation( TraceContext &traceContext, sphere_t &sphere, const bool isTransformed );

const bbox3_t CM_CalculateSphereTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius );



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
void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const vec3_t &offset, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
void CM_TraceSphereThroughSphere( TraceContext &traceContext, mleaf_t *leaf );


/**
*   @brief	Traces the traceContext headNode leaf against the passed in leaf,
*			picking a designated 'Shape Hull' trace function based on the 
*			traceContext's traceType. (Capsule, Sphere, Box, etc).
**/
void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
        
		/**
		*	Determine what Leaf Trace to use.
		**/
		// 'Sphere' THROUGH 'Box Brush' Trace:
		if ( traceContext.headNodeType == CMHullType::World &&
			traceContext.traceType == CMHullType::Sphere ) {
			CM_TraceSphereThroughBrush( traceContext, brush, leaf );
		// 'Sphere' THROUGH 'Box Brush' Trace:
		} else if ( traceContext.headNodeType == CMHullType::Box &&
			traceContext.traceType == CMHullType::Sphere ) {
			CM_TraceSphereThroughBrush( traceContext, brush, leaf );

		// 'Sphere' THROUGH 'Sphere' Trace:
		} else if ( traceContext.headNodeType == CMHullType::Sphere &&
			traceContext.traceType == CMHullType::Sphere ) {
			CM_TraceSphereThroughSphere( traceContext, leaf );

		// Default: 'Box' IN 'Brush' Trace:
		} else {
	        CM_TraceBoxThroughBrush( traceContext, brush, leaf );
		}

		// Return if we didn't collide.
		if ( !traceContext.traceResult.fraction ) {
		//	traceContext.realFraction = 0.f;
		//if ( traceContext.traceResult.allSolid ) {
		    return;
		}
    }
}



/**
*
*
*	Box through Capsule/Sphere/Box/'LeafBrush'.
*
*
**/
/**
*   @brief 
**/
void CM_TraceCapsuleThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	return;

	//// Test bounds vs trace bounds.
	if (!CM_TraceIntersectBounds( 
			traceContext, leaf->bounds 
	) ) {
		return;
	}
	// Well here we go I guess.
	vec3_t capsuleStartTop = traceContext.start + traceContext.traceSphere.offset;
	vec3_t capsuleStartBottom = traceContext.start - traceContext.traceSphere.offset;
	vec3_t capsuleEndTop = traceContext.end + traceContext.traceSphere.offset;
	vec3_t capsuleEndBottom = traceContext.end - traceContext.traceSphere.offset;
		
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

	float offsetZ = leafSphere.offset.z;
	vec3_t leafSphereOffset = leafSphere.offset; //{ 0.f, 0.f, offsetZ };

	// Test Radius: See if the spheres overlap
	float testRadius = ( traceContext.traceSphere.radius + leafSphere.radius ) * ( traceContext.traceSphere.radius + leafSphere.radius );

	//
	// Top Sphere.
	//
	vec3_t topSphere = sphereOffset;//sphereOffset + vec3_t{ 0.f, 0.f, offsetZ };
	topSphere.z += offsetZ;

	//
	// Bottom Sphere.
	//
	vec3_t bottomSphere = sphereOffset;// - vec3_t{ 0.f, 0.f, offsetZ };
	bottomSphere.z -= offsetZ;

	//
	// HMM: Expand radius with that of the sphere.
	//
	float newRadius = leafSphere.radius + traceContext.traceSphere.radius;



	//
	// If between cylinder up and lower bounds.
	//
	//if ( ( sphereTop.z >= pointA.z && sphereTop.z <= pointB.z ) || ( sphereBottom.z >= pointB.z && sphereBottom.z <= pointB.z ) ) {
	if ( traceContext.start.x != traceContext.end.x || traceContext.start.y != traceContext.end.y ) {
		// Height of the expanded cylinder is the height of both cylinders minus the radius of both spheres.
		float h = leafSphere.halfHeight + traceContext.traceSphere.halfHeight - newRadius;

		// If the cylinder has a height:
		if ( h > 0 ) {
			// Test for collisions between the cylinders.
			CM_TraceThroughVerticalCylinder( traceContext, sphereOffset, newRadius, h, traceContext.start, traceContext.end, leaf->contents );
		}
	}

	// Test for collisions between the spheres.
	//CM_TraceThroughSphere( traceContext, topSphere, newRadius, capsuleStartBottom, capsuleEndBottom, leaf->contents );
	//CM_TraceThroughSphere( traceContext, bottomSphere, newRadius, capsuleStartTop, capsuleEndTop, leaf->contents );

	//CM_TraceThroughSphere( traceContext, bottomSphere, newRadius, capsuleStartBottom, capsuleEndBottom, leaf->contents );
	//CM_TraceThroughSphere( traceContext, topSphere, newRadius, capsuleStartTop, capsuleEndTop, leaf->contents );
}

/**
*   @brief 
**/

void CM_TraceBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
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
	const bool hitCylinder		= bbox3_intersects_sphere( noZAbsoluteBounds, cylinderTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );
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
	const bool hitTopSphere		= bbox3_intersects_sphere( traceContext.absoluteBounds, topTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );
	
	// Calculate offset for bottom sphere, see if we hit it.
	sphere_t bottomTestSphere = intersectTestSphere;
	testSphere.offset = { 0.f, 0.f, t };
	const bool hitBottomSphere	= bbox3_intersects_sphere( traceContext.absoluteBounds, bottomTestSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON );
	
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
	leafTraceBoxHull = CM_NewBoundingBoxHull( 
		bbox3_from_center_size( traceContext.size, bbox3_center( leaf->bounds ) ),
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);

	// "Abuse", the Sphere Test Hull.
	leafTraceCapsuleHull.sphere = intersectTestSphere;

	// Change trace type.
	traceContext.traceType = CMHullType::Capsule;
	// Keep our traceSphere as is however.
	//traceContext.traceSphere = traceSphere;

	// Replace the head node we're working with.
	traceContext.headNode = leafTraceBoxHull.headNode;
	traceContext.headNodeLeaf = &leafTraceBoxHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;

	// Perform trace.
	CM_TraceThroughLeaf( traceContext, &leafTraceBoxHull.leaf );
}

/**
*   @brief 
**/
void CM_SetTraceBox( TraceContext &traceContext, const bbox3_t &traceBox, const bool boundsPointCase = false );
void CM_SetTraceSphere( TraceContext &traceContext, const bbox3_t &sphereBounds, const glm::mat4 &matTransform );
void CM_TraceBoxThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;

	/**
	*	Calculate the 'Leaf Sphere' to test the trace with.
	**/
	//const bbox3_t oldLeafBounds = oldLeafNode->bounds;
	const bbox3_t traceBounds = traceContext.boundsEpsilonOffset;
	const vec3_t traceTransformOrigin = glmvec4_to_phvec( traceContext.matTransform[3] );

	// Create the test sphere.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		//leafBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, leaf->bounds );
	}
	sphere_t traceSphere = sphere_from_size( bbox3_symmetrical( traceBounds ), vec3_zero() );	//sphere_t testSphere = CM_SphereFromBounds( leafBounds, vec3_zero() );
	traceSphere.offset = { 0.f, 0.f, 0.f };
	if ( traceContext.isTransformedTrace ) {
	//	CM_Matrix_TransformSphere( traceContext.matTransform, traceSphere );
	}
	//	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, traceSphere, !traceContext.isTransformedTrace );


	/**
	*	Ensure we are hitting this 'Leaf Sphere' before Testing any further.
	**/
	if ( !CM_TraceIntersectSphere( traceContext, traceSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, CM_RAD_EPSILON ) ) {
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
	leafTraceSphereHull.sphere = traceSphere;//= traceSphere;

	leafTraceBoxHull = CM_NewBoundingBoxHull( 
		// Bounds.
		inPlaceLeafBounds,
		// Ensure it keeps its contents.
		leaf->contents 
	);


	/**
	*	Prepare and perform the actual 'Leaf Trace' itself.
	**/
	// Change trace type.
	traceContext.traceType = CMHullType::Sphere;
	// Keep our traceSphere as is however.
	//traceContext.traceSphere = traceSphere;

	// Replace the head node we're working with.
	traceContext.headNode = leafTraceBoxHull.headNode;
	traceContext.headNodeLeaf = &leafTraceBoxHull.leaf;
	traceContext.headNodeType = CMHullType::Box;
	
	// Perform Trace.
	CM_TraceThroughLeaf( traceContext, &leafTraceBoxHull.leaf );

	// Make sure to reset trace type, headnode(leaf) and its type.
	//traceContext.traceType = oldTraceType;
	//traceContext.headNode = oldHeadNode;
	//traceContext.headNodeLeaf = oldLeafNode;
	//traceContext.headNodeType = oldHeadNodeType;

	//CM_TraceThroughLeaf( traceContext, leaf );
}

/**
*   @brief 
**/
//void CM_TraceSphereLeafThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
void CM_TraceSphereThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	/**
	*	Ensure we are hitting this bounding box before testing any further.
	**/
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
	*	Test whether the two spheres intersect/overlap.
	**/
	sphere_t traceSphere = traceContext.traceSphere;

	const vec3_t traceSphereOffsetOrigin = traceContext.traceSphere.origin + traceContext.traceSphere.offset;
	const vec3_t traceSphereStart		 = traceContext.start - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	const vec3_t traceSphereEnd			 = traceContext.end - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start - leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end - leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Total test radius. ( Seems to work. )
	//const float testRadius = ( traceSphere.radius + CM_RAD_EPSILON ) + ( leafSphere.radius + CM_RAD_EPSILON );
	const float testRadius = ( traceSphere.radius + CM_RAD_EPSILON ) + ( leafSphere.radius + CM_RAD_EPSILON );

	// Now perform sphere trace.
	CM_TraceThroughSphere( traceContext, traceSphereOffsetOrigin, traceContext.traceSphere.offset, testRadius, leafSphereStart, leafSphereEnd, leaf->contents );
	//CM_TraceThroughSphere( traceContext, leafSphereOffsetOrigin, leafSphere.offset, testRadius, traceSphereStart, traceSphereEnd, leaf->contents );
}


/***
*
*
*	Shape specific support Trace functions:
*
*
***/
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