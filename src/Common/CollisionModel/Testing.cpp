/***
*
*	License here.
*
*	@file
*
*	Collision Model:	'Area Boxing' API: Allows for acquiring all nodes residing in the specified bounds area.
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
#include "Common/CollisionModel/OctagonBoxHull.h"
#include "Common/CollisionModel/Testing.h"
#include "Common/CollisionModel/Tracing.h"



//! Needed/
extern BoxHull boxHull;
//! Needed/
extern OctagonHull octagonHull;

/**
*   BoxLeaf 'Work'
**/
static struct BoxLeafsWork {
    //! Leaf count.
    int32_t leafCount = 0;
    //! Max leaf count.
    int32_t leafMaximumCount = 0;
    //! Leaf List.
    mleaf_t  **leafList = nullptr;
    ////! Mins of Leaf.
    //const float *leafMins = nullptr;
    ////! Maxs of Leaf.
    //const float *leafMaxs = nullptr;
	bbox3_t leafBounds = bbox3_zero();
    //! Top node of Leaf.
    mnode_t  *leafTopNode = nullptr;
} boxLeafsWork;


//
//	TODO: Remove these after having "mainstreamed/unified" our sorry, mixed up vector maths.
//	The implementations reside in /Common/CollisionModel.cpp
//
void CM_AngleVectors( const vec3_t &angles, vec3_t &forward, vec3_t &right, vec3_t &up );
void CM_AnglesToAxis( const vec3_t &angles, vec3_t axis[3] );
void CM_Matrix_TransformVector( vec3_t m[3], const vec3_t &v, vec3_t &out );
 const vec3_t glmvec3_to_phvec( const glm::vec3 &glmv );
  const glm::vec3 phvec_to_glmvec3( const vec3_t &phv );


/**
*   @brief  Fills in a list of all the leafs touched
**/
static void CM_BoxLeafs_r( mnode_t *node ) {
    while ( node->plane ) {
        int32_t s = BoxOnPlaneSideFast( boxLeafsWork.leafBounds, node->plane );
        if ( s == 1 ) {
            node = node->children[ 0 ];
        } else if ( s == 2 ) {
            node = node->children[ 1 ];
        } else {
            // go down both
            if ( !boxLeafsWork.leafTopNode ) {
                boxLeafsWork.leafTopNode = node;
            }
            CM_BoxLeafs_r( node->children[ 0 ] );
            node = node->children[ 1 ];
        }
    }

    if ( boxLeafsWork.leafCount < boxLeafsWork.leafMaximumCount ) {
        boxLeafsWork.leafList[ boxLeafsWork.leafCount++ ] = (mleaf_t *)node;
    }
}

/**
*   @brief  
**/
const int32_t CM_BoxLeafs_headnode( const bbox3_t &bounds, mleaf_t **list, const int32_t listSize, mnode_t *headNode, mnode_t **topNode ) {
    boxLeafsWork.leafList   = list;
    boxLeafsWork.leafCount  = 0;
    boxLeafsWork.leafMaximumCount = listSize;
    boxLeafsWork.leafBounds	= bounds;

    boxLeafsWork.leafTopNode = nullptr;

    CM_BoxLeafs_r( headNode );

    if ( topNode ) {
        *topNode = boxLeafsWork.leafTopNode;
    }

    return boxLeafsWork.leafCount;
}

/**
*   @brief  
**/
const int32_t CM_BoxLeafs( cm_t *cm, const bbox3_t &bounds, mleaf_t **list, const int32_t listSize, mnode_t **topNode ) {
	// No map loaded means no leafs found, return 0.
    if ( !cm || !cm->cache ) {
        return 0;
	}

	// Otherwise select all leafs that match our bounding box head node.
    return CM_BoxLeafs_headnode( bounds, list, listSize, cm->cache->nodes, topNode );
}

/**
*   @brief  Check for what brush contents reside at vec3 'p' inside given node list.
**/
const glm::mat4 ph_mat_identity();
int32_t CM_PointContents( cm_t *cm, const vec3_t &p, mnode_t *headNode, const glm::mat4 &matInvTransform ) {
	// Need a head node.
	if ( !headNode ) {
        return 0;
    }

	// No map loaded means no leafs found, return 0.
    if ( !cm || !cm->cache ) {
        return 0;
	}

    // subtract origin offset
    vec3_t point = p;

    // Octagon Cylinder offset.
	//if ( headNode == octagonHull.headNode ) {
	//       point -= traceWork.cylinderOffset;
	//   } else if ( headNode == boxHull.headNode ) {
	//       point -= traceWork.cylinderOffset;
	//   }

	if ( matInvTransform != ph_mat_identity() ) {
		glm::vec4 transformedPoint = matInvTransform * glm::vec4( phvec_to_glmvec3( p ), 1. );
		point = glmvec3_to_phvec( transformedPoint );
	}

    mleaf_t *leaf = BSP_PointLeaf( headNode, point );
	if ( !leaf ) {
		return 0;
	}

    return leaf->contents;
}