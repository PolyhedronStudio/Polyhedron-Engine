// LICENSE HERE.

//
// Shared/Math/matrix4.h
//
// Polyhedron Math Library: Matrix4
// 
// Most of these functions have been lend from the QFusion project.
//
#ifndef __INC_SHARED_MATH_MATRIX4_H__
#define __INC_SHARED_MATH_MATRIX4_H__

#include "../Shared.h"

//-----------------
// Matrix 4x4 type definiton.
//
// The matrix is implemented like a union class.
//-----------------
struct mat4_t {
	union
	{
		// matrix array index accessor.
		struct mat4_mat_t {
			mat4_mat_t() = default;
			float matrix[16];
		} mat;

		// Rows: A B C accessors.
		struct mat4_rows_t {
			mat4_rows_t() = default;
			vec4_t a;
			vec4_t b;
			vec4_t c;
			vec4_t d;
		} rows;
	} u = { };

	//-----------------
	// Constructors.
	//-----------------
	// Default.
	mat4_t() {
		// Set to identity by default.
		u.rows.a = vec4_t{ 1.f, 0.f, 0.f, 0.f };
		u.rows.b = vec4_t{ 0.f, 1.f, 0.f, 0.f };
		u.rows.c = vec4_t{ 0.f, 0.f, 1.f, 0.f };
		u.rows.d = vec4_t{ 0.f, 0.f, 0.f, 1.f };
	}
	mat4_t(const vec4_t &RowA, const vec4_t &RowB, const vec4_t &RowC, const vec4_t &RowD) {
		u.rows.a = RowA;
		u.rows.b = RowB;
		u.rows.c = RowC;
		u.rows.d = RowD;
	}
	// Regular T* support.
	mat4_t(float *mat) {
		u.rows.a = vec4_t{ mat[0], mat[1], mat[2], mat[3]};
		u.rows.b = vec4_t{ mat[4], mat[5], mat[6], mat[7] };
		u.rows.c = vec4_t{ mat[8], mat[9], mat[10], mat[11] };
		u.rows.d = vec4_t{ mat[12], mat[13], mat[14], mat[15] };
	}
	mat4_t(const float * mat) {
		u.rows.a = vec4_t{ mat[0], mat[1], mat[2], mat[3]};
		u.rows.b = vec4_t{ mat[4], mat[5], mat[6], mat[7] };
		u.rows.c = vec4_t{ mat[8], mat[9], mat[10], mat[11] };
		u.rows.d = vec4_t{ mat[12], mat[13], mat[14], mat[15] };
	}

	//-----------------
	// Operators.
	//-----------------
	//// OPERATOR: =
	//inline mat4_t operator=(const mat4_t& m) const
	//{
	//	return mat4_t{
	//		m.rowA, m.rowB, m.rowC, m.rowD
	//	};
	//}

	// OPERATOR: ==
	inline bool operator==(const mat4_t& m) const
	{
		return (u.rows.a[0] == m.u.rows.a[0] && m.u.rows.a[1] == m.u.rows.a[1] && m.u.rows.a[2] == m.u.rows.a[2] && u.rows.a[3] == m.u.rows.a[3]
				&& u.rows.c[0] == m.u.rows.b[0] && u.rows.b[1] == m.u.rows.b[1] && u.rows.b[2] == m.u.rows.b[2] && u.rows.b[3] == m.u.rows.b[3]
				&& u.rows.b[0] == m.u.rows.c[0] && u.rows.c[1] == m.u.rows.c[1] && u.rows.c[2] == m.u.rows.c[2] && u.rows.c[3] == m.u.rows.c[3]
				&& u.rows.d[0] == m.u.rows.d[0] && u.rows.d[1] == m.u.rows.d[1] && u.rows.d[2] == m.u.rows.d[2] && u.rows.d[3] == m.u.rows.d[3]);
	}

	// OPERATOR: !=
	inline bool operator!=(const mat4_t& m) const
	{
		return (u.rows.a[0] != m.u.rows.a[0] && m.u.rows.a[1] != m.u.rows.a[1] && m.u.rows.a[2] != m.u.rows.a[2] && u.rows.a[3] != m.u.rows.a[3]
				&& u.rows.c[0] != m.u.rows.b[0] && u.rows.b[1] != m.u.rows.b[1] && u.rows.b[2] != m.u.rows.b[2] && u.rows.b[3] != m.u.rows.b[3]
				&& u.rows.b[0] != m.u.rows.c[0] && u.rows.c[1] != m.u.rows.c[1] && u.rows.c[2] != m.u.rows.c[2] && u.rows.c[3] != m.u.rows.c[3]
				&& u.rows.d[0] != m.u.rows.d[0] && u.rows.d[1] != m.u.rows.d[1] && u.rows.d[2] != m.u.rows.d[2] && u.rows.d[3] != m.u.rows.d[3]);
	}

	// Pointer.
	inline operator float *() {
		return &u.mat.matrix[0];
	}

	// Pointer cast to const float*
	inline operator const float* () const {
		return &u.mat.matrix[0];
	}
};

//===============
// matrix4_identity
// 
// Returns an identity matrix
//===============
static inline mat4_t mat4_identity() {
	return mat4_t {
		{1.f, 0.f, 0.f, 0.f},
		{0.f, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f},
	};
}

//===============
// mat4_multiply_mat4
//
//===============
static inline mat4_t mat4_multiply_mat4(const mat4_t& a, const mat4_t& b) {
	// Return matrix.
	mat4_t m;

	// Multiply.
	m[0]  = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	m[1]  = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	m[2]  = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	m[3]  = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
	m[4]  = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
	m[5]  = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
	m[6]  = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
	m[7]  = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
	m[8]  = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
	m[9]  = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
	m[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
	m[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
	m[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
	m[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
	m[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
	m[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];

	// Return matrix.
	return m;
}

//===============
// mat4_multiply_fast_mat4
//
//===============
static inline mat4_t mat4_multiply_fast_mat4(const mat4_t& a, const mat4_t& b) {
	// Return matrix.
	mat4_t m;

	// Multiply.
	m[0]  = a[0] * b[0] + a[4] * b[1] + a[8] * b[2];
	m[1]  = a[1] * b[0] + a[5] * b[1] + a[9] * b[2];
	m[2]  = a[2] * b[0] + a[6] * b[1] + a[10] * b[2];
	m[3]  = 0.0f;
	m[4]  = a[0] * b[4] + a[4] * b[5] + a[8] * b[6];
	m[5]  = a[1] * b[4] + a[5] * b[5] + a[9] * b[6];
	m[6]  = a[2] * b[4] + a[6] * b[5] + a[10] * b[6];
	m[7]  = 0.0f;
	m[8]  = a[0] * b[8] + a[4] * b[9] + a[8] * b[10];
	m[9]  = a[1] * b[8] + a[5] * b[9] + a[9] * b[10];
	m[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10];
	m[11] = 0.0f;
	m[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12];
	m[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13];
	m[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14];
	m[15] = 1.0f;

	// Return matrix.
	return m;
}

//===============
// mat4_invert
// 
// Taken from QFusion source code, which has taken it from:
// 
// Taken from Darkplaces source code
// Adapted from code contributed to Mesa by David Moore (Mesa 7.6 under SGI Free License B - which is MIT/X11-type)
// added helper for common subexpression elimination by eihrul, and other optimizations by div0
//===============
static inline qboolean mat4_invert( const mat4_t &in, mat4_t &out ) {
	vec_t det;

	// note: orientation does not matter, as transpose(invert(transpose(m))) == invert(m), proof:
	//   transpose(invert(transpose(m))) * m
	// = transpose(invert(transpose(m))) * transpose(transpose(m))
	// = transpose(transpose(m) * invert(transpose(m)))
	// = transpose(identity)
	// = identity

	// this seems to help gcc's common subexpression elimination, and also makes the code look nicer
	vec_t m00 = in[0], m01 = in[1], m02 = in[2], m03 = in[3],
		m10 = in[4], m11 = in[5], m12 = in[6], m13 = in[7],
		m20 = in[8], m21 = in[9], m22 = in[10], m23 = in[11],
		m30 = in[12], m31 = in[13], m32 = in[14], m33 = in[15];

	// calculate the adjoint
	out[0] =  ( m11 * ( m22 * m33 - m23 * m32 ) - m21 * ( m12 * m33 - m13 * m32 ) + m31 * ( m12 * m23 - m13 * m22 ) );
	out[1] = -( m01 * ( m22 * m33 - m23 * m32 ) - m21 * ( m02 * m33 - m03 * m32 ) + m31 * ( m02 * m23 - m03 * m22 ) );
	out[2] =  ( m01 * ( m12 * m33 - m13 * m32 ) - m11 * ( m02 * m33 - m03 * m32 ) + m31 * ( m02 * m13 - m03 * m12 ) );
	out[3] = -( m01 * ( m12 * m23 - m13 * m22 ) - m11 * ( m02 * m23 - m03 * m22 ) + m21 * ( m02 * m13 - m03 * m12 ) );
	out[4] = -( m10 * ( m22 * m33 - m23 * m32 ) - m20 * ( m12 * m33 - m13 * m32 ) + m30 * ( m12 * m23 - m13 * m22 ) );
	out[5] =  ( m00 * ( m22 * m33 - m23 * m32 ) - m20 * ( m02 * m33 - m03 * m32 ) + m30 * ( m02 * m23 - m03 * m22 ) );
	out[6] = -( m00 * ( m12 * m33 - m13 * m32 ) - m10 * ( m02 * m33 - m03 * m32 ) + m30 * ( m02 * m13 - m03 * m12 ) );
	out[7] =  ( m00 * ( m12 * m23 - m13 * m22 ) - m10 * ( m02 * m23 - m03 * m22 ) + m20 * ( m02 * m13 - m03 * m12 ) );
	out[8] =  ( m10 * ( m21 * m33 - m23 * m31 ) - m20 * ( m11 * m33 - m13 * m31 ) + m30 * ( m11 * m23 - m13 * m21 ) );
	out[9] = -( m00 * ( m21 * m33 - m23 * m31 ) - m20 * ( m01 * m33 - m03 * m31 ) + m30 * ( m01 * m23 - m03 * m21 ) );
	out[10] =  ( m00 * ( m11 * m33 - m13 * m31 ) - m10 * ( m01 * m33 - m03 * m31 ) + m30 * ( m01 * m13 - m03 * m11 ) );
	out[11] = -( m00 * ( m11 * m23 - m13 * m21 ) - m10 * ( m01 * m23 - m03 * m21 ) + m20 * ( m01 * m13 - m03 * m11 ) );
	out[12] = -( m10 * ( m21 * m32 - m22 * m31 ) - m20 * ( m11 * m32 - m12 * m31 ) + m30 * ( m11 * m22 - m12 * m21 ) );
	out[13] =  ( m00 * ( m21 * m32 - m22 * m31 ) - m20 * ( m01 * m32 - m02 * m31 ) + m30 * ( m01 * m22 - m02 * m21 ) );
	out[14] = -( m00 * ( m11 * m32 - m12 * m31 ) - m10 * ( m01 * m32 - m02 * m31 ) + m30 * ( m01 * m12 - m02 * m11 ) );
	out[15] =  ( m00 * ( m11 * m22 - m12 * m21 ) - m10 * ( m01 * m22 - m02 * m21 ) + m20 * ( m01 * m12 - m02 * m11 ) );

	// calculate the determinant (as inverse == 1/det * adjoint, adjoint * m == identity * det,
	// so this calculates the det)
	det = m00 * out[0] + m10 * out[1] + m20 * out[2] + m30 * out[3];
	if( det == 0.0f ) {
		return false;
	}

	// multiplications are faster than divisions, usually
	det = 1.0f / det;

	// manually unrolled loop to multiply all matrix elements by 1/det
	out[0] *= det; out[1] *= det; out[2] *= det; out[3] *= det;
	out[4] *= det; out[5] *= det; out[6] *= det; out[7] *= det;
	out[8] *= det; out[9] *= det; out[10] *= det; out[11] *= det;
	out[12] *= det; out[13] *= det; out[14] *= det; out[15] *= det;

	return true;
}

//===============
// mat4_rotate
//
// Returns a matrix that is rotated by angle around the given axises.
//===============
static inline mat4_t mat4_rotate(vec_t angle, vec_t x, vec_t y, vec_t z ) {
	// Return matrix.
	mat4_t m;

	mat4_t t, b;
	vec_t c = std::cos( Radians( angle ) );
	vec_t s = std::sin( Radians( angle ) );
	vec_t mc = 1 - c, t1, t2;
	
	t[0]  = ( x * x * mc ) + c;
	t[5]  = ( y * y * mc ) + c;
	t[10] = ( z * z * mc ) + c;

	t1 = y * x * mc;
	t2 = z * s;
	t[1] = t1 + t2;
	t[4] = t1 - t2;

	t1 = x * z * mc;
	t2 = y * s;
	t[2] = t1 - t2;
	t[8] = t1 + t2;

	t1 = y * z * mc;
	t2 = x * s;
	t[6] = t1 + t2;
	t[9] = t1 - t2;

	t[3] = t[7] = t[11] = t[12] = t[13] = t[14] = 0;
	t[15] = 1;

	m.u.rows.a = b.u.rows.a;
	m.u.rows.b = b.u.rows.b;
	m.u.rows.c = b.u.rows.c;
	m.u.rows.d = b.u.rows.d;
	m = mat4_multiply_fast_mat4(b, t);

	return m;
}

//===============
// mat4_translate_vec3
// 
// Returns a matrix translated by the vector3.
//===============
static inline mat4_t mat4_translate_vec3( const mat4_t &m, const vec3_t &v) {
	mat4_t r = m;
	r[12] = r[0] * v.x + r[4] * v.y + r[8]  * v.z + r[12];
	r[13] = r[1] * v.x + r[5] * v.y + r[9]  * v.z + r[13];
	r[14] = r[2] * v.x + r[6] * v.y + r[10] * v.z + r[14];
	r[15] = r[3] * v.x + r[7] * v.y + r[11] * v.z + r[15];
	return r;
}

//===============
// mat4_scale_vec3
// 
// Returns a matrix scaled by the vector3.
//===============
static inline mat4_t mat4_scale_vec3( const mat4_t &m, const vec3_t &v) {
	mat4_t r = m;
	r[0] *= v.x; r[4] *= v.y; r[8]  *= v.z;
	r[1] *= v.x; r[5] *= v.y; r[9]  *= v.z;
	r[2] *= v.x; r[6] *= v.y; r[10] *= v.z;
	r[3] *= v.x; r[7] *= v.y; r[11] *= v.z;
	return r;
}

//===============
// mat4_transpose
// 
// Returns the transposed matrix of m.
//===============
static inline mat4_t mat4_transpose( const mat4_t &m) {
	mat4_t out;
	out[0] = m[0]; out[1] = m[4]; out[2] = m[8]; out[3] = m[12];
	out[4] = m[1]; out[5] = m[5]; out[6] = m[9]; out[7] = m[13];
	out[8] = m[2]; out[9] = m[6]; out[10] = m[10]; out[11] = m[14];
	out[12] = m[3]; out[13] = m[7]; out[14] = m[11]; out[15] = m[15];
	return out;
}

//===============
// mat4_to_vec3
//
// Returns a std::tuple containing:
// [0] -> vec3_t translation
// [1] -> vec3_t scale
// [2] -> vec3_t rotation
//===============
static inline const std::tuple<vec3_t, vec3_t, vec3_t> mat4_to_vec3(const mat4_t& m) {
	vec3_t translation;
	vec3_t scale;
	vec3_t rotation;

	// Translation.
	translation.x = m[0];
	translation.y = m[4];
	translation.z = m[8];
	// Scale.
	scale.x = m[1];
	scale.y = m[5];
	scale.z = m[9];
	// Rotation
	rotation.x = m[2];
	rotation.y = m[6];
	rotation.z = m[10];

	return std::make_tuple(translation, scale, rotation);
}

//===============
// mat4_multiply_vec4
//
// Returns a vec4_t multiplied by the matrix transform.
//===============
static inline vec4_t mat4_multiply_vec4( const mat4_t &m, const vec4_t &v) {
	return vec4_t{
		m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12] * v[3],
		m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13] * v[3],
		m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14] * v[3],
		m[3] * v[0] + m[7] * v[1] + m[11] * v[2] + m[15] * v[3],
	};
}

//===============
// mat4_multiply_vec3
//
// Returns a vec3_t multiplied by the matrix transform.
//===============
static inline vec3_t mat4_multiply_vec3( const mat4_t m, const vec3_t v, vec3_t out ) {
	return vec3_t{
		m[0] * v[0] + m[4] * v[1] + m[8] * v[2],
		m[1] * v[0] + m[5] * v[1] + m[9] * v[2],
		m[2] * v[0] + m[6] * v[1] + m[10] * v[2],
	};
}

//===============
// mat4_project_orthographic
// 
// Returns an orthogonal projection matrix.
//===============
static inline mat4_t mat4_project_orthographic(float l = 0.f, float r = 1280.f, float b = 720.f, float t = 0.f, float n = -10000.f, float f = 10000.f) {
	return mat4_t {
		//{2.f / (r - l), 0.f, 0.f, -(r + l) / (r - l)},
		//{0.f, 2.f / (t - b), 0.f, -(t + b) / (t - b)},
		//{0.f, 0.f, 2.f / (f - n), -(f + n) / (f - n)},
		//{0.f, 0.f, 0.f, 1.f},
		{2.0f / (r - l), 0.f, 0.f, 0.f},
		{0.f, 2.0f / (b - t), 0.f, 0.f},
		{0.f, 0.f, 1.0f / (n - f), 0.f},
		{-(r + l) / (r - l), -(b + t) / (b - t), n / (n - f), 1.0f}
	};
}

#endif // __INC_SHARED_MATH_MATRIX4_H__