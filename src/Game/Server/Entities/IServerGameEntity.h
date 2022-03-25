/***
*
*	License here.
*
*	@file
*
*	ServerGame Entity Interface.
* 
***/
#pragma once

// ServerGame Exports Interface. TODO: Obviously, still implement.
//#include "Shared/Interfaces/IServerGameExports.h"

// GameLocals.
#include "../ServerGameLocals.h"
// SharedGame Entity Interface.
//#include "Game/Shared/SharedGame.h"




/**
*   IClientGameEntity
**/
class IServerGameEntity : public ISharedGameEntity {
public:
    /**
    *
    * 
    *   Constructor/Destructor AND TypeInfo related.
    *
    * 
    **/
    //! Constructor/Destructor.
    virtual ~IServerGameEntity() = default;

    //! Runtime type information
    DefineAbstractClass( IServerGameEntity, ISharedGameEntity );

    
    /**
    *   @brief  Checks if this entity class is exactly the same type of given entityClass.
    *   @param  entityClass:    an entity class which must be inherited from SVGBaseEntity.
    **/
    template<typename entityClass>
    bool IsClass() const { // every entity has a ClassInfo, thanks to the DefineXYZ macro
        return GetTypeInfo()->IsClass( entityClass::ClassInfo );
    }

    /**
    *   @brief  Checks if this entity class is a subclass of another, or is the same class
    *   @param  entityClass:    an entity class which must be inherited from SVGBaseEntity.
    **/
    template<typename entityClass>
    bool IsSubclassOf() const {
        return GetTypeInfo()->IsSubclassOf( entityClass::ClassInfo );
    }


    /**
    *
    * 
    *   SharedGame ClassEntity Interface Functions.
    *
    * 
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() = 0;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() = 0;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() = 0;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() = 0;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() = 0;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) = 0; // Called for each key:value when parsing the entity dictionary.



    /***
    *
    *   ServerGame Class Entity Functions.
    * 
    *   These functions are all implemented in CLGBaseEntity, some are purposely left empty to act
    *   as a stub. Just because they are empty doesn't mean one can't fill them in and make them tick.
    *   It does require one to have a proper understanding of the matter before doing so. Breaking things
    *   by doing so is easy.
    *
    ***/
    virtual void DispatchUseCallback(ClassEntity* other, ClassEntity* activator) = 0;
    virtual void DispatchDieCallback(ClassEntity* inflictor, ClassEntity* attacker, int damage, const vec3_t& point) = 0;
    virtual void DispatchBlockedCallback(ClassEntity* other) = 0;
    virtual void DispatchTouchCallback(ClassEntity* self, ClassEntity* other, CollisionPlane* plane, CollisionSurface* surf) = 0;
    virtual void DispatchTakeDamageCallback(ClassEntity* other, float kick, int32_t damage) = 0;



    using ThinkCallbackPointer      = void(IServerGameEntity::*)(void);
    using UseCallbackPointer        = void(IServerGameEntity::*)(IServerGameEntity* other, IServerGameEntity* activator);
    using TouchCallbackPointer      = void(IServerGameEntity::*)(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    using BlockedCallbackPointer    = void(IServerGameEntity::*)(IServerGameEntity* other);
    using TakeDamageCallbackPointer = void(IServerGameEntity::*)(IServerGameEntity* other, float kick, int32_t damage);
    using DieCallbackPointer        = void(IServerGameEntity::*)(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);


public:
    /**
    *
    *   Ugly, but effective callback SET methods.
    *
    **/
    /**
    *   @brief  Sets the 'Think' callback function.
    **/
    template<typename function>
    inline void SetThinkCallback(function f)
    {
        thinkFunction = static_cast<ThinkCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'Think' callback set. False if it is nullptr.
    **/
    inline qboolean HasThinkCallback() {
        return (thinkFunction != nullptr ? true : false);
    }

    /**
    *   @brief  Sets the 'Use' callback function.
    **/
    template<typename function>
    inline void SetUseCallback(function f)
    {
        useFunction = static_cast<UseCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'Use' callback set. False if it is nullptr.
    **/
    inline qboolean HasUseCallback() {
        return (useFunction != nullptr ? true : false);
    }

    /**
    *   @brief  Sets the 'Touch' callback function.
    **/
    template<typename function>
    inline void SetTouchCallback(function f)
    {
        touchFunction = static_cast<TouchCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'Touch' callback set. False if it is nullptr.
    **/
    inline qboolean HasTouchCallback() {
        return (touchFunction != nullptr ? true : false);
    }

    /**
    *   @brief  Sets the 'Blocked' callback function.
    **/
    template<typename function>
    inline void SetBlockedCallback(function f)
    {
        blockedFunction = static_cast<BlockedCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'Blocked' callback set. False if it is nullptr.
    **/
    inline qboolean HasBlockedCallback() {
        return (takeDamageFunction != nullptr ? true : false);
    }

    /**
    *   @brief  Sets the 'SetTakeDamage' callback function.
    **/
    template<typename function>
    inline void SetTakeDamageCallback(function f)
    {
        takeDamageFunction = static_cast<TakeDamageCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'TakeDamage' callback set. False if it is nullptr.
    **/
    inline qboolean HasTakeDamageCallback() {
        return (takeDamageFunction != nullptr ? true : false);
    }

    /**
    *   @brief  Sets the 'Die' callback function.
    **/
    template<typename function>
    inline void SetDieCallback(function f)
    {
        dieFunction = static_cast<DieCallbackPointer>(f);
    }
    /**
    *   @return True if it has a 'Die' callback set. False if it is nullptr.
    **/
    inline qboolean HasDieCallback() {
        return (dieFunction != nullptr ? true : false);
    }


protected:
    /**
    *   Dispatch Callback Function Pointers.
    **/
    ThinkCallbackPointer        thinkFunction       = nullptr;
    UseCallbackPointer          useFunction         = nullptr;
    TouchCallbackPointer        touchFunction       = nullptr;
    BlockedCallbackPointer      blockedFunction     = nullptr;
    TakeDamageCallbackPointer   takeDamageFunction  = nullptr;
    DieCallbackPointer          dieFunction         = nullptr;
private:

};