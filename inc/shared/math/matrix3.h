// LICENSE HERE.

//
// shared/math/matrix3.h
//
// N&C Math Library: Matrix3
// 
// Implemented like a union class, not templated though.
//
#ifndef __INC_SHARED_MATH_MATRIX3_H__
#define __INC_SHARED_MATH_MATRIX3_H__

//-----------------
// Matrix 3x3 type definiton.
//
// The matrix is implemented like a union class.
//-----------------
struct mat3_t {
	union
	{
		// matrix array index accessor.
		float matrix[9];

		// Rows: A B C accessors.
		struct {
			vec3_t rowA;
			vec3_t rowB;
			vec3_t rowC;
		};
	};

	//-----------------
	// Constructors.
	//-----------------
	// Default.
	mat3_t() {
		// Set to identity by default.
		rowA = vec3_t{ 1.f, 0.f, 0.f };
		rowB = vec3_t{ 0.f, 1.f, 0.f };
		rowC = vec3_t{ 0.f, 0.f, 1.f };
	}

	// Assign.
	mat3_t(const vec3_t &RowA, const vec3_t &RowB, const vec3_t &RowC) {
		rowA = RowA;
		rowB = RowB;
		rowC = RowC;
	}

	// Regular T* support.
	mat3_t(float *mat) {
		rowA = vec3_t{ mat[0], mat[1], mat[2] };
		rowB = vec3_t{ mat[3], mat[4], mat[5] };
		rowC = vec3_t{ mat[6], mat[7], mat[8] };
	}
	mat3_t(const float * mat) {
		rowA = vec3_t{ mat[0], mat[1], mat[2] };
		rowB = vec3_t{ mat[3], mat[4], mat[5] };
		rowC = vec3_t{ mat[6], mat[7], mat[8] };
	}

	//-----------------
	// Operators.
	//-----------------
	// OPERATOR: ==
	inline bool operator==(const mat3_t& m) const
	{
		return (rowA[0] == m.rowA[0] && rowA[1] == m.rowA[1] && rowA[2] == m.rowA[2]
			&& rowB[0] == m.rowB[0] && rowB[1] == m.rowB[1] && rowB[2] == m.rowB[2]
			&& rowC[0] == m.rowC[0] && rowC[1] == m.rowC[1] && rowC[2] == m.rowC[2]);
	}

	// OPERATOR: !=
	inline bool operator!=(const mat3_t& m) const
	{
		return (rowA[0] != m.rowA[0] && rowA[1] != m.rowA[1] && rowA[2] != m.rowA[2]
			&& rowB[0] != m.rowB[0] && rowB[1] != m.rowB[1] && rowB[2] != m.rowB[2]
			&& rowC[0] != m.rowC[0] && rowC[1] != m.rowC[1] && rowC[2] != m.rowC[2]);
	}

	// Pointer.
	inline operator float *() {
		return &matrix[0];
	}

	// Pointer cast to const float*
	inline operator const float* () const {
		return &matrix[0];
	}
};

//
//===============
// matrix3_identity
// 
// Returns an identity matrix
//===============
//
static inline mat3_t matrix3_identity() {
	mat3_t m = {
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1}
	};

	return m;
}

//
//===============
// matrix3_multiply
// 
// Returns a resulting matrix of 'm1' * 'm2'
//===============
//
static inline mat3_t matrix3_multiply(const mat3_t &m1, const mat3_t &m2) {
	mat3_t out;

	return mat3_t{
		{ 
			m1[0] * m2[0] + m1[1] * m2[3] + m1[2] * m2[6],
			m1[0] * m2[1] + m1[1] * m2[4] + m1[2] * m2[7],
			m1[0] * m2[2] + m1[1] * m2[5] + m1[2] * m2[8]
		},
		{
			m1[3] * m2[0] + m1[4] * m2[3] + m1[5] * m2[6],
			m1[3] * m2[1] + m1[4] * m2[4] + m1[5] * m2[7],
			m1[3] * m2[2] + m1[4] * m2[5] + m1[5] * m2[8]
		},
		{
			m1[6] * m2[0] + m1[7] * m2[3] + m1[8] * m2[6],
			m1[6] * m2[1] + m1[7] * m2[4] + m1[8] * m2[7],
			m1[6] * m2[2] + m1[7] * m2[5] + m1[8] * m2[8]
		}
	};
}

//
//===============
// matrix3_transform_vector
// 
// Returns the transformed vec3 of 'm' and 'v'.
//===============
//
static inline vec3_t matrix3_transform_vector(const mat3_t &m, const vec3_t &v) {
	return vec3_t{
		m[0] * v[0] + m[1] * v[1] + m[2] * v[2],
		m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
		m[6] * v[0] + m[7] * v[1] + m[8] * v[2] 
	};
}

//
//===============
// matrix3_identity
// 
// Returns a transosed variant of matrix 'in'
//===============
//
static inline mat3_t matrix3_transpose(const mat3_t &in) {
	//vec3_t out;

	//out[0] = in[0];
	//out[4] = in[4];
	//out[8] = in[8];

	//out[1] = in[3];
	//out[2] = in[6];
	//out[3] = in[1];
	//out[5] = in[7];
	//out[6] = in[2];
	//out[7] = in[5];

	return mat3_t{
		{in[0], in[3], in[6]},
		{in[1], in[4], in[7]},
		{in[2], in[5], in[8]}
	};
}

//
//===============
// matrix3_from_angles
// 
// Returns a matrix 3 from a vec3 set of angles.
//===============
//
static inline mat3_t matrix3_from_angles(const vec3_t &angles) {
	mat3_t m;
	vec3_vectors(angles, &m.rowA, &m.rowB, &m.rowC);
	return m;
}

//
//===============
// matrix3_to_angles
// 
// Returns a set of angles in vec3_t from a mat3_t.
//===============
//
static inline vec3_t matrix3_to_angles(const mat3_t &m) {
	vec3_t angles;
	float c;
	float pitch;

	pitch = -std::asinf(m[2]);
	c = std::cosf(pitch);
	if (fabs(c) > 5 * 10e-6) {     // Gimball lock?
		// no
		c = 1.0f / c;
		angles[vec3_t::Pitch] = Degrees(pitch);
		angles[vec3_t::Yaw] = Degrees(std::atan2f(m[1] * c, m[0] * c));
		angles[vec3_t::Roll] = Degrees(std::atan2f(-m[5] * c, m[8] * c));
	}
	else {
		// yes
		angles[vec3_t::Pitch] = m[2] > 0 ? -90 : 90;
		angles[vec3_t::Yaw] = Degrees(std::atan2f(m[3], -m[4]));
		angles[vec3_t::Roll] = 180;
	}

	return angles;
}

//
//===============
// matrix3_rotate
// 
// Returns 'in' rotated around 'point' by 'angle'.
//===============
//
static inline mat3_t matrix3_rotate(const mat3_t& in, float angle, const vec3_t &point) {
	mat3_t t, b;

	vec_t c = cos(Radians(angle));
	vec_t s = sin(Radians(angle));
	vec_t mc = 1 - c, t1, t2;

	t[0] = (point.x * point.x * mc) + c;
	t[4] = (point.y * point.y * mc) + c;
	t[8] = (point.z * point.z * mc) + c;

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
	return matrix3_multiply(b, t);
}

#endif