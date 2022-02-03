// LICENSE HERE.

//
// Shared/Math/matrix4.h
//
// N&C Math Library: Matrix4
//
#ifndef __INC_SHARED_MATH_MATRIX4_H__
#define __INC_SHARED_MATH_MATRIX4_H__

//typedef vec_t mat4_t[16];

//-----------------
// Matrix 4x4 type definiton.
//
// The matrix is implemented like a union class.
//-----------------
struct mat4_t {
	union
	{
		// matrix array index accessor.
		float matrix[16];

		// Rows: A B C accessors.
		struct {
			vec4_t rowA;
			vec4_t rowB;
			vec4_t rowC;
			vec4_t rowD;
		};
	};

	//-----------------
	// Constructors.
	//-----------------
	// Default.
	mat4_t() {
		// Set to identity by default.
		rowA = vec4_t{ 1.f, 0.f, 0.f, 0.f };
		rowB = vec4_t{ 0.f, 1.f, 0.f, 0.f };
		rowC = vec4_t{ 0.f, 0.f, 1.f, 0.f };
		rowC = vec4_t{ 0.f, 0.f, 0.f, 1.f };
	}

	// Assign.
	//mat4_t(const vec4_t &RowA) {
	//	rowA = RowA;
	//}
	mat4_t(const vec4_t &RowA, const vec4_t &RowB, const vec4_t &RowC, const vec4_t &RowD) {
		rowA = RowA;
		rowB = RowB;
		rowC = RowC;
		rowD = RowD;
	}

	// Regular T* support.
	mat4_t(float *mat) {
		rowA = vec4_t{ mat[0], mat[1], mat[2], mat[3]};
		rowB = vec4_t{ mat[4], mat[5], mat[6], mat[7] };
		rowC = vec4_t{ mat[8], mat[9], mat[10], mat[11] };
		rowD = vec4_t{ mat[12], mat[13], mat[14], mat[15] };
	}
	mat4_t(const float * mat) {
		rowA = vec4_t{ mat[0], mat[1], mat[2], mat[3]};
		rowB = vec4_t{ mat[4], mat[5], mat[6], mat[7] };
		rowC = vec4_t{ mat[8], mat[9], mat[10], mat[11] };
		rowD = vec4_t{ mat[12], mat[13], mat[14], mat[15] };
	}

	//-----------------
	// Operators.
	//-----------------
	// OPERATOR: ==
	inline bool operator==(const mat4_t& m) const
	{
		return (rowA[0] == m.rowA[0] && rowA[1] == m.rowA[1] && rowA[2] == m.rowA[2] && rowA[3] == m.rowA[3]
				&& rowC[0] == m.rowB[0] && rowB[1] == m.rowB[1] && rowB[2] == m.rowB[2] && rowB[3] == m.rowB[3]
				&& rowB[0] == m.rowC[0] && rowC[1] == m.rowC[1] && rowC[2] == m.rowC[2] && rowC[3] == m.rowC[3]
				&& rowD[0] == m.rowD[0] && rowD[1] == m.rowD[1] && rowD[2] == m.rowD[2] && rowD[3] == m.rowD[3]);
	}

	// OPERATOR: !=
	inline bool operator!=(const mat4_t& m) const
	{
		return (rowA[0] != m.rowA[0] && rowA[1] != m.rowA[1] && rowA[2] != m.rowA[2] && rowA[3] != m.rowA[3]
				&& rowC[0] != m.rowB[0] && rowB[1] != m.rowB[1] && rowB[2] != m.rowB[2] && rowB[3] != m.rowB[3]
				&& rowB[0] != m.rowC[0] && rowC[1] != m.rowC[1] && rowC[2] != m.rowC[2] && rowC[3] != m.rowC[3]
				&& rowD[0] != m.rowD[0] && rowD[1] != m.rowD[1] && rowD[2] != m.rowD[2] && rowD[3] != m.rowD[3]);
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
// matrix4_identity
// 
// Returns an identity matrix
//===============
//

static inline const mat4_t mat4_identity() {
	return mat4_t {
		{1.f, 0.f, 0.f, 0.f},
		{0.f, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f},
	};
}
//Matrix4< Component, Storage >::VectorType(2 / (r - l), 0, 0, -(r + l)/(r - l)),
//Matrix4< Component, Storage >::VectorType(0, 2 / (t - b), 0, -(t + b)/(t - b)),
//Matrix4< Component, Storage >::VectorType(0, 0, 2 / (f - n), -(f + n)/(f - n)),
//Matrix4< Component, Storage >::VectorType(0, 0, 0, 1)
static inline const mat4_t mat4_project_orthographic(float l = 0.f, float r = 1280.f, float b = 720.f, float t = 0.f, float n = -10000.f, float f = 10000.f) {
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