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
	const vec3_t playerMoveOrigin = ( isClientEntity ? gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
	//const vec3_t playerMoveOrigin = ( isClientEntity ? cl->predictedState.viewOrigin : vec3_zero() ); //gePushMove->GetClient()->playerState.pmove.origin : vec3_zero() );// = p->playerMoveOrigin;
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
		.moveTime = (level.extrapolatedTime - FRAMERATE_MS).count(),
		.moveNextTime = level.extrapolatedTime.count(),
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

	if (pushedEntities->entityNumber < 0) {
		return nullptr;
	}

	// Utilize the EntityHandle class.
	SGEntityHandle ehPushedStateEntity = gameWorld->GetPODEntityByIndex(pushedEntityState->entityNumber);

	// Validate the actual EntityHandle and get a hopefully non (nullptr) GameEntity. 
	GameEntity *validEntity = SGGameWorld::ValidateEntity(ehPushedStateEntity);

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

    trace = SG_Trace( start, ent->GetMins(), ent->GetMaxs(), end, ent, mask );

	if ( ent->GetMoveType() == MoveType::Push || !trace.startSolid ) {
	//if (!trace.startSolid) {//if (!trace.startSolid) {
		ent->SetOrigin( trace.endPosition);
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

    SGTraceResult trace = SG_Trace( geTestSubject->GetOrigin(), geTestSubject->GetMins(), geTestSubject->GetMaxs(), geTestSubject->GetOrigin(), geTestSubject, clipMask );

    if ( trace.startSolid ) {
		SGGameWorld *gameWorld = GetGameWorld();

	    return (GameEntity*)( gameWorld->GetWorldspawnGameEntity() );
    }

    return nullptr;
}
static GameEntity *SG_TestEntityRotation( GameEntity *geTestSubject ) {
	
	// Get mins and maxs.
	vec3_t mins = geTestSubject->GetMins();
	vec3_t maxs = geTestSubject->GetMaxs();

    // expand for rotation
    float max = 0;
	float v = 0;
    for ( int32_t i = 0; i < 3; i++ ) {
        v = fabs( mins[i] );
        if ( v > max ) {
            max = v;
		}
        v = fabs( maxs[i] );
        if ( v > max ) {
            max = v;
		}
    }

	// Calcualte absMin and absMax for rotated BSP Entity.
	const vec3_t rotatedBoxMins = vec3_t{-max, -max, mins[2] };
	const vec3_t rotatedBoxMaxs = vec3_t{ max, max,  maxs[2] };

	int32_t clipMask = 0;
    if (geTestSubject->GetClipMask()) {
	    clipMask = geTestSubject->GetClipMask();
    } else {
        clipMask = BrushContentsMask::Solid;
    }

    SGTraceResult trace = SG_Trace(geTestSubject->GetOrigin(), rotatedBoxMins, rotatedBoxMaxs, geTestSubject->GetOrigin(), geTestSubject, clipMask);

    if (trace.startSolid) {
		SGGameWorld *gameWorld = GetGameWorld();

	    return (GameEntity*)(gameWorld->GetWorldspawnGameEntity());
    }

    return nullptr;
}

/**
*	@brief	Objects need to be moved back on a failed push, otherwise 
*			riders would continue to slide.
**/
const bool SG_Push_IsSameEntity( GameEntity *geFirst, GameEntity *geSecond ) {
	// The same by pointer.
	const int32_t numberGeFirst = ( geFirst ? geFirst->GetNumber() : -1 );
	const int32_t numberGeSecond= ( geSecond ? geSecond->GetNumber() : -1 );

	// If first or second is -1, it is a false. (We do not return true in case of it being empty space)
	if ( numberGeFirst == -1 || numberGeSecond == -1 ) {
		return false;
	}

	if ( numberGeFirst == numberGeSecond ) {
		return true;
	}

	return false;
}
const bool SG_Push( SGEntityHandle &entityHandle, const vec3_t &partOrigin, const vec3_t &deltaMove, const vec3_t &angularMove ) {
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

    // Find the bounding box.
    vec3_t mins = gePusher->GetAbsoluteMin() + deltaMove;
    vec3_t maxs = gePusher->GetAbsoluteMax() + deltaMove;

    // We need this for pushing things later
    org = vec3_negate(angularMove);
    AngleVectors(org, &forward, &right, &up);

	/**	
	*	Store the needed pushed entity information in a pushed entity state.
	**/
	SG_PushEntityState( gePusher );

	/**	
	*	Move the Pusher to its wished final position.
	**/
    gePusher->SetOrigin( gePusher->GetOrigin() + deltaMove );//gePusher->SetOrigin( gePusher->GetOrigin() + move );
    gePusher->SetAngles( gePusher->GetAngles() + angularMove );
    gePusher->LinkEntity();
    
	//#ifdef SHAREDGAME_CLIENTGAME
	//int64_t frameNumber = cl->frame.number;
	//int64_t timeNow = level.time.count();
	//int64_t timeNext = (level.time + FRAMERATE_MS).count();
	//SG_Print( PrintType::DeveloperWarning, fmt::format("[[[[CLG]]]]: origin({}, {}, {}), deltaMove({}, {}, {}), frameNumber({}), timeStart({}), timeNext({})\n", partOrigin.x, partOrigin.y, partOrigin.z, deltaMove.x, deltaMove.y, deltaMove.z, frameNumber, timeNow, timeNext ));
	//#endif
	//#ifdef SHAREDGAME_SERVERGAME
	//int64_t frameNumber = level.frameNumber;
	//int64_t timeNow = level.time.count();
	//int64_t timeNext = (level.time + FRAMERATE_MS).count();
	//SG_Print( PrintType::DeveloperWarning, fmt::format("[[[[SVG]]]]: origin({}, {}, {}), deltaMove({}, {}, {}), frameNumber({}), timeStart({}), timeNext({})\n", partOrigin.x, partOrigin.y, partOrigin.z, deltaMove.x, deltaMove.y, deltaMove.z, frameNumber, timeNow, timeNext ));
	//#endif

	/**	
	*	See if the position has been taken by other entities, and if so, try and push each other.
	**/
	// Get a range of all pushable entities in our world. (A valid GameEntity and Inuse.)
	SGGameWorld *gameWorld = GetGameWorld();
	//auto gePushables = SG_BoxEntities( mins, maxs, MAX_POD_ENTITIES, AreaEntities::Solid );
	auto gePushables = gameWorld->GetGameEntityRange(0, MAX_POD_ENTITIES) | cef::IsValidPointer | cef::InUse;

	// Iterate over the pushable entities.
	for ( auto geCheck : gePushables ) {
        // Fetch its properties to work with.
        const qboolean isInUse = geCheck->IsInUse();
        const int32_t moveType = geCheck->GetMoveType();
        const vec3_t absMin = geCheck->GetAbsoluteMin();
        const vec3_t absMax = geCheck->GetAbsoluteMax();

		// Skip moveTypes that aren't pushed around at all.
        if ( moveType == MoveType::Push   || moveType == MoveType::Stop   ||
			 moveType == MoveType::None   || moveType == MoveType::NoClip ||
			 moveType == MoveType::Spectator ) {
            continue;
		}

		// Entity has to be linked in.
		//#if SHAREDGAME_CLIENTGAME
		//		//if ( !geCheck->GetLinkCount() ) {
		//		if ( !geCheck->GetPODEntity()->area.prev ) {
		//			auto client = geCheck->GetClient();
		//			if (client) {
		//			//	SG_Print( PrintType::DeveloperWarning, fmt::format( "[Ent(#{}),Client(#{})]: Not linked in anywhere", geCheck->GetNumber(), client->clientNumber ) );
		//			} else {
		//			//	SG_Print( PrintType::DeveloperWarning, fmt::format( "[Ent(#{}),Client(nullptr)]: Not linked in anywhere\n", geCheck->GetNumber() ) );
		//			}
		//#endif
		//#if SHAREDGAME_SERVERGAME
        if ( !geCheck->GetPODEntity()->area.prev ) {
		//#endif
            continue;       // not linked in anywhere
		}

        // if the entity is standing on the pusher, it will definitely be moved
		GameEntity *geCheckGroundEntity = SGGameWorld::ValidateEntity( geCheck->GetGroundEntityHandle() );
		if ( !SG_Push_IsSameEntity( geCheckGroundEntity, gePusher ) ) {
            // see if the ent needs to be tested
            if ( absMin[0] >= maxs[0]	||
                absMin[1] >= maxs[1]	||
                absMin[2] >= maxs[2]	||
                absMax[0] <= mins[0]	||
                absMax[1] <= mins[1]	||
                absMax[2] <= mins[2] ) {
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
			*	Try moving the contacted entity.
			**/
            geCheck->SetOrigin( geCheck->GetOrigin() + deltaMove );
            if ( geCheck->GetClient() ) {
                // FIXME: doesn't rotate monsters?
                // FIXME: skuller: needs client side interpolation
				#if USE_SMOOTH_DELTA_ANGLES
				geCheck->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
				#endif
			} else {
				#if USE_SMOOTH_DELTA_ANGLES
				vec3_t angles = geCheck->GetAngles();
				//angles[vec3_t::Yaw] += angularMove[vec3_t::Yaw];
				//				SG_Print( PrintType::DeveloperWarning, fmt::format("[Ent(#{}) Client(nullptr)]:", geCheck->GetNumber(), 
				//		 angularMove[vec3_t::Pitch],
				//		 angularMove[vec3_t::Yaw],
				//		angularMove[vec3_t::Roll]
				//	)
				//);
				geCheck->SetAngles(angles);
				#endif
			}


			/**
			*	Figure movement due to the pusher's Angular Move.
			**/
            org = geCheck->GetOrigin() - gePusher->GetOrigin();
            org2[0] = vec3_dot(org, forward);
            org2[1] = -vec3_dot(org, right);
            org2[2] = vec3_dot(org, up);
            move2 = org2 - org;
            geCheck->SetOrigin( geCheck->GetOrigin() + move2 );

			// Client pmove.
            if ( geCheck->GetClient() ) {
				org = geCheck->GetClient()->playerState.pmove.origin - gePusher->GetOrigin(); //VectorSubtract(geCheck->state.origin, pusher->state.origin, org);
				org2[0] = vec3_dot(org, forward);
				org2[1] = -vec3_dot(org, right);
				org2[2] = vec3_dot(org, up);
				move2 = org2 - org;
				geCheck->GetClient()->playerState.pmove.origin += move2;
			}

            // May have pushed them off an edge
			if ( !SG_Push_IsSameEntity( geCheckGroundEntity, gePusher ) ) {
                geCheck->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
			}

			// Test whether entity is inside of another, if not, push was okay so link it and move
			// on to the next entity that needs pushing.
            geBlock = SG_TestEntityPosition( geCheck );

			//GameEntity *geBlock2 = SG_TestEntityRotation( geCheck );
            if ( !geBlock /*&& !geBlock2 */) {
                // pushed ok
                geCheck->LinkEntity();
                // impact?
                continue;
            }// else {
				// Of it is ok to leave in the old position, do it.
				// This is only relevent for riding entities, not pushed
				// FIXME: this doesn't acount for rotation
				geCheck->SetOrigin( geCheck->GetOrigin() - deltaMove );//geCheck->state.origin -= move;
				// Client origin.
				if ( geCheck->GetClient() ) {
					geCheck->GetClient()->playerState.pmove.origin -= deltaMove;
				}
				//geCheck->SetAngles( geCheck->GetAngles() - move2 );

				geBlock = SG_TestEntityPosition( geCheck );
				//GameEntity* geBlock2 = SG_TestEntityRotation( geCheck );
				if ( !geBlock /*|| !geBlock2 */) { //} || !geBlock2) {
					SG_PopPushedEntityState( geCheck );

					continue;
				}
			//}
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

			//pusherEntity->SetOrigin(p->origin);
   //         pusherEntity->SetAngles(p->angles);
#if USE_SMOOTH_DELTA_ANGLES
            if (pusherEntity->GetClient()) {
                // Delta angles are calculated for both, cl game and sv game.
				pusherEntity->GetClient()->playerState.pmove.deltaAngles[vec3_t::Yaw] = p->deltaYaw;
				#ifdef SHAREDGAME_SERVERGAME
				//pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
				//pusherEntity->SetOrigin( p->playerMoveOrigin );
				vec3_t reverseDeltaMove = vec3_zero();
				SG_LinearMovementDelta( pusherEntity->GetPODEntity(), (p->moveTime - (FRAMERATE_MS).count()), (p->moveNextTime - (FRAMERATE_MS).count()), reverseDeltaMove );
				// Negate the delta move and use that to return with instead. Current times might be different than the stored previous time.
				//const vec3_t negatedDeltaMove = vec3_negate( deltaMove );
				pusherEntity->GetClient()->playerState.pmove.origin -= reverseDeltaMove; //p->playerMoveOrigin;
				pusherEntity->SetOrigin( pusherEntity->GetOrigin() - reverseDeltaMove );
				#endif

				// Specific client side origin handling in order to support async physics.
				#ifdef SHAREDGAME_CLIENTGAME
				vec3_t reverseDeltaMove = vec3_zero();
				SG_LinearMovementDelta( pusherEntity->GetPODEntity(), (p->moveTime - (FRAMERATE_MS).count()), p->moveTime, reverseDeltaMove );
				// Negate the delta move and use that to return with instead. Current times might be different than the stored previous time.
				//const vec3_t negatedDeltaMove = vec3_negate( deltaMove );
				pusherEntity->GetClient()->playerState.pmove.origin -= reverseDeltaMove; //p->playerMoveOrigin;
				//pusherEntity->SetOrigin( pusherEntity->GetOrigin() - reverseDeltaMove );
				//pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
				//pusherEntity->SetOrigin( cl->predictedState.viewOrigin );
				#endif

				//pusherEntity->GetClient()->playerState.pmove.origin = p->playerMoveOrigin;
				//#ifdef SHAREDGAME_CLIENTGAME
				
				//#endif
			} else {
				pusherEntity->SetOrigin(p->origin);
				pusherEntity->SetAngles(p->angles);

				vec3_t newAngles = pusherEntity->GetAngles();
				newAngles[vec3_t::Yaw] = p->deltaYaw;
				pusherEntity->SetAngles(newAngles);
			}
#endif
			// Link Entity back in.
            pusherEntity->LinkEntity();
		
			// Be sure to check for ground.
			#ifdef SHAREDGAME_CLIENTGAME
			if ( !pusherEntity->GetClient() ) {
				SG_CheckGround( pusherEntity );
			}
			#endif

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

        if (partVelocity.x || partVelocity.y || partVelocity.z ||
            partAngularVelocity.x || partAngularVelocity.y || partAngularVelocity.z) 
        {
			PODEntity *partPODEntity = part->GetPODEntity();
			vec3_t partOrigin = vec3_zero();

			if ( partPODEntity->linearMovement ) {
			#ifdef SHAREDGAME_CLIENTGAME
				// Calculate the actual origin of the mover for the next serverframe moment in time. 
				//SG_LinearMovement( partPODEntity, (level.time + FRAMERATE_MS).count(), partOrigin );
				SG_LinearMovement( partPODEntity, (level.extrapolatedTime ).count(), partOrigin );
				// Delta move.
				//SG_LinearMovementDelta( partPODEntity, level.time.count(), ( level.time + FRAMERATE_MS ).count(), move );
				SG_LinearMovementDelta( partPODEntity, (level.extrapolatedTime - FRAMERATE_MS).count(), ( level.extrapolatedTime ).count(), move );

				// Calculate angular velocity.
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() ); //VectorScale( part->avelocity, FRAMETIME, amove );

				// Debug.
				const vec3_t fromMove = part->GetOrigin();
				const vec3_t toMove = fromMove + move;
				SG_Print( PrintType::Developer, 
						 fmt::format( "[CLG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n",
						 ent->GetNumber(),
						 level.time.count(),
						 fromMove.x,
						 fromMove.y,
						 fromMove.z,
						 toMove.x,
						 toMove.y,
						 toMove.z,
						 move.x,
						 move.y,
						 move.z
				));
			#endif
			#ifdef SHAREDGAME_SERVERGAME
				// Calculate the actual origin of the mover for the next serverframe moment in time. 
				SG_LinearMovement( partPODEntity, (level.time).count(), partOrigin );
				// Delta move.
				SG_LinearMovementDelta( partPODEntity, (level.time - FRAMERATE_MS).count(), (level.time).count(), move );

				// Calculate angular velocity.
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() ); //VectorScale( part->avelocity, FRAMETIME, amove );

				// Debug.
				const vec3_t fromMove = part->GetOrigin();
				const vec3_t toMove = fromMove + move;
				SG_Print( PrintType::Developer, 
						 fmt::format( "[SVG SG_Physics_Pusher(#{}, 'func_plat', level.time({})]: fromMove({}, {}, {}), toMove({}, {}, {}), move({}, {}, {})\n",
						 ent->GetNumber(),
						 level.time.count(),
						 fromMove.x,
						 fromMove.y,
						 fromMove.z,
						 toMove.x,
						 toMove.y,
						 toMove.z,
						 move.x,
						 move.y,
						 move.z
				));
			#endif
			} else {
				// object is moving
				move = vec3_scale( part->GetVelocity(), FRAMETIME_S.count() );
				amove = vec3_scale( part->GetAngularVelocity(), FRAMETIME_S.count() );
			}

            SGEntityHandle partHandle(part);
            if ( !SG_Push( partHandle, partOrigin, move, amove ) )
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
				mv->SetNextThinkTime( mv->GetNextThinkTime() + FRAMERATE_MS );
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