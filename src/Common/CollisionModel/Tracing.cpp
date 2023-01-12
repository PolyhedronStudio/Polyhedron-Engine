/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Box Tracing API.
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


// TODO remove this function when it will no longer be useful
// This function is a compatibility layer

//---- SETTINGS:
// Use FLT_EPSILON for FRAC_EPSILON
#define USE_FLT_EPSILON_AS_FRAC_EPSILON

// Use a smaller DIST_EPSILON ( 0.03125 instead of 0.125 )
#define USE_SMALLER_DIST_EPSILON

// Enable Capsule Hull support
#define TRACE_ENABLE_CAPSULE_HULL

// Enable Sphere Hull support
#define TRACE_ENABLE_SPHERE_HULL

// Enable optimized point trace support. (Seems broken for capsules atm)
//#define TRACE_ENABLE_BOUNDS_POINT_CASE



// TODO: Perhaps remove? Seems they serve no need anymore.
// Do not perform the tracing process if a brush side has no plane, that would give incorrect results.
//#define TRACE_ESCAPE_TRACETOBRUSH_IF_SIDE_HAS_NO_PLANE
// Do not perform the testing process on planes if a brush side has no plane, that would give incorrect results.
//#define TRACE_ESCAPE_TESTINBRUSH_IF_SIDE_HAS_NO_PLANE
//----

//---- PRECISION CONFIG:
// 1.0 epsilon for cylinder and sphere radius offsets.
static constexpr float RAD_EPSILON = 1.0;

// 1/32 epsilon to keep floating point happy
#ifdef USE_SMALLER_DIST_EPSILON
static constexpr float DIST_EPSILON = 1.0 / 32.0; //1.0f / 32.0f;//0.125; //1.0f / 64.0f;
#else
static constexpr float DIST_EPSILON = 0.125;
#endif
//----

// Fraction Epsilon
#ifdef USE_FLT_EPSILON_AS_FRAC_EPSILON
static constexpr float FRAC_EPSILON = FLT_EPSILON; //1.0f / 1024.0f;
#else
static constexpr float FRAC_EPSILON = 1.0f / 1024.0f;
#endif


//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;
//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;

#ifdef TRACE_ENABLE_CAPSULE_HULL
	//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
	extern CapsuleHull capsuleHull;
#endif

#ifdef TRACE_ENABLE_SPHERE_HULL
	//! All round 'sphere hull' data, accessed in a few other CollisionModel files as extern.
	extern SphereHull sphereHull;
#endif


// TEMPORARILY, NEED TO ALLOW TRACETHROUGHLEAF to HAVE A 2 HULL VARIETY AND PREVENT TEMP VAR USAGE LIKE THIS
SphereHull boundsTestSphereHull;
CapsuleHull boundsTestCapsuleHull;

//
//	TODO: Remove these after having "mainstreamed/unified" our sorry, mixed up vector maths.
//	The implementations reside in /Common/CollisionModel.cpp
//
void CM_AngleVectors( const vec3_t &angles, vec3_t &forward, vec3_t &right, vec3_t &up );
void CM_AnglesToAxis( const vec3_t &angles, vec3_t axis[3]);
void CM_Matrix_TransformVector( vec3_t m[3], const vec3_t &v, vec3_t &out );
const bbox3_t CM_Matrix_TransformBounds( const glm::mat4 &matrix, const bbox3_t &bounds );
const bbox3_t CM_EntityBounds( const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds );
BoxHull CM_NewBoundingBoxHull( const bbox3_t &bounds, const int32_t contents );
SphereHull CM_NewSphereHull( const bbox3_t &bounds, const int32_t contents );
CapsuleHull CM_NewCapsuleHull( const bbox3_t &bounds, const int32_t contents );


/**
*
*
*	Declared here so we can stuff them in 1 block, easier to read less scrolling work hehe
*
*
**/
// TraceContext headNode leaf IN other leaf tests:
static void CM_TestCapsuleLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf );
static void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf );
static void CM_TestBoxLeafInSphere( TraceContext &traceContext, mleaf_t *leaf );
static void CM_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf );

// TraceContext headNode traceType shape tests IN other leaf brush:
static void CM_TestCapsuleInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
static void CM_TestSphereInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );
static void CM_TestBoundingBoxInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf );

// TraceContext headNode leaf TRACE THROUGH other leaf AND shape tests:
static void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf );

static void CM_TraceThroughVerticalCylinder( TraceContext &traceContext, const vec3_t &origin, const float radius, const float halfHeight, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
static void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents );


/**
*
*
*   Utilities - To be moved elsewhere later on.
*
*
**/
/**
*	@return	The transformed by 'matrix' recalculated 'plane'. (distance, normal, signbits and type)
**/
static CollisionPlane CM_TransformPlane( CollisionPlane *plane, const glm::mat4 &transformMatrix = ph_mat_identity() /*glm::identity< glm::mat4 >()*/ ) {
	CollisionPlane transformedPlane = *plane;

	// Scake for dist.
	const float scale = sqrtf(transformMatrix[0][0] * transformMatrix[0][0] + transformMatrix[0][1] * transformMatrix[0][1] + transformMatrix[0][2] * transformMatrix[0][2]);
	const float iscale = 1.f / scale;

	// Normal.
	const vec3_t n = transformedPlane.normal;
	const float x = (n.x * transformMatrix[0][0] + n.y * transformMatrix[1][0] + n.z * transformMatrix[2][0]) * iscale;
	const float y = (n.x * transformMatrix[0][1] + n.y * transformMatrix[1][1] + n.z * transformMatrix[2][1]) * iscale;
	const float z = (n.x * transformMatrix[0][2] + n.y * transformMatrix[1][2] + n.z * transformMatrix[2][2]) * iscale;

	// Assign new dist and normal.
	transformedPlane.dist = transformedPlane.dist * scale + (x * transformMatrix[3][0] + y * transformMatrix[3][1] + z * transformMatrix[3][2]);;
	transformedPlane.normal = { x, y, z };

	// Update plane signbits and type.
	SetPlaneSignbits( &transformedPlane );
	SetPlaneType( &transformedPlane );

	// Return new plane.
	return transformedPlane;
}

/**
*	@brief	Projects a point onto a vector.
**/
static const vec3_t CM_ProjectPointOntoVector( const vec3_t vPoint, const vec3_t vStart, const vec3_t vDir ) {
	
	// Project onto the directional vector for this segment.
	const vec3_t projectVector = vPoint - vStart;						//VectorSubtract( point, vStart, pVec );
	return vec3_fmaf( vStart, vec3_dot( projectVector, vDir ), vDir );	//VectorMA( vStart, DotProduct( pVec, vDir ), vDir, vProj );
}

/**
*	@brief	Point distance from line.
**/
static const float CM_DistanceFromLineSquared( const vec3_t &p, const vec3_t &lp1, const vec3_t &lp2, const vec3_t &dir )
{
	vec3_t t;
	int32_t j = 0;

	vec3_t proj = CM_ProjectPointOntoVector( p, lp1, dir );

	for ( j = 0; j < 3; j++ ) {
		if ( ( proj[ j ] > lp1[ j ] && proj[ j ] > lp2[ j ] ) || ( proj[ j ] < lp1[ j ] && proj[ j ] < lp2[ j ] ) )
		{
			break;
		}
	}

	if ( j < 3 ) {
		if ( fabsf( proj[ j ] - lp1[ j ] ) < fabsf( proj[ j ] - lp2[ j ] ) ) {
			//t = p - lp1; //VectorSubtract( p, lp1, t );
			return vec3_length_squared( p - lp1 );
		} else {
			//t = p - lp2; //VectorSubtract( p, lp2, t );
			return vec3_length_squared( p - lp2 );
		}

		//return vec3_length_squared( t );//VectorLengthSquared( t );
	}

	//VectorSubtract( p, proj, t );
	//return VectorLengthSquared( t );
	return vec3_length_squared( p - proj );
}



/**
*
*
*	TraceBounds:
*
*
**/
/**
*	@return	The entire absolute 'box' trace bounds in world space.
**/
const bbox3_t CM_CalculateBoxTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds ) {
	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { start,  end };

	// Add bounds epsilon.
	return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
				bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
			CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) 
																//);
	//return bbox3_expand_box(
	//			bbox3_from_points( tracePoints, 2 ), 
	//			bbox3_expand( bounds, CM_BOUNDS_EPSILON ) 
	//		);
}

/**
*	@return	The entire absolute 'Capsule' trace bounds in world space.
**/
const bbox3_t CM_CalculateCapsuleTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius ) {
	//vec3_t newStart = start;
	//vec3_t newEnd = end;

	//for ( int32_t i = 0; i < 3; i++ ) {
	//	if ( start[i] < end[i] ) {
	//		newStart[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		newEnd[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	} else {
	//		newStart[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		newEnd[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	}
	//}

	//// Prepare array for from_points method.
	//const vec3_t tracePoints[] = { newStart,  newEnd };

	//// Add bounds epsilon.
	//return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
	//			bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
	//		CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) 
		// Calc spherical offset bounds.
	//bbox3_t sphereTraceBounds;
	//for ( int32_t i = 0; i < 3; i++ ) {
	//	if ( start[i] < end[i] ) {
	//		sphereTraceBounds.mins[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		sphereTraceBounds.maxs[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	} else {
	//		sphereTraceBounds.mins[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		sphereTraceBounds.maxs[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	}
	//}
	//// Add bounds epsilon.
	//return bbox3_expand( sphereTraceBounds,	CM_BOUNDS_EPSILON );



	// LAST ONE SEEMED OFF:
	//vec3_t newStart = start;
	//vec3_t newEnd = end;

	//for ( int32_t i = 0; i < 3; i++ ) {
	//	if ( start[i] < end[i] ) {
	//		newStart[i] = start[i] /*- fabsf( sphereOffset[i] )*/ - sphereRadius;
	//		newEnd[i] = end[i] /*+ fabsf( sphereOffset[i] )*/ + sphereRadius;
	//	} else {
	//		newStart[i] = end[i] /*- fabsf( sphereOffset[i] )*/ - sphereRadius;
	//		newEnd[i] = start[i] /*+ fabsf( sphereOffset[i] )*/ + sphereRadius;
	//	}
	//}

	//// Prepare array for from_points method.
	//const vec3_t tracePoints[] = { newStart,  newEnd };

	//// Add bounds epsilon.
	//return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
	//			bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
	//		CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON )
	bbox3_t sphereTraceBounds;
	for ( int32_t i = 0; i < 3; i++ ) {
		if ( start[i] < end[i] ) {
			sphereTraceBounds.mins[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadius;
			sphereTraceBounds.maxs[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadius;
		} else {
			sphereTraceBounds.mins[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadius;
			sphereTraceBounds.maxs[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadius;
		}
	}
	// Add bounds epsilon.
	return bbox3_expand( sphereTraceBounds,	CM_BOUNDS_EPSILON );
}
/**
*	@return	The entire absolute 'Sphere' trace bounds in world space.
**/
const bbox3_t CM_CalculateSphereTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius ) {
	vec3_t newStart = start;
	vec3_t newEnd = end;

	for ( int32_t i = 0; i < 3; i++ ) {
		if ( start[i] < end[i] ) {
			newStart[i] = start[i] /*- fabsf( sphereOffset[i] )*/ - sphereRadius;
			newEnd[i] = end[i] /*+ fabsf( sphereOffset[i] )*/ + sphereRadius;
		} else {
			newStart[i] = end[i] /*- fabsf( sphereOffset[i] )*/ - sphereRadius;
			newEnd[i] = start[i] /*+ fabsf( sphereOffset[i] )*/ + sphereRadius;
		}
	}

	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { newStart,  newEnd };

	// Add bounds epsilon.
	return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
				bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
			CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) 
	//// Calc spherical offset bounds.
	//bbox3_t sphereTraceBounds;
	//for ( int32_t i = 0; i < 3; i++ ) {
	//	if ( start[i] < end[i] ) {
	//		sphereTraceBounds.mins[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		sphereTraceBounds.maxs[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	} else {
	//		sphereTraceBounds.mins[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadius;
	//		sphereTraceBounds.maxs[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadius;
	//	}
	//}
	//// Add bounds epsilon.
	//return bbox3_expand( sphereTraceBounds,	CM_BOUNDS_EPSILON );
}
/**
*	@return	True if the bounds intersected.
**/
static bool CM_TraceIntersectBounds( TraceContext &traceContext, const bbox3_t &testBounds ) {
	bbox3_t transformedTestBounds = testBounds;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		transformedTestBounds = CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	}

	return bbox3_intersects( traceContext.absoluteBounds, transformedTestBounds );
}

/**
*	@return	True if the sphere and the trace bounds intersect.
**/
/**
*	@brief	Returns true if boxA its bounds intersect the sphere of box B, false otherwise.
**/
inline const bool bbox3_intersects_sphere( const bbox3_t &boxA, const sphere_t &sphere, const float radiusDistEpsilon ) {
	//if ( boxA.mins.x >= boxB.maxs.x || boxA.mins.y >= boxB.maxs.y || boxA.mins.z >= boxB.maxs.z ) {
	//	return false;
	//}

	//if ( boxA.maxs.x <= boxB.mins.x || boxA.maxs.y <= boxB.mins.y || boxA.maxs.z <= boxB.mins.z ) {
	//	return false;
	//}
	const float testRadius = flt_square( sphere.radius );
	const vec3_t sphereCenter = sphere.offset;

	float deltaMin = 0;

	for( int32_t i = 0; i < 3; i++ ) {
		if( sphereCenter[i] < boxA.mins[i] ) {
			deltaMin += flt_square( sphereCenter[i] - ( boxA.mins[i] - radiusDistEpsilon ) ); 
		} else if( sphereCenter[i] > boxA.maxs[i] ) {
			deltaMin += flt_square( sphereCenter[i] - ( boxA.maxs[i] + radiusDistEpsilon ) );     
		}
	}

	if( deltaMin <= testRadius) {
		return true;
	}

	return false;
}
static bool CM_TraceIntersectSphere( TraceContext &traceContext, const sphere_t &sphere, const float radiusDistEpsilon = 0.f ) {
	sphere_t transformedTestSphere = sphere;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matTransform * phvec_to_glmvec4( transformedTestSphere.offset ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.offset = glmvec3_to_phvec( v3TransformedOffset );
		//transformedTestBounds = CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	}

	return bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTestSphere, radiusDistEpsilon );
}



/**
*
*
*	Sphere CollisionModel Utilities:
*
*	TODO: Might move elsewhere.
*
*
**/
/**
*	@brief	Calculates a spherical collision shape from a 'size' vector, for use with sphere/capsule hull tracing.
**/
sphere_t CM_SphereFromSize( const vec3_t &size, const vec3_t offset = vec3_zero() ) {
	//float halfWidth = symm

	//const vec3_t origin = vec3_scale( newBB.mins + newBB.maxs, 0.5f );
	//const float sphereRadius = vec3_dot( newBB.maxs - origin );

	//	//const float radius = size.x;//( size.x > size.z ? size.z : size.x );
	//return {
	//	.radius = sphereRadius,
	//	.halfHeight = size.z,
	//	.halfWidth = size.x,
	////	//.offset = vec3_t{ offset.x, offset.y, offset.z }
	//	.offset = vec3_t{ offset.x, offset.y, offset.z + sphereRadius }
	//};

	// Calculate symmetrical size box, for acquiring halfwidth and height
	const bbox3_t newBB = bbox3_from_center_size( size, offset );

	const vec3_t symmetricSize = bbox3_symmetrical( newBB );
	const float halfHeight= symmetricSize.z;
	const float halfWidth = symmetricSize.x;

	const float radius = ( halfWidth > halfHeight ? halfHeight : halfWidth );

	return {
		.radius = radius,
		.halfHeight = halfHeight,
		.halfWidth = halfWidth,
		//.offset = vec3_t{ offset.x, offset.y, offset.z }
		.offset = vec3_t{ offset.x, offset.y, offset.z - radius }
	};
}

/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with sphere/capsule hull tracing.
**/
sphere_t CM_SphereFromBounds( const bbox3_t &bounds, const vec3_t offset = vec3_zero() ) {

	const vec3_t boxOffset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

	const vec3_t symmetricSize = bbox3_symmetrical( bounds );
	const float halfHeight= symmetricSize.z;
	const float halfWidth = symmetricSize.x;

	//const float radius = ( halfWidth > halfHeight ? halfHeight : halfWidth );
	const float radius = bbox3_radius( bounds );

	const float offsetRadius = ( halfWidth > halfHeight ? halfHeight : halfWidth );
	return {
		.radius = radius,
		.halfHeight = halfHeight,
		.halfWidth = halfWidth,
		//.offset = vec3_t{ offset.x, offset.y, offset.z }
		.offset = offset - boxOffset - vec3_t{ 0.f, 0.f, offsetRadius }//vec3_t{ offset.x, offset.y, offset.z }
	};
}


/**
*
*
*
*	TraceContext traceType Switch functions:
*
*
*
**/
static void CM_TraceContext_TraceTypeToCapsule() {

}
static void CM_TraceContext_TraceTypeToSphere() {

}
static void CM_TraceContext_TraceTypeToBox() {

}



/**
*
*
*
*	TraceContext traceType based 'Shape Hull' in 'Leaf Shape Hull' Tests:
*
*
*
**/
/**
*   @brief	Tests whether the traceContext traceType capsule against a temporary leaf capsule.
**/
static void CM_TestCapsuleLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
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
static void CM_TestBoxLeafInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
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
	bbox3_t boxBounds = traceContext.bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matTransform, boxBounds );
	}
	// Get the 'Capsule' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}
	
	/**
	*	Create temporary Hulls for the Capsule and the Box.
	**/
	// Capsule:
	CapsuleHull capsuleHull = CM_NewCapsuleHull( 
		boxBounds, oldLeafNode->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
	// Box:
	BoxHull boxHull = CM_NewBoundingBoxHull( 
		leafBounds, 
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);

	/**
	*	Adjust the trace context with the exchanged hulls and perform a Test in Leaf.
	**/
	traceContext.traceType = CMHullType::Capsule;
	//traceContext.traceSphere = capsuleHull.capsule;
	traceContext.headNode = capsuleHull.headNode;
	traceContext.headNodeLeaf = &capsuleHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;

	// Store Capsule hull thingy.
	boundsTestCapsuleHull = capsuleHull;

	// Perform testing.
	CM_TestInLeaf( traceContext, &capsuleHull.leaf );
}
/**
*   @brief	Exchanges the tracing type with 'Sphere' tracing against a temporary box hull,
*			to then proceed performing a leaf test on it.
**/
static void CM_TestBoxLeafInSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;


	/**
	*	Acquire the bounds, and in case of a transformed trace, exchange their 
	*	transformations by the inverse/transform matrices.
	**/
	// Get the 'Sphere' bounds, and transform by 'matTransform' in case of transformed traces.
	bbox3_t boxBounds = traceContext.bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matTransform, boxBounds );
	}
	// Get the 'Sphere' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}
	
	/**
	*	Create temporary Hulls for the Sphere and the Box.
	**/
	// Sphere:
	SphereHull sphereHull = CM_NewSphereHull( 
		boxBounds, oldLeafNode->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
	// Box:
	BoxHull boxHull = CM_NewBoundingBoxHull( 
		leafBounds, 
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);

	/**
	*	Adjust the trace context with the exchanged hulls and perform a Test in Leaf.
	**/
	traceContext.traceType = CMHullType::Sphere;
	//traceContext.traceSphere = sphereHull.sphere;
	traceContext.headNode = sphereHull.headNode;
	traceContext.headNodeLeaf = &sphereHull.leaf;
	traceContext.headNodeType = CMHullType::Sphere;

	// Store sphere hull thingy.
	boundsTestSphereHull = sphereHull;

	// Perform testing.
	CM_TestInLeaf( traceContext, &boxHull.leaf );
}

/**
*   @brief	Tests the traceContext headNode leaf against the passed in leaf,
*			picking a designated 'Shape Hull' test function based on the 
*			traceContext's traceType. (Capsule, Sphere, Box, etc).
**/
static void CM_TestInLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
*
*
*	TraceContext traceType based 'Shape Hull' in 'Leaf Shape Hull' traces:
*
*
**/
/**
*   @brief 
**/
static void CM_TraceCapsuleThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
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
static void CM_TraceBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
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
	bbox3_t boxBounds = traceContext.bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matTransform, boxBounds );
	}
	// Get the 'Capsule' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}
	
	/**
	*	Create temporary Hulls for the Capsule and the Box.
	**/
	// Capsule:
	CapsuleHull capsuleHull = CM_NewCapsuleHull( 
		boxBounds, oldLeafNode->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
	// Box:
	BoxHull boxHull = CM_NewBoundingBoxHull( 
		leafBounds, 
		leaf->contents // Replacing the leaf with the box temporarily, so use its contents.
	);
	
	if ( !CM_TraceIntersectSphere( traceContext, capsuleHull.sphere ) ) {
		return;
	}

	/**
	*	Adjust the trace context with the exchanged hulls and perform a Test in Leaf.
	**/
	traceContext.traceType = CMHullType::Capsule;
	//traceContext.traceSphere = capsuleHull.capsule;
	traceContext.headNode = capsuleHull.headNode;
	traceContext.headNodeLeaf = &capsuleHull.leaf;
	traceContext.headNodeType = CMHullType::Capsule;

	// Store Capsule hull thingy.
	boundsTestCapsuleHull = capsuleHull;

	// Perform testing.
	CM_TraceThroughLeaf( traceContext, &capsuleHull.leaf );
}
/**
*   @brief 
**/
static void CM_TraceBoxThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
	// Old head and leaf node and trace type.
	int32_t oldTraceType = traceContext.traceType;
	int32_t oldHeadNodeType = traceContext.headNodeType;
	mnode_t *oldHeadNode = traceContext.headNode;
	mleaf_t *oldLeafNode = traceContext.headNodeLeaf;
	
	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}

	/**
	*	Acquire the bounds, and in case of a transformed trace, exchange their 
	*	transformations by the inverse/transform matrices.
	**/
	// Get the 'Sphere' bounds, and transform by 'matTransform' in case of transformed traces.
	bbox3_t boxBounds = traceContext.boundsEpsilonOffset;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matTransform, boxBounds );
	}
	// Get the 'Sphere' bounds, and transform by 'matInvTransform' in case of transformed traces.
	bbox3_t leafBounds = leaf->bounds;
	if ( traceContext.isTransformedTrace ) {
		CM_Matrix_TransformBounds( traceContext.matInvTransform, leafBounds );
	}
	
	/**
	*	Create temporary Hulls for the Sphere and the Box.
	**/
	// Sphere:
	SphereHull sphereHull = CM_NewSphereHull( 
		boxBounds, oldLeafNode->contents // We're replacing the old node with this one temporarily, so use its contents.
	);
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
	CM_TraceThroughLeaf( traceContext, &boxHull.leaf );

	//sphere_t leafSphere = sphereHull.sphere;
	//vec3_t sphereOffset = leafSphere.offset;

	//float offsetZ = leafSphere.offset.z;
	//vec3_t leafSphereOffset = leafSphere.offset; //{ 0.f, 0.f, offsetZ };

	//// Test Radius: See if the spheres overlap
	//float testRadius = ( traceContext.traceSphere.radius + leafSphere.radius ) * ( traceContext.traceSphere.radius + leafSphere.radius );

	////
	//// Top Sphere.
	////
	////vec3_t sphereTop = sphereOffset;//sphereOffset + vec3_t{ 0.f, 0.f, offsetZ };
	////sphereTop.z += offsetZ;
	////vec3_t sphereBottom = sphereOffset;//sphereOffset + vec3_t{ 0.f, 0.f, offsetZ };
	////sphereTop.z -= offsetZ;

	//float newRadius = leafSphere.radius + traceContext.traceSphere.radius;

	//// Start/End 

	//// Start/End for leafSphere.
	//vec3_t startBottom = traceContext.start - leafSphere.offset;
	//vec3_t endBottom = traceContext.end + leafSphere.offset;

	//CM_TraceThroughSphere( traceContext, leafSphere.offset, newRadius, startBottom, endBottom, leaf->contents );
	////CM_TraceThroughSphere( traceContext, bottomSphere, newRadius, capsuleStartTop, capsuleEndTop, leaf->contents );
}



/**
*
*
*	TraceContext traceType based 'Hull Shape' in 'Leaf Brush' tests:
*
*
**/
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
static void CM_TestCapsuleInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	if ( traceContext.headNodeType != CMHullType::Capsule ) {
		return;
	}

	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestCapsuleHull.sphere, RAD_EPSILON ) ) {
		return;
	}

	mbrushside_t *brushSide = brush->firstbrushside;

	//if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
	//	return;
	//}

	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// Adjust the plane distance appropriately for radius.
		//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
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

		vec3_t startPoint = traceContext.start;
		//vec3_t endPoint = traceContext.end;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			//endPoint = traceContext.end - traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			//endPoint = traceContext.end + traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal ) - dist;
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - dist;

		// if completely in front of face, no intersection
		if ( d1 > 0 )
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
static void CM_TestSphereInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	//if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere )) {
	//	return;
	//}
	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere, RAD_EPSILON ) ) {
		return;
	}

	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {
		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}
		// Calculate dist.
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.radius;
		}

		
		// Find the closest point on the capsule to the plane
		//vec3_t startPoint = traceContext.start;
		//vec3_t endPoint = traceContext.end;
	

		// Find the closest point on the capsule to the plane
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		//float t = vec3_dot( traceContext.traceSphere.offset, transformedPlane.normal );
		vec3_t startPoint;
		//vec3_t endPoint;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			//endPoint = traceContext.end + traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			//endPoint = traceContext.end - traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
		const float d1 = vec3_dot( startPoint, transformedPlane.normal) - ( transformedPlane.dist - traceContext.traceSphere.radius );
		//const float d2 = vec3_dot( endPoint, transformedPlane.normal ) - ( transformedPlane.dist + traceContext.traceSphere.radius );

		// if completely in front of face, no intersection
		if ( d1 > 0 )
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
static void CM_TestBoundingBoxInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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
			dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
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



/**
*	@brief	Get the first intersection of the ray with the sphere.
**/
static void CM_TraceThroughSphere( TraceContext &traceContext, const vec3_t &origin, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius + RAD_EPSILON;
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
static void CM_TraceThroughVerticalCylinder( TraceContext &traceContext, const vec3_t &origin, const float radius, const float halfHeight, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 ) {
	
	// Squared radius
	const float squaredRadius = flt_square( radius );
	
	// Dist Epsilon radius & squared radius.
	const float radiusDistEpsilon = radius + DIST_EPSILON;
	const float squaredRadiusDistEpsilon = flt_square( radiusDistEpsilon );

	// Radius Epsilon radius & squared radius.
	const float radiusRadEpsilon = radius + RAD_EPSILON;
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



/**
*
*
*	TraceContext traceType based 'Hull Shape' through ' Leaf Brush' tracing:
*
*
**/
/**
*   @brief Performs a 'Capsule Hull' based trace by clipping the hull to all leaf brushes, storing the final
*	trace clipping results.
**/
static void CM_TraceCapsuleThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }
	
	// Our headnode type is sphere, we can cast
	//if ( !CM_TraceIntersectSphere( traceContext, boundsTestCapsuleHull.sphere, RAD_EPSILON ) ) {
	//	return;
	//}
	if ( traceContext.headNodeType != CMHullType::Capsule ) {
		return;
	}

	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestCapsuleHull.sphere, RAD_EPSILON ) ) {
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
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			// Adjust the plane distance appropriately for radius.
			//const float dist = transformedPlane.dist + traceContext.traceSphere.radius;
			dist = transformedPlane.dist + traceContext.traceSphere.radius;
		}

		// Find the closest point on the capsule to the plane
		//float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );
		float t = vec3_dot( transformedPlane.normal, traceContext.traceSphere.offset );

		vec3_t startPoint = traceContext.start;
		vec3_t endPoint = traceContext.end;
		if ( t > 0 ) {
			startPoint = traceContext.start - traceContext.traceSphere.offset;
			endPoint = traceContext.end - traceContext.traceSphere.offset;
		} else {
			startPoint = traceContext.start + traceContext.traceSphere.offset;
			endPoint = traceContext.end + traceContext.traceSphere.offset;
		}
	
		// Calculate trace line.
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
static void CM_TraceSphereThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }

	if ( traceContext.headNodeType != CMHullType::Sphere) {
		return;
	}

	// Our headnode type is sphere, we can cast
	if ( !CM_TraceIntersectSphere( traceContext, boundsTestSphereHull.sphere, RAD_EPSILON ) ) {
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
static void CM_TraceBoxThroughBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
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

/**
*   @brief	Traces the traceContext headNode leaf against the passed in leaf,
*			picking a designated 'Shape Hull' trace function based on the 
*			traceContext's traceType. (Capsule, Sphere, Box, etc).
**/
static void CM_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
*	'BSP World Tree' Sweep tracing:
*
*
**/
/**
*   @brief 
**/
static void CM_RecursiveTraceThroughTree( TraceContext &traceContext, mnode_t *node, float p1f, float p2f, const vec3_t &p1, const vec3_t &p2 ) {
    if (!node) {
		return;
	}
recheck:
	// If plane is NULL, we are in a leaf node
    CollisionPlane *plane = node->plane;

//#ifdef TRACE_PLANE_SIDE_FIX
//	// If true, we already hit something nearer.
//    if (traceContext.realFraction <= p1f) {
//        return;
//    }
//#else
//	// If true, we already hit something nearer.
	if (traceContext.traceResult.fraction <= p1f) {
		return;
	}
//#endif

	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
	CollisionPlane transformedPlane = *plane;
	//if ( traceContext.isTransformedTrace ) { TransformPlane }

    //
    // Find the point distances to the seperating plane and the offset for the size of the box.
    //
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;

	// Axial planes.
    if ( transformedPlane.type < 3 ) {
        t1 = p1[ transformedPlane.type ] - transformedPlane.dist;
        t2 = p2[ transformedPlane.type ] - transformedPlane.dist;
        offset = traceContext.extents[ transformedPlane.type ];
	// Non axial planes use dot product testing.
    } else {
        t1 = vec3_dot( transformedPlane.normal, p1 ) - transformedPlane.dist;
        t2 = vec3_dot( transformedPlane.normal, p2 ) - transformedPlane.dist;
        if ( traceContext.isPoint ) {
			offset = 0;
        } else {
			offset = (fabs( traceContext.extents.x * transformedPlane.normal.x ) +
                     fabs( traceContext.extents.y * transformedPlane.normal.y ) +
                     fabs( traceContext.extents.z * transformedPlane.normal.z )) * 3;
		}
    }

    // See which sides we need to consider.
    if ( t1 >= offset && t2 >= offset ) { //if ( t1 >= offset + 1.f && t2 >= offset + 1.f ) {
		// Get child node.
		node = node->children[0];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// TODO: First checking for any intersection might be a good thing to do here.
			CM_TraceThroughLeaf( traceContext, leafNode );
			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }
	if ( t1 < -offset && t2 < -offset ) { //if ( t1 < -offset /*- 1.f */&& t2 < -offset /*- 1.f */) {
		// Get child node.
		node = node->children[1];
		// See if we hit a leaf node, and trace through it if we did before exiting our traversal.
		if ( !node->plane ) {
			mleaf_t *leafNode = (mleaf_t *)node;
			// TODO: First checking for any intersection might be a good thing to do here.
			CM_TraceThroughLeaf( traceContext, leafNode );
			return;
		}
		// Perform another check traversing even further down the BSP tree.
		goto recheck;
    }

    // Dp NOT put the crosspoint DIST_EPSILON pixels on the near side
    int32_t side = 0;
    float idist = 0.f;
    float fractionA = 0.f;
    float fractionB = 0.f;
    if (t1 < t2) {
        idist = 1.0f / (t1 - t2);
        side = 1;
        fractionB = (t1 + offset /*+ DIST_EPSILON*/) * idist;
        fractionA = (t1 - offset /*+ DIST_EPSILON*/) * idist;
    } else if (t1 > t2) {
        idist = 1.0f / (t1 - t2);
        side = 0;
		fractionB = (t1 - offset /*- DIST_EPSILON*/) * idist;
        fractionA = (t1 + offset /*+ DIST_EPSILON*/) * idist;
    } else {
        side = 0;
        fractionA = 1.f;
        fractionB = 0.f;
    }

	// Move up to the node in case we can potentially hit it.
	if ( p1f < traceContext.realFraction ) {
		fractionA = Clampf( fractionA, 0.f, 1.f );

		float midf = p1f + ( p2f - p1f ) * fractionA;
		vec3_t mid = vec3_mix( p1, p2, fractionA );

		mnode_t *childNode = node->children[side];
		if ( !childNode->plane ) {
			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveTraceThroughTree( traceContext, childNode, p1f, midf, p1, mid );
		}
	}

    // Go past the node.
	fractionB = Clampf( fractionB, 0.f, 1.f );
    const float midf = p1f + ( p2f - p1f ) * fractionB;

	if ( midf < traceContext.realFraction ) {
		const vec3_t mid = vec3_mix( p1, p2, fractionB );

		mnode_t *childNode = node->children[side ^ 1];
		if ( !childNode->plane ) {
			CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		} else {
			CM_RecursiveTraceThroughTree( traceContext, childNode, midf, p2f, mid, p2 );
		}
	}
}



/**
*
*
*	Trace Callbacks:
*
*	Each callback prepares a specific traceContext to pass on to the internal trace routine.
*
*
**/
/**
*   @brief	The actual main trace implementation, operates on a TraceContext prepared and supplied by helper trace functions.
**/
static const TraceResult _CM_Trace( TraceContext &traceContext ) {
	// Determine if we're tracing against box hulls, or the actual world BSP brushes..
	//const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
	//						 && traceContext.headNode != capsuleHull.headNode 
	//						 && traceContext.headNode != sphereHull.headNode 
	//						 && traceContext.headNode != octagonHull.headNode );
	const bool worldTrace = !( traceContext.traceType > CMHullType::World );
	
	// Whether to use specific position point case testing:
	const bool positionPointCase = vec3_equal( traceContext.start, traceContext.end );
	// Whether to use specifics point bounds case testing:
#ifdef TRACE_ENABLE_BOUNDS_POINT_CASE
	const bool boundsPointCase = vec3_equal( traceContext.bounds.mins, vec3_zero() ) && vec3_equal( traceContext.bounds.maxs, vec3_zero() );
#else
	const bool boundsPointCase = false;
#endif
	/**
	*	For multi-check avoidance.
	**/
    traceContext.collisionModel->checkCount++;
	traceContext.checkCount = traceContext.collisionModel->checkCount;

	/**
	*	Figure out the leaf/node and get its boundingbox to test for clipping against with.
	**/
    if ( !traceContext.headNode ) {
        return traceContext.traceResult;
    }
	
	/**
	*	Determine if the headNode is our BSP world, or one of our specific hull types
	*	set to trace against. (Usually this means we're testing against an entity.)
	**/
	mleaf_t *nodeLeaf = (mleaf_t*)&traceContext.headNode;
	if ( !worldTrace ) {
		nodeLeaf = traceContext.headNodeLeaf;
		if ( !nodeLeaf ) {
			return traceContext.traceResult;
		}
	}

    /**
    *	Calculate Trace Size Extents:
	*
	*	Expand our bounds by CM_BOUNDS_EPSILON offset and transform the bounds by our inverse matrix 
	*	when dealing with a transformedTrace, before calculating its symmetrical size.
    **/
	// Calculate PointTrace Case Size:
	if ( boundsPointCase ) {
		traceContext.isPoint = true;
		traceContext.extents = vec3_zero();
	// Calculate BoundsTrace Case Size:
	} else {
		// Not a point trace.
		traceContext.isPoint = false;
		// Calculate the bounds with our epsilon offset.
		traceContext.boundsEpsilonOffset = bbox3_expand( traceContext.bounds, CM_BOUNDS_EPSILON );

		// Transformed Path:
		if ( traceContext.isTransformedTrace ) {
			traceContext.transformedBounds = CM_Matrix_TransformBounds( traceContext.matTransform, traceContext.boundsEpsilonOffset );
			// NOTE: If the uncommented variety fails somehow, revert to this.
			//traceContext.extents = bbox3_symmetrical( bbox3_expand( CM_Matrix_TransformBounds( traceContext.matInvTransform, traceContext.bounds ), CM_BOUNDS_EPSILON ) );
		
			// The size of the transformedBounds box.
			traceContext.size = bbox3_size( traceContext.transformedBounds );
			// Symmetrical extents of our transformedBounds box.
			traceContext.extents = bbox3_symmetrical( traceContext.transformedBounds );
		// Non-Transformed Path:
		} else {	
			// Transformed bounds, same as regular bounds in this case:
			traceContext.transformedBounds = traceContext.boundsEpsilonOffset;

			// The size of the transformedBounds box.
			traceContext.size = bbox3_size( traceContext.transformedBounds );
			// Symmetrical extents of our boundsEpislonOffset box.
			traceContext.extents = bbox3_symmetrical( traceContext.boundsEpsilonOffset );
		}
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( traceContext.bounds, traceContext.offsets );



	/**
	*	Take care of calculating shapes depending on our trace type.
	**/
	// Calculate needed sphere radius, halfheight, and offsets for a Capsule trace.
	if ( traceContext.traceType == CMHullType::Capsule ) {
		// Setup our context
		traceContext.traceSphere = CM_SphereFromBounds( traceContext.boundsEpsilonOffset, bbox3_center( traceContext.boundsEpsilonOffset ) );
		
		// Rotated sphere offset for capsule
		if ( traceContext.isTransformedTrace ) {
			// GLM:
			glm::vec4 vOffset = phvec_to_glmvec4( traceContext.traceSphere.offset, 1 );
			vOffset = traceContext.matTransform * vOffset;
			glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
			// END OF GLM:

			traceContext.traceSphere.offset = glmvec3_to_phvec( v3Offset );
		}
	// Calculate needed sphere radius, halfheight, and offsets for a Capsule trace.
	} else if ( traceContext.traceType == CMHullType::Sphere ) {
		// Setup our context
		traceContext.traceSphere = CM_SphereFromBounds( traceContext.boundsEpsilonOffset, bbox3_center( traceContext.boundsEpsilonOffset ) );
		
		// Rotated sphere offset for capsule
		if ( traceContext.isTransformedTrace ) {
			// GLM:
			glm::vec4 vOffset = phvec_to_glmvec4( traceContext.traceSphere.offset, 1 );
			vOffset = traceContext.matTransform * vOffset;
			glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
			// END OF GLM:

			traceContext.traceSphere.offset = glmvec3_to_phvec( v3Offset );
		}

	}

	/**
	*	Calculate Absolute Tracing Bounds.
	**/
	// 'Capsule'-specific Trace Bounds:
	if ( traceContext.traceType == CMHullType::Capsule ) {
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		traceContext.absoluteBounds = CM_CalculateCapsuleTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, traceContext.traceSphere.offset, traceContext.traceSphere.radius );
	// 'Sphere'-specific Trace Bounds:
	} else if ( traceContext.traceType == CMHullType::Sphere ) {
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		traceContext.absoluteBounds = CM_CalculateSphereTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, traceContext.traceSphere.offset, traceContext.traceSphere.radius );
	// 'Box' Default Trace Bounds:
	} else {
		traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
	}

	/**
	*	Determine whether said nodeLeaf bounds intersected to our trace bounds.
	**/
	//const bool leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
	bool leafBoundsIntersected = false;
	// 'Capsule'-specific Trace Bounds:
	if ( traceContext.traceType == CMHullType::Capsule ) {
		leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
	// 'Sphere'-specific Trace Bounds:
	} else if ( traceContext.traceType == CMHullType::Sphere ) {
		leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
	// 'Box' Default Trace Bounds:
	} else {
		leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
	}

	/**
	*	Check for position test special case.
	**/
	if ( positionPointCase ) {
		// Transform the absoluteBounds using matInvTransform if we're dealing with a transformed trace.
		bbox3_t absoluteBounds = traceContext.absoluteBounds;
		if ( traceContext.isTransformedTrace ) {
			absoluteBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, absoluteBounds );
		}

		// Perform world leaf test.
        if (worldTrace) {
			mleaf_t *leafs[1024];
			int32_t topNode = 0;

			const int32_t numberOfLeafs = CM_BoxLeafs_headnode( absoluteBounds, leafs, Q_COUNTOF(leafs), traceContext.headNode, nullptr );
			for ( int32_t i = 0; i < numberOfLeafs; i++ ) {
				// Make sure that the contents of said leaf are compatible with the mask we trace for.
				// Also ensure it has a valid plane to work with.
				if ( ( leafs[ i ]->contents & traceContext.contentsMask ) ) {
					const bbox3_t leafBounds = leafs[ i ] ->bounds;
					
					if ( CM_TraceIntersectBounds( traceContext, leafBounds ) ) {
						CM_TestInLeaf( traceContext, leafs[ i ] );
					}
				}

				// Break out if we're in an allSolid.
				if ( traceContext.traceResult.allSolid ) {
					break;
				}
			}
		// Perform a node leaf test.
		//} else {
		} else if ( leafBoundsIntersected ) {
			// 'Capsule' Testing:
			if ( traceContext.headNodeType == CMHullType::Capsule ) {
				// 'Capsule' in 'Capsule':
				if ( traceContext.traceType == CMHullType::Capsule ) {
					CM_TestCapsuleLeafInCapsule( traceContext, nodeLeaf );
				// 'Sphere' in 'Capsule'
				//if ( traceContext.traceType == CMHullType::Sphere ) {
					//CM_TestSphereInCapsule( traceContext, nodeLeaf );
				//}
				// 'Box' in 'Capsule':
				} else {
					CM_TestBoxLeafInCapsule( traceContext, nodeLeaf );
				}
			// 'Sphere' Testing:
			} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
				// 'Sphere' in 'Sphere'.
				//if ( traceContext.traceType == CMHullType::Sphere ) {
					//CM_TestSphereInSphere( traceContext, nodeLeaf );
				// 'Capsule' in 'Sphere'.
				//} else if ( traceContext.traceType == CMHullType::Capsule) {
					//CM_TestCapsuleInSphere( traceContext, nodeLeaf );
				// Default to: 'Bounding Box' in 'Sphere'.
				//} else {
					CM_TestBoxLeafInSphere( traceContext, nodeLeaf );
				//}
			// Default 'Box' Testing:
			} else {

				// Test whether our trace box intersects with the entity leaf node.
				CM_TestInLeaf( traceContext, nodeLeaf );
			}
		}

        traceContext.traceResult.endPosition = traceContext.start;
        return traceContext.traceResult;
	}

    /**
    *	Trace to leaf or perform a general sweeping through world.
    **/
	if ( worldTrace ) {
		// Transformed path.
		if ( traceContext.isTransformedTrace ) {
			glm::vec4 transformedStart = traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.start ), 1.0 );
			glm::vec4 transformedEnd =  traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.end ), 1.0 );
			CM_RecursiveTraceThroughTree( traceContext, traceContext.headNode, 0, 1, glmvec3_to_phvec( transformedStart ), glmvec3_to_phvec( transformedEnd ) );
		// Non transformed path.
		} else {	
			CM_RecursiveTraceThroughTree( traceContext, traceContext.headNode, 0, 1, traceContext.start, traceContext.end );
		}
	//} else {
	} else if ( leafBoundsIntersected ) {
		// 'Capsule' Tracing:
		if ( traceContext.headNodeType == CMHullType::Capsule ) {
			// 'Capsule' in 'Capsule':
			if ( traceContext.traceType == CMHullType::Capsule ) {
				CM_TraceCapsuleThroughCapsule( traceContext, nodeLeaf );
			// 'Box' in 'Capsule':
			} else {
				CM_TraceBoxThroughCapsule( traceContext, nodeLeaf );
			}
		} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
			//// 'Sphere' in 'Sphere'.
			//if ( traceContext.traceType == CMHullType::Sphere ) {
			//	//CM_TraceSphereThroughSphere( traceContext, nodeLeaf );
			//// 'Capsule' in 'Sphere'.
			//} else if ( traceContext.traceType == CMHullType::Capsule ) {
			//	//CM_TraceCapsuleThroughSphere( traceContext, nodeLeaf );
			//// Default to: 'Bounding Box' in 'Sphere'.
			//} else {
				CM_TraceBoxThroughSphere( traceContext, nodeLeaf );
			//}
		// Default 'Box' Tracing:
		} else {
			// Test whether our trace box intersects with the entity leaf node.
			CM_TraceThroughLeaf( traceContext, nodeLeaf );
		}
	}

    /**
    *	Last of all, calculate the exact trace end position based on the traceResult's clip fraction.
    **/
	traceContext.traceResult.fraction = Clampf( traceContext.traceResult.fraction, 0.f, 1.f );
	//traceContext.traceResult.fraction = Maxf( 0.f, traceContext.traceResult.fraction );
	if ( traceContext.traceResult.fraction == 0.f ) {
		traceContext.traceResult.endPosition = traceContext.start;
	} else if ( traceContext.traceResult.fraction == 1.f ) {
		traceContext.traceResult.endPosition = traceContext.end;
	} else {
		traceContext.traceResult.endPosition = vec3_mix( traceContext.start, traceContext.end, traceContext.traceResult.fraction );
	}

	// Return finalized trace result.
	return traceContext.traceResult;
}

/**
*   @brief  General box tracing routine.
**/
const TraceResult CM_BoxTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, const int32_t brushContentsMask = 0 ) {
	// Prepare trace context.
	TraceContext traceContext;

	// User input.
	traceContext.collisionModel = collisionModel;
	traceContext.headNode = headNode;
	traceContext.contentsMask = brushContentsMask;
	traceContext.matTransform = ph_mat_identity(); //glm::identity< glm::mat4 >();
	traceContext.matInvTransform = ph_mat_identity(); //glm::identity< glm::mat4 >();
	traceContext.start = start;
	traceContext.end = end;
	traceContext.bounds = bounds;
	// AbsoluteBounds: Now calculated in _CM_Trace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBounds( start, end, bounds );

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
		traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
		traceContext.traceType = CMHullType::Sphere;
		traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
		traceContext.traceType = CMHullType::Octagon;
		traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
		traceContext.traceType = CMHullType::Box;
		traceContext.headNodeType = CMHullType::Box;
		traceContext.headNodeLeaf = (mleaf_t*)&boxHull.leaf;
	// Resort to world 'sweeping'.
	} else { //if ( headNode == collisionModel->cache->nodes ) {
		traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Non Transformed Trace.
	traceContext.isTransformedTrace = false;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Trace( traceContext );
}

/**
*   @brief  Same as CM_TraceBox but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedBoxTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, const int32_t brushContentsMask = 0, const glm::mat4 &matTransform = ph_mat_identity(), const glm::mat4 &matInvTransform = ph_mat_identity() ) {
	// Prepare trace context.
	TraceContext traceContext;

	// User input.
	traceContext.collisionModel = collisionModel;
	traceContext.headNode = headNode;
	traceContext.contentsMask = brushContentsMask;
	traceContext.matTransform = matTransform;
	traceContext.matInvTransform = matInvTransform;
	traceContext.start = start;
	traceContext.end = end;
	traceContext.bounds = bounds;
	// AbsoluteBounds: Now calculated in _CM_Trace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBounds( start, end, bounds );

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
		traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
		traceContext.traceType = CMHullType::Sphere;
		traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
		traceContext.traceType = CMHullType::Octagon;
		traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
		traceContext.traceType = CMHullType::Box;
		traceContext.headNodeType = CMHullType::Box;
		traceContext.headNodeLeaf = (mleaf_t*)&boxHull.leaf;
	// Resort to world 'sweeping'.
	} else { //if ( headNode == collisionModel->cache->nodes ) {
		traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Transformed Trace.
	traceContext.isTransformedTrace = true;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Trace( traceContext );
}



/**
*
*
*
*	'Sphere' Tracing:
*
*
**/
/**
*   @brief	The actual main 'Sphere' trace implementation, operates on a TraceContext prepared and supplied by helper trace functions.
**/
static const TraceResult _CM_Sphere_Trace( TraceContext &traceContext ) {
	// Determine if we're tracing against box hulls, or the actual worldBSP brushes..
	// We base it on the headNode, not the traceType, since it will be Spherical either way.
	const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
							 && traceContext.headNode != capsuleHull.headNode 
							 && traceContext.headNode != sphereHull.headNode 
							 && traceContext.headNode != octagonHull.headNode );
	//const bool worldTrace = !( traceContext.traceType > CMHullType::World );
	
	// Whether to use specific position point case testing:
	const bool positionPointCase = vec3_equal( traceContext.start, traceContext.end );
	// Whether to use specifics point bounds case testing:
#ifdef TRACE_ENABLE_BOUNDS_POINT_CASE
	const bool boundsPointCase = vec3_equal( traceContext.bounds.mins, vec3_zero() ) && vec3_equal( traceContext.bounds.maxs, vec3_zero() );
#else
	const bool boundsPointCase = false;
#endif
	/**
	*	For multi-check avoidance.
	**/
    traceContext.collisionModel->checkCount++;
	traceContext.checkCount = traceContext.collisionModel->checkCount;

	/**
	*	Figure out the leaf/node and get its boundingbox to test for clipping against with.
	**/
    if ( !traceContext.headNode ) {
        return traceContext.traceResult;
    }
	
	/**
	*	Determine if the headNode is our BSP world, or one of our specific hull types
	*	set to trace against. (Usually this means we're testing against an entity.)
	**/
	mleaf_t *nodeLeaf = (mleaf_t*)&traceContext.headNode;
	if ( !worldTrace ) {
		nodeLeaf = traceContext.headNodeLeaf;
		if ( !nodeLeaf ) {
			return traceContext.traceResult;
		}
	}

    /**
    *	Calculate Trace Size Extents:
	*
	*	Expand our bounds by CM_BOUNDS_EPSILON offset and transform the bounds by our inverse matrix 
	*	when dealing with a transformedTrace, before calculating its symmetrical size.
    **/
	// Calculate PointTrace Case Size:
	if ( boundsPointCase ) {
		traceContext.isPoint = true;
		traceContext.extents = vec3_zero();
	// Calculate BoundsTrace Case Size:
	} else {
		// Not a point trace.
		traceContext.isPoint = false;
		// Calculate the bounds with our epsilon offset.
		traceContext.boundsEpsilonOffset = bbox3_expand( traceContext.bounds, CM_BOUNDS_EPSILON );

		// Transformed Path:
		if ( traceContext.isTransformedTrace ) {
			traceContext.transformedBounds = CM_Matrix_TransformBounds( traceContext.matTransform, traceContext.boundsEpsilonOffset );
			// NOTE: If the uncommented variety fails somehow, revert to this.
			//traceContext.extents = bbox3_symmetrical( bbox3_expand( CM_Matrix_TransformBounds( traceContext.matInvTransform, traceContext.bounds ), CM_BOUNDS_EPSILON ) );
		
			// The size of the transformedBounds box.
			traceContext.size = bbox3_size( traceContext.transformedBounds );
			// Symmetrical extents of our transformedBounds box.
			traceContext.extents = bbox3_symmetrical( traceContext.transformedBounds );
		// Non-Transformed Path:
		} else {	
			// Transformed bounds, same as regular bounds in this case:
			traceContext.transformedBounds = traceContext.boundsEpsilonOffset;

			// The size of the transformedBounds box.
			traceContext.size = bbox3_size( traceContext.transformedBounds );
			// Symmetrical extents of our boundsEpislonOffset box.
			traceContext.extents = bbox3_symmetrical( traceContext.boundsEpsilonOffset );
		}
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( traceContext.bounds, traceContext.offsets );

	/**
	*	Calculate Absolute Tracing Bounds.
	**/
	traceContext.absoluteBounds = CM_CalculateSphereTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, traceContext.traceSphere.offset, traceContext.traceSphere.radius );

	/**
	*	Take care of calculating shapes depending on our trace type.
	**/
	// Setup our context
	traceContext.traceSphere = CM_SphereFromBounds( traceContext.boundsEpsilonOffset, bbox3_center( traceContext.boundsEpsilonOffset ) );
		
	// Rotated sphere offset for capsule
	if ( traceContext.isTransformedTrace ) {
		// GLM:
		glm::vec4 vOffset = glm::vec4( phvec_to_glmvec3( traceContext.traceSphere.offset ), 1 );
		vOffset = traceContext.matTransform * vOffset;
		glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
		// END OF GLM:

		traceContext.traceSphere.offset = glmvec3_to_phvec( v3Offset );
	}


	/**
	*	Determine whether said nodeLeaf bounds intersected to our trace bounds.
	**/
	const bool leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );

	/**
	*	Check for position test special case.
	**/
	if ( positionPointCase ) {
		// Transform the absoluteBounds using matInvTransform if we're dealing with a transformed trace.
		bbox3_t absoluteBounds = traceContext.absoluteBounds;
		if ( traceContext.isTransformedTrace ) {
			absoluteBounds = CM_Matrix_TransformBounds( traceContext.matInvTransform, absoluteBounds );
		}

		// Perform world leaf test.
        if (worldTrace) {
			mleaf_t *leafs[1024];
			int32_t topNode = 0;

			const int32_t numberOfLeafs = CM_BoxLeafs_headnode( absoluteBounds, leafs, Q_COUNTOF(leafs), traceContext.headNode, nullptr );
			for ( int32_t i = 0; i < numberOfLeafs; i++ ) {
				// Make sure that the contents of said leaf are compatible with the mask we trace for.
				// Also ensure it has a valid plane to work with.
				if ( ( leafs[ i ]->contents & traceContext.contentsMask ) ) {
					const bbox3_t leafBounds = leafs[ i ] ->bounds;
					
					if ( CM_TraceIntersectBounds( traceContext, leafBounds ) ) {
						CM_TestInLeaf( traceContext, leafs[ i ] );
					}
				}

				// Break out if we're in an allSolid.
				if ( traceContext.traceResult.allSolid ) {
					break;
				}
			}
		// Perform a node leaf test.
		//} else {
		} else if ( leafBoundsIntersected ) {
			// 'Capsule' Testing:
			if ( traceContext.headNodeType == CMHullType::Capsule ) {
				//CM_TestSphereInCapsule( traceContext, nodeLeaf );
				CM_TestBoxLeafInCapsule( traceContext, nodeLeaf );
			// 'Sphere' Testing:
			} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
				//CM_TestSphereInSphere( traceContext, nodeLeaf );
				CM_TestBoxLeafInSphere( traceContext, nodeLeaf );
				//}
			// Default 'Box' Testing:
			} else {
				// Test whether our trace box intersects with the entity leaf node.
				CM_TestInLeaf( traceContext, nodeLeaf );
			}
		}

        traceContext.traceResult.endPosition = traceContext.start;
        return traceContext.traceResult;
	}

    /**
    *	Trace to leaf or perform a general sweeping through world.
    **/
	if ( worldTrace ) {
		// Transformed path.
		if ( traceContext.isTransformedTrace ) {
			glm::vec4 transformedStart = traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.start ), 1.0 );
			glm::vec4 transformedEnd =  traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.end ), 1.0 );
			CM_RecursiveTraceThroughTree( traceContext, traceContext.headNode, 0, 1, glmvec3_to_phvec( transformedStart ), glmvec3_to_phvec( transformedEnd ) );
		// Non transformed path.
		} else {	
			CM_RecursiveTraceThroughTree( traceContext, traceContext.headNode, 0, 1, traceContext.start, traceContext.end );
		}
	//} else {
	} else if ( leafBoundsIntersected ) {
		// 'Capsule' Tracing:
		if ( traceContext.headNodeType == CMHullType::Capsule ) {
			//CM_TraceSphereThroughCapsule( traceContext, nodeLeaf );
			CM_TraceBoxThroughCapsule( traceContext, nodeLeaf );
		} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
			//CM_TraceSphereThroughSphere( traceContext, nodeLeaf );
			CM_TraceBoxThroughSphere( traceContext, nodeLeaf );
		// Default 'Box' Tracing:
		} else {
			// Test whether our trace box intersects with the entity leaf node.
			CM_TraceThroughLeaf( traceContext, nodeLeaf );
		}
	}

    /**
    *	Last of all, calculate the exact trace end position based on the traceResult's clip fraction.
    **/
	traceContext.traceResult.fraction = Clampf( traceContext.traceResult.fraction, 0.f, 1.f );
	//traceContext.traceResult.fraction = Maxf( 0.f, traceContext.traceResult.fraction );
	if ( traceContext.traceResult.fraction == 0.f ) {
		traceContext.traceResult.endPosition = traceContext.start;
	} else if ( traceContext.traceResult.fraction == 1.f ) {
		traceContext.traceResult.endPosition = traceContext.end;
	} else {
		traceContext.traceResult.endPosition = vec3_mix( traceContext.start, traceContext.end, traceContext.traceResult.fraction );
	}

	// Return finalized trace result.
	return traceContext.traceResult;
}

/**
*   @brief  General 'Sphere' shape tracing routine.
**/
const TraceResult CM_SphereTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, const int32_t brushContentsMask = 0 ) {
	// Prepare trace context.
	TraceContext traceContext;

	// User input.
	traceContext.collisionModel = collisionModel;
	traceContext.headNode = headNode;
	traceContext.contentsMask = brushContentsMask;
	traceContext.matTransform = ph_mat_identity(); //glm::identity< glm::mat4 >();
	traceContext.matInvTransform = ph_mat_identity(); //glm::identity< glm::mat4 >();
	traceContext.start = start;
	traceContext.end = end;
	traceContext.bounds = bounds;
	// AbsoluteBounds: Now calculated in _CM_Trace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBowunds( start, end, bounds );

	// We trace by Sphere by default.
	traceContext.traceType = CMHullType::Sphere;

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
	//	traceContext.traceType = CMHullType::Capsule;
		traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
	//	traceContext.traceType = CMHullType::Sphere;
		traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
	//	traceContext.traceType = CMHullType::Octagon;
		traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
	//	traceContext.traceType = CMHullType::Box;
		traceContext.headNodeType = CMHullType::Box;
		traceContext.headNodeLeaf = (mleaf_t*)&boxHull.leaf;
	// Resort to world 'sweeping'.
	} else { //if ( headNode == collisionModel->cache->nodes ) {
		//traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Non Transformed Trace.
	traceContext.isTransformedTrace = false;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Sphere_Trace( traceContext );
}

/**
*   @brief  Same as CM_TraceSphere but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedSphereTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, mnode_t *headNode, const int32_t brushContentsMask = 0, const glm::mat4 &matTransform = ph_mat_identity(), const glm::mat4 &matInvTransform = ph_mat_identity() ) {
	// Prepare trace context.
	TraceContext traceContext;

	// User input.
	traceContext.collisionModel = collisionModel;
	traceContext.headNode = headNode;
	traceContext.contentsMask = brushContentsMask;
	traceContext.matTransform = matTransform;
	traceContext.matInvTransform = matInvTransform;
	traceContext.start = start;
	traceContext.end = end;
	traceContext.bounds = bounds;
	// AbsoluteBounds: Now calculated in _CM_Trace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBounds( start, end, bounds );

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.

	// We trace by Sphere by default.
	traceContext.traceType = CMHullType::Sphere;

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
	//	traceContext.traceType = CMHullType::Capsule;
		traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
	//	traceContext.traceType = CMHullType::Sphere;
		traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
	//	traceContext.traceType = CMHullType::Octagon;
		traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
	//	traceContext.traceType = CMHullType::Box;
		traceContext.headNodeType = CMHullType::Box;
		traceContext.headNodeLeaf = (mleaf_t*)&boxHull.leaf;
	// Resort to world 'sweeping'.
	} else { //if ( headNode == collisionModel->cache->nodes ) {
		//traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Transformed Trace.
	traceContext.isTransformedTrace = true;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Sphere_Trace( traceContext );
}