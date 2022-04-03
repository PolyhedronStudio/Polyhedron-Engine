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
// TODO: These should ultimately be removed and/or replaced.
//-----------------
#define NUMVERTEXNORMALS    162
extern const vec3_t bytedirs[NUMVERTEXNORMALS];
