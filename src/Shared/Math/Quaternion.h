/***
*
*	License here.
*
*	@file
*
*	Quaternion struct and operation implementations.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"

// We need to know about mat3_t for various functions to work.
#include "Matrix3x3.h"

// WID: This file obviously needs more work, but we just merely added it in for now
// so we can maintain vkpt changes from q2rtx (be compatible yo)
//typedef vec_t quat_t[4];

/**
*	@brief Quat type definiton: (X, Y, Z, W). W is the rotational angle around the XYZ axis.
**/
template<typename T> struct quat_template {
    union
    {
        T xyzw[4];

        // X Y Z desegnator accessors.
        struct {
            T x, y, z, w;
        };
    };

    /**
    *	Constructors.
    **/
    // Default.
    quat_template() { x = y = z = 0.f; w = 1.f; }
	//quat_template(T x, T y, T z, T w) { x = y = z = 0.f; w = 1.f; }

	/**
	*	@brief	Copy Constructor.
	**/
	quat_template( quat_template<T>& quaternion )  {
		this->x = quaternion.x;
		this->y = quaternion.y;
		this->z = quaternion.z;
		this->w = quaternion.w;
	}
	quat_template( const quat_template<T>& quaternion )  {
		this->x = quaternion.x;
		this->y = quaternion.y;
		this->z = quaternion.z;
		this->w = quaternion.w;
	}

    // Assign.
	quat_template( vec4_t v ) { x = v.x; y = v.y; z = v.z; w = v.w; }
//	quat_template(const T X, const T Y, const T Z, const T W) { x = X; y = Y; z = Z; w = W; }
    quat_template(T X, T Y, T Z, T W) { x = X; y = Y; z = Z; w = W; }

    // Regular *quat float array support.
    quat_template(T* quat) { x = quat[0]; y = quat[1]; z = quat[2]; w = quat[3]; }
    quat_template(const T* vec) { x = vec[0]; y = vec[1]; z = vec[2]; w = vec[3]; }

    /**
    *	Operators
    **/    
		// Pointer.
    inline operator T* () { 
        return &xyzw[0];
    }
    
    // Pointer cast to const float*
    inline operator const T* () const { 
        return &xyzw[0];
    }
};

typedef quat_template<float> quat_t;


/**
*	@return	A unit scaled identity quaternion.
**/
inline const quat_t quat_identity( ) {
	return { 0.f, 0.f, 0.f, 1.f };
}

inline void quat_copy( const quat_t &q1, quat_t &q2 ) {
	q2.x = q1.x;
	q2.y = q1.y;
	q2.z = q1.z;
	q2.w = q1.w;
}
	//void Quat_Copy( const quat_t q1, quat_t q2 ) {
//	q2[0] = q1[0];
//	q2[1] = q1[1];
//	q2[2] = q1[2];
//	q2[3] = q1[3];
//}

/**
*	@return A non scaled quaternion containing the vec3_t translation.
**/
inline const quat_t quat_from_vec3( const vec3_t &v ) {
	return {
		(v.x),
		(v.y),
		(v.z),
		(float)(-sqrtf(max(1.f - v.x * v.x - v.y * v.y - v.z * v.z, 0.0f)))
	};
}

/**
*	@return True if the quaternions are identical, false otherwise.
**/
inline const bool quat_compare( const quat_t &qa, const quat_t &qb ) {
	if( qa.x != qb.x || qa.y != qb.y || qa.z != qb.z || qa.w != qb.w ) {
		return false;
	}
	return true;
}

/**
*	@return	The conjugated form of the quaternion.
**/
inline const quat_t quat_conjugate( const quat_t &quaternion ) {
	return {
		-quaternion.x,
		-quaternion.y,
		-quaternion.z,
		quaternion.w
	};
}

/**
*	@return The dot product of quaternion A and quaternion B.
**/
inline const vec_t quat_dot( const quat_t qa, const quat_t qb ) {
	return ( qa.x * qb.x + qa.y * qb.y + qa.z * qb.z + qa.w * qb.w );
}

/**
*	@param	&length Gets assigned the quaternion normal length.
*	@return	A normalized version of quaternion.
**/
inline const vec_t quat_normalize( quat_t &quaternion ) {

	const vec_t length = quaternion[0] * quaternion[0] + quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3];
	if( length != 0 ) {
		vec_t ilength = 1.0 / sqrt( length );
		quaternion[0] *= ilength;
		quaternion[1] *= ilength;
		quaternion[2] *= ilength;
		quaternion[3] *= ilength;
	}

	return length;
}

/**
*	@brief	Assigns the inversed quaternion values to qb and normalizes qb.
*	@return	Length of normalized qb.
**/
inline vec_t quat_inverse( const quat_t &qa, quat_t &qb ) {
	qb = quat_conjugate( qa );

	return quat_normalize( qb );
}

/**
*	@brief	Creates a quaternion from a matrix 3x3.
**/
inline const quat_t quat_from_mat3( const mat3_t &m ) {
	quat_t q;
	vec_t tr, s;

	tr = m[0] + m[4] + m[8];
	if( tr > 0.00001 ) {
		s = sqrt( tr + 1.0 );
		q[3] = s * 0.5; s = 0.5 / s;
		q[0] = ( m[7] - m[5] ) * s;
		q[1] = ( m[2] - m[6] ) * s;
		q[2] = ( m[3] - m[1] ) * s;
	} else {
		int i, j, k;

		i = 0;
		if( m[4] > m[i * 3 + i] ) {
			i = 1;
		}
		if( m[8] > m[i * 3 + i] ) {
			i = 2;
		}
		j = ( i + 1 ) % 3;
		k = ( i + 2 ) % 3;

		s = sqrt( m[i * 3 + i] - ( m[j * 3 + j] + m[k * 3 + k] ) + 1.0 );

		q[i] = s * 0.5; if( s != 0.0 ) {
			s = 0.5 / s;
		}
		q[j] = ( m[j * 3 + i] + m[i * 3 + j] ) * s;
		q[k] = ( m[k * 3 + i] + m[i * 3 + k] ) * s;
		q[3] = ( m[k * 3 + j] - m[j * 3 + k] ) * s;
	}

	quat_normalize( q );

	return q;
}

/**
*	@return A multiplied quaternion of qa and qb.
**/
inline const quat_t quat_multiply ( const quat_t &q1, const quat_t &q2 ) {
	return {
		q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1],
		q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2],
		q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0],
		q1[3] * q2[3] - q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2]
	};
}

/**
*	@brief Performs a linear interpolation (origin).
**/
inline const quat_t quat_linear_lerp( const quat_t &q1, const quat_t &q2, float t ) {
	const float scale0 = 1.0f - t;
	const float scale1 = t;

	return {
		scale0 * q1[0] + scale1 * q2[0],
		scale0 * q1[1] + scale1 * q2[1],
		scale0 * q1[2] + scale1 * q2[2],
		scale0 * q1[3] + scale1 * q2[3]
	};
}

/**
*	@brief Lerps the Quats translation.
**/
// TODO: See how we can use std lib rsqrt?
inline const float Q_RSqrt( const float number ) {
	int i;
	float x2, y;

	if( number == 0.0 ) {
		return 0.0;
	}

	x2 = number * 0.5f;
	y = number;
	i = *(int *) &y;    // evil floating point bit level hacking
	i = 0x5f3759df - ( i >> 1 ); // what the fuck?
	y = *(float *) &i;
	y = y * ( 1.5f - ( x2 * y * y ) ); // this can be done a second time

	return y;
}

/**
*	@brief	Performs if needed a spherical interpolation, linear otherwise.
**/
inline const quat_t quat_lerp( const quat_t &q1, const quat_t &q2, vec_t t ) {
	quat_t p1;
	//vec_t omega, cosom, sinom, scale0, scale1, sinsqr;

	// If the quats are qual there's no lerping, return q1.
	if( quat_compare( q1, q2 ) ) {
		return q1;
	}

	vec_t cosom = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];
	if( cosom < 0.0 ) {
		cosom = -cosom;
		p1[0] = -q1[0]; p1[1] = -q1[1];
		p1[2] = -q1[2]; p1[3] = -q1[3];
	} else {
		p1[0] = q1[0]; p1[1] = q1[1];
		p1[2] = q1[2]; p1[3] = q1[3];
	}

	if( cosom >= 1.0 - 0.0001 ) {
		return quat_linear_lerp( q1, q2, t );
	}

	const vec_t sinsqr = 1.0 - cosom * cosom;
	const vec_t sinom = Q_RSqrt( sinsqr );
	const vec_t omega = atan2( sinsqr * sinom, cosom );
	const vec_t scale0 = sin( ( 1.0 - t ) * omega ) * sinom;
	const vec_t scale1 = sin( t * omega ) * sinom;

	return {
		scale0 * p1[0] + scale1 * q2[0],
		scale0 * p1[1] + scale1 * q2[1],
		scale0 * p1[2] + scale1 * q2[2],
		scale0 * p1[3] + scale1 * q2[3]
	};
}

/**
*	@brief	Generates axis vectors from the quaternion.
**/
inline void quat_vectors( const quat_t &q, vec3_t &f, vec3_t &r, vec3_t &u ) {
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];

	xx = q[0] * x2; yy = q[1] * y2; zz = q[2] * z2;
	f[0] = 1.0f - yy - zz; r[1] = 1.0f - xx - zz; u[2] = 1.0f - xx - yy;

	yz = q[1] * z2; wx = q[3] * x2;
	r[2] = yz - wx; u[1] = yz + wx;

	xy = q[0] * y2; wz = q[3] * z2;
	f[1] = xy - wz; r[0] = xy + wz;

	xz = q[0] * z2; wy = q[3] * y2;
	f[2] = xz + wy; u[0] = xz - wy;
}
/**
*	@brief	Converts the quaternion to a mat3 and assigns it to reference m.
**/
inline const void quat_to_mat3( const quat_t &q, mat3_t &m ) {
	quat_vectors( q, m.u.rows.a, m.u.rows.b, m.u.rows.c );
}
//inline const mat3_t quat_to_mat3( const quat_t &q, mat3_t &m ) {
//	quat_vectors( q, m.u.rows.a, m.u.rows.b, m.u.rows.c );
//	return m;
//}

/**
*	@brief	Transforms a vector using the quat's translate and rotation.
**/
inline void quat_transform_vec3( const quat_t &q, const vec3_t &v, vec3_t out ) {
#if 0
	vec_t wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// 9 muls, 3 adds
	x2 = q[0] + q[0]; y2 = q[1] + q[1]; z2 = q[2] + q[2];
	xx = q[0] * x2; xy = q[0] * y2; xz = q[0] * z2;
	yy = q[1] * y2; yz = q[1] * z2; zz = q[2] * z2;
	wx = q[3] * x2; wy = q[3] * y2; wz = q[3] * z2;

	// 9 muls, 9 subs, 9 adds
	out[0] = ( 1.0f - yy - zz ) * v[0] + ( xy - wz ) * v[1] + ( xz + wy ) * v[2];
	out[1] = ( xy + wz ) * v[0] + ( 1.0f - xx - zz ) * v[1] + ( yz - wx ) * v[2];
	out[2] = ( xz - wy ) * v[0] + ( yz + wx ) * v[1] + ( 1.0f - xx - yy ) * v[2];
#else
	vec3_t t;

	CrossProduct( &q[0], v, t ); // 6 muls, 3 subs
	VectorScale( t, 2, t );      // 3 muls
	CrossProduct( &q[0], t, out );// 6 muls, 3 subs
	VectorMA( out, q[3], t, out );// 3 muls, 3 adds
#endif
}

inline void quat_concat_transforms( const quat_t &q1, const vec3_t &v1, const quat_t &q2, const vec3_t &v2, quat_t &q, vec3_t &v ) {
	q = quat_multiply( q1, q2 );
	quat_transform_vec3( q1, v2, v );
	v[0] += v1[0]; v[1] += v1[1]; v[2] += v1[2];
}