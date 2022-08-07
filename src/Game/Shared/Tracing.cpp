/***
*
*	License here.
*
*	@file
*
*	ClientGame Tracing Utility. Takes care of handling entities appropriately.
*
***/
//! Include the code base of the GameModule we're compiling against.
#include "Game/Shared/GameBindings/GameModuleImports.h"
#include "Game/Shared/Tracing.h"

/**
* @brief Constructs a ClientGame Trace Result from the engine's Common trace result.
* @param traceResult A reference to the trace result.
**/
SGTraceResult::SGTraceResult(TraceResult& traceResult) :
	allSolid(traceResult.allSolid),
	contents(traceResult.contents),
	endPosition(traceResult.endPosition),
	fraction(traceResult.fraction),
	plane(traceResult.plane),
	startSolid(traceResult.startSolid),
	surface(traceResult.surface)
{
	offsets[0] = traceResult.offsets[0];
	offsets[1] = traceResult.offsets[1];
	offsets[2] = traceResult.offsets[2];
	offsets[3] = traceResult.offsets[3];
	offsets[4] = traceResult.offsets[4];
	offsets[5] = traceResult.offsets[5];
	offsets[6] = traceResult.offsets[6];
	offsets[7] = traceResult.offsets[7];

	// If an entity has been found, look it up in our client entities and assign it.
	SGGameWorld *gameWorld = GetGameWorld();
	GameEntityVector &gameEntities = gameWorld->GetGameEntities();
	if (gameWorld) {
		GameEntityVector &gameEntities = gameWorld->GetGameEntities();

		if (traceResult.ent) {
			// Cast to PODEntity.
			PODEntity *podEntity = static_cast<PODEntity*>(traceResult.ent);

			// Acquire number.
			const uint32_t index = podEntity->currentState.number;

			// Look for entity. (Should be done using gameworld get by index...)
			if (index < gameEntities.size() && gameEntities[index] != NULL) {
				gameEntity = gameEntities[index];
			} else {
				// Default to Worldspawn instead.
				gameEntity = (GameEntity*)(gameWorld->GetWorldspawnGameEntity());
				// POD Entity still needs to be set to worldspawn.
				podEntity = gameWorld->GetWorldspawnPODEntity();
			}

			// Assign the podEntity.
			this->podEntity = podEntity;
		} else {
			// Default to Worldspawn instead.
			gameEntity = (GameEntity*)(gameWorld->GetWorldspawnGameEntity());

			// POD Entity still needs to be set to worldspawn.
			podEntity = gameWorld->GetWorldspawnPODEntity();
		}
	} else {
		podEntity = traceResult.ent;
		gameEntity = nullptr;
	}
}

/**
* @brief Constructs a ClientGame Trace Result from the engine's Common trace result.
* @param traceResult A const reference to the trace result.
**/
SGTraceResult::SGTraceResult(const TraceResult& traceResult) :
		allSolid(traceResult.allSolid),
	contents(traceResult.contents),
	endPosition(traceResult.endPosition),
	fraction(traceResult.fraction),
	plane(traceResult.plane),
	startSolid(traceResult.startSolid),
	surface(traceResult.surface)
{
	offsets[0] = traceResult.offsets[0];
	offsets[1] = traceResult.offsets[1];
	offsets[2] = traceResult.offsets[2];
	offsets[3] = traceResult.offsets[3];
	offsets[4] = traceResult.offsets[4];
	offsets[5] = traceResult.offsets[5];
	offsets[6] = traceResult.offsets[6];
	offsets[7] = traceResult.offsets[7];

	// If an entity has been found, look it up in our client entities and assign it.
	SGGameWorld *gameWorld = GetGameWorld();
	GameEntityVector &gameEntities = gameWorld->GetGameEntities();

	if (traceResult.ent) {
		// Cast to PODEntity.
		PODEntity *tracePODEntity = static_cast<PODEntity*>(traceResult.ent);

		// Acquire index number.
		const uint32_t index = tracePODEntity->currentState.number;

		// Look up both the POD and Game Enties into our gameworld to make sure we got the proper pointers.
		SGGameWorld *gameWorld = GetGameWorld();
		gameEntity	= gameWorld->GetGameEntityByIndex(index);
		podEntity	= gameWorld->GetPODEntityByIndex(index);

		// We do want to have a matching game entity.
		if (!gameEntity) {
			// Otherwise default both, POD and Game Entity to Worldspawn instead.
			gameEntity	= gameWorld->GetGameEntityByIndex(0);
			// POD Entity still needs to be set to worldspawn.
			podEntity	= gameWorld->GetPODEntityByIndex(0);
		}
	} else {
		// Default to Worldspawn instead.
		gameEntity	= gameWorld->GetGameEntityByIndex(0);
		// POD Entity still needs to be set to worldspawn.
		podEntity	= gameWorld->GetPODEntityByIndex(0);
	}
}

/**
*	@brief	SharedGame Trace Functionality: Supports GameEntities :-)
**/
SGTraceResult SG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, GameEntity* skipGameEntity, const int32_t& contentMask) {
    // Fetch POD Entity to use for pass entity testing.
    PODEntity* podEntity = (skipGameEntity ? skipGameEntity->GetPODEntity() : NULL);

	// Execute and return the actual trace.
#ifdef SHAREDGAME_SERVERGAME
    return gi.Trace(start, mins, maxs, end, (struct PODEntity*)podEntity, contentMask);
#endif
#ifdef SHAREDGAME_CLIENTGAME
    return clgi.Trace(start, mins, maxs, end, (struct PODEntity*)podEntity, contentMask);
#endif
}

/**
*	@brief	SharedGame PointContents Functionality: Supports GameEntities :-)
**/
const int32_t SG_PointContents(const vec3_t &point) {
#ifdef SHAREDGAME_SERVERGAME
	return gi.PointContents(point);
#endif
#ifdef SHAREDGAME_CLIENTGAME
    return clgi.PointContents(point);
#endif
}

/**
*	@return	GameEntityVector filled with the entities that were residing inside the box. Will not exceed listCount limit.
**/
GameEntityVector SG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType) {
    // Boxed server entities set by gi.BoxEntities.
    PODEntity* boxedServerEntities[MAX_WIRED_POD_ENTITIES];

    // Vector of the boxed class entities to return.
    GameEntityVector boxedClassEntities;

    // Acquire pointer to the class entities array.
	SGGameWorld *gameWorld = GetGameWorld();
    GameEntityVector gameEntities = gameWorld->GetGameEntities();

    // Ensure the listCount can't exceed the max edicts.
    if (listCount > MAX_WIRED_POD_ENTITIES) {
        listCount = MAX_WIRED_POD_ENTITIES;
    }

    // Box the entities.
#ifdef SHAREDGAME_CLIENTGAME
	int32_t numEntities = clgi.BoxEntities(mins, maxs, boxedServerEntities, listCount, areaType);
#endif
#ifdef SHAREDGAME_SERVERGAME
    int32_t numEntities = gi.BoxEntities(mins, maxs, boxedServerEntities, listCount, areaType);
#endif

    // Go through the boxed entities list, and store there classEntities (SVGBaseEntity aka baseEntities).
    for (int32_t i = 0; i < numEntities; i++) {
        if (gameEntities[boxedServerEntities[i]->currentState.number] != nullptr) {
            boxedClassEntities.push_back(gameEntities[boxedServerEntities[i]->currentState.number]);
        }
    }

    // Return our boxed base entities vector.
    return boxedClassEntities;
}

/**
*	@brief	Scans whether this entity is touching any others, and if so, dispatches their touch callback function.
**/
void SG_TouchTriggers(GameEntity* geToucher) {
	if (!geToucher) {
		return;
	}
	// Dead things don't activate triggers!
	if ( geToucher &&
		( ( geToucher->GetClient() ) || ( geToucher->GetServerFlags() & EntityServerFlags::Monster ) ) &&
		geToucher->GetHealth() <= 0
	) {
        return;
	}

    // Fetch the boxed entities.
    GameEntityVector touched = SG_BoxEntities(geToucher->GetAbsoluteMin(), geToucher->GetAbsoluteMax(), MAX_WIRED_POD_ENTITIES, AreaEntities::Triggers);

    // Do some extra sanity checks on the touched entity list. It is possible to have 
    // an entity be removed before we get to it (kill triggered).
    for (auto& touchedEntity : touched) {
        if (!touchedEntity) {
	        continue;
        }
	    if (!touchedEntity->GetPODEntity()) {
	        continue;
	    }
	    if (!touchedEntity->IsInUse()) {
		    continue;
	    }

        touchedEntity->DispatchTouchCallback(touchedEntity, geToucher, NULL, NULL);
    }
}
