// LICENSE HERE.

//
// shared/math/vector3.cpp
//
#include "Shared/Shared.h"

/*
==================
SetupRotationMatrix

Setup rotation matrix given the normalized direction vector and angle to rotate
around this vector. Adapted from Mesa 3D implementation of _math_matrix_rotate.
==================
*/
void SetupRotationMatrix(vec3_t* matrix, const vec3_t& dir, float degrees)
{
    vec_t   angle, s, c, one_c, xx, yy, zz, xy, yz, zx, xs, ys, zs;

    angle = Radians(degrees);
    s = std::sinf(angle);
    c = std::cosf(angle);
    one_c = 1.0F - c;

    xx = dir.xyz[0] * dir.xyz[0];
    yy = dir.xyz[1] * dir.xyz[1];
    zz = dir.xyz[2] * dir.xyz[2];
    xy = dir.xyz[0] * dir.xyz[1];
    yz = dir.xyz[1] * dir.xyz[2];
    zx = dir.xyz[2] * dir.xyz[0];
    xs = dir.xyz[0] * s;
    ys = dir.xyz[1] * s;
    zs = dir.xyz[2] * s;

    matrix[0].xyz[0] = (one_c * xx) + c;
    matrix[0].xyz[1] = (one_c * xy) - zs;
    matrix[0].xyz[2] = (one_c * zx) + ys;

    matrix[1].xyz[0] = (one_c * xy) + zs;
    matrix[1].xyz[1] = (one_c * yy) + c;
    matrix[1].xyz[2] = (one_c * yz) - xs;

    matrix[2].xyz[0] = (one_c * zx) - ys;
    matrix[2].xyz[1] = (one_c * yz) + xs;
    matrix[2].xyz[2] = (one_c * zz) + c;
}


void RotatePointAroundVector(vec3_t& dst, const vec3_t& dir, const vec3_t& point, float degrees)
{
    vec3_t  matrix[3];

    SetupRotationMatrix(matrix, dir, degrees);

    dst.x = DotProduct(matrix[0], point);
    dst.y = DotProduct(matrix[1], point);
    dst.z = DotProduct(matrix[2], point);
}


#if USE_REF

#if USE_REF == REF_SOFT

void ProjectPointOnPlane(vec3_t& dst, const vec3_t& p, const vec3_t& normal)
{
    float d;
    vec3_t n;
    float inv_denom;

    inv_denom = 1.0F / DotProduct(normal, normal);

    d = DotProduct(normal, p) * inv_denom;

    n[0] = normal[0] * inv_denom;
    n[1] = normal[1] * inv_denom;
    n[2] = normal[2] * inv_denom;

    dst[0] = p[0] - d * n[0];
    dst[1] = p[1] - d * n[1];
    dst[2] = p[2] - d * n[2];
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector(vec3_t& dst, const vec3_t& src)
{
    int pos;
    int i;
    float minelem = 1.0F;
    vec3_t tempvec;

    /*
    ** find the smallest magnitude axially aligned vector
    */
    for (pos = 0, i = 0; i < 3; i++) {
        if (fabs(src[i]) < minelem) {
            pos = i;
            minelem = fabs(src[i]);
        }
    }
    tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
    tempvec[pos] = 1.0F;

    /*
    ** project the point onto the plane defined by src
    */
    ProjectPointOnPlane(dst, tempvec, src);

    /*
    ** normalize the result
    */
    VectorNormalize(dst);
}

#endif  // USE_REF == REF_SOFT

#endif  // USE_REF