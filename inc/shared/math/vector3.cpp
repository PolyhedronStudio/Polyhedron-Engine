/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
//
// shared/math/vector3.cpp
// 
// Vector class, kindly taken from https://github.com/Admer456/ioq3-burek
//
#include "shared/shared.h"
#include "shared/math/vector3.hpp"

constexpr float MPi = 3.14159f;

const Vector Vector::Zero = Vector(0, 0, 0);

Vector Vector::Normalized() const
{
	float length = Length();
	if (length == 0.0f)
	{
		// Approximate infinity, I guess?
		length = 0.00000001f;
	}

	return *this / length;
}

Vector Vector::ToAngles(bool flipPitch) const
{
	float forward;
	float yaw, pitch;

	if (y == 0 && x == 0)
	{
		yaw = 0;
		if (z > 0)
		{
			pitch = 90;
		}

		else
		{
			pitch = 270;
		}
	}

	else
	{
		if (x)
		{
			yaw = (float)(atan2(y, x) * 180 / MPi);
		}

		else if (y > 0)
		{
			yaw = 90;
		}

		else
		{
			yaw = 270;
		}

		if (yaw < 0)
		{
			yaw += 360;
		}

		forward = sqrt(x * x + y * y);
		pitch = (float)(atan2(z, forward) * 180 / MPi);

		if (pitch < 0)
		{
			pitch += 360;
		}
	}

	return Vector(flipPitch ? pitch : -pitch, yaw, 0);
}

inline const Vector& Vector::RotatedAboutAxis(const Vector& axis, float angle) const
{
	Vector cosV = cos(angle) * *this;
	Vector K = (1.0f - cos(angle)) * (axis * *this) * axis;
	Vector cross = sin(angle) * CrossProduct(axis);

	return cosV + K + cross;
}


inline const Vector& Vector::ProjectedOnPlane(const Vector& normal)
{
	float inverseDenominator = 1.0f / (normal * normal);
	float dot = (normal * *this) * inverseDenominator;
	Vector inverseNormal = normal * inverseDenominator;

	return *this - (dot * inverseNormal);
}

Vector Vector::FromAngles(float pitch, float yaw, float radius)
{
	pitch = pitch * (MPi * 2 / 360);
	yaw = yaw * (MPi * 2 / 360);

	return Vector(
		cos(yaw) * sin(pitch) * radius,
		sin(yaw) * sin(pitch) * radius,
		cos(pitch) * radius
	);
}

void Vector::AngleVectors(const Vector& angles, Vector* forward, Vector* right, Vector* up)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = angles.y * (MPi * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles.x * (MPi * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles.z * (MPi * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if (up)
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
