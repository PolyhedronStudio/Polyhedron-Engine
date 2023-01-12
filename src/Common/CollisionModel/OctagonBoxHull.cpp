/***
*
*	License here.
*
*	@file
*
*	Collision Model:	Octagon Box Hull API - Creates a headnode for tracing an octagonal shaped box
*						for non brush entities. Suits characters and barrels, or other circle like shaped
*						needs mostly.
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

#include "Common/CollisionModel/OctagonBoxHull.h"
#include "Common/CollisionModel/Tracing.h"



//! All round 'octagon hull' data, accessed in a few other CollisionModel files as extern.
OctagonHull octagonHull = {};

/**
*   @brief  Set up the planes and nodes so that the 10 floats of an octagon box
*           can just be stored out and get a proper clipping hull structure.
**/
void CM_InitOctagonBoxHull() {
    octagonHull.headNode = &octagonHull.nodes[0];

    octagonHull.brush.numsides = 10;
    octagonHull.brush.firstbrushside = &octagonHull.brushSides[0];
    octagonHull.brush.contents = BrushContents::Monster;

    octagonHull.leaf.firstleafbrush = &octagonHull.leafBrush;
    octagonHull.leaf.numleafbrushes = 1;
    octagonHull.leaf.contents = BrushContents::Monster;

    octagonHull.leafBrush = &octagonHull.brush;

    for (int32_t i = 0; i < 6; i++) {
        // Determine side.
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &octagonHull.brushSides[i];
        brushSide->plane = &octagonHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &octagonHull.nodes[i];
        node->plane = &octagonHull.planes[i * 2];
        node->children[side] = (mnode_t *)&octagonHull.emptyLeaf;
        node->children[side ^ 1] = &octagonHull.nodes[i + 1];

        // Plane A.
        CollisionPlane *plane = &octagonHull.planes[i * 2];
        plane->type = (i >> 1);
        plane->normal[(i >> 1)] = 1;
		SetPlaneType( plane );
		SetPlaneSignbits( plane );
        // Plane B.
        plane = &octagonHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->signBits = (1 << (i >> 1));
        plane->normal[(i >> 1)] = -1;
		SetPlaneType( plane );
		SetPlaneSignbits( plane );
	}

    const vec3_t oct_dirs[4] = { { 1, 1, 0 }, { -1, 1, 0 }, { -1, -1, 0 }, { 1, -1, 0 } };

    for (int32_t i = 6; i < 10; i++) {
        int32_t side = (i & 1);

        // Setup Brush Sides.
        mbrushside_t *brushSide = &octagonHull.brushSides[i];
        brushSide->plane = &octagonHull.planes[i * 2 + side];
        brushSide->texinfo = CM_GetNullTextureInfo();

        // Setup Box Nodes.
        mnode_t *node = &octagonHull.nodes[i];
        node->plane = &octagonHull.planes[i * 2];
        node->children[side] = (mnode_t *)&octagonHull.emptyLeaf;
        if (i != 9) {
            node->children[side ^ 1] = &octagonHull.nodes[i + 1];
        } else {
            node->children[side ^ 1] = (mnode_t *)&octagonHull.leaf;
        }

        // Plane A.
        CollisionPlane *plane = &octagonHull.planes[i * 2];
        plane->type = 3;// + (i >> 1);
        plane->normal = oct_dirs[i - 6];
        SetPlaneType(plane);
        SetPlaneSignbits(plane);

        // Plane B.
        plane = &octagonHull.planes[i * 2 + 1];
        plane->type = 3 + (i >> 1);
        plane->normal = oct_dirs[(i - 6)];
        //plane->signBits = (1 << (i >> 1)); //SetPlaneSignbits(plane);
        SetPlaneType(plane);
        SetPlaneSignbits(plane);
    }
}

/**
*   @brief  Utility function to complement CM_HeadnodeForOctagon with.
**/
static inline float CalculateOctagonPlaneDist( CollisionPlane &plane, const vec3_t &mins, const vec3_t &maxs, bool negate = false ) {
    if (negate == true) {
        return vec3_dot( plane.normal, {(plane.signBits & 1) ? -mins[0] : -maxs[0], (plane.signBits & 2) ? -mins[1] : -maxs[1], (plane.signBits & 4) ? -mins[2] : -maxs[2]});//-d;//d;
    } else {
        return vec3_dot( plane.normal, {(plane.signBits & 1) ? mins[0] : maxs[0], (plane.signBits & 2) ? mins[1] : maxs[1], (plane.signBits & 4) ? mins[2] : maxs[2]});//-d;//d;
    }
}

/**
*   @brief  To keep everything totally uniform, bounding boxes are turned into small
*           BSP trees instead of being compared directly.
**/
mnode_t* CM_HeadnodeForOctagon( const bbox3_t &bounds, const int32_t contents = BrushContents::Solid ) {
	// Set Brush and Leaf contents
	octagonHull.brush.contents = contents;
	octagonHull.leaf.contents = contents;
		
	// Calculate half size for mins and maxs.
	const vec3_t offset = vec3_scale( bounds.mins + bounds.maxs, 0.5f );

    const vec3_t centerSize[2] = {
        bounds.mins - offset, // Not sure why but this --> mins - offset, // was somehow not working well.
        bounds.maxs - offset, // Not sure why but this --> maxs - offset, // was somehow not working well.
    };
	
	const vec3_t halfSize[2] = {
		vec3_scale( bounds.mins, 0.5f ),
		vec3_scale( bounds.maxs, 0.5f )
	};

	//const vec3_t mins = bounds.mins;//[0];
	//const vec3_t maxs = bounds.maxs;//size[1];
	const vec3_t mins = bounds.mins;//bounds.mins;
	const vec3_t maxs = bounds.maxs;//bounds.maxs;
	
	octagonHull.headNode->bounds = octagonHull.leaf.bounds = bounds;

    // Setup the box distances.
    octagonHull.planes[0].dist = maxs[0];
    octagonHull.planes[1].dist = -maxs[0];
    octagonHull.planes[2].dist = mins[0];
    octagonHull.planes[3].dist = -mins[0];
    octagonHull.planes[4].dist = maxs[1];
    octagonHull.planes[5].dist = -maxs[1];
    octagonHull.planes[6].dist = mins[1];
    octagonHull.planes[7].dist = -mins[1];
    octagonHull.planes[8].dist = maxs[2];
    octagonHull.planes[9].dist = -maxs[2];
    octagonHull.planes[10].dist = mins[2];
    octagonHull.planes[11].dist = -mins[2];

    // Calculate actual up to scale normals for the non axial planes.
	const float a = halfSize[1][0]; // Half-X
	const float b = halfSize[1][1]; // Half-Y
	//const float a = size[ 1 ][0]; // Half-X
	//const float b = size[ 1 ][1]; // Half-Y
	float dist = sqrt( a * a + b * b ); // Hypothenuse

	float cosa = a / dist;
	float sina = b / dist;

	// Calculate and set distances for each non axial plane.
    octagonHull.planes[12].normal   = vec3_t{cosa, sina, 0.f};
    octagonHull.planes[12].dist     = CalculateOctagonPlaneDist(octagonHull.planes[12], halfSize[0], halfSize[1]);
    //octagonHull.planes[12].dist     = CalculateOctagonPlaneDist(octagonHull.planes[12], mins, maxs );
    octagonHull.planes[13].normal   = vec3_t{cosa, sina, 0.f};
    octagonHull.planes[13].dist     = CalculateOctagonPlaneDist(octagonHull.planes[13], halfSize[0], halfSize[1], true);
    //octagonHull.planes[13].dist     = CalculateOctagonPlaneDist(octagonHull.planes[13], mins, maxs, true);

    octagonHull.planes[14].normal   = vec3_t{-cosa, sina, 0.f};
    octagonHull.planes[14].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[14], halfSize[0], halfSize[1], true);
    //octagonHull.planes[14].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[14], mins, maxs, true);
    octagonHull.planes[15].normal   = vec3_t{-cosa, sina, 0.f};
    octagonHull.planes[15].dist     = CalculateOctagonPlaneDist(octagonHull.planes[15], halfSize[0], halfSize[1]);
    //octagonHull.planes[15].dist     = CalculateOctagonPlaneDist(octagonHull.planes[15], mins, maxs);

    octagonHull.planes[16].normal   = vec3_t{-cosa, -sina, 0.f};
    octagonHull.planes[16].dist     = CalculateOctagonPlaneDist(octagonHull.planes[16], halfSize[0], halfSize[1]);
    //octagonHull.planes[16].dist     = CalculateOctagonPlaneDist(octagonHull.planes[16], mins, maxs);
    octagonHull.planes[17].normal   = vec3_t{-cosa, -sina, 0.f};
    octagonHull.planes[17].dist     = CalculateOctagonPlaneDist(octagonHull.planes[17], halfSize[0], halfSize[1], true);
    //octagonHull.planes[17].dist     = CalculateOctagonPlaneDist(octagonHull.planes[17], mins, maxs, true);

    octagonHull.planes[18].normal   = vec3_t{cosa, -sina, 0.f};
    octagonHull.planes[18].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[18], halfSize[0], halfSize[1], true);
    //octagonHull.planes[18].dist     = -CalculateOctagonPlaneDist(octagonHull.planes[18], mins, maxs, true);

    octagonHull.planes[19].normal   = vec3_t{cosa, -sina, 0.f};
    octagonHull.planes[19].dist     = CalculateOctagonPlaneDist(octagonHull.planes[19], halfSize[0], halfSize[1]);
    //octagonHull.planes[19].dist     = CalculateOctagonPlaneDist(octagonHull.planes[19], mins, maxs);
    
	// Cheers, we made it to this point, enjoy the new octagonHull :)
    return octagonHull.headNode;
}