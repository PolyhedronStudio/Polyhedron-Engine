/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
// cmodel.c -- model loading

#include "Shared/Shared.h"

#include "Common/Bsp.h"
#include "Common/Cmd.h"
#include "Common/CVar.h"
#include "Common/CollisionModel.h"
#include "Common/Common.h"
#include "Common/Zone.h"
#include "System/Hunk.h"

#include "Common/CollisionModel/AreaPortals.h"
#include "Common/CollisionModel/BoundingBoxHull.h"
#include "Common/CollisionModel/SphereHull.h"
#include "Common/CollisionModel/CapsuleHull.h"
#include "Common/CollisionModel/OctagonBoxHull.h"

/**
*   CollisionModel data.
**/
//! Useful for debugging, turns all areas on.
cvar_t *map_noareas = nullptr; // TODO: Let's have this not be extern.



/**
*
*
* 
*   General Collision Model Utility Functions.
*
* 
*
**/
/**
*	@return	The transformed by 'matrix' recalculated 'plane'. (distance, normal, signbits and type)
**/
CollisionPlane CM_TransformPlane( CollisionPlane *plane, const glm::mat4 &transformMatrix ) {
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
*	@return	The translated by 'matrix' recalculated 'plane'. (distance, normal, signbits and type)
**/
CollisionPlane CM_TranslatePlane( CollisionPlane *plane, const glm::mat4 &translateMatrix ) {
	CollisionPlane transformedPlane = *plane;

	glm::vec3 translate = translateMatrix[3];

	const glm::mat4 transformMatrix = glm::translate( ph_mat_identity(), translate );

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
const vec3_t CM_ProjectPointOntoVector( const vec3_t vPoint, const vec3_t vStart, const vec3_t vDir ) {
	
	// Project onto the directional vector for this segment.
	const vec3_t projectVector = vPoint - vStart;						//VectorSubtract( point, vStart, pVec );
	return vec3_fmaf( vStart, vec3_dot( projectVector, vDir ), vDir );	//VectorMA( vStart, DotProduct( pVec, vDir ), vDir, vProj );
}

/**
*	@brief	Point distance from line.
**/
const float CM_DistanceFromLineSquared( const vec3_t &p, const vec3_t &lp1, const vec3_t &lp2, const vec3_t &dir )
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
*	@brief	Transforms the bounds' mins and maxs by the matrix.
**/
const bbox3_t CM_Matrix_TransformBounds( const glm::mat4 &matrix, const bbox3_t &bounds ) {
	// Convert box to points that we can operate with.
	static vec3_t points[8];
	bbox3_to_points( bounds, points );

	// Set box to infinite to append it with points.
	bbox3_t transformedBounds = bbox3_infinity();
	
	// Get translation point for non identities.
	for ( int32_t i = 0; i < 8; i++ ) {
		glm::vec3 point = phvec_to_glmvec3( points[i] ); 
		glm::vec4 transformedPoint = matrix * glm::vec4( point.x, point.y, point.z, 1 );
		point = glm::vec3( transformedPoint.x / transformedPoint.w, transformedPoint.y / transformedPoint.w, transformedPoint.z / transformedPoint.w );

		// Append bounds.
		transformedBounds = bbox3_append( transformedBounds, glmvec3_to_phvec( point ) );
	}

	// Return transformed bounds.
	return transformedBounds;
}

/**
*	@brief	Transforms the sphere's origin by the matrix.
**/
const sphere_t CM_Matrix_TransformSphere( const glm::mat4 &matrix, const sphere_t &sphere ) {
	sphere_t transformedSphere = sphere;

	glm::vec4 transformedOrigin = matrix * phvec_to_glmvec4( transformedSphere.origin ); //CM_Matrix_TransformBounds( traceContext.matTransform, transformedTestBounds );
	glm::vec3 vTransformedOrigin = glm::vec3( transformedOrigin.x / transformedOrigin.w, transformedOrigin.y / transformedOrigin.w, transformedOrigin.z / transformedOrigin.w );
	transformedSphere.origin = glmvec3_to_phvec( vTransformedOrigin );

	// Return transformed bounds.
	return transformedSphere;
}

/**
*	@return	The box transformed by 'matrix', and expands the box 1.f in case of Solid::BSP
**/
const bbox3_t CM_EntityBounds( const uint32_t solid, const glm::mat4 &matrix, const bbox3_t &bounds ) {
	// Transform and set bounds.
	bbox3_t transformedBounds = CM_Matrix_TransformBounds( matrix, bounds );
	// Add an offset to BSP brushes.
	// TODO: solid == 31, should be client specific and check for PACKED_BSP
	if ( solid == Solid::BSP || solid == 31 ) {
		transformedBounds = bbox3_expand( transformedBounds, CM_BOUNDS_EPSILON );
	}
	return transformedBounds;

	// Transform and set bounds.
	//bbox3_t transformedBounds = CM_Matrix_TransformBounds( matrix, bounds );

	// Add an offset to BSP brushes.
	// TODO: solid == 31, should be client specific and check for PACKED_BSP
	//if ( solid == Solid::BSP || solid == 31U ) {
	//	return CM_Matrix_TransformBounds( matrix, bbox3_expand( bounds, CM_BOUNDS_EPSILON ) );
	//} else {
	//	return CM_Matrix_TransformBounds( matrix, bounds );
	//}
}

/**
*
*
* 
*   Initialize.
*
* 
*
**/
/**
*   @brief Initializes the collision model subsystem, preparing box hulls, nullLeafs/textureInfo.
**/
void CM_Init() {
	// Initialize hulls for trace clipping non brush entities with.
    CM_InitBoxHull();
	CM_InitSphereHull();
	CM_InitCapsuleHull();
    CM_InitOctagonBoxHull();

	// Useful for debugging, turns all areas on.
    map_noareas = Cvar_Get("map_noareas", "0", 0);
}

/**
*
*
* 
*   BSP.
*
* 
*
**/
/**
*   @brief  Loads in the BSP map and its "submodels".
**/
qerror_t CM_LoadMap( cm_t *cm, const char *name )
{
    bsp_t *cache;

    qerror_t ret = BSP_Load(name, &cache);
    if (!cache) {
        return ret;
    }

    cm->cache = cache;
    cm->floodnums = (int*)Z_TagMallocz( sizeof( int ) * cm->cache->numareas + sizeof( qboolean ) * ( cm->cache->lastareaportal + 1 ), TAG_CMODEL );
    cm->portalopen = (qboolean *)( cm->floodnums + cm->cache->numareas );
    FloodAreaConnections(cm);

    return Q_ERR_SUCCESS;
}
/**
*   @brief  Frees the map and all of its "submodels".
**/
void CM_FreeMap( cm_t *cm ) {
	// Free up floodnums memory.
    if ( cm->floodnums ) {
        Z_Free( cm->floodnums );
    }

	// Free up BSP.
    BSP_Free( cm->cache );

	// TODO: Oh boy, memset...
    memset( cm, 0, sizeof( *cm ) );
}

/**
*   @brief  Creates the collision model using the already loaded BSP as its cache.
**/
const cm_t &&CM_CreateFromBSP( bsp_t *bsp, const char *name ) {
	cm_t cm{
		.cache = bsp,
	};

    cm.floodnums = (int*)Z_TagMallocz(sizeof(int) * cm.cache->numareas		+		sizeof(qboolean) * (cm.cache->lastareaportal + 1), TAG_CMODEL);
    cm.portalopen = (qboolean *)(cm.floodnums + cm.cache->numareas);
    FloodAreaConnections(&cm);

    return std::move(cm);
}
/**
*	@brief	Frees the floodnums from memory after unsetting the cache pointer.
**/
void CM_FreeFromBSP( cm_t *cm ) {
	if ( !cm ) {
		// Error?
		return;
	}

	// Unset BSP pointer to free the collision model cache.
	cm->cache = nullptr;

	// Free floodnums.
	if ( cm->floodnums ) {
		Z_Free( cm->floodnums );
	}
}

/**
*   @return Pointer to a valid node matching 'number'.
**/
mnode_t *CM_NodeNum( cm_t *cm, int number )
{
    if ( !cm->cache ) {
        return (mnode_t *)CM_GetNullLeaf();
    }
    if ( number == -1 ) {
        return (mnode_t *)cm->cache->leafs;   // special case for solid leaf
    }
    if ( number < 0 || number >= cm->cache->numnodes ) {
        Com_EPrintf( "%s: bad number: %d\n", __func__, number );
        return (mnode_t *)CM_GetNullLeaf();
    }
    return cm->cache->nodes + number;
}

/**
*	@return	Used as a 'nullptr' for scenarios where a leaf is null.
**/
mleaf_t *CM_GetNullLeaf() {
	static mleaf_t nullLeaf = { .cluster = -1 };
	return &nullLeaf;
}
/**
*   @return Pointer to a valid leaf matching 'number'.
**/
mleaf_t *CM_LeafNum( cm_t *cm, int number ) {
	// No map loaded, return null leaf ptr.
    if ( !cm->cache ) {
        return CM_GetNullLeaf();
    }
	// Ensure leaf number is within bounds, otherwise return null leaf ptr.
    if ( number < 0 || number >= cm->cache->numleafs ) {
        Com_EPrintf( "%s: bad number: %d\n", __func__, number );
        return CM_GetNullLeaf();
    }
	// Return matching leaf by number.
    return cm->cache->leafs + number;
}

/**
*   @return Pointer to the leaf matching vec3 'p'. Nullptr if it is called without a map loaded.
**/
mleaf_t *CM_PointLeaf( cm_t *cm, const vec3_t &p ) {
	// The actually might call this without map loaded.
    if ( !cm->cache ) {
        return CM_GetNullLeaf();       
    }
	// Find and return pointer to a matching leaf for point 'p'.
    return BSP_PointLeaf( cm->cache->nodes, p );
}

/**
*	@return	Used as a 'nullptr' for scenarios where there is no surface texture info available.
**/
mtexinfo_t *CM_GetNullTextureInfo() {
	static mtexinfo_t  nullTextureInfo = {};
	return &nullTextureInfo;
}