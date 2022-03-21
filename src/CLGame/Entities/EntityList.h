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
*   Contains a vector of pointers to Client Base Entities, acting In similar fashion to the
*   Server Game Module's Base Entities. The first MAX_ENTITIES are reserved for server side
*   entities. When no specific client side entity class has been registered to a classname 
*   it'll spawn a default client entity which acts no different than how entities would do in 
*   the time of Quake 2. 
*   
*   Since we're dealing with incoming packets telling us the state of each entity in frame, 
*   we'll have to carefully manage which states belong to what entities. The entity classname
*   is send over the wire as a hash value(Com_HashString). Whenever it changes, the current
*   class object here is destroyed immediately and replaced by the class which registered name
*   has an equal hashed string. If it fails to find an equal hashed string it'll resort to
*   spawning a CLGBaseEntity instead.
* 
*   In similar fashion like the SVGBaseEntity you have Set, Get, and callback and think functions
*   working accordingly.
* 
**/
using CLGEntityVector = std::vector<CLGBaseEntity*>;

class EntityList {
public:
    //! Constructor/Destructor.
    EntityList() {
        // 2048 for server entities, and 2048 more for... whichever.
        classEntities.reserve(MAX_PACKET_ENTITIES * 2);
    }
    virtual ~EntityList() = default;

    /**
    *   @brief  
    **/
    template <typename T> T* SpawnEntityFromState(ClientEntity *clgEntity, const EntityState &state) {
	    // Ensure ID is within bounds.
	    if (state.number < 0 || state.number > classEntities.size()) {
		    return nullptr;
	    }
        
        // Fetch entity ptr from slot.
        CLGBaseEntity *classEntity = classEntities.data()[state.number];
        
        // Clear out old entity if it exists. 
        if (classEntity != nullptr) {
            // Notify and give it a chance to clean up before its actual deallocation takes place.
            classEntity->OnDeallocate();

            // Delete object.
            delete classEntity;
            classEntity = nullptr;
        }
        
        // Create new class entity object.
        classEntity = new T(clgEntity);

        // Insert it at state.number index.
        classEntities.emplace(classEntities.begin() + state.number, dynamic_cast<T*>(classEntity));

        // Return pointer.
        return dynamic_cast<T*>(classEntity);
    }

    /**
    *   @brief  Clears the list by deallocating all its members.
    **/
    void Clear();

    /**
    *   @return A pointer to the entity who's index matches the state number.
    **/
    CLGBaseEntity *GetByStateNumber(int32_t number);

    /**
    *   @brief  Inserts the class entity pointer at the number index of our class entity vector.
    *   @return Pointer to the entity being inserted.
    **/
    CLGBaseEntity *InsertAtSlotNumber(CLGBaseEntity *clgEntity, int32_t number);

private:
    //! First 2048 are reserved for server side entities.
    CLGEntityVector classEntities;
};