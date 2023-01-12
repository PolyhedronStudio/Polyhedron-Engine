/***
*
*	License here.
*
*	@file
*
*	Math Lib: Contains utility functions for making life with GLM easier.
* 
***/
#pragma once

#include "Shared/Shared.h"

#include "glm/gtx/rotate_vector.hpp"



/**
*
*
*	Vector Utilities:
*
*
**/
/**
*	@return	A glm::vec3 containing the values of the passed varliable's Polyhedron vec3_t type.
**/
inline const glm::vec3 phvec_to_glmvec3( const vec3_t &phv ) {
	return glm::vec3( phv.x, phv.y, phv.z );
}
/**
*	@return	A glm::vec4 containing the values of the passed varliable's Polyhedron vec3_t type with its w component
*			defaulting to 1, and can be optionally changed..
**/
inline const glm::vec4 phvec_to_glmvec4( const vec3_t &phv, float w = 1.f ) {
	return glm::vec4( phv.x, phv.y, phv.z, w );
}

/**
*	@return	A Polyhedron vec3_t containing the X,Y,Z components of the glm::vec3
**/
inline const vec3_t glmvec3_to_phvec( const glm::vec3 &glmv ) {
	return { glmv.x, glmv.y, glmv.z };
}
/**
*	@return	A Polyhedron vec3_t containing the X,Y,Z components of the glm::vec4
**/
inline const vec3_t glmvec4_to_phvec( const glm::vec4 &glmv ) {
	return { glmv.x, glmv.y, glmv.z };
}



/**
*
*
*	Matrix Utilities:
*
*
**/
/**
*	@return	An identity matrix that is adjusted to the Quake Coordinate System( Where X,Y,Z turns into: X,Z,-Y ).
**/
inline const glm::mat4 ph_mat_identity() {
	//! The actual identity matrix.
	static const glm::mat4 _matPhIdentity = glm::mat4(
			glm::quatLookAtLH( glm::vec3( 0.f, 0.f, 1.f ), glm::vec3( 0, 1.f, 0.f ) )
		) * glm::mat4( 1.0 );

	// Return.
	return _matPhIdentity;
}



/**
*
*
*	Quaternion Utilities:
*
*
**/
/**
*	@brief	Normalization of the quaternion is optional, and enabled by default.
*	@return	A 'rotation' glm::quat set to the 'roll', 'pitch', and 'yaw' angles. 
**/
inline constexpr glm::quat glm_quat_from_ph_euler( const float roll, const float pitch, const float yaw, const bool normalize = true ) {
	// calculate trig identities
	//float cr = cos(roll/2);
	//float cp = cos(pitch/2);
	//float cy = cos(yaw/2);
	//float sr = sin(roll/2);
	//float sp = sin(pitch/2);
	//float sy = sin(yaw/2);
	//float cpcy = cp * cy;
	//float spsy = sp * sy;
	// Return the euler derived quaternion.
	//return glm::quat(
	//	cr * cpcy + sr * spsy, // w
	//	sr * cpcy - cr * spsy, // x
	//	cr * sp * cy + sr * cp * sy, // y
	//	cr * cp * sy - sr * sp * cy
	//);
	if ( normalize ) {
		return glm::normalize( glm::quat( glm::vec3( roll, pitch, yaw ) ) );
	} else {
		return glm::quat( glm::vec3( roll, pitch, yaw ) );
	}
}

/**
*	@return	A Polyhedron vec3_t containing the euler coordinates of the Quaternions rotation.
**/
inline const vec3_t glm_quat_to_ph_euler( const glm::quat &quat ) {
	// Extract Yaw Pitch Roll.
	const float slerpAnglesYaw = glm::yaw( quat );
	const float slerpAnglesPitch = glm::pitch( quat );
	const float slerpAnglesRoll = glm::roll( quat );

	// Return as euler vec3_t.
	return { slerpAnglesPitch, slerpAnglesYaw, slerpAnglesRoll };
}

/**
*	@brief	A wrapper for accepting euler angles straight from a Polyhedron vec3_t	
*	@return	The results of glm_quat_from_ph_euler( roll, pitch, yaw ) of said eulerAngles.
**/
inline constexpr glm::quat glm_quat_from_ph_euler( const vec3_t &eulerAngles ) {
	return glm_quat_from_ph_euler( glm::radians( eulerAngles[vec3_t::Roll] ), glm::radians( eulerAngles[ vec3_t::Pitch ] ), glm::radians( eulerAngles[vec3_t::Yaw] ) );
}

/**
*	@return	The slerped euler angles by fraction, between anglesA and anglesB.
**/
inline const vec3_t glm_quat_slerp_ph_euler( const vec3_t &anglesA, const vec3_t &anglesB, const float fraction ) {
	// Calculate rotation angle A quaternion.
	glm::quat aRotation = glm_quat_from_ph_euler( anglesA );
	// Calculate rotation angle B quaternion.
	glm::quat bRotation = glm_quat_from_ph_euler( anglesB );

	// Slerp
	glm::quat slerpRotation = glm::normalize( glm::slerp( aRotation, bRotation, fraction ) );

	return glm_quat_to_ph_euler( slerpRotation );
}