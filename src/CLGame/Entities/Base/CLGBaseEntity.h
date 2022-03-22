/***
*
*	License here.
*
*	@file
*
*	Client Game BaseEntity.
* 
***/
#pragma once

// Client Game Exports Interface.
#include "Shared/Interfaces/IClientGameExports.h"

// Client Game ClassEntity Interface.
#include "../IClientGameClassEntity.h"


class CLGBaseEntity : public IClientGameClassEntity {
public:
    /**
    *
    *   Function pointers for actual callbacks.
    *
    **/
    using ThinkCallbackPointer      = void(CLGBaseEntity::*)(void);
    using UseCallbackPointer        = void(CLGBaseEntity::*)(CLGBaseEntity* other, CLGBaseEntity* activator);
    using TouchCallbackPointer      = void(CLGBaseEntity::*)(CLGBaseEntity* self, CLGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    using BlockedCallbackPointer    = void(CLGBaseEntity::*)(CLGBaseEntity* other);
    using TakeDamageCallbackPointer = void(CLGBaseEntity::*)(CLGBaseEntity* other, float kick, int32_t damage);
    using DieCallbackPointer        = void(CLGBaseEntity::*)(CLGBaseEntity* inflictor, CLGBaseEntity* attacker, int damage, const vec3_t& point);

    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    CLGBaseEntity(ClientEntity *clEntity);
    virtual ~CLGBaseEntity() = default;

    // Runtime type information
	DefineMapClass( "CLGBaseEntity", CLGBaseEntity, IClientGameClassEntity);

    // Checks if this entity class is exactly the given class
    // @param entityClass: an entity class which must inherint from SVGBaseEntity
    template<typename entityClass>
    bool IsClass() const { // every entity has a ClassInfo, thanks to the DefineXYZ macro
        return GetTypeInfo()->IsClass( entityClass::ClassInfo );
    }

    // Checks if this entity class is a subclass of another, or is the same class
    // @param entityClass: an entity class which must inherint from SVGBaseEntity
    template<typename entityClass>
    bool IsSubclassOf() const {
        return GetTypeInfo()->IsSubclassOf( entityClass::ClassInfo );
    }

    /**
    *
    *   Client Class Entity Interface Functions.
    *
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() override;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() override;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() override;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() override;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.



    /***
    *
    *   Client Class Entity Functions.
    *
    ***/
    /**
    *   @brief  Updates the entity with the data of the newly passed EntityState object.
    **/
    virtual void UpdateFromState(const EntityState &state) override;

    /**
    *   @return A reference to the current state object.
    **/
    inline const EntityState& GetCurrentEntityState() final {
        return currentState;
    }

    /**
    *   @brief  Sets the classname of this entity.
    **/
    virtual void SetClassname(const std::string& classname) override;

    /**
    *   @return A string containing the entity's classname.
    **/
    virtual const std::string GetClassname() override;
    /**
    *   @return An uint32_t containing the hashed classname string.
    **/
    virtual uint32_t GetHashedClassname() override;

    /**
    *   @brief  Sets a pointer referring to this class' client entity.
    **/
    virtual void SetClientEntity(ClientEntity* clEntity) override;

    /**
    *   @return The pointer referring to this class' client entity.
    **/
    virtual ClientEntity* GetClientEntity() override;


    /***
    *
    *   OnEventCallbacks.
    *
    ***/
    /**
    *   @brief  Gets called right before the moment of deallocation happens.
    **/
    virtual void OnDeallocate() override;

    /**
    *   @brief
    **/

    ///**
    //*   @brief  Only sets the entity ID if it has a valid clientEntity pointer to set it on.
    //**/
    //inline void SetEntityID(int32_t id) {
    //    if (clientEntity) {
    //        clientEntity->id = id;
    //    }
    //}
    ///**
    //*   @return Returns the clientEntity id when its a valid pointer. World(0), otherwise.
    //**/
    //inline int32_t GetEntityID() {
    //    if (clientEntity) {
    //        return clientEntity->id;
    //    }

    //    return 0; // None, or world for that matter.
    //}

private:
    // Classname.
    std::string classname = "";

    //  Hashed classname string.
    uint32_t hashedClassname = 0;

    //! Pointer to the client entity which owns this class entity.
    ClientEntity *clientEntity = nullptr;

    // Client Class Entities maintain their own states. (Get copied in from updates.)
    EntityState currentState = {};
    EntityState previousState = {};
};