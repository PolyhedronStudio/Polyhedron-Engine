// Core.
#include "../ServerGameLocal.h"

// Entities.
#include "../Entities.h"
#include "../Entities/Base/SVGBasePlayer.h"

// Gamemodes.
#include "../Gamemodes/IGamemode.h"
#include "../Gamemodes/DefaultGamemode.h"
#include "../Gamemodes/CoopGamemode.h"
#include "../Gamemodes/DeathmatchGamemode.h"

// Gameworld.
#include "../World/GameWorld.h"

// Cvars.
extern cvar_t *gamemode;


/**
*	@brief Initializes the gameworld and its member objects.
***/
void Gameworld::Initialize() {
	// Prepare the items.
    PrepareItems();

	// Prepare the entities.
    PrepareEntities();

	// Prepare the clients.
    PrepareClients();

	// What game could one play without a gamemode?
	SetupGamemode();
}

/**
*	@brief Shutsdown the gameworld and its member objects.
**/
void Gameworld::Shutdown() {
	DestroyGamemode();
}



/**
*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
**/
void Gameworld::SetupGamemode() {
	// Fetch gamemode as std::string
	std::string gamemodeStr = gamemode->string;

	// Detect which game ode to allocate for this game round.
	if (gamemodeStr == "deathmatch") {
		// Deathmatch gameplay mode.
	    currentGamemode = new DeathmatchGamemode();
	} else if (gamemodeStr == "coop") {
		// Cooperative gameplay mode.
	    currentGamemode = new CoopGamemode();
	} else {
		// Acts as a singleplayer game mode.
	    currentGamemode = new DefaultGamemode();
	}
}

/**
*	@brief	Destroys the current gamemode object.
**/
void Gameworld::DestroyGamemode() {
	// Always make sure it is valid to begin with.
    if (currentGamemode != nullptr) {
		delete currentGamemode;
		currentGamemode = nullptr;
	}
}



/**
*   @brief	Assigns the exported globals.entities and ensures that all class entities 
*			are set to nullptr. Also does a clamp on maxEntities for sanity.
**/
void Gameworld::PrepareEntities() {
    // Clamp it just in case.
    int32_t maxEntities = Clampi(MAX_EDICTS, (int)maximumclients->value + 1, MAX_EDICTS);

    // Setup the globals entities pointer and max entities value so
	// that the server can access it.
    globals.entities = serverEntities;
    globals.maxEntities = this->maxEntities = maxEntities;

    // Ensure, all base entities are nullptrs. Just to be save.
    for (int32_t i = 0; i < MAX_EDICTS; i++) {
		classEntities[i] = nullptr;
    }
}

/**
*   @brief Prepares the game clients array for use.
**/
void Gameworld::PrepareClients() {
    // Allocate our clients array.
    maxClients = maximumclients->value;
    clients = (ServerClient*)gi.TagMalloc(maxClients * sizeof(clients[0]), TAG_GAME);  // CPP: Cast

    // Current total number of entities in our game = world + maximum clients.
    globals.numberOfEntities = numberOfEntities = maxClients + 1;
}

/**
*	@brief Prepares the game's client entities with a base player class entity.
**/
void Gameworld::PreparePlayers() {
    // Allocate a classentity for each client in existence.
    for (int32_t i = 1; i < maxClients + 1; i++) {
		// Fetch server entity.
		Entity* svEntity = &serverEntities[i];

		// We can fetch the number based on subtracting these two pointers.
		svEntity->state.number = svEntity - serverEntities;

		// Allocate player client class entity
		SVGBasePlayer* playerClientEntity = CreateClassEntity<SVGBasePlayer>(svEntity, false);	//SVG_SpawnClassEntity(serverEntity, serverEntity->classname);

		// Be sure to reset their inuse, after all, they aren't in use.
		playerClientEntity->SetInUse(false);

		// Fetch client index.
		const int32_t clientIndex = i - 1;  // Same as the older: serverEntity - g_entities - 1;

		// Assign the designated client to this SVGBasePlayer entity.
		playerClientEntity->SetClient(&clients[clientIndex]);
    }
}

/**
*	@brief	Parses the 'entities' string and assigns each parsed entity to the
*			first free server entity slot there is. After doing so, allocates
*			a class entity based on the 'classname' of the parsed entity.
**/
qboolean Gameworld::SpawnEntitiesFromString(const char* mapName, const char* entities, const char* spawnpoint) {
	// Clear level state.
    level = {};

    // Delete class entities if they are allocated, and reset the server entity to a zero state.
    for (int32_t i = 0; i < game.GetMaxEntities(); i++) {
		// Delete class entity.
		if (classEntities[i]) {
		    delete classEntities[i];
			classEntities[i] = NULL;
		}

		// Reset server entity to a zero state.
		serverEntities[i] = {};
    }

	// Copy in the map name and designated spawnpoint(if any.)
    strncpy(level.mapName, mapName, sizeof(level.mapName) - 1);
    strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint) - 1);

	//// Set client field for each reserved client entity.
	//for (int32_t i = 0; i < game.GetMaxClients(); i++) {
	//	serverEntities[i + 1].client = game.clients + i;
	//}

	// Spawn SVGBasePlayer classes for each reserved client entity.
    PreparePlayers();

	// We'll keep on parsing until this is set to false.
	qboolean isParsing = true;
	
	// This gets set to false the immediate moment we run into parsing trouble.
	qboolean parsedSuccessfully = false;

	// Token pointer.
	char *com_token = nullptr;

	// Pointer to the server entity we intend to employ.
	Entity *serverEntity = nullptr;

	// Engage parsing.
	while (isParsing == true) {
		// Parse the opening brace.
		com_token = COM_Parse(&entities);

		if (!entities) {
			break;
		}

		if (com_token[0] != '{') {
			gi.Error("SpawnEntitiesFromString: found %s when expecting {", com_token);
			return false;
		}

		// Pick the first entity there is, start asking for 
		if (!serverEntity) {
			serverEntity = serverEntities;
		} else {
			serverEntity = ObtainFreeServerEntity();
		}

		// Now we've got the reserved server entity to use, let's parse the entity.
		ParseEntityString(&entities, serverEntity);

		// Allocate the class entity, and call its spawn.
		if (!SpawnParsedClassEntity(serverEntity)) {
			parsedSuccessfully = false;
		}
	}

	// Post spawn entities.
	for (auto& classEntity : classEntities) {
		if (classEntity) {
			classEntity->PostSpawn();
		}
	}

	// Find and hook team slaves.
	FindTeams();

	// Initialize player trail...
	// SVG_PlayerTrail_Init

	return parsedSuccessfully;
}

/**
*   @brief  Frees the given server entity and its class entity in order to recycle it again.
*
*   @return A pointer to the class entity on success, nullptr on failure.
**/
void Gameworld::FreeServerEntity(Entity* svEntity) {
    // Sanity check.
    if (!svEntity) {
		return;
    }

    // Unlink entity from world.
    gi.UnlinkEntity(svEntity);

    // Get entity number.
    int32_t entityNumber = svEntity->state.number;

    // Prevent freeing "special edicts". Clients, and the dead "client body queue".
    if (entityNumber <= game.GetMaxClients()  + BODY_QUEUE_SIZE) {
		gi.DPrintf("Tried to free special edict: %i\n", entityNumber);
		return;
    }

    // Delete the actual entity class pointer.
    FreeClassEntity(svEntity);

    // Reset the entities values.
    *svEntity = {};

    // Store the freeTime, so we can prevent allocating a new entity with this ID too soon.
    // If we don't, we risk the chance of a client lerping between an older entity that
    // was taking up this current slot.
    svEntity->freeTime = level.time;

    // Last but not least, since it isn't in use anymore, let it be known.
    svEntity->inUse = false;

    // Reset serverFlags.
    svEntity->serverFlags = 0;

    // Ensure the class entity is nullified.
    svEntity->classEntity = nullptr;
}

/**
*   @brief  Frees the given class entity.
*
*   @return True on success, false on failure.
**/
qboolean Gameworld::FreeClassEntity(Entity *svEntity) {
    // Sanity check.
    if (!svEntity) {
        gi.DPrintf("WARNING: tried to %s on a nullptr entity.", __func__);
        return false;
    }

    // Used as return value.
    qboolean freedClassEntity = false;

    // Fetch entity number.
    int32_t entityNumber = svEntity->state.number;

    // Special class entity handling IF it still has one.
    if (svEntity->classEntity) {
		// Get pointer to class entity.
		SVGBaseEntity *classEntity = svEntity->classEntity;

        // Remove the classEntity reference
        classEntity->SetGroundEntity(nullptr);
        classEntity->SetLinkCount(0);
        classEntity->SetGroundEntityLinkCount(0);
        classEntity->SetServerEntity(nullptr);
        
		// Reset server entity's class entity pointer.
		svEntity->classEntity = nullptr;
    }

    // For whichever faulty reason the entity might not have had a classentity,
    // so we do an extra delete here just in case.
    if (classEntities[entityNumber] != nullptr) {
        // Free it.
        delete classEntities[entityNumber];
        classEntities[entityNumber] = nullptr;

        // Freed class entity.
        freedClassEntity = true;
    }

    // Return result.
    return freedClassEntity;
}

/**
*   @brief Utility function so we can acquire a valid SVGBasePlayer* pointer.
* 
*   @return A valid pointer to the entity's SVGBasePlayer class entity. nullptr on failure.
**/
SVGBasePlayer* Gameworld::GetPlayerClassEntity(Entity* serverEntity) {
    // Ensure the entity is valid.
    if (!serverEntity || !serverEntity->client || !serverEntity->classEntity || !serverEntity->inUse) {
		return nullptr;
    }

    // Ensure that its classentity is of or derived of SVGBasePlayer.
    SVGBaseEntity* classEntity = serverEntity->classEntity;

    if (!classEntity->IsSubclassOf<SVGBasePlayer>()) {
		return nullptr;
    }

    // We can safely cast to SVGBasePlayer now.
    SVGBasePlayer* player = dynamic_cast<SVGBasePlayer*>(serverEntity->classEntity);

    // Return it.
    return player;
}

/**
*	@brief	Looks for the first free server entity in our buffer.
* 
*	@details	Either finds a free server entity, or initializes a new one.
*				Try to avoid reusing an entity that was recently freed, because it
*				can cause the client to Think the entity morphed into something else
*				instead of being removed and recreated, which can cause interpolated
*				angles and bad trails.
* 
*	@return	If successful, a valid pointer to the entity. If not, a nullptr.
**/
Entity* Gameworld::ObtainFreeServerEntity() { 
    // Incrementor, declared here so we can access it later on.
	int32_t i = 0;

	// Pointer to server entity to return.
	Entity *svEntity = nullptr;
    
	// Acquire a pointer to the first server entity to start checking from.
	svEntity = &serverEntities[maxClients + 1];

	// We'll loop until from maxclients + 1(world entity) till the numberOfEntities 
	// has been reached. If we never managed to return a pointer to a valid server 
	// entity right now, we're going to have to increase the amount of entities in use. 
	// 
	// However, this ONLY proceeds if we haven't already hit the maximum entity count.
	for (int32_t i = maxClients + 1; i < numberOfEntities; i++, svEntity++) {
        // The first couple seconds of server time can involve a lot of
        // freeing and allocating, so relax the replacement policy
	    if (!svEntity->inUse && (svEntity->freeTime < 2.f || level.time - svEntity->freeTime > 0.5f)) {
            //SVG_InitEntity(serverEntity);
            // Set entity to "inUse".
			svEntity->inUse = true;
			
			// Set the entity state number.
			svEntity->state.number = svEntity - serverEntities;
			
			// Return the newly found server entity pointer.
			return svEntity;
        }
    }

	// Do a safety check to prevent crossing maximum entity limit. If we do, error out.
    if (i >= maxEntities) {
        gi.Error("Gameworld::ObtainFreeServerEntity: no free edicts");
		return nullptr;
	}

    // If we've gotten past the gi.Error, it means we can safely increase the number of entities.
    numberOfEntities++;
	globals.numberOfEntities = numberOfEntities;

    // Set entity to "inUse".
    svEntity->inUse = true;

    // Set the entity state number.
    svEntity->state.number = svEntity - serverEntities;

    // Return the server entity.
    return svEntity;
}

/**
*   @brief Chain together all entities with a matching team field
* 
*   @details    All but the first will have the EntityFlags::TeamSlave flag set.
*               All but the last will have the teamchain field set to the next one.
**/
void Gameworld::FindTeams() {
    Entity *e = nullptr, *e2 = nullptr;
    SVGBaseEntity *chain = nullptr;
    int32_t i, j;

    int32_t c = 0;
    int32_t c2 = 0;
    for (i = 1, e = serverEntities + i; i < numberOfEntities; i++, e++) {
        // Fetch class entity.
        SVGBaseEntity *classEntity = classEntities[e->state.number];

        if (classEntity == nullptr) {
            continue;
		}
        if (!classEntity->IsInUse()) {
            continue;
		}
        if (classEntity->GetTeam().empty()) {
            continue;
		}
		if (classEntity->GetFlags() & EntityFlags::TeamSlave) { 
            continue;
		}
        chain = classEntity;
        classEntity->SetTeamMasterEntity(classEntity);
        c++;
        c2++;

        for (j = i + 1, e2 = e + 1 ; j < globals.numberOfEntities ; j++, e2++) {
            // Fetch class entity.
            SVGBaseEntity* classEntity2 = classEntities[e->state.number];

            if (classEntity2 == nullptr) { 
                continue;
			}
            if (!classEntity2->IsInUse()) { 
                continue;
			}
			if (classEntity2->GetTeam().empty()) {
				continue;
			}
			if (classEntity2->GetFlags() & EntityFlags::TeamSlave) {
				continue;
			}
            if (classEntity->GetTeam() == classEntity2->GetTeam()) {
                c2++;
                chain->SetTeamChainEntity(classEntity2);
                classEntity2->SetTeamMasterEntity(classEntity);
                chain = classEntity2;
                classEntity2->SetFlags(classEntity2->GetFlags() | EntityFlags::TeamSlave);
            }
        }
    }

    gi.DPrintf("%i teams with %i entities\n", c, c2);
}

/**
*	@brief	Parses the BSP Entity string and places the results in the server
*			entity dictionary.
**/
qboolean Gameworld::ParseEntityString(const char** data, Entity* svEntity) {
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
		    gi.Error("%s: EOF without closing brace", __func__);
		    return false;
		}

		// Parse the value.
		value = COM_Parse(data);
		// If we are at the end of the string without a closing brace, error out.
		if (!*data) {
		    gi.Error("%s: EOF without closing brace", __func__);
			return false;
		}

		// Ensure we had a value.
		if (value[0] == '}') {
		    gi.Error("%s: closing brace without value for key %s", __func__, key);
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
		svEntity->entityDictionary[key] = value;
    }

	// If we failed to parse the entity properly, zero this one back out.
    if (!parsedSuccessfully) {
		*svEntity = {};
		return false;
	}

	// Return the result.
	return parsedSuccessfully;
}

/**
*   @brief  Allocates the class entity determined by the classname key, and
*           then does a precache before spawning the class entity.
**/
qboolean Gameworld::SpawnParsedClassEntity(Entity* svEntity) {
	// Acquire dictionary.
    auto &dictionary = svEntity->entityDictionary;

	// Get state number.
    int32_t stateNumber = svEntity->state.number;

	// If it does not have a classname key we're in for trouble.
    if (!svEntity->entityDictionary.contains("classname")) {
		// Error out.
		gi.Error("%s: Can't spawn parsed server entity #%i due to a missing classname key.\n");
		
		// Failed.
		return false;
    }

	// Actually spawn the class entity.
    svEntity->classEntity = AllocateClassEntity(svEntity, svEntity->entityDictionary["classname"]);

    // Something went wrong with allocating the class entity.
    if (!svEntity->classEntity) {
		// Be sure to free it.
		FreeServerEntity(svEntity);

		// Failed.
		gi.DPrintf("WARNING: Spawning entity(%s) failed.\n", svEntity->entityDictionary["classname"]);
		return false;
		//return false;
    }

    // Initialise the entity with its respected keyvalue properties
    for (const auto& keyValueEntry : svEntity->entityDictionary) {
		svEntity->classEntity->SpawnKey(keyValueEntry.first, keyValueEntry.second);
    }

    // Precache the entity.
    svEntity->classEntity->Precache();

	// Aaaand, spawn it.
    svEntity->classEntity->Spawn();

	// Success.
	return true;
}

/**
*	@brief	Seeks through the type info system for a class registered under the classname string.
*			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
*			try and allocate it.
*	@return	nullptr in case of failure, a valid pointer to a class entity otherwise.
**/
SVGBaseEntity *Gameworld::AllocateClassEntity(Entity* svEntity, const std::string &classname) {
    // Start with a nice nullptr.
    SVGBaseEntity* spawnEntity = nullptr;

	// Safety check.
    if (!svEntity) {
		return nullptr;
    }

    // Get entity state number.
    int32_t stateNumber = svEntity->state.number;

	// Warn if a slot is already occupied.
    if (classEntities[stateNumber] != nullptr) {
		// Warn.
		gi.DPrintf("WARNING: trying to allocate class entity '%s' the slot #%i was pre-occupied.\n");

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
		    gi.DPrintf("WARNING: unknown entity '%s'\n", classname.c_str());

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && info->IsMapSpawnable()) {
		// Allocate and return out new class entity.
		return (classEntities[stateNumber] = info->AllocateInstance(svEntity));
    } else {
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			gi.DPrintf("WARNING: tried to allocate an abstract class '%s'\n", info->classname);
		} else if (!info->IsMapSpawnable()) {
		    gi.DPrintf("WARNING: tried to allocate a code-only class '%s'\n", info->classname);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}