/***
*
*	License here.
*
*	@file
*
*	ServerGame Base Entity implementation. All entities in the ServerGame module
*   derive from this class.
* 
*   It supplies you with get/set methods, optionable callbacks to set and dispatch.
* 
***/
#pragma once

// Forward declare the entity handle object class.
class SGEntityHandle;

/**
*   SVGBaseEntity
**/
class SVGBaseEntity : public IServerGameEntity {
public:
    /**
    *
    *
    *   FunctionPointers for Dispatch Callbacks.
    *
    *
    **/
    //using ThinkCallbackPointer      = void(SVGBaseEntity::*)(void);
    //using UseCallbackPointer        = void(SVGBaseEntity::*)(IServerGameEntity* other, IServerGameEntity* activator);
    //using TouchCallbackPointer      = void(SVGBaseEntity::*)(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    //using BlockedCallbackPointer    = void(SVGBaseEntity::*)(IServerGameEntity* other);
    //using TakeDamageCallbackPointer = void(SVGBaseEntity::*)(IServerGameEntity* other, float kick, int32_t damage);
    //using DieCallbackPointer        = void(SVGBaseEntity::*)(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);



    /**
    *
    *
    *   Constructor/Destructor & TypeInfo Utilities.
    *
    *
    **/
    //! Constructor/Deconstructor.
    SVGBaseEntity(PODEntity *svEntity);
    virtual ~SVGBaseEntity() = default;

    //! Runtime type information.
    DefineClass( SVGBaseEntity, IServerGameEntity);

    ///**
    //*   @brief  Checks if this entity class is exactly the same type of given entityClass.
    //*   @param  entityClass:    an entity class which must be inherited from SVGBaseEntity.
    //**/
    //template<typename entityClass>
    //bool IsClass() const { // every entity has a ClassInfo, thanks to the DefineXYZ macro
    //    return GetTypeInfo()->IsClass( entityClass::ClassInfo );
    //}

    ///**
    //*   @brief  Checks if this entity class is a subclass of another, or is the same class
    //*   @param  entityClass:    an entity class which must be inherited from SVGBaseEntity.
    //**/
    //template<typename entityClass>
    //bool IsSubclassOf() const {
    //    return GetTypeInfo()->IsSubclassOf( entityClass::ClassInfo );
    //}
     


    //! Used for returning vectors from a const vec3_t & reference.
    static vec3_t ZeroVec3;

    /**
    *
    *
    *   Entity Interface Implementations.
    *
    *
    **/
    /**
    *   @brief  Called during load time right before spawning the actual entities.
    *           Used to "pre-""cache" all media data.
    **/
    virtual void Precache() override;    // Precaches data.

    /**
    *   @brief  Called when this entity is ready to spawn. Used to setup the entity
    *           and prepare it for gameplay.
    * 
    *           DO NOT try and refer to other entities here. Use 'PostSpawn' instead.
    **/
    virtual void Spawn() override;       // Spawns the entity.

    /**
    *   @brief  Called when this entity is about to respawn.
    **/
    virtual void Respawn() override;     // Respawns the entity.

    /**
    *   @brief  When all other entities have had a chance to spawn, they get a call to
    *           'PostSpawn' where if needed, a reference to other entities can be made.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.

    /**
    *   @brief  Called each server game frame to let the entity 'think'. This function
    *           in return dispatches the set think callback method when its nextThink
    *           time decides it is time to do so.
    **/
    virtual void Think() override;       // General entity thinking routine.

    /**
    *   @brief  Used to either parse, or even ignore, key/value pairs that are set for
    *           an entity using the map editor.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.



    /**
    *
    *
    *   ServerGame Entity Functions.
    *
    *
    **/
    /**
    *   @brief  Link entity to world for collision testing using gi.LinkEntity.
    **/
    virtual void LinkEntity() override;
    /**
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    virtual void UnlinkEntity() override;
    
    /**
    *   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
    *           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
    *           on us during the current server game frame we're processing.
    **/
    virtual void Remove() override;

    /**
    *   @return Pointer to the server side entity.
    **/
    virtual Entity* GetPODEntity() final {
        return podEntity;
    }
    /**
    *   @brief  Used only in SVG_FreeEntity and SVG_CreateGameEntity.
    *   @return Pointer to the server side entity.
    **/
    virtual void SetPODEntity(PODEntity *svEntity) final {
        podEntity = svEntity;
    }



    /**
    *
    *
    *   Callback Functions to Dispatch.
    *
    *
    **/
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  other:      
    *   @param  activator:  
    **/
    virtual void DispatchUseCallback(IServerGameEntity* other, IServerGameEntity* activator) override;
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    virtual void DispatchDieCallback(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point) override;
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  other:  
    **/
    virtual void DispatchBlockedCallback(IServerGameEntity* other) override;
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  self:   
    *   @param  other:  
    *   @param  plane:  
    *   @param  surf:   
    **/
    virtual void DispatchTouchCallback(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) override;
    /**
    *   @brief  Dispatches 'TakeDamage' callback.
    *   @param  other:
    *   @param  kick:
    *   @param  damage:
    **/
    virtual void DispatchTakeDamageCallback(IServerGameEntity* other, float kick, int32_t damage) override;
    /**
    *   @brief  Dispatches 'Stop' callback.
    **/
    virtual void DispatchStopCallback() override;



    /**
    *
    *
    *   Target Functions.
    *
    *
    **/
    /**
    *   @brief  Calls Use on this entity's targets, and deletes the kill target if any is set.
    *   @param  activatorOverride:  if nullptr, the entity's own activator is used and if the entity's own activator is nullptr, 
    *                               then this entity itself becomes the activator
    **/
    virtual void UseTargets( IServerGameEntity* activatorOverride = nullptr ) override;



    /**
    *
    *
    *   Base entity Set/Get:
    *
    *
    **/
    /**
    *   @returns The local center(world-space) of the entity's Bounding Box.
    **/
    virtual vec3_t GetAbsoluteCenter() override {
        return vec3_scale( GetAbsoluteMax() + GetAbsoluteMin(), 0.5f );
    }

    /**
    *   @brief Get/Set: BoundingBox Mins
    **/
    virtual const vec3_t&    GetAbsoluteMin() override { return podEntity->absMin; }
    virtual void             SetAbsoluteMin(const vec3_t &absMin) override { podEntity->absMin = absMin; }

    /**
    *   @brief Get/Set: BoundingBox Maxs
    **/
    virtual const vec3_t&    GetAbsoluteMax() override { return podEntity->absMax; }
    virtual void             SetAbsoluteMax(const vec3_t &absMax) override { podEntity->absMax = absMax; }

    /**
    *   @brief Get/Set: Activator
    **/
    virtual IServerGameEntity*   GetActivator() override { return activatorEntityPtr; }
    virtual void                         SetActivator(IServerGameEntity* activator) override { this->activatorEntityPtr = activator; }

    /**
    *   @brief Get/Set: Angles
    **/
    virtual const vec3_t&    GetAngles() override { return podEntity->currentState.angles; }
    virtual void             SetAngles(const vec3_t& angles) override { podEntity->currentState.angles = angles; }

    /**
    *   @brief Get/Set: Angular Velocity
    **/
    virtual const vec3_t&    GetAngularVelocity() override { return angularVelocity; }
    virtual void             SetAngularVelocity(const vec3_t& angularVelocity) override { this->angularVelocity = angularVelocity; }

    /**
    *   @return The local center(model-space) of the entity's Bounding Box.
    **/
    virtual vec3_t GetCenter() override { return vec3_scale( GetMaxs() + GetMins(), 0.5f ); }

    /**
    *   @brief Set: Mins and Maxs determining the entity's Bounding Box
    **/
    virtual void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) override {
        podEntity->mins = mins;
        podEntity->maxs = maxs;
    }

    /**
    *   @brief Get/Set: Classname
    **/
    virtual const std::string GetClassname() override { return classname; }
    virtual void SetClassname(const std::string &classname) override { this->classname = classname; }

    /**
    *   @brief Get: Entity Client
    **/
    gclient_s* GetClient() override {
        return podEntity->client;
    }

    /**
    *   @brief Get/Set: Clip Mask
    **/
    virtual const int32_t    GetClipMask() override { return podEntity->clipMask; }
    virtual void             SetClipMask(const int32_t clipMask) override { podEntity->clipMask = clipMask; }

    /**
    *   @brief Get/Set: Count
    **/
    virtual const int32_t    GetCount() override { return count; }
    virtual void             SetCount(const int32_t count) override { this->count = count; }

    /**
    *   @brief Get/Set: Damage
    **/
    virtual const int32_t    GetDamage() override { return damage; }
    virtual void             SetDamage(const int32_t damage) override { this->damage = damage; }

    /**
    *   @brief Get/Set: Dead Flag
    **/
    virtual const int32_t    GetDeadFlag() override { return deadFlag; }
    virtual void             SetDeadFlag(const int32_t deadFlag) override { this->deadFlag = deadFlag; }

    /**
    *   @brief Get/Set: Delay Time
    **/
    virtual const Frametime& GetDelayTime() override { return delayTime; }
    virtual void             SetDelayTime(const Frametime& delayTime) override { this->delayTime = delayTime; }

    /**
    *   @brief Get/Set: Effects
    **/
    virtual const uint32_t   GetEffects() override { return podEntity->currentState.effects; }
    virtual void             SetEffects(const uint32_t effects) override { podEntity->currentState.effects = effects; }

    /**
    *   @brief Get/Set: Enemy
    **/
    virtual GameEntity*   GetEnemy() override { return geEnemyEntity; }
    virtual void          SetEnemy(GameEntity* enemy) override { this->geEnemyEntity = enemy; }

    /**
    *   @brief Get: Entity Dictionary.
    **/
    virtual SpawnKeyValues &GetEntityDictionary() override { return podEntity->spawnKeyValues; }

    /**
    *   @brief Get/Set: Event ID
    **/
    virtual const uint8_t    GetEventID() override { return podEntity->currentState.eventID; }
    virtual void             SetEventID(const uint8_t eventID) override { podEntity->currentState.eventID = eventID; }

    /**
    *   @brief Get/Set: Flags
    **/
    virtual const int32_t    GetFlags() override { return flags; }
    virtual void             SetFlags(const int32_t flags) override { this->flags = flags; }

    /**
    *   @brief Get/Set: Animation Frame
    **/
    virtual const float      GetAnimationFrame() override { return podEntity->currentState.animationFrame; }
    virtual void             SetAnimationFrame(const float frame) override { podEntity->currentState.animationFrame = frame; }

    /**
    *   @brief Get/Set: Gravity
    **/
    virtual const float      GetGravity() override { return gravity; }
    virtual void             SetGravity(const float gravity) override { this->gravity = gravity; }

    /**
    *   @brief Get/Set: Ground Entity
    **/
    virtual SGEntityHandle   &GetGroundEntityHandle() override { return groundEntityHandle; }
	virtual PODEntity		*GetGroundPODEntity() override { return groundEntityHandle.Get(); }
	virtual void             SetGroundEntity(const SGEntityHandle &ehGroundEntity) { groundEntityHandle = ehGroundEntity; } //this->groundEntity = groundEntity; }

    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    virtual int32_t          GetGroundEntityLinkCount() override { return groundEntityLinkCount; }
    virtual void             SetGroundEntityLinkCount(int32_t groundEntityLinkCount) { this->groundEntityLinkCount = groundEntityLinkCount; }

    /**
    *   @brief Get/Set: Health
    **/
    virtual const int32_t    GetHealth() override { return health; }
    virtual void             SetHealth(const int32_t health) override { this->health = health; }

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    virtual const float      GetIdealYawAngle() override { return idealYawAngle; }
    virtual void             SetIdealYawAngle(const float idealYawAngle) override { this->idealYawAngle = idealYawAngle; }

    /**
    *   @brief Is/Set: In Use.
    **/
    virtual const qboolean   IsInUse() override { return podEntity->inUse; }
    virtual void             SetInUse(const qboolean inUse) override { podEntity->inUse = inUse; }

    /**
    *   @brief Get/Set: Kill Target.
    **/
    virtual const std::string&   GetKillTarget() override { return killTargetStr; }
    virtual void                 SetKillTarget(const std::string& killTarget) override { this->killTargetStr = killTarget; }

    /**
    *   @brief Get/Set: Link Count.
    **/
    virtual const int32_t    GetLinkCount() override { return (podEntity ? podEntity->linkCount : 0); }
    virtual void             SetLinkCount(const int32_t linkCount) override { podEntity->linkCount = linkCount; }

    /**
    *   @brief Get/Set: Mass
    **/
    virtual int32_t          GetMass() override { return mass; }
    virtual void             SetMass(const int32_t mass) override { this->mass = mass; }

    /**
    *   @brief Get/Set: Max Health
    **/
    virtual const int32_t    GetMaxHealth() override { return maxHealth; }
    virtual void             SetMaxHealth(const int32_t maxHealth) override { this->maxHealth = maxHealth; }

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    virtual const vec3_t&    GetMaxs() override { return podEntity->maxs; }
    virtual void             SetMaxs(const vec3_t& maxs) override { podEntity->maxs = maxs; }

    /**
    *   @brief Get/Set: Message
    **/
    virtual const std::string&   GetMessage() override { return messageStr; }
    virtual void                 SetMessage(const std::string& message) override { this->messageStr = message; }

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    virtual const vec3_t&    GetMins() override { return podEntity->mins; }
    virtual void             SetMins(const vec3_t& mins) override { podEntity->mins = mins; }
   
    /**
    *   @brief Get/Set: Model
    **/
    virtual const std::string&   GetModel() override { return model; }
    virtual void                 SetModel(const std::string &model) override {
        // Set model.
        this->model = model;

        // Set the model.
        gi.SetModel(podEntity, model.c_str());

        // Set model index.
        SetModelIndex(gi.PrecacheModel(model.c_str()));
    }

    /**
    *   @brief Get/Set: Model Index 1
    **/
    virtual const int32_t    GetModelIndex() override { return podEntity->currentState.modelIndex;  }
    virtual void             SetModelIndex(const int32_t index) override { podEntity->currentState.modelIndex = index;  }
    /**
    *   @brief Get/Set: Model Index 2
    **/
    virtual const int32_t    GetModelIndex2() override { return podEntity->currentState.modelIndex2; }
    virtual void             SetModelIndex2(const int32_t index) override { podEntity->currentState.modelIndex2 = index; }
    /**
    *   @brief Get/Set: Model Index 3
    **/
    virtual const int32_t    GetModelIndex3() override { return podEntity->currentState.modelIndex3; }
    virtual void             SetModelIndex3(const int32_t index) override { podEntity->currentState.modelIndex3 = index; }
    /**
    *   @brief Get/Set: Model Index 4
    **/
    virtual const int32_t    GetModelIndex4() override { return podEntity->currentState.modelIndex4; }
    virtual void SetModelIndex4(const int32_t index) override { podEntity->currentState.modelIndex4 = index; }

    /**
    *   @brief Get/Set: Move Type.
    **/
    virtual const int32_t    GetMoveType() override { return moveType; } 
    virtual void             SetMoveType(const int32_t moveType) override { this->moveType = moveType; }

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    virtual const GameTime&  GetNextThinkTime() override { return nextThinkTime; }
    virtual void             SetNextThinkTime(const Frametime& nextThinkTime) override { this->nextThinkTime = duration_cast<GameTime>(nextThinkTime); }

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    virtual const int32_t    GetNoiseIndexA() override { return noiseIndexA; }
    virtual void             SetNoiseIndexA(const int32_t noiseIndexA) override { this->noiseIndexA = noiseIndexA; }

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    virtual const int32_t    GetNoiseIndexB() override { return noiseIndexB; }
    virtual void             SetNoiseIndexB(const int32_t noiseIndexB) override { this->noiseIndexB = noiseIndexB; }

    /**
    *   @brief Get/Set:     State Number
    **/
    virtual const int32_t    GetNumber() override { return podEntity->currentState.number; }
    virtual void             SetNumber(const int32_t number) override { podEntity->currentState.number = number; }

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    virtual IServerGameEntity*   GetOldEnemy() override { return oldEnemyEntity; }
    virtual void                 SetOldEnemy(IServerGameEntity* oldEnemy) override { this->oldEnemyEntity = oldEnemy; }

    /**
    *   @brief Get/Set:     Old Origin
    **/
    virtual const vec3_t&    GetOldOrigin() override { return podEntity->currentState.oldOrigin; }
    virtual void             SetOldOrigin(const vec3_t& oldOrigin) override { podEntity->currentState.oldOrigin = oldOrigin; }

    /**
    *   @brief Get/Set:     Origin
    **/
    virtual const vec3_t&    GetOrigin() override { return podEntity->currentState.origin; }
    virtual void             SetOrigin(const vec3_t& origin) override { podEntity->currentState.origin = origin; }

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    virtual IServerGameEntity*   GetOwner() override { return this->ownerEntity; }
    virtual void                 SetOwner(IServerGameEntity* owner) override { this->ownerEntity = owner; }

    /**
    *   @brief Get/Set:     Render Effects
    **/
    virtual const int32_t    GetRenderEffects() override { return podEntity->currentState.renderEffects; }
    virtual void             SetRenderEffects(const int32_t renderEffects) override { podEntity->currentState.renderEffects = renderEffects; }
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    virtual const char* GetPathTarget() override { return nullptr; }

    /**
    *   @brief Get/Set:     Server Flags
    **/
    virtual const int32_t    GetServerFlags() override { return podEntity->serverFlags; }
    virtual void             SetServerFlags(const int32_t serverFlags) override { podEntity->serverFlags = serverFlags; }

    /**
    *   @brief Get/Set:     Skin Number
    **/
    virtual const int32_t    GetSkinNumber() override { return podEntity->currentState.skinNumber; }
    virtual void             SetSkinNumber(const int32_t skinNumber) override { podEntity->currentState.skinNumber = skinNumber; }

    /**
    *   @brief Get/Set:     Entity Size
    **/
    virtual const vec3_t&    GetSize() override { return podEntity->size; }
    virtual void             SetSize(const vec3_t& size) override { podEntity->size = size; }

    /**
    *   @brief Get/Set:     Solid
    **/
    virtual const uint32_t   GetSolid() override { return podEntity->solid; }
    virtual void             SetSolid(const uint32_t solid) override { podEntity->solid = solid; }

    /**
    *   @brief Get/Set:     Sound.
    **/
    virtual const int32_t GetSound() override { return podEntity->currentState.sound; }
    virtual void SetSound(const int32_t sound) override { podEntity->currentState.sound = sound; }

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    virtual const int32_t    GetSpawnFlags() override { return spawnFlags; }
    virtual void             SetSpawnFlags(const int32_t spawnFlags) override { this->spawnFlags = spawnFlags; }

    /**
    *   @brief Get/Set:     Entity State
    **/
    virtual const EntityState&   GetState() override { return podEntity->currentState; }
    virtual void                 SetState(const EntityState &state) override { podEntity->currentState = state; }

    /**
    *   @brief Get/Set:     Style
    **/
    virtual const int32_t    GetStyle() override { return style; }
    virtual void             SetStyle(const int32_t style) override { this->style = style; }

    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const int32_t    GetTakeDamage() override { return takeDamage; }
    virtual void             SetTakeDamage(const int32_t takeDamage) override { this->takeDamage = takeDamage; }
    
    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const std::string&   GetTarget() override { return targetStr; }
    virtual void                 SetTarget(const std::string& target) override { this->targetStr = target; }

    /**
    *   @brief Get/Set:     Target Name
    **/
    virtual const std::string&   GetTargetName() override { return targetNameStr; }
    virtual void                 SetTargetName(const std::string& targetName) override { this->targetNameStr = targetName; }

    /**
    *   @brief Get/Set:     Team
    **/
    virtual const std::string&   GetTeam() override { return teamStr; }
    virtual void                 SetTeam(const std::string &team) override { this->teamStr = team; }

    /**
    *   @brief Get/Set:     Team Chain
    **/
    virtual IServerGameEntity*   GetTeamChainEntity() override { return teamChainEntity; }
    virtual void                 SetTeamChainEntity(IServerGameEntity* entity) override { teamChainEntity = entity; }

    /**
    *   @brief Get/Set:     Team Master
    **/
    virtual IServerGameEntity*   GetTeamMasterEntity() override { return teamMasterEntity; }
    virtual void                 SetTeamMasterEntity(IServerGameEntity* entity) override { teamMasterEntity = entity; }

	/**
	*	@brief Get/Set:     Use Flags that determine if, and how a player can use this entity. (Toggle, continuous, single button.)
	**/
	virtual const int32_t        GetUseEntityFlags() { return useEntityFlags; }
	virtual void                 SetUseEntityFlags(const int32_t useFlags) { this->useEntityFlags = useFlags; }

    /**
    *   @brief Get/Set:     Velocity
    **/
    virtual const vec3_t& GetVelocity() override { return velocity; }
    virtual void SetVelocity(const vec3_t &velocity) override { this->velocity = velocity; }

    /**
    *   @brief Get/Set:     View Height
    **/
    virtual const int32_t    GetViewHeight() override { return viewHeight; }
    virtual void             SetViewHeight(const int32_t height) override { this->viewHeight = height; }

    /**
    *   @brief Get/Set:     Wait Time
    **/
    virtual const Frametime& GetWaitTime() override { return waitTime; }
    virtual void             SetWaitTime(const Frametime &waitTime) override { this->waitTime = waitTime; }

    /**
    *   @brief Get/Set:     Water Level
    **/
    virtual const int32_t    GetWaterLevel() override { return waterLevel; }
    virtual void             SetWaterLevel(const int32_t waterLevel) override { this->waterLevel = waterLevel; }

    /**
    *   @brief Get/Set:     Water Type
    **/
    virtual const int32_t    GetWaterType() override { return waterType; }
    virtual void             SetWaterType(const int32_t waterType) { this->waterType = waterType; }

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    virtual const float      GetYawSpeed() override { return yawSpeed; }
    virtual void             SetYawSpeed(const float yawSpeed) override { this->yawSpeed = yawSpeed; }



    /**
    *
    *
    *   Placeholders for BaseMover.
    *
    *
    **/
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual float GetAcceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual float GetDeceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual const vec3_t& GetEndPosition() { return ZeroVec3; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual float    GetSpeed() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual const vec3_t& GetStartPosition() { return ZeroVec3; }



protected:
    /**
    *   Server Entity Pointer.
    **/
    //Entity *podEntity = nullptr;


    /**
    *   Entity Flags
    **/
    //! Entity flags, general flags, flags... :) 
    int32_t flags = 0;
    //! Entity spawn flags (Such as, is this a dropped item?)
    int32_t spawnFlags = 0;
	//! Entity 'use' flags. Determines how the player can 'Use' interact with this entity.
	int32_t useEntityFlags = 0;
    
    /**
    *   Entity Strings/Targetnames.
    **/
    // Classname of this entity.
    std::string classname = "";
    // Entity MODEL filename.
    std::string model = "";
    // Trigger kill target string.
    std::string killTargetStr = "";
    // Trigger its message string.
    std::string messageStr = "";
    // Trigger target string.
    std::string targetStr = "";
    // Trigger its own targetname string.
    std::string targetNameStr = "";
    // Team string.
    std::string teamStr = "";


    /**
    *   Entity Category Types (Move, Water, what have ya? Add in here.)
    **/
    //! Move Type. (MoveType::xxx)
    int32_t moveType = MoveType::None;
    //! WaterType::xxxx
    int32_t waterType = 0; // TODO: Introduce WaterType "enum".
    //! WaterLevel::xxxx
    int32_t waterLevel = WaterLevel::None;


    /**
    *   Entity Physics
    **/
    //! Velocity.
    vec3_t velocity = vec3_zero();
    //! Angular Velocity.
    vec3_t angularVelocity = vec3_zero();
    //! Mass
    int32_t mass = 0;
    //! Per entity gravity multiplier (1.0 is normal). TIP: Use for lowgrav artifact, flares
    float gravity = 1.0f;
    //! Ground Entity link count. (To keep track if it is linked or not.)
    int32_t groundEntityLinkCount = 0;
    //! Yaw Speed. (Should be for monsters, move over to SVGBaseMonster?)
    float yawSpeed = 0.f;
    //! Ideal Yaw Angle. (Should be for monsters, move over to SVGBaseMonster?)
    float idealYawAngle = 0.f;


    /**
    *   Entity Goal, Move, Activator.
    **/

    //! Move Target Entity.
    Entity* moveTargetPtr = nullptr;
    //! The entity that activated this
    IServerGameEntity* activatorEntityPtr = nullptr;


    /**
    *   Entity Noise Indices.
    **/
    //! Noise Index A.
    int32_t noiseIndexA = 0;
    //! Noise Index B.
    int32_t noiseIndexB = 0;


    /**
    *   Entity 'Timing'
    **/
    //! The next 'think' time, determines when to call the 'think' callback.
    GameTime nextThinkTime = GameTime::zero();
    //! Delay before calling trigger execution.
    Frametime delayTime = Frametime::zero();
    //! Wait time before triggering at all, in case it was set to auto.
    Frametime waitTime = Frametime::zero();


    /**
    *   Entity '(Health-)Stats'
    **/
    //! Current health.
    int32_t health = 0;
    //! Maximum health.
    int32_t maxHealth = 0;


    /**
    *   Entity 'Game Settings'
    **/
    //! The height above the origin, this is where EYE SIGHT is at.
    int32_t viewHeight = 0;
    //! Determines how to interpret, take damage like a man or like a ... ? Yeah, pick up soap.
    int32_t takeDamage = 0;
    //! Actual damage it does if encountered or fucked around with.
    int32_t damage = 0;
    //! Dead Flag. (Are we dead, dying or...?)
    int32_t deadFlag = 0;
    //! Count (usually used for SVGBaseItem)
    int32_t count = 0;
    //! Style/AreaPortal
    int32_t style = 0;

    
    /**
    *   Entity pointers.
    **/
    //! Current active enemy, NULL if not any.    
    IServerGameEntity* geEnemyEntity      = nullptr;
    //! Ground entity we're standing on.
    IServerGameEntity* groundEntity     = nullptr;
	SGEntityHandle groundEntityHandle;
    //! Old enemy.
    IServerGameEntity* oldEnemyEntity   = nullptr;
    //! Owner. (Such as, did the player fire a blaster bolt? If so, the owner is...)
    IServerGameEntity* ownerEntity      = nullptr;
    //! Team Chain.
    IServerGameEntity* teamChainEntity  = nullptr;
    //! Team Master.
    IServerGameEntity* teamMasterEntity = nullptr;
    


public:
    /**
    * 
    *   Entity Utility callbacks that can be set as a nextThink function.
    * 
    **/
    /**
    *   @brief  Callback method to use for freeing this entity. It calls upon Remove()
    **/
    void SVGBaseEntityThinkFree(void);

    /**
    *   @brief  Callback for assigning when "no thinking" behavior is wished for.
    **/
    void SVGBaseEntityThinkNull() { }
};