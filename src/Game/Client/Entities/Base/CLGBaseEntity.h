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
#include "../IClientGameEntity.h"



/**
*   CLGBaseEntity
**/
class CLGBaseEntity : public IClientGameEntity {
public:
    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    CLGBaseEntity(ClientEntity *clEntity);
    virtual ~CLGBaseEntity() = default;

    // Runtime type information
	DefineMapClass( "CLGBaseEntity", CLGBaseEntity, IClientGameEntity);

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
    *   ClientGame Entity Interface Functions.
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
    * 
    *   ClientGame BaseEntity Functions.
    *
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
    *   @returen True if the entity is still in the current frame.
    **/
    //virtual const qboolean  IsInUse() final;

    /**
    *   @return A string containing the entity's classname.
    **/
    virtual const std::string GetClassname() final;
    /**
    *   @return An uint32_t containing the hashed classname string.
    **/
    virtual uint32_t GetHashedClassname() final;

    /**
    *   @return Pointer to the client/server side POD Entity.
    **/
    inline PODEntity* GetPODEntity() final {
        return podEntity;
    }
    /**
    *   @brief  Sets the pointer ot the client/server side POD Entity.
    *           Used only in SVG_FreeEntity and SVG_CreateClassEntity.
    **/
    inline void SetPODEntity(PODEntity* podEntity) {
        this->podEntity = podEntity;
    }



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
    virtual void OnDeallocate() override;



    /**
    *   [Stub Implementation]
    *   @brief  Sets classname.
    **/
    virtual void SetClassname(const std::string& classname) final {};
    /**
    *   [Stub Implementation]
    *   @brief  Link entity to world for collision testing using gi.LinkEntity.
    **/
    void LinkEntity() override {};
    /**
    *   [Stub Implementation]
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    void UnlinkEntity() override {};
    /**
    *   [Stub Implementation]
    *   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
    *           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
    *           on us during the current server game frame we're processing.
    **/
    void Remove() override {};



    /**
    *
    *   [Stub Implementation]
    *   Target Functions.
    *
    *
    **/
    /**
    *   @brief  Calls Use on this entity's targets, and deletes the kill target if any is set.
    *   @param  activatorOverride:  if nullptr, the entity's own activator is used and if the entity's own activator is nullptr, 
    *                               then this entity itself becomes the activator
    **/
    void UseTargets( ClassEntity* activatorOverride = nullptr ) {};



    /***
    *
    * 
    *   ClientGame Base Entity Set/Get:
    *
    * 
    ***/
    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    inline const GameTime &GetNextThinkTime() final { 
        //Com_DPrint("WTF NEXTTHINKTIME\n");
        return nextThinkTime;
    }
    inline void SetNextThinkTime(const Frametime &nextThinkTime) final {
        //Com_DPrint("WTF SETNEXTTHINKTIME: %f\n", nextThinkTime);
        this->nextThinkTime = duration_cast<GameTime>(nextThinkTime);
    };

    /**
    *
    *
    *   Base entity Set/Get Stubs:
    *
    *
    **/
    /**
    *   @returns The local center(world-space) of the entity's Bounding Box.
    **/
    virtual vec3_t          GetAbsoluteCenter() { return vec3_zero(); };

    /**
    *   @brief Get/Set: BoundingBox Mins
    **/
    virtual const vec3_t&   GetAbsoluteMin() { return vec3_zero(); };
    virtual void            SetAbsoluteMin(const vec3_t &absMin) {};

    /**
    *   @brief Get/Set: BoundingBox Maxs
    **/
    virtual const vec3_t&   GetAbsoluteMax() { return vec3_zero(); };
    virtual void            SetAbsoluteMax(const vec3_t &absMax) {};

    /**
    *   @brief Get/Set: Activator
    **/
    virtual ClassEntity*    GetActivator() { return nullptr; };
    virtual void            SetActivator(ClassEntity* activator) {};

    /**
    *   @brief Get/Set: Angles
    **/
    virtual const vec3_t&   GetAngles() { return vec3_zero(); };
    virtual void            SetAngles(const vec3_t& angles) {};

    /**
    *   @brief Get/Set: Angular Velocity
    **/
    virtual const vec3_t&   GetAngularVelocity() { return vec3_zero(); };
    virtual void            SetAngularVelocity(const vec3_t& angularVelocity) {};

    /**
    *   @return The local center(model-space) of the entity's Bounding Box.
    **/
    virtual vec3_t          GetCenter() { return vec3_zero(); };

    /**
    *   @brief Set: Mins and Maxs determining the entity's Bounding Box
    **/
    virtual void            SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) {};

    /**
    *   @brief Get/Set: Classname
    **/
    //virtual const std::string   GetClassname() { return ""; };
    //virtual void                SetClassname(const std::string &classname) {};

    /**
    *   @brief Get: Entity Client
    **/
    virtual gclient_s* GetClient() { return 0; };

    /**
    *   @brief Get/Set: Clip Mask
    **/
    virtual const int32_t   GetClipMask() { return 0; };
    virtual void            SetClipMask(const int32_t clipMask) {};

    /**
    *   @brief Get/Set: Count
    **/
    virtual const int32_t   GetCount() { return 0; };
    virtual void            SetCount(const int32_t count) {};

    /**
    *   @brief Get/Set: Damage
    **/
    virtual const int32_t   GetDamage() { return 0; };
    virtual void            SetDamage(const int32_t damage) {};

    /**
    *   @brief Get/Set: Dead Flag
    **/
    virtual const int32_t   GetDeadFlag() { return 0; };
    virtual void            SetDeadFlag(const int32_t deadFlag) {};

    /**
    *   @brief Get/Set: Delay Time
    **/
    virtual const Frametime&    GetDelayTime() { return delayTime; };
    virtual void                SetDelayTime(const Frametime &delayTime) { this->delayTime = delayTime; };

    /**
    *   @brief Get/Set: Effects
    **/
    virtual const uint32_t  GetEffects() { return 0; };
    virtual void            SetEffects(const uint32_t effects) {};

    /**
    *   @brief Get/Set: Enemy
    **/
    virtual ClassEntity*    GetEnemy() { return 0; };
    virtual void            SetEnemy(ClassEntity* enemy) {};

    /**
    *   @brief Get: Entity Dictionary.
    **/
    virtual EntityDictionary &GetEntityDictionary() { return podEntity->entityDictionary; };

    /**
    *   @brief Get/Set: Event ID
    **/
    virtual const uint8_t   GetEventID() { return 0; };
    virtual void            SetEventID(const uint8_t eventID) {};

    /**
    *   @brief Get/Set: Flags
    **/
    virtual const int32_t   GetFlags() { return 0; };
    virtual void            SetFlags(const int32_t flags) {};

    /**
    *   @brief Get/Set: Animation Frame
    **/
    virtual const float     GetAnimationFrame() { return 0.f; };
    virtual void            SetAnimationFrame(const float frame) {};

    /**
    *   @brief Get/Set: Gravity
    **/
    virtual const float     GetGravity() { return 0.f; };
    virtual void            SetGravity(const float gravity) {};

    /**
    *   @brief Get/Set: Ground Entity
    **/
    // TODO TODO TODO: Fix it so it returns the actual ground entity....
    virtual SGEntityHandle  GetGroundEntity() { return SGEntityHandle(); };
    virtual void            SetGroundEntity(ClassEntity* groundEntity) {};

    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    virtual int32_t         GetGroundEntityLinkCount() { return 0; };
    virtual void            SetGroundEntityLinkCount(int32_t groundEntityLinkCount) {};

    /**
    *   @brief Get/Set: Health
    **/
    virtual const int32_t   GetHealth() { return 0; };
    virtual void            SetHealth(const int32_t health) {};

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    virtual const float     GetIdealYawAngle() { return 0.f; };
    virtual void            SetIdealYawAngle(const float idealYawAngle) {};

    /**
    *   @brief Is/Set: In Use.
    **/
    virtual const qboolean        IsInUse() override { //return 0; };
        if (podEntity) {
            return cl->frame.number == podEntity->serverFrame;
        } else {
            false;
        }
    }
    virtual void            SetInUse(const qboolean inUse) {};

    /**
    *   @brief Get/Set: Kill Target.
    **/
    virtual const std::string&  GetKillTarget() { return ""; };
    virtual void                SetKillTarget(const std::string& killTarget) {};

    /**
    *   @brief Get/Set: Link Count.
    **/
    virtual const int32_t   GetLinkCount() { return 0; };
    virtual void            SetLinkCount(const int32_t linkCount) {};

    /**
    *   @brief Get/Set: Mass
    **/
    virtual int32_t         GetMass() { return 0; };
    virtual void            SetMass(const int32_t mass) {};

    /**
    *   @brief Get/Set: Max Health
    **/
    virtual const int32_t   GetMaxHealth() { return 0; };
    virtual void            SetMaxHealth(const int32_t maxHealth) {};

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    virtual const vec3_t&   GetMaxs() { return vec3_zero(); };
    virtual void            SetMaxs(const vec3_t& maxs) {};

    /**
    *   @brief Get/Set: Message
    **/
    virtual const std::string&  GetMessage() { return ""; };
    virtual void                SetMessage(const std::string& message) {};

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    virtual const vec3_t&   GetMins() { return vec3_zero(); };
    virtual void            SetMins(const vec3_t& mins) {};
   
    /**
    *   @brief Get/Set: Model
    **/
    virtual const std::string&  GetModel() { return ""; };
    virtual void                SetModel(const std::string &model) {};

    /**
    *   @brief Get/Set: Model Index 1
    **/
    virtual const int32_t   GetModelIndex() { return 0; };
    virtual void            SetModelIndex(const int32_t index) {};
    /**
    *   @brief Get/Set: Model Index 2
    **/
    virtual const int32_t   GetModelIndex2() { return 0; };
    virtual void            SetModelIndex2(const int32_t index) {};
    /**
    *   @brief Get/Set: Model Index 3
    **/
    virtual const int32_t   GetModelIndex3() { return 0; };
    virtual void            SetModelIndex3(const int32_t index) {};
    /**
    *   @brief Get/Set: Model Index 4
    **/
    virtual const int32_t   GetModelIndex4() { return 0; };
    virtual void            SetModelIndex4(const int32_t index) {};

    /**
    *   @brief Get/Set: Move Type.
    **/
    virtual const int32_t   GetMoveType() { return 0; };
    virtual void            SetMoveType(const int32_t moveType) {};

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    //virtual const float     GetNextThinkTime() { return 0.f; };
    //virtual void            SetNextThinkTime(const float nextThinkTime) {};

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    virtual const int32_t   GetNoiseIndexA() { return 0; };
    virtual void            SetNoiseIndexA(const int32_t noiseIndexA) {};

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    virtual const int32_t   GetNoiseIndexB() { return 0; };
    virtual void            SetNoiseIndexB(const int32_t noiseIndexB) {};

    /**
    *   @brief Get/Set:     State Number
    **/
    virtual const int32_t   GetNumber() { return 0; };
    virtual void            SetNumber(const int32_t number) {};

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    virtual ClassEntity*    GetOldEnemy() { return nullptr; }
    virtual void            SetOldEnemy(ClassEntity* oldEnemy) {};

    /**
    *   @brief Get/Set:     Old Origin
    **/
    virtual const vec3_t&   GetOldOrigin() { return vec3_zero(); };
    virtual void            SetOldOrigin(const vec3_t& oldOrigin) {};

    /**
    *   @brief Get/Set:     Origin
    **/
    virtual const vec3_t&   GetOrigin() { return vec3_zero(); };
    virtual void            SetOrigin(const vec3_t& origin) {};

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    virtual ClassEntity*    GetOwner() { return nullptr; };
    virtual void            SetOwner(ClassEntity* owner) {};

    /**
    *   @brief Get/Set:     Render Effects
    **/
    virtual const int32_t   GetRenderEffects() { return 0; };
    virtual void            SetRenderEffects(const int32_t renderEffects) {};
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    virtual const char*     GetPathTarget() { return ""; };

    /**
    *   @brief Get/Set:     Server Flags
    **/
    virtual const int32_t   GetServerFlags() { return 0; };
    virtual void            SetServerFlags(const int32_t serverFlags) {};

    /**
    *   @brief Get/Set:     Skin Number
    **/
    virtual const int32_t   GetSkinNumber() { return 0; };
    virtual void            SetSkinNumber(const int32_t skinNumber) {};

    /**
    *   @brief Get/Set:     Entity Size
    **/
    virtual const vec3_t&   GetSize() { return vec3_zero(); };
    virtual void            SetSize(const vec3_t& size) {};

    /**
    *   @brief Get/Set:     Solid
    **/
    virtual const uint32_t  GetSolid() { return 0; };
    virtual void            SetSolid(const uint32_t solid) {};

    /**
    *   @brief Get/Set:     Sound.
    **/
    virtual const int32_t   GetSound() { return 0; };
    virtual void            SetSound(const int32_t sound) {};

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    virtual const int32_t   GetSpawnFlags() { return 0; };
    virtual void            SetSpawnFlags(const int32_t spawnFlags) {};

    /**
    *   @brief Get/Set:     Entity State
    **/
    virtual const EntityState&   GetState() { return podEntity->current; };
    virtual void                 SetState(const EntityState &state) { podEntity->current = state; };

    /**
    *   @brief Get/Set:     Style
    **/
    virtual const int32_t   GetStyle() { return 0; };
    virtual void            SetStyle(const int32_t style) {};

    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const int32_t   GetTakeDamage() { return 0; };
    virtual void            SetTakeDamage(const int32_t takeDamage) {};
    
    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const std::string&   GetTarget() { return ""; };
    virtual void                 SetTarget(const std::string& target) {};

    /**
    *   @brief Get/Set:     Target Name
    **/
    virtual const std::string&   GetTargetName() { return ""; };
    virtual void                 SetTargetName(const std::string& targetName) {};

    /**
    *   @brief Get/Set:     Team
    **/
    virtual const std::string&   GetTeam() { return ""; };
    virtual void                 SetTeam(const std::string &team) {};

    /**
    *   @brief Get/Set:     Team Chain
    **/
    virtual ClassEntity*    GetTeamChainEntity() { return nullptr; }
    virtual void            SetTeamChainEntity(ClassEntity* entity) {};

    /**
    *   @brief Get/Set:     Team Master
    **/
    virtual ClassEntity*    GetTeamMasterEntity() { return nullptr; };
    virtual void            SetTeamMasterEntity(ClassEntity* entity) {};

    /**
    *   @brief Get/Set:     Velocity
    **/
    virtual const vec3_t&   GetVelocity() { return vec3_zero(); };
    virtual void            SetVelocity(const vec3_t &velocity) {};

    /**
    *   @brief Get/Set:     View Height
    **/
    virtual const int32_t   GetViewHeight() { return 0; };
    virtual void            SetViewHeight(const int32_t height) {};

    /**
    *   @brief Get/Set:     Wait Time
    **/
    virtual const Frametime&    GetWaitTime() { return waitTime; }
    virtual void                SetWaitTime(const Frametime &waitTime) { this->waitTime = waitTime; };

    /**
    *   @brief Get/Set:     Water Level
    **/
    virtual const int32_t   GetWaterLevel() { return 0; };
    virtual void            SetWaterLevel(const int32_t waterLevel) {};

    /**
    *   @brief Get/Set:     Water Type
    **/
    virtual const int32_t   GetWaterType() { return 0; }
    virtual void            SetWaterType(const int32_t waterType) {};

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    virtual const float     GetYawSpeed() { return 0.f; };
    virtual void            SetYawSpeed(const float yawSpeed) {};



private:
    //! Pointer to the client entity which owns this class entity.
    ClientEntity *podEntity = nullptr;

    //! Refresh Entity Object.
    r_entity_t refreshEntity = {};

    // Client Class Entities maintain their own states. (Get copied in from updates.)
    EntityState currentState = {};
    EntityState previousState = {};



private:
    /**
    *   Entity 'Timing'
    **/
    //! The next 'think' time, determines when to call the 'think' callback.
    GameTime nextThinkTime = GameTime::zero();
    //! Delay before calling trigger execution.
    Frametime delayTime = Frametime::zero();
    //! Wait time before triggering at all, in case it was set to auto.
    Frametime waitTime = Frametime::zero();



public:
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
    void DispatchUseCallback(ClassEntity* other, ClassEntity* activator);
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    void DispatchDieCallback(ClassEntity* inflictor, ClassEntity* attacker, int damage, const vec3_t& point);
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  other:  
    **/
    void DispatchBlockedCallback(ClassEntity* other);
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  self:   
    *   @param  other:  
    *   @param  plane:  
    *   @param  surf:   
    **/
    void DispatchTouchCallback(ClassEntity* self, ClassEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    /**
    *   @brief  Dispatches 'TakeDamage' callback.
    *   @param  other:
    *   @param  kick:
    *   @param  damage:
    **/
    void DispatchTakeDamageCallback(ClassEntity* other, float kick, int32_t damage);



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
};