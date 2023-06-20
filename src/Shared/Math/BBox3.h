/***
*
*	License here.
*
*	@file
*
*	BoundingBox(vec3_t based) class and operation implementations.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"


/**
*	@brief Box 3 type definiton: (mins, maxs). The bounds are implemented like a union class.
**/
struct bbox3_t {
    union
    {
        // XYZ array index accessor.
        vec3_t bounds[2];

        // X Y Z desegnator accessors.
        struct {
            vec3_t mins;
			vec3_t maxs;
        };
    };

	/**
	*	@brief	Specific Intersection Test Types for use with bbox3_intersects_sphere.
	**/
	struct IntersectType {
		// Box VS Sphere Types:
		static constexpr int32_t HollowBox_HollowSphere = 0;
		static constexpr int32_t HollowBox_SolidSphere = 1;
		static constexpr int32_t SolidBox_HollowSphere = 2;
		static constexpr int32_t SolidBox_SolidSphere = 3;
	};

    /**
    *	Constructors.
    **/
    // Default.
    bbox3_t() { mins = vec3_zero(); maxs = vec3_zero(); }

    // Assign.
	bbox3_t(const vec3_t &boxMins, const vec3_t &boxMaxs ) { mins = boxMins; maxs = boxMaxs; }

    // Regular *vec_t support.
    bbox3_t(vec3_t* vec) { mins = vec[0]; maxs = vec[1]; }
	bbox3_t(const vec3_t* vec) { mins = vec[0]; maxs = vec[1]; }
	bbox3_t(const float *vec) { mins = { vec[0], vec[1], vec[2] }; maxs = { vec[3], vec[4], vec[5] }; }

    /**
    *	Operators
    **/    
	// Pointer.
    inline operator float* () { 
        return &mins[0];
    }
    inline operator vec3_t* () { 
        return &mins;
    }
    
    // Pointer cast to const float*
    inline operator const float* () const { 
        return &mins[0];
    }
    inline operator const vec3_t* () const { 
        return &mins;
    }

    //// OPERATOR: + vec3_template
    //inline vec3_template operator +(const vec3_template& operand) const
    //{
    //    return vec3_template{
    //        x + operand.x,
    //        y + operand.y,
    //        z + operand.z
    //    };
    //}

    //// OPERATOR: - vec3_template
    //inline vec3_template operator -(const vec3_template& operand) const
    //{
    //    return vec3_template{
    //        x - operand.x,
    //        y - operand.y,
    //        z - operand.z
    //    };
    //}

    //// OPERATOR: / vec3_template
    //inline vec3_template operator /(const vec3_template& operand) const
    //{
    //    return vec3_template{
    //        x / operand.x,
    //        y / operand.y,
    //        z / operand.z
    //    };
    //}

    //// OPERATOR: * vec3_template
    //inline vec3_template operator *(const vec3_template& operand) const
    //{
    //    return vec3_template{
    //        x * operand.x,
    //        y * operand.y,
    //        z * operand.z
    //    };
    //}
    //// OPERATOR: * float
    //inline vec3_template operator *(const float operand) const
    //{
    //    return vec3_template{
    //        x * operand,
    //        y * operand,
    //        z * operand
    //    };
    //}

    //// OPERATOR: -= vec3_template
    //inline const vec3_template& operator -=(const vec3_template& operand) {
    //    x -= operand.x;
    //    y -= operand.y;
    //    z -= operand.z;
    //    return *this;
    //}

    ////// OPERATOR: += vec3_template
    //inline const vec3_template& operator +=(const vec3_template& operand) {
    //    x += operand.x;
    //    y += operand.y;
    //    z += operand.z;
    //    return *this;
    //}

    ////// OPERATOR: /= vec3_template
    //inline const vec3_template& operator *=(const vec3_template& operand) {
    //    x *= operand.x;
    //    y *= operand.y;
    //    z *= operand.z;
    //    return *this;
    //}

    ////// OPERATOR: *= vec3_template
    //inline const vec3_template& operator /=(const vec3_template& operand) {
    //    x /= operand.x;
    //    y /= operand.y;
    //    z /= operand.z;
    //    return *this;
    //}
};


//
//=============================================================================
// 
// Modern vec3_t Inline Functions:
//
//=============================================================================
//

/**
*	@brief	Constructs a vec3_zero centered matching bounding box from the size vector.
*	@return A bbox3 containing the correct mins and maxs matching a zero center origin.
**/
inline const bbox3_t bbox3_from_size( const vec3_t &size ) {
	return bbox3_t{
		vec3_scale( size, -0.5f ),
		vec3_scale( size, 0.5f )
	};
}
/**
*	@brief	Constructs a vec3_zero centered matching bounding box from the x,y,z values.
*	@return A bbox3 containing the correct mins and maxs matching a zero center origin.
**/
inline const bbox3_t bbox3_from_size( const float x, const float y, const float z ) {
	return bbox3_from_size( { x, y, z } );
}

/**
*	@return A zero sized box.
**/
inline const bbox3_t bbox3_zero( ) {
	return bbox3_t{
		vec3_zero(),
		vec3_zero()
	};
}
/**
*	@brief	Constructs an INFINITY sized box which can be used to add points to (ie it scaled down to),
*			ensuring that it remains centered around its zero point.
*	@return A box sized to INFINITY.
**/
inline const bbox3_t bbox3_infinity( ) {
	return bbox3_t{
		{ INFINITY, INFINITY, INFINITY },
		{ -INFINITY, -INFINITY, -INFINITY }
	};
}

/**
*	@return	A box with extended bounds if, point < mins, or point > maxs.
**/
inline const bbox3_t bbox3_append( const bbox3_t &bbox, const vec3_t &point ) {
	return bbox3_t {
		vec3_minf( bbox.mins, point ),
		vec3_maxf( bbox.maxs, point )
	};
}
/**
*	@return	A box that 'unites' both into one.
**/
inline const bbox3_t bbox3_union(const bbox3_t &bboxA, const bbox3_t &bboxB) {
	return bbox3_t{
		vec3_minf( bboxA.mins, bboxB.mins ),
		vec3_maxf( bboxA.maxs, bboxB.maxs ),
	};
}

/**
*	@return	A bbox constructed out of the list of points.
**/
inline const bbox3_t bbox3_from_points( const vec3_t *points, const uint32_t numberOfPoints ) {
	// Construct an infinite sized box to work from.
	bbox3_t bbox = bbox3_infinity();

	// Append with found points.
	for ( uint32_t i = 0; i < numberOfPoints; i++, points++ ) {
		bbox = bbox3_append( bbox, *points );//points[i] );
	}

	// Return.
	return bbox;
}
/**
*	@brief	Writes the eight corner points of the bounding box to "points".
*			It must be at least 8 `vec3_t` wide. The output of the points are in
*			axis order - assuming bitflags of 1 2 4 = X Y Z - where a bit unset is
*			mins and a bit set is maxs.
**/
inline void bbox3_to_points( const bbox3_t &box, vec3_t *points ) {
	for ( int32_t i = 0; i < 8; i++ ) {
		points[i] = {
			( ( i & 1 ) ? box.maxs : box.mins ).x,
			( ( i & 2 ) ? box.maxs : box.mins ).y,
			( ( i & 4 ) ? box.maxs : box.mins ).z
		};
	}
}

/**
*	@brief	Returns true if boxA its bounds intersect the bounds of box B, false otherwise.
**/
inline const bool bbox3_intersects( const bbox3_t &boxA, const bbox3_t &boxB ) {
	if ( boxA.mins.x >= boxB.maxs.x || boxA.mins.y >= boxB.maxs.y || boxA.mins.z >= boxB.maxs.z ) {
		return false;
	}

	if ( boxA.maxs.x <= boxB.mins.x || boxA.maxs.y <= boxB.mins.y || boxA.maxs.z <= boxB.mins.z ) {
		return false;
	}

	return true;
}

/**
*	@brief	Returns true if 'box' contains point 'point'
**/
inline const bool bbox3_contains_point( const bbox3_t &box, const vec3_t &point ) {
	if ( point.x >= box.maxs.x || point.y >= box.maxs.y || point.z >= box.maxs.z ) {
		return false;
	}

	if ( point.x <= box.mins.x || point.y <= box.mins.y || point.z <= box.mins.z ) {
		return false;
	}

	return true;
}


/**
*	@return	The relative size of the box' bounds. Also works as a vector
*	between the two points of a box.
**/
inline const vec3_t bbox3_size( const bbox3_t &box ) {
	return box.maxs - box.mins;
}

/**
*	@return	The distance between the two corners of the box's bounds.
**/
inline const float bbox3_distance( const bbox3_t &box ) {
	return vec3_distance( box.maxs, box.mins );
}

/**
*	@return	The radius of the bounds. A sphere that contains the entire box.
**/
inline const float bbox3_radius( const bbox3_t &box ) {
	return bbox3_distance( box ) / 2.f;
}

/**
*	@return	The center point of the bounding box.
**/
inline const vec3_t bbox3_center( const bbox3_t &box ) {
	return vec3_mix( box.mins, box.maxs, .5f );
}

/**
*	@return	A bounding box based on the 'size' vec3, centered along 'center'.
**/
inline const bbox3_t bbox3_from_center_size( const vec3_t &size, const vec3_t &center = vec3_zero() ) {
	const vec3_t halfSize = vec3_scale( size, .5f );
	return bbox3_t {
		center - halfSize,
		center + halfSize,
	};
}

/**
*	@return	A bounding box based on the 'radius', centered along 'center'.
**/
inline const bbox3_t bbox3_from_center_radius( const float radius, const vec3_t &center = vec3_zero() ) {
	const vec3_t radiusVec = { radius, radius, radius };
	return bbox3_t {
		center - radiusVec,
		center + radiusVec,
	};
}

///**
//*	@return A bounding box expanded, or shrunk(in case of negative values), on all axis.
//**/
//inline const bbox3_t bbox3_expand_vec3( const bbox3_t &box, const vec3_t &expansion ) {
//	return bbox3_t {
//		box.mins - expansion,
//		box.maxs + expansion
//	};
//}
///**
//*	@return A bounding box expanded, or shrunk(in case of negative values), on all axis.
//**/
//inline const bbox3_t bbox3_expand_bbox3( const bbox3_t &box, const bbox3_t &expansion) {
//	return bbox3_t {
//		box.mins + expansion.mins,
//		box.maxs + expansion.maxs
//	};
//}
///**
//*	@return A bounding box expanded, or shrunk(in case of negative values), on all axis.
//**/
//inline const bbox3_t bbox3_expandf( const bbox3_t &box, const float expansion ) {
//	return bbox3_expand_vec3( box, vec3_t{ expansion, expansion, expansion } );
//}

inline const bbox3_t bbox3_expand3( const bbox3_t &bounds, const vec3_t &expansion ) {
	return bbox3_t {
		bounds.mins - expansion,
		bounds.maxs + expansion
	};
}
inline const bbox3_t bbox3_expand( const bbox3_t &bounds, const float expansion ) {
	return bbox3_expand3( bounds, vec3_t { expansion, expansion, expansion } );
}
inline const bbox3_t bbox3_expand_box( const bbox3_t &boundsA, const bbox3_t &boundsB ) {
	return bbox3_t { 
		boundsA.mins + boundsB.mins,
		boundsA.maxs + boundsB.maxs 
	};
}

/**
*	@return	The 'point' clamped within the bounds of 'bounds'.
**/
inline const vec3_t bbox3_clamp_point( const bbox3_t &bounds, const vec3_t point ) {
	return vec3_clamp( point, bounds.mins, bounds.maxs );
}

/**
*	@return	The bounds of 'boundsB' clamped within and to the bounds of 'boundsA'.
**/
inline const bbox3_t bbox3_clamp_bounds( const bbox3_t &boundsA, const bbox3_t &boundsB ) {
	return bbox3_t{
		vec3_clamp( boundsB.mins, boundsA.mins, boundsA.maxs ),
		vec3_clamp( boundsB.maxs, boundsA.mins, boundsA.maxs ),
	};
}

/**
*	@return	A random point within the bounds of 'bounds'.
**/
inline const vec3_t bbox3_random_point( const bbox3_t &bounds ) {
	return vec3_mix3( bounds.mins, bounds.maxs, vec3_random() );
}

/**
*	@return	True if box 'a' and box 'b' are equal. False otherwise.
**/
inline const bool bbox3_equal( const bbox3_t &boxA, const bbox3_t &boxB ) {
	return vec3_equal( boxA.mins, boxB.mins ) && vec3_equal( boxA.maxs, boxB.maxs );
}

/**
*	@return The symmetrical extents of the bbox 'bounds'.
**/
inline const vec3_t bbox3_symmetrical( const bbox3_t &bounds ) {
	return vec3_maxf( vec3_fabsf( bounds.mins ), vec3_fabsf( bounds.maxs ) );
}

/**
*	@return	A scaled version of 'bounds'.
**/
inline const bbox3_t bbox3_scale( const bbox3_t &bounds, const float scale ) {
	return bbox3_t {
		vec3_scale( bounds.mins, scale ),
		vec3_scale( bounds.maxs, scale ),
	};
}

/**
*	@brief	"A Simple Method for Box-Sphere Intersection Testing",
*			by Jim Arvo, in "Graphics Gems", Academic Press, 1990.
*
*			This routine tests for intersection between an axis-aligned box (bbox3_t)
*			and a dimensional sphere(sphere_t). The 'testType' argument indicates whether the shapes
*			are to be regarded as plain surfaces, or plain solids.
*						
*	@param	testType	Mode:  Meaning:
*			
*						0      'Hollow Box' vs 'Hollow Sphere'
*						1      'Hollow Box' vs 'Solid  Sphere'
*						2      'Solid  Box' vs 'Hollow Sphere'
*						3      'Solid  Box' vs 'Solid  Sphere'
**/
const bool bbox3_intersects_sphere( const bbox3_t &boxA, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon, const bool useOriginOffset = true );
/**
*	@brief	"A Simple Method for Box-Sphere Intersection Testing",
*			by Jim Arvo, in "Graphics Gems", Academic Press, 1990.
*
*			This routine tests for intersection between an axis-aligned box (bbox3_t)
*			and a dimensional sphere(sphere_t). The 'testType' argument indicates whether the shapes
*			are to be regarded as plain surfaces, or plain solids.
*						
*	@param	testType	Mode:  Meaning:
*			
*						0      'Hollow Box' vs 'Hollow Sphere'
*						1      'Hollow Box' vs 'Solid  Sphere'
*						2      'Solid  Box' vs 'Hollow Sphere'
*						3      'Solid  Box' vs 'Solid  Sphere'
**/
const bool sphere_intersects_bbox3( const bbox3_t &boxA, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon, const bool useOriginOffset = true );

/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with sphere/capsule hull tracing.
**/
const sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin );
/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with capsule hull tracing.
**/
const sphere_t bbox3_to_capsule( const bbox3_t &bounds, const vec3_t &origin );