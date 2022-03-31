/***
*
*	License here.
*
*	@file
*
*	Client Game EntityList implementation.
* 
***/
#include "../../ClientGameLocals.h"
#include "../../Main.h"

// Base Entity.
#include "CLGBaseEntity.h"



//! Used for returning vectors from a const vec3_t & reference.
vec3_t CLGBaseEntity::ZeroVec3 = vec3_zero();

//! Used for returning strings from a const std::string & reference.
std::string CLGBaseEntity::EmptyString = "";

/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
CLGBaseEntity::CLGBaseEntity(ClientEntity* clEntity) : IClientGameEntity(), podEntity(clEntity) {
    
}



/**
*
*   Client Class Entity Interface Functions.
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void CLGBaseEntity::Precache() {
}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void CLGBaseEntity::Spawn() {

}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void CLGBaseEntity::Respawn() {

}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void CLGBaseEntity::PostSpawn() {

}

/**
*   @brief  General entity thinking routine.
**/
void CLGBaseEntity::Think() {
	// Safety check.
    if (thinkFunction == nullptr) {
		return;
    }

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

/**
*   @brief  Act upon the parsed key and value.
**/
void CLGBaseEntity::SpawnKey(const std::string& key, const std::string& value) {

}



/***
*
* 
*   Client Class Entity Functions.
*
* 
***/
/**
*   @brief  Updates the entity with the data of the newly passed EntityState object.
**/
void CLGBaseEntity::UpdateFromState(const EntityState& state) {
    previousState = currentState;
    currentState = state;
}

/**
*   @returen True if the entity is still in the current frame.
**/
//const qboolean CLGBaseEntity::IsInUse() {
//    if (podEntity) {
//        return cl->frame.number == podEntity->serverFrame;
//    } else {
//        false;
//    }
//}

/**
*   @brief  Stub.
**/
const std::string CLGBaseEntity::GetClassname() {
    // Returns this classname, the base entity.
    return GetTypeInfo()->classname;
}

/**
*   @return An uint32_t containing the hashed classname string.
**/
uint32_t CLGBaseEntity::GetHashedClassname() {
    return GetTypeInfo()->hashedMapClass;
}



/***
*
*   OnEventCallbacks.
*
***/
/**
*   @brief  Gets called right before the moment of deallocation happens.
**/
void CLGBaseEntity::OnDeallocate() {

}



/***
*
* 
*   Dispatch Callback Functions.
*
* 
***/
/**
*   @brief  Dispatches 'Use' callback.
*   @param  other:      
*   @param  activator:  
**/
void CLGBaseEntity::DispatchUseCallback(IClientGameEntity* other, IClientGameEntity* activator) {
	// Safety check.
	if (useFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*useFunction)(other, activator);
}

/**
*   @brief  Dispatches 'Die' callback.
*   @param  inflictor:  
*   @param  attacker:   
*   @param  damage:     
*   @param  pointer:    
**/
void CLGBaseEntity::DispatchDieCallback(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point) {
	// Safety check.
	if (dieFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*dieFunction)(inflictor, attacker, damage, point);
}


/**
*   @brief  Dispatches 'Blocked' callback.
*   @param  other:  
**/
void CLGBaseEntity::DispatchBlockedCallback(IClientGameEntity* other) {
	// Safety check.
	if (blockedFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*blockedFunction)(other);
}

/**
*   @brief  Dispatches 'Touch' callback.
*   @param  self:   
*   @param  other:  
*   @param  plane:  
*   @param  surf:   
**/
void CLGBaseEntity::DispatchTouchCallback(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// Safety check.
	if (touchFunction == nullptr)
		return;

	// Execute 'Touch' callback function.
	(this->*touchFunction)(self, other, plane, surf);
}

/**
*   @brief  Dispatches 'TakeDamage' callback.
*   @param  other:
*   @param  kick:
*   @param  damage:
**/
void CLGBaseEntity::DispatchTakeDamageCallback(IClientGameEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
}