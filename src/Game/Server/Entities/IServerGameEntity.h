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
//#include "../../Shared/SharedGame.h"
// SharedGame Entity Interface.
#include "../../Shared/Entities/ISharedGameEntity.h"


class ISharedGameEntity;

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
    *
    * 
    *   ServerGame ONLY GameEntity Interface Functions.
    *
    * 
    **/
	//!
	//!	None as of yet.
	//!


    /**
    *
    *
    *   Dispatch Callback Functionalities.
    *
    *
    **/
    //! 'Think' Callback Pointer. (Gets dispatched by an entity's Think method based on nextThinkTime.)
    using ThinkCallbackPointer      = void(IServerGameEntity::*)(void);
    //! 'Use' Callback Pointer.
    using UseCallbackPointer        = void(IServerGameEntity::*)(IServerGameEntity* other, IServerGameEntity* activator);
    //! 'Touch' Callback Pointer.
    using TouchCallbackPointer      = void(IServerGameEntity::*)(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    //! 'Blocked' Callback Pointer.
    using BlockedCallbackPointer    = void(IServerGameEntity::*)(IServerGameEntity* other);
    //! 'Damage' Callback Pointer.
    using TakeDamageCallbackPointer = void(IServerGameEntity::*)(IServerGameEntity* other, float kick, int32_t damage);
    //! 'Die' Callback Pointer.
    using DieCallbackPointer        = void(IServerGameEntity::*)(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);

    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  other:      
    *   @param  activator:  
    **/
    virtual void DispatchUseCallback(GameEntity* other, GameEntity* activator) = 0;
    /**
    *   @brief  Dispatches 'Die' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    virtual void DispatchDieCallback(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) = 0;
    /**
    *   @brief  Dispatches 'Blocked' callback.
    *   @param  other:  
    **/
    virtual void DispatchBlockedCallback(GameEntity* other) = 0;
    /**
    *   @brief  Dispatches 'Touch' callback.
    *   @param  self:   
    *   @param  other:  
    *   @param  plane:  
    *   @param  surf:   
    **/
    virtual void DispatchTouchCallback(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf) = 0;
    /**
    *   @brief  Dispatches 'TakeDamage' callback.
    *   @param  other:
    *   @param  kick:
    *   @param  damage:
    **/
    virtual void DispatchTakeDamageCallback(GameEntity* other, float kick, int32_t damage) = 0;



    /**
    *
    *
    *   For Physics Entities.
    *
    *
    **/
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetAcceleration() = 0;
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetDeceleration() = 0;
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetEndPosition() = 0;
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetSpeed() = 0;
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetStartPosition() = 0;



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
    inline const qboolean HasThinkCallback() {
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
    inline const qboolean HasUseCallback() {
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
    inline const qboolean HasTouchCallback() {
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
    inline const qboolean HasBlockedCallback() {
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
    inline const qboolean HasTakeDamageCallback() {
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
    inline const qboolean HasDieCallback() {
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