/***
*
*	License here.
*
*	@file
*
*	Sphere(vec3_t based) utility function implementations.
*
*	TODO: Also contains capsule sphere utilities, move elsewhere.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"

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
const bool bbox3_intersects_sphere( const bbox3_t &boxA, const sphere_t &sphere, const int32_t testType, const float radiusDistEpsilon ) {
//int Box_Sphere_Intersect( n, Bmin, Bmax, C, r, mode )
//int    n;       /* The dimension of the space.           */
//float  Bmin[];  /* The minimum of the box for each axis. */
//float  Bmax[];  /* The maximum of the box for each axis. */
//float  C[];     /* The sphere center in n-space.         */
//float  r;       /* The radius of the sphere.             */
//int    mode;    /* Selects hollow or solid.              */
//{
//float  a, b;
//float  dmin, dmax;
//float  r2 = SQR( r );
//int    i, face;
//
	// Squared Radius to test against.
	const float testRadius = flt_square( sphere.radius + radiusDistEpsilon );
	
	// Calculate sphere center, keep it's optional 'offset' from 'origin' in-mind.
	const vec3_t sphereCenter = sphere.origin + sphere.offset;

	// 'Hollow Box' vs 'Hollow Sphere'
	if ( testType == bbox3_t::IntersectType::HollowBox_HollowSphere ) {
//    case 0: /* Hollow Box and Hollow Sphere */
//        dmin = 0;
//        dmax = 0;
//        face = FALSE;
//        for( i = 0; i < n; i++ ) {
//            a = SQR( C[i] - Bmin[i] );
//            b = SQR( C[i] - Bmax[i] );
//            dmax += MAX( a, b );
//            if( C[i] < Bmin[i] ) {
//                face = TRUE;
//                dmin += a;
//                }
//            else if( C[i] > Bmax[i] ) {
//                face = TRUE;
//                dmin += b;
//                }
//            else if( MIN( a, b ) <= r2 ) face = TRUE;
//            }
//        if( face && ( dmin <= r2 ) && ( r2 <= dmax ) ) return TRUE;
		// Delta values to keep score of the actual surface differences.
		float deltaMin = 0;
		float deltaMax = 0;

		// Whether we're on face.
		bool face = false;

		// Iterate over all three axial components.
		for( int32_t i = 0; i < 3; i++ ) {
			// Calculate total coverage for mins.
			const float a = flt_square( sphereCenter[i] - ( boxA.mins[i] ) );
			// Calculate total coverage for mins.
			const float b = flt_square( sphereCenter[i] - ( boxA.maxs[i] ) );     

			// Sum the max of either two.
			deltaMax += Maxf( a, b );

			// Sum the max of either two.
			if( sphereCenter[i] < boxA.mins[i] ) {
				face = true;
				deltaMin += a;
			} else if( sphereCenter[i] > boxA.maxs[i] ) {
				face = true;
				deltaMin += b;
			} else if ( Minf( a, b ) <= testRadius ) {
				face = true;
			}
		}

		// Intersected.
		if( face == true && ( deltaMin <= testRadius ) && ( testRadius <= deltaMax ) ) {
			return true;
		}
		return false;
	// 'Hollow Box' vs 'Solid  Sphere'
	} else if ( testType == bbox3_t::IntersectType::HollowBox_SolidSphere ) {
//        dmin = 0;
//        face = FALSE;
//        for( i = 0; i < n; i++ ) {
//            if( C[i] < Bmin[i] ) {
//                face = TRUE;
//                dmin += SQR( C[i] - Bmin[i] );
//                }
//            else if( C[i] > Bmax[i] ) {
//                face = TRUE;
//                dmin += SQR( C[i] - Bmax[i] );     
//                }
//            else if( C[i] - Bmin[i] <= r ) face = TRUE;
//            else if( Bmax[i] - C[i] <= r ) face = TRUE;
//            }
//        if( face && ( dmin <= r2 ) ) return TRUE;
		// Delta values to keep score of the actual surface differences.
		float deltaMin = 0;

		// Whether we're on face.
		bool face = false;
		// Iterate over all three axial components.
		for( int32_t i = 0; i < 3; i++ ) {
			// Sum the max of either two.
			if( sphereCenter[i] < boxA.mins[i] ) {
				face = true;
				deltaMin += flt_square( sphereCenter[i] - ( boxA.mins[i] ) );
			} else if( sphereCenter[i] > boxA.maxs[i] ) {
				face = true;
				deltaMin += flt_square( sphereCenter[i] - ( boxA.maxs[i] ) );
			} else if (sphereCenter[i] - boxA.mins[i] <= testRadius) {
				face = true;
			} else if (boxA.maxs[i] - sphereCenter[i] <= testRadius) {
				face = true;
			}
		}

		// Intersected.
		if( face == true && ( deltaMin <= testRadius ) ) {
			return true;
		}
	// 'Solid  Box' vs 'Hollow Sphere'
	} else if ( testType == bbox3_t::IntersectType::SolidBox_HollowSphere ) {
//dmax = 0;
//dmin = 0;
//for( i = 0; i < n; i++ ) {
//    a = SQR( C[i] - Bmin[i] );
//    b = SQR( C[i] - Bmax[i] );
//    dmax += MAX( a, b );
//    if( C[i] < Bmin[i] ) dmin += a; else
//    if( C[i] > Bmax[i] ) dmin += b;
//    }
//if( dmin <= r2 && r2 <= dmax ) return TRUE;
		// Delta values to keep score of the actual surface differences.
		float deltaMin = 0;
		float deltaMax = 0;

		// Iterate over all three axial components.
		for( int32_t i = 0; i < 3; i++ ) {
			// Calculate total coverage for mins.
			const float a = flt_square( sphereCenter[i] - ( boxA.mins[i] ) );
			// Calculate total coverage for mins.
			const float b = flt_square( sphereCenter[i] - ( boxA.maxs[i] ) );     

			// Sum the max of either two.
			deltaMax += Maxf( a, b );
			if( sphereCenter[i] < boxA.mins[i] ) {
				deltaMin += a;
			} else if( sphereCenter[i] > boxA.maxs[i] ) {
				deltaMin += b;
			}
		}

		// Intersected.
		if( ( deltaMin <= testRadius ) && ( testRadius <= deltaMax ) ) {
			return true;
		}
	// 'Solid  Box' vs 'Solid  Sphere'
	} else if ( testType == bbox3_t::IntersectType::SolidBox_SolidSphere ) {
//    case 3: /* Solid Box and Solid Sphere */
//        dmin = 0;
//        for( i = 0; i < n; i++ ) {
//            if( C[i] < Bmin[i] ) dmin += SQR( C[i] - Bmin[i] ); else
//            if( C[i] > Bmax[i] ) dmin += SQR( C[i] - Bmax[i] );     
//            }
//        if( dmin <= r2 ) return TRUE;
//        break;
		// Delta values.
		float deltaMin = 0;

		// Iterate over all three axial components.
		for( int32_t i = 0; i < 3; i++ ) {
			// Sum the max of either two.
			if( sphereCenter[i] < boxA.mins[i] ) {
				deltaMin += flt_square( sphereCenter[i] - ( boxA.mins[i] ) );
			} else if( sphereCenter[i] > boxA.maxs[i] ) {
				deltaMin += flt_square( sphereCenter[i] - ( boxA.maxs[i] ) );
			}
		}

		// Intersected.
		if( deltaMin <= testRadius ) {
			return true;
		}
	} 
	return false;
}

/**
*	@brief	Calculates a spherical collision shape from a 'size' vector, for use with sphere/capsule hull tracing.
**/
sphere_t sphere_from_size( const vec3_t &size, const vec3_t &origin ) {
	// Calculkate the new 'origin' centered box.
	const bbox3_t originCenteredBounds = bbox3_from_center_size( size, origin );

	// Get its symmetrical size for use on determining a proper 'Offset Radius'.
	const vec3_t symmetricSize = bbox3_symmetrical( originCenteredBounds );
	const float halfHeight= symmetricSize.z;
	const float halfWidth = symmetricSize.x;

	// Actual radius.
	const float radius = bbox3_radius( originCenteredBounds );
	// 'Offset Radius'
	const float offsetRadius = ( halfWidth > halfHeight ? halfHeight : halfWidth );
	//const float radius = ( halfWidth > halfHeight ? halfHeight : halfWidth );

	// And we got our 'sphere'.
	return {
		// Actual radius.
		.radius = radius,
		// Offset Radius from its origin point.
		.offsetRadius = offsetRadius,

		// Half Width/Height.
		.halfHeight = halfHeight,
		.halfWidth = halfWidth,

		// The origin.
		.origin = origin,

		// The offset to use.
		.offset = vec3_zero() //vec3_t{ 0.f, 0.f, ( halfHeight - offsetRadius ) }
	};
}

/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with sphere/capsule hull tracing.
**/
sphere_t bbox3_to_sphere( const bbox3_t &bounds, const vec3_t &origin ) {
	// Get the bounds size of the box for use with calculating an 'origin' centered bounds box.
	const vec3_t boundsSize = bbox3_size( bounds );

	// Use FromSize function from here on to calculate the rest.
	return sphere_from_size( boundsSize, origin );
}


/**
*	@brief	Appropriately centers a sphere's offset to rotation.
**/
void sphere_calculate_offset_rotation( const glm::mat4 &matTransform, const glm::mat4 &matInvTransform, sphere_t &sphere, const bool isTransformed  ) {
	//if ( traceContext.isTransformedTrace ) {
	//	glm::vec4 vOffset = phvec_to_glmvec4( sphere.offset, 1 );
	//	const float t = sphere.halfHeight - sphere.offsetRadius;
	//	vOffset = traceContext.matInvTransform * glm::vec4( t, t, t, 1.f ) * traceContext.matTransform;
	//	glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
	//	sphere.offset = glmvec3_to_phvec( v3Offset );
	//} else {
		//const float t = sphere.halfHeight - sphere.offsetRadius;
		sphere.offset = { 0, 0, 0 };
//	}

	//	//vOffset = glm::vec4( t, t, t, 1.f ) * traceContext.matTransform * vOffset * traceContext.matInvTransform ;
	//	//vOffset = traceContext.matTransform * glm::vec4( t, t, t, 1.f ) * traceContext.matInvTransform * vOffset;
		//glm::vec4 vRotationOffset = vOffset + ( vOffset - ( glm::vec4( t, t, t, 1.f ) * traceContext.matTransform ) );
		//vOffset = vRotationOffset;
	
	
	// Transformed path:
	if ( isTransformed ) {
		glm::vec4 vOffset = phvec_to_glmvec4( sphere.offset, 1 );
		//const float t = sphere.halfHeight - sphere.radius;
		const float t = sphere.halfHeight - sphere.offsetRadius;
		vOffset = matTransform * glm::vec4( t, t, t, 1.f ) * matInvTransform;
		glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
		sphere.offset = glmvec3_to_phvec( v3Offset );
	} else {
		const float t = sphere.halfHeight - sphere.offsetRadius;
		sphere.offset = { 0, 0, 0 };//t };
	}

}



/**
*
*
*	Capsule Related. TODO: Move elsewhere.
*
*
**/
/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with capsule hull tracing.
**/
sphere_t capsule_sphere_from_size( const vec3_t &size, const vec3_t &origin ) {
	// Calculkate the new 'origin' centered box.
	const bbox3_t originCenteredBounds = bbox3_from_center_size( size, origin );

	// Get its symmetrical size for use on determining a proper 'Offset Radius'.
	const vec3_t symmetricSize = bbox3_symmetrical( originCenteredBounds );
	const float halfHeight= symmetricSize.z;
	const float halfWidth = symmetricSize.x;

	// Actual radius.
	const float radius = bbox3_radius( originCenteredBounds );
	// 'Offset Radius'
	const float offsetRadius = ( halfWidth > halfHeight ? halfHeight : halfWidth );
	//const float radius = ( halfWidth > halfHeight ? halfHeight : halfWidth );

	// And we got our 'sphere'.
	return {
		// Actual radius.
		.radius = radius,
		// Offset Radius from its origin point.
		.offsetRadius = offsetRadius,

		// Half Width/Height.
		.halfHeight = halfHeight,
		.halfWidth = halfWidth,

		// The origin.
		.origin = origin,

		// The offset to use.
		.offset = vec3_zero() //vec3_t{ 0.f, 0.f, ( halfHeight - offsetRadius ) }
	};
}

/**
*	@brief	Calculates a spherical collision shape from the 'bounds' box, for use with capsule hull tracing.
**/
sphere_t bbox3_to_capsule( const bbox3_t &bounds, const vec3_t &origin ) {
	// Get the bounds size of the box for use with calculating an 'origin' centered bounds box.
	const vec3_t boundsSize = bbox3_size( bounds );

	// Use FromSize function from here on to calculate the rest.
	return capsule_sphere_from_size( boundsSize, origin );
}

/**
*	@brief	Appropriately centers a capsule's offset to rotation.
**/
void capsule_calculate_offset_rotation( const glm::mat4 &matTransform, const glm::mat4 &matInvTransform, sphere_t &sphere, const bool isTransformed ) {
	// Transformed path:
	if ( isTransformed ) {
		glm::vec4 vOffset = phvec_to_glmvec4( sphere.offset, 1 );
		const float t = sphere.halfHeight - sphere.offsetRadius;
		vOffset = matTransform * glm::vec4( t, t, t, 1.f ) * matInvTransform;
		glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
		sphere.offset = glmvec3_to_phvec( v3Offset );		
	} else {
		const float t = sphere.halfHeight - sphere.offsetRadius;
		sphere.offset = { 0, 0, t };
	}
	
	//if ( traceContext.isTransformedTrace ) {
	//	glm::vec4 vOffset = phvec_to_glmvec4( sphere.offset, 1 );
	//	const float t = sphere.halfHeight - sphere.offsetRadius;
	//	vOffset = traceContext.matInvTransform * glm::vec4( t, t, t, 1.f ) * traceContext.matTransform;
	//	glm::vec3 v3Offset = { vOffset.x / vOffset.w, vOffset.y / vOffset.w, vOffset.z / vOffset.w };
	//	sphere.offset = glmvec3_to_phvec( v3Offset );
	//} else {
		//const float t = sphere.halfHeight - sphere.offsetRadius;
		//sphere.offset = { 0, 0, 0 };
//	}

	//	//vOffset = glm::vec4( t, t, t, 1.f ) * traceContext.matTransform * vOffset * traceContext.matInvTransform ;
	//	//vOffset = traceContext.matTransform * glm::vec4( t, t, t, 1.f ) * traceContext.matInvTransform * vOffset;
		//glm::vec4 vRotationOffset = vOffset + ( vOffset - ( glm::vec4( t, t, t, 1.f ) * traceContext.matTransform ) );
		//vOffset = vRotationOffset;

}
