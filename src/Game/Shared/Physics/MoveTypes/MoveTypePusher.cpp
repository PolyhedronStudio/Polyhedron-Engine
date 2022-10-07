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
*	@brief	Pushes a new Pushed Entity State for the gePusher Game Entity.
**/
const PushedEntityState &&SG_PushEntityState( GameEntity* gePushMove ) {
	const bool isClientEntity = ( gePushMove->GetClient() != nullptr ? true : false );
	const float deltaYaw = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] : gePushMove->GetAngles()[vec3_t::Yaw] );
	#ifdef SHAREDGAME_CLIENTGAME
	//const vec3_t playerMoveOrigin = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	const vec3_t playerMoveOrigin = ( isClientEntity ? cl->predictedState.viewOrigin : vec3_zero() ); //gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	#else
	const vec3_t playerMoveOrigin = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	#endif
	#ifdef SHAREDGAME_CLIENTGAME
	gePushMove->EnableExtrapolation();
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

	return std::move(returnedState);
}
void SG_PopPushedEntityState(GameEntity* gePushMove) {
	lastPushedEntityState--;
	#ifdef SHAREDGAME_CLIENTGAME
	gePushMove->DisableExtrapolation();
	#endif
}

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
*	@brief	Pushes the entity. Does not change the entities velocity at all
**/
SGTraceResult SG_PushEntity( GameEntity *gePushEntity, const vec3_t &pushOffset ) {
	SGTraceResult trace;
    int32_t     mask = 0;

	GameEntity *ent = gePushEntity;

    // Calculate start for push.
    vec3_t start = ent->GetOrigin();

    // Calculate end for push.
    vec3_t end = start + pushOffset;

retry:
    if (ent->GetClipMask())
        mask = ent->GetClipMask();
    else
        mask = BrushContentsMask::Solid;

    trace = SG_Trace(start, ent->GetMins(), ent->GetMaxs(), end, ent, mask);

	if (ent->GetMoveType() == MoveType::Push || !trace.startSolid) {
	//if (!trace.startSolid) {//if (!trace.startSolid) {
		ent->SetOrigin(trace.endPosition);
	}
	//}
    //ent->SetOrigin(trace.endPosition);
    ent->LinkEntity();

    if ( trace.fraction != 1.0f ) {
		// Dispatch impact callbacks.
        SG_Impact(ent, trace);

        // if the pushed entity went away and the pusher is still there
        if ( (!trace.gameEntity || !trace.gameEntity->IsInUse()) && ent->GetMoveType() == MoveType::Push && ent->IsInUse()) {
            // move the pusher back and try again
            ent->SetOrigin( start );
            ent->LinkEntity();
            goto retry;
        }
    }

    if ( ent->IsInUse() ) {
        SG_TouchTriggers( ent );
	}

    return trace;
}

/**
*	@brief	Tests whether the entity position would be trapped in a Solid.
*	@return	(nullptr) in case it is free from being trapped. Worldspawn entity otherwise.
**/
static GameEntity *SG_TestEntityPosition( GameEntity *geTestSubject ) {
	int32_t clipMask = 0;

    if ( geTestSubject->GetClipMask() ) {
	    clipMask = geTestSubject->GetClipMask();
    } else {
        clipMask = BrushContentsMask::Solid;
    }

	const vec3_t testSubjectOrigin = ( geTestSubject->GetClient() ? geTestSubject->GetClient()->playerState.pmove.origin : geTestSubject->GetOrigin() );

    SGTraceResult trace = SG_Trace( testSubjectOrigin, geTestSubject->GetMins(), geTestSubject->GetMaxs(), testSubjectOrigin, geTestSubject, clipMask );

	if (trace.startSolid == false && trace.allSolid == false ) {
		return nullptr;
	}

    //if ( trace.startSolid ) {
		SGGameWorld *gameWorld = GetGameWorld();

	    return (GameEntity*)( gameWorld->GetWorldspawnGameEntity() );
    //}

    //return nullptr;
}
static GameEntity *SG_TestEntityRotation( GameEntity *geTestSubject ) {
	// mins/maxs are the bounds at destination.
	// totalMins / totalMaxs are the bounds for the entire move
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();
	vec3_t totalMins = vec3_zero();
	vec3_t totalMaxs = vec3_zero();
	// Get bounds radius.
	const float radius = RadiusFromBounds( geTestSubject->GetMins(), geTestSubject->GetMaxs() );

	// Calculate bounds at destination.
	const vec3_t testSubjectOrigin = ( geTestSubject->GetClient() ? geTestSubject->GetClient()->playerState.pmove.origin : geTestSubject->GetOrigin() );

	for ( int32_t i = 0; i < 3; i++ ) {
		mins[i] = testSubjectOrigin[i] - radius;
		maxs[i] = testSubjectOrigin[i] + radius;

		//mins[i] = pusherOrigin[i] + move[i] - radius;
		//maxs[i] = pusherOrigin[i] + move[i] + radius;
		//totalMins[i] = mins[i] - move[i];
		//totalMaxs[i] = maxs[i] - move[i];
	}

	int32_t clipMask = 0;
    if ( geTestSubject->GetClipMask() ) {
	    clipMask = geTestSubject->GetClipMask();
    } else {
        clipMask = BrushContentsMask::Solid;
    }

    SGTraceResult trace = SG_Trace( testSubjectOrigin, mins, maxs, testSubjectOrigin, geTestSubject, clipMask);

	if (trace.startSolid == false && trace.allSolid == false  && trace.fraction == 1 ) {
		return nullptr;
	}

    //if ( trace.startSolid ) {
		SGGameWorld *gameWorld = GetGameWorld();

	    return (GameEntity*)( gameWorld->GetWorldspawnGameEntity() );
    //}

    //return nullptr;
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

////////////////////////////////////
void G_CreateRotationMatrix( const vec3_t angles, vec3_t *matrix ) {
	AngleVectors( angles, &matrix[0], &matrix[1], &matrix[2] );
	//VectorInverse(matrix[1]);
	matrix[1] = vec3_negate( matrix[1] );
}

/*
================
G_TransposeMatrix
================
*/
void G_TransposeMatrix( vec3_t *matrix, vec3_t *transpose ) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
G_RotatePoint
================
*/
void G_RotatePoint( vec3_t &point, vec3_t *matrix) {
	vec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = DotProduct(matrix[0], tvec);
	point[1] = DotProduct(matrix[1], tvec);
	point[2] = DotProduct(matrix[2], tvec);
}

/////////////////////////////////////////////////////////////
/**
*	@brief	Tests the 'push entity' (not the mover) for whether it should be pushed, or skipped from
*			being pushed.
*	@return	False if no push should be performed for this entity, or gePushEntity is (nullptr).
*			Conditions are based on its moveType and whether the entity is local or not.
*
*			If it passes all tests, it returns true indicating this entity should be along pushed with the move.
**/
static inline bool SG_Mover_ShouldPushEntity( GameEntity *gePushEntity ) {
	// Make sure it is valid.
	if ( !gePushEntity ) {
		return false;
	}

	// Get ourselves easy access to data we wish to check upon.
    const int32_t	pushEntityMoveType = gePushEntity->GetMoveType();

	// Local entity.
	#ifdef SHAREDGAME_CLIENTGAME
	const bool isLocalEntity = gePushEntity->GetPODEntity()->isLocal;
	#endif

	//#ifdef SHAREDGAME_CLIENTGAME
	//const std::string pushablesStr2 = fmt::format(
	//	"CLG!!: Entity({}), inUse({}), moveType({}, linkCount({}), groundLinkEntityCount({}), origin({},{},{})\n",
	//	geCheck->GetNumber(),
	//	isInUse,
	//	moveType,
	//	geCheck->GetLinkCount(),
	//	geCheck->GetGroundEntityLinkCount(),
	//	geCheck->GetOrigin().x,
	//	geCheck->GetOrigin().y,
	//	geCheck->GetOrigin().z
	//);
	//SG_Print( PrintType::DeveloperWarning, pushablesStr2 );
	//#endif
	//#ifdef SHAREDGAME_SERVERGAME
	//const std::string pushablesStr2 = fmt::format(
	//	"SVG!!: Entity({}), inUse({}), moveType({}, linkCount({}), groundLinkEntityCount({}), origin({},{},{})\n",
	//	geCheck->GetNumber(),
	//	isInUse,
	//	moveType,
	//	geCheck->GetLinkCount(),
	//	geCheck->GetGroundEntityLinkCount(),
	//	geCheck->GetOrigin().x,
	//	geCheck->GetOrigin().y,
	//	geCheck->GetOrigin().z
	//);
	//SG_Print( PrintType::DeveloperWarning, pushablesStr2 );
	//#endif

	// Skip moveTypes that aren't pushed around at all.
	#ifdef SHAREDGAME_CLIENTGAME
    if ( pushEntityMoveType == MoveType::Push   || pushEntityMoveType == MoveType::Stop   ||
			pushEntityMoveType == MoveType::NoClip || pushEntityMoveType == MoveType::Spectator ||
		( isLocalEntity && pushEntityMoveType == MoveType::None ) ) {
        return false;
	}
	#endif
	#ifdef SHAREDGAME_SERVERGAME
    if ( pushEntityMoveType == MoveType::Push   || pushEntityMoveType == MoveType::Stop   ||
			pushEntityMoveType == MoveType::None   || pushEntityMoveType == MoveType::NoClip ||
			pushEntityMoveType == MoveType::Spectator ) {
        return false;
	}
	#endif

	// Entity has to be linked in.
    if ( !gePushEntity->GetPODEntity()->area.prev ) {
        return false;       // not linked in anywhere
	}

	return true;
}

/**
*	@brief	Will push the mover into its deltaMove + deltaAngularMove combined offset direction.
*	@return	false if the move failed (i.e could've been blocked.), true if it succeeds.
**/
const bool SG_MoverPush( SGEntityHandle &entityHandle, const vec3_t &partOrigin, const vec3_t &deltaMove, const vec3_t &angularDeltaMove ) {
    GameEntity* geCheck = nullptr;
    GameEntity* geBlock = nullptr;
    PushedEntityState *p = nullptr;
    vec3_t org, org2;
	vec3_t move2;
	vec3_t forward, right, up;

    // Assign handle to base entity.
    GameEntity* gePusher = SGGameWorld::ValidateEntity(entityHandle);

    // Ensure it is a valid entity.
    if (!gePusher) {
	    SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
	    return false;
    }

	const vec3_t pusherMove = partOrigin - gePusher->GetOrigin();
	const vec3_t move = deltaMove; //pusherMove;

    // We need this for pushing things later
    org = vec3_negate( angularDeltaMove );
    AngleVectors(org, &forward, &right, &up);
	
	// We push the required 'moment in time' state to later on reverse this move if required. (i.e it got blocked)
	SG_PushEntityState( gePusher );

	// Calculate the pusher bounds as well as the entire move's bounds.
	// The first is used to test whether any entities found in the entire move are standing on it or not.
	// The second is used to get all entities 'within its box' to use for testing.
	//
	// mins/maxs are the bounds at destination.
	// totalMins / totalMaxs are the bounds for the entire move
	vec3_t mins = vec3_zero();
	vec3_t maxs = vec3_zero();
	vec3_t totalMins = vec3_zero();
	vec3_t totalMaxs = vec3_zero();

	// Special handling for any rotating pushers.
	if ( vec3_equal( gePusher->GetAngles(), vec3_zero() ) || vec3_equal( gePusher->GetAngularVelocity(), vec3_zero() ) ) {
		// Get bounds radius.
		const float radius = RadiusFromBounds( gePusher->GetMins(), gePusher->GetMaxs() );

		// Calculate bounds at destination.
		const vec3_t pusherOrigin = gePusher->GetOrigin();

		// Adjust move to rotating bounds radius.
		for ( int32_t i = 0; i < 3; i++ ) {
			mins[i] = pusherOrigin[i] + move[i] - radius;
			maxs[i] = pusherOrigin[i] + move[i] + radius;
			totalMins[i] = mins[i] - move[i];
			totalMaxs[i] = maxs[i] - move[i];
		}
	// Handling for non rotating pushers.
	} else {
		// Find the bounding box of the pusher's move.
		mins = gePusher->GetAbsoluteMin() + move;
	    maxs = gePusher->GetAbsoluteMax() + move;

		// Calculate total move mins and maxs.
		totalMins = gePusher->GetAbsoluteMin();
		totalMaxs = gePusher->GetAbsoluteMax();

		// Finish off totalMins and totalMaxs.
		//for ( int32_t i = 0; i < 3; i++ ) {
		//	if ( move[i] > 0 ) {
		//		totalMaxs[i] += move[i];
		//	} else {
		//		totalMins[i] += move[i];
		//	}
		//}
		// Add the move coordinates to the 'total' bounds in order to get the entire move bounds.
		AddPointToBounds( move, totalMins, totalMaxs );
	}

	// Unlink first.
    gePusher->SetOrigin( gePusher->GetOrigin() + deltaMove );
    gePusher->SetAngles( gePusher->GetAngles() + angularDeltaMove );
	gePusher->LinkEntity();

	/**	
	*	See if the position has been taken by other entities, and if so, try and push each other.
	**/
	// Get a range of all pushable entities in our world. (A valid GameEntity and Inuse.)
	SGGameWorld *gameWorld = GetGameWorld();
//	auto gePushables = gameWorld->GetGameEntityRange(0, MAX_POD_ENTITIES) | cef::IsValidPointer | cef::InUse;
	auto gePushables = SG_BoxEntities( totalMins, totalMaxs, MAX_POD_ENTITIES, AreaEntities::Solid );



	#ifdef SHAREDGAME_CLIENTGAME
	const std::string pushablesStr = fmt::format(
		"CLG_BoxEntities(count: {}) {{\n",
		gePushables.size()
	);
	SG_Print( PrintType::DeveloperWarning, pushablesStr );
	#endif
	#ifdef SHAREDGAME_SERVERGAME
	const std::string pushablesStr = fmt::format(
		"SVG_BoxEntities(count: {}) {{\n",
		gePushables.size()
	);
	SG_Print( PrintType::DeveloperWarning, pushablesStr );
	#endif

	// Iterate over the pushable entities.
	for ( auto geCheck : gePushables ) {
		// Figure out whether this entity should be pushed along this move or be skipped instead.
		if ( !SG_Mover_ShouldPushEntity( geCheck ) ) {
			continue;
		}

		// Check whether the entity is standing on our pusher(making him a rider), in which case he will
		// definitely need to be moved.
        // if the entity is standing on the pusher, it will definitely be moved
		GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );

		// (Can't be the same entity in that case, so make sure we check for that.)
		const vec3_t checkAbsoluteMins = geCheck->GetAbsoluteMin();
		const vec3_t checkAbsoluteMaxs = geCheck->GetAbsoluteMax();
		if ( !SG_Push_IsSameEntity( geCheckGroundEntity, gePusher ) ) {
            // see if the ent needs to be tested
            if ( checkAbsoluteMins[0] >= maxs[0]	||
                checkAbsoluteMins[1] >= maxs[1]	||
                checkAbsoluteMins[2] >= maxs[2]	||
                checkAbsoluteMaxs[0] <= mins[0]	||
                checkAbsoluteMaxs[1] <= mins[1]	||
                checkAbsoluteMaxs[2] <= mins[2] ) {
                continue;
			}

            // see if the ent's bbox is inside the pusher's final position
            if ( !SG_TestEntityPosition( geCheck ) ) {
                continue;
			}   
        }

		// See if any solid entities are inside the final position
		if ( (gePusher->GetMoveType() == MoveType::Push) || SG_Push_IsSameEntity( geCheckGroundEntity, gePusher ) ) {
			/**
			*	Store the needed pushed entity information in a pushed entity state.	
			**/
			SG_PushEntityState( geCheck );
			
			/**
			*	In order to start moving our contacted entity, we need to calculate its full final
			*	move based on the optional angular delta move vector.
			**/
			vec3_t matrix[3], transpose[3];
			AnglesToAxis( angularDeltaMove, transpose );//G_CreateRotationMatrix( angularMove, transpose );
			G_TransposeMatrix( transpose, matrix );//G_TransposeMatrix( transpose, matrix );
			if ( geCheck->GetClient() ) {
				// geCheck->GetClient()->playerState.pmove.origin <-- should be the actual current moment in time origin if linear mover.
				org = geCheck->GetClient()->playerState.pmove.origin - gePusher->GetOrigin();
			} else {
				// geCheck->GetOrigin <-- should be the actual current moment in time origin if linear mover.
				org = geCheck->GetOrigin() - gePusher->GetOrigin();
			}
			org2 = org;
			RotatePoint( org2, matrix );//G_RotatePoint( org2, matrix );
			move2 = org2 - org;

			/**
			*	Try moving the contacted entity.
			**/
            geCheck->SetOrigin( geCheck->GetOrigin() + deltaMove );
            if ( geCheck->GetClient() ) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
				#if USE_SMOOTH_DELTA_ANGLES
				geCheck->GetClient()->playerState.pmove.origin += deltaMove;
				geCheck->GetClient()->playerState.pmove.deltaAngles[ vec3_t::Yaw ] += angularDeltaMove[ vec3_t::Yaw ];
				#endif

			} else {
				#if USE_SMOOTH_DELTA_ANGLES
				vec3_t angles = geCheck->GetAngles();
				angles[ vec3_t::Yaw ] += angularDeltaMove[ vec3_t::Yaw ];
				geCheck->SetAngles(angles);
				#endif
			}


			/**
			*	Figure movement due to the pusher's Angular Move.
			**/
            geCheck->SetOrigin( geCheck->GetOrigin() + move2 );

			// Client pmove.
            if ( geCheck->GetClient() ) {
				geCheck->GetClient()->playerState.pmove.origin += move2; //[vec3_t::Yaw] += move2[vec3_t::Yaw];
			}

            // May have pushed them off an edge
			if ( !SG_Push_IsSameEntity( geCheckGroundEntity, gePusher ) ) {
                geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
			}

			// Test whether entity is inside of another, if not, push was okay so link it and move
			// on to the next entity that needs pushing.
            geBlock = SG_TestEntityPosition( geCheck );

			GameEntity *geBlock2 = SG_TestEntityRotation( geCheck );
            if ( !geBlock && !geBlock2 ) {
                // pushed ok
                geCheck->LinkEntity();
                // impact?
                continue;
            } else {
				// Of it is ok to leave in the old position, do it.
				// This is only relevent for riding entities, not pushed
				// FIXME: this doesn't acount for rotation

				// Client origin.
				if ( geCheck->GetClient() ) {
					geCheck->SetOrigin( geCheck->GetOrigin() - move );//geCheck->state.origin -= move;
					geCheck->SetOrigin( geCheck->GetOrigin() - move2 );

					geCheck->GetClient()->playerState.pmove.origin -= move;
					geCheck->GetClient()->playerState.pmove.origin -= move2;

					// Rotate back angles.
					geCheck->GetClient()->playerState.pmove.deltaAngles[ vec3_t::Yaw ] -= angularDeltaMove[ vec3_t::Yaw ];
					//geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] -= move2[vec3_t::Yaw];
				} else {
					// Get current angles to rotate back from.
					vec3_t checkAngles = geCheck->GetAngles();

					// Rotate back origin.
					geCheck->SetOrigin( geCheck->GetOrigin() - move );
					geCheck->SetOrigin( geCheck->GetOrigin() - move2 );
					
					// Rotate back angles.
					checkAngles[ vec3_t::Yaw ] -= angularDeltaMove[ vec3_t::Yaw ];
					//checkAngles[vec3_t::Yaw] -= move2[vec3_t::Yaw];
					
					// Apply new angles.
					geCheck->SetAngles( checkAngles ); //geCheck->SetAngles( geCheck->GetAngles() - move2 );
				}

				geBlock = SG_TestEntityPosition( geCheck );
				GameEntity* geBlock2 = SG_TestEntityRotation( geCheck );
				if ( !geBlock || !geBlock2 ) {//|| !geBlock2 ) { //} || !geBlock2) {
					SG_PopPushedEntityState( geCheck );

					geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
					//if (geCheck->GetClient()) {

					//	SG_CheckGround( geCheck );
					//} else {
					//	SG_Monster_CheckGround( geCheck );
					//}
					continue;
				} /*else {
					geBlock->LinkEntity();
				}*/
			}
        }

        // Save off the obstacle so we can call the block function later on.
        pushObstacle = geCheck;

        // Move back any entities we already moved. We'll go backwards, so if the same entity was pushed
        // twice, it goes back to the original position.
		for (p = lastPushedEntityState - 1 ; p >= pushedEntities; p--) {
	        // Fetch pusher's base entity.
            GameEntity *pusherEntity = SG_GetGameEntityFromPushedState(p);//GameEntity* pusherEntity = p->entityHandle;

            // Ensure we are dealing with a valid pusher entity.
            if (!pusherEntity) {
				SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
                continue;
            }

			//
            //pusherEntity->SetAngles(p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (pusherEntity->GetClient()) {
                // Delta angles are calculated for both, cl game and sv game.
				pusherEntity->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = p->deltaYaw;
				#ifdef SHAREDGAME_SERVERGAME
				pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
				pusherEntity->SetOrigin( p->origin );
				#endif
				// Specific client side origin handling in order to support async physics.
				#ifdef SHAREDGAME_CLIENTGAME
				pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
				//pusherEntity->SetOrigin( cl->predictedState.viewOrigin );
				pusherEntity->SetOrigin( p->origin );

				#endif

			//	SG_CheckGround( pusherEntity );
			} else {
				vec3_t newAngles = pusherEntity->GetAngles();
				//newAngles[vec3_t::Yaw] = p->deltaYaw;
				pusherEntity->SetAngles( p->angles );//newAngles);

				pusherEntity->SetOrigin(p->origin);
			//	SG_Monster_CheckGround( pusherEntity );
			}
#endif

			// Link Entity back in.
            pusherEntity->LinkEntity();
		
			// Be sure to check for ground.
			//#ifdef SHAREDGAME_CLIENTGAME
			//if ( !pusherEntity->GetClient() ) {
			//}
			//#endif

        }

        return false;
    }

	// Call TouchTriggers on all our moved entities.
    for (p = lastPushedEntityState - 1; p >= pushedEntities; p--) {
        // Fetch pusher's base entity.
        GameEntity* pusherEntity = SG_GetGameEntityFromPushedState(p);

        // Ensure we are dealing with a valid pusher entity.
	    if (!pusherEntity) {
			SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
            continue;
	    }

		// Enjoy,
	    SG_TouchTriggers(pusherEntity);
    }

    return true;
}

/**
*	@brief Logic for MoveType::(Push, Stop): Pushes all objects except for brush models. 
**/
void SG_Physics_Pusher( SGEntityHandle &gePusherHandle ) {
	vec3_t move = vec3_zero();
	vec3_t amove = vec3_zero();
	GameEntity *part = nullptr;
	GameEntity *mv = nullptr;

    // Assign handle to base entity.
    GameEntity* ent = SGGameWorld::ValidateEntity(gePusherHandle);

    // Ensure it is a valid entity.
    if (!ent) {
		SG_Print( PrintType::DeveloperWarning, fmt::format( "{}({}): got an invalid entity handle!\n", __func__, sharedModuleName ) );
        return;
    }

    // if not a team captain, so movement will be handled elsewhere
    if (ent->GetFlags() & EntityFlags::TeamSlave) {
        return;
	}

	// First clear out the vector.
retry:
	lastPushedEntityState = pushedEntities;

    for (part = ent; part ; part = part->GetTeamChainEntity()) {
        // Fetch pusher part, its Velocity.
        vec3_t partVelocity = part->GetVelocity();

        // Fetch pusher part, its Angular Velocity.
        vec3_t partAngularVelocity = part->GetAngularVelocity();

        if ( !vec3_equal( partVelocity, vec3_zero() ) || !vec3_equal( partAngularVelocity, vec3_zero() ) ) {
			PODEntity *partPODEntity = part->GetPODEntity();
			vec3_t partOrigin = vec3_zero();
			
			if ( partPODEntity->linearMovement.isMoving ) {
			#ifdef SHAREDGAME_CLIENTGAME
				// Calculate the actual origin of the mover for the next serverframe moment in time. 
				SG_LinearMovement( partPODEntity, (level.extrapolatedTime ).count(), partOrigin );
				// Calculate the Delta Move offset between last and current frame.
				SG_LinearMovementDelta( partPODEntity, ( level.extrapolatedTime - FRAMERATE_MS ).count(), ( level.extrapolatedTime ).count(), move );

				// Calculate angular velocity.
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() ); //VectorScale( part->avelocity, FRAMETIME, amove );

				// Debug.
				const vec3_t fromMove = part->GetOrigin();
				const vec3_t toMove = fromMove + move;
				SG_Print( PrintType::Developer, 
						 fmt::format( "[CLG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n",
						 ent->GetNumber(), level.time.count(),
						 fromMove.x, fromMove.y, fromMove.z, toMove.x, toMove.y, toMove.z, move.x, move.y, move.z
				));
			#endif
			#ifdef SHAREDGAME_SERVERGAME
				// Calculate the actual origin of the mover for the next serverframe moment in time. 
				SG_LinearMovement( partPODEntity, (level.time).count(), partOrigin );
				// Calculate the Delta Move offset between last and current frame.
				SG_LinearMovementDelta( partPODEntity, (level.time - FRAMERATE_MS).count(), (level.time).count(), move );
				// Calculate angular velocity.
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() ); //VectorScale( part->avelocity, FRAMETIME, amove );

				// Debug.
				const vec3_t fromMove = part->GetOrigin();
				const vec3_t toMove = fromMove + move;
				SG_Print( PrintType::Developer, 
						 fmt::format( "[SVG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n",
						 ent->GetNumber(), level.time.count(),
						 fromMove.x, fromMove.y, fromMove.z, toMove.x, toMove.y, toMove.z, move.x, move.y, move.z
				));
			#endif
			// Regular velocity and angular movement.
			} else {
				move = vec3_scale( part->GetVelocity(), FRAMETIME_S.count() );
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() );
			}

            SGEntityHandle partHandle(part);
            if ( !SG_MoverPush( partHandle, partOrigin, move, amove ) )
                break;  // move was Blocked
        }
    }


	if (lastPushedEntityState > &pushedEntities[MAX_WIRED_POD_ENTITIES]) {
        SG_Error( ErrorType::Drop, fmt::format( "pushed_p > &pushed[{}], memory corrupted", MAX_WIRED_POD_ENTITIES ) );
	}

    if (part) {
        // the move failed, bump all nextThinkTime times and back out moves
        for (mv = ent ; mv ; mv = mv->GetTeamChainEntity()) {
            if (mv->GetNextThinkTime() > GameTime::zero()) {
				#ifdef SHAREDGAME_CLIENTGAME
				mv->EnableExtrapolation();
				//mv->SetNextThinkTime(level.extrapolatedTime);//mv->GetNextThinkTime() + FRAMERATE_MS);//);//FRAMETIME_S);// 1_hz);
				//mv->EnableExtrapolation();
//				GameTime timeAddition = mv->GetNextThinkTime() - level.extrapolatedTime;

				//mv->SetNextThinkTime( mv->GetNextThinkTime() + FRAMERATE_MS );
				mv->SetNextThinkTime( level.extrapolatedTime + FRAMERATE_MS );
				//mv->SetNextThinkTime( mv->GetNextThinkTime() + FRAMERATE_MS );
				#else
				mv->SetNextThinkTime( mv->GetNextThinkTime() + FRAMERATE_MS );//);//FRAMETIME_S);// 1_hz);
				#endif
            }
        }

        // if the pusher has a "Blocked" function, call it
        // otherwise, just stay in place until the obstacle is gone
        if (pushObstacle) {
            part->DispatchBlockedCallback(pushObstacle);
        }

#if 1
        // if the pushed entity went away and the pusher is still there
        if ((pushObstacle && !pushObstacle->IsInUse()) && (part && part->IsInUse())) {
            goto retry;
		}
#endif
    } else {
        // the move succeeded, so call all Think functions
        for (part = ent ; part ; part = part->GetTeamChainEntity()) {
            SG_RunThink(part);
        }
    }
}