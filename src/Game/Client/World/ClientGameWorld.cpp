/***
*
*	License here.
*
*	@file
*
***/
// Shared.
#include "../../Shared/SharedGame.h"

// Core.
#include "../ClientGameLocals.h"

// Exports.
#include "../Exports/Entities.h"

// Entities.
#include "../Entities/Base/CLGBasePlayer.h"
#include "../Entities/DebrisEntity.h"
#include "../Entities/GibEntity.h"

// GameModes.
//#include "../GameModes/IGameMode.h"
//#include "../GameModes/DefaultGameMode.h"
//#include "../GameModes/CoopGameMode.h"
//#include "../GameModes/DeathMatchGameMode.h"

// GameWorld.
#include "ClientGameWorld.h"

// Cvars.
//extern cvar_t *gamemode;


/**
*	@brief Initializes the gameworld and its member objects.
***/
void ClientGameWorld::Initialize() {
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
void ClientGameWorld::Shutdown() {
	DestroyGameMode();
}



/**
*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
**/
void ClientGameWorld::SetupGameMode() {
	//// Fetch gamemode as std::string
	//std::string gamemodeStr = gamemode->string;

	//// Detect which game ode to allocate for this game round.
	//if (gamemodeStr == "deathmatch") {
	//	// Deathmatch gameplay mode.
	//    currentGameMode = new DeathmatchGameMode();
	//} else if (gamemodeStr == "coop") {
	//	// Cooperative gameplay mode.
	//    currentGameMode = new CoopGameMode();
	//} else {
	//	// Acts as a singleplayer game mode.
	//    currentGameMode = new DefaultGameMode();
	//}
}

/**
*	@brief	Destroys the current gamemode object.
**/
void ClientGameWorld::DestroyGameMode() {
	//// Always make sure it is valid to begin with.
 //   if (currentGameMode != nullptr) {
	//	delete currentGameMode;
	//	currentGameMode = nullptr;
	//}
}



/**
*   @brief	Assigns the exported globals.entities and ensures that all class entities 
*			are set to nullptr. Also does a clamp on maxEntities for sanity.
**/
void ClientGameWorld::PrepareEntities() {
    // 1024 for wired entities, and 3072 more for... whichever.
    gameEntities.resize(MAX_CLIENT_POD_ENTITIES);

    // To ensure that slot 0 is in use, keeps indexes correct.
//    gameEntities.push_back(nullptr);
 //   // Clamp it just in case.
//   int32_t maxEntities = Clampi(MAX_WIRED_POD_ENTITIES, (int)maximumclients->value + 1, MAX_WIRED_POD_ENTITIES);

 //   // Setup the globals entities pointer and max entities value so
	//// that the server can access it.
 //   globals.entities = podEntities;
 //   globals.maxEntities = this->maxEntities = maxEntities;

 //   // Ensure, all base entities are nullptrs. Just to be save.
	//for (int32_t i = 0; i < MAX_CLIENT_POD_ENTITIES; i++) {
	//	gameEntities[i] = nullptr;
	//}

 //   // Delete class entities if they are allocated, and reset the client entity to a zero state.
    for (int32_t i = 0; i < MAX_POD_ENTITIES; i++) {
		// Store fields we want to keep from the PODEntity.
		PODEntity backupPODEntity = podEntities[i];

		// Delete game entity.
		if (gameEntities.size() > i && gameEntities[i]) {
			// Notify about deallocation.
			gameEntities[i]->OnDeallocate();

			// Clean memory.
		    delete gameEntities[i];
			gameEntities[i] = nullptr;
		}

		// Reset client PODEntity to a fresh state.
		const bool isLocal = (i > MAX_WIRED_POD_ENTITIES ? true : false);
		podEntities[i] = {
			.currentState = {
				.number = i,	// We want to ensure its currentState number matches the index.
			},
			.previousState = {
				.number = i		// Same for previousState number.
			},
			.isLocal = isLocal,
			.inUse = false,
			.clientEntityNumber = i, // Last but not least, the actual clientEntityNumber.
		};
    }
}

/**
*   @brief Prepares the game clients array for use.
**/
void ClientGameWorld::PrepareClients() {
    // Allocate our clients array.
	cvar_t *maximumclients = clgi.Cvar_Get("maxclients", nullptr, 0);
	
	if (maximumclients && maximumclients->integer) {
		maxClients = maximumclients->integer;
		Com_DPrint("MAXIMUM CLIENTS YOOOO DAWG================%i\n", maximumclients->integer);
	}

    //maxClients = maximumclients->value;
    clients = (ServerClient*)clgi.Z_TagMalloc(maxClients * sizeof(clients[0]), memtag_t::TAG_GENERAL);  // CPP: Cast

    // Current total number of entities in our game = world + maximum clients.
    numberOfEntities = maxClients + 1;
	//globals.numberOfEntities = numberOfEntities;
}

/**
*	@brief Prepares the game's client entities with a base player game entity.
**/
void ClientGameWorld::PreparePlayers() {
	// Allocate a classentity for each client in existence.
	for (int32_t i = 1; i < maxClients + 1; i++) {//maxClients + 1; i++) {
		//const int32_t entityNumber = i;

		// Acquire POD entity.
		PODEntity *podEntity = GetPODEntityByIndex(i);

		// Setup the entity.
		(*podEntity) = {
			.currentState {
				.number = i,
			},
			.previousState = {
				.number = i,
			},
			.isLocal = false,
			.inUse = true,
			.gameEntity = CreateGameEntity<CLGBasePacketEntity>(podEntity, false, true), //CreateGameEntityFromClassname(podEntity, "CLGBasePacketEntity"),
			.clientEntityNumber = i,
		};
		
		//// We can fetch the number based on subtracting these two pointers.
		//podEntity->clientEntityNumber = podEntity - podEntities;
		//podEntity->currentState.number = podEntity->clientEntityNumber;

		//// Allocate player client game entity
		//CLGBasePlayer* playerClientEntity = CreateGameEntity<CLGBasePlayer>(podEntity, false);

		//if (playerClientEntity) {
		//	// Be sure to reset their inuse, after all, they aren't in use.
		//	playerClientEntity->SetInUse(false);

		//	// Fetch client index.
		//	const int32_t clientIndex = i - 1;  // Same as the older: podEntities - podEntities - 1;

		//	// Assign the designated client to this SVGBasePlayer entity.
		//	//playerClientEntity->SetClient(&clients[clientIndex]);
		//} else {
		//	Com_DPrint("CLGWarning: ClientGameWorld failed to prepare playerClientEntity(#%i)\n", entityNumber);
		//}
	}

	//// Prepare body queue.
	//for (int32_t i = maxClients; i <= maxClients + BODY_QUEUE_SIZE; i++) {//maxClients + 1; i++) {
	//	const int32_t entityNumber = i + 1;

	//	// Acquire POD entity.
	//	PODEntity *podEntity = GetPODEntityByIndex(entityNumber);

	//	podEntity->currentState.number = entityNumber;
	//	podEntity->previousState.number = entityNumber;
	//	podEntity->clientEntityNumber = entityNumber;
	//	podEntity->gameEntity = nullptr; // TODO: ??

	//	// Create a CLGBasePacketEntity.
	//	podEntity->gameEntity = CreateGameEntityFromClassname(podEntity, "CLGBasePacketEntity");//CreateGameEntity<CLGBasePacketEntity>(podEntity, false);

	//	// They aren't in use of course, and they come from the server.
	//	podEntity->inUse = false;
	//	podEntity->isLocal = false;
	//}


		//// Allocate player client game entity
		//SVGBasePlayer* playerClientEntity = CreateGameEntity<SVGBasePlayer>(podEntity, false);

		//// Be sure to reset their inuse, after all, they aren't in use.
		//playerClientEntity->SetInUse(false);

		//// Fetch client index.
		//const int32_t clientIndex = i - 1;  // Same as the older: podEntities - podEntities - 1;

		//// Assign the designated client to this SVGBasePlayer entity.
		//playerClientEntity->SetClient(&clients[clientIndex]);
  //  }
}

/**
*	@brief	Reserves the game's body queue entity slots.
**/
void ClientGameWorld::PrepareBodyQueue() {
    // Reserve some spots for dead player bodies for coop / deathmatch
    //level.bodyQue = 0;
	int32_t startRange = maxClients + 1;
	int32_t endRange = startRange + BODY_QUEUE_SIZE;
    for (int32_t entityNumber = startRange; entityNumber < endRange; entityNumber++) {

		// Acquire POD entity.
		PODEntity *podEntity = GetPODEntityByIndex(entityNumber);

		// Setup the entity.
		(*podEntity) = {
			.currentState {
				.number = entityNumber,
			},
			.previousState = {
				.number = entityNumber,
			},
			.isLocal = false,
			.inUse = true,
			.gameEntity = CreateGameEntity<CLGBasePacketEntity>(podEntity, false, true),//CreateGameEntityFromClassname(podEntity, "CLGBasePacketEntity"),
			.clientEntityNumber = entityNumber,
		};
    }
}

/**
*	@brief	Parses the 'entities' string and assigns each parsed entity to the
*			first free client entity slot there is. After doing so, allocates
*			a game entity based on the 'classname' of the parsed entity.
**/
qboolean ClientGameWorld::PrepareBSPEntities(const char* mapName, const char* bspString, const char* spawnpoint) {
	// Clear level state.
    level = {
		// Mapname.
		.mapName = mapName,
	};

	// Occupy player entity slots for future use.
    PreparePlayers();

	// Reserve dead body queue slots.
	PrepareBodyQueue();

    // Parsing state variables.
	qboolean isParsing = true; // We'll keep on parsing until this is set to false.
	qboolean parsedSuccessfully = false;// This gets set to false the immediate moment we run into parsing trouble.
	char *com_token = nullptr; // Token pointer.
	PODEntity *clientEntity = nullptr; // Pointer to the client entity we intend to employ.
    uint32_t packetEntityIndex = maxClients + 1 + BODY_QUEUE_SIZE; // We start from the max clients.         
	uint32_t localEntityIndex = MAX_WIRED_POD_ENTITIES + RESERVED_ENTITIY_COUNT; // TODO: That RESERVED_COUNT thingy.

	// Acquire gameworld and entity arrays.
	ClientGameWorld *gameWorld = GetGameWorld();

	//// First 3 reserved entities.
	for (int i = MAX_WIRED_POD_ENTITIES; i < MAX_WIRED_POD_ENTITIES + RESERVED_ENTITIY_COUNT; i++) {
	//	PODEntity *reservedPODEntity = GetUnusedPODEntity(false);
	//	
	//	// Create a base local entity for these reserved ones for now.
	//	auto *gameEntity = CreateGameEntity<CLGBaseLocalEntity>(reservedPODEntity, false, false);
	//	gameEntity->SetInUse(false);

	//	//// Acquire POD entity.
		PODEntity *podEntity = GetPODEntityByIndex(i);

		// Setup the entity.
		(*podEntity) = {
			.currentState {
				.number = i,
			},
			.previousState = {
				.number = i,
			},
			.isLocal = false,
			.inUse = false,
			//.gameEntity = CreateGameEntityFromClassname(podEntity, "CLGBaseLocalEntity"),
			.gameEntity = nullptr,
			.clientEntityNumber = i,
		};
	} 

	// Engage parsing.
	while (!!isParsing == true) {
		// Parse the opening brace.
		com_token = COM_Parse(&bspString);

        // Break out when we're done and there is no string data left to parse.
		if (!bspString) {
			break;
		}

        // If the token isn't a {, something is off.
		if (com_token[0] != '{') {
		    Com_Error(ErrorType::Drop, "PrepareBSPEntities: found %s when expecting {", com_token);
			return false;
		}

		// SpawnKeys.
		SpawnKeyValues parsedKeyValues;
        parsedSuccessfully = ParseEntityString(&bspString, parsedKeyValues);

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
			podEntity = GetUnusedPODEntity(false);

			if (!podEntity) {
				Com_DPrint("WTF IS THIS SHIT?\n");
			}
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

		// Acquire a POD Entity.
		//PODEntity *podEntity = GetPODEntityByIndex(entityIndex);

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

	//// Initialize player trail...
	//// SVG_PlayerTrail_Init

	return parsedSuccessfully;
}

/**
*	@brief	Looks for the first free client entity in our buffer.
* 
*	@details	Either finds a free client entity, or initializes a new one.
*				Try to avoid reusing an entity that was recently freed, because it
*				can cause the client to Think the entity morphed into something else
*				instead of being removed and recreated, which can cause interpolated
*				angles and bad trails.
* 
*	@return	If successful, a valid pointer to the entity. If not, a nullptr.
**/
PODEntity* ClientGameWorld::GetUnusedPODEntity(bool isWired) { 
	// We'll loop until from maxclients + 1(world entity) till the numberOfEntities 
	// has been reached. If we never managed to return a pointer to a valid server 
	// entity right now, we're going to have to increase the amount of entities in use. 
	// 
	// However, this ONLY proceeds if we haven't already hit the maximum entity count.
	int32_t rangeStart		= (isWired ? (maxClients + 1) + BODY_QUEUE_SIZE : MAX_WIRED_POD_ENTITIES + RESERVED_ENTITIY_COUNT);
	int32_t rangeMaximum	= (isWired ? MAX_WIRED_POD_ENTITIES : MAX_CLIENT_POD_ENTITIES);

	// Loop through the range to find a free unused POD Entity.
	int32_t podEntityIndex = 0;
	for (int32_t podEntityIndex = rangeStart; podEntityIndex < rangeMaximum + 1; podEntityIndex++) {
		// Fetch POD Entity.
		PODEntity *podEntity = GetPODEntityByIndex(podEntityIndex);

        // The first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
	    if (!podEntity->inUse && (podEntity->freeTime == GameTime::zero() || podEntity->freeTime < FRAMETIME_S * 2 || level.time - podEntity->freeTime > 500ms)) {
            // Set entity to "inUse".
			podEntity->inUse = true;
			
			// Ensure the entity is set to be local/non-local.
			podEntity->isLocal = (isWired ? false : true);

			// Ensure the entity number is set correctly.
			podEntity->clientEntityNumber = podEntityIndex;
			podEntity->currentState.number = podEntityIndex;//podEntity - podEntities;
			// If it is a local entity...
			if (!isWired) {
				podEntity->previousState.number = podEntityIndex;
			}
			
			if (podEntityIndex >= numberOfEntities) {
				numberOfEntities = podEntityIndex;
			}

			// Return the newly found POD entity pointer.
			return podEntity;
        }
    }

	// Do a safety check to prevent crossing maximum entity limit. If we do, error out.
    if (podEntityIndex >= (isWired ? MAX_WIRED_POD_ENTITIES : MAX_CLIENT_POD_ENTITIES)) {// maxEntities) {
        Com_Error( ErrorType::Drop, "ClientGameWorld::GetUnusedPODEntity: no free edicts");
		return nullptr;
	}

	return nullptr;
 //   // Incrementor, declared here so we can access it later on.
	//int32_t i = 0;

	//// Acquire a pointer to the first client entity to start checking from.
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
	//		podEntity->state.number = podEntity - podEntities;
	//		
	//		// Return the newly found client entity pointer.
	//		return podEntity;
 //       }
 //   }

	//// Do a safety check to prevent crossing maximum entity limit. If we do, error out.
 //   if (i >= maxEntities) {
 //       gi.Error("ClientGameWorld::GetUnusedPODEntity: no free edicts");
	//	return nullptr;
	//}

 //   // If we've gotten past the gi.Error, it means we can safely increase the number of entities.
 //   numberOfEntities++;
	//globals.numberOfEntities = numberOfEntities;

 //   // Set entity to "inUse".
 //   podEntity->inUse = true;

 //   // Set the entity state number.
 //   podEntity->state.number = podEntity - podEntities;

 //   // Return the client entity.
 //   return podEntity;
	return nullptr;
}

/**
*   @brief Chain together all entities with a matching team field
* 
*   @details    All but the first will have the EntityFlags::TeamSlave flag set.
*               All but the last will have the teamchain field set to the next one.
**/
void ClientGameWorld::FindTeams() {
    PODEntity *podEntityA = nullptr;
	PODEntity *podEntityB = nullptr;
    GameEntity *chain = nullptr;
    int32_t i, j;

    int32_t c = 0;
    int32_t c2 = 0;
    for (i = 1, podEntityA = podEntities + i; i < numberOfEntities; i++, podEntityA++) {
        // Fetch game entity.
        GameEntity *gameEntityA = gameEntities[podEntityA->clientEntityNumber];

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

        for (j = i + 1, podEntityB = podEntityA + 1 ; j < numberOfEntities ; j++, podEntityB++) {
            // Fetch game entity.
            GameEntity* gameEntityB = gameEntities[podEntityA->clientEntityNumber];

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

    Com_DPrintf("ClientGameWorld: Found (#%i) Teams with (#%i) of total Entities.\n", c, c2);
}

/**
*	@brief	Parses the BSP Entity string and places the results in the server
*			entity dictionary.
**/
qboolean ClientGameWorld::ParseEntityString(const char** data, SpawnKeyValues &parsedKeyValues) {
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
qboolean ClientGameWorld::CreateGameEntityFromDictionary(PODEntity *podEntity, SpawnKeyValues &dictionary) {
    // Get client side entity number.
    const int32_t clientEntityNumber = podEntity->clientEntityNumber;

    // If it does not have a classname key we're in for trouble.
    if (!dictionary.contains("classname") || dictionary["classname"].empty()) {
		// For the ClientGame we only do some simple warning print.
		Com_WPrint("CLGWarning: Can't spawn ClientGameEntity for PODEntity(#%i) due to a missing 'classname' key/value.\n", __func__, clientEntityNumber);
		return false;
    }

    // Actually spawn the game entity.
    IClientGameEntity *gameEntity = CreateGameEntityFromClassname(podEntity, dictionary["classname"]);
	
    // This only happens if something went badly wrong. (It shouldn't.)
    if (!gameEntity) {
		// Reset the client entity.
//		*clEntity = { .clientEntityNumber = stateNumber };//FreeClientEntity(clEntity);
		(*podEntity) = {
			.currentState = {
				.number = clientEntityNumber,
			},
			.previousState = {
				.number = clientEntityNumber,
			},
			.gameEntity = nullptr,
			.clientEntityNumber = clientEntityNumber,
		};
		//podEntity->clientEntityNumber = stateNumber;

		// Failed.
		Com_DPrint("CLGWarning: Spawning entity(%s) failed.\n", dictionary["classname"].c_str());
		return false;
	}
	
	// Assign game entity to POD entity.
	podEntity->gameEntity = gameEntity;

    // Initialise the entity with its respected keyvalue properties
    for (const auto& keyValueEntry : dictionary) {
		gameEntity->SpawnKey(keyValueEntry.first, keyValueEntry.second);
	}
	
	// Spawned.
	return true;
}

/**
*	@brief	Seeks through the type info system for a class registered under the classname string.
*			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
*			try and allocate it.
*	@return	nullptr in case of failure, a valid pointer to a game entity otherwise.
**/
GameEntity *ClientGameWorld::CreateGameEntityFromClassname(PODEntity *podEntity, const std::string &classname) {
	// Start with a nice nullptr.
    GameEntity* spawnEntity = nullptr;

	// Safety check.
    if (!podEntity) {
		return nullptr;
    }

    // Get entity state number.
    int32_t stateNumber = podEntity->clientEntityNumber;

	// Warn if a slot is already occupied.
    if (gameEntities[stateNumber] != nullptr) {
		// Warn.
		Com_DPrint("CLGWarning: trying to allocate game entity '%s' the slot #%i was pre-occupied.\n", classname.c_str(), stateNumber);

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
		    Com_DPrint("CLGWarning: unknown entity '%s'\n", classname.c_str());

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
			Com_DPrint("CLGWarning: tried to allocate an abstract class '%s'\n", info->classname);
		} else if (!info->IsMapSpawnable()) {
		    Com_DPrint("CLGWarning: %s tried to allocate a code-only class '%s'\n", __func__, info->classname);
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
void ClientGameWorld::FreePODEntity(PODEntity* podEntity) {
    // Sanity check.
    if (!podEntity) {
		return;
    }

    // Unlink entity from world.
    //gi.UnlinkEntity(podEntity);

    // Get entity number.
    int32_t entityNumber = podEntity->clientEntityNumber;

    // Prevent freeing "special edicts". Clients, and the dead "client body queue".
	static constexpr int32_t BODY_QUEUE_SIZE = 8;
    if (entityNumber <= game.GetMaxClients() + BODY_QUEUE_SIZE) {
		Com_DPrint("Tried to free special edict: %i\n", entityNumber);
		return;
    }

    // Delete the actual entity class pointer.
    FreeGameEntity(podEntity);

    // Reset the entities values.
	(*podEntity) = {
		// Ensure the states are assigned the correct entityNumber.
		.currentState = {
			.number = entityNumber,
		},
		.previousState = {
			.number = entityNumber,
		},
		// It has no Game Entity anymore.
		.gameEntity = nullptr,
		//// Store the freeTime, so we can prevent allocating a new entity with this ID too soon.
		//// If we don't, we risk the chance of a client lerping between an older entity that
		//// was taking up this current slot.
		.freeTime = level.time,
		// And ensure our main identifier has the correct entityNumber set.
		.clientEntityNumber = entityNumber,
	};


    //podEntity->freeTime = level.time;
    //podEntity->gameEntity = nullptr;
}

/**
*   @brief  Frees the given game entity class object.
*
*   @return True on success, false on failure.
**/
qboolean ClientGameWorld::FreeGameEntity(PODEntity* podEntity) {
    // Sanity check.
    if (!podEntity) {
		Com_DPrint("CLGWarning: tried to %s on a nullptr PODEntity!\n", __func__);
		return false;
    }

    // Used as return value.
    qboolean freedGameEntity = false;

    // Fetch entity number.
    int32_t entityNumber = podEntity->clientEntityNumber;

    // If it has a pointer to a game entity, we use that instead.
    if (podEntity->gameEntity) {
		// Get pointer to game entity.
		IClientGameEntity* gameEntity = static_cast<IClientGameEntity*>(podEntity->gameEntity);

		// Remove the gameEntity reference
		gameEntity->SetLinkCount(0);
		gameEntity->SetGroundEntityLinkCount(0);
		gameEntity->SetGroundEntity(nullptr);
		gameEntity->SetPODEntity(nullptr);

		// Reset POD entity's game entity pointer.
		podEntity->gameEntity = nullptr;

		// Delete game entity.
		delete gameEntities[entityNumber];
		gameEntities[entityNumber] = nullptr;

		// Freed game entity.
		freedGameEntity = true;
    }

    // If however for whichever reason the entity might not have had a classentity pointer set..
	// We make sure to do an extra delete here by fetching it in the game entities array instead.
    if (gameEntities[entityNumber] != nullptr) {
		// Unset important parts.
		GameEntity *gameEntity = gameEntities[entityNumber];
		gameEntity->SetLinkCount(0);
		gameEntity->SetGroundEntityLinkCount(0);
		gameEntity->SetGroundEntity(nullptr);
		gameEntity->SetPODEntity(nullptr);

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
*   @brief  Creates a new GameEntity for the ClientEntity based on the passed state. If the 
*			hashedClassname values are identical it'll call its UpdateFromState on the already
*			instanced object.
*
*   @return On success: A pointer to the ClientEntity's GameEntity, which may be newly allocated. On failure: A nullptr.
**/
GameEntity* ClientGameWorld::UpdateGameEntityFromState(const EntityState& state, PODEntity* clEntity) {
    // Start with a nice nullptr.
    IClientGameEntity* spawnEntity = nullptr;

	// Safety check.
    if (!clEntity) {
		return nullptr;
    }

    // Get entity state number.
    const int32_t stateNumber = state.number;

	// Acquire hashedClassname.
	const uint32_t currentHashedClassname = state.hashedClassname;

	// If the previous and current entity number and game entity classname hash are a match, 
	// update the current entity from state instead.
	if (clEntity->gameEntity && stateNumber == clEntity->clientEntityNumber) {
		uint32_t previousHashedClassname = clEntity->previousState.hashedClassname;
	
		if (currentHashedClassname == previousHashedClassname) {
			// Acquire a pointer to the already in-place game entity instead of allocating a new one.
			clEntity->gameEntity = GetGameEntityByIndex(stateNumber);

			// Update it based on state and return its pointer.
			if (clEntity->gameEntity) {
				static_cast<IClientGameEntity*>(clEntity->gameEntity)->UpdateFromState(state);
			} else {
				// Do nothing. 
				Com_DPrint("CLG (%s): PODEntity(%i) had a (nullptr) GameEntity\n", __func__, state.number, clEntity->clientEntityNumber);
			}

			return static_cast<IClientGameEntity*>(clEntity->gameEntity);
		}
	} else {
		// Debug Print.
		//if (state.number != clEntity->clientEntityNumber) {
		//	Com_DPrint("CLG (%s): state.number(%i) mismatched clEntity->clientEntityNumber(%i)\n", __func__, state.number, clEntity->clientEntityNumber);
		//}
	}

    // New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByHashedMapName(currentHashedClassname);
    if (info == nullptr) {
		// Then try finding it by the C++ class name. (USE THIS FOR SPAWNING BSP STRING ENTITIES.)
		//if ((info = TypeInfo::GetInfoByName(classname.c_str())) == nullptr) {
		// 
		if ((info = TypeInfo::GetInfoByName("CLGBasePacketEntity")) == nullptr) {
			// Warn.
		    Com_DPrint("CLG (%s): info = TypeInfo::GetInfoByName(\"CLGBasePacketEntity\")) == nullptr\n");

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && (info->IsMapSpawnable() || info->IsGameSpawnable())) {
		// Allocate and return a pointer to the new game entity object.
		clEntity->gameEntity = gameEntities[state.number] = info->AllocateInstance(clEntity); //InsertAt(state.number, info->AllocateInstance(clEntity));

		// Spawn if needed.
		if (clEntity->gameEntity) {
			// TODO: Use a SpawnFromState function here instead.
			clEntity->gameEntity->Spawn();
		}

		// If it isn't a nullptr...
		if (!clEntity->gameEntity ) {
			// Inform us about what entity failed exactly.
			const char *classname = (info->IsMapSpawnable() ? info->mapClass : info->classname);
			Com_DPrint("CLG (%s): Failed to spawn GameEntity(classname: '%s' (HASH: #%ui) for PODEntity(%i)\n", __func__, classname, currentHashedClassname, clEntity->clientEntityNumber);
			return nullptr;
			//classEntity = new CLGBasePacketEntity(clEntity);
		}

		// Update its current state.
		static_cast<IClientGameEntity*>(clEntity->gameEntity)->UpdateFromState(state);

		// Return game entity.
		return static_cast<IClientGameEntity*>(clEntity->gameEntity);
    } else {
		// Inform us about what entity failed exactly.
		const char *classname = (info->IsMapSpawnable() ? info->mapClass : info->classname);
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			Com_DPrint("CLG (%s): Tried to allocate an 'abstract class' GameEntity(classname: '%s' (HASH: #%ui) for PODEntity(%i)\n", __func__, classname, currentHashedClassname, clEntity->clientEntityNumber);
		} else if (!info->IsMapSpawnable()) {
		    Com_DPrint("CLG (%s): Tried to allocate a 'code-only' GameEntity(classname: '%s' (HASH: #%ui) for PODEntity(%i)\n", __func__, classname, currentHashedClassname, clEntity->clientEntityNumber);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}

/**
*   @brief  When the client receives state updates it calls into this function so we can update
*           the game entity belonging to the server side entity(defined by state.number).
* 
*           If the hashed classname differs, we allocate a new one instead. Also we ensure to 
*           always update its PODEntity pointer to the appropriate new one instead.
* 
*   @return True on success, false in case of trouble. (Should never happen, and if it does,
*           well... file an issue lmao.)
**/
qboolean ClientGameWorld::UpdateFromState(PODEntity *clEntity, const EntityState& state) {
    // Sanity check. Even though it shouldn't have reached this point of execution if the entity was nullptr.
    if (!clEntity) {
        // Developer warning.
        Com_DPrint("CLG (%s): Called with a clEntity(nullptr)!\n", __func__);

        return false;
    }

    
    // Depending on the state it will either return a pointer to a new classentity type, or to an already existing in place one.
    GameEntity *clgEntity = UpdateGameEntityFromState(state, clEntity);

	// Debug.
	if (!clgEntity) {
		Com_DPrint("CLG (%s): Had a (nullptr) returned from UpdateGameEntityFromState!\n", __func__);
	}

  //  // 
  //  if (clgEntity) {
		//clgEntity->Precache();
  //      clgEntity->Spawn();
  //  }
    // Do a debug print.
#ifdef _DEBUG
    if (clgEntity) {
        PODEntity *podEntity = clgEntity->GetPODEntity();

	    const char *mapClass = clgEntity->GetTypeInfo()->mapClass; // typeinfo->classname = C++ classname.
	    uint32_t hashedMapClass = clgEntity->GetTypeInfo()->hashedMapClass; // hashed mapClass.

        if (podEntity) {
    	    clgi.Com_LPrintf(PrintType::Warning, "CLG (%s): clEntNumber=%i, svEntNumber=%i, mapClass=%s, hashedMapClass=%i\n", __func__, podEntity->clientEntityNumber, state.number, mapClass, hashedMapClass);
        } else {
    	    clgi.Com_LPrintf(PrintType::Warning, "CLG (%s): clEntity=nullptr, svEntNumber=%i, mapClass=%s, hashedMapClass=%i\n", __func__, state.number, mapClass, hashedMapClass);
        }
    }
#endif

    return (clgEntity != nullptr ? true : false);
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
IClientGameEntity* ClientGameWorld::ValidateEntity(const SGEntityHandle &entityHandle, bool requireClient, bool requireInUse) {
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
	return nullptr;
}



/**
*   @brief  Spawns a debris model entity at the given origin.
*   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
**/
void ClientGameWorld::ThrowDebris(GameEntity* debrisser, const std::string &debrisModel, const vec3_t& origin, float speed) { 
	DebrisEntity::Create(debrisser, debrisModel, origin, speed); 
}

/**
*   @brief  Spawns a gib model entity flying at random velocities and directions.
*   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
*/
void ClientGameWorld::ThrowGib(const vec3_t &origin, const vec3_t &velocity, const std::string& gibModel, int32_t damage, int32_t gibType) { 
	GibEntity::Create(origin, velocity, gibModel, damage, gibType);
}