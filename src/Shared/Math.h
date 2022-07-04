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

//-----------------
// Utilities
//-----------------
#include "Math/Utilities.h"

//-----------------
// Vectors
//-----------------
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Vector5.h"

//-----------------
// Matrixes
//-----------------
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"

//-----------------
// Plane
//-----------------
#include "Math/Plane.h"

//-----------------
// Colors
//-----------------
#include "Math/Color.h"

//-----------------
// Rectangles.
//-----------------
#include "Math/Rectangle.h"

//-----------------
// Quaternion.
//-----------------
#include "Math/Quaternion.h"




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