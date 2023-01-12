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
#include "Common/CollisionModel/CapsuleHull.h"
#include "Common/CollisionModel/OctagonBoxHull.h"
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"


//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;
//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
extern CapsuleHull capsuleHull;
//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;


//
//	TODO: Remove these after having "mainstreamed/unified" our sorry, mixed up vector maths.
//	The implementations reside in /Common/CollisionModel.cpp
//
void CM_AngleVectors( const vec3_t &angles, vec3_t &forward, vec3_t &right, vec3_t &up );
void CM_AnglesToAxis( const vec3_t &angles, vec3_t axis[3]);
void CM_Matrix_TransformVector( vec3_t m[3], const vec3_t &v, vec3_t &out );
const bbox3_t CM_Matrix_TransformBounds( const glm::mat4 &matrix, const bbox3_t &bounds );
const bbox3_t CM_EntityBounds( const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds );
const glm::vec3 phvec_to_glmvec3( const vec3_t &phv );
const vec3_t glmvec3_to_phvec( const glm::vec3 &glmv );
const glm::mat4 ph_mat_identity();
BoxHull CM_NewBoundingBoxHull( const bbox3_t &bounds, const int32_t contents );

/**
*
*
*   BSPTree Traversal, Testing & Clipping.
*
*
**/
// Diff method...
#define TRACEFIX

// 1/32 epsilon to keep floating point happy
static constexpr float DIST_EPSILON = 0.125f; //1.0f / 32.0f;//0.125; //1.0f / 64.0f;
//static constexpr float DIST_EPSILON = 0.125;

// Fraction Epsilon
//static constexpr float FRAC_EPSILON = FLT_EPSILON; // This FLT_EPSILON method works also, should be more precise.
static constexpr float FRAC_EPSILON = 1.0f / 1024.0f;




/**
*	@brief
**/

/**
*	@brief
**/
static CollisionPlane CM_TransformPlane( CollisionPlane *plane, const glm::mat4 &transformMatrix = ph_mat_identity() /*glm::identity< glm::mat4 >()*/ ) {
	//
	// #1: GLM METHOD - Seemed slightly dysfunctional though.
	//
	//CollisionPlane transformedPlane = {};
	//
	//glm::vec3 planeNormal = phvec_to_glmvec3( plane->normal );
	//// Point on plane.
	//glm::vec4 o = glm::vec4( planeNormal * plane->dist, 1. );
	//// Plane Normal vec4.
	//glm::vec4 n = glm::vec4( planeNormal, 0 );
	//
	//// Transform point.
	//o = transformMatrix * o;
	//// Transform normal.
	////n = glm::inverseTranspose( transformMatrix ) * ph_mat_identity() * n;
	//n = glm::transpose( glm::inverse( transformMatrix ) ) * n;
	//
	//// Acquire results.
	//const vec3_t phN = vec3_normalize( glmvec3_to_phvec( { n.x, n.y, n.z } ) );
	//const vec3_t phO = glmvec3_to_phvec( { o.x, o.y, o.z } );
	//transformedPlane.normal = vec3_normalize( phN );
	//transformedPlane.dist = vec3_dot( phO, phN );
	//
	//SetPlaneSignbits( &transformedPlane );
	//SetPlaneType( &transformedPlane );

	//
	// #2 NEW METHOD:
	//
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
}

/**
*	@return	The entire absolute 'capsule' trace bounds in world space.
**/
const bbox3_t CM_CalculateCapsuleTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius ) {
	// Calc spherical offset bounds.
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
//static bool CM_TraceIntersectPoint( TraceContext &traceContext, const vec3_t &point ) {
//	glm::vec4 transformedPoint = glm::vec4( phvec_to_glmvec3( point ), 1.f );
//
//	// Transform bounds if we're dealing with a transformed trace.
//	if ( traceContext.isTransformedTrace ) {
//		transformedPoint = traceContext.matTransform * transformedPoint;
//	}
//
//	return bbox3_contains_point( traceContext.absoluteBounds, glmvec3_to_phvec( transformedPoint ) );
//}
/**
*
**/
static sphere_t CM_CalculateCMSphere( const bbox3_t &symmetricBounds ) {
	// Pick radius from bounds.
	const vec3_t size = symmetricBounds.maxs; //bbox3_size( symmetricBounds );
	const float halfWidth = size.x;
	const float halfHeight = size.z;
	const float sphereRadius = ( halfWidth > halfHeight ? halfHeight : halfWidth );

	// Setup our context
	return {
		.radius = sphereRadius,
		.halfHeight = halfHeight,
		.offset = { 0.0f, 0.0f, halfHeight - sphereRadius }
	};
}


/**
*
*
*	Leaf Testing Functions
*
*
**/
/**
*   @brief Test whether the box(mins, and maxs) when located at p1 is inside of the brush, or not.
**/
static void CM_TestBoxInBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Ensure we got brush sides to test for.
    if (!brush->numsides) {
        return;
    }

	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}

	mbrushside_t *brushSide = brush->firstbrushside;
	for (int32_t i = 0; i < brush->numsides; i++, brushSide++) {

		// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
		CollisionPlane transformedPlane = *brushSide->plane;
		if ( traceContext.isTransformedTrace ) {
			transformedPlane = CM_TransformPlane( &transformedPlane, traceContext.matTransform );
		}

		// push the plane out appropriately for mins/maxs
		// if completely in front of face, no intersection
		//const float dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		//const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;
		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		}
		//const float dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		
		const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;
		//const float d2 = vec3_dot( traceContext.end, transformedPlane.normal ) - dist;

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
*   @brief 
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
        
        CM_TestBoxInBrush( traceContext, brush, leaf );
        
		//if ( !traceContext.traceResult.fraction ) {
		if ( traceContext.traceResult.allSolid ) {
		    return;
		}

    }
}

/**
*   @brief 
**/
static void CM_TestCapsuleInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	//CM_TestInLeaf( traceContext, leaf );

	// Well here we go I guess.
	vec3_t sphereTop = traceContext.start + traceContext.traceSphere.offset;
	vec3_t sphereBottom = traceContext.start - traceContext.traceSphere.offset;
	vec3_t sphereOffset = (traceContext.bounds.mins + traceContext.bounds.maxs) * 0.5f;

	// Calculate sphere for leaf
	const bbox3_t leafSymmetricBounds = bbox3_from_center_size( 
		bbox3_symmetrical( leaf->bounds ), vec3_zero() // // bbox3_size( leaf->bounds ), vec3_zero() 
	);
	sphere_t leafSphere = CM_CalculateCMSphere( leafSymmetricBounds );
	float offsetZ = leafSphere.offset.z;

	// Test Radius: See if the spheres overlap
	float testRadius = ( traceContext.traceSphere.radius + leafSphere.radius ) * ( traceContext.traceSphere.radius + leafSphere.radius );

	//
	// Top Sphere.
	//
	vec3_t pointA = sphereOffset + vec3_t{ 0.f, 0.f, offsetZ };
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
	vec3_t pointB = sphereOffset - vec3_t{ 0.f, 0.f, offsetZ };
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
	if ( ( sphereTop.z >= pointA.z && sphereTop.z <= pointB.z ) || ( sphereBottom.z >= pointB.z && sphereBottom.z <= pointB.z ) ) {
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
*   @brief 
**/
static void CM_TestBoundingBoxInCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	// Switch trace type to capsule and replace the bounding box with the capsule for our leaf.
	const bbox3_t leafSymmetricBounds = bbox3_from_center_size( 
		bbox3_symmetrical( leaf->bounds ), vec3_zero() // bbox3_size( leaf->bounds ), vec3_zero() 
	);
	traceContext.traceType = CMHullType::Capsule;
	traceContext.traceSphere = CM_CalculateCMSphere( leafSymmetricBounds );

	// Replace the capsule with the bbox to perform testing.
	BoxHull tempBoxHull = CM_NewBoundingBoxHull( traceWork.size, traceContext.contentsMask );

	// Perform testing.
	CM_TestInLeaf( traceContext, &tempBoxHull.leaf );
}



/**
*
*
*	Leaf Tracing
*
*
**/
/**
*   @brief Clips the box to the brush if needed.
**/
static void CM_TraceToBrush( TraceContext &traceContext, mbrush_t *brush, mleaf_t *leaf ) {
	// Make sure it has actual sides to clip to.
    if ( !brush->numsides ) {
        return;
    }

	if ( !CM_TraceIntersectBounds( traceContext, bbox3_expand( leaf->bounds, CM_BOUNDS_EPSILON ) ) ) {
		return;
	}

    bool getOut = false;
    bool startOut = false;

    float fraction = 0.f;

    float enterFractionA = -1.f;
    float enterFractionB = -1.f;
    float leaveFraction = 1.f;

#ifdef TRACEFIX
	float enterDistance = 0.f, move = 1.f;
#endif

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
		
		// THIS WORKS:
		//const float dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		//const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;
		//const float d2 = vec3_dot( traceContext.end, transformedPlane.normal ) - dist;

		float dist = 0.f;
		if ( traceContext.isPoint ) {
			dist = transformedPlane.dist;
		} else {
			dist = transformedPlane.dist - vec3_dot( traceContext.offsets[ transformedPlane.signBits ], transformedPlane.normal );
		}
		const float d1 = vec3_dot( traceContext.start, transformedPlane.normal ) - dist;
		const float d2 = vec3_dot( traceContext.end, transformedPlane.normal ) - dist;

        if ( d2 > 0.f ) {//0 ) {
            getOut = true; // endpoint is not in solid
        }
        if ( d1 > 0.f ) {
            startOut = true;
        }

		// This works.
        //// if completely in front of face, no intersection
        //if ( d1 > 0 && d2 >= d1 ) {
        //    return;
        //}
		// if it doesn't cross the plane, the plane isn't relevent
		//if ( d1 <= 0 && d2 <= 0 ) {
        //    continue;
        //}
			// if completely in front of face, no intersection with the entire brush
			if (d1 > 0.f && ( /*d2 >= DIST_EPSILON || */d2 >= d1 )  ) {
			//if (d1 > 0.f && d2 >= d1 ) { 
				return;
			}

			// if it doesn't cross the plane, the plane isn't relevent
			//if ( d1 <= -DIST_EPSILON && d2 <= -0 ) {
			if (d1 <= 0.f && d2 <= 0.f ) {
				continue;
			}

#ifdef TRACEFIX
		float f = d1 - d2;

		if ( f > 0.f ) {
			f = ( d1 - DIST_EPSILON ) / f;
			if ( f < 0.f ) {
				f = 0.f;
			}
			if ( f > enterFractionA ) {
				enterDistance = d1;
				move = d1 - d2;
				enterFractionA = f;
				clipPlane = transformedPlane;
				leadSide = side;
			}
		} else if ( f < 0.f ) {
			f = ( d1 + DIST_EPSILON ) / f;
			if ( f > 1.f ) {
				f = 1.f;
			}
			if ( f < leaveFraction ) {
				leaveFraction = f;
			}
		}

		//float f = d1 - d2;

		//if ( d1 > d2 ) {
		//	f = (d1 - DIST_EPSILON) / f;
		//	if ( f < 0 ) {
		//		f = 0;
		//	}
		//	if ( f > enterFractionA ) {
		//		enterDistance = d1;
		//		move = d1 - d2;
		//		enterFractionA = f;
		//		clipPlane = transformedPlane;
		//		leadSide = side;
		//	}
		//} else {
		//	f = (d1+DIST_EPSILON) / f;
		//	if ( f > 1 ) {
		//		f = 1;
		//	}
		//	if ( f < leaveFraction ) {
		//		leaveFraction = f;
		//	}
		//}
#else
        // Crosses faces.
        float f = d1 - d2;
		// Enter:
        if ( f > 0) {
            f = d1 / f;
            if ( f > enterFractionA ) {
                enterFractionA = f;
                clipPlane = transformedPlane;
                leadSide = side;
				// Nudged Fraction.
                enterFractionB = (d1 - DIST_EPSILON) / (d1 - d2);
            }
		 // Leave:
        } else if ( f < 0 ) {
            f = d1 / f;
            if ( f < leaveFraction ) {
                leaveFraction = f;
            }
        }
#endif
	}

	if ( !startOut ) {
        // Original point was inside brush.
        traceContext.traceResult.startSolid = true;
		// TODO: Set trace contents to brush we started in, and overwrite if it got out?
		traceContext.traceResult.contents = brush->contents;

        if ( !getOut ) {
			// Set trace context contents.
			traceContext.realFraction = 0.f;
			traceContext.traceResult.allSolid = true;
            traceContext.traceResult.fraction = 0.f;
        }
		return;
	}

	#ifndef TRACEFIX
    if ( enterFractionA <= -1 ) {
        return;
    }

    if ( enterFractionA > leaveFraction ) {
        return;
    }
	#endif
#ifdef TRACEFIX
	if ( enterFractionA - FRAC_EPSILON <= leaveFraction ) {
			//if ( enterFractionA > -1 && enterFractionA < traceContext.realFraction && enterFractionB < traceContext.traceResult.fraction ) {
			if ( enterFractionA > -1.f && enterFractionA < traceContext.realFraction ) {
				if ( enterFractionA < 0.f ) {// 0 ) {
					enterFractionA = 0.f;
				}

				traceContext.realFraction = enterFractionA;
				traceContext.traceResult.plane = clipPlane;
				//if ( !leadSide || !leadSide->texinfo ) {
				//	traceContext.traceResult.surface = nullptr;
				//} else {
					traceContext.traceResult.surface = &(leadSide->texinfo->c);
				//}
				traceContext.traceResult.contents = brush->contents;
				traceContext.traceResult.fraction = ( enterDistance - DIST_EPSILON ) / move;
				if ( traceContext.traceResult.fraction < 0.f ) {
					traceContext.traceResult.fraction = 0.0f;
				}
			}
		}
	}

#else
	// Check if this reduces collision time range.
	if ( enterFractionA < traceContext.realFraction ) {
		if ( enterFractionB < traceContext.traceResult.fraction ) {
			traceContext.realFraction = enterFractionA;
			traceContext.traceResult.plane = clipPlane;
			traceContext.traceResult.surface = &(leadSide->texinfo->c);
			traceContext.traceResult.contents = brush->contents;
			traceContext.traceResult.fraction = enterFractionB;
		}
	}
}
#endif

/**
*   @brief 
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
        
		// Clip the trace box to the brush itself.
        CM_TraceToBrush( traceContext, brush, leaf );
        
		// Return if we didn't collide.
		if ( !traceContext.traceResult.fraction ) {
		//if ( traceContext.traceResult.allSolid ) {
		    return;
		}
    }
}

/**
*   @brief 
**/
static void CM_TraceCapsuleThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	CM_TraceThroughLeaf( traceContext, leaf );
}

/**
*   @brief 
**/
static void CM_TraceBoundingBoxThroughCapsule( TraceContext &traceContext, mleaf_t *leaf ) {
	// Switch trace type to capsule and replace the bounding box with the capsule for our leaf.
	const bbox3_t leafSymmetricBounds = bbox3_from_center_size( 
		bbox3_symmetrical( leaf->bounds ), vec3_zero() // // bbox3_size( leaf->bounds ), vec3_zero() 
	);
	traceContext.traceType = CMHullType::Capsule;
	traceContext.traceSphere = CM_CalculateCMSphere( leafSymmetricBounds );

	// Replace the capsule with the bbox to perform testing.
	BoxHull tempBoxHull = CM_NewBoundingBoxHull( traceWork.size, traceContext.contentsMask );

	// Perform testing.
	CM_TraceThroughLeaf( traceContext, &tempBoxHull.leaf );
}

/**
*   @brief 
**/
static void CM_RecursiveTraceThroughTree( TraceContext &traceContext, mnode_t *node, float p1f, float p2f, const vec3_t &p1, const vec3_t &p2 ) {
recheck:
//#ifdef TRACEFIX
//	// If true, we already hit something nearer.
//    if (traceContext.realFraction <= p1f) {
//        return;
//    }
//#else
	// If true, we already hit something nearer.
	if (traceContext.traceResult.fraction <= p1f) {
		return;
	}
//#endif

    // If plane is NULL, we are in a leaf node
    CollisionPlane *plane = node->plane;
	
    if (!plane) {
		mleaf_t *leafNode = (mleaf_t *)node;
		if ( ( leafNode->contents & traceContext.contentsMask ) ) {
			const bbox3_t leafBounds = leafNode->bounds;
			if ( CM_TraceIntersectBounds( traceContext, leafBounds ) ) {
			   CM_TraceThroughLeaf( traceContext, leafNode );
			}
		}
        return;
    }

	// This is the plane we actually work with, making sure it is transformed properly if we are performing a transformed trace.
	CollisionPlane transformedPlane = *plane; //if ( traceContext.isTransformedTrace ) {}

    //
    // find the point distances to the seperating plane
    // and the offset for the size of the box
    //
    float offset = 0.f;
    float t1 = 0.f;
    float t2 = 0.f;

	// Axial planes.
    if ( transformedPlane.type < 3 ) {
        t1 = p1[ transformedPlane.type ] - transformedPlane.dist;
        t2 = p2[ transformedPlane.type ] - transformedPlane.dist;
        offset = traceContext.size[ transformedPlane.type ];
	// Non axial planes.
    } else {
        t1 = vec3_dot( transformedPlane.normal, p1 ) - transformedPlane.dist;
        t2 = vec3_dot( transformedPlane.normal, p2 ) - transformedPlane.dist;
        if ( traceContext.isPoint ) {
			offset = 0;
        } else {
			offset = (fabs( traceContext.size.x * transformedPlane.normal.x ) +
                     fabs( traceContext.size.y * transformedPlane.normal.y ) +
                     fabs( traceContext.size.z * transformedPlane.normal.z )) * 3;
		}
    }

    // see which sides we need to consider
    if ( t1 >= offset /*+ 1.f */&& t2 >= offset /*+ 1.f*/) {
        //node = node->children[0];
        //goto recheck;
	    CM_RecursiveTraceThroughTree( traceContext, node->children[ 0 ], p1f, p2f, p1, p2 );
		return;
    }
    if ( t1 < -offset /*- 1.f */&& t2 < -offset /*- 1.f */) {
        //node = node->children[1];
        //goto recheck;
	    CM_RecursiveTraceThroughTree( traceContext, node->children[ 1 ], p1f, p2f, p1, p2 );
		return;
    }

    // put the crosspoint DIST_EPSILON pixels on the near side
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

#if 1

    // Move up to the node
    fractionA = Clampf( fractionA, 0.f, 1.f );

    float midf = p1f + ( p2f - p1f ) * fractionA;
    vec3_t mid = vec3_mix( p1, p2, fractionA );

    CM_RecursiveTraceThroughTree( traceContext, node->children[side], p1f, midf, p1, mid );

    // Go past the node
    fractionB = Clampf( fractionB, 0.f, 1.f );

    midf = p1f + ( p2f - p1f ) * fractionB;
    mid = vec3_mix( p1, p2, fractionB );

    CM_RecursiveTraceThroughTree( traceContext, node->children[side ^ 1], midf, p2f, mid, p2 );
#else
    // Move up to the node if we can potentially hit it.
	if ( p1f < traceContext.realFraction ) {
		fractionA = Clampf( fractionA, 0.f, 1.f );

		const float midf1 = p1f + ( p2f - p1f ) * fractionA;

		const vec3_t mid = vec3_mix( p1, p2, fractionA );

		mnode_t *childNode = node->children[side];

		// No plane, leaf node.
		//if ( !childNode->plane ) {
		//	CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		//} else {
			CM_RecursiveTraceToNode( traceContext, childNode, p1f, midf1, p1, mid );
		//}
	}

	// Go past the node.
	fractionB = Clampf( fractionB, 0.f, 1.f );
	const float midf2 = p1f + ( p2f - p1f ) * fractionB;

	if ( midf2 < traceContext.realFraction ) {
		const vec3_t mid = vec3_mix( p1, p2, fractionB );

		mnode_t *childNode = node->children[side ^ 1];

		// No plane, leaf node.
		//if ( !childNode->plane ) {
		//	CM_TraceThroughLeaf( traceContext, (mleaf_t*)childNode );
		//} else {
			CM_RecursiveTraceToNode( traceContext, childNode, midf2, p2f, mid, p2 );
		//}
	}
#endif
}

/**
*   @brief	The actual main trace implementation, operates on a TraceContext prepared and supplied by helper trace functions.
**/
static const TraceResult _CM_BoxTrace( TraceContext &traceContext ) {
	// Determine if we're tracing against box hulls, or the actual world BSP brushes..
	const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
							 && traceContext.headNode != octagonHull.headNode 
							 && traceContext.headNode != capsuleHull.headNode );
	
	// Whether to use specific position point case testing:
	const bool positionPointCase = vec3_equal( traceContext.start, traceContext.end );
	// Whether to use specifics point bounds case testing:
	const bool boundsPointCase = vec3_equal( traceContext.bounds.mins, vec3_zero() ) && vec3_equal( traceContext.bounds.maxs, vec3_zero() );

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
	
	// We got a headNode, make a distinction between our entity hulls (boundingboxhull, octagonhull), or 
	// or our BSP mesh leaf nodes.
	mleaf_t *nodeLeaf = (mleaf_t*)&traceContext.headNode;
	// BoundingBox Hull:
	if ( traceContext.headNode == boxHull.headNode ) {
		nodeLeaf = &boxHull.leaf;
	// Capsule Hull:
	} else if ( traceContext.headNode == capsuleHull.headNode ) {
		nodeLeaf = &capsuleHull.leaf;
	// OctagonBox Hull:
	} else if ( traceContext.headNode == octagonHull.headNode ) {
		nodeLeaf = &octagonHull.leaf;
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
		traceContext.size = vec3_zero();

		//// Calculate needed sphere radius, halfheight, and offsets for a Capsule trace.
		//if ( traceContext.traceType == CMHullType::Capsule ) {
		//	// Generate symmetric bounds box based on our size parameter.
		//	traceContext.symmetricBounds = bbox3_from_center_size( bbox3_size( traceContext.bounds ), vec3_zero());

		//	// Setup our context
		//	traceContext.traceSphere = CM_CalculateCMSphere( traceContext.symmetricBounds );
		//}
	// Calculate BoundsTrace Case Size:
	} else {
		// Not a point trace.
		traceContext.isPoint = false;
		// Transformed Path:
		if ( traceContext.isTransformedTrace ) {
			// This seems to make more common sense tbh.
			traceContext.size = bbox3_symmetrical( bbox3_expand( CM_Matrix_TransformBounds( traceContext.matInvTransform, traceContext.bounds ), CM_BOUNDS_EPSILON ) );
			// NOTE: If the uncommented variety fails somehow, revert to this.
			//traceContext.size = bbox3_symmetrical( CM_Matrix_TransformBounds( traceContext.matInvTransform, bbox3_expand( traceContext.bounds, CM_BOUNDS_EPSILON ) ) );
		// Non-Transformed Path:
		} else {	
			traceContext.size = bbox3_symmetrical( bbox3_expand( traceContext.bounds, CM_BOUNDS_EPSILON ) );
		}

		// Calculate needed sphere radius, halfheight, and offsets for a Capsule trace.
		if ( traceContext.traceType == CMHullType::Capsule ) {
			// Generate symmetric bounds box based on our size parameter.
			traceContext.symmetricBounds = bbox3_from_center_size( traceContext.size, vec3_zero() );

			// Setup our context
			traceContext.traceSphere = CM_CalculateCMSphere( traceContext.symmetricBounds );
		}
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( traceContext.bounds, traceContext.offsets );

	/**
	*	Calculate Absolute Tracing Bounds.
	**/
	// 'Capsule'-specific Trace Bounds:
	if ( traceContext.traceType == CMHullType::Capsule ) {
		traceContext.absoluteBounds = CM_CalculateCapsuleTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, traceContext.traceSphere.offset, traceContext.traceSphere.radius );
	// 'Box' Default Trace Bounds:
	} else {
		traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
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
				if ( traceContext.traceResult.allSolid) {
					break;
				}
			}
		// Perform a node leaf test.
		} else {
		//} else if ( leafBoundsIntersected ) {
			// Capsule Testing:
			if ( traceContext.headNode == capsuleHull.headNode ) {
				// 'Capsule' in 'Capsule':
				if ( traceContext.traceType == CMHullType::Capsule ) {
					CM_TestCapsuleInCapsule( traceContext, nodeLeaf );
				// 'Box' in 'Capsule':
				} else {
					CM_TestBoundingBoxInCapsule( traceContext, nodeLeaf );
				}
			// Default Box Testing:.
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
	//} else if ( leafBoundsIntersected ) {
	} else {//if ( leafBoundsIntersected ) {
		// Capsule Tracing:
		if ( traceContext.headNode == capsuleHull.headNode ) {
			// 'Capsule' in 'Capsule':
			if ( traceContext.traceType == CMHullType::Capsule ) {
				CM_TraceCapsuleThroughCapsule( traceContext, nodeLeaf );
			// 'Box' in 'Capsule':
			} else {
				CM_TraceBoundingBoxThroughCapsule( traceContext, nodeLeaf );
			}
		// Default Box Tracing:
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
	// AbsoluteBounds: Now calculated in _CM_BoxTrace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBounds( start, end, bounds );

	// Set our trace type based on our headNode in use.
	// Capsule Tracing:
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
	// Default Box Tracing:
	} else {
		traceContext.traceType = CMHullType::Box;
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
	return _CM_BoxTrace( traceContext );
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
	// AbsoluteBounds: Now calculated in _CM_BoxTrace depending on trace type.
	//traceContext.absoluteBounds = CM_CalculateTraceBounds( start, end, bounds );

	// Set our trace type based on our headNode in use.
	// Capsule Tracing:
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
	// Default Box Tracing:
	} else {
		traceContext.traceType = CMHullType::Box;
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
	return _CM_BoxTrace( traceContext );
}