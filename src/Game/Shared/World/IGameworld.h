/***
*
*	License here.
*
*	@file
*
*	The Gameworld class exists to create, destroy and keep track of entity their lifetimes.
*	(including but not limited to, player entities, explosions, gibs and debris.)
* 
*	Aside from that it also households the currently active Gamemode.
*
***/
#pragma once

// Pre-declare.
class ISharedGameEntity;
class SGEntityHandle;
class Worldspawn;
class IGamemode;


/**
*	@brief GameWorld regulates the lifetime management of all entities.
* 
*	@details 
**/
class IGameworld {
public:
    /**
	*	@brief Default constructor.
	**/
    IGameworld() = default;

    /**
	*	@brief Default destructor
	**/
    virtual ~IGameworld() = default;

public:
    /**
	*	@brief Initializes the gameworld and its member objects.
	***/
    virtual void Initialize() = 0;
    /**
	*	@brief Shutsdown the gameworld and its member objects.
	**/
    virtual void Shutdown() = 0;



    /**
	*	@brief	Creates the correct gamemode object instance based on the gamemode cvar.
	**/
    virtual void SetupGamemode() = 0;
    /**
	*	@brief	Destroys the current gamemode object.
	**/
    virtual void DestroyGamemode() = 0;
    /**
	*	@return A pointer to the current active game mode.
	**/
    virtual IGamemode* GetGamemode() = 0;



    /**
    *   @return A pointer to the gameworld's clients array.
    **/
    virtual ServerClient* GetClients() = 0;
    /**
    *   @return The maximum allowed clients in this game.
    **/
    virtual int32_t GetMaxClients() = 0; 
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxEntities.
    * 
    *   @return The maximum allowed entities in this game.
    **/
    virtual int32_t GetMaxEntities() = 0;
    /**
	*	@return	Total number of spawned entities.
	**/
    virtual int32_t GetNumberOfEntities() = 0;


    /**
	*	@brief	Parses the 'entites' string and assigns each parsed entity to the
	*			first free server entity slot there is. After doing so, allocates
	*			a class entity based on the 'classname' of the parsed entity.
	**/
    virtual qboolean SpawnFromBSPString(const char* mapName, const char* entities, const char* spawnpoint) = 0;
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
    virtual PODEntity* ObtainFreePODEntity() = 0; // Rename to: QueryForPODEntity

 //   /**
	//*   @brief  Creates and assigns a class entity to the given server entity based on the classname.
 //   *
 //   *   @return A pointer to the class entity on success, nullptr on failure.
 //   **/
 //   template<typename entityClass> inline entityClass* CreateClassEntity(Entity* svEntity = nullptr, bool allocateNewServerEntity = true) {
 //       // Class entity to be returned.
 //       entityClass* classEntity = nullptr;

 //       // If a null entity was passed, create a new one
	//    if (svEntity == nullptr) {
 //           if (allocateNewServerEntity) {
 //               svEntity = ObtainFreeServerEntity();
 //           } else {
 //               gi.DPrintf("WARNING: tried to spawn a class entity when the edict is null\n");
 //               return nullptr;
 //           }
 //       }
 //       
 //       // Abstract classes will have AllocateInstance as nullptr, hence we gotta check for that
 //       if (entityClass::ClassInfo.AllocateInstance) {
 //   	    // Entities that aren't in the type info system will error out here
 //           classEntity = static_cast<entityClass*>(entityClass::ClassInfo.AllocateInstance(svEntity));
 //   
 //           // Be sure ti set its classname.
 //           classEntity->SetClassname(classEntity->GetTypeInfo()->classname);

 //           // Store the svEntity's class entity pointer.
 //           svEntity->classEntity = classEntity;

 //           if (nullptr == classEntities[svEntity->state.number]) {
 //               classEntities[svEntity->state.number] = classEntity;
 //           } else {
 //               gi.DPrintf("ERROR: edict %i is already taken\n", svEntity->state.number);
 //           }
 //       }
 //       return classEntity;
 //   }
    /**
	*   @brief  Frees the given server entity and its class entity in order to recycle it again.
    *
    *   @return A pointer to the class entity on success, nullptr on failure.
    **/
    virtual void FreePODEntity(PODEntity *podEntity) = 0;;
    /**
	*   @brief  Frees the given class entity.
    *
    *   @return True on success, false on failure.
    **/
    virtual qboolean FreeClassEntity(PODEntity* podEntity) = 0;
    /**
    *   @brief	Utility function so we can acquire a valid SVGBasePlayer*. It operates
    *			by using an entity handle in order to make sure that it has a valid
    *			server and class entity object.
    *	@param	requireValidClient	Expands the check to make sure the entity's client isn't set to nullptr.
    *	@param	requireInUse		Expands the check to make sure the entity has its inUse set to true.
    * 
    *   @return A valid pointer to the entity's SVGBasePlayer class entity. nullptr on failure.
    **/
    static ClassEntity* ValidateEntity(const SGEntityHandle &entityHandle, bool requireClient = false, bool requireInUse = false);



    /**
    *   @brief Selectively acquire a list of Entity* derived objects using entity filters.
    * 
    *   @return Returns a span containing all the entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    template<std::size_t start, std::size_t count> inline auto GetPODEntityRange() -> std::span<PODEntity, count> {
        return std::span(podEntities).subspan(start, count); //return std::span(serverEntities).subspan<start, count>(); 
    }
    /**
    *   @brief Selectively acquire a list of SVGBaseEntity* derived objects using class entity filters.
    * 
    *   @return Returns a span containing all the base entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    template<std::size_t start, std::size_t count> inline auto GetClassEntityRange() -> ClassEntitySpan {
        return ClassEntitySpan(classEntities).subspan(start, count); //return std::span(classEntities).subspan<start, count>(); 
    }
    /**
    *   @brief Selectively acquire a list of Entity* derived objects using entity filters. Use the templated version where possible.
    * 
    *   @return Returns a span containing all the entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    inline PODEntitySpan GetPODEntityRange(std::size_t start, std::size_t count) { 
        return PODEntitySpan(serverEntities).subspan(start, count); 
    }
    /**
    *   @brief Selectively acquire a list of SVGBaseEntity* derived objects using class entity filters. Use the templated version where possible.
    * 
    *   @return Returns a span containing all the base entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    inline ClassEntitySpan GetClassEntityRange(std::size_t start, std::size_t count) {
        return ClassEntitySpan(classEntities).subspan(start, count); 
    }



    /**
	*	@return	A pointer to the pod entities array.
	**/
    virtual PODEntity* GetPODEntities() = 0;
	/**
    *   @return A pointer of the pod entity located at index.
    **/
    virtual PODEntity* GetPODEntityByIndex(uint32_t index) = 0;

    /**
	*	@return	A pointer to the class entities array.
	**/
    virtual ISharedGameEntity** GetClassEntities() = 0;
    /**
    *   @return A pointer of the pod entity located at index.
    **/
    ISharedGameEntity* GetClassEntityByIndex(uint32_t index) = 0;

    /**
	*   @return A pointer to the worldspawn class entity.
	**/
    virtual Entity* GetWorldspawnPODEntity() = 0;
    /**
	*   @return A pointer to the worldspawn class entity.
	**/
    virtual Worldspawn* GetWorldspawnClassEntity() = 0;



    /**
    *   @brief  Spawns a debris model entity at the given origin.
    *   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
    **/
    virtual void ThrowDebris(SVGBaseEntity* debrisser, const std::string& gibModel, const vec3_t& origin, float speed) = 0;

    /**
    *   @brief  Spawns a gib model entity flying at random velocities and directions.
    *   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
    */
    virtual void ThrowGib(SVGBaseEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType) = 0;



private:
    //! Currently active game mode.
    IGamemode* currentGamemode = nullptr;

    //! Clients array, allocated to the size of maxclients cvar.
    ServerClient *clients = nullptr;

    //! Assigned the value of the latched cvar maxclients. Makes for easier access.
    int32_t maxClients = 0;

    //! Assigned the clamped(MAX_EDICTS) value of the latched cvar maxentities. Makes for easier access.
    int32_t maxEntities = 0;


    
private:
    /**
    *   @brief Allocates the client's array for the reserved amount.
    **/
    virtual void PrepareClients() = 0;
    /**
    *	@brief Prepares the game's client entities with a base player class entity.
    **/
    virtual void PreparePlayers() = 0;


private:
    // Array storing the POD server entities.
    PODEntity serverEntities[MAX_EDICTS];

    //! Array for storing the server game's class entities.
    ISharedGameEntity* classEntities[MAX_EDICTS];

    //! Total number of actively spawned entities.
    int32_t numberOfEntities = 0;


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
    qboolean ParseEntityString(const char** data, Entity* svEntity);

    /**
    *   @brief  Allocates the class entity determined by the classname key, and
    *           then does a precache before spawning the class entity.
    **/
    qboolean SpawnParsedClassEntity(Entity* svEntity);

    /**
    *	@brief	Seeks through the type info system for a class registered under the classname string.
    *			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
    *			try and allocate it.
    *	@return	nullptr in case of failure, a valid pointer to a class entity otherwise.
    **/
    IServerGameEntity* AllocateClassEntity(Entity* svEntity, const std::string& classname);
};
