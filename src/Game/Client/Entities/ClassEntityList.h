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

// Predeclaration(s).
class CLGBaseEntity;

/**
*
*   Manages allocation and destruction of client game class entities.
* 
*   The first 2048 slots are reserved strictly for server entities only.
*   The second 2048 slots are reserved for client side only entities.
*   
*   Since we're dealing with incoming packets telling us the state of each entity in frame, 
*   we'll have to carefully manage which states belong to what entities. The entity classname
*   is send over the wire as a hash value(Com_HashString). Whenever it changes, the current
*   class object here is destroyed immediately and replaced by the class which registered name
*   has an equal hashed string. If it fails to find an equal hashed string it'll resort to
*   spawning a CLGBaseEntity instead.
* 
*   In similar fashion like the SVGBaseEntity you have Set, Get, callback and think functions
*   respectively.
* 
**/
using CLGEntityVector = std::vector<IClientGameEntity*>;

class ClassEntityList {
public:
    //! Constructor/Destructor.
    ClassEntityList() {
        // 2048 for server entities, and 2048 more for... whichever.
        classEntities.reserve(MAX_PACKET_ENTITIES * 2);

        // To ensure that slot 0 is in use, keeps indexes correct.
        classEntities.push_back(nullptr);
    }
    virtual ~ClassEntityList() = default;

    /**
    *   @brief  Clears the list by deallocating all its members.
    **/
    void Clear();

    /**
    *   @brief  Spawns and inserts a new class entity at the state.number index, assigning the client entity pointer
    *           as its 'soulmate', I suppose.
    *   @return Pointer to the class entity object on sucess. On failure, nullptr.
    **/
    IClientGameEntity *SpawnFromState(const EntityState &state, ClientEntity *clEntity);

    /**
    *   @return A pointer to the entity who's index matches the state number.
    **/
    IClientGameEntity *GetByNumber(int32_t number);

    /**
    *   @brief  Inserts the class entity pointer at the number index of our class entity vector.
    *   @param  force   When set to true it'll delete any previously allocated class entity occupying the given slot.
    *   @return Pointer to the entity being inserted. nullptr on failure.
    **/
    IClientGameEntity *InsertAt(int32_t number, IClientGameEntity *clgEntity, bool force = true);

    inline CLGEntityVector *GetClassEntities() { return &classEntities; };
private:
    //! First 2048 are reserved for server side entities.
    CLGEntityVector classEntities;
};