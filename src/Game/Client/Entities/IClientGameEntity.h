/***
*
*	License here.
*
*	@file
*
*	ClientGame Entity Interface.
* 
***/
#pragma once


/**
*   IClientGameEntity
**/
class IClientGameEntity : public ISharedGameEntity {
public:
    /**
    *
    * 
    *   Constructor/Destructor AND TypeInfo related.
    *
    * 
    **/
    //! Constructor/Destructor.
    virtual ~IClientGameEntity() = default;

    //! Runtime type information
    DefineAbstractClass( IClientGameEntity, ISharedGameEntity );



    /**
    *
    * 
    *   ClientGame ONLY GameEntity Interface Functions.
    *
    * 
    **/
	//!
	//!	None as of yet.
	//!



    /***
    *
    * 
    *   ClientGame Entity Functions.
    *
    * 
    ***/
    /**
    *   @brief  
    **/

    /**
    *   @brief  Updates the entity with the data of the newly passed EntityState object.
    **/
    virtual void UpdateFromState(const EntityState &state) = 0;

    /**
    *   @returen True if the entity is still in the current frame.
    **/
    virtual const qboolean  IsInUse() = 0;


    /**
    *   @brief  Sets the classname of this entity.
    **/
    virtual void SetClassname(const std::string& classname) = 0;

    /**
    *   @return A string containing the entity's classname.
    **/
    virtual const std::string GetClassname() = 0;
    /**
    *   @return An uint32_t containing the hashed classname string.
    **/
    virtual uint32_t GetHashedClassname() = 0;

    /**
    *   @brief  Sets a pointer referring to this class' client entity.
    **/
    virtual void SetPODEntity(PODEntity* podEntity) = 0;

    /**
    *   @return The pointer referring to this class' client entity.
    **/
    virtual PODEntity* GetPODEntity() = 0;


    /***
    *
    * 
    *   OnEventCallbacks.
    *
    * 
    ***/
    /**
    *   @brief  Gets called right before the moment of deallocation happens.
    **/
    virtual void OnDeallocate() = 0;



    /**
    *
    *
    *   Dispatch Callback Functionalities.
    *
    *
    **/
    //! 'Think' Callback Pointer. (Gets dispatched by an entity's Think method based on nextThinkTime.)
    using ThinkCallbackPointer      = void(IClientGameEntity::*)(void);
    //! 'Use' Callback Pointer.
    using UseCallbackPointer        = void(IClientGameEntity::*)(IClientGameEntity* other, IClientGameEntity* activator);
    //! 'Touch' Callback Pointer.
    using TouchCallbackPointer      = void(IClientGameEntity::*)(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    //! 'Blocked' Callback Pointer.
    using BlockedCallbackPointer    = void(IClientGameEntity::*)(IClientGameEntity* other);
    //! 'Damage' Callback Pointer.
    using TakeDamageCallbackPointer = void(IClientGameEntity::*)(IClientGameEntity* other, float kick, int32_t damage);
    //! 'Die' Callback Pointer.
    using DieCallbackPointer        = void(IClientGameEntity::*)(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point);

    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  other:      
    *   @param  activator:  
    **/
    virtual void DispatchUseCallback(GameEntity* other, GameEntity* activator) = 0;
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    virtual void DispatchDieCallback(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) = 0;
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  other:  
    **/
    virtual void DispatchBlockedCallback(GameEntity* other) = 0;
    /**
    *   @brief  Dispatches 'Block' callback.
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
    inline const qboolean HasThinkCallback() override {
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
    inline const qboolean HasUseCallback() override {
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
    inline const qboolean HasTouchCallback() override {
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
    inline const qboolean HasBlockedCallback() override {
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
    inline const qboolean HasTakeDamageCallback() override {
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
    inline const qboolean HasDieCallback() override {
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
};