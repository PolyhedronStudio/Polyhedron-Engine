/***
*
*	License here.
*
*	@file
*
*	Collision Model: AreaPortal API.
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

#include "Common/CollisionModel/AreaPortals.h"
#include "Common/CollisionModel/Testing.h"

//! Useful for debugging, turns all areas on.
extern cvar_t *map_noareas; // TODO: Let's have this not be extern.

/**
*   @brief
**/
static void FloodArea_r( cm_t *cm, int32_t number, int32_t floodnum ) {
	// Get area number to flood.
    marea_t *area = &cm->cache->areas[ number ];

	// See if the flood numbers match, and make sure it did not get 'reflooded'.
    if ( area->floodvalid == cm->floodValid ) {
        if ( cm->floodnums[ number ] == floodnum ) {
            return;
        }
        Com_Error(ErrorType::Drop, "FloodArea_r: reflooded");
    }

	// Update flood counts.
    cm->floodnums[ number ] = floodnum;
    area->floodvalid = cm->floodValid;

	// Iterate and flood all connected areas if their portals are open.
    mareaportal_t *p = area->firstareaportal;
    for ( int32_t i = 0; i < area->numareaportals; i++, p++ ) {
        if ( cm->portalopen[ p->portalnum ] ) {
            FloodArea_r( cm, p->otherarea, floodnum );
        }
    }
}

/**
*   @brief
**/
void FloodAreaConnections( cm_t *cm ) {
    // All current floods are now invalid
    cm->floodValid++;
    int32_t floodnum = 0;

    // Area 0 is not used
    for ( int32_t i = 1; i < cm->cache->numareas; i++ ) {
        marea_t *area = &cm->cache->areas[ i ];
        if ( area->floodvalid == cm->floodValid ) {
            continue;       // already flooded into
        }
        floodnum++;
        FloodArea_r( cm, i, floodnum );
    }
}

/**
*   @brief
**/
void CM_SetAreaPortalState( cm_t *cm, int32_t portalnum, qboolean open )
{
    if ( !cm->cache ) {
        return;
    }

    if ( portalnum < 0 || portalnum >= MAX_MAP_AREAPORTALS ) {
        Com_EPrintf( "%s: portalnum %d is out of range\n", __func__, portalnum );
        return;
    }

    // ignore areaportals not referenced by areas
    if ( portalnum > cm->cache->lastareaportal ) {
        Com_DPrintf( "%s: portalnum %d is not in use\n", __func__, portalnum );
        return;
    }

    cm->portalopen[ portalnum ] = open;
    FloodAreaConnections( cm );
}

/**
*   @brief
**/
qboolean CM_AreasConnected( cm_t *cm, int32_t area1, int32_t area2 ) {
    bsp_t *cache = cm->cache;

    if ( !cache ) {
        return false;
    }
    if ( map_noareas->integer ) {
        return true;
    }
    if ( area1 < 1 || area2 < 1 ) {
        return false;
    }
    if ( area1 >= cache->numareas || area2 >= cache->numareas ) {
        Com_EPrintf( "%s: area > numareas\n", __func__ );
        return false;
    }
    if ( cm->floodnums[ area1] == cm->floodnums[ area2 ] ) {
        return true;
    }

    return false;
}

/**
*   @brief  Writes a length byte followed by a bit vector of all the areas
*           that area in the same flood as the area parameter
*
*           This is used by the client refreshes to cull visibility
**/
int32_t CM_WriteAreaBits( cm_t *cm, byte *buffer, int32_t area ) {
    bsp_t *cache = cm->cache;

    if (!cache) {
        return 0;
    }

    int32_t bytes = (cache->numareas + 7) >> 3;

	// In case of no area, or in case of wishing to debug when 'map_noareas' is set, send everything.
    if ( map_noareas->integer || !area ) {
        memset( buffer, 255, bytes );
    } else {
        memset( buffer, 0, bytes );

        int32_t floodnum = cm->floodnums[ area ];
        for ( int32_t i = 0; i < cache->numareas; i++ ) {
            if ( cm->floodnums[i] == floodnum ) {
                Q_SetBit(buffer, i);
            }
        }
    }

    return bytes;
}

/**
*   @brief
**/
int32_t CM_WritePortalBits( cm_t *cm, byte *buffer ) {
    if ( !cm->cache ) {
        return 0;
    }

    int32_t numportals = min( cm->cache->lastareaportal + 1, MAX_MAP_PORTAL_BYTES << 3 );

    int32_t bytes = ( numportals + 7 ) >> 3;
    memset( buffer, 0, bytes );
    for ( int32_t i = 0; i < numportals; i++ ) {
        if ( cm->portalopen[ i ] ) {
            Q_SetBit( buffer, i );
        }
    }

    return bytes;
}

/**
*   @brief
**/
void CM_SetPortalStates( cm_t *cm, byte *buffer, int32_t bytes ) {
    if ( !cm->cache ) {
        return;
    }

    if ( !bytes ) {
        for ( int32_t i = 0; i <= cm->cache->lastareaportal; i++ ) {
            cm->portalopen[ i ] = true;
        }
    } else {
        int32_t numPortals = min(cm->cache->lastareaportal + 1, bytes << 3);
        int32_t i = 0;
        for ( i = 0; i < numPortals; i++ ) {
            cm->portalopen[ i ] = Q_IsBitSet( buffer, i );
        }
        for ( ; i <= cm->cache->lastareaportal; i++ ) {
            cm->portalopen[ i ] = true;
        }
    }

    FloodAreaConnections( cm );
}

/**
*   @return True if any leaf under headNode has a cluster that is potentially visible
**/
qboolean CM_HeadnodeVisible( mnode_t *node, byte *visbits ) {
    while ( node->plane ) {
        if ( CM_HeadnodeVisible(node->children[ 0 ], visbits ) ) {
            return true;
        }
        node = node->children[ 1 ];
    }

    mleaf_t *leaf = (mleaf_t *)node;
    int32_t cluster = leaf->cluster;
    if ( cluster == -1 ) {
        return false;
    }
    if ( Q_IsBitSet( visbits, cluster ) ) {
        return true;
    }
    return false;
}

/**
*   @brief  The client will interpolate the view position, so we can't use a single PVS point
**/
byte *CM_FatPVS( cm_t *cm, byte *mask, const vec3_t &org, int32_t vis ) {
    static byte    temp[VIS_MAX_BYTES];
    mleaf_t *leafs[64];
    int     clusters[64];
    uint_fast32_t *src, *dst;
    vec3_t  mins, maxs;

    if ( !cm->cache ) {   // map not loaded
        return (byte*)memset( mask, 0, VIS_MAX_BYTES ); // CPP: Cast
    }
    if ( !cm->cache->vis ) {
        return (byte*)memset( mask, 0xff, VIS_MAX_BYTES ); // CPP: Cast
    }

    for ( int32_t i = 0; i < 3; i++ ) {
        mins[ i ] = org[ i ] - 8;
        maxs[ i ] = org[ i ] + 8;
    }

	int32_t count = CM_BoxLeafs( cm, bbox3_t { mins, maxs }, leafs, 64, NULL );
    if ( count < 1 ) {
        Com_Error( ErrorType::Drop, "CM_FatPVS: leaf count < 1" );
    }
    int32_t longs = VIS_FAST_LONGS( cm->cache );

    // convert leafs to clusters
    for ( int32_t i = 0; i < count; i++ ) {
        clusters[i] = leafs[i]->cluster;
    }

    BSP_ClusterVis( cm->cache, mask, clusters[0], vis );

    // or in all the other leaf bits
    for ( int32_t i = 1; i < count; i++ ) {
        for ( int32_t j = 0; j < i; j++ ) {
            if ( clusters[ i ] == clusters[ j ] ) {
                goto nextleaf; // already have the cluster we want
            }
        }
        src = (uint_fast32_t *)BSP_ClusterVis( cm->cache, temp, clusters[i], vis );
        dst = (uint_fast32_t *)mask;
        for ( int32_t j = 0; j < longs; j++ ) {
            *dst++ |= *src++;
        }

nextleaf:;
    }

    return mask;
}