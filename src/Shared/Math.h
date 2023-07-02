/***
*
*	License here.
*
*	@file
* 
*   Shared Math Library.
*
***/
#pragma once

/**
*	Utilities
**/
#include "Math/Utilities.h"




/**
*	Vectors
**/
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Vector5.h"

/**
*	Matrices
**/
#include "Math/Matrix3x3.h"
#include "Math/Matrix4x4.h"
/**
*	Ultimately needs relocating.
**/
// plane_t structure
//-----------------
typedef struct cplane_s {
    vec3_t  normal = vec3_zero();
    float   dist = 0;
    byte    type = 0;           //! For fast side tests.
    byte    signBits = 0;       //! signx + (signy<<1) + (signz<<1)
    byte    pad[2] = {};
} CollisionPlane;
/**
*	Sphere
**/
#include "Math/Sphere.h"

/**
*	BoundingBox(vec3_t)
**/
#include "Math/BBox3.h"

/**
*	Plane
**/
#include "Math/Plane.h"

/**
*	Colors
**/
#include "Math/Color.h"

/**
*	Rectangles.
**/
#include "Math/Rectangle.h"

/**
*	Quaternions
**/
#include "Math/Quaternion.h"
#include "Math/DualQuaternion.h"


/**
*	GLM Binding/Utilities
**/
#include "Math/GLM.h"

//-----------------
// Specific Byte and Direction Vector Utilities.
//-----------------
#define NUMVERTEXNORMALS    162
extern const vec3_t normalizedByteDirectionTable[NUMVERTEXNORMALS];

/**
*	@return The byteIndex of the normalized direction vector lookup table that closest matches the direction vector.
**/
int32_t DirectionToByte(const vec3_t &dir);
/**
*	@brief	Gets the direction vector to the normalized direction vector table, positioned at 'byteindex'.
**/
void ByteToDirection(int32_t byteIndex, vec3_t &direction);