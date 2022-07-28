/***
*
*	License here.
*
*	@file
*
*	Matrix3x3 struct and operation implementations.
*
***/
#pragma once

// Shared header.
#include "../Shared.h"


/**
*	@brief	3x3 Matrix type. Implemented as union struct.
**/
struct mat3_t {
	union
	{
		struct mat3_floats_t {
			mat3_floats_t () = default;
			// matrix array index accessor.
			float matrix[9];
		} mat;

		// Rows: A B C accessors.
		struct mat3_rows_t {
			mat3_rows_t() = default;
			vec3_t a;
			vec3_t b;
			vec3_t c;
		} rows;
	} u = { };



    /**
    *	Constructors.
    **/
	// Default.
	mat3_t() {
		// Set to identity by default.
		u.rows.a = vec3_t{ 1.f, 0.f, 0.f };
		u.rows.b = vec3_t{ 0.f, 1.f, 0.f };
		u.rows.c = vec3_t{ 0.f, 0.f, 1.f };
	}

	// Assign.
	mat3_t(const vec3_t &RowA, const vec3_t &RowB, const vec3_t &RowC) {
		u.rows.a =  RowA;
		u.rows.b =  RowB;
		u.rows.c =  RowC;
	}

	// Regular T* support.
	mat3_t(float *mat) {
		u.rows.a =  vec3_t{ mat[0], mat[1], mat[2] };
		u.rows.b =  vec3_t{ mat[3], mat[4], mat[5] };
		u.rows.c =  vec3_t{ mat[6], mat[7], mat[8] };
	}
	mat3_t(const float * mat) {
		u.rows.a =  vec3_t{ mat[0], mat[1], mat[2] };
 		u.rows.b =  vec3_t{ mat[3], mat[4], mat[5] };
		u.rows.c =  vec3_t{ mat[6], mat[7], mat[8] };
	}



    /**
    *	Operators
    **/
	/**
	*	@brief	== operator.
	**/
	inline bool operator==(const mat3_t& m) const
	{
		return (u.rows.a[0] == m.u.rows.a[0] && u.rows.a[1] == m.u.rows.a[1] && u.rows.a[2] == m.u.rows.a[2]
			&& u.rows.b[0] == m.u.rows.b[0] && u.rows.b[1] == m.u.rows.b[1] && u.rows.b[2] == m.u.rows.b[2]
			&& u.rows.c[0] == m.u.rows.c[0] && u.rows.c[1] == m.u.rows.c[1] && u.rows.c[2] == m.u.rows.c[2]);
	}

	/**
	*	@brief	!= operator.
	**/
	inline bool operator!=(const mat3_t& m) const
	{
		return (u.rows.a[0] != m.u.rows.a[0] && u.rows.a[1] != m.u.rows.a[1] && u.rows.a[2] != m.u.rows.a[2]
			&& u.rows.b[0] != m.u.rows.b[0] && u.rows.b[1] != m.u.rows.b[1] && u.rows.b[2] != m.u.rows.b[2]
			&& u.rows.c[0] != m.u.rows.c[0] && u.rows.c[1] != m.u.rows.c[1] && u.rows.c[2] != m.u.rows.c[2]);
	}

	/**
	*	@brief	*(pointer) operator.
	**/
	inline operator float *() {
		return &u.mat.matrix[0];
	}

	/**
	*	@brief	const *(pointer) operator.
	**/
	inline operator const float* () const {
		return &u.mat.matrix[0];
	}
};

/**
*	@return	An identity 3x3 matrix.
**/
static inline const mat3_t &&mat3_identity() {
	return std::move(
		mat3_t {
			vec3_t{1.f, 0.f, 0.f},
			vec3_t{0.f, 1.f, 0.f},
			vec3_t{0.f, 0.f, 1.f}
		}
	);
}

/**
*	@return The resulting matrix of 'm1' * 'm2'
**/
static inline const mat3_t &&mat3_multiply( const mat3_t &m1, const mat3_t &m2 ) {
	return std::move(
		mat3_t {
			vec3_t{ 
				m1[0] * m2[0] + m1[1] * m2[3] + m1[2] * m2[6],
				m1[0] * m2[1] + m1[1] * m2[4] + m1[2] * m2[7],
				m1[0] * m2[2] + m1[1] * m2[5] + m1[2] * m2[8]
			},
			vec3_t{
				m1[3] * m2[0] + m1[4] * m2[3] + m1[5] * m2[6],
				m1[3] * m2[1] + m1[4] * m2[4] + m1[5] * m2[7],
				m1[3] * m2[2] + m1[4] * m2[5] + m1[5] * m2[8]
			},
			vec3_t{
				m1[6] * m2[0] + m1[7] * m2[3] + m1[8] * m2[6],
				m1[6] * m2[1] + m1[7] * m2[4] + m1[8] * m2[7],
				m1[6] * m2[2] + m1[7] * m2[5] + m1[8] * m2[8]
			}
		}
	);
}

/**
*	@return The transformed vec3 of 'm' and 'v'.
**/
static inline const vec3_t &&mat3_transform_vector( const mat3_t &m, const vec3_t &v ) {
	return std::move(
		vec3_t {
			m[0] * v[0] + m[1] * v[1] + m[2] * v[2],
			m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
			m[6] * v[0] + m[7] * v[1] + m[8] * v[2] 
		}
	);
}


/**
*	@return The transposed variant of matrix 'in'.
**/
static inline const mat3_t &&mat3_transpose( const mat3_t &in ) {
	return std::move(
		mat3_t {
			vec3_t{in[0], in[3], in[6]},
			vec3_t{in[1], in[4], in[7]},
			vec3_t{in[2], in[5], in[8]}
		}
	);
}

/**
*	@return A matrix 3x3 generated from a vec3 set of angles.
**/
static inline const mat3_t &&mat3_from_angles( const vec3_t &angles ) {
	// Stores matrix row vectors.
	vec3_t rowA = vec3_zero(), rowB = vec3_zero(), rowC = vec3_zero();
	
	// Convert angles to matrix 3x3.
	vec3_vectors(angles, &rowA, &rowB, &rowC);

	return std::move( 
		mat3_t{
			rowA, 
			rowB, 
			rowC
		} 
	);
}

/**
*	@return A vec3 containing the angles of matrix 'm'.
**/
static inline const vec3_t &&mat3_to_angles( const mat3_t &m ) {
	vec3_t angles = vec3_zero();
	float c;
	float pitch;

	pitch = -asinf(m[2]);
	c = cosf(pitch);
	if (fabs(c) > 5 * 10e-6) {     // Gimball lock?
		// no
		c = 1.0f / c;
		angles[vec3_t::Pitch] = Degrees(pitch);
		angles[vec3_t::Yaw] = Degrees(atan2f(m[1] * c, m[0] * c));
		angles[vec3_t::Roll] = Degrees(atan2f(-m[5] * c, m[8] * c));
	}
	else {
		// yes
		angles[vec3_t::Pitch] = m[2] > 0 ? -90 : 90;
		angles[vec3_t::Yaw] = Degrees(atan2f(m[3], -m[4]));
		angles[vec3_t::Roll] = 180;
	}

	return std::move( angles );
}

/**
*	@return A matrix 3x3 containing the results of rotating 'in' around 'point' by 'angle'.
**/
static inline const mat3_t &&mat3_rotate( const mat3_t& in, float angle, const vec3_t &point ) {
	mat3_t t, b;

	vec_t c = cos( Radians(angle ) );
	vec_t s = sin( Radians( angle ) );
	vec_t mc = 1 - c, t1, t2;

	t[0] = ( point.x * point.x * mc ) + c;
	t[4] = ( point.y * point.y * mc ) + c;
	t[8] = ( point.z * point.z * mc ) + c;

	t1 = point.y * point.x * mc;
	t2 = point.z * s;
	t[1] = t1 + t2;
	t[3] = t1 - t2;

	t1 = point.x * point.z * mc;
	t2 = point.y * s;
	t[2] = t1 - t2;
	t[6] = t1 + t2;

	t1 = point.y * point.z * mc;
	t2 = point.x * s;
	t[5] = t1 + t2;
	t[7] = t1 - t2;

	b = in;
	return std::move( mat3_multiply( b, t ) );
}