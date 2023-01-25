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


// Implementations are at the bottom of the file.
void CM_TraceThroughVerticalCylinder( TraceContext &traceContext, const vec3_t &origin, const float radius, const float halfHeight, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents );



/**
*   @brief	Traces the traceContext headNode leaf against the passed in leaf,
*			picking a designated 'Shape Hull' trace function based on the 
*			traceContext's traceType. (Capsule, Sphere, Box, etc).
**/
void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
    if ( !( leaf->contents & traceContext.contentsMask ) ) {
        return;
    }

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
        
		// Perform a specific hull type test based on our trace type.
		// 'Capsule' brush Trace:
		if ( traceContext.headNodeType == CMHullType::Capsule ) {
			CM_TraceCapsuleThroughBrush( traceContext, brush, leaf );
		// 'Sphere' brush Trace:
		} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
			CM_TraceSphereThroughBrush( traceContext, brush, leaf );
		// Default to, 'Box' brush Trace:
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
	CM_TraceThroughSphere( traceContext, topSphere, newRadius, capsuleStartBottom, capsuleEndBottom, leaf->contents );
	CM_TraceThroughSphere( traceContext, bottomSphere, newRadius, capsuleStartTop, capsuleEndTop, leaf->contents );

	//CM_TraceThroughSphere( traceContext, bottomSphere, newRadius, capsuleStartBottom, capsuleEndBottom, leaf->contents );
	//CM_TraceThroughSphere( traceContext, topSphere, newRadius, capsuleStartTop, capsuleEndTop, leaf->contents );
}

/**
*   @brief 
**/
sphere_t CM_CapsuleSphereFromBounds( const bbox3_t &bounds, const vec3_t &origin = vec3_zero() );
void CM_CalculateSphereOffsetRotation( TraceContext &traceContext, sphere_t &sphere );
const bool bbox3_intersects_sphere( const bbox3_t &boxA, const sphere_t &sphere, const float radiusDistEpsilon );

void CM_TraceBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;


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
	CM_TraceThroughLeaf( traceContext, &boxHull.leaf );

	//// Revert back to old.
	//traceContext.traceType = oldTraceType;
	//traceContext.traceSphere = oldTraceSphere;
	//traceContext.headNodeType = oldHeadNodeType;
	//traceContext.headNode = oldHeadNode;
	//traceContext.headNodeLeaf = oldLeafNode;
}
/**
*   @brief 
**/
void CM_TraceBoxThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
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
	// Get the 'Sphere' bounds, and transform by 'matTransform' in case of transformed traces.
	bbox3_t boxBounds = traceContext.transformedBounds;
	if ( traceContext.isTransformedTrace ) {
//		boxBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, boxBounds );
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

	// Perform testing.
	CM_TraceThroughLeaf( traceContext, &boxHull.leaf );

	//// Revert back to old.
	//traceContext.traceType = oldTraceType;
	//traceContext.traceSphere = oldTraceSphere;
	//traceContext.headNodeType = oldHeadNodeType;
	//traceContext.headNode = oldHeadNode;
	//traceContext.headNodeLeaf = oldLeafNode;
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
void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius + CM_RAD_EPSILON;
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
				const vec3_t intersectDirection = /*vec3_xy( */intersection - origin;// );


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