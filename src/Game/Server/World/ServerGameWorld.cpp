/***
*
*	License here.
*
*	@file
*
***/
// Core.
#include "../ServerGameLocals.h"

// Entities.
#include "../Entities.h"
#include "../Entities/Base/SVGBasePlayer.h"
#include "../Entities/Base/DebrisEntity.h"
#include "../Entities/Base/GibEntity.h"

// GameModes.
#include "../Gamemodes/IGamemode.h"
#include "../Gamemodes/DefaultGamemode.h"
#include "../Gamemodes/CoopGamemode.h"
#include "../Gamemodes/DeathMatchGamemode.h"

// GameWorld.
#include "../World/ServerGameWorld.h"

// Cvars.
extern cvar_t *gamemode;


/**
*	@brief Initializes the gameworld and its member objects.
***/
void ServerGameWorld::Initialize() {
	// Prepare the items.
    PrepareItems();

	// Prepare the entities.
    PrepareEntities();

	// Prepare the clients.
    PrepareClients();

	// What game could one play without a gamemode?
	SetupGameMode();
}

/**
*	@brief Shutsdown the gameworld and its member objects.
**/
void ServerGameWorld::Shutdown() {
	DestroyGameMode();
}



/**
*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
**/
void ServerGameWorld::SetupGameMode() {
	// Fetch gamemode as std::string
	std::string gamemodeStr = gamemode->string;

	// Detect which game ode to allocate for this game round.
	if (gamemodeStr == "deathmatch") {
		// Deathmatch gameplay mode.
	    currentGameMode = new DeathmatchGameMode();
	} else if (gamemodeStr == "coop") {
		// Cooperative gameplay mode.
	    currentGameMode = new CoopGameMode();
	} else {
		// Acts as a singleplayer game mode.
	    currentGameMode = new DefaultGameMode();
	}
}

/**
*	@brief	Destroys the current gamemode object.
**/
void ServerGameWorld::DestroyGameMode() {
	// Always make sure it is valid to begin with.
    if (currentGameMode != nullptr) {
		delete currentGameMode;
		currentGameMode = nullptr;
	}
}



/**
*   @brief	Assigns the exported globals.entities and ensures that all class entities 
*			are set to nullptr. Also does a clamp on maxEntities for sanity.
**/
void ServerGameWorld::PrepareEntities() {
    // Clamp it just in case.
    int32_t maxEntities = Clampi(MAX_WIRED_POD_ENTITIES, (int)maximumclients->value + 1, MAX_WIRED_POD_ENTITIES);

    // Setup the globals entities pointer and max entities value so
	// that the server can access it.
    globals.entities = podEntities;
    globals.maxEntities = this->maxEntities = maxEntities;

    // Ensure, all base entities are nullptrs. Just to be save.
    for (int32_t i = 0; i < MAX_POD_ENTITIES; i++) {
		gameEntities[i] = nullptr;
    }
}

/**
*   @brief Prepares the game clients array for use.
**/
void ServerGameWorld::PrepareClients() {
    // Allocate our clients array.
    maxClients = maximumclients->value;
    clients = (ServerClient*)gi.TagMalloc(maxClients * sizeof(clients[0]), TAG_GAME);  // CPP: Cast

    // Current total number of entities in our game = world + maximum clients.
    globals.numberOfEntities = numberOfEntities = maxClients + 1;
}

/**
*	@brief Prepares the game's client entities with a base player game entity.
**/
void ServerGameWorld::PreparePlayers() {
    // Allocate a classentity for each client in existence.
    for (int32_t i = 1; i < maxClients + 1; i++) {
		// Fetch POD entity.
		PODEntity* podEntity = &podEntities[i];

		// We can fetch the number based on subtracting these two pointers.
		podEntity->currentState.number = podEntity - podEntities;

		// Allocate player client game entity
		SVGBasePlayer* playerClientEntity = CreateGameEntity<SVGBasePlayer>(podEntity, false);

		// Be sure to reset their inuse, after all, they aren't in use.
		playerClientEntity->SetInUse(false);

		// Fetch client index.
		const int32_t clientIndex = i - 1;  // Same as the older: podEntities - podEntities - 1;

		// Assign the designated client to this SVGBasePlayer entity.
		playerClientEntity->SetClient(&clients[clientIndex]);
    }
}

/**
*	@brief	Reserves the game's body queue entity slots.
**/
void ServerGameWorld::PrepareBodyQueue() {
    // Reserve some spots for dead player bodies for coop / deathmatch
    level.bodyQue = 0;
    for (int i = 0; i < BODY_QUEUE_SIZE; i++) {
	    Entity* ent = GetUnusedPODEntity();

        //ent->classname = "bodyque";
    }
}

/**
*	@brief	Parses the 'entities' string and assigns each parsed entity to the
*			first free POD entity slot there is. After doing so, allocates
*			a game entity based on the 'classname' of the parsed entity.
**/
qboolean ServerGameWorld::PrepareBSPEntities(const char* mapName, const char* entities, const char* spawnpoint) {
	// Clear level state.
    level = {};

    // Delete class entities if they are allocated, and reset the POD entity to a zero state.
    for (int32_t i = 0; i < game.GetMaxEntities(); i++) {
		// Delete game entity.
		if (gameEntities[i]) {
		    delete gameEntities[i];
			gameEntities[i] = NULL;
		}

		// Reset POD entity to a zero state.
		gameEntities[i] = {};
    }

	// Copy in the map name and designated spawnpoint(if any.)
    strncpy(level.mapName, mapName, sizeof(level.mapName) - 1);
    strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint) - 1);

	// Spawn SVGBasePlayer classes for each reserved client entity.
    PreparePlayers();

	// Reserve dead body queue slots.
	PrepareBodyQueue();

	// We'll keep on parsing until this is set to false.
	qboolean isParsing = true;
	// This gets set to false the immediate moment we run into parsing trouble.
	qboolean parsedSuccessfully = false;
	// Token pointer.
	char *com_token = nullptr;

    uint32_t packetEntityIndex = 0; // We start from the max clients.         
	uint32_t localEntityIndex = MAX_WIRED_POD_ENTITIES; // TODO: That RESERVED_COUNT thingy.

	// Engage parsing.
	while (!!isParsing == true) {
		// Parse the opening brace.
		com_token = COM_Parse(&entities);

		if (!entities) {
			break;
		}

		if (com_token[0] != '{') {
			gi.Error(ErrorType::Drop, "SVGame Error: PrepareBSPEntities: found %s when expecting {\n", com_token);
			return false;
		}

		// SpawnKeys.
		SpawnKeyValues parsedKeyValues;
        parsedSuccessfully = ParseEntityString(&entities, parsedKeyValues);

		// Is this entity local?
		bool isLocal = false;
		
		// This is the actual entity index that's in use.
		uint32_t entityIndex = 0;

		// POD Entity.
		PODEntity *podEntity = nullptr;
		
		// If the dictionary has a classname, and it has client_ residing in it, change the entity index to use.
		if (parsedKeyValues.contains("classname") && (parsedKeyValues["classname"].find("client_") != std::string::npos || parsedKeyValues["classname"].find("_client") != std::string::npos)) {
			// Increment local entity count.
			localEntityIndex++;
		//	//entityIndex = localEntityIndex;

		//	// Entity is local.
			isLocal = true;
			//podEntity = GetUnusedPODEntity(false);
		} else if (parsedKeyValues.contains("classname") && parsedKeyValues["classname"] == "worldspawn") {
		//if (parsedKeyValues.contains("classname") && parsedKeyValues["classname"] == "worldspawn") {
			// Just use 0 index.
			//entityIndex = 0;
			podEntity = GetPODEntityByIndex(0);
		} else {
			// Increment packet entity count. 
			packetEntityIndex++;
			//entityIndex = packetEntityIndex;
			podEntity = GetUnusedPODEntity(true);
		}

		// Ensure the POD Entity is a valid ptr.
		if (!podEntity) {
			parsedSuccessfully = false;
			continue;
		}

		// Setup basic POD data.
		//podEntity->clientEntityNumber	= entityIndex;
		podEntity->isLocal	= isLocal;
				
		// Try and create the game entity from dictionary, and precache/spawn it.
		if (!CreateGameEntityFromDictionary(podEntity, parsedKeyValues)) {
			FreePODEntity(podEntity);
			parsedSuccessfully = false;
			continue;
		}

		// Precache & Spawn.
		podEntity->gameEntity->Precache();
		podEntity->gameEntity->Spawn();

		// If it was a local entity, be sure to prepare its previousState.
		if (isLocal) {
			//podEntity->previousState = podEntity->currentState;
		}
		
		// Assign parsed dictionary to entity.
		podEntity->spawnKeyValues = parsedKeyValues;
	}

	// Post spawn entities.
	for (auto& gameEntity : gameEntities) {
		if (gameEntity) {
			gameEntity->PostSpawn();
		}
	}

	// Find and hook team slaves.
	FindTeams();

	// Initialize player trail...
	// SVG_PlayerTrail_Init

	return parsedSuccessfully;
}

/**
*	@brief	Looks for the first free POD entity in our buffer.
* 
*	@details	Either finds a free POD entity, or initializes a new one.
*				Try to avoid reusing an entity that was recently freed, because it
*				can cause the client to Think the entity morphed into something else
*				instead of being removed and recreated, which can cause interpolated
*				angles and bad trails.
* 
*	@return	If successful, a valid pointer to the entity. If not, a nullptr.
**/
PODEntity* ServerGameWorld::GetUnusedPODEntity(bool isWired) { 
	// We'll loop until from maxclients + 1(world entity) till the numberOfEntities 
	// has been reached. If we never managed to return a pointer to a valid server 
	// entity right now, we're going to have to increase the amount of entities in use. 
	// 
	// However, this ONLY proceeds if we haven't already hit the maximum entity count.
	int32_t rangeStart		= (isWired ? maxClients + 1 : MAX_WIRED_POD_ENTITIES);
	int32_t rangeMaximum	= (isWired ? MAX_WIRED_POD_ENTITIES : MAX_SERVER_POD_ENTITIES);

	// Loop through the range to find a free unused POD Entity.
	int32_t podEntityIndex = 0;
	for (int32_t podEntityIndex = rangeStart; podEntityIndex < rangeMaximum; podEntityIndex++) {
		// Fetch POD Entity.
		PODEntity *podEntity = GetPODEntityByIndex(podEntityIndex);

        // The first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
	    if (!podEntity->inUse && (podEntity->freeTime < FRAMETIME_S * 2 || level.time - podEntity->freeTime > 500ms)) {
            // Set entity to "inUse".
			podEntity->inUse = true;
			
			// Ensure the entity is set to be local/non-local.
			podEntity->isLocal = (isWired ? false : true);

			// Ensure the state number is set correctly.
			podEntity->currentState.number = podEntityIndex;//podEntity - podEntities;
			
			if (podEntityIndex >= numberOfEntities) {
				numberOfEntities++;
				globals.numberOfEntities = numberOfEntities;
			}

			// Return the newly found POD entity pointer.
			return podEntity;
        }
    }

	// Do a safety check to prevent crossing maximum entity limit. If we do, error out.
    if (podEntityIndex >= (isWired ? MAX_WIRED_POD_ENTITIES : MAX_SERVER_POD_ENTITIES)) {// maxEntities) {
        gi.Error(ErrorType::Drop, "SVGame Error: ServerGameWorld::GetUnusedPODEntity: no free edicts\n");
		return nullptr;
	}

	return nullptr;
 //   // If we've gotten past the gi.Error, it means we can safely increase the number of entities.
 //   numberOfEntities++;
	//globals.numberOfEntities = numberOfEntities;

 //   // Set entity to "inUse".
 //   podEntity->inUse = true;

 //   // Set the entity state number.
 //   podEntity->currentState.number = podEntity - podEntities;

 //   // Return the POD entity.
 //   return podEntity;
	//   // Incrementor, declared here so we can access it later on.
	//int32_t i = 0;

	//// Acquire a pointer to the first POD entity to start checking from.
	//PODEntity *podEntity = &podEntities[maxClients + 1];

	//// We'll loop until from maxclients + 1(world entity) till the numberOfEntities 
	//// has been reached. If we never managed to return a pointer to a valid server 
	//// entity right now, we're going to have to increase the amount of entities in use. 
	//// 
	//// However, this ONLY proceeds if we haven't already hit the maximum entity count.
	//for (int32_t i = maxClients + 1; i < numberOfEntities; i++, podEntity++) {
 //       // The first couple seconds of server time can involve a lot of
 //       // freeing and allocating, so relax the replacement policy
	//    if (!podEntity->inUse && (podEntity->freeTime < FRAMETIME_S * 2 || level.time - podEntity->freeTime > 500ms)) {
 //           //SVG_InitEntity(serverEntity);
 //           // Set entity to "inUse".
	//		podEntity->inUse = true;
	//		
	//		// Set the entity state number.
	//		podEntity->currentState.number = podEntity - podEntities;
	//		
	//		// Return the newly found POD entity pointer.
	//		return podEntity;
 //       }
 //   }

	//// Do a safety check to prevent crossing maximum entity limit. If we do, error out.
 //   if (i >= maxEntities) {
 //       gi.Error(ErrorType::Drop, "SVGame Error: ServerGameWorld::GetUnusedPODEntity: no free edicts");
	//	return nullptr;
	//}

 //   // If we've gotten past the gi.Error, it means we can safely increase the number of entities.
 //   numberOfEntities++;
	//globals.numberOfEntities = numberOfEntities;

 //   // Set entity to "inUse".
 //   podEntity->inUse = true;

 //   // Set the entity state number.
 //   podEntity->currentState.number = podEntity - podEntities;

 //   // Return the POD entity.
 //   return podEntity;
}

/**
*   @brief Chain together all entities with a matching team field
* 
*   @details    All but the first will have the EntityFlags::TeamSlave flag set.
*               All but the last will have the teamchain field set to the next one.
**/
void ServerGameWorld::FindTeams() {
    PODEntity *podEntityA = nullptr;
	PODEntity *podEntityB = nullptr;
    GameEntity *chain = nullptr;
    int32_t i, j;

    int32_t c = 0;
    int32_t c2 = 0;
    for (i = 1, podEntityA = podEntities + i; i < numberOfEntities; i++, podEntityA++) {
        // Fetch game entity.
        GameEntity *gameEntityA = gameEntities[podEntityA->currentState.number];

        if (gameEntityA == nullptr) {
            continue;
		}
        if (!gameEntityA->IsInUse()) {
            continue;
		}
        if (gameEntityA->GetTeam().empty()) {
            continue;
		}
		if (gameEntityA->GetFlags() & EntityFlags::TeamSlave) { 
            continue;
		}
        chain = gameEntityA;
        gameEntityA->SetTeamMasterEntity(gameEntityA);
        c++;
        c2++;

        for (j = i + 1, podEntityB = podEntityA + 1 ; j < globals.numberOfEntities ; j++, podEntityB++) {
            // Fetch game entity.
            GameEntity* gameEntityB = gameEntities[podEntityA->currentState.number];

            if (gameEntityB == nullptr) { 
                continue;
			}
            if (!gameEntityB->IsInUse()) { 
                continue;
			}
			if (gameEntityB->GetTeam().empty()) {
				continue;
			}
			if (gameEntityB->GetFlags() & EntityFlags::TeamSlave) {
				continue;
			}
            if (gameEntityA->GetTeam() == gameEntityB->GetTeam()) {
                c2++;
                chain->SetTeamChainEntity(gameEntityB);
                gameEntityB->SetTeamMasterEntity(gameEntityA);
                chain = gameEntityB;
                gameEntityB->SetFlags(gameEntityB->GetFlags() | EntityFlags::TeamSlave);
            }
        }
    }

    gi.DPrintf("ServerGameWorld: Found (#%i) Teams with (#%i) of total Entities.\n", c, c2);
}

/**
*	@brief	Parses the BSP Entity string and places the results in the server
*			entity dictionary.
**/
qboolean ServerGameWorld::ParseEntityString(const char** data, SpawnKeyValues &parsedKeyValues) {
	// False until proven otherwise.
	qboolean parsedSuccessfully = false;

	// Key value ptrs.
	char *key = nullptr, *value = nullptr;

	// Go through all the dictionary pairs.
  	while (1) {
		// Parse the key.
		key = COM_Parse(data);
		
		// If we hit a }, it means we're done parsing, break out of this loop.
		if (key[0] == '}') {
			break;
		}

		// If we are at the end of the string without a closing brace, error out.
		if (!*data) {
			Com_Error(ErrorType::Drop, "%s: EOF without closing brace", __func__);
			return false;
		}

		// Parse the value.
		value = COM_Parse(data);

		// If we are at the end of the string without a closing brace, error out.
		if (!*data) {
			Com_Error(ErrorType::Drop, "%s: EOF without closing brace", __func__);
			return false;
		}

		// Ensure we had a value.
		if (value[0] == '}') {
			Com_Error(ErrorType::Drop, "%s: closing brace without value for key %s", __func__, key);
			return false;
		   }

		// We successfully managed to parse this entity.
		parsedSuccessfully = true;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (key[0] == '_') {
			continue;
		}

		// Insert the key/value into the dictionary.
		parsedKeyValues.try_emplace(std::string(key),std::string(value));// = value;
	}

	// Return the result.
	return parsedSuccessfully;
}

/**
*   @brief  Allocates the game entity determined by the classname key, and
*           then does a precache before spawning the game entity.
**/
qboolean ServerGameWorld::CreateGameEntityFromDictionary(PODEntity *podEntity, SpawnKeyValues &dictionary) {

	// We do need a PODEntity of course.
	if (!podEntity) {
		gi.Error(ErrorType::Drop, "SVGame Error: Called %s with a nullptr PODEntity!\n", __func__);
		return false;
	}

	// Get state number for debug/error logging.
    int32_t stateNumber = podEntity->currentState.number;

	// It needs the classname key, as well as it needs to have a value for it, how else can we spawn a game entity?
    if (!dictionary.contains("classname") || dictionary["classname"].empty()) {
		// For the server game we error out in this case, it can't go on since it is the actual game master.
		gi.Error(ErrorType::Drop, "SVGame Error: Can't spawn ServerGameEntity for PODEntity(#%i) due to a missing 'classname' key/value.\n", stateNumber);
		return false;
    }

	// Actually spawn the game entity.
    IServerGameEntity *gameEntity = CreateGameEntityFromClassname(podEntity, dictionary["classname"]);

    // Something went wrong with allocating the game entity.
    if (!gameEntity) {
		// Free/reset the PODEntity for reusal.
		//FreePODEntity(podEntity);
		gi.DPrintf("SVGWarning: Spawning entity(%s) failed.\n", dictionary["classname"].c_str());
		return false;
    }

	// Assign game entity to POD entity.
	podEntity->gameEntity = gameEntity;

    // Initialise the entity with its respected keyvalue properties
    for (const auto& keyValueEntry : dictionary) {
		gameEntity->SpawnKey(keyValueEntry.first, keyValueEntry.second);
	}

	// Success.
	return true;
}

/**
*	@brief	Seeks through the type info system for a class registered under the classname string.
*			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
*			try and allocate it.
*	@return	nullptr in case of failure, a valid pointer to a game entity otherwise.
**/
GameEntity *ServerGameWorld::CreateGameEntityFromClassname(PODEntity *podEntity, const std::string &classname) {
    // Start with a nice nullptr.
    GameEntity* spawnEntity = nullptr;

	// Safety check.
    if (!podEntity) {
		return nullptr;
    }

    // Get entity state number.
    int32_t stateNumber = podEntity->currentState.number;

	// Warn if a slot is already occupied.
    if (gameEntities[stateNumber] != nullptr) {
		// Warn.
		gi.DPrintf("SVGWarning: trying to allocate game entity '%s' the slot #%i was pre-occupied.\n");

		// Return nullptr.
		return nullptr;
    }

    // New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByMapName(classname.c_str());
    if (info == nullptr) {
		// Then try finding it by the C++ class name
		if ((info = TypeInfo::GetInfoByName(classname.c_str())) == nullptr) {
			// Warn.
		    gi.DPrintf("SVGWarning: unknown entity '%s'\n", classname.c_str());

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && info->IsMapSpawnable()) {
		// Allocate and return out new game entity.
		return (gameEntities[stateNumber] = info->AllocateInstance(podEntity));
    } else {
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			gi.DPrintf("SVGWarning: tried to allocate an abstract class '%s'\n", info->classname);
		} else if (!info->IsMapSpawnable()) {
		    gi.DPrintf("SVGWarning: tried to allocate a code-only class '%s'\n", info->classname);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}

/**
*   @brief  Frees the given pod entity and its game class object entity 
*			rendering this entity to be used for recycling.
*
*   @return A pointer to the game entity on success, nullptr on failure.
**/
void ServerGameWorld::FreePODEntity(PODEntity* podEntity) {
    // Sanity check.
    if (!podEntity) {
		return;
    }

    // Unlink entity from world.
    gi.UnlinkEntity(podEntity);

    // Get entity number.
    int32_t entityNumber = podEntity->currentState.number;

    // Prevent freeing "special edicts". Clients, and the dead "client body queue".
    if (entityNumber <= game.GetMaxClients() + BODY_QUEUE_SIZE) {
		gi.DPrintf("Tried to free special edict: %i\n", entityNumber);
		return;
    }

    // Delete the actual entity class pointer.
    FreeGameEntity(podEntity);

    // Reset the entities values.
    *podEntity = {
		.inUse = false,
		.serverFlags = 0,
		.gameEntity = nullptr,
		.freeTime = level.time,
	};

    //// Store the freeTime, so we can prevent allocating a new entity with this ID too soon.
    //// If we don't, we risk the chance of a client lerping between an older entity that
    //// was taking up this current slot.
    //podEntity->freeTime = level.time;

    //// Last but not least, since it isn't in use anymore, let it be known.
    //podEntity->inUse = false;

    //// Reset serverFlags.
    //podEntity->serverFlags = 0;

    //// Ensure the game entity is nullified.
    //podEntity->gameEntity = nullptr;
}

/**
*   @brief  Frees the given game entity class object.
*
*   @return True on success, false on failure.
**/
qboolean ServerGameWorld::FreeGameEntity(PODEntity* podEntity) {
    // Sanity check.
    if (!podEntity) {
		gi.DPrintf("SVGWarning: tried to %s on a nullptr PODEntity!\n", __func__);
		return false;
    }

    // Used as return value.
    qboolean freedGameEntity = false;

    // Fetch entity number.
    int32_t entityNumber = podEntity->currentState.number;

    // Special game entity handling IF it still has one.
    if (podEntity->gameEntity) {
		// Get pointer to game entity.
		IServerGameEntity* gameEntity = static_cast<IServerGameEntity*>(podEntity->gameEntity);

		// Remove the gameEntity reference
		gameEntity->SetGroundEntity( SGEntityHandle() );
		gameEntity->SetLinkCount(0);
		gameEntity->SetGroundEntityLinkCount(0);
		gameEntity->SetPODEntity(nullptr);

		// Reset POD entity's game entity pointer.
		podEntity->gameEntity = nullptr;
    }

    // For whichever faulty reason the entity might not have had a classentity,
    // so we do an extra delete here just in case.
    if (gameEntities[entityNumber] != nullptr) {
		// Free it.
		delete gameEntities[entityNumber];
		gameEntities[entityNumber] = nullptr;

		// Freed game entity.
		freedGameEntity = true;
    }

    // Return result.
    return freedGameEntity;
}

/**
*   @brief	Utility function so we can acquire a valid entity pointer. It operates
*			by using an entity handle in order to make sure that it has a valid
*			server and game entity object.
*	@param	requireValidClient	Expands the check to make sure the entity's client isn't set to nullptr.
*	@param	requireInUse		Expands the check to make sure the entity has its inUse set to true.
* 
*   @return A valid pointer to the entity game entity. nullptr on failure.
**/
IServerGameEntity* ServerGameWorld::ValidateEntity(const SGEntityHandle &entityHandle, bool requireClient, bool requireInUse) {
	// Ensure the handle is valid.
	if (!entityHandle || 
			!(
				// Check for non nullptr client pointer.
				( requireClient == true ? ( entityHandle.Get()->client != nullptr ? true : false ) : true ) &&
				// Check for inUse.
				( requireInUse == true ? entityHandle.Get()->inUse : true )  
			)
		)
	{
		return nullptr;
	}

	// It's lame, but without requiring a const entity handle, we can't pass
	// in any pointers to (server-)entity types.
	return (* const_cast< SGEntityHandle& >( entityHandle ) );

	//   // Ensure the entity is valid.
 //   if (!*entityHandle || (!entityHandle.Get() ||
	//		(
	//			!(requireClient == true ? (entityHandle.Get()->client != nullptr ? true : false) : true) 
	//			&& !(requireInUse == true ? entityHandle.Get()->inUse : true)) 
	//		)
	//	)
	//{
	//	return nullptr;
 //   }

 //   // It's lame, but without requiring a const entity handle, we can't pass
	//// in any pointers to (server-)entity types.
	//SGEntityHandle castHandle = static_cast<SGEntityHandle>(entityHandle);
	//
	//// Ensure it is of class type player.
 //   if (!castHandle->IsSubclassOf<SVGBasePlayer>()) {
	//	return nullptr;
 //   }

 //   // We've got a definite valid player entity here. Return it.
 //   return dynamic_cast<SVGBasePlayer*>(*castHandle);
}



/**
*   @brief  Spawns a debris model entity at the given origin.
*   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
**/
void ServerGameWorld::ThrowDebris(GameEntity* debrisser, const int32_t debrisModelIndex, const vec3_t& origin, float speed) { 
	//DebrisEntity::Create(debrisser, gibModel, origin, speed); 
	gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    gi.MSG_WriteUint8(TempEntityEvent::DebrisGib);//WriteByte(TempEntityEvent::Blood);
    gi.MSG_WriteUint16(debrisser->GetNumber());
	// Write a debris model index.
	gi.MSG_WriteUint8(debrisModelIndex);
	// We subtract the debrisser->origin from the spawn origin of debris to
	// wire it as a half-float.
	
	// Write out vector offset results as byte encoded floats, yugghh :-)
	//const vec3_t offsetScale = { 64.f, 64.f, 64.f };
	const vec3_t debrisOffset = (debrisser->GetOrigin() - origin);
	const vec3_t wireOffset = debrisOffset;// * offsetScale;
	gi.MSG_WriteInt8(wireOffset.x);
	gi.MSG_WriteInt8(wireOffset.y);
	gi.MSG_WriteInt8(wireOffset.z);

	//gi.MSG_WriteVector3(debrisser->GetOrigin() - origin, true);
	// Speed will in most cases be a float of 0 to 2. So encode it as an Uint8.
	gi.MSG_WriteUint8(speed * 255);
    gi.Multicast(debrisser->GetOrigin(), Multicast::PVS);
}

/**
*   @brief  Spawns a gib model entity flying at random velocities and directions.
*   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
*/
void ServerGameWorld::ThrowGib(GameEntity* gibber, const std::string& gibModel, uint32_t count, int32_t damage, int32_t gibType) { 
	//GibEntity::Create(gibber, gibModel, damage, gibType);
	if (!gibber) {
		gi.DPrintf("SVGWarning: (%s): *gibber is (nullptr)!\n", __func__);
		return;
	}
    gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte(ServerGameCommand::TempEntityEvent);
    gi.MSG_WriteUint8(TempEntityEvent::BodyGib);//WriteByte(TempEntityEvent::Blood);
    gi.MSG_WriteUint16(gibber->GetNumber());
	//gi.MSG_WriteVector3(gibber->GetVelocity(), false);
    gi.MSG_WriteUint8(count);
    gi.Multicast(gibber->GetOrigin(), Multicast::PVS);
}