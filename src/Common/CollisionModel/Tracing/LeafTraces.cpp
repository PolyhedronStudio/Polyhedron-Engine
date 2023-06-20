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
void CM_TraceBox_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );
void CM_TraceLineThroughSphere( TraceContext &traceContext, const vec3_t &origin, const vec3_t &offset, const float radius, const vec3_t &start, const vec3_t &end, const int32_t leafContents );
void CM_TraceAABBThroughSphere( TraceContext &traceContext, const vec3_t &sphereOrigin, const vec3_t &sphereOffset, const float sphereRadius, const vec3_t &start, const vec3_t &end, const int32_t leafContents = 0 );
void CM_TraceSphere_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf );


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
		if ( traceContext.traceShape == TraceShape::Sphere ) {
			CM_TraceSphere_TraceThroughBrush( traceContext, brush, leaf );
		// Trace: 'TraceBox' -> 'Box Brush' 
		// Trace: 'TraceBox' -> 'World Brush' 
		} else {
			CM_TraceBox_TraceThroughBrush( traceContext, brush, leaf );
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
*
*
*	'TraceBox' Leaf Traces:
*
*
**/
/**
*   @brief	Trace 'TraceBox' through Leaf.
**/
void CM_TraceBox_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
	if ( leaf->shapeType == CMHullType::Sphere ) {
		CM_TraceBox_TraceThroughSphereLeaf( traceContext, leaf );
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
		CM_TraceBox_TraceThroughBrush( traceContext, brush, leaf );

		// Return if we didn't collide.
		//if ( traceContext.traceResult.allSolid ) {
		if ( !traceContext.traceResult.fraction ) {
			traceContext.realFraction = 0.f;
		    return;
		}
    }
}

/**
*   @brief 
**/
void CM_TraceBox_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	/**
	*	Ensure our absolute trace bounds intersect with the sphere leaf's bounds.
	**/
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


	/**
	*	Calculate exact start/end trace points for the sphere's offsetOrigins.
	**/	
	//const vec3_t aabbOrigin			 = 
	const vec3_t aabbTraceStart			 = traceContext.start;
	const vec3_t aabbTraceEnd			 = traceContext.end;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = transformedLeafSphere.origin + transformedLeafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Total test radius. ( Seems to work. )
	const float testRadius = ( traceContext.sphereTrace.transformedSphere.radius + CM_RAD_EPSILON ) + ( transformedLeafSphere.radius + CM_RAD_EPSILON );

	/**
	*	Perform the 'AABB Through Sphere' ('brush')-trace.
	**/
	CM_TraceAABBThroughSphere( traceContext, leafSphereOffsetOrigin, transformedLeafSphere.offset, testRadius, aabbTraceStart, aabbTraceEnd, leaf->contents );
}

/**
*   @brief 
**/
void CM_TraceBox_ThroughCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	return;
}



/**
*
*
*	'TraceSphere' Leaf Traces:
*
*
**/
/**
*   @brief	Trace 'TraceSphere' through Leaf.
**/
void CM_TraceSphere_TraceThroughLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
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
	if ( !CM_TraceIntersectBounds( traceContext, leaf->bounds ) ) {
		return;
	}
	//sphere_t transformedTraceSphere = traceContext.sphereTrace.transformedSphere;
	//if ( !bbox3_intersects_sphere( traceContext.absoluteBounds, transformedTraceSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON, true ) ) {
	//	return;
	//}

	/**
	*
	**/
	// Trace: 'TraceSphere' -> 'Sphere 'Leaf Brush' 
	if ( leaf->shapeType == CMHullType::Sphere ) {
		// Trace line against all brushes in the leaf
		mbrush_t **leafBrush = leaf->firstleafbrush;

		if ( leafBrush ) {
		   mbrush_t *brush = *leafBrush;

			// Skip the brush('continue') if we've already checked this brush in another leaf
			if ( brush->checkcount == traceContext.collisionModel->checkCount ) {
				return;   
			}
        
			// Assign current check count.
			brush->checkcount = traceContext.collisionModel->checkCount;

			// Skip the brush('continue') if the contents mask does not match to that of our trace.
			if ( !( brush->contents & traceContext.contentsMask ) ) {
				return;
			}
		}

		CM_TraceSphere_TraceThroughSphereLeaf( traceContext, leaf );
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
		CM_TraceSphere_TraceThroughBrush( traceContext, brush, leaf );

		// Return if we didn't collide.
		//if ( traceContext.traceResult.allSolid ) {
		if ( !traceContext.traceResult.fraction ) {
			traceContext.realFraction = 0.f;
		    return;
		}
    }
}

/**
*   @brief 
**/
//void CM_TraceSphereLeafThroughSphere( TraceContext &traceContext, mleaf_t *leaf ) {
void CM_TraceSphere_TraceThroughSphereLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	/**
	*	Ensure our absolute trace bounds intersect with the sphere leaf's bounds.
	**/
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
	if ( !sphere_intersects_bbox3( traceContext.absoluteBounds, transformedLeafSphere, bbox3_t::IntersectType::SolidBox_HollowSphere, CM_RAD_EPSILON, true ) ) {
		return;
	}


	/**
	*	Calculate exact start/end trace points for the sphere's offsetOrigins.
	**/	
	sphere_t traceSphere = traceContext.sphereTrace.transformedSphere;

	const vec3_t traceSphereOffsetOrigin = traceSphere.origin + traceSphere.offset;
	const vec3_t traceSphereStart		 = traceContext.start + traceSphereOffsetOrigin;// - traceSphereOffsetOrigin;// + traceContext.traceSphere.offset; //+ traceSphereOffsetOrigin;
	const vec3_t traceSphereEnd			 = traceContext.end + traceSphereOffsetOrigin;// - traceSphereOffsetOrigin; //traceContext.traceSphere.offset;// - traceSphereOffsetOrigin;

	// Leaf Sphere.
	const vec3_t leafSphereOffsetOrigin = leafSphere.origin + leafSphere.offset;
	const vec3_t leafSphereStart		= traceContext.start + leafSphereOffsetOrigin; //leafSphere.offset;// + traceSphereOffsetOrigin;
	const vec3_t leafSphereEnd			= traceContext.end + leafSphereOffsetOrigin; //leafSphere.offset;// - traceSphereOffsetOrigin;
	
	// Total test radius.
	const float testRadius = ( traceContext.sphereTrace.transformedSphere.radius + CM_RAD_EPSILON ) + ( transformedLeafSphere.radius + CM_RAD_EPSILON );

	// Now perform sphere trace.
	CM_TraceLineThroughSphere( traceContext, traceSphereOffsetOrigin, traceSphere.offset, testRadius, leafSphereStart, leafSphereEnd, leaf->contents );
	//CM_TraceLineThroughSphere( traceContext, leafSphereOffsetOrigin, leafSphere.offset, testRadius, traceSphereStart, traceSphereEnd, leaf->contents );
}



/**
*
*
*	'TraceCapsule' Leaf Traces:
*
*
**/
/**
*   @brief 
**/
void CM_TraceCapsule_ThroughCapsuleLeaf( TraceContext &traceContext, mleaf_t *leaf ) {
	return;
}