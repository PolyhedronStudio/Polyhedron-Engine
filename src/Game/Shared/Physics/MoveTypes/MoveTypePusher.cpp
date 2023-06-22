/***
*
*	License here.
*
*	@file
*
*	Both the ClientGame and the ServerGame modules share the same general Physics code.
* 
***/
#pragma once

//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"

// Physics.
#include "../Physics.h"
#include "../RootMotionMove.h"

#ifdef SHAREDGAME_CLIENTGAME
#include "Game/Client/Entities/GibEntity.h"
#endif

#define PRINT_DEBUG_ROTATOR_CLIPPING

/***
*
*
*	NEW ROTATION MATRIX
*
*
***/

void CM_AngleVectors( const vec3_t angles, vec3_t &forward, vec3_t &right, vec3_t &up )
{
	//{
	//		float cr = cos(roll/2);
	//float cp = cos(pitch/2);
	//float cy = cos(yaw/2);
	//float sr = sin(roll/2);
	//float sp = sin(pitch/2);
	//float sy = sin(yaw/2);
	//float cpcy = cp * cy;
	//float spsy = sp * sy;
	//// Return the euler derived quaternion.
	//return glm::quat(
	//	cr * cpcy + sr * spsy,			// w
	//	sr * cpcy - cr * spsy,			// x
	//	cr * sp * cy + sr * cp * sy,	// y
	//	cr * cp * sy - sr * sp * cy		// z
	//	}
	float angle;
	static float sr, sp, sy, cr, cp, cy, t;
	// static to help MS compiler fp bugs

	angle = Radians( angles[vec3_t::Yaw] );
	sy = sin( angle );
	cy = cos( angle );
	angle = Radians( angles[vec3_t::Pitch] );
	sp = sin( angle );
	cp = cos( angle );
	angle = Radians( angles[vec3_t::Roll] );
	sr = sin( angle );
	cr = cos( angle );

	if( forward )
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if( right )
	{
		t = sr*sp;
		right[0] = ( -1*t*cy+ -1*cr* -sy );
		right[1] = ( -1*t*sy+ -1*cr*cy );
		right[2] = -1*sr*cp;
	}
	if( up )
	{
		t = cr*sp;
		up[0] = ( t*cy+ -sr* -sy );
		up[1] = ( t*sy+ -sr*cy );
		up[2] = cr*cp;
	}
}

void CM_AnglesToAxis( const vec3_t &angles, vec3_t axis[3])
{
	CM_AngleVectors( angles, axis[0], axis[1], axis[2] );
	VectorInverse( axis[1] );
}
static void CM_Matrix_TransformVector( vec3_t m[3], vec3_t &v, vec3_t &out )
{
	out[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
	out[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
	out[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
}


/////////////////////////// TODO: REMOVE THESE AND GET ACCESS FROM EM ELSEWHERE.

/////////////////////////// TODO: REMOVE THESE AND GET ACCESS FROM EM ELSEWHERE.
const vec3_t G_CalculateAngularMove( const vec3_t &pusherOrigin, const vec3_t &riderOrigin, const vec3_t &angularMove ) {
	//vec3_t matrix[3], transpose[3];
	//G_CreateRotationMatrix( angularMove, transpose );
	//G_TransposeMatrix( transpose, matrix );
	//// geCheck->GetClient()->playerState.pmove.origin <-- should be the actual current moment in time origin if linear mover.
	//vec3_t org = riderOrigin - pusherOrigin;
	//vec3_t org2 = org;
	//G_RotatePoint( org2, matrix );
	//const vec3_t finalMove = org2 - org;
	//return finalMove;

	//vec3_t axis[3];
	//CM_AnglesToAxis( vec3_zero() - angularMove, axis );
	//vec3_t temp = riderOrigin - pusherOrigin;
	//const vec3_t org2 = temp;
	//vec3_t finalMove = vec3_zero();
 //   CM_Matrix_TransformVector( axis, temp, finalMove );
	//return finalMove - org2;

	//vec3_t modelSpaceOrigin = riderOrigin - pusherOrigin;

	//glm::mat4 matTrans = glm::translate( glm::mat4( 1.0 ), phvec_to_glmvec3( pusherOrigin ) );
	////glm::mat4 matEulerAngles = glm::eulerAngleXYZ( glm::radians( glmEuler.x ), glm::radians( glmEuler.y ), glm::radians( glmEuler.z ) );
	//const vec3_t eulerAngles = vec3_clamp_euler( vec3_zero() - angularMove ); 
	//glm::mat4 matRotation = glm::rotate( matTrans,    Radians(-eulerAngles[ vec3_t::PYR::Yaw ] ),	glm::vec3( 1.0, 0.0, 0.0 ) );
	//matTrans = matTrans * matRotation;
	////glm::mat4 finalTrans = glm::translate( matTrans, phvec_to_glmvec3( vec3_negate( modelSpaceOrigin ) ) );


	vec3_t modelSpaceOrigin = riderOrigin - pusherOrigin;

	// Translate into pusher origin.
	glm::mat4 matPusherTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3(pusherOrigin));
	// Now rotate it into its rotation.
	const vec3_t eulerAngles = vec3_clamp_euler( vec3_negate( angularMove ) ); 
	//glm::mat4 matRotation = glm::rotate( ph_mat_identity(), glm::radians(eulerAngles.x), glm::vec3(1.0, 0.0, 0.0));
	//matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.y ), glm::vec3( 0.0, 0.0, 1.0 ) );
	//matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.z ), glm::vec3( 0.0, 1.0, 0.0 ) );
	glm::mat4 matBoxRotation = glm::rotate( matBoxRotation, glm::radians( -eulerAngles.y ), glm::vec3( 0.0, 0.0, 1.0 ) );
	matBoxRotation = glm::rotate( glm::mat4( 1.0 ), glm::radians( eulerAngles.x ), glm::vec3( 1.0, 0.0, 0.0 ) );
	matBoxRotation = glm::rotate( matBoxRotation, glm::radians( eulerAngles.z ), glm::vec3( 0.0, 1.0, 0.0 ) );

	//// Invert its translation.
	matPusherTranslate = matBoxRotation * glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( pusherOrigin ) ) );

	//// Create translation matrix of said entity.
	glm::mat4 matRiderTranslate = glm::translate( ph_mat_identity(), phvec_to_glmvec3( riderOrigin ) );

	//// Transform by matPusherTranslate matrix.
	glm::vec4 finalTransformOrigin = matPusherTranslate * glm::vec4( phvec_to_glmvec3( riderOrigin ), 1 );
	//glm::mat4 matEulerAngles = glm::eulerAngleXYZ( glm::radians( glmEuler.x ), glm::radians( glmEuler.y ), glm::radians( glmEuler.z ) );

//----------------
	//glm::mat4 matTrans = glm::translate( ph_mat_identity(), phvec_to_glmvec3( pusherOrigin ) );
	//////glm::mat4 matEulerAngles = glm::eulerAngleXYZ( glm::radians( glmEuler.x ), glm::radians( glmEuler.y ), glm::radians( glmEuler.z ) );
	//const vec3_t eulerAngles = vec3_clamp_euler( vec3_negate( angularMove ) ); 
	//////glm::mat4 matRotation = glm::rotate( glm::identity<glm::mat4>(), glm::radians( eulerAngles.y ), glm::vec3( 0.0, 0.0, 1.0 ) );
	//////	matRotation = glm::rotate( matRotation, glm::radians( -eulerAngles.x ), glm::vec3( 0.0, 1.0, 0.0 ) );
	//////	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.z ), glm::vec3( 1.0, 0.0, 0.0 ) );

	////	//glm::mat4 matRotation = glm::rotate( glm::identity<glm::mat4>(), eulerAngles.y, glm::vec3( 0.0, 1.0, 0.0 ) );
	////	//matRotation = glm::rotate( matRotation, -eulerAngles.x, glm::vec3( 1.0, 0.0, 0.0 ) );
	////	//matRotation = glm::rotate( matRotation, eulerAngles.z, glm::vec3( 0.0, 0.0, 1.0 ) );

	//glm::mat4 matRotation = glm::rotate( glm::mat4( 1.0 ), glm::radians( eulerAngles.x ), glm::vec3( 1.0, 0.0, 0.0 ) );
	//	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.y ), glm::vec3( 0.0, 0.0, 1.0 ) );
	//	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.z ), glm::vec3( 0.0, 1.0, 0.0 ) );

	////glm::mat4 finalTrans = matTrans * matRotation * glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( modelSpaceOrigin ) ) );
	//glm::mat4 finalTrans = matTrans * matRotation * glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( modelSpaceOrigin ) ) );
	//glm::vec3 finalTransformOrigin = finalTrans[3];
//--------------------
	//glm::mat4 matTrans = glm::translate( ph_mat_identity(), phvec_to_glmvec3( pusherOrigin ) );
	////glm::mat4 matEulerAngles = glm::eulerAngleXYZ( glm::radians( glmEuler.x ), glm::radians( glmEuler.y ), glm::radians( glmEuler.z ) );
	//const vec3_t eulerAngles = vec3_clamp_euler( vec3_negate( angularMove ) ); 
	////glm::mat4 matRotation = glm::rotate( glm::identity<glm::mat4>(), glm::radians( eulerAngles.y ), glm::vec3( 0.0, 0.0, 1.0 ) );
	////	matRotation = glm::rotate( matRotation, glm::radians( -eulerAngles.x ), glm::vec3( 0.0, 1.0, 0.0 ) );
	////	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.z ), glm::vec3( 1.0, 0.0, 0.0 ) );

	//	//glm::mat4 matRotation = glm::rotate( glm::identity<glm::mat4>(), eulerAngles.y, glm::vec3( 0.0, 1.0, 0.0 ) );
	//	//matRotation = glm::rotate( matRotation, -eulerAngles.x, glm::vec3( 1.0, 0.0, 0.0 ) );
	//	//matRotation = glm::rotate( matRotation, eulerAngles.z, glm::vec3( 0.0, 0.0, 1.0 ) );
	//glm::mat4 matRotation = glm::rotate( ph_mat_identity(), glm::radians(eulerAngles.y), glm::vec3(0.0, 0.0, 1.0));
	//	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.x ), glm::vec3( 0.0, 1.0, 0.0 ) );
	//	matRotation = glm::rotate( matRotation, glm::radians( eulerAngles.z ), glm::vec3(1.0, 0.0, 0.0));

	//glm::mat4 finalTrans = glm::translate( ph_mat_identity(), phvec_to_glmvec3( vec3_negate( modelSpaceOrigin ) ) ) * matRotation * matTrans;
	//glm::vec3 finalTransformOrigin = finalTrans[3];


	//matTrans = matTrans * matRotation;
	//glm::mat4 finalTrans = glm::translate( matTrans, phvec_to_glmvec3( vec3_negate( modelSpaceOrigin ) ) );

	return glmvec3_to_phvec( glm::vec3( finalTransformOrigin.x / finalTransformOrigin.w, finalTransformOrigin.y / finalTransformOrigin.w, finalTransformOrigin.z / finalTransformOrigin.w ));
}


/***
*
*
*	ROTATION MATRIX
*
*
***/
//void G_CreateRotationMatrix( const vec3_t angles, vec3_t *matrix ) {
//	AngleVectors( angles, &matrix[0], &matrix[1], &matrix[2] );
//	//VectorInverse(matrix[1]);
//	matrix[1] = vec3_negate( matrix[1] );
//}
//
///*
//================
//G_TransposeMatrix
//================
//*/
//void G_TransposeMatrix( vec3_t *matrix, vec3_t *transpose ) {
//	int i, j;
//	for (i = 0; i < 3; i++) {
//		for (j = 0; j < 3; j++) {
//			transpose[i][j] = matrix[j][i];
//		}
//	}
//}
//
///*
//================
//G_RotatePoint
//================
//*/
//void G_RotatePoint( vec3_t &point, vec3_t *matrix) {
//	vec3_t tvec;
//
//	VectorCopy(point, tvec);
//	point[0] = DotProduct(matrix[0], tvec);
//	point[1] = DotProduct(matrix[1], tvec);
//	point[2] = DotProduct(matrix[2], tvec);
//}
//const vec3_t G_CalculateAngularMove( const vec3_t &pusherOrigin, const vec3_t &riderOrigin, const vec3_t &angularMove ) {
//	vec3_t matrix[3], transpose[3];
//	G_CreateRotationMatrix( angularMove, transpose );
//	G_TransposeMatrix( transpose, matrix );
//	// geCheck->GetClient()->playerState.pmove.origin <-- should be the actual current moment in time origin if linear mover.
//	vec3_t org = riderOrigin - pusherOrigin;
//	vec3_t org2 = org;
//	G_RotatePoint( org2, matrix );
//	const vec3_t finalMove = org2 - org;
//	return finalMove;
//}



/***
*
*
*	PUSH/POP ENTITY STATE OF MOMENT IN TIME.
*
*
***/
/**
*	@brief	Contains data of entities that are pushed by MoveType::Push objects. (BrushModels usually.)
**/
struct PushedEntityState {
	int32_t entityNumber = -1;
	//int32_t entityNumber = -1;
	//GameEntity *entityHandle;
	vec3_t origin = vec3_zero();
	vec3_t angles = vec3_zero();
	float deltaYaw = 0.f;
	// This is used as velocity wtf?
	vec3_t playerMoveOrigin = vec3_zero();

	// The actual time at which this entity's delta move is at.
	int64_t moveTime = 0;
	int64_t moveNextTime = 0;
};

//! Reserved size std::vector containing all the pushed entity states.
//static std::vector<PushedEntityState> pushedEntities(MAX_POD_ENTITIES);
static PushedEntityState pushedEntities[MAX_POD_ENTITIES];

//! Pointer to the LAST pushed entity state.
static PushedEntityState *lastPushedEntityState = nullptr;
static int32_t lastPushedStateNumber = 0;

//! Pointer to a game entity that's pointing to our pushed obstacle.
static GameEntity *pushObstacle = nullptr;

/**
*	@brief	Utilitzes the EntityHandle class to retreive a valid GameEntity for
*			the Pushed Entity State.
*
*	@return On success, a pointer to a valid GameEntity. On failure, a (nullptr).
**/
static GameEntity *SG_GetGameEntityFromPushedState( PushedEntityState *pushedEntityState) {
	// Get GameWorld.
	SGGameWorld *gameWorld = GetGameWorld();

	if ( pushedEntities->entityNumber < 0 ) {
		return nullptr;
	}

	// Utilize the EntityHandle class.
	SGEntityHandle ehPushedStateEntity = gameWorld->GetPODEntityByIndex( pushedEntityState->entityNumber );

	// Validate the actual EntityHandle and get a hopefully non (nullptr) GameEntity. 
	GameEntity *validEntity = SGGameWorld::ValidateEntity( ehPushedStateEntity );

	// Return our pointer.
	return validEntity;
}

/**
*	@brief	Pushes a new Pushed Entity State for the gePusher Game Entity.
**/
const PushedEntityState SG_PushEntityState( GameEntity* gePushMove ) {
	const bool isClientEntity = ( gePushMove->GetClient() != nullptr ? true : false );
	const float deltaYaw = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] : gePushMove->GetAngles()[vec3_t::Yaw] );
	#ifdef SHAREDGAME_CLIENTGAME
	//const vec3_t playerMoveOrigin = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	const vec3_t playerMoveOrigin = ( isClientEntity ? cl->predictedState.viewOrigin : vec3_zero() ); //gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	#else
	const vec3_t playerMoveOrigin = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	#endif
	#ifdef SHAREDGAME_CLIENTGAME
	//gePushMove->EnableExtrapolation();
	#endif
	// Create state.
	PushedEntityState returnedState = *lastPushedEntityState;
	*lastPushedEntityState = {
		.entityNumber = gePushMove->GetNumber(),
		.origin = gePushMove->GetOrigin(),
		.angles = gePushMove->GetAngles(),
		.deltaYaw = deltaYaw,
		.playerMoveOrigin = playerMoveOrigin,

		#ifdef SHAREDGAME_CLIENTGAME
		.moveTime = ( level.extrapolatedTime ).count(),
		.moveNextTime = ( level.extrapolatedTime + FRAMERATE_MS ).count(),
		#else
		.moveTime = (level.time).count(),
		.moveNextTime = (level.time + FRAMERATE_MS).count(),
		#endif
	};
	lastPushedEntityState++;

	return returnedState;
}

// TODO: Store which angles we pushed and pop those back instead of angles and then yaw only.
void SG_PopPushedEntityState( const bool linkEntity = true, gclient_s *geCheckClient = nullptr, const bool fetchClientFromEntity = false ) {
	// Pop.
	lastPushedEntityState--;

	// Get gameworld.
	GameEntity *gePushMove = SG_GetGameEntityFromPushedState( lastPushedEntityState );
	if ( !gePushMove ) {
		// TODO: WARN.
		return;
	}

	// Revert state.
	// if client.
	//if ( client ) {
	//	if ( gePushMove->GetClient() ) {
	//		// Correct entity angles.
	//		gePushMove->GetClient()->playerState.pmove.origin = lastPushedEntityState->playerMoveOrigin;
	//		gePushMove->GetClient()->playerState.pmove.deltaAngles[ vec3_t::Yaw ] = lastPushedEntityState->deltaYaw;
	//	}
	//} else {
	//	gePushMove->SetOrigin( lastPushedEntityState->origin );
	//	//gePushMove->SetAngles( lastPushedEntityState->angles );
	//	// if getclient {
	//	{
	//		vec3_t angles = lastPushedEntityState->angles;
	//		angles[ vec3_t::Yaw ] = lastPushedEntityState->deltaYaw;
	//		gePushMove->SetAngles( angles );
	//	}
	//}
	//if ( linkEntity ) {
	//	gePushMove->LinkEntity();
	//}
	//#ifdef SHAREDGAME_CLIENTGAME
	////gePushMove->DisableExtrapolation();
	//#endif


	/*
	USED TO WORK:
	if ( fetchClientFromEntity) {
		geCheckClient = gePushMove->GetClient();
	}
	if ( geCheckClient ) {
		// Correct entity angles.
		geCheckClient->playerState.pmove.origin = lastPushedEntityState->playerMoveOrigin;
		geCheckClient->playerState.pmove.deltaAngles[ vec3_t::Yaw ] = lastPushedEntityState->deltaYaw;
	} else {
		gePushMove->SetOrigin( lastPushedEntityState->origin );
		gePushMove->SetAngles( lastPushedEntityState->angles );
		if ( linkEntity ) {
			gePushMove->LinkEntity();
		}
	}*/
	gePushMove->SetOrigin( lastPushedEntityState->origin );
	gePushMove->SetAngles( lastPushedEntityState->angles );
	if ( fetchClientFromEntity) {
		geCheckClient = gePushMove->GetClient();
	}
	if ( geCheckClient ) {
		// Correct entity angles.
		geCheckClient->playerState.pmove.origin = lastPushedEntityState->playerMoveOrigin;
		geCheckClient->playerState.pmove.deltaAngles[ vec3_t::Yaw ] = lastPushedEntityState->deltaYaw;
	}

	#ifdef SHAREDGAME_CLIENTGAME
	//gePushMove->DisableExtrapolation();
	#endif
}


/***
*
*
*	Utility functions for checking and performing entity adjustments.
*
*
***/
/**
*	@brief	TODO: Move into elsewhere.
**/
const SGTraceResult SG_Clip( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, GameEntity *geSkipEntity, GameEntity *geClipEntity, const int32_t contentMask ) {
	PODEntity *podSkipEntity = ( geSkipEntity ? geSkipEntity->GetPODEntity() : nullptr );
	PODEntity *podClipEntity = ( geClipEntity ? geClipEntity->GetPODEntity() : nullptr );

	#ifdef SHAREDGAME_CLIENTGAME
		return clgi.Clip( start, mins, maxs, end, podSkipEntity, podClipEntity, contentMask );
	#else
		return gi.Clip( start, mins, maxs, end, podSkipEntity, podClipEntity, contentMask );
	#endif
}
/**
*	@brief	TODO: Move into elsewhere.
**/
const SGTraceResult SG_ClipSphere( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const sphere_t &sphere, GameEntity *geSkipEntity, GameEntity *geClipEntity, const int32_t contentMask ) {
	PODEntity *podSkipEntity = ( geSkipEntity ? geSkipEntity->GetPODEntity() : nullptr );
	PODEntity *podClipEntity = ( geClipEntity ? geClipEntity->GetPODEntity() : nullptr );

	#ifdef SHAREDGAME_CLIENTGAME
		return clgi.ClipSphere( start, mins, maxs, end, sphere, podSkipEntity, podClipEntity, contentMask );
	#else
		return gi.ClipSphere( start, mins, maxs, end, sphere, podSkipEntity, podClipEntity, contentMask );
	#endif
}
/**
*	@brief	TODO: Move into elsewhere.
**/
const SGTraceResult SG_Pusher_ClipEntity( const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end, const sphere_t &sphere, GameEntity *geSkipEntity, GameEntity *geClipEntity, const int32_t contentMask, const int32_t entitySolid ) {
	//PODEntity *podSkipEntity = ( geSkipEntity ? geSkipEntity->GetPODEntity() : nullptr );
	//PODEntity *podClipEntity = ( geClipEntity ? geClipEntity->GetPODEntity() : nullptr );

	//#ifdef SHAREDGAME_CLIENTGAME
	//	return clgi.ClipSphere( start, mins, maxs, end, sphere, podSkipEntity, podClipEntity, contentMask );
	//#else
	//	return gi.ClipSphere( start, mins, maxs, end, sphere, podSkipEntity, podClipEntity, contentMask );
	//#endif
	// Sphere tracing:
	if ( entitySolid == Solid::Sphere) {
		return SG_ClipSphere( start, mins, maxs, end, sphere, geSkipEntity, geClipEntity, contentMask );
	} else {
		return SG_Clip( start, mins, maxs, end, geSkipEntity, geClipEntity, contentMask );
	}
}
/**
*	@brief	SharedGame Trace Functionality: Supports GameEntities :-)
**/
const SGTraceResult SG_Pusher_TraceEntity(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, const sphere_t &sphere, GameEntity* skipGameEntity, const int32_t contentMask, const int32_t entitySolid ) {
//    // Fetch POD Entity to use for pass entity testing.
//    PODEntity* podEntity = (skipGameEntity ? skipGameEntity->GetPODEntity() : NULL);
//
//	// Execute and return the actual trace.
//#ifdef SHAREDGAME_SERVERGAME
//    return gi.Trace( start, mins, maxs, end, (struct PODEntity*)podEntity, contentMask );
//#endif
//#ifdef SHAREDGAME_CLIENTGAME
//    return clgi.Trace( start, mins, maxs, end, (struct PODEntity*)podEntity, contentMask );
//#endif
	if ( entitySolid == Solid::Sphere ) {
		return SG_SphereTrace( start, mins, maxs, end, sphere, skipGameEntity, contentMask );
	} else {
		return SG_Trace( start, mins, maxs, end, skipGameEntity, contentMask );
	}
}

/**
*	@brief	Objects need to be moved back on a failed push, otherwise 
*			riders would continue to slide.
**/
const bool SG_Push_IsSameEntity( GameEntity *geFirst, GameEntity *geSecond ) {
	// The same by pointer.
	const int32_t numberGeFirst		= ( geFirst ? geFirst->GetNumber() : -1 );
	const int32_t numberGeSecond	= ( geSecond ? geSecond->GetNumber() : -1 );

	// If first or second is -1, it is a false. (We do not return true in case of it being empty space)
	if ( numberGeFirst == -1 || numberGeSecond == -1 ) {
		return false;
	}

	if ( numberGeFirst == numberGeSecond ) {
		return true;
	}

	return false;
}

/**
*	@return	The origin of the entity, or if asked for, the entity's client playerstate pmove origin.
**/
const vec3_t SG_Push_GetEntityOrigin(GameEntity *geCheck, gclient_s *geCheckClient = nullptr ) {
	if ( !geCheck ) {
		#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
		SG_Print( PrintType::DeveloperWarning, fmt::format( "Tried to get the origin from a (nullptr) GameEntity.\n" ) );
		#endif
		return vec3_zero();
	}

	// Client Origin Path.
	//if ( geCheckClient ) {
	//	return geCheckClient->playerState.pmove.origin;
	//}


	//if ( geCheck->GetClient() ) { //geCheckClient ) {
	//	return geCheck->GetClient()->playerState.pmove.origin; //geCheckClient->playerState.pmove.origin;
	//// GameEntity Origin Path.
	//} else {
		return geCheck->GetOrigin();
	//}
}
/**
*	@brief	Moves the entity to its newly designated origin.
*	@param	setClientOrigin	When true, if an entity has a client ptr, assigns the origin to its
*			client playerstate pmove origin instead.
**/
const void SG_Push_SetEntityOrigin( GameEntity *geCheck, const vec3_t &geNewOrigin, gclient_s *geCheckClient = nullptr ) {
	if ( !geCheck ) {
		#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
		SG_Print( PrintType::DeveloperWarning, fmt::format( "Tried to push move a (nullptr) GameEntity.\n" ) );
		#endif
		return;
	}

	//// Client Origin Path:
	//if ( geCheckClient ) {
	//	geCheckClient->playerState.pmove.origin = geNewOrigin;
	//// GameEntity Origin Path.
	//} else {
	//	geCheck->SetOrigin( geNewOrigin );
	//}

	// Client Origin Path:
	//if ( geCheckClient ) {
	//	geCheckClient->playerState.pmove.origin = geNewOrigin;
	//// GameEntity Origin Path.
	//} else {
		geCheck->SetOrigin( geNewOrigin );
		if ( geCheck->GetClient() ) {
			geCheck->GetClient()->playerState.pmove.origin = geNewOrigin;
		}
//	}
}

/**
*	@brief	Checks whether the entity is in a good position. (Non Solid.)
*	@param	client	When true, will use the client's origin instead of the GameEntity's origin.
**/
const bool SG_Push_IsValidOrigin( GameEntity *geCheck, gclient_s *geCheckClient = nullptr, SGTraceResult *debugTrace = nullptr ) {
	// Acquire the clip mask to use.
	const int32_t clipMask = SG_SolidMaskForGameEntity( geCheck );

	// Perform a test trace.
	SGTraceResult testTrace;

	const vec3_t testOrigin = SG_Push_GetEntityOrigin( geCheck );
	testTrace = SG_Pusher_TraceEntity( testOrigin, geCheck->GetMins(), geCheck->GetMaxs(), testOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, clipMask, geCheck->GetSolid() );

	// Return a boolean test for the resulting outcome.
	if ( debugTrace ) {
		*debugTrace = testTrace;
	}
	if ( geCheck->GetSolid() == Solid::Sphere ) {
		return ( testTrace.startSolid == false && testTrace.allSolid == false && ( testTrace.fraction == 1.f || !testTrace.podEntity ) );
		//return ( testTrace.allSolid == false || !testTrace.podEntity );
		//return ( testTrace.startSolid == false && testTrace.allSolid == false );
	} else {
		if ( testTrace.startSolid == false && testTrace.allSolid == false /*&& testTrace.fraction == 0.f */) {
			int x = 10;
		}
		return ( testTrace.startSolid == false && testTrace.allSolid == false && ( testTrace.fraction == 1.f || !testTrace.podEntity ) );
	}
}

/**
*	@brief	True if the entity is in, or could be moved, to a valid position, false otherwise.
*	@param	client	When true, will use the client's origin instead of the GameEntity's origin.
**/
const bool SG_Push_CorrectOrigin( GameEntity *geCheck, gclient_s *geCheckClient = nullptr, vec3_t *angularMove = nullptr ) {

	// Offsets to translate origin into.
	/*const *///float offsets[] = { 0, 1, -1 };
	float offsets[] = { 0, 1, -1 };

	// If we got angles, apply them to our offsets.
	//if ( angularMove ) {
	//	const vec3_t normalizedAngles = vec3_normalize( *angularMove );
	//	for (int i = 0; i < 3; i++) {
	//		offsets[i] = offsets[i] * normalizedAngles[i];
	//	}
	//}



	// First test if we're already in a valid position before attempting to correct us.
	SGTraceResult firstTrace;
	SGTraceResult secondTrace; 
	
	// Get Origins.
	vec3_t currentOrigin = SG_Push_GetEntityOrigin( geCheck );

	if ( SG_Push_IsValidOrigin( geCheck, nullptr, &firstTrace ) ) {
		return true;
	}

	if ( *angularMove ) {
		if ( geCheck->GetSolid() == Solid::Sphere ) {
			const float sphereRadius = geCheck->GetPODEntity()->boundsAbsoluteSphere.radius;
			const float scale = ( sphereRadius ) + ( sphereRadius * ( 1.0f / firstTrace.fraction ) );
			SG_Push_SetEntityOrigin( geCheck, vec3_fmaf( currentOrigin, scale, vec3_normalize( vec3_negate( *angularMove ) ) ) );
		} else {
			SG_Push_SetEntityOrigin( geCheck, vec3_fmaf( currentOrigin, 1.f + (firstTrace.fraction), vec3_normalize( vec3_negate( *angularMove ) ) ) );
		}
		geCheck->LinkEntity();
		if ( SG_Push_IsValidOrigin( geCheck, nullptr, &firstTrace ) ) {
			return true;
		}
	}

	for ( int32_t i = 0; i < 3; i++ ) {
		for ( int32_t j = 0; j < 3; j++ ) {
			for ( int32_t k = 0; k < 3; k++ ) {
				//const vec3_t offset = { offsets[i], offsets[j], offsets[k] };
				vec3_t offset = { offsets[i], offsets[j], offsets[k] };
				//if ( angularMove ) {
				//	offset *= vec3_normalize( vec3_negate( *angularMove ) );
				//}
				// Set origin and link into position.
				/*
				*	GameEntity:
				*/
				SG_Push_SetEntityOrigin( geCheck, currentOrigin + offset );
				geCheck->LinkEntity();

				// Test whether it's a good position.
				if ( SG_Push_IsValidOrigin( geCheck, nullptr, &secondTrace ) ) {
					return true;
				}
			}
		}
	}

	// Still inside of some solid, revert.
	SG_Push_SetEntityOrigin( geCheck, currentOrigin );
	geCheck->LinkEntity();
	#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
		SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): Still inside of a solid, reverting origin.\n", geCheck->GetNumber() ) );
		#ifdef SHAREDGAME_SERVERGAME
		int x = 10;
		#endif
	#endif

	return false;
}

/**
*	@return	True if the entity has a valid 'solid' which we want to 'push' around. False otherwise.
**/
const bool SG_Push_EntityValidSolid( GameEntity *geCheck ) {
	// Get solid.
	const int32_t	pushEntitySolidType = geCheck->GetSolid();

// ClientGame:
#ifdef SHAREDGAME_CLIENTGAME
	// Local entity.
	const bool isLocalEntity = geCheck->GetPODEntity()->isLocal;

	// Solid check.
	if ( pushEntitySolidType == Solid::BSP || pushEntitySolidType == PACKED_BSP ) {
		return false;
	}
#endif
// ServerGame:
#ifdef SHAREDGAME_SERVERGAME
	// Solid check.
	if ( pushEntitySolidType == Solid::BSP ) {
		return false;
	}
#endif

	// Valid solid type
	return true;
}

/**
*	@return	True if the entity has a valid 'move type' which we want to 'push' around. False otherwise.
**/
const bool SG_Push_EntityValidMoveType( GameEntity *geCheck ) {
	// Get MoveType.
    const int32_t	pushEntityMoveType = geCheck->GetMoveType();

// ClientGame:
#ifdef SHAREDGAME_CLIENTGAME
	// Local entity.
	const bool isLocalEntity = geCheck->GetPODEntity()->isLocal;

	// Move Type checks.
	if ( pushEntityMoveType == MoveType::Push || pushEntityMoveType == MoveType::Stop || pushEntityMoveType == MoveType::NoClip || pushEntityMoveType == MoveType::Spectator ||
		( isLocalEntity && pushEntityMoveType == MoveType::None ) ) {
        return false;
	}
#endif
// ServerGame:
#ifdef SHAREDGAME_SERVERGAME
	if ( pushEntityMoveType == MoveType::Push || pushEntityMoveType == MoveType::Stop ||
		pushEntityMoveType == MoveType::None || pushEntityMoveType == MoveType::NoClip ||
		pushEntityMoveType == MoveType::Spectator ) {
        return false;
	}
#endif

	return true;
}

/**
*	@return True if the entity is linked in, false otherwise.
**/
const bool SG_Push_EntityIsLinked( GameEntity *geCheck ) {
    if ( !geCheck->GetPODEntity()->area.prev ) {
        return false;       // not linked in anywhere
	}

	return true;
}

/***
*
*
*	Push Entity: Translate Mechanics.
*
*
***/
/**
*	@brief	Will push the mover into its deltaMove + deltaAngularMove combined offset direction.
*	@return	false if the move failed (i.e could've been blocked.), true if it succeeds.
**/
GameEntity *SG_Pusher_Translate( SGEntityHandle &entityHandle, const vec3_t &pusherOrigin, const vec3_t &deltaMove, const vec3_t &angularDeltaMove ) {
	/**
	*	
	**/
	SGGameWorld *gameWorld = GetGameWorld();

	/**
	*	Validate pusher entity and push its state.
	**/
	// Assign handle to base entity.
    GameEntity* gePusher = SGGameWorld::ValidateEntity(entityHandle);

    // Ensure it is a valid entity.
    if (!gePusher) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
	    return nullptr;
    }
	
	// Push its state.
	SG_PushEntityState( gePusher );

	/**
	*	Calculate bounds of the entire move.
	**/
	// Total bounds.
	bbox3_t totalBounds = gePusher->GetPODEntity()->absoluteBounds;
	// Unlink pusher.
	gePusher->UnlinkEntity();

	// Acquire the original origin we start from.
	const vec3_t originalOrigin = gePusher->GetOrigin();
	// Calculate the final destination origin..
	const vec3_t destinationOrigin = pusherOrigin;

	// Set to destination origin, and link entity in for calculating absolute bounds.
	gePusher->SetOrigin( destinationOrigin );
	// Link pusher back in.
	gePusher->LinkEntity();

	// Union the previous bounds with the destination absolute bounds to get us the full move bounds.
	totalBounds = bbox3_union( totalBounds, gePusher->GetPODEntity()->absoluteBounds );
	totalBounds = bbox3_expand( totalBounds, 1.f );

	/**
	*	Get all entities residing in the move's bounds box.
	**/
	// Get a range of all pushable entities in our world. (A valid GameEntity and Inuse.)
	auto gePushables = SG_BoxEntities( totalBounds.mins, totalBounds.maxs, MAX_POD_ENTITIES, AreaEntities::Solid );

	/**
	*	Iterate all resulting entities of BoxEntities and determine whether they are riding the pusher,
	*	or being pushed by it. If they are we try and move them into their new spot, when any of them
	*	gets blocked we revert all entity moves and return the blocking game entity.
	**/
	for ( auto geCheck : gePushables ) {
		/**
		*	Perform several tests on whether we truly must include this entity in our move.
		**/
		// MoveType:
		if ( !SG_Push_EntityValidMoveType( geCheck ) ) {
			continue;
		}
		// Solid:
		if ( !SG_Push_EntityValidSolid( geCheck ) ) {
			continue;
		}
		// Linked or not:
		if ( !SG_Push_EntityIsLinked( geCheck ) ) {
			continue;
		}

		/**
		*	See if we can skip this entity by checking if it is in a 'good' valid position, AND not riding us.
		**/
		// We need to work the client's pmove as well in case it has one.
		gclient_s *geCheckClient = geCheck->GetClient();

		// Get ground entity and make sure it is validated properly.
		GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );
		// Inspect whether we are riding the pusher, or not.
		const bool isRiderEntity = SG_Push_IsSameEntity( gePusher, geCheckGroundEntity );

		// Ensure we're in a good position.
		/*
		*	GameEntity:
		*/
		if ( SG_Push_IsValidOrigin( geCheck ) && !isRiderEntity && !geCheckClient ) {
			continue;
		}
		/*
		*	Client:
		*/
		if ( geCheckClient && SG_Push_IsValidOrigin( geCheck, geCheckClient ) && !isRiderEntity ) {
			continue;
		}

		/**
		*	If we are the pusher, or if the entity is riding the pusher, move the entity
		**/
		if ( gePusher->GetMoveType() == MoveType::Push || isRiderEntity ) {
			// Push the entity onto our stack.
			SG_PushEntityState( geCheck );

			/**
			*	Rider Route:
			*
			*	Special route for the riders. Make sure we're on top of it, in a good position,
			*	after which we move it by its full translation. 
			*
			*	If we fail to do so, by intersecting with the mover, we might have been pushed off
			*	by other means. Give it's old origin a spin since it's position might now be valid.
			**/
			if ( isRiderEntity ) {
				// Unlink the pusher entity so we can properly trace to our new origin without intersecting.
				gePusher->UnlinkEntity();

				// Try and perform the entire full move.
				/*
				*	GameEntity: 
				*/
				// Calculate trace origins.
				const vec3_t geTraceStart	= SG_Push_GetEntityOrigin( geCheck );
				const vec3_t geTraceEnd		= geTraceStart + deltaMove;

				// Perform trace.
				const SGTraceResult geMoveTrace = SG_Pusher_TraceEntity( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, SG_SolidMaskForGameEntity( geCheck ), geCheck->GetSolid() );
				/*
				*	Client:
				*/
				vec3_t clTraceStart	= vec3_zero();
				vec3_t clTraceEnd	= vec3_zero();
				SGTraceResult clMoveTrace;	
				if ( geCheckClient ) {
					// Calculate trace origins.
					clTraceStart	= SG_Push_GetEntityOrigin( geCheck, geCheckClient );
					clTraceEnd		= clTraceStart + deltaMove;

					// Perform trace.
					clMoveTrace = SG_Pusher_TraceEntity( clTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), clTraceEnd, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, SG_SolidMaskForGameEntity( geCheck ), geCheck->GetSolid() );
				}

				// Link the pusher back in.
				gePusher->LinkEntity();

				// Set origin, and see if the position is correct.
				/*
				*	GameEntity: 
				*/
				//if ( !geCheckClient ) {
					// Move the rider into the trace's resulting end position.
					SG_Push_SetEntityOrigin( geCheck, geMoveTrace.endPosition );

					// See if our origin is good.
					if ( SG_Push_CorrectOrigin( geCheck ) && !geCheckClient ) {
						// ONLY IF: We got no client attached to this entity:
						continue;
					}
				//}
				/*
				*	Client:
				*/
				if ( geCheckClient ) {
					// Move the rider into the trace's resulting end position.
					SG_Push_SetEntityOrigin( geCheck, clMoveTrace.endPosition, geCheckClient );

					// See if our origin is good.
					if ( SG_Push_CorrectOrigin( geCheck, geCheckClient ) ) {
						continue;
					}
				}

				// We intersected with the mover. Probably because something else pushed us off,
				// which is not our intent. Try and resolve it by moving back to the original origin.
				SG_PopPushedEntityState( true, nullptr, true );

				/*
				*	GameEntity: Set origin, and see if the position is correct.
				*/
				// Once again, see if we're good.
				if ( SG_Push_CorrectOrigin( geCheck ) ) {
					// ONLY IF: We got no client attached to this entity:
					if ( !geCheckClient ) {
						continue;
					}
				}
				/*
				*	Client: Set origin, and see if the position is correct.
				*/
				// Once again, see if we're good.
				if ( geCheckClient && SG_Push_CorrectOrigin( geCheck, geCheckClient ) ) {
					continue;
				}

			// Too bad, didn't move, we're blocked.
			} else {
				/**
				*	Non-Rider Route:
				*
				*	If we're not riding it, then we're likely being pushed by it (given we're within its boundaries).
				*	Perform a backwards trace to see where we intersected with the pusher.
				**/
				// Unlink non-rider entity.
				geCheck->UnlinkEntity();

				// Restore pusher entity to original origin.
				gePusher->SetOrigin( originalOrigin );
				// Link pusher back in.
				gePusher->LinkEntity();

				// Clip the entity to the pusher.
				/*
				*	GameEntity: 
				*/
				// Calculate trace origins.
				const vec3_t geTraceStart	= SG_Push_GetEntityOrigin( geCheck );
				const vec3_t geTraceEnd		= geTraceStart - deltaMove;

				// Perform trace.
				SGTraceResult geClipTrace;// = SG_Clip( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
				if ( geCheck->GetSolid() == Solid::Sphere ) {
					geClipTrace = SG_ClipSphere( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
				} else {
					geClipTrace = SG_Clip( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
				}
				/*
				*	Client:
				*/
				vec3_t clTraceStart	= vec3_zero();
				vec3_t clTraceEnd	= vec3_zero();
				SGTraceResult clClipTrace;
				if ( geCheckClient ) {
					// Calculate trace origins.
					clTraceStart	= SG_Push_GetEntityOrigin( geCheck, geCheckClient );
					clTraceEnd		= clTraceStart - deltaMove;

					// Perform trace.
					if ( geCheck->GetSolid() == Solid::Sphere ) {
						clClipTrace = SG_ClipSphere( clTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), clTraceEnd, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
					} else {
						clClipTrace = SG_Clip( clTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), clTraceEnd, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
					}
				}
								
				// Move pusher back to destination.
				gePusher->SetOrigin( destinationOrigin );
				// Link pusher back in.
				gePusher->LinkEntity();

				// See if we collided with it.
				/*
				*	GameEntity: 
				*/
				if ( geClipTrace.fraction >= 1.0 && !geCheckClient ) {
					#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
					SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): FALSE positive clip to Pusher(#{}).\n", geCheck->GetNumber(), gePusher->GetNumber() ) );
					#endif
					continue;
				}
				/*
				*	Client:
				*/
				if ( geCheckClient && clClipTrace.fraction >= 1.0 ) {
					#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
					SG_Print( PrintType::DeveloperWarning, fmt::format( "Client(#{}): FALSE positive clip to Pusher(#{}).\n", geCheck->GetNumber(), gePusher->GetNumber()));
					#endif
					continue;
				}

				// See if the entity we hit actually IS the pusher.
				/*
				*	GameEntity: 
				*/
				//if ( !geCheckClient ) {
					if ( SG_Push_IsSameEntity( geClipTrace.gameEntity, gePusher ) ) {
						// Since we hit the pusher, now perform a trace against the rest of the 'world'.
						const float geRemainingDistance = 1.0f - geClipTrace.fraction;

						// Calculate origins for trace.
						const vec3_t geOrigin		= SG_Push_GetEntityOrigin( geCheck );
						const vec3_t geNewOrigin	= vec3_fmaf( geOrigin, geRemainingDistance, deltaMove * vec3_fabsf( geClipTrace.plane.normal ) );

						// Perform trace.
						const SGTraceResult geTranslateTrace = SG_Pusher_TraceEntity( geOrigin, geCheck->GetMins(), geCheck->GetMaxs(), geNewOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere, gePusher, SG_SolidMaskForGameEntity( geCheck ), geCheck->GetSolid() );
						SG_Push_SetEntityOrigin( geCheck, geTranslateTrace.endPosition );
					
						// Make sure to test for it however unlikely it may be for it to be incorrect.
						if ( SG_Push_CorrectOrigin( geCheck ) && !geCheckClient ) {
							#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
							SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Pushed GameEntity(#{}) into CORRECT position.\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
							#endif
							continue;
						}
						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Pushed GameEntity(#{}) into BAD position.\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
						#endif
					} else {
						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Did NOT CLIP GameEntity(#{}).\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
						#endif
					}
				//}
				/*
				*	Client: 
				*/
				if ( geCheckClient ) {
					if ( SG_Push_IsSameEntity( clClipTrace.gameEntity, gePusher ) ) {
						// Since we hit the pusher, now perform a trace against the rest of the 'world'.
						const float clRemainingDistance = 1.0f - clClipTrace.fraction;

						// Calculate origins for trace.
						const vec3_t clOrigin		= SG_Push_GetEntityOrigin( geCheck, geCheckClient );
						const vec3_t clNewOrigin	= vec3_fmaf( clOrigin, clRemainingDistance, deltaMove * vec3_fabsf( clClipTrace.plane.normal ) );

						// Need to unlink geCheck first.
						geCheck->UnlinkEntity();
						// Perform trace.
						const SGTraceResult clTranslateTrace = SG_Pusher_TraceEntity( clOrigin, geCheck->GetMins(), geCheck->GetMaxs(), clNewOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere,gePusher, SG_SolidMaskForGameEntity( geCheck ), geCheck->GetSolid()  );
						SG_Push_SetEntityOrigin( geCheck, clTranslateTrace.endPosition, geCheckClient );

						// Make sure to test for it however unlikely it may be for it to be incorrect.
						if ( SG_Push_CorrectOrigin( geCheck, geCheckClient ) ) {
							#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
							SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Pushed Client(#{}) into CORRECT position.\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
							#endif
							// Relink.
							//geCheck->LinkEntity();
							continue;
						}
						// Relink.
						//geCheck->LinkEntity();

						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Pushed Client(#{}) into BAD position.\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
						#endif
					} else {
						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Did NOT CLIP Client(#{}).\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
						#endif
					}
				} 

			} // if ( isRiderEntity ) { 
		} // if ( gePusher->GetMoveType() == MoveType::Push || isRiderEntity ) {
		
		// Dispatch blocked callback
		SGEntityHandle ehCheck = geCheck;
		gePusher->DispatchBlockedCallback( geCheck );
		// Continue to nexti teration if, and only IF, the entity has been 'destroyed'/'killed'.
		GameEntity *validatedGeCheck = SGGameWorld::ValidateEntity( geCheck );
		if ( !validatedGeCheck || !validatedGeCheck->IsInUse() || validatedGeCheck->GetDeadFlag() == DeadFlags::Dead || ( validatedGeCheck->GetServerFlags() & EntityServerFlags::DeadMonster ) ) {
			continue;
		}

		/**
		*	#5:	Move back any entities we already moved. We'll go backwards, so if the same entity was pushed
		*		twice, it goes back to the original position.
		**/
		while ( lastPushedEntityState > pushedEntities) {
			SG_PopPushedEntityState( true, nullptr, true );
		}

        return geCheck;
    }
	
	// Set the pusher to its destinated origin once and for all.
	gePusher->SetOrigin( destinationOrigin );

	// Call TouchTriggers on all our moved entities.
    for ( PushedEntityState *p = lastPushedEntityState - 1; p >= pushedEntities; p--) {
        // Fetch pusher's base entity.
        GameEntity* gePushedState= SG_GetGameEntityFromPushedState(p);

        // Ensure we are dealing with a valid pusher entity.
	    if ( !gePushedState ) {
			SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
            continue;
	    }

		// Enjoy,
		if ( gePushedState->IsInUse() ) {
			// Link entity.
			gePushedState->LinkEntity(); 
			// Check for ground
			//if ( !SG_Push_IsSameEntity( gePushedState, gePusher ) ) {
				if ( gePushedState->GetClient() ) {
					SG_CheckGround( gePushedState );
				} else {
					SG_Monster_CheckGround( gePushedState );
					//SG_CheckGround( gePushedState );
				}
			//}
			// SG_CheckSolids

			// Touch triggers.
		    SG_TouchTriggers( gePushedState );
		}
    }

    return nullptr;
}



/***
*
*
*	PUSHER ENTITY "ROTATION" MECHANICS.
*
*
***/
const SGTraceResult SG_Push_RotateAndTrace( const vec3_t &angles, GameEntity *gePusher, GameEntity *geCheck, gclient_s *geCheckClient = nullptr ) {
	// Place the pusher into position.
	gePusher->SetAngles( angles );
	gePusher->LinkEntity();

	// Get the geCheck(-client optional) origin.
	const vec3_t geOrigin = SG_Push_GetEntityOrigin( geCheck );
	const vec3_t geMins = geCheck->GetMins();
	const vec3_t geMaxs = geCheck->GetMaxs();

	// Perform and return clipping trace results.
	if ( geCheck->GetSolid() == Solid::Sphere ) {
		return SG_ClipSphere( geOrigin, geMins, geMaxs, geOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
	} else {
		return SG_Clip( geOrigin, geMins, geMaxs, geOrigin, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
	}
}

static const vec3_t SG_Push_SlerpAnglesByQuaternion( const vec3_t &anglesA, const vec3_t &anglesB, const float fraction ) {
	// Calculate rotation angle A quaternion.
	glm::quat aRotation = glm_quat_from_ph_euler( anglesA );
	// Calculate rotation angle B quaternion.
	glm::quat bRotation = glm_quat_from_ph_euler( anglesB );

	// Slerp
	glm::quat slerpRotation = glm::slerp( aRotation, bRotation, fraction );

	// Extract.
	float slerpAnglesYaw = glm::yaw( slerpRotation );
	float slerpAnglesPitch = glm::pitch( slerpRotation );
	float slerpAnglesRoll = glm::roll( slerpRotation );
	
	// Return.
	return { slerpAnglesPitch, slerpAnglesYaw, slerpAnglesRoll };
}

constexpr float TIME_OF_IMPACT_MIN_FRACTION = 0.015625f;//0.03125f;
const bool SG_Push_CalculateRotationTimeOfImpact( const vec3_t &originalAngles, const vec3_t &destinationAngles, const float left, const float right, GameEntity *gePusher, GameEntity *geCheck, gclient_s *geCheckClient = nullptr ) {
	// Left trace.
	const vec3_t anglesLeft = vec3_mix( originalAngles, destinationAngles, left );
	const SGTraceResult trLeft = SG_Push_RotateAndTrace( anglesLeft, gePusher, geCheck, geCheckClient );

	// Half trace.
	const float half = Mixf(left, right, 0.5f);
	const vec3_t anglesHalf = vec3_mix( originalAngles, destinationAngles, half );
	const SGTraceResult trHalf = SG_Push_RotateAndTrace( anglesHalf, gePusher, geCheck, geCheckClient );

	// See if we can exit already, or proceed further to the left.
	if ( trHalf.fraction == 1.f && trLeft.fraction == 1.f ) {
		return 0.f;
	}
	if ( ( trLeft.fraction == 1.f ) && ( trHalf.fraction < 1.f ) ) {
		if ( ( half - left < TIME_OF_IMPACT_MIN_FRACTION ) ) { //} && (!trHalf.allSolid /*|| trHalf.startSolid */) ) {
			return left;
		}

		return SG_Push_CalculateRotationTimeOfImpact( originalAngles, destinationAngles, left, half, gePusher, geCheck, geCheckClient );
	}


	// Right Trace.
	const vec3_t anglesRight = vec3_mix( originalAngles, destinationAngles, right );
	const SGTraceResult trRight = SG_Push_RotateAndTrace( anglesRight, gePusher, geCheck, geCheckClient );

	// See if we can exit already, or proceed further to the left.
	if ( trHalf.fraction == 1.f && trRight.fraction == 1.f ) {
		return 0.f;
	}
	if ( ( trHalf.fraction == 1.f ) && ( trRight.fraction < 1.f ) ) {
		if ( ( half - left < TIME_OF_IMPACT_MIN_FRACTION ) ) { //} && (!trRight.allSolid /*|| trRight.startSolid*/) ) {
			return half;
		}

		return SG_Push_CalculateRotationTimeOfImpact( originalAngles, destinationAngles, half, right, gePusher, geCheck, geCheckClient );
	}

	// Both positions are occupied by mover, shouldn't happen, exception is the first iteration.
	return 0.f;
}


/**
*	@brief	Rotates the entity appropriately.
**/
void SG_Push_RotateEntity( float yaw, GameEntity *gePusher, GameEntity *geCheck, gclient_s *geCheckClient = nullptr ) {
	// Get ground entity and make sure it is validated properly.
	GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );

	// Inspect whether we are riding the pusher, or not.
	if ( SG_Push_IsSameEntity( gePusher, geCheckGroundEntity ) ) {
		if ( geCheck->GetClient() ) {
			geCheck->GetClient()->playerState.pmove.deltaAngles.y = AngleMod ( geCheck->GetClient()->playerState.pmove.deltaAngles.y + yaw );
		} else {

			const vec3_t geCheckAngles = geCheck->GetAngles();
			geCheck->SetAngles({
				geCheckAngles.x,
				geCheckAngles.y + yaw,
				geCheckAngles.z
			});
		}
	}
}

/**
*	@brief	Calculates the rotation for the point, around the origin.
*	@return	The new point after being rotated.
**/
const vec3_t SG_Push_RotatePointAroundOrigin( const vec3_t &point, const vec3_t &origin, const vec3_t &eulerAngles ) {
	// GLM V4
	glm::vec4 originV4 = glm::vec4( origin.x, origin.y, origin.z, 1.0 );
	glm::vec4 oldPointV4 = glm::vec4( point.x, point.y, point.z, 1.0 );
	
	// Orientation Quaternion.
	const glm::quat quatOrientation( ph_mat_identity() );
	// Calculate our actual 'to rotate' quaternion based on Euler, oh yikes.
	const glm::quat quatEuler = glm::normalize( glm_quat_from_ph_euler(  eulerAngles  ) );

	// Rotate along orientation and normalize resulting Quat.
	const glm::quat nQuatRotation = glm::normalize( quatOrientation * quatEuler );

	// Calculate the new point, rotated along the origin.
	//glm::vec4 rotatedPoint = originV4 + ( glm::conjugate( nQuatRotation ) * ( oldPointV4 - originV4 ) * nQuatRotation );// * glm::conjugate( nQuatOrientation );//  ( oldPointV4 - originV4 ) );

	glm::mat4 matTranslatePoint = glm::translate(ph_mat_identity(), glm::vec3( origin.x, origin.y, origin.z ) );
	glm::mat4 matInvTranslatePoint = glm::inverse( matTranslatePoint );
	glm::mat4 matRotation = matTranslatePoint * glm::mat4_cast( nQuatRotation ) * matInvTranslatePoint;
	glm::vec4 rotatedPoint = matRotation * oldPointV4;

	// Return results.
	return glmvec3_to_phvec( glm::vec3( rotatedPoint.x / rotatedPoint.w, rotatedPoint.y / rotatedPoint.w, rotatedPoint.z / rotatedPoint.w ) );
}

/**
*	@brief	Will push the mover into its deltaMove + deltaAngularMove combined offset direction.
*	@return	A pointer to the blocking entity, nullptr if non blocked and the move succeeded.
**/
GameEntity *SG_Pusher_Rotate( SGEntityHandle &entityHandle, const vec3_t &partOrigin, const vec3_t &deltaMove, const vec3_t &angularDeltaMove ) {
	/**
	*	
	**/
	SGGameWorld *gameWorld = GetGameWorld();

	/**
	*	Validate pusher entity and push its state.
	**/
	// Assign handle to base entity.
    GameEntity* gePusher = SGGameWorld::ValidateEntity(entityHandle);

    // Ensure it is a valid entity.
    if (!gePusher) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
	    return nullptr;
    }
	
	// Push its state.
	SG_PushEntityState( gePusher );

	/**
	*	Calculate bounds of the entire move.
	**/
	// Total bounds.
	bbox3_t totalBounds = gePusher->GetPODEntity()->absoluteBounds;
	// Unlink pusher.
	gePusher->UnlinkEntity();

	// Acquire the original angles we start from.
	const vec3_t originalAngles = gePusher->GetAngles();
	// Calculate the final destination origin..
	const vec3_t destinationAngles = originalAngles + angularDeltaMove;

	// Set to destination origin, and link entity in for calculating absolute bounds.
	gePusher->SetAngles( destinationAngles );
	// Link pusher back in.
	gePusher->LinkEntity();

	// Union the previous bounds with the destination absolute bounds to get us the full move bounds.
	totalBounds = bbox3_union( totalBounds, gePusher->GetPODEntity()->absoluteBounds );
	totalBounds = bbox3_expand( totalBounds, 1.f );

	/**
	*	Get all entities residing in the move's bounds box.
	**/
	// Get a range of all pushable entities in our world. (A valid GameEntity and Inuse.)
	auto gePushables = SG_BoxEntities( totalBounds.mins, totalBounds.maxs, MAX_POD_ENTITIES, AreaEntities::Solid );

	/**
	*	Iterate all resulting entities of BoxEntities and determine whether they are riding the pusher,
	*	or being pushed by it. If they are we try and move them into their new spot, when any of them
	*	gets blocked we revert all entity moves and return the blocking game entity.
	**/
	for ( auto geCheck : gePushables ) {
		/**
		*	Perform several tests on whether we truly must include this entity in our move.
		**/
		// MoveType:
		if ( !SG_Push_EntityValidMoveType( geCheck ) ) {
			continue;
		}
		// Solid:
		if ( !SG_Push_EntityValidSolid( geCheck ) ) {
			continue;
		}
		// Linked or not.
		if ( !SG_Push_EntityIsLinked( geCheck ) ) {
			continue;
		}


		/**
		*	See if we can skip this entity by checking if it is in a 'good' valid position, AND not riding us.
		**/
		// We need to work the client's pmove as well in case it has one.
		gclient_s *geCheckClient = geCheck->GetClient();

		// Get ground entity and make sure it is validated properly.
		GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );
		// Inspect whether we are riding the pusher, or not.
		const bool isRiderEntity = SG_Push_IsSameEntity( gePusher, geCheckGroundEntity );

		// Ensure we're in a good position.
		/*
		*	GameEntity:
		*/
		//if ( !geCheckClient ) {
			if ( SG_Push_IsValidOrigin( geCheck ) && !isRiderEntity ) {
				continue;
			}
		//}
		/*
		*	Client:
		*/
		//if ( geCheckClient ) { 
		//	if ( !isRiderEntity && SG_Push_IsValidOrigin( geCheck, geCheckClient ) ) {
		//		continue;
		//	}
		//}


		/**
		*	If we are the pusher, or if the entity is riding the pusher, move the entity
		**/
		if ( gePusher->GetMoveType() == MoveType::Push || isRiderEntity ) {
			// Push the entity onto our stack.
			SG_PushEntityState( geCheck );

			/**
			*	Inspect whether we are going to collide with the pusher by placing us into
			*	the pusher's final angles, and then see if we intersected with it or not.
			**/
			gePusher->SetAngles( destinationAngles );
			gePusher->LinkEntity();


			/**
			*
			*
			*	GameEntity: 
			*
			*
			**/
			//if ( !geCheckClient ) {
				// Calculate trace origins.
				const vec3_t geTraceStart	= SG_Push_GetEntityOrigin( geCheck );
				const vec3_t geTraceEnd		= geTraceStart;//SG_Push_RotatePointAroundOrigin(geTraceStart, gePusher->GetOrigin(), angularDeltaMove);;//geTraceStart;

				// Perform clip entity trace to see how much of the move can be performed.
				SGTraceResult geClipTrace;
				if ( geCheck->GetSolid() == Solid::Sphere ) {
					geClipTrace = SG_ClipSphere( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
				} else {
					geClipTrace = SG_Clip( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
				}
				//SGTraceResult geClipTrace = SG_Trace( geTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), geTraceEnd, geCheck, SG_SolidMaskForGameEntity( geCheck ) );

				// See if we intersected with it, and if so, determine by how much for our remaining move to perform.
				float geRemainingMove = 1.0f;
				if ( geClipTrace.fraction < 1.f ) {
					// Calculate the time of impact.
					geRemainingMove = 1.0f - geClipTrace.fraction;
					//geRemainingMove = 1.0f - SG_Push_CalculateRotationTimeOfImpact( originalAngles, destinationAngles, 0.f, 1.f, gePusher, geCheck, nullptr );

					// Now put is back into the destined angles position.
					gePusher->SetAngles( destinationAngles );
					gePusher->LinkEntity();

					//#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
					//SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): FALSE positive clip to Pusher(#{}).\n", geCheck->GetNumber(), gePusher->GetNumber() ) );
					//#endif
					//continue;
				}
				
				// Calculate the rotation around the origin, moving until we got one that does not clip.
				const vec3_t geOriginalOrigin = SG_Push_GetEntityOrigin( geCheck );

				// Try twitching several into negative as well as positive directions, in an attempt to try and find a fitting spot.
				const int32_t geTotalMovements = 55;
				int32_t i = 0;
				vec3_t geAngularMove = angularDeltaMove;
				//vec3_t geRotateOrigin = geOriginalOrigin;
				for ( i = 0; i < geTotalMovements; i++ ) {
					// Calculate offset exchanging from positive to negated.
					int32_t offset = (int32_t)ceilf( i * 0.5f );
					if ( i & 1 ) {
						offset = -offset;
					}

					// Calculate fraction to move for.
					const float geAngularMoveScale = ( geClipTrace.fraction < 1.f ? geRemainingMove + (geRemainingMove * offset * 0.5f) : 1.f );
					// Scale our move by fraction.
					geAngularMove = vec3_scale( angularDeltaMove, geAngularMoveScale );
					//const float MINROTANGLEYO = 5.625; // 45, 22.5, 11.25, 5.625
					//if ( fabs( geAngularMove.y ) < MINROTANGLEYO ) {
					//	if ( geAngularMove.y > 0 ) {
					//		geAngularMove.y = MINROTANGLEYO;
					//	} else {
					//		geAngularMove.y = -MINROTANGLEYO;
					//	}
					//}
					// Calculate exact origin and move into it.
					const vec3_t geRotateOrigin = SG_Push_RotatePointAroundOrigin( geOriginalOrigin, gePusher->GetOrigin(), geAngularMove );
					SG_Push_SetEntityOrigin( geCheck, geRotateOrigin );

					// Perform test clipping trace.
					SGTraceResult trClipper;// = SG_Clip( geOriginalOrigin, geCheck->GetMins(), geCheck->GetMaxs(), geRotateOrigin, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
					if ( geCheck->GetSolid() == Solid::Sphere ) {
						trClipper = SG_ClipSphere( geOriginalOrigin, geCheck->GetMins(), geCheck->GetMaxs(), geRotateOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
					} else {
						trClipper = SG_Clip( geOriginalOrigin, geCheck->GetMins(), geCheck->GetMaxs(), geRotateOrigin, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
					}
					if ( !trClipper.podEntity || trClipper.fraction >= 1.0f ) {//trClipper.fraction == 1.0f ) {
						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): rotated(totalMoves: {}) by Pusher(#{}) to GOOD origin fraction({}), allSolid({}), startSolid({}).\n", geCheck->GetNumber(), i, gePusher->GetNumber(), trClipper.fraction, ( trClipper.allSolid ? "true" : "false" ), ( trClipper.startSolid ? "true" : "false" )));
						#endif
						break;
					} else {
						#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
						SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): tried to rotate(totalMoves: {}) by Pusher(#{}) to BAD origin fraction({}), allSolid({}), startSolid({}).\n", geCheck->GetNumber(), i, gePusher->GetNumber(), trClipper.fraction, ( trClipper.allSolid ? "true" : "false" ), ( trClipper.startSolid ? "true" : "false" )));
						#endif
					}
				}
				if (i == geTotalMovements) {
					#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
					SG_Print( PrintType::DeveloperWarning, fmt::format( "GameEntity(#{}): rotated(totalMoves: {}) by Pusher(#{}) but COULD NOT FIT({}).\n", geCheck->GetNumber(), i, gePusher->GetNumber(), geRemainingMove ) );
					#endif
				}

				// Clip the rest of the movement.
				// Perform world trace from original, to the TOI calculated move.
				const vec3_t geOrigin = SG_Push_GetEntityOrigin( geCheck );
				const SGTraceResult geFinalTrace = SG_Pusher_TraceEntity( geOriginalOrigin, geCheck->GetMins(), geCheck->GetMaxs(), geOrigin, geCheck->GetPODEntity()->boundsAbsoluteSphere, geCheck, SG_SolidMaskForGameEntity( geCheck ), geCheck->GetSolid() );
				// Set final new origin.
				SG_Push_SetEntityOrigin( geCheck, geFinalTrace.endPosition );
					// DEBUG: For the love of god stop fucking thinking we're blocked when hit by a corner segment.
				//SG_Print(PrintType::Developer, fmt::format( "angularDeltaMove({},{},{}), geAngularMove({},{},{}), clipTraceFraction({}), remainingMove({})\n",
				//		 angularDeltaMove.x, angularDeltaMove.y, angularDeltaMove.z,
				//		 geAngularMove.x, geAngularMove.y, geAngularMove.z, 
				//		 geClipTrace.fraction,
				//		 geRemainingMove));
				// END OF DEBUG - PEACE OUT

				// Perform a test for whether we have been separated or not.
				vec3_t correctAngles = angularDeltaMove;
				if ( SG_Push_CorrectOrigin( geCheck, nullptr, &correctAngles ) ) {
					SG_Push_RotateEntity( geAngularMove[ vec3_t::Yaw ], gePusher, geCheck );
					break;
				}

				/**
				*	Prevent entities that might've been pushed off by the world or other entities from actually doing so,
				*	by reverting to original position and retry again.
				**/
				//GameEntity *geCheckClientGroundEntity = ;
				//if ( SG_Push_IsSameEntity( geClipTrace.gameEntity, gePusher ) ) {
				if ( SG_Push_IsSameEntity( gePusher, SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() ) ) ) {
					if ( !geCheckClient) {
						SG_PopPushedEntityState( true, nullptr, false );
					} else {
						SG_PopPushedEntityState( true, nullptr, true );
						//SG_PopPushedEntityState( true, geCheckClient, false );
					}
						
						//SG_PopPushedEntityState( true, nullptr, true );

					// Correct origin if need be, and continue.
					if ( SG_Push_CorrectOrigin( geCheck ) ) {
						//if ( !geCheckClient) {
							continue;
						//}
					}
				}

				// DEBUG: For the love of god stop fucking thinking we're blocked when hit by a corner segment.
				//SG_Print(PrintType::Developer, fmt::format( "BLOCKED: angularDeltaMove({},{},{}), geAngularMove({},{},{}), clipTraceFraction({}), remainingMove({})\n",
				//		 angularDeltaMove.x, angularDeltaMove.y, angularDeltaMove.z,
				//		 geAngularMove.x, geAngularMove.y, geAngularMove.z, 
				//		 geClipTrace.fraction,
				//		 geRemainingMove));
				// END OF DEBUG - PEACE OUT

				//// Perform a test for whether we have been separated or not.
				//if ( SG_Push_CorrectOrigin( geCheck ) ) {
				//	SG_Push_RotateEntity( angularDeltaMove[ vec3_t::Yaw ], gePusher, geCheck );
				//	break;
				//}
				///**
				//*	Prevent entities that might've been pushed off by the world or other entities from actually doing so,
				//*	by reverting to original position and retry again.
				//**/
				//GameEntity *geCheckClientGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );
				//if ( SG_Push_IsSameEntity( gePusher, geCheckClientGroundEntity ) ) {
				////if ( SG_Push_IsSameEntity( geClipTrace.gameEntity, gePusher ) ) {
				//	// Pop Entity State.
				//	SG_PopPushedEntityState( true, nullptr, false );

				//	// Correct origin if need be, and continue.
				//	if ( SG_Push_CorrectOrigin( geCheck ) ) {
				//		continue;
				//	}
				//}


			/**
			*
			*
			*	Client: 
			*
			*
			**/
			//if ( geCheckClient ) {
			//	// Calculate trace origins.
			//	vec3_t clTraceStart	= SG_Push_GetEntityOrigin( geCheck, geCheckClient );
			//	vec3_t clTraceEnd	= clTraceStart;

			//	// Perform trace.
			//	SGTraceResult clClipTrace = SG_Clip( clTraceStart, geCheck->GetMins(), geCheck->GetMaxs(), clTraceEnd, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );

			//	//
			//	float clRemainingMove = 1.0f;
			//	if ( clClipTrace.fraction <= 1.f ) {
			//		// Calculate the time of impact.
			//		clRemainingMove = 1.0f - clClipTrace.fraction;
			//		//clRemainingMove = 1.0f - SG_Push_CalculateRotationTimeOfImpact( originalAngles, destinationAngles, 0.f, 1.f, gePusher, geCheck, geCheckClient );

			//		// Now put is back into the destined angles position.
			//		gePusher->SetAngles( destinationAngles );
			//		gePusher->LinkEntity();
			//	}

			//	// Calculate the rotation around the origin, moving until we got one that does not clip.
			//	const vec3_t clOriginalOrigin = SG_Push_GetEntityOrigin( geCheck, geCheckClient );
			//	
			//	// Try twitching several into negative as well as positive directions, in an attempt to try and find a fitting spot.
			//	const int32_t clTotalMovements = 55;
			//	int32_t i = 0;
			//	vec3_t clAngularMove = angularDeltaMove;
			//	for ( i = 0; i < clTotalMovements; i++ ) {
			//		// Calculate offset exchanging from positive to negated.
			//		int32_t offset = (int32_t)ceilf( i * 0.5f );
			//		if ( i & 1 ) {
			//			offset = -offset;
			//		}

			//		// Calculate fraction to move for.
			//		const float clAngularMoveScale = (clClipTrace.fraction < 1.f ? clRemainingMove + (clRemainingMove * offset * 0.5f) : 1.f );
			//		// Scale our move by fraction.
			//		clAngularMove = vec3_scale( angularDeltaMove, clAngularMoveScale );
			//	
			//		// Calculate exact origin and move into it.
			//		const vec3_t testOrigin = SG_Push_RotatePointAroundOrigin( clOriginalOrigin, gePusher->GetOrigin(), clAngularMove );
			//		SG_Push_SetEntityOrigin( geCheck, testOrigin, geCheckClient );

			//		// Perform test clipping trace.
			//		SGTraceResult trClipper = SG_Clip( testOrigin, geCheck->GetMins(), geCheck->GetMaxs(), testOrigin, geCheck, gePusher, SG_SolidMaskForGameEntity( geCheck ) );
			//		if ( trClipper.startSolid == false && trClipper.allSolid == false && trClipper.fraction == 1.0f ) {
			//			#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
			//			SG_Print( PrintType::DeveloperWarning, fmt::format( "Client(#{}): rotated(totalMoves: {}) by Pusher(#{}) to GOOD origin.\n", geCheck->GetNumber(), i, gePusher->GetNumber() ) );
			//			#endif
			//			break;				
			//		}		
			//	}
			//	if ( i == clTotalMovements ) {
			//		#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
			//		SG_Print( PrintType::DeveloperWarning, fmt::format( "Client(#{}): rotated(totalMoves: {}) by Pusher(#{}) but COULD NOT FIT({}).\n", geCheck->GetNumber(), i, gePusher->GetNumber(), clRemainingMove ) );
			//		#endif
			//	}
			//	// Clip the rest of the movement.
			//	const vec3_t clOrigin = SG_Push_GetEntityOrigin( geCheck, geCheckClient );
			//	const SGTraceResult clFinalTrace = SG_Trace( clOriginalOrigin, geCheck->GetMins(), geCheck->GetMaxs(), clOrigin, geCheck, SG_SolidMaskForGameEntity( geCheck ) );
			//	// Set final new origin.
			//	SG_Push_SetEntityOrigin( geCheck, clFinalTrace.endPosition, geCheckClient );

			//	// Perform a test for whether we have been separated or not.
			//	if ( SG_Push_CorrectOrigin( geCheck, geCheckClient ) ) {
			//		SG_Push_RotateEntity( clAngularMove[ vec3_t::Yaw ], gePusher, geCheck, geCheckClient );
			//		break;
			//	}

			//	/**
			//	*	Prevent entities that might've been pushed off by the world or other entities from actually doing so,
			//	*	by reverting to original position and retry again.
			//	**/
			//	GameEntity *geCheckClientGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );
			//	SG_CheckGround( geCheck );
			//	if ( SG_Push_IsSameEntity( gePusher, geCheckClientGroundEntity ) ) {
			//	//if ( SG_Push_IsSameEntity( clClipTrace.gameEntity, gePusher ) ) {
			//		// Pop Entity State.
			//		SG_PopPushedEntityState( true, geCheckClient, false ); // Has ALREADY been popped!

			//		// Correct origin if need be, and continue.
			//		if ( SG_Push_CorrectOrigin( geCheck, geCheckClient ) ) {
			//			continue;
			//		}
			//	}
			//}

		}

		/**
		*	Try and destroy the object which is currently blocking us.
		**/
		// Dispatch blocked callback
		SGEntityHandle ehCheck = geCheck;

		const vec3_t gePusherAngles = gePusher->GetAngles();
		const vec3_t geCheckAngles = geCheck->GetAngles();
		SG_Print( PrintType::Developer, fmt::format( "Pusher(#{}, angles: {},{},{}) blocked GameEntity(#{}, angles: {},{},{})\n",
			gePusher->GetNumber(),
			gePusherAngles.x,
			gePusherAngles.y,
			gePusherAngles.z,
			geCheck->GetNumber(),
			geCheckAngles.x,
			geCheckAngles.y,
			geCheckAngles.z
		));
		gePusher->DispatchBlockedCallback( geCheck );
		// Continue to nexti teration if, and only IF, the entity has been 'destroyed'/'killed'.
		GameEntity *validatedGeCheck = SGGameWorld::ValidateEntity( geCheck );
		if ( !validatedGeCheck || !validatedGeCheck->IsInUse() || validatedGeCheck->GetDeadFlag() == DeadFlags::Dead || ( validatedGeCheck->GetServerFlags() & EntityServerFlags::DeadMonster ) ) {
			continue;
		}
		
		#ifdef PRINT_DEBUG_ROTATOR_CLIPPING
		SG_Print( PrintType::DeveloperWarning, fmt::format( "Pusher(#{}): Blocked by GameEntity(#{}).\n", gePusher->GetNumber(), geCheck->GetNumber() ) );
		#endif

		/**
		*	#5:	Move back any entities we already moved. We'll go backwards, so if the same entity was pushed
		*		twice, it goes back to the original position.
		**/
		while ( lastPushedEntityState > pushedEntities) {
			SG_PopPushedEntityState( true, nullptr, true );
			//SG_PopPushedEntityState( );
		}

        return geCheck;
	}

	// Call TouchTriggers on all our moved entities.
    for ( PushedEntityState *p = lastPushedEntityState - 1; p >= pushedEntities; p--) {
        // Fetch pusher's base entity.
        GameEntity* gePushedState= SG_GetGameEntityFromPushedState(p);

        // Ensure we are dealing with a valid pusher entity.
	    if ( !gePushedState ) {
			SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
            continue;
	    }

		// Enjoy,
		if ( gePushedState->IsInUse() ) {
			// Link entity.
			gePushedState->LinkEntity(); 
			// Check for ground
			//if ( !SG_Push_IsSameEntity( gePushedState, gePusher ) ) {
				if ( gePushedState->GetClient() ) {
					SG_CheckGround( gePushedState );
				} else {
					SG_Monster_CheckGround( gePushedState );
					//SG_CheckGround( gePushedState );
				}
			//}
			// SG_CheckSolids

			// Touch triggers.
		    SG_TouchTriggers( gePushedState );
		}
    }

	return nullptr;
}

/**
*	@brief	Calculates delta move, deltaAngularMove for both linear and non linear movers. In the
*			case of linear it also calculates the current actual pusherOrigin.
**/
void SG_Physics_Pusher_CalculateMove( GameEntity *gePusher, vec3_t &pusherOrigin, vec3_t &deltaMove, vec3_t &angularDeltaMove  ) {
	if ( !gePusher ) {
		return;
	}

	// Get POD Entity.
	PODEntity *podPusher = gePusher->GetPODEntity();

	// Origin, and deltaMove for time based linear path movement entities.
	if ( podPusher->linearMovement.isMoving ) {
		#ifdef SHAREDGAME_CLIENTGAME
		SG_LinearMovement( podPusher, (level.extrapolatedTime ).count(), pusherOrigin );
		SG_LinearMovementDelta( podPusher, ( level.extrapolatedTime - FRAMERATE_MS ).count(), ( level.extrapolatedTime ).count(), deltaMove );
		// Debug.
		//const vec3_t fromMove = part->GetOrigin(); const vec3_t toMove = fromMove + move;
		//SG_Print( PrintType::Developer, fmt::format( "[CLG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n", ent->GetNumber(), level.time.count(), fromMove.x, fromMove.y, fromMove.z, toMove.x, toMove.y, toMove.z, move.x, move.y, move.z ));			#endif
		#endif
		#ifdef SHAREDGAME_SERVERGAME
		SG_LinearMovement( podPusher, (level.time).count(), pusherOrigin );
		SG_LinearMovementDelta( podPusher, (level.time - FRAMERATE_MS).count(), (level.time).count(), deltaMove );
		// Debug.
		//const vec3_t fromMove = part->GetOrigin(); const vec3_t toMove = fromMove + move;
		//SG_Print( PrintType::Developer, fmt::format( "[SVG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n", ent->GetNumber(), level.time.count(), fromMove.x, fromMove.y, fromMove.z, toMove.x, toMove.y, toMove.z, move.x, move.y, move.z ));
		#endif
	// Non linear movement velocity.
	} else {
		deltaMove = vec3_scale( gePusher->GetVelocity(), FRAMETIME_S.count() );
		pusherOrigin = gePusher->GetOrigin() + deltaMove;
	}

	// Angular Velocity move.
	angularDeltaMove = vec3_scale( gePusher->GetAngularVelocity(), FRAMETIME_S.count() );
}

/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle ) {
    // Assign handle to base entity.
    GameEntity* gePusher = SGGameWorld::ValidateEntity(gePusherHandle);

    // Ensure it is a valid entity.
    if ( !gePusher ) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return;
    }

    // Team captains move, and tell others when to move. Hence, team master and slave.
    if ( gePusher->GetFlags() & EntityFlags::TeamSlave ) {
        return;
	}

	// Reset pushed entity state.
	lastPushedEntityState = pushedEntities;

	// If non null, it is set to the entity which resulted in our move being blocked.
	GameEntity *geObstacle = nullptr;

	// Start iterating over our team.
    for ( GameEntity *gePart = gePusher; gePart != nullptr; gePart = gePart->GetTeamChainEntity() ) {

		SGEntityHandle ehPart( gePart );

		// Results of the processed move for the current frame(moment in time).
		vec3_t pusherOrigin	= vec3_zero(); // UNUSED: Until we add client prediction back in.
		vec3_t deltaMove	= vec3_zero();
		vec3_t angularDeltaMove	= vec3_zero();

		// Calculate the actual move to attempt and traverse.
		SG_Physics_Pusher_CalculateMove( gePart, pusherOrigin, deltaMove, angularDeltaMove );
		#ifdef SHAREDGAME_CLIENTGAME
		//SG_Print( PrintType::DeveloperWarning, fmt::format( "SG_Physics_Pusher(#{}): [avel({},{},{})],[vel({},{},{}]\n", gePart->GetNumber(), gePart->GetVelocity().x, gePart->GetVelocity().y, gePart->GetVelocity().z, gePart->GetAngularVelocity().x, gePart->GetAngularVelocity().y, gePart->GetAngularVelocity().z ));
		#endif
		//SG_Print( PrintType::DeveloperWarning, fmt::format( "SG_Physics_Pusher(#{}): [vel({},{},{})],[avel({},{},{}]\n", gePart->GetNumber(), gePart->GetVelocity().x, gePart->GetVelocity().y, gePart->GetVelocity().z, gePart->GetAngularVelocity().x, gePart->GetAngularVelocity().y, gePart->GetAngularVelocity().z ));

		// If we got velocity, perform the actual translate move for all entities first.
        if ( !vec3_equal( gePart->GetVelocity(), vec3_zero() ) ) {
			// Try and translate our pusher into its new origin.
		//	SG_Print( PrintType::DeveloperWarning, fmt::format( "VELOCITY OF THAT THING: {}, {}, {}, {}\n", gePart->GetNumber(), gePart->GetVelocity().x, gePart->GetVelocity().y, gePart->GetVelocity().z ));
			geObstacle = SG_Pusher_Translate( ehPart, pusherOrigin, deltaMove, angularDeltaMove );

			// We hit something, so break out of our loop right here and now.
			if ( geObstacle ) {
				break;
			}
		}

		// And perform angular velocity rotational movement after.
		if ( !vec3_equal( gePart->GetAngularVelocity(), vec3_zero() ) ) {
			// Try and rotate us into the pusher's angular direction.
			geObstacle = SG_Pusher_Rotate( ehPart, pusherOrigin, deltaMove, angularDeltaMove );

			if ( geObstacle ) {
				break;
			}
			
		//	gePusher->SetAngles( gePusher->GetAngles() + angularDeltaMove );
		}
	}

	// If the move succeeded(no obstacle hit) dispatch all 'Think' callbacks.
	if ( !geObstacle ) {
		for ( GameEntity *gePart = gePusher; gePart != nullptr; gePart = gePart->GetTeamChainEntity() ) {
			SG_RunThink( gePart );
		}
	}
}