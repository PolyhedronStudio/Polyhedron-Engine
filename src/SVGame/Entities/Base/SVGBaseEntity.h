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
class SVGEntityHandle;

// Include Entity Handle.
#include "SharedGame/Entities/SGEntityHandle.h"



/**
*   SVGBaseEntity
**/
class SVGBaseEntity {
public:
    /**
    *
    *
    *   FunctionPointers for Dispatch Callbacks.
    *
    *
    **/
    using ThinkCallbackPointer      = void(SVGBaseEntity::*)(void);
    using UseCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* other, SVGBaseEntity* activator);
    using TouchCallbackPointer      = void(SVGBaseEntity::*)(SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    using BlockedCallbackPointer    = void(SVGBaseEntity::*)(SVGBaseEntity* other);
    using TakeDamageCallbackPointer = void(SVGBaseEntity::*)(SVGBaseEntity* other, float kick, int32_t damage);
    using DieCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);



    /**
    *
    *
    *   Constructor/Destructor & TypeInfo Utilities.
    *
    *
    **/
    //! Constructor/Deconstructor.
    SVGBaseEntity(Entity* svEntity);
    virtual ~SVGBaseEntity() = default;

    //! Runtime type information.
    DefineTopAbstractClass( SVGBaseEntity );

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
    *   Entity Interface Implementations.
    *
    *
    **/
    /**
    *   @brief  Called during load time right before spawning the actual entities.
    *           Used to "pre-""cache" all media data.
    **/
    virtual void Precache();    // Precaches data.

    /**
    *   @brief  Called when this entity is ready to spawn. Used to setup the entity
    *           and prepare it for gameplay.
    * 
    *           DO NOT try and refer to other entities here. Use 'PostSpawn' instead.
    **/
    virtual void Spawn();       // Spawns the entity.

    /**
    *   @brief  Called when this entity is about to respawn.
    **/
    virtual void Respawn();     // Respawns the entity.

    /**
    *   @brief  When all other entities have had a chance to spawn, they get a call to
    *           'PostSpawn' where if needed, a reference to other entities can be made.
    **/
    virtual void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.

    /**
    *   @brief  Called each server game frame to let the entity 'think'. This function
    *           in return dispatches the set think callback method when its nextThink
    *           time decides it is time to do so.
    **/
    virtual void Think();       // General entity thinking routine.

    /**
    *   @brief  Used to either parse, or even ignore, key/value pairs that are set for
    *           an entity using the map editor.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value); // Called for each key:value when parsing the entity dictionary.



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
    void LinkEntity();
    /**
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    void UnlinkEntity();
    
    /**
    *   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
    *           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
    *           on us during the current server game frame we're processing.
    **/
    void Remove();

    /**
    *   @return Pointer to the server side entity.
    **/
    inline Entity* GetPODEntity() {
        return serverEntity;
    }
    /**
    *   @brief  Used only in SVG_FreeEntity and SVG_CreateClassEntity.
    *   @return Pointer to the server side entity.
    **/
    inline void SetPODEntity(Entity* svEntity) {
        serverEntity = svEntity;
    }



    /**
    *
    *
    *   Callback Functions to Dispatch.
    *
    *
    **/
    // Admer: these should all be prefixed with Dispatch
    void Use(SVGBaseEntity* other, SVGBaseEntity* activator);
    void Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void Blocked(SVGBaseEntity* other);
    void Touch(SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    void TakeDamage(SVGBaseEntity* other, float kick, int32_t damage);



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
    void UseTargets( SVGBaseEntity* activatorOverride = nullptr );



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
    inline vec3_t GetAbsoluteCenter(){
        return vec3_scale( GetAbsoluteMax() + GetAbsoluteMin(), 0.5f );
    }

    /**
    *   @brief Get/Set: BoundingBox Mins
    **/
    inline const vec3_t&    GetAbsoluteMin() { return serverEntity->absMin; }
    inline void             SetAbsoluteMin(const vec3_t &absMin) { serverEntity->absMin = absMin; }

    /**
    *   @brief Get/Set: BoundingBox Maxs
    **/
    inline const vec3_t&    GetAbsoluteMax() { return serverEntity->absMax; }
    inline void             SetAbsoluteMax(const vec3_t &absMax) { serverEntity->absMax = absMax; }

    /**
    *   @brief Get/Set: Activator
    **/
    virtual inline SVGBaseEntity*   GetActivator() { return activatorEntityPtr; }
    inline void                     SetActivator(SVGBaseEntity* activator) { this->activatorEntityPtr = activator; }

    /**
    *   @brief Get/Set: Angles
    **/
    inline const vec3_t&    GetAngles() { return serverEntity->state.angles; }
    inline void             SetAngles(const vec3_t& angles) { serverEntity->state.angles = angles; }

    /**
    *   @brief Get/Set: Angular Velocity
    **/
    inline const vec3_t&    GetAngularVelocity() { return angularVelocity; }
    inline void             SetAngularVelocity(const vec3_t& angularVelocity) { this->angularVelocity = angularVelocity; }

    /**
    *   @return The local center(model-space) of the entity's Bounding Box.
    **/
    inline vec3_t GetCenter() { return vec3_scale( GetMaxs() + GetMins(), 0.5f ); }

    /**
    *   @brief Set: Mins and Maxs determining the entity's Bounding Box
    **/
    inline void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) {
        serverEntity->mins = mins;
        serverEntity->maxs = maxs;
    }

    /**
    *   @brief Get/Set: Classname
    **/
    inline const std::string &GetClassname() { return classname; }
    inline void SetClassname(const std::string &classname) { this->classname = classname; }

    /**
    *   @brief Get: Entity Client
    **/
    gclient_s* GetClient() {
        return serverEntity->client;
    }

    /**
    *   @brief Get/Set: Clip Mask
    **/
    inline const int32_t    GetClipMask() { return serverEntity->clipMask; }
    inline void             SetClipMask(const int32_t clipMask) { serverEntity->clipMask = clipMask; }

    /**
    *   @brief Get/Set: Count
    **/
    inline const int32_t    GetCount() { return count; }
    inline void             SetCount(const int32_t count) { this->count = count; }

    /**
    *   @brief Get/Set: Damage
    **/
    inline const int32_t    GetDamage() { return damage; }
    inline void             SetDamage(const int32_t damage) { this->damage = damage; }

    /**
    *   @brief Get/Set: Dead Flag
    **/
    inline const int32_t    GetDeadFlag() { return deadFlag; }
    inline void             SetDeadFlag(const int32_t deadFlag) { this->deadFlag = deadFlag; }

    /**
    *   @brief Get/Set: Delay Time
    **/
    inline const float      GetDelayTime() { return delayTime; }
    inline void             SetDelayTime(const float delayTime) { this->delayTime = delayTime; }

    /**
    *   @brief Get/Set: Effects
    **/
    inline const uint32_t   GetEffects() { return serverEntity->state.effects; }
    inline void             SetEffects(const uint32_t effects) { serverEntity->state.effects = effects; }

    /**
    *   @brief Get/Set: Enemy
    **/
    inline SVGBaseEntity*   GetEnemy() { return enemyEntity; }
    inline void             SetEnemy(SVGBaseEntity* enemy) { this->enemyEntity = enemy; }

    /**
    *   @brief Get: Entity Dictionary.
    **/
    virtual inline EntityDictionary &GetEntityDictionary() { return serverEntity->entityDictionary; }

    /**
    *   @brief Get/Set: Event ID
    **/
    inline const uint8_t    GetEventID() { return serverEntity->state.eventID; }
    inline void             SetEventID(const uint8_t eventID) { serverEntity->state.eventID = eventID; }

    /**
    *   @brief Get/Set: Flags
    **/
    inline const int32_t    GetFlags() { return flags; }
    inline void             SetFlags(const int32_t flags) { this->flags = flags; }

    /**
    *   @brief Get/Set: Animation Frame
    **/
    inline const float      GetAnimationFrame() { return serverEntity->state.animationFrame; }
    inline void             SetAnimationFrame(const float frame) { serverEntity->state.animationFrame = frame; }

    /**
    *   @brief Get/Set: Gravity
    **/
    inline const float      GetGravity() { return gravity; }
    inline void             SetGravity(const float gravity) { this->gravity = gravity; }

    /**
    *   @brief Get/Set: Ground Entity
    **/
    inline SGEntityHandle   GetGroundEntity() { return groundEntity; }
    inline void             SetGroundEntity(SVGBaseEntity* groundEntity) { this->groundEntity = groundEntity; }

    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    inline int32_t          GetGroundEntityLinkCount() { return groundEntityLinkCount; }
    inline void             SetGroundEntityLinkCount(int32_t groundEntityLinkCount) { this->groundEntityLinkCount = groundEntityLinkCount; }

    /**
    *   @brief Get/Set: Health
    **/
    inline const int32_t    GetHealth() { return health; }
    inline void             SetHealth(const int32_t health) { this->health = health; }

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    inline const float      GetIdealYawAngle() { return idealYawAngle; }
    inline void             SetIdealYawAngle(const float idealYawAngle) { this->idealYawAngle = idealYawAngle; }

    /**
    *   @brief Is/Set: In Use.
    **/
    inline qboolean         IsInUse() { return serverEntity->inUse; }
    inline void             SetInUse(const qboolean inUse) { serverEntity->inUse = inUse; }

    /**
    *   @brief Get/Set: Kill Target.
    **/
    inline const std::string&   GetKillTarget() { return killTargetStr; }
    inline void                 SetKillTarget(const std::string& killTarget) { this->killTargetStr = killTarget; }

    /**
    *   @brief Get/Set: Link Count.
    **/
    inline const int32_t    GetLinkCount() { return (serverEntity ? serverEntity->linkCount : 0); }
    inline void             SetLinkCount(const int32_t linkCount) { serverEntity->linkCount = linkCount; }

    /**
    *   @brief Get/Set: Mass
    **/
    inline int32_t          GetMass() { return mass; }
    inline void             SetMass(const int32_t mass) { this->mass = mass; }

    /**
    *   @brief Get/Set: Max Health
    **/
    inline const int32_t    GetMaxHealth() { return maxHealth; }
    inline void             SetMaxHealth(const int32_t maxHealth) { this->maxHealth = maxHealth; }

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    inline const vec3_t&    GetMaxs() { return serverEntity->maxs; }
    inline void             SetMaxs(const vec3_t& maxs) { serverEntity->maxs = maxs; }

    /**
    *   @brief Get/Set: Message
    **/
    inline const std::string&   GetMessage() { return messageStr; }
    inline void                 SetMessage(const std::string& message) { this->messageStr = message; }

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    inline const vec3_t&    GetMins() { return serverEntity->mins; }
    inline void             SetMins(const vec3_t& mins) { serverEntity->mins = mins; }
   
    /**
    *   @brief Get/Set: Model
    **/
    inline const std::string&   GetModel() { return model; }
    inline void                 SetModel(const std::string &model) {
        // Set model.
        this->model = model;

        // Set the model.
        gi.SetModel(serverEntity, model.c_str());

        // Set model index.
        SetModelIndex(gi.ModelIndex(model.c_str()));
    }

    /**
    *   @brief Get/Set: Model Index 1
    **/
    inline const int32_t    GetModelIndex()  { return serverEntity->state.modelIndex;  }
    inline void             SetModelIndex(const int32_t index)  { serverEntity->state.modelIndex = index;  }
    /**
    *   @brief Get/Set: Model Index 2
    **/
    inline const int32_t    GetModelIndex2() { return serverEntity->state.modelIndex2; }
    inline void             SetModelIndex2(const int32_t index) { serverEntity->state.modelIndex2 = index; }
    /**
    *   @brief Get/Set: Model Index 3
    **/
    inline const int32_t    GetModelIndex3() { return serverEntity->state.modelIndex3; }
    inline void             SetModelIndex3(const int32_t index) { serverEntity->state.modelIndex3 = index; }
    /**
    *   @brief Get/Set: Model Index 4
    **/
    inline const int32_t    GetModelIndex4() { return serverEntity->state.modelIndex4; }
    inline void SetModelIndex4(const int32_t index) { serverEntity->state.modelIndex4 = index; }

    /**
    *   @brief Get/Set: Move Type.
    **/
    inline const int32_t    GetMoveType() { return moveType; } 
    inline void             SetMoveType(const int32_t moveType) { this->moveType = moveType; }

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    inline const float      GetNextThinkTime() { return nextThinkTime; }
    inline void             SetNextThinkTime(const float nextThinkTime) { this->nextThinkTime = nextThinkTime; }

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    inline const int32_t    GetNoiseIndexA() { return noiseIndexA; }
    inline void             SetNoiseIndexA(const int32_t noiseIndexA) { this->noiseIndexA = noiseIndexA; }

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    inline const int32_t    GetNoiseIndexB() { return noiseIndexB; }
    inline void             SetNoiseIndexB(const int32_t noiseIndexB) { this->noiseIndexB = noiseIndexB; }

    /**
    *   @brief Get/Set:     State Number
    **/
    inline const int32_t    GetNumber() { return serverEntity->state.number; }
    inline void             SetNumber(const int32_t number) { serverEntity->state.number = number; }

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    inline SVGBaseEntity*   GetOldEnemy() { return oldEnemyEntity; }
    inline void             SetOldEnemy(SVGBaseEntity* oldEnemy) { this->oldEnemyEntity = oldEnemy; }

    /**
    *   @brief Get/Set:     Old Origin
    **/
    inline const vec3_t&    GetOldOrigin() { return serverEntity->state.oldOrigin; }
    inline void             SetOldOrigin(const vec3_t& oldOrigin) { serverEntity->state.oldOrigin = oldOrigin; }

    /**
    *   @brief Get/Set:     Origin
    **/
    inline const vec3_t&    GetOrigin() { return serverEntity->state.origin; }
    inline void             SetOrigin(const vec3_t& origin) { serverEntity->state.origin = origin; }

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    inline SVGBaseEntity*   GetOwner() { return this->ownerEntity; }
    inline void             SetOwner(SVGBaseEntity* owner) { this->ownerEntity = owner; }

    /**
    *   @brief Get/Set:     Render Effects
    **/
    inline const int32_t    GetRenderEffects() { return serverEntity->state.renderEffects; }
    inline void             SetRenderEffects(const int32_t renderEffects) { serverEntity->state.renderEffects = renderEffects; }
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    inline virtual const char* GetPathTarget() { return nullptr; }

    /**
    *   @brief Get/Set:     Server Flags
    **/
    inline const int32_t    GetServerFlags() { return serverEntity->serverFlags; }
    inline void             SetServerFlags(const int32_t serverFlags) { serverEntity->serverFlags = serverFlags; }

    /**
    *   @brief Get/Set:     Skin Number
    **/
    inline const int32_t    GetSkinNumber() { return serverEntity->state.skinNumber; }
    inline void             SetSkinNumber(const int32_t skinNumber) { serverEntity->state.skinNumber = skinNumber; }

    /**
    *   @brief Get/Set:     Entity Size
    **/
    inline const vec3_t&    GetSize() { return serverEntity->size; }
    inline void             SetSize(const vec3_t& size) { serverEntity->size = size; }

    /**
    *   @brief Get/Set:     Solid
    **/
    inline const uint32_t   GetSolid() { return serverEntity->solid; }
    inline void             SetSolid(const uint32_t solid) { serverEntity->solid = solid; }

    /**
    *   @brief Get/Set:     Sound.
    **/
    inline const int32_t GetSound() { return serverEntity->state.sound; }
    inline void SetSound(const int32_t sound) { serverEntity->state.sound = sound; }

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    inline const int32_t    GetSpawnFlags() { return spawnFlags; }
    inline void             SetSpawnFlags(const int32_t spawnFlags) { this->spawnFlags = spawnFlags; }

    /**
    *   @brief Get/Set:     Entity State
    **/
    inline const EntityState&   GetState() { return serverEntity->state; }
    inline void                 SetState(const EntityState &state) { serverEntity->state = state; }

    /**
    *   @brief Get/Set:     Style
    **/
    inline const int32_t    GetStyle() { return style; }
    inline void             SetStyle(const int32_t style) { this->style = style; }

    /**
    *   @brief Get/Set:     Take Damage
    **/
    inline const int32_t    GetTakeDamage() { return takeDamage; }
    inline void             SetTakeDamage(const int32_t takeDamage) { this->takeDamage = takeDamage; }
    
    /**
    *   @brief Get/Set:     Take Damage
    **/
    inline const std::string&   GetTarget() { return targetStr; }
    inline void                 SetTarget(const std::string& target) { this->targetStr = target; }

    /**
    *   @brief Get/Set:     Target Name
    **/
    inline const std::string&   GetTargetName() { return targetNameStr; }
    inline void                 SetTargetName(const std::string& targetName) { this->targetNameStr = targetName; }

    /**
    *   @brief Get/Set:     Team
    **/
    inline const std::string&   GetTeam() { return teamStr; }
    inline void                 SetTeam(const std::string &team) { this->teamStr = team; }

    /**
    *   @brief Get/Set:     Team Chain
    **/
    inline SVGBaseEntity*   GetTeamChainEntity() { return teamChainEntity; }
    inline void             SetTeamChainEntity(SVGBaseEntity* entity) { teamChainEntity = entity; }

    /**
    *   @brief Get/Set:     Team Master
    **/
    inline SVGBaseEntity*   GetTeamMasterEntity() { return teamMasterEntity; }
    inline void             SetTeamMasterEntity(SVGBaseEntity* entity) { teamMasterEntity = entity; }

    /**
    *   @brief Get/Set:     Velocity
    **/
    inline const vec3_t& GetVelocity() { return velocity; }
    inline void SetVelocity(const vec3_t &velocity) { this->velocity = velocity; }

    /**
    *   @brief Get/Set:     View Height
    **/
    inline const int32_t    GetViewHeight() { return viewHeight; }
    inline void             SetViewHeight(const int32_t height) { this->viewHeight = height; }

    /**
    *   @brief Get/Set:     Wait Time
    **/
    inline const float      GetWaitTime() { return waitTime; }
    inline void             SetWaitTime(const float waitTime) { this->waitTime = waitTime; }

    /**
    *   @brief Get/Set:     Water Level
    **/
    inline const int32_t    GetWaterLevel() { return waterLevel; }
    inline void             SetWaterLevel(const int32_t waterLevel) { this->waterLevel = waterLevel; }

    /**
    *   @brief Get/Set:     Water Type
    **/
    inline const int32_t    GetWaterType() { return waterType; }
    inline void             SetWaterType(const int32_t waterType) { this->waterType = waterType; }

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    inline const float      GetYawSpeed() { return yawSpeed; }
    inline void             SetYawSpeed(const float yawSpeed) { this->yawSpeed = yawSpeed; }



    /**
    *
    *
    *   Placeholders for SVGBaseMover.
    *
    *
    **/
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetAcceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetDeceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetEndPosition() { return vec3_zero(); }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float    GetSpeed() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetStartPosition() { return vec3_zero(); }



protected:
    /**
    *   Entity Dictionary Parsing Utilities.
    * 
    *   @return True on success, false on failure.
    **/
    qboolean ParseFloatKeyValue(const std::string& key, const std::string& value, float& floatNumber);
    qboolean ParseIntegerKeyValue(const std::string& key, const std::string& value, int32_t& integerNumber);
    qboolean ParseUnsignedIntegerKeyValue(const std::string& key, const std::string& value, uint32_t& unsignedIntegerNumber);
    qboolean ParseStringKeyValue(const std::string& key, const std::string& value, std::string& stringValue);
    qboolean ParseVector3KeyValue(const std::string& key, const std::string& value, vec3_t& vectorValue);


    /**
    *   Server Entity Pointer.
    **/
    Entity *serverEntity = nullptr;


    /**
    *   Entity Flags
    **/
    //! Entity flags, general flags, flags... :) 
    int32_t flags = 0;
    //! Entity spawn flags (Such as, is this a dropped item?)
    int32_t spawnFlags = 0;

    
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
    //! Goal Entity.
    Entity* goalEntityPtr = nullptr;
    //! Move Target Entity.
    Entity* moveTargetPtr = nullptr;
    //! The entity that activated this
    SVGBaseEntity* activatorEntityPtr = nullptr;


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
    float nextThinkTime = 0.f;
    //! Delay before calling trigger execution.
    float delayTime = 0.f;
    //! Wait time before triggering at all, in case it was set to auto.
    float waitTime = 0.f;


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
    SVGBaseEntity* enemyEntity      = nullptr;
    //! Ground entity we're standing on.
    SVGBaseEntity* groundEntity     = nullptr;
    //! Old enemy.
    SVGBaseEntity* oldEnemyEntity   = nullptr;
    //! Owner. (Such as, did the player fire a blaster bolt? If so, the owner is...)
    SVGBaseEntity* ownerEntity      = nullptr;
    //! Team Chain.
    SVGBaseEntity* teamChainEntity  = nullptr;
    //! Team Master.
    SVGBaseEntity* teamMasterEntity = nullptr;
    


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



protected:
    /**
    *   Dispatch Callback Function Pointers.
    **/
    ThinkCallbackPointer        thinkFunction = nullptr;
    UseCallbackPointer          useFunction = nullptr;
    TouchCallbackPointer        touchFunction = nullptr;
    BlockedCallbackPointer      blockedFunction = nullptr;
    TakeDamageCallbackPointer   takeDamageFunction = nullptr;
    DieCallbackPointer          dieFunction = nullptr;

public:
    /**
    * 
    *   Base callback implementations that can be set by all derived entities.
    * 
    **/
    /**
    *   @brief  Callback method to use for freeing this entity.
    **/
    void SVGBaseEntityThinkFree(void);

    /**
    *   @brief  Callback for assigning when "no thinking" behavior is wished for.
    **/
    void SVGBaseEntityThinkNull() { }
};