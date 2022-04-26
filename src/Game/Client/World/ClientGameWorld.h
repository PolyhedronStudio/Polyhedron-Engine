/***
*
*	License here.
*
*	@file
*
*	GameWorld class for managing entity lifetime. 
*	(Creation, Destruction, Spawning etc.)
* 
*	Aside from managing entity lifetimes, it is also the general keeper of the
*	current active gamemode. The gamemode and gameworld are close friends who
*	go along happily hand in hand.
*
***/
#pragma once

// Pre-define.
class IGameMode;
class IGameWorld;
class IClientGameEntity;
class SGEntityHandle;
class CLGBasePacketEntity;
class CLGBaseLocalEntity;
class Worldspawn;

// GameWorld Interface.
#include "../../../Game/Shared/World/IGameWorld.h"


/***
*	Configuration of reserved entity types, and the start of the local entity indexes.
***/
//! Use a static entity ID on some things because the renderer relies on EntityID to match between meshes
//! between the current and previous frames.
static constexpr int32_t RESERVED_LOCAL_ENTITIY_ID_GUN = 0;
static constexpr int32_t RESERVED_ENTITIY_SHADERBALLS = 1;
static constexpr int32_t RESERVED_ENTITIY_COUNT = 2;

//! The actual start of the local entities index.
static constexpr int32_t LOCAL_ENTITIES_START_INDEX = MAX_WIRED_POD_ENTITIES + RESERVED_ENTITIY_COUNT;



/**
*	@brief GameWorld regulates the lifetime management of all entities.
* 
*	@details 
**/
class ClientGameWorld : public IGameWorld {
public:
    /**
	*	@brief Default constructor.
	**/
    ClientGameWorld() = default;

    /**
	*	@brief Default destructor
	**/
    ~ClientGameWorld() = default;

public:
    /**
	*	@brief Initializes the gameworld and its member objects.
	***/
    void Initialize();
    /**
	*	@brief Shutsdown the gameworld and its member objects.
	**/
    void Shutdown();



    /**
	*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
	**/
    void SetupGameMode();
    /**
	*	@brief	Destroys the current gamemode object.
	**/
    void DestroyGameMode();
    /**
	*	@return A pointer to the current active game mode.
	**/
    inline IGameMode* GetGameMode() { return currentGameMode; }



    /**
    *   @return A pointer to the gameworld's clients array.
    **/
    inline ServerClient* GetClients() final { return clients; }
    /**
    *   @return The maximum allowed clients in this game.
    **/
    inline int32_t GetMaxClients() final { return maxClients; }
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxEntities.
    * 
    *   @return The maximum allowed entities in this game.
    **/
    inline int32_t GetMaxEntities() final { return MAX_POD_ENTITIES; }
    /**
	*	@return	Total number of spawned entities.
	**/
    inline int32_t GetNumberOfEntities() final { return numberOfEntities; }


    /**
	*	@brief	Parses the 'entities' string in order to create, precache and spawn
	*			a GameEntity which matches to the entity's set classname.
	**/
    qboolean SpawnFromBSPString(const char* mapName, const char* bspString, const char* spawnpoint) final;
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
    PODEntity* GetUnusedPODEntity(bool isWired = true) final;

 //   /**
	//*   @brief  Creates and assigns a game entity to the given server entity based on the classname.
 //   *
 //   *   @return A pointer to the game entity on success, nullptr on failure.
 //   **/
 //   template<typename entityClass> inline entityClass* CreateGameEntity(PODEntity *podEntity = nullptr, bool allocateNewServerEntity = true) {
 //       // Class entity to be returned.
 //       entityClass* gameEntity = nullptr;

 //    //   // If a null entity was passed, create a new one
	//    if (podEntity == nullptr) {
 //           if (allocateNewServerEntity) {
 //               podEntity = GetUnusedPODEntity();
 //           } else {
 //               Com_DPrint("WARNING: tried to spawn a game entity when the edict is null\n");
 //               return nullptr;
 //           }
 //       }
 //       
 //       // Abstract classes will have AllocateInstance as nullptr, hence we gotta check for that
 //       if (entityClass::ClassInfo.AllocateInstance) {
	//	    // Entities that aren't in the type info system will error out here
	//		gameEntity = static_cast<entityClass*>(entityClass::ClassInfo.AllocateInstance(podEntity));
 //   
	//		// Be sure to set its classname.
	//		gameEntity->SetClassname(gameEntity->GetTypeInfo()->classname);

	//		// Store the podEntity's game entity pointer.
	//		podEntity->gameEntity = gameEntity;

	//		if (gameEntities[podEntity->currentState.number] == nullptr) {
	//			gameEntities[podEntity->currentState.number] = gameEntity;
	//		} else {
	//			Com_DPrint("ERROR: edict %i is already taken\n", podEntity->currentState.number);
	//		}
	//	}

 //       return gameEntity;
 //   }
    

	/**
	*   @brief  Creates a new GameEntity for the ClientEntity based on the passed state. If the 
	*			hashedClassname values are identical it'll call its UpdateFromState on the already
	*			instanced object.
	*
	*   @return On success: A pointer to the ClientEntity's GameEntity, which may be newly allocated. On failure: A nullptr.
	**/
	GameEntity* UpdateGameEntityFromState(const EntityState& state, PODEntity* clEntity);
	/**
	*   @brief  When the client receives state updates it calls into this function so we can update
	*           the game entity belonging to the server side entity(defined by state.number).
	* 
	*           It defers the updating of game entity to the UpdateGameEntityFromState function.
	* 
	*   @return True on success, false in case of trouble. (Should never happen, and if it does,
	*           well... file an issue lmao.)
	**/
	qboolean UpdateFromState(PODEntity *clEntity, const EntityState& state);
	
	/**
	*   @brief  Frees the given server entity and its game entity in order to recycle it again.
    *
    *   @return A pointer to the game entity on success, nullptr on failure.
    **/
    void FreePODEntity(PODEntity *podEntity) override;
    /**
	*   @brief  Frees the given game entity.
    *
    *   @return True on success, false on failure.
    **/
    qboolean FreeGameEntity(PODEntity* podEntity) override;


    /**
    *   @brief	Utility function so we can acquire a valid SVGBasePlayer*. It operates
    *			by using an entity handle in order to make sure that it has a valid
    *			server and game entity object.
    *	@param	requireValidClient	Expands the check to make sure the entity's client isn't set to nullptr.
    *	@param	requireInUse		Expands the check to make sure the entity has its inUse set to true.
    * 
    *   @return A valid pointer to the entity's SVGBasePlayer game entity. nullptr on failure.
    **/
    static IClientGameEntity* ValidateEntity(const SGEntityHandle &entityHandle, bool requireClient = false, bool requireInUse = false);


    /**
	*	@return	A pointer to the server entities array.
	**/
    inline PODEntity* GetPODEntities() final { 
		return &podEntities[0]; 
	}
    /**
    *   @return A pointer of the server entity located at index.
    **/
    inline PODEntity* GetPODEntityByIndex(uint32_t index) final {
        if (index < 0 || index >= MAX_POD_ENTITIES) {
            return nullptr; 
        }
	    return &podEntities[index];
    }

    /**
	*	@return	A pointer to the class entities array.
	**/
    inline GameEntityVector &GetGameEntities() final {
        return gameEntities;
    }	
	/**
    *   @return A pointer of the server entity located at index.
    **/
    inline GameEntity* GetGameEntityByIndex(int32_t index) final {
		// Ensure ID is within bounds.
		if (index < 0 || index >= MAX_POD_ENTITIES) {
			return nullptr;
		}

		// Return game entity that belongs to this ID.
		return gameEntities[index];
    }

    /**
	*   @return A pointer to the worldspawn game entity.
	**/
    inline PODEntity* GetWorldspawnPODEntity() final { 
        return &podEntities[0]; 
    }
    /**
	*   @return A pointer to the worldspawn game entity.
	**/
    inline Worldspawn* GetWorldspawnGameEntity() final { 
        return nullptr;//dynamic_cast<Worldspawn*>(gameEntities[0]); 
    }



    ///**
    //*   @brief  Spawns a debris model entity at the given origin.
    //*   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
    //**/
    //void ThrowDebris(SVGBaseEntity* debrisser, const std::string& gibModel, const vec3_t& origin, float speed);

    ///**
    //*   @brief  Spawns a gib model entity flying at random velocities and directions.
    //*   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
    //*/
    //void ThrowGib(SVGBaseEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType);
    /**
    *   @brief  Spawns a debris model entity at the given origin.
    *   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
    **/
    void ThrowDebris(GameEntity* debrisser, const std::string& gibModel, const vec3_t& origin, float speed) override;

    /**
    *   @brief  Spawns a gib model entity flying at random velocities and directions.
    *   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
    */
    void ThrowGib(GameEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType) override;



private:
    //! Assigned the value of the latched cvar maxclients. Makes for easier access.
    int32_t maxClients = 0;
	//! Assigned the clamped(MAX_WIRED_POD_ENTITIES) value of the latched cvar maxentities. Makes for easier access.
    int32_t maxEntities = 0;

	//! Clients array, allocated to the size of maxclients cvar.
    ServerClient *clients = nullptr;

	//! Currently active game mode.
    IGameMode* currentGameMode = nullptr;


    
private:
    /**
    *   @brief Allocates the client's array for the reserved amount.
    **/
    void PrepareClients();
    /**
    *	@brief Prepares the game's client entities with a base player game entity.
    **/
    void PreparePlayers();


private:
    //// Array storing the POD server entities.
    //Entity serverEntities[MAX_WIRED_POD_ENTITIES];

    ////! Array for storing the server game's class entities.
    //GameEntity* gameEntities[MAX_WIRED_POD_ENTITIES];

    ////! Total number of actively spawned entities.
    //int32_t numberOfEntities = 0;


    /**
    *   @brief Nothing yet.
    **/
    void PrepareItems() { };

    /**
    *   @brief	Clamps maxEntities if needed, assigns the exported globals.entities and ensures
    *			that all class entities are set to nullptr.
    **/
    void PrepareEntities();

    /**
    *   @brief Chain together all entities with a matching team field
    * 
    *   @details    All but the first will have the EntityFlags::TeamSlave flag set.
    *               All but the last will have the teamchain field set to the next one.
    **/
    void FindTeams();

	/**
	*	@brief	Parses the BSP Entity string and places the results in the server
	*			entity dictionary.
	*	@return	True in case it succeeded parsing the entity string.
	**/
	qboolean ParseEntityString(const char** data, SpawnKeyValues &parsedKeyValues);


    /**
    *   @brief  Allocates the game entity determined by the classname key, and
    *           then does a precache before spawning the game entity.
    **/
    qboolean CreateGameEntityFromDictionary(PODEntity *podEntity, SpawnKeyValues &dictionary);

    /**
    *	@brief	Seeks through the type info system for a class registered under the classname string.
    *			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
    *			try and allocate it.
    *	@return	nullptr in case of failure, a valid pointer to a game entity otherwise.
    **/
    GameEntity* CreateGameEntityFromClassname(PODEntity *podEntity, const std::string& classname);
};
