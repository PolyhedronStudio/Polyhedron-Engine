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

#include "Common/CollisionModel/Tracing/LeafTests.h"
#include "Common/CollisionModel/Tracing/LeafTraces.h"



// TEMPORARILY, NEED TO ALLOW TRACETHROUGHLEAF to HAVE A 2 HULL VARIETY AND PREVENT TEMP VAR USAGE LIKE THIS
//SphereHull boundsTestSphereHull;
//CapsuleHull boundsTestCapsuleHull;


/**
*	Temporary Hulls: Adjusted on the fly for each trace, storing the required data for
*	the 'entity headnodes' passed to the trace.
**/
//! All round 'box hull' data, accessed in a few other CollisionModel files as extern.
extern BoxHull boxHull;
//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
extern OctagonHull octagonHull;
//! All round 'capsule hull' data, accessed in a few other CollisionModel files as extern.
extern CapsuleHull capsuleHull;
//! All round 'sphere hull' data, accessed in a few other CollisionModel files as extern.
extern SphereHull sphereHull;


/**
*	In-Place Hulls: Used during Leaf/Brush Tests and Leaf/Brush Traces when requiring to 
*	convert a said passed-in Leaf Node into a different Shape type Hull Leaf Node.
**/
////! For 'Box' leaf testing.
//BoxHull leafTestBoxHull;
////! For 'Sphere' leaf testing.
//SphereHull leafTestSphereHull;
////! For 'Ca[si;e' leaf testing.
//CapsuleHull leafTestCapsuleHull;
//
////! For 'Box' leaf tracing.
//BoxHull leafTraceBoxHull;
////! For 'Sphere' leaf tracing.
//SphereHull leafTraceSphereHull;
////! For 'Capsule' leaf tracing.
//CapsuleHull leafTraceCapsuleHull;


/**
*
*
*   Utilities - To be moved elsewhere later on.
*
*
**/



/**
*
*
*	'BSP World Tree' Sweeping:
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
*
*	TraceBox Calculation:
*
*
*
**/
/**
*	@brief	Sets the 'trace box' to actually sweep and trace with, as well as calculating its
*			transformed bounds, size, extents, and offsets.
**/
void CM_SetTraceBox( TraceContext &traceContext, const bbox3_t &traceBox, const bool boundsPointCase = false ) {
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
			//traceContext.transformedBounds = bbox3_expand( CM_Matrix_TransformBounds( traceContext.matTransform, traceContext.bounds ), CM_BOUNDS_EPSILON );
		
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
			traceContext.extents = bbox3_symmetrical( traceContext.transformedBounds );
		}
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( traceContext.bounds, traceContext.offsets );


	/**
	*	Take care of calculating a correct bounds size matching shape for specific 'Shape' trace types.
	**/
	// Calculate needed capsule radius, halfheight, and offsets for a Capsule trace.
	if ( traceContext.traceType == CMHullType::Capsule ) {
		// Calculate the trace 'Capsule Sphere' for testing with.
		traceContext.traceSphere = bbox3_to_capsule( traceContext.bounds, vec3_zero() );
		// Rotated sphere offset for capsule
		if ( traceContext.isTransformedTrace ) {
			traceContext.traceSphere = CM_Matrix_TransformSphere( traceContext.matTransform, traceContext.traceSphere );
		}
	// Calculate needed sphere radius, halfheight, and offsets for a Capsule trace.
	} else if ( traceContext.traceType == CMHullType::Sphere ) {
		// Calculate the trace 'Sphere' for sphere testing with.
		traceContext.traceSphere = sphere_from_size( bbox3_symmetrical( traceContext.boundsEpsilonOffset ), vec3_zero() );
		// Rotated sphere offset for capsule
		if ( traceContext.isTransformedTrace ) {
			traceContext.traceSphere = CM_Matrix_TransformSphere( traceContext.matTransform, traceContext.traceSphere );
		}

		// Calculate offset rotation.
		sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, traceContext.traceSphere, traceContext.isTransformedTrace );
	}
}

/**
*	@brief
**/
void CM_SetTraceSphere( TraceContext &traceContext, const bbox3_t &sphereBounds, const glm::mat4 &matTransform ) {
	// Calculate the trace 'Sphere' for sphere testing with.
	traceContext.traceSphere = sphere_from_size( bbox3_symmetrical( sphereBounds ), vec3_zero() );
	// Rotated sphere offset for capsule
	if ( matTransform != ph_mat_identity() ) {//traceContext.isTransformedTrace ) {
		traceContext.traceSphere = CM_Matrix_TransformSphere( matTransform, traceContext.traceSphere );
	}
}

/**
*
*
*
*	TraceBounds Calculation:
*
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
			CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) //);
}

/**
*	@return	The entire absolute 'Capsule' trace bounds in world space.
**/
const bbox3_t CM_CalculateCapsuleTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius ) {
	vec3_t newStart = start;
	vec3_t newEnd = end;

	// Add/Subtract the sphere offset from and radiuses in order to get the total start and end points.
	for ( int32_t i = 0; i < 3; i++ ) {
		if ( start[i] < end[i] ) {
			newStart[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadius;
			newEnd[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadius;
		} else {
			newStart[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadius;
			newEnd[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadius;
		}
	}

	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { newStart,  newEnd };

	// Add bounds epsilon.
	return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
				bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
			CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) 
}

/**
*	@return	The entire absolute 'Sphere' trace bounds in world space.
**/
const bbox3_t CM_CalculateSphereTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius ) {
	vec3_t newStart = start;
	vec3_t newEnd = end;

	const float sphereRadiustDistEpsilon = sphereRadius + CM_RAD_EPSILON;
	for ( int32_t i = 0; i < 3; i++ ) {
		if ( start[i] < end[i] ) {
			newStart[i] = start[i] - fabsf( sphereOffset[i] ) - sphereRadiustDistEpsilon;
			newEnd[i] = end[i] + fabsf( sphereOffset[i] ) + sphereRadiustDistEpsilon;
		} else {
			newStart[i] = end[i] - fabsf( sphereOffset[i] ) - sphereRadiustDistEpsilon;
			newEnd[i] = start[i] + fabsf( sphereOffset[i] ) + sphereRadiustDistEpsilon;
		}
	}

	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { newStart,  newEnd };
	return bbox3_expand_box(						//return bbox3_expand_box(
				bbox3_from_points( tracePoints, 2 ), bounds );
}

/**
*	@return	True if the bounds intersected.
**/
const bool CM_TraceIntersectBounds( TraceContext &traceContext, const bbox3_t &testBounds ) {
	bbox3_t transformedTestBounds = testBounds;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		transformedTestBounds = CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	}

	return bbox3_intersects( traceContext.absoluteBounds, transformedTestBounds );
}

/**
*	@return	True if the 2D Cylinder and the Trace Bounds intersect.
**/
const bool CM_TraceIntersect2DCylinder( TraceContext &traceContext, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon ) {
	sphere_t transformedTestSphere = sphere;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matTransform * phvec_to_glmvec4( transformedTestSphere.offset ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.offset = glmvec3_to_phvec( v3TransformedOffset );
	}

	// Nullify the z of both, bounds and sphere
	bbox3_t noZBounds = { vec3_xy( traceContext.absoluteBounds.mins ), vec3_xy( traceContext.absoluteBounds.maxs ) };
	sphere_t noZSphere = transformedTestSphere;
	noZSphere.offset = vec3_xy( noZSphere.offset );

	return bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTestSphere, testType, radiusDistEpsilon );
}
/**
*	@return	True if the Sphere and the Trace Bounds intersect.
**/
const bool CM_TraceIntersectSphere( TraceContext &traceContext, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon ) {
	sphere_t transformedTestSphere = sphere;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
	}

	return bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTestSphere, testType, radiusDistEpsilon );
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
	/*const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
							 && traceContext.headNode != capsuleHull.headNode 
							 && traceContext.headNode != sphereHull.headNode 
							 && traceContext.headNode != octagonHull.headNode );
	*/
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
    *	Calculate the 'TraceBox' and its 'Sphere' equivelant match.:
	*
	*	Expand our bounds by CM_BOUNDS_EPSILON offset and transform the bounds by our inverse matrix 
	*	when dealing with a transformedTrace, before calculating its symmetrical size.
    **/
	CM_SetTraceBox( traceContext, traceContext.bounds, boundsPointCase );


	/**
	*	Calculate Absolute Tracing Bounds.
	**/
	// 'Capsule'-specific Trace Bounds:
	if ( traceContext.headNodeType == CMHullType::Capsule ) {
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		const vec3_t sphereOffsetOrigin = traceContext.traceSphere.offset;
		
		traceContext.absoluteBounds = CM_CalculateCapsuleTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, sphereOffsetOrigin, traceContext.traceSphere.offsetRadius );
	// 'Sphere'-specific Trace Bounds:
	} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
		const vec3_t sphereOffsetOrigin = traceContext.traceSphere.origin + traceContext.traceSphere.offset;
		//traceContext.absoluteBounds = CM_CalculateBoxTraceBounds( traceContext.start, traceContext.end, traceContext.bounds );
		traceContext.absoluteBounds = CM_CalculateSphereTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, sphereOffsetOrigin, traceContext.traceSphere.offsetRadius );
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
	if ( traceContext.headNodeType == CMHullType::Capsule ) {
		leafBoundsIntersected = CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
	// 'Sphere'-specific Trace Bounds:
	} else if ( traceContext.headNodeType == CMHullType::Sphere ) {
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
				if ( traceContext.traceType == CMHullType::Sphere ) {
					CM_TestSphereLeafInSphere( traceContext, nodeLeaf );
					//CM_TestBoxLeafInSphere( traceContext, nodeLeaf );

				// 'Capsule' in 'Sphere'.
				//} else if ( traceContext.traceType == CMHullType::Capsule) {
					//CM_TestCapsuleInSphere( traceContext, nodeLeaf );
				// Default to: 'Bounding Box' in 'Sphere'.
				} else {
					CM_TestBoxLeafInSphere( traceContext, nodeLeaf );
				}
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
			if ( traceContext.traceType == CMHullType::Sphere ) {
				CM_TraceSphereThroughSphere( traceContext, nodeLeaf );
			//// 'Capsule' in 'Sphere'.
			//} else if ( traceContext.traceType == CMHullType::Capsule ) {
			//	//CM_TraceCapsuleThroughSphere( traceContext, nodeLeaf );
			//// Default to: 'Bounding Box' in 'Sphere'.
			} else {
				CM_TraceBoxThroughSphere( traceContext, nodeLeaf );
			}
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

	// We trace by Sphere by default.
	traceContext.traceType = CMHullType::Box;

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

	// We trace by Sphere by default.
	traceContext.traceType = CMHullType::Box;

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
	//const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
	//						 && traceContext.headNode != capsuleHull.headNode 
	//						 && traceContext.headNode != sphereHull.headNode 
	//						 && traceContext.headNode != octagonHull.headNode );

	// Determine if we're tracing against box hulls, or the actual world BSP brushes..
	//const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
	//						 && traceContext.headNode != capsuleHull.headNode 
	//						 && traceContext.headNode != sphereHull.headNode 
	//						 && traceContext.headNode != octagonHull.headNode );
	const bool worldTrace = !( traceContext.headNodeType > CMHullType::World );
	
	//const bool worldTrace = !( traceContext.traceType > CMHullType::World );
	//sconst bool worldTrace = ( traceContext.headNode == &traceContext.collisionModel->cache->nodes[0] );

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
			//traceContext.transformedBounds = bbox3_expand( CM_Matrix_TransformBounds( traceContext.matTransform, traceContext.bounds ), CM_BOUNDS_EPSILON );
		
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
			traceContext.extents = bbox3_symmetrical( traceContext.transformedBounds );
		}
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( traceContext.bounds, traceContext.offsets );

	/**
	*	Calculate Absolute Tracing Bounds.
	**/
	const vec3_t sphereOffsetOrigin = traceContext.traceSphere.origin + traceContext.traceSphere.offset;

	traceContext.absoluteBounds = CM_CalculateSphereTraceBounds( traceContext.start, traceContext.end, traceContext.bounds, sphereOffsetOrigin, traceContext.traceSphere.offsetRadius );

	/**
	*	Take care of calculating shapes depending on our trace type.
	**/
	// Calculate the trace 'Sphere' for sphere testing with.
	traceContext.traceSphere = sphere_from_size( bbox3_symmetrical( traceContext.boundsEpsilonOffset ), vec3_zero() );
	// Rotated sphere offset for capsule
	if ( traceContext.isTransformedTrace ) {
		traceContext.traceSphere = CM_Matrix_TransformSphere( traceContext.matTransform, traceContext.traceSphere );
	}

	// Calculate offset rotation.
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, traceContext.traceSphere, traceContext.isTransformedTrace );



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
				CM_TestSphereLeafInSphere( traceContext, nodeLeaf );
				//CM_TestBoxLeafInSphere( traceContext, nodeLeaf );
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
			CM_TraceSphereThroughSphere( traceContext, nodeLeaf );
			//CM_TraceBoxThroughSphere( traceContext, nodeLeaf );
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
	traceContext.headNodeType = CMHullType::Sphere;

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
	//	traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
		traceContext.traceType = CMHullType::Sphere;
	//	traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
		traceContext.traceType = CMHullType::Octagon;
	//	traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
		traceContext.traceType = CMHullType::Box;
	//	traceContext.headNodeType = CMHullType::Box;
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
	traceContext.headNodeType = CMHullType::Sphere;

	// Determine the trace hull type, as well as the headNode type,
	// needed to choose a distinct test and trace path.
	// 'Capsule Hull':
	if ( headNode == capsuleHull.headNode ) {
		traceContext.traceType = CMHullType::Capsule;
	//	traceContext.headNodeType = CMHullType::Capsule;
		traceContext.headNodeLeaf = (mleaf_t*)&capsuleHull.leaf;
	// 'Sphere Hull':
	} else if ( headNode == sphereHull.headNode ) {
		traceContext.traceType = CMHullType::Sphere;
	//	traceContext.headNodeType = CMHullType::Sphere;
		traceContext.headNodeLeaf = (mleaf_t*)&sphereHull.leaf;
	// 'OctagonBox Hull'
	} else if ( headNode == octagonHull.headNode ) {
		traceContext.traceType = CMHullType::Octagon;
	//	traceContext.headNodeType = CMHullType::Octagon;
		traceContext.headNodeLeaf = (mleaf_t*)&octagonHull.leaf;
	// 'BoundingBox Hull'
	} else if ( headNode == boxHull.headNode ) {
		traceContext.traceType = CMHullType::Box;
	//	traceContext.headNodeType = CMHullType::Box;
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
	return _CM_Sphere_Trace( traceContext );
}