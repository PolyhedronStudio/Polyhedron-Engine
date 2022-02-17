/***
*
*	License here.
*
*	@file
*
*	Gameworld class for managing entity lifetime. 
*	(Creation, Destruction, Spawning etc.)
* 
*	Aside from managing entity lifetimes, it is also the general keeper of the
*	current active gamemode. The gamemode and gameworld are close friends who
*	go along happily hand in hand.
*
***/
#pragma once

// Pre-define.
class SVGBaseEntity;
class SVGEntityHandle;
class Worldspawn;
class IGamemode;

#include "../Entities.h"
#include "../Entities/Worldspawn.h"
/**
*	@brief GameWorld regulates the lifetime management of all entities.
* 
*	@details 
**/
class Gameworld {
public:
    /**
	*	@brief Default constructor.
	**/
    Gameworld() = default;

    /**
	*	@brief Default destructor
	**/
    ~Gameworld() = default;

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
    void SetupGamemode();
    /**
	*	@brief	Destroys the current gamemode object.
	**/
    void DestroyGamemode();
    /**
	*	@return A pointer to the current active game mode.
	**/
    inline IGamemode* GetGamemode() { return currentGamemode; }



    /**
    *   @return A pointer to the gameworld's clients array.
    **/
    inline ServerClient* GetClients() { return clients; }
    /**
    *   @return The maximum allowed clients in this game.
    **/
    inline int32_t GetMaxClients() { return maxClients; }
    /**
    *   @brief  Code shortcut for acquiring gameworld's maxEntities.
    * 
    *   @return The maximum allowed entities in this game.
    **/
    inline int32_t GetMaxEntities() { return maxEntities; }
    /**
	*	@return	Total number of spawned entities.
	**/
    inline int32_t GetNumberOfEntities() { return numberOfEntities; }


    /**
	*	@brief	Parses the 'entites' string and assigns each parsed entity to the
	*			first free server entity slot there is. After doing so, allocates
	*			a class entity based on the 'classname' of the parsed entity.
	**/
    qboolean SpawnEntitiesFromString(const char* mapName, const char* entities, const char* spawnpoint);
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
    Entity* ObtainFreeServerEntity();
    /**
	*   @brief  Creates and assigns a class entity to the given server entity based on the classname.
    *
    *   @return A pointer to the class entity on success, nullptr on failure.
    **/
    template<typename entityClass> inline entityClass* CreateClassEntity(Entity* svEntity = nullptr, bool allocateNewServerEntity = true) {
        // Class entity to be returned.
        entityClass* classEntity = nullptr;

        // If a null entity was passed, create a new one
	    if (svEntity == nullptr) {
            if (allocateNewServerEntity) {
                svEntity = ObtainFreeServerEntity();
            } else {
                gi.DPrintf("WARNING: tried to spawn a class entity when the edict is null\n");
                return nullptr;
            }
        }
        
        // Abstract classes will have AllocateInstance as nullptr, hence we gotta check for that
        if (entityClass::ClassInfo.AllocateInstance) {
    	    // Entities that aren't in the type info system will error out here
            classEntity = static_cast<entityClass*>(entityClass::ClassInfo.AllocateInstance(svEntity));
    
            // Be sure ti set its classname.
            classEntity->SetClassname(classEntity->GetTypeInfo()->classname);

            // Store the svEntity's class entity pointer.
            svEntity->classEntity = classEntity;

            if (nullptr == classEntities[svEntity->state.number]) {
                classEntities[svEntity->state.number] = classEntity;
            } else {
                gi.DPrintf("ERROR: edict %i is already taken\n", svEntity->state.number);
            }
        }
        return classEntity;
    }
    /**
	*   @brief  Frees the given server entity and its class entity in order to recycle it again.
    *
    *   @return A pointer to the class entity on success, nullptr on failure.
    **/
    void FreeServerEntity(Entity *svEntity);
    /**
	*   @brief  Frees the given class entity.
    *
    *   @return True on success, false on failure.
    **/
    qboolean FreeClassEntity(Entity* svEntity);
    /**
    *   @brief Utility function so we can acquire a valid SVGBasePlayer* pointer.
    * 
    *   @return A valid pointer to the entity's SVGBasePlayer class entity. nullptr on failure.
    **/
    SVGBasePlayer* GetPlayerClassEntity(Entity* serverEntity);



    /**
    *   @brief Selectively acquire a list of Entity* derived objects using entity filters.
    * 
    *   @return Returns a span containing all the entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    template<std::size_t start, std::size_t count> inline auto GetServerEntityRange() -> std::span<Entity, count> {
        return std::span(serverEntities).subspan<start, count>(); 
    }
    /**
    *   @brief Selectively acquire a list of SVGBaseEntity* derived objects using class entity filters.
    * 
    *   @return Returns a span containing all the base entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    template<std::size_t start, std::size_t count> inline auto GetClassEntityRange() -> std::span<SVGBaseEntity*, count> {
        return std::span(classEntities).subspan<start, count>(); 
    }
    /**
    *   @brief Selectively acquire a list of Entity* derived objects using entity filters. Use the templated version where possible.
    * 
    *   @return Returns a span containing all the entities from the range of [start] to [start + count]
    *           that passed the filter process.
    **/
    inline ServerEntitySpan GetServerEntityRange(std::size_t start, std::size_t count) { 
        return ServerEntitySpan(serverEntities).subspan(start, count); 
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
	*	@return	A pointer to the server entities array.
	**/
    inline Entity* GetServerEntities() { return &serverEntities[0]; }

    /**
    *   @return A pointer of the server entity located at index.
    **/
    inline Entity* GetServerEntityByIndex(uint32_t index) {
        if (index < 0 || index >= MAX_EDICTS) {
            return nullptr; 
        }
	    return &serverEntities[index];
    }

    /**
	*	@return	A pointer to the class entities array.
	**/
    inline SVGBaseEntity** GetClassEntities() {
        return classEntities; 
    }

    /**
    *   @return A pointer of the server entity located at index.
    **/
    inline SVGBaseEntity* GetClassEntityByIndex(uint32_t index) {
    	if (index < 0 || index >= MAX_EDICTS) {
    	    return nullptr;
	    }
	    return classEntities[index];
    }

    /**
	*   @return A pointer to the worldspawn class entity.
	**/
    inline Entity* GetWorldspawnServerEntity() { 
        return &serverEntities[0]; 
    }
    
    /**
	*   @return A pointer to the worldspawn class entity.
	**/
    inline Worldspawn* GetWorldspawnClassEntity() { 
        return dynamic_cast<Worldspawn*>(classEntities[0]); 
    }



    /**
    *   @brief  Spawns a debris model entity at the given origin.
    *   @param  debrisser Pointer to an entity where it should acquire a debris its velocity from.
    **/
    void ThrowDebris(SVGBaseEntity* debrisser, const std::string& gibModel, const vec3_t& origin, float speed);

    /**
    *   @brief  Spawns a gib model entity flying at random velocities and directions.
    *   @param  gibber Pointer to the entity that is being gibbed. It is used to calculate bbox size of the gibs.
    */
    void ThrowGib(SVGBaseEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType);



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
    void PrepareClients();
    /**
    *	@brief Prepares the game's client entities with a base player class entity.
    **/
    void PreparePlayers();

private:
    // Array storing the POD server entities.
    Entity serverEntities[MAX_EDICTS];

    //! Array for storing the server game's class entities.
    SVGBaseEntity* classEntities[MAX_EDICTS];

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
    SVGBaseEntity* AllocateClassEntity(Entity* svEntity, const std::string& classname);
};
