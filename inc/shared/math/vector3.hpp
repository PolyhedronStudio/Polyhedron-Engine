/*
// LICENSE HERE.

//
// shared/math/vector3.h
//
// vec3_t compatible Vector3 class.
//
*/
#ifndef __SHARED_MATH_VEC3_T_H__
#define __SHARED_MATH_VEC3_T_H__

class Vector
{
public:
	Vector() { x = y = z = 0; }

	// Basic way of making a vector
	Vector(float X, float Y, float Z) { x = X; y = Y; z = Z; }

	// Vector from another vector
	Vector(const Vector& v) { x = v.x; y = v.y; z = v.z; }

	// vec3_t support
	Vector(float* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }
	Vector(const float* vec) { x = vec[0]; y = vec[1]; z = vec[2]; }

public: // Utilities

	// Length of the vector
	inline float Length() const
	{
		return (float)sqrt(static_cast<float>(x * x + y * y + z * z));
	}

	inline float Length2D() const
	{
		return (float)sqrt(x * x + y * y);
	}

	// Returns a normalised vector, does not actually
	// convert this vector into a normalised one
	Vector Normalized() const;

	// Converts this vector into a normalised one
	inline void Normalize()
	{
		*this /= Length();
	}

	// Returns an angle vector from this vector's XYZ coords
	// Note: Returns in degrees
	// Originally taken from vectoangles in q_math.cpp
	Vector ToAngles(bool flipPitch = false) const;

	// A cross product. What is there to say?
	inline const Vector& CrossProduct(const Vector& op) const
	{
		return Vector(
			(y * op.z - z * op.y),
			(z * op.x - x * op.z),
			(x * op.y - y * op.x)
		);
	}

	// Returns a rotated vector around an axis
	// Formula: https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
	// TODO: Write a Quaternion class for these rotations lol
	// Note: You can use this to achieve local-space rotations by specifying 
	//       the axis as either a forward, right or up vector
	inline const Vector& RotatedAboutAxis(const Vector& axis, float angle) const;

	// Same as RotatedAboutAxis, except it actually modifies this vector
	inline void RotateAboutAxis(const Vector& axis, float angle)
	{
		*this = RotatedAboutAxis(axis, angle);
	}

	// I got no idea how this thing works nor what I can use it for...
	inline const Vector& ProjectedOnPlane(const Vector& normal);

	// Same as ProjectedOnPlane, except it modifies this vector
	inline void ProjectOnPlane(const Vector& normal)
	{
		*this = ProjectedOnPlane(normal);
	}

	// This one might be useful when you wanna calculate bounce directions
	// The "mod" parameter adds some sorta bias to the reflection:
	// If mod is higher, then the reflection angle is closer to the normal vector,
	// otherwise if it's lower, then the reflection is more perpendicular to the normal vector
	inline const Vector& Reflect(const Vector& normal, float mod = 2.0f) const
	{
		float dot = *this * normal;
		Vector projected = (normal * (mod * dot));
		return *this - projected;
	}

	// Similar to the == operator except it takes into account an epsilon
	bool Equals(const Vector& v, float epsilon = 0.01f) const
	{
		bool X = x<v.x + epsilon && x>v.x - epsilon;
		bool Y = y<v.y + epsilon && y>v.y - epsilon;
		bool Z = z<v.z + epsilon && z>v.z - epsilon;

		return X && Y && Z;
	}

	// For compatibility with, you know, vec3_t
	inline void CopyToArray(float* v) const
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	}

	// Snaps the vector to integers
	inline void Snap()
	{
		x = static_cast<int>(x);
		y = static_cast<int>(y);
		z = static_cast<int>(z);
	}

	// Same as Snap() but returns a new vector, not modifying this one 
	inline Vector Snapped() const
	{
		Vector v(*this);
		v.Snap();
		return v;
	}

public: // Static methods

	// A Vector from pitch and yaw angles, using spherical coordinates
	// Expects angles in degrees
	static Vector FromAngles(float pitch, float yaw, float radius = 1.0f);

	// Makes 3 directional vectors from given PYR angles
	// Note: the 'right' vector should be subtracted from any vector 
	//       you wanna add it to, since the positive Y axis is actually left!
	// Note 2: Expects angles to be in degrees
	// Note 3: forward, right and up can be nullptr; they're ignored in that case
	static void AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up);

public: // Operators

	// Vector + Vector
	inline Vector operator+ (const Vector& operand) const
	{
		return Vector(x + operand.x, y + operand.y, z + operand.z);
	}

	// Vector - Vector
	inline Vector operator- (const Vector& operand) const
	{
		return Vector(x - operand.x, y - operand.y, z - operand.z);
	}

	// Vector += Vector
	inline const Vector& operator+= (const Vector& operand)
	{
		x += operand.x;
		y += operand.y;
		z += operand.z;

		return *this;
	}

	// Vector -= Vector
	inline const Vector& operator-= (const Vector& operand)
	{
		x -= operand.x;
		y -= operand.y;
		z -= operand.z;

		return *this;
	}

	// Vector == Vector
	inline bool operator== (const Vector& v)
	{
		return x == v.x && y == v.y && z == v.z;
	}

	// Vector * float
	inline Vector operator* (const float& operand) const
	{
		return Vector(x * operand, y * operand, z * operand);
	}

	// Vector / float
	inline Vector operator/ (const float& operand) const
	{
		return Vector(x / operand, y / operand, z / operand);
	}

	// Vector *= float
	inline const Vector& operator*= (const float& operand)
	{
		x *= operand;
		y *= operand;
		z *= operand;

		return *this;
	}

	// Vector /= float
	inline const Vector& operator/= (const float& operand)
	{
		x /= operand;
		y /= operand;
		z /= operand;

		return *this;
	}

	// This is dot product, not multiply!
	// Revise vector maths
	inline float operator* (const Vector& operand) const
	{
		return x * operand.x + y * operand.y + z * operand.z;
	}

	// = Vector
	inline void operator= (const Vector& assigned)
	{
		x = assigned.x;
		y = assigned.y;
		z = assigned.z;
	}

	// Special operator to convert into a vec3_t
	inline operator float* ()
	{
		return &x;
	}

	// There are some special cases where this is needed
	inline operator const float* () const
	{
		return &x;
	}

public: // Some constants
	static const Vector Zero;

public: // Actual variables
	float x, y, z;
};

// Support for float * Vector
inline Vector operator* (float operand, const Vector& vector)
{
	return vector * operand;
}


#endif // __SHARED_MATH_VEC3_T_H__