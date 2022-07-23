/***
*
*	License here.
*
*	@file
*
*	Dual Quaternion struct and operation implementations.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"

// We need to know about mat3_t and quat_t for various functions to work.
#include "Matrix3x3.h"
#include "Quaternion.h"

// WID: This file obviously needs more work, but we just merely added it in for now
// so we can maintain vkpt changes from q2rtx (be compatible yo)
//typedef vec_t quat_t[4];

/**
*	@brief Quat type definiton: (X, Y, Z, W). W is the rotational angle around the XYZ axis.
**/
template<typename T> struct dualquat_template {
    T dq[2];

    dualquat_template() = default;
	dualquat_template(quat_t *real_and_dual[2]) {
		dq[0] = real_and_dual[0];
		dq[1] = real_and_dual[1];
	}
	dualquat_template(const quat_t *real_and_dual[2]) {
		dq[0] = real_and_dual[0];
		dq[1] = real_and_dual[1];
	}
	dualquat_template(quat_t &real, quat_t &dual) {
		dq[0] = real;
		dq[1] = dual;
	}
	dualquat_template(const quat_t &real, const quat_t &dual) {
		dq[0] = real;
		dq[1] = dual;
	}
	//	dq = {
	//		{ 0.f, 0.f, 0.f, 1.f },
	//		{ 0.f, 0.f, 0.f, 1.f }
	//	};
	//}

    /**
    *	Operators
    **/    
	// Pointer.
    inline operator T* () { 
        return &dq[0];
    }
    
    // Pointer cast to const float*
    inline operator const T* () const { 
        return &dq[0];
    }

};

typedef dualquat_template<quat_t> dualquat_t;


//============================================================================

/**
*	@return	An identity dual quaternion
**/
inline const dualquat_t dualquat_identity( ) {
	return dualquat_t(
		quat_t ( 0.f, 0.f, 0.f, 1.f ),
		quat_t ( 0.f, 0.f, 0.f, 1.f)
	);
}

/**
*	@brief	Copies the content of dualquat in, to dualquat out.
**/
inline void dualquat_copy( const dualquat_t &in, dualquat_t &out ) {
	quat_copy( in.dq[0], out.dq[0]);
	quat_copy( in.dq[1], out.dq[1]);
}

/**
*	@brief	Sets the translation quat to origin v.
**/
inline void dualquat_set_translation( dualquat_t &dq, const vec3_t &v ) {
	dq.dq[1] = {
		0.5f * ( v[0] * dq.dq[0][3] + v[1] * dq.dq[0][2] - v[2] * dq.dq[0][1] ),
		0.5f * ( -v[0] * dq.dq[0][2] + v[1] * dq.dq[0][3] + v[2] * dq.dq[0][0] ),
		0.5f * ( v[0] * dq.dq[0][1] - v[1] * dq.dq[0][0] + v[2] * dq.dq[0][3] ),
		-0.5f * ( v[0] * dq.dq[0][0] + v[1] * dq.dq[0][1] + v[2] * dq.dq[0][2] )
	};
}

/**
*	@return	A dualquat with rot/scale extracted from mat3, translation from vec3.
**/
inline const dualquat_t dualquat_from_mat3_vec3( const mat3_t &m, const vec3_t &translate ) {
	return dualquat_t {
		quat_from_mat3( m ),
		quat_from_vec3( translate),
	};
}

/**
*	@return	A dualquat constructed from angles and translation.
**/
inline const dualquat_t dualquat_from_angles_axis( const vec3_t &angles, const vec3_t &translate ) {
	//mat3_t axis;

	// TODO: This should work also.
	const mat3_t axis = matrix3_from_angles( angles );
	// Aonvert angles to axis.
	//AnglesToAxis( angles, axis );

	// Set translation.
	return dualquat_from_mat3_vec3( axis, translate );
}

/**
*	@return	A dualquat constructed from a quat and a translation vector.
**/
inline const dualquat_t dualquat_from_quat_vec3( const quat_t &q, const vec3_t &translate ) { 
	dualquat_t dq;

	// Copy in regular quaternion.
	dq.dq[0] = q;
	quat_normalize( dq.dq[0] );

	// Convert translate vector to the 'dual part'
	dualquat_set_translation( dq, translate );

	// Return.
	return dq;
}

/**
*	@return	A dualquat constructed from a quat3 and a translation vector.
**/
inline const dualquat_t dualquat_from_quat3_vec3( const vec3_t &quat3, const vec3_t &translate ) { 
	dualquat_t dq;

	// Copy in regular quaternion.
	dq.dq[0] = quat_from_vec3( quat3 );
	quat_normalize( dq.dq[0] );

	// Convert translate vector to the 'dual part'
	dualquat_set_translation( dq, translate );

	// Return.
	return dq;
}

/**
*	@return	The translation vector part of the dualquat.
**/
inline const vec3_t dualquat_get_translation(const dualquat_t &dq) {
	const quat_t &real = dq.dq[0];
	const quat_t &dual = dq.dq[1];

	// Get our translation vector.
	vec3_t translate = vec3_zero();
	CrossProduct( real, dual, translate );
	// TODO: Does this work? Hehe.
	translate = vec3_fmaf(translate, real[3], dual.xyzw);
	translate = vec3_fmaf(translate, -dual[3], real.xyzw);
	// Return.
	return vec3_scale( translate, 2.f );
}

/**
*	@brief	Copies the regular real quaternion part out, as well as the translation vector.
**/
inline const void dualquat_to_quat_vec3( const dualquat_t &dq, quat_t &q, vec3_t &v ) {
	// Regular quaternion.
	quat_copy( dq.dq[0], q );

	// Translation vector.
	v = dualquat_get_translation( dq );
}

/**
*	@brief	Copies the regular real quaternion part out to mat3, and the the translation to vec3.
**/
inline const void dualquat_to_mat3_vec3( const dualquat_t &dq, mat3_t &m, vec3_t &v ) {
	// Regular quaternion.
	quat_to_mat3( dq.dq[0], m );

	// Translation vector.
	v = dualquat_get_translation( dq );
}

/**
*	@brief	Inverts the dual quat.
**/
inline const dualquat_t dualquat_invert( const dualquat_t &dq ) {
	dualquat_t newDq = {
		quat_conjugate( dq.dq[0] ),
		quat_conjugate( dq.dq[1] )
	};
	quat_t &real = newDq.dq[0];
	quat_t &dual = newDq.dq[1];

	const vec_t scalar = 2 * quat_dot( real, dual );
	dual[0] -= real[0] * scalar;
	dual[1] -= real[1] * scalar;
	dual[2] -= real[2] * scalar;
	dual[3] -= real[3] * scalar;

	return dq;
}

/**
*	@brief	Normalizes the dualquat. Returns the length.
**/
inline vec_t dualquat_normalize( dualquat_t &dq ) {
	// Get reference to both, 'real' and 'dual', quaternion parts.
	quat_t &real = dq.dq[0];
	quat_t &dual = dq.dq[1];

	// Calculate length.
	const vec_t length = real[0] * real[0] + real[1] * real[1] + real[2] * real[2] + real[3] * real[3];
	if ( length != 0 ) {
		const vec_t ilength = 1.0 / sqrt( length );
		real = vec4_scale( real.xyzw, ilength );
		dual = vec4_scale( dual.xyzw, ilength );
	}
	return length;
}

/**
*	@return	The multiplication dualquat_t of dq1 and dq2.
**/
inline const dualquat_t dualquat_multiply( const dualquat_t &dq1, const dualquat_t &dq2 ) {
	// The new dq.
	dualquat_t dq;

	// Temp quats.
	quat_t tq1, tq2;

	// Temporarily, used for setting translation vector.
	const quat_t tempq1 = quat_multiply( dq1[0], dq2[1] );
	const quat_t tempq2 = quat_multiply( dq1[1], dq2[0] );

	// Actually multiply the real quat part.
	dq[0] = quat_multiply( dq1[0], dq2[0] );

	// Get translation.
	dq[1][0] = tq1[0] + tq2[0];
	dq[1][1] = tq1[1] + tq2[1];
	dq[1][2] = tq1[2] + tq2[2];
	dq[1][3] = tq1[3] + tq2[3];

	return dq;
}

// Lerp the dual quats.
inline void dualquat_lerp ( const dualquat_t &dq1, const dualquat_t &dq2, vec_t t, dualquat_t &out ) {
	const vec_t k = (dq1[0][0] * dq2[0][0] + dq1[0][1] * dq2[0][1] + dq1[0][2] * dq2[0][2] + dq1[0][3] * dq2[0][3]) < 0 ? -t : t;
	t = 1.0 - t;

	for ( int32_t i = 0; i < 4; i++ ) {
		out[0][i] = dq1[0][i] * t + dq2[0][i] * k;
	}
	for ( int32_t i = 0; i < 4; i++ ) {
		out[1][i] = dq1[1][i] * t + dq2[1][i] * k;
	}

	quat_normalize( out[0] );
}