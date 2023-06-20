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

#include "Common/CollisionModel/Tracing/BrushTests.h"
#include "Common/CollisionModel/Tracing/BrushTraces.h"
#include "Common/CollisionModel/Tracing/LeafTests.h"
#include "Common/CollisionModel/Tracing/LeafTraces.h"

#include "Common/CollisionModel/Tracing/BoxSweep.h"
#include "Common/CollisionModel/Tracing/SphereSweep.h"


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
*	Config
**/
//#define TRACE_ENABLE_BOUNDS_POINT_CASE


/**
*
*
*
*	Trace Shape calculations:
*
*
*
**/
/**
*	@brief	Calculates the AABBTrace bounds needed to trace, by expanding our bounds by CM_BOUNDS_EPSILON offset and transform 
*			the bounds by our inverse matrix when dealing with a transformedTrace, after which it finishes by calculating its 
*			symmetrical extents.
**/
const TraceContext::AABBTrace CM_AABB_CalculateTraceShape( TraceContext &traceContext, const bbox3_t &bounds, const bool boundsPointCase, 
													   const bool isTransformedTrace, const glm::mat4 &matTransform, const glm::mat4 &matInvTransform ) {
	// Our AABB data container.
	TraceContext::AABBTrace aabbTrace;

	// Set bounds.
	aabbTrace.bounds = bounds;

	// Calculate PointTrace Case Size:
	if ( boundsPointCase ) {
		aabbTrace.extents = vec3_zero();
	// Calculate BoundsTrace Case Size:
	} else {
		// Calculate the bounds with our epsilon offset.
		aabbTrace.boundsEpsilonOffset = bbox3_expand( traceContext.aabbTrace.bounds, CM_BOUNDS_EPSILON );

		// Transformed Path:
		if ( isTransformedTrace ) {
			aabbTrace.transformedBounds = CM_Matrix_TransformBounds( matTransform, aabbTrace.boundsEpsilonOffset );
			// NOTE: If the uncommented variety fails somehow, revert to this.
			//aabbTrace.transformedBounds = bbox3_expand( CM_Matrix_TransformBounds( traceContext.matTransform, traceContext.aabbTrace.bounds ), CM_BOUNDS_EPSILON );
		// Non-Transformed Path:
		} else {	
			// Transformed bounds, same as regular bounds in this case:
			aabbTrace.transformedBounds = aabbTrace.boundsEpsilonOffset;
		}

		// The size of the transformedBounds box.
		aabbTrace.size = bbox3_size( aabbTrace.transformedBounds );
		// Symmetrical extents of our boundsEpislonOffset box.
		aabbTrace.extents = bbox3_symmetrical( aabbTrace.transformedBounds );
	}

	/**
	*	Calculate box 'offset' points for easy plane side testing using the box' corners.
	**/
	bbox3_to_points( aabbTrace.bounds, aabbTrace.offsets );
	bbox3_to_points( aabbTrace.transformedBounds, aabbTrace.transformedOffsets );


	return aabbTrace;
}

/**
*	@brief	Calculates a new sphere shape to trace with for the trace context.
*	@param	isTransformed			When true, the transformedSphere is transformed by the trace context's transform matrix. If false
*									however, it will be identical to the non transformed sphere itself.
*	@param	expandRadiusEpsilon		Expand by radius epsilon, for use if sphereBounds isn't epsilonOffset yet.
**/
const TraceContext::SphereTrace CM_Sphere_CalculateTraceShape( TraceContext &traceContext, const bbox3_t &sphereBounds, const bool boundsPointCase, const bool expandRadiusEpsilon, 
														 const bool isTransformedTrace, const glm::mat4 &matTransform, const glm::mat4 &matInvTransform ) {
	// Our sphere data container.
	TraceContext::SphereTrace sphereTrace;

	// Expand by radius epsilon, for use if sphereBounds isn't epsilonOffset yet.
	sphereTrace.sourceBounds = ( expandRadiusEpsilon ? sphereBounds : bbox3_expand( sphereBounds, CM_RAD_EPSILON ) );
	
	//if ( boundsPointCase ) {
	//	sphereTrace.sourceBounds = bbox3_expand( bbox3_zero(), CM_RAD_EPSILON );
	//}

	// Calculate sphere based on the symmetrical bounds extents with its origin set to the bounds' center point.
	sphereTrace.sphere = sphere_from_size( bbox3_symmetrical( sphereTrace.sourceBounds ), bbox3_center( sphereTrace.sourceBounds ) ); //bbox3_center( epsilonSphereBounds ) );
	sphere_calculate_offset_rotation( matTransform, matInvTransform, sphereTrace.sphere, isTransformedTrace );

	// Calculate the transformed trace sphere.
	if ( isTransformedTrace ) {
		// Calculate the transformed sphere origin.
		sphereTrace.transformedSphere = CM_Matrix_TransformSphere( matTransform, sphereTrace.sphere );

		// Now calculate rotational offsets for non transformed and transformed trace spheres.
		sphere_calculate_offset_rotation( matTransform, matInvTransform, sphereTrace.transformedSphere, isTransformedTrace );
	} else {
		// Before copying it into transformed trace sphere.
		sphereTrace.transformedSphere = sphereTrace.sphere;
	}

	// Return our sphere trace.
	return sphereTrace;
}

const TraceContext::SphereTrace CM_Sphere_CalculateTraceShape( TraceContext &traceContext, const bbox3_t &sphereBounds, const sphere_t &sphere, const bool boundsPointCase, const bool expandRadiusEpsilon, 
														 const bool isTransformedTrace, const glm::mat4 &matTransform, const glm::mat4 &matInvTransform ) {
	// Our sphere data container.
	TraceContext::SphereTrace sphereTrace;

	// Expand by radius epsilon, for use if sphereBounds isn't epsilonOffset yet.
	sphereTrace.sourceBounds = ( expandRadiusEpsilon ? sphereBounds : bbox3_expand( sphereBounds, CM_RAD_EPSILON ) );
	
	//if ( boundsPointCase ) {
	//	sphereTrace.sourceBounds = bbox3_expand( bbox3_zero(), CM_RAD_EPSILON );
	//}

	// Calculate sphere based on the symmetrical bounds extents with its origin set to the bounds' center point.
	sphereTrace.sphere = sphere;

	// Calculate the transformed trace sphere.
	if ( isTransformedTrace ) {
		// Calculate the transformed sphere origin.
		sphereTrace.transformedSphere = CM_Matrix_TransformSphere( matTransform, sphereTrace.sphere );

		// Now calculate rotational offsets for non transformed and transformed trace spheres.
		sphere_calculate_offset_rotation( matTransform, matInvTransform, sphereTrace.transformedSphere, isTransformedTrace );
	} else {
		// Before copying it into transformed trace sphere.
		sphereTrace.transformedSphere = sphereTrace.sphere;
	}

	// Return our sphere trace.
	return sphereTrace;
}



/**
*
*
*
*	'Absolute Trace Bounds' Calculations:
*
*
*
**/
/**
*	@return	The entire absolute 'box' trace bounds in world space.
**/
const bbox3_t CM_AABB_CalculateTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds ) {
	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { start,  end };

	// Add bounds epsilon.
	return bbox3_expand( bbox3_expand_box(						//return bbox3_expand_box(
				bbox3_from_points( tracePoints, 2 ), bounds ),	//	bbox3_from_points( tracePoints, 2 ), 
			CM_BOUNDS_EPSILON );								//	bbox3_expand( bounds, CM_BOUNDS_EPSILON ) //);
}

/**
*	@return	The entire absolute 'Sphere' trace bounds in world space.
**/
const bbox3_t CM_Sphere_CalculateTraceBounds( const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const vec3_t &sphereOffset, const float sphereRadius, const float sphereRadiusEpsilon ) {
	vec3_t newStart = start;
	vec3_t newEnd = end;

	const float sphereRadiustDistEpsilon = sphereRadius + sphereRadiusEpsilon;
	for ( int32_t i = 0; i < 3; i++ ) {
		if ( start[i] < end[i] ) {
			newStart[i] = ( start[i] - fabsf( sphereOffset[i] ) ) - sphereRadiustDistEpsilon;
			newEnd[i] = ( end[i] + fabsf( sphereOffset[i] ) ) + sphereRadiustDistEpsilon;
		} else {
			newStart[i] = ( end[i] - fabsf( sphereOffset[i] ) ) - sphereRadiustDistEpsilon;
			newEnd[i] = ( start[i] + fabsf( sphereOffset[i] ) ) + sphereRadiustDistEpsilon;
		}
	}

	// Prepare array for from_points method.
	const vec3_t tracePoints[] = { newStart,  newEnd };
	return bbox3_from_points( tracePoints, 2 );
}



/**
*
*
*
*	Intersection Checks:
*
*
*
**/
/**
*	@return	True if the bounds intersected our traceBounds. Bounds will be transformed if necessary.
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
*	@return	True if the Sphere and the traceBounds intersect. Sphere will be transformed if necessary.
**/
const bool CM_TraceIntersectSphere( TraceContext &traceContext, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon, const bool useOriginOffset ) {
	sphere_t transformedTestSphere = sphere;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		glm::vec4 transformedOffset = traceContext.matTransform * phvec_to_glmvec4( transformedTestSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
		glm::vec3 v3TransformedOffset = glm::vec3( transformedOffset.x / transformedOffset.w, transformedOffset.y / transformedOffset.w, transformedOffset.z / transformedOffset.w );
		transformedTestSphere.origin = glmvec3_to_phvec( v3TransformedOffset );
	}

	// Calculate offset rotation.
	sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, transformedTestSphere, traceContext.isTransformedTrace );

	return bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTestSphere, testType, radiusDistEpsilon, useOriginOffset );
}

const bool CM_AABB_TraceIntersectLeafSphere( const TraceContext &traceContext, mleaf_t *nodeLeaf, const float radiusDistEpsilon ) {
	bbox3_t transformedTestBounds = nodeLeaf->bounds;

	// Transform bounds if we're dealing with a transformed trace.
	if ( traceContext.isTransformedTrace ) {
		transformedTestBounds = CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	}

	// Test sphere against node leaf bounds.
	return sphere_intersects_bbox3( transformedTestBounds, traceContext.sphereTrace.transformedSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, radiusDistEpsilon );
}

/**
*	@brief	Checks whether the two spheres intersected or not.
*	@return	0 if no intersection. 1 if it intersected the 'top', 2 if it intersected the bottom, 3 if it intersected both sides.
**/
const bool CM_Sphere_TraceIntersectLeafSphere( const TraceContext &traceContext, mleaf_t *nodeLeaf, const float radiusDistEpsilon ) {
	sphere_t traceSphere = traceContext.sphereTrace.transformedSphere;

	const vec3_t traceSphereOffsetOrigin = traceSphere.origin + traceSphere.offset;
	const vec3_t traceSphereStart		 = traceContext.start - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	const vec3_t traceSphereEnd			 = traceContext.end - traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	// Leaf Sphere.
	sphere_t leafSphere = nodeLeaf->sphereShape; //bbox3_center( epsilonSphereBounds ) );
	//sphere_calculate_offset_rotation( traceContext.matTransform, traceContext.matInvTransform, leafSphere, traceContext.isTransformedTrace );

	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start - leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end - leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;


	/**
	*	Test
	**/
	// Keep score of what we intersected with.
	int32_t intersections = 0;
	// Total test radius. (Seems to work)
	//const float testRadius = /*flt_square*/( traceSphere.radius + leafSphere.radius );
	const float testRadius = flt_square( traceSphere.radius + radiusDistEpsilon + leafSphere.radius + radiusDistEpsilon );

	// Top point.
	const vec3_t pointTop = leafSphereOffsetOrigin - traceSphereStart;
	if ( vec3_length_squared( pointTop ) < testRadius ) {
		intersections &= 1;
	}

	// Bottom point.
	const vec3_t pointBottom = leafSphereOffsetOrigin - traceSphereEnd; //traceSphereOffsetOrigin - traceSphereBottom;
	if ( vec3_length_squared( pointBottom ) < testRadius ) {
		intersections += 2;
	}
	// No intersection.
	return intersections;
}

/**
*	@brief	Perform an 'absolute bounds', trace shape intersection test on the leaf node. First it
*			checks if the actual leafNode bounds are intersection the absolute bounds, after which
*			it performs an actual 'shape specific' to 'leaf bounds' test.
*	@return	true if the trace absolute bounds intersected with the leaf. false otherwise.
**/
static const bool _CM_Trace_LeafIntersectAbsoluteBounds( TraceContext &traceContext, mleaf_t *leafNode ) {
	bool boundsIntersected = false;

	/**
	*	Sphere TraceShape:
	**/
	if ( traceContext.traceShape == TraceShape::Sphere ) {
		// Bounds check.
		boundsIntersected = CM_TraceIntersectBounds( traceContext, leafNode->bounds );

		// If intersected, perform specific shape type intersection tests.
		if ( boundsIntersected ) {
			// SphereTrace vs Sphere Node.
			if ( leafNode->shapeType == CMHullType::Sphere ) {
				boundsIntersected = CM_Sphere_TraceIntersectLeafSphere( traceContext, leafNode, CM_RAD_EPSILON );
			// SphereTrace vs Box Node.
			} else {
				// Test sphere against node leaf bounds.
				boundsIntersected = sphere_intersects_bbox3( leafNode->bounds, traceContext.sphereTrace.transformedSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, 0.f );
			}
		}

	/**
	*	AABB TraceShape:
	**/
	} else {
		// Bounds check.
		boundsIntersected = CM_TraceIntersectBounds( traceContext, leafNode->bounds );//( !worldTrace ? CM_TraceIntersectBounds( traceContext, nodeLeaf->bounds ) : false );
		
		// If intersected, perform specific shape type intersection tests.
		if ( boundsIntersected ) {
			// SphereTrace vs Sphere Node.
			if ( leafNode->shapeType == CMHullType::Sphere ) {
				boundsIntersected = CM_AABB_TraceIntersectLeafSphere( traceContext, leafNode, 0.f );
			// SphereTrace vs Box Node.
			} else {
				// Test sphere against node leaf bounds.
				boundsIntersected = bbox3_intersects_sphere( leafNode->bounds, traceContext.sphereTrace.transformedSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, 0.f );
			}
		}
	}

	// Return results.
	return boundsIntersected;
}

/**
*	@brief	Performs proper position point testing.
**/
static const bool _CM_Trace_LeafIntersectPositionPoint(  TraceContext &traceContext, mleaf_t *leafNode ) {
	bool boundsIntersected = false;

	/**
	*	Sphere TraceShape:
	**/
	if ( traceContext.traceShape == TraceShape::Sphere ) {
		// First test for AABB
		boundsIntersected = CM_TraceIntersectBounds( traceContext, leafNode->bounds );

		// Then for Sphere.
		if ( boundsIntersected ) {
			// TODO: Sphere vs bbox instead?
			boundsIntersected = CM_Sphere_TraceIntersectLeafSphere( traceContext, leafNode, CM_RAD_EPSILON );//bbox3_intersects_sphere( leafBounds , traceContext.sphereTrace.transformedSphere, bbox3_t::IntersectType::SolidBox_SolidSphere, 1.f );
		}

	/**
	*	AABB TraceShape:
	**/
	} else {
		// Bounds check.
		boundsIntersected = CM_TraceIntersectBounds( traceContext, leafNode->bounds );
	}

	// Return results.
	return boundsIntersected;
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
*
**/
static void _CM_Trace_CalculateTraceShapes( TraceContext &traceContext, const bool boundsPointCase ) {
		// AABB Trace
		traceContext.aabbTrace = CM_AABB_CalculateTraceShape( 
			traceContext, 
		
			// Source Bounds. (Already set before calling _CM_Trace.)
			traceContext.aabbTrace.bounds, 

			// Calculate AABB trace for point tracing or not.
			boundsPointCase,
			//  Determines whether to calculated a transformed sphere generated from AABBTrace.bounds or not.
			traceContext.isTransformedTrace, 
		
			// Use the trace context's matrices.
			traceContext.matTransform,
			traceContext.matInvTransform
		);
	

		/**
		*	Calculate the data for 'Sphere Shape Tracing'.
		**/
		traceContext.sphereTrace = CM_Sphere_CalculateTraceShape(
			traceContext,

			// Source Sphere Bounds.
			traceContext.aabbTrace.boundsEpsilonOffset,
			// Bounds Sphere
			traceContext.sphereTrace.sphere,

			// Calculate Sphere trace for point tracing or not.
			boundsPointCase,
			// Add the radius epsilon offset before storing bounds as our 'sourceBounds'.
			true,
			// Determines whether to calculated a transformed sphere generated from AABBTrace.bounds or not.
			traceContext.isTransformedTrace, 
		
			// Use the trace context's matrices.
			traceContext.matTransform,
			traceContext.matInvTransform
		);
}



/**
*   @brief	The actual main trace implementation, operates on a TraceContext prepared and supplied by helper trace functions, sweeping
*			the said TraceShape through the head/leaf -node(s) from point 'start' to 'end' point.
**/
static const TraceResult _CM_Trace( TraceContext &traceContext ) {
	// Determine if we're tracing against box hulls, or the actual world BSP brushes..
	/*const bool worldTrace = ( traceContext.headNode != boxHull.headNode 
							 && traceContext.headNode != capsuleHull.headNode 
							 && traceContext.headNode != sphereHull.headNode 
							 && traceContext.headNode != octagonHull.headNode );
	*/
	// Determine if we're tracing against box hulls, or the actual world BSP brushes..
	const bool worldTrace = ( traceContext.headNodeLeaf == nullptr ? true : false ); //!( traceContext.headNodeType > CMHullType::World );
	
	// Whether to use specific position point case testing:
	const bool positionPointCase = vec3_equal( traceContext.start, traceContext.end );
	// Whether to use specifics point bounds case testing:
#ifdef TRACE_ENABLE_BOUNDS_POINT_CASE
	const bool boundsPointCase = vec3_equal( traceContext.aabbTrace.bounds.mins, vec3_zero() ) && vec3_equal( traceContext.aabbTrace.bounds.maxs, vec3_zero() );
#else
	const bool boundsPointCase = false;
#endif


	/**
	*	For brush multi-check avoidance.
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
    *	Calculate the data for 'AABB Shape Tracing'.
    **/
	_CM_Trace_CalculateTraceShapes( traceContext, boundsPointCase );


	/**
	*	Calculate absolute tracing bounds for the traceShape we're using.
	**/
	// 'Sphere' Trace Bounds:
	if ( traceContext.traceShape == TraceShape::Sphere ||
		traceContext.headNodeType == TraceShape::Sphere ) {
		const vec3_t sphereOffsetOrigin = /*traceContext.sphereTrace.transformedSphere.origin + */traceContext.sphereTrace.transformedSphere.offset;
		traceContext.absoluteBounds = CM_Sphere_CalculateTraceBounds( traceContext.start, traceContext.end, traceContext.aabbTrace.bounds, sphereOffsetOrigin, traceContext.sphereTrace.sphere.radius );
	// 'AABB' Trace Bounds:
	} else {
		traceContext.absoluteBounds = CM_AABB_CalculateTraceBounds( traceContext.start, traceContext.end, traceContext.aabbTrace.bounds );
	}


	/**
	*	Determine whether said nodeLeaf bounds intersected to our trace bounds.
	**/
	bool leafBoundsIntersected = _CM_Trace_LeafIntersectAbsoluteBounds( traceContext, nodeLeaf );

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
        if ( worldTrace ) {
			mleaf_t *leafs[1024];
			int32_t topNode = 0;

			const int32_t numberOfLeafs = CM_BoxLeafs_headnode( absoluteBounds, leafs, Q_COUNTOF(leafs), traceContext.headNode, nullptr );
			for ( int32_t i = 0; i < numberOfLeafs; i++ ) {
				// Make sure that the contents of said leaf are compatible with the mask we trace for.
				// Also ensure it has a valid plane to work with.
				if ( ( leafs[ i ]->contents & traceContext.contentsMask ) ) {
					//
					bool worldLeafBoundsIntersected = _CM_Trace_LeafIntersectPositionPoint( traceContext, leafs[ i ] );;

					// Perform a test in case this leaf's bound intersected to our trace bounds.
					if ( worldLeafBoundsIntersected ) {
						if ( traceContext.traceShape == TraceShape::Sphere ) {
							CM_TraceSphere_TestInLeaf( traceContext, leafs[ i ] );
						} else {
							CM_TraceBox_TestInLeaf( traceContext, leafs[ i ] );
						}
					}

					// Break out if we're in an allSolid.
					if ( traceContext.traceResult.allSolid ) {
						break;
					}
				}
			}
		// Perform a node leaf test.
		//} else {
		} else {
			if ( leafBoundsIntersected ) {
				if ( traceContext.traceShape == TraceShape::Sphere ) {
					CM_TraceSphere_TestInLeaf( traceContext, nodeLeaf );
				} else {
					CM_TraceBox_TestInLeaf( traceContext, nodeLeaf );
				}
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
			const vec3_t transformedStart = glmvec4_to_phvec( traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.start ), 1.0 ) );
			const vec3_t transformedEnd = glmvec4_to_phvec( traceContext.matInvTransform * glm::vec4( phvec_to_glmvec3( traceContext.end ), 1.0 ) );			

			// 'Sphere Sweep':
			if ( traceContext.traceShape == TraceShape::Sphere ) {
				CM_RecursiveSphereTraceThroughTree( traceContext, traceContext.headNode, 0, 1, transformedStart, transformedEnd );
			// Default 'Box Sweep':
			} else {
				CM_RecursiveBoxTraceThroughTree( traceContext, traceContext.headNode, 0, 1, transformedStart, transformedEnd );
			}
		// Non transformed path.
		} else {	
			// 'Sphere Sweep':
			if ( traceContext.traceShape == TraceShape::Sphere ) {
				CM_RecursiveSphereTraceThroughTree( traceContext, traceContext.headNode, 0, 1, traceContext.start, traceContext.end );
			// Default 'Box Sweep':
			} else {
				CM_RecursiveBoxTraceThroughTree( traceContext, traceContext.headNode, 0, 1, traceContext.start, traceContext.end );
			}
		}
	} else {
		if ( leafBoundsIntersected ) {
			if ( traceContext.traceShape == TraceShape::Sphere ) {
				CM_TraceSphere_TraceThroughLeaf( traceContext, nodeLeaf );
			} else {
				CM_TraceBox_TraceThroughLeaf( traceContext, nodeLeaf );
			}
		}
	}

    /**
    *	Store the final resulting 'bounds corner offsets'.
    **/
	for ( int32_t i = 0; i < 8; i++ ) {
		traceContext.traceResult.offsets[i] = traceContext.aabbTrace.offsets[i];
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

	// Set our AABBTrace Bounds early on so _CM_Trace can calculate the rest from there.
	traceContext.aabbTrace.bounds = bounds;
	const bbox3_t boundsEpsilonOffset = bbox3_expand( bounds, CM_RAD_EPSILON );
	traceContext.sphereTrace.sphere = sphere_from_size( bbox3_symmetrical( bounds ), bbox3_center( bounds ) );
	//traceContext.sphereTrace.sphere = sphere_from_size( bbox3_symmetrical( bounds ), bbox3_center( bounds ) );

	// Set 'Sphere' as our Trace Shape.
	traceContext.traceShape = TraceShape::AABB;
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
	traceContext.realFraction = 1.f + DIST_EPSILON;
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

	// Set our AABBTrace Bounds early on so _CM_Trace can calculate the rest from there.
	traceContext.aabbTrace.bounds = bounds;
	const bbox3_t boundsEpsilonOffset = bbox3_expand( bounds, CM_RAD_EPSILON );
	traceContext.sphereTrace.sphere = sphere_from_size( bbox3_symmetrical( bounds ), bbox3_center( bounds ) );

	// Set 'Sphere' as our Trace Shape.
	traceContext.traceShape = TraceShape::AABB;
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
	traceContext.realFraction = 1.f + DIST_EPSILON;
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
*   @brief  General 'Sphere' shape tracing routine.
**/
const TraceResult CM_SphereTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const sphere_t &sphere, mnode_t *headNode, const int32_t brushContentsMask = 0 ) {
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

	// Set our AABBTrace Bounds early on so _CM_Trace can calculate the rest from there.
	traceContext.aabbTrace.bounds = bounds;
	traceContext.sphereTrace.sphere = sphere;

	// Set 'Sphere' as our Trace Shape.
	traceContext.traceShape = TraceShape::Sphere;
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
		traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Non Transformed Trace.
	traceContext.isTransformedTrace = false;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f + DIST_EPSILON;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Trace( traceContext );
}

/**
*   @brief  Same as CM_TraceSphere but also handles offsetting and rotation of the end points 
*           for moving and rotating entities. (Brush Models are the only rotating entities.)
**/
const TraceResult CM_TransformedSphereTrace( cm_t *collisionModel, const vec3_t &start, const vec3_t &end, const bbox3_t &bounds, const sphere_t &sphere, mnode_t *headNode, const int32_t brushContentsMask = 0, const glm::mat4 &matTransform = ph_mat_identity(), const glm::mat4 &matInvTransform = ph_mat_identity() ) {
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

	// Set our AABBTrace Bounds early on so _CM_Trace can calculate the rest from there.
	traceContext.aabbTrace.bounds = bounds;
	traceContext.sphereTrace.sphere = sphere;

	// Set 'Sphere' as our Trace Shape.
	traceContext.traceShape = TraceShape::Sphere;
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
		traceContext.traceType = CMHullType::World;
		traceContext.headNodeType = CMHullType::World;
		// Set to nullptr in case of World.
		traceContext.headNodeLeaf = nullptr;
	}

	// Transformed Trace.
	traceContext.isTransformedTrace = true;
	// Non-Nudged Fraction: the epsilon considers blockers with realFraction == 1, and the nudged fraction as < 1.0.
	traceContext.realFraction = 1.f + DIST_EPSILON;
	// Nudged Fraction.
	traceContext.traceResult.fraction = 1.f;
	// Defaults to null texture info surface. (The pointer expects a non nullptr value.)
	traceContext.traceResult.surface = &(CM_GetNullTextureInfo()->c);

	// Perform trace and return trace results.
	return _CM_Trace( traceContext );
}