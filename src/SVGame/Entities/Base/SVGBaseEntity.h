/*
// LICENSE HERE.

//
// SVGBaseEntity.h
//
// Base entity class, where the fun begins. All entities are inherited from this,
// one way or the other :)
//
*/
#pragma once

// It makes sense to include TypeInfo in SVGBaseEntity.h, 
// because this class absolutely requires it
#include "../../TypeInfo.h"

// Forward declare.
class SVGEntityHandle;

#include "SVGEntityHandle.h"

class SVGBaseEntity {
public:
    //
    // Function pointers for actual callbacks.
    //
    using ThinkCallbackPointer      = void(SVGBaseEntity::*)(void);
    using UseCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* other, SVGBaseEntity* activator);
    using TouchCallbackPointer      = void(SVGBaseEntity::*)(SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    using BlockedCallbackPointer    = void(SVGBaseEntity::*)(SVGBaseEntity* other);
    using TakeDamageCallbackPointer = void(SVGBaseEntity::*)(SVGBaseEntity* other, float kick, int32_t damage);
    using DieCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

    //
    // Constructor/Deconstructor.
    //
    SVGBaseEntity(Entity* svEntity);
    virtual ~SVGBaseEntity();

    // Runtime type information
    DefineTopAbstractClass( SVGBaseEntity );

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

    //
    // Interface functions.
    //
    virtual void Precache();    // Precaches data.
    virtual void Spawn();       // Spawns the entity.
    virtual void Respawn();     // Respawns the entity.
    virtual void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think();       // General entity thinking routine.

    virtual void SpawnKey(const std::string& key, const std::string& value); // Called for each key:value when parsing the entity dictionary.

    //
    // Callback functions.
    // // Admer: these should all be prefixed with Dispatch
    void Use(SVGBaseEntity* other, SVGBaseEntity* activator);
    void Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void Blocked(SVGBaseEntity* other);
    void Touch(SVGBaseEntity* self, SVGBaseEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    void TakeDamage(SVGBaseEntity* other, float kick, int32_t damage);

    //
    // Entity interaction functions
    //
    
    //  Calls Use on this entity's targets, and deletes its killtargets if any
    //  @param activatorOverride: if nullptr, the entity's own activator is used and if the entity's own activator is nullptr, then this entity itself is the activator
    void UseTargets( SVGBaseEntity* activatorOverride = nullptr );

    //
    // Entity Get Functions.
    //
    // @returns The entity's center in world coordinates
    inline vec3_t GetAbsoluteCenter(){
        return vec3_scale( GetAbsoluteMax() + GetAbsoluteMin(), 0.5f );
    }

    // Return the bounding box absolute 'min' value.
    inline const vec3_t& GetAbsoluteMin() {
        return serverEntity->absMin;
    }

    // Return the bounding box absolute 'max' value.
    inline const vec3_t& GetAbsoluteMax() {
        return serverEntity->absMax;
    }
    // Placeholder, implemented by SVGBaseMover, and derivates of that class.
    virtual inline float GetAcceleration() {
        return 0.f;
    }
    // Placeholder, implemented by SVGBaseTrigger, and derivates of that class.
    virtual inline SVGBaseEntity* GetActivator() {
        return activatorEntityPtr;
    }
    // Return the 'angles' value.
    inline const vec3_t& GetAngles() { return serverEntity->state.angles; }

    // Return the 'angularVelocity' value.
    inline const vec3_t& GetAngularVelocity() { return angularVelocity; }
    
    // @returns The local center
    inline vec3_t GetCenter() { return vec3_scale( GetMaxs() + GetMins(), 0.5f ); }
    
    // Return the 'classname' value.
    inline const std::string &GetClassname() { return classname; }
    // Set the 'classname' value.
    inline void SetClassname(const std::string &classname) { this->classname = classname; }

    // Return the 'client' pointer.
    gclient_s* GetClient() {
        return serverEntity->client;
    }

    // Return the 'clipmask' value.
    inline const int32_t GetClipMask() { return serverEntity->clipMask; }

    // Return the 'count' value.
    inline const int32_t GetCount() { return count; }

    // Return the 'damage' value.
    inline const int32_t GetDamage() { return damage; }

    // Get the 'deadFlag' value.
    inline const int32_t GetDeadFlag() { return deadFlag; }

    // Placeholder, implemented by SVGBaseMover, and derivates of that class.
    virtual inline float GetDeceleration() { return 0.f; }
    // Return the 'delay' value.
    inline const float &GetDelayTime() { return delayTime; }

    // Return the 'effects' value.
    inline const uint32_t GetEffects() { return serverEntity->state.effects; }

    // Return the 'enemyPtr' entity pointer.
    inline SVGBaseEntity* GetEnemy() { return enemyEntity; }

    // Placeholder, implemented by SVGBaseMover, and derivates of that class.
    virtual inline const vec3_t& GetEndPosition() { return vec3_zero(); }

    // Returns a reference to the 'entityDictionary'.
    virtual inline EntityDictionary &GetEntityDictionary() { return serverEntity->entityDictionary; }

    // Return the 'eventID' value.
    inline const uint8_t GetEventID() { return serverEntity->state.eventID; }

    // Return the 'flags' value.
    inline const int32_t GetFlags() { return flags; }

    // Return the 'frame' value.
    inline const float GetAnimationFrame() { return serverEntity->state.animationFrame; }

    // Return the 'gravity' value.
    inline const float GetGravity() { return gravity; }

    // Return the 'groundEntitPtr' entity.
    inline SVGEntityHandle GetGroundEntity() { return groundEntity; }

    // Return the 'groundEntityLinkCount' value.
    inline int32_t GetGroundEntityLinkCount() { return groundEntityLinkCount; }

    // Return the 'health' value.
    inline const int32_t GetHealth() { return health; }

    inline const float GetIdealYawAngle() { return idealYawAngle; }

    // Get the 'inuse' value.
    inline qboolean IsInUse() { return serverEntity->inUse; }

    // Get the 'killTarget' entity value.
    inline const std::string &GetKillTarget() { return killTargetStr; }

    // Get the 'linkCount' value.
    inline const int32_t GetLinkCount() { return (serverEntity ? serverEntity->linkCount : 0); }

    // Return the 'mass' value.
    inline int32_t GetMass() { return mass; }

    // Return the 'maxHealth' value.
    inline const int32_t GetMaxHealth() { return maxHealth; }

    // Return the bounding box 'maxs' value.
    inline const vec3_t& GetMaxs() { return serverEntity->maxs; }

    // Return the 'message' value.
    inline const std::string &GetMessage() { return messageStr; }

    // Return the bounding box 'mins' value.
    inline const vec3_t& GetMins() { return serverEntity->mins; }
   
    // Return the 'model' value.
    inline const std::string &GetModel() { return model; }

    // Model Index 1.
    inline const int32_t    GetModelIndex()  { return serverEntity->state.modelIndex;  }
    // Model Index 2.
    inline const int32_t    GetModelIndex2() { return serverEntity->state.modelIndex2; }
    // Model Index 3.
    inline const int32_t    GetModelIndex3() { return serverEntity->state.modelIndex3; }
    // Model Index 4.
    inline const int32_t    GetModelIndex4() { return serverEntity->state.modelIndex4; }

    // Return the 'movetype' value.
    inline const int32_t    GetMoveType() { return moveType; } 
    // Set the 'moveType' value.
    inline void             SetMoveType(const int32_t &moveType) { this->moveType = moveType; }

    // Next Think Time.
    inline const float  GetNextThinkTime() { return nextThinkTime; }
    inline void         SetNextThinkTime(const float& nextThinkTime) { this->nextThinkTime = nextThinkTime; }

    // Noise Index A.
    inline const int32_t    GetNoiseIndexA() { return noiseIndexA; }
    inline void             SetNoiseIndexA(const int32_t& noiseIndexA) { this->noiseIndexA = noiseIndexA; }

    // Noise Index B.
    inline const int32_t    GetNoiseIndexB() { return noiseIndexB; }
    inline void             SetNoiseIndexB(const int32_t& noiseIndexB) { this->noiseIndexB = noiseIndexB; }

    // State Number.
    inline const int32_t    GetNumber() { return serverEntity->state.number; }
    inline void             SetNumber(const int32_t number) { serverEntity->state.number = number; }

    // Old Enemy.
    SVGBaseEntity*  GetOldEnemy() { return oldEnemyEntity; }
    inline void     SetOldEnemy(SVGBaseEntity* oldEnemy) { this->oldEnemyEntity = oldEnemy; }

    // State Old Origin.
    inline const vec3_t&    GetOldOrigin() { return serverEntity->state.oldOrigin; }
    inline void             SetOldOrigin(const vec3_t& oldOrigin) { serverEntity->state.oldOrigin = oldOrigin; }

    // State Origin.
    inline const vec3_t &GetOrigin() { return serverEntity->state.origin; }
    inline void         SetOrigin(const vec3_t& origin) { serverEntity->state.origin = origin; }

    // Owner Entity.
    inline SVGBaseEntity*   GetOwner() { return this->ownerEntity; }
    inline void             SetOwner(SVGBaseEntity* owner) { this->ownerEntity = owner; }

    // Render Effects.
    inline const int32_t    GetRenderEffects() { return serverEntity->state.renderEffects; }
    inline void             SetRenderEffects(const int32_t& renderEffects) { serverEntity->state.renderEffects = renderEffects; }
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    inline virtual const char* GetPathTarget() { return nullptr; }

    // Server Flags.
    inline const int32_t    GetServerFlags() { return serverEntity->serverFlags; }
    inline void             SetServerFlags(const int32_t &serverFlags) { serverEntity->serverFlags = serverFlags; }

    // Skin Number.
    inline const int32_t    GetSkinNumber() { return serverEntity->state.skinNumber; }
    inline void             SetSkinNumber(const int32_t& skinNumber) { serverEntity->state.skinNumber = skinNumber; }

    // Entity Size.
    inline const vec3_t&    GetSize() { return serverEntity->size; }
    inline void             SetSize(const vec3_t& size) { serverEntity->size = size; }

    // Solid.
    inline const uint32_t   GetSolid() { return serverEntity->solid; }
    inline void             SetSolid(const uint32_t &solid) { serverEntity->solid = solid; }

    // Spawn Flags.
    inline const int32_t    GetSpawnFlags() { return spawnFlags; }
    inline void             SetSpawnFlags(const int32_t& spawnFlags) { this->spawnFlags = spawnFlags; }

    // Placeholder, implemented by SVGBaseMover, and derivates of that class.
    virtual inline float    GetSpeed() { return 0.f; }
    // Placeholder, implemented by SVGBaseMover, and derivates of that class.
    virtual inline const vec3_t& GetStartPosition() { return vec3_zero(); }

    // Return a reference to the serverEntity its state.
    inline const EntityState& GetState() { return serverEntity->state; }

    // Return the 'style' value.
    inline const int32_t GetStyle() { return style; }

    // Return the 'sound' value.
    inline const int32_t GetSound() { return serverEntity->state.sound; }

    // Return the 'takeDamage' value.
    inline const int32_t GetTakeDamage() { return takeDamage; }

    // Return the 'target' entity value.
    inline const std::string& GetTarget() { return targetStr; }
    // Return the 'targetName' entity value.
    inline const std::string& GetTargetName() { return targetNameStr; }

    // Return the 'team' entity value.
    inline const std::string &GetTeam() { return teamStr; }

    // Return the 'teamChain' entity value.
    inline SVGBaseEntity* GetTeamChainEntity() { return teamChainEntity; }

    // Return the 'teamMaster' entity value.
    inline SVGBaseEntity *GetTeamMasterEntity() { return teamMasterEntity; }

    // Return the 'viewHeight' entity value.
    inline const int32_t GetViewHeight() { return viewHeight; }

    // Return the 'velocity' value.
    inline const vec3_t& GetVelocity() { return velocity; }

    // Return the 'wait' value.
    inline const float& GetWaitTime() { return waitTime; }

    // Return the 'waterLevel' value.
    inline const int32_t GetWaterLevel() { return waterLevel; }

    // Return the 'waterType' value.
    inline const int32_t GetWaterType() { return waterType; }

    inline const float GetYawSpeed() { return yawSpeed; }

    //
    // Entity Set Functions.
    //  
    // Set the bounding box absolute 'min' value.
    inline void SetAbsoluteMin(const vec3_t &absMin) { serverEntity->absMin = absMin; }

    // Set the bounding box absolute 'max' value.
    inline void SetAbsoluteMax(const vec3_t &absMax) { serverEntity->absMax = absMax; }

    // Set the 'activatorEntity' pointer.
    inline void SetActivator(SVGBaseEntity* activator) { this->activatorEntityPtr = activator; }

    // Set the 'angles' value.
    inline void SetAngles(const vec3_t& angles) { serverEntity->state.angles = angles; }

    // Set the 'angularVelocity' value.
    inline void SetAngularVelocity(const vec3_t& angularVelocity) { this->angularVelocity = angularVelocity; }

    // Set the 'mins', and 'maxs' values of the entity bounding box.
    inline void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) {
        serverEntity->mins = mins;
        serverEntity->maxs = maxs;
    }

    // Set the 'clipmask' value.
    inline void SetClipMask(const int32_t &clipMask) { serverEntity->clipMask = clipMask; }

    // Set the 'count' value.
    inline void SetCount(const int32_t& count) { this->count = count; }
    // Set the 'damage' value.
    inline void SetDamage(const int32_t &damage) { this->damage = damage; }

    // Set the 'deadFlag' value.
    inline void SetDeadFlag(const int32_t& deadFlag) { this->deadFlag = deadFlag; }

    // Set the 'delayTime' value.
    inline void SetDelayTime(const float& delayTime) { this->delayTime = delayTime; }

    // Set the 'effects' value.
    inline void SetEffects(const uint32_t &effects) { serverEntity->state.effects = effects; }

    // Set the 'enemyPtr' pointer.
    inline void SetEnemy(SVGBaseEntity* enemy) { this->enemyEntity = enemy; }

    // Return the 'eventID' value.
    inline void SetEventID(const uint8_t &eventID) { serverEntity->state.eventID = eventID; }

    // Set the 'flags' value.
    inline void SetFlags(const int32_t &flags) { this->flags = flags; }

    // Set the 'frame' value.
    inline void SetAnimationFrame(const float &frame) { serverEntity->state.animationFrame = frame; }

    // Set the 'gravity' value.
    inline void SetGravity(const float &gravity) { this->gravity = gravity; }

    // Set the 'groundEntitPtr' entity.
    inline void SetGroundEntity(SVGBaseEntity* groundEntity) { this->groundEntity = groundEntity; }

    // Set the 'groundEntityLinkCount' value.
    inline void SetGroundEntityLinkCount(int32_t groundEntityLinkCount) { this->groundEntityLinkCount = groundEntityLinkCount; }

    // Set the 'health' value.
    inline void SetHealth(const int32_t &health) { this->health = health; }

    // Set the 'idealYawAngle' value.
    inline void SetIdealYawAngle(const float& idealYawAngle) { this->idealYawAngle = idealYawAngle; }

    // Set the 'inuse' value.
    inline void SetInUse(const qboolean& inUse) { serverEntity->inUse = inUse; }
    
    // Set the 'killTargetSTr' value.
    inline void SetKillTarget(const std::string& killTarget) { this->killTargetStr = killTarget; }

    // Set the 'linkCount' value.
    inline void SetLinkCount(const int32_t &linkCount) { serverEntity->linkCount = linkCount; }

    // Set the 'mass' value.
    inline void SetMass(const int32_t &_mass) { this->mass = _mass; }

    // Set the 'maxHealth' value.
    inline void SetMaxHealth(const int32_t& maxHealth) { this->maxHealth = maxHealth; }

    // Set the 'maxs' value.
    inline void SetMaxs(const vec3_t& maxs) { serverEntity->maxs = maxs; }

    // Set the 'messageStr' value.
    inline void SetMessage(const std::string& message) { this->messageStr = message; }
    
    // Set the 'mins' value.
    inline void SetMins(const vec3_t& mins) { serverEntity->mins = mins; }

    // Set the 'modelIndex, modelIndex1, modelIndex2, modelIndex3' values.
    inline void SetModelIndex(const int32_t& index)  { serverEntity->state.modelIndex = index;  }
    inline void SetModelIndex2(const int32_t& index) { serverEntity->state.modelIndex2 = index; }
    inline void SetModelIndex3(const int32_t& index) { serverEntity->state.modelIndex3 = index; }
    inline void SetModelIndex4(const int32_t& index) { serverEntity->state.modelIndex4 = index; }

    // Set the 'model' value.
    inline void SetModel(const std::string &model) {
        // Set model.
        this->model = model;

        // Set the model.
        gi.SetModel(serverEntity, model.c_str());

        // Set model index.
        SetModelIndex(gi.ModelIndex(model.c_str()));
    }






    
    







    // Sets the 'sound' value.
    inline void SetSound(const int32_t& sound) { serverEntity->state.sound = sound; }



    // Set another copy of a serverEntity its state.
    inline void SetState(const EntityState &state) { serverEntity->state = state; }

    // Set the 'style' value.
    inline void SetStyle(const int32_t &style) { this->style = style; }

    // Set the 'takeDamage' value.
    inline void SetTakeDamage(const int32_t& takeDamage) { this->takeDamage = takeDamage; }

    // Set the 'target' entity value.
    inline void SetTarget(const std::string& target) { this->targetStr = target; }
    // Set the 'targetName' entity value.
    inline void SetTargetName(const std::string& targetName) { this->targetNameStr = targetName; }

    // Set the 'team' index value.
    inline void SetTeam(const std::string &team) { this->teamStr = team; }

    // Set the 'teamChain' entity value.
    inline void SetTeamChainEntity(SVGBaseEntity* entity) { teamChainEntity = entity; }

    // Set the 'teamMaster' entity value.
    inline void SetTeamMasterEntity(SVGBaseEntity* entity) { teamMasterEntity = entity; }

    // Set the 'viewHeight' entity value.
    inline void SetViewHeight(const int32_t& height) { this->viewHeight = height; }

    // Set the 'velocity' value.
    inline void SetVelocity(const vec3_t &velocity) { this->velocity = velocity; }

    // Return the 'wait' value.
    inline void SetWaitTime(const float& waitTime) { this->waitTime = waitTime; }

    // Return the 'waterLevel' value.
    inline void SetWaterLevel(const int32_t &waterLevel) { this->waterLevel = waterLevel; }

    // Return the 'waterType' value.
    inline void SetWaterType(const int32_t &waterType) { this->waterType = waterType; }

    // Yaw Speed. (Should be for monsters...)
    inline void SetYawSpeed(const float& yawSpeed) { this->yawSpeed = yawSpeed; }

    //
    // General Entity Functions.
    //
    // Link entity to world for collision testing using gi.LinkEntity.
    void LinkEntity();
    
    // Marks the entity to be removed in the next server frame
    // This is preferred to SVG_FreeEntity, as it is safer
    void Remove();

    // Unlink the entity from the world for collision testing.
    void UnlinkEntity();

    // Returns the server entity pointer.
    inline Entity* GetServerEntity() {
        return serverEntity;
    }

    // Used only in SVG_FreeEntity and SVG_CreateClassEntity
    inline void SetServerEntity( Entity* svEntity )
    {
        serverEntity = svEntity;
    }

protected:
    //
    // Entity Dictionary Value Parsing functions.
    // 
    // When successful they return true, false otherwise.
    // 
    qboolean ParseFloatKeyValue(const std::string& key, const std::string& value, float& floatNumber);
    qboolean ParseIntegerKeyValue(const std::string& key, const std::string& value, int32_t& integerNumber);
    qboolean ParseUnsignedIntegerKeyValue(const std::string& key, const std::string& value, uint32_t& unsignedIntegerNumber);
    qboolean ParseStringKeyValue(const std::string& key, const std::string& value, std::string& stringValue);
    qboolean ParseVector3KeyValue(const std::string& key, const std::string& value, vec3_t& vectorValue);

    //
    // The actual entity this class is a member of.
    //
    Entity *serverEntity = nullptr;

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    // 
    //---------------------------------
    // -- Flags
    // Entity flags, general flags, flags... :) 
    int32_t flags = 0;
    // Entity spawn flags (Such as, is this a dropped item?)
    int32_t spawnFlags = 0;

    //---------------------------------
    // -- Strings.
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

    //---------------------------------
    // -- Types (Move, Water, what have ya? Add in here.)
    // Move Type. (MoveType::xxx)
    int32_t moveType = MoveType::None;
    // WaterType::xxxx
    int32_t waterType = 0; // TODO: Introduce WaterType "enum".
    // WaterLevel::xxxx
    int32_t waterLevel = WaterLevel::None;

    //---------------------------------
    // -- Physics
    // Angle direction: Set in Trenchbroom -1 = up -2 = down.
    //float angle;
    // Velocity.
    vec3_t velocity = vec3_zero();
    // Angular Velocity.
    vec3_t angularVelocity = vec3_zero();
    // Mass
    int32_t mass = 0;
    // Per entity gravity multiplier (1.0 is normal). TIP: Use for lowgrav artifact, flares
    float gravity = 1.0f;
    
    //-----------------------------------
    // -- Pointers.
    // Goal Entity.
    Entity* goalEntityPtr = nullptr;
    // Move Target Entity.
    Entity* moveTargetPtr = nullptr;
    // The entity that activated this
    SVGBaseEntity* activatorEntityPtr = nullptr;
    
    // Yaw Speed. (Should be for monsters...)
    float yawSpeed = 0.f;
    // Ideal Yaw Angle. (Should be for monsters...)
    float idealYawAngle = 0.f;

    //------------------------------------
    // Noise indexes.
    int32_t noiseIndexA = 0;
    int32_t noiseIndexB = 0;

    //------------------------------------
    // Timing.
    // The next 'think' time, determines when to call the 'think' callback.
    float nextThinkTime = 0.f;
    // Delay before calling trigger execution.
    float delayTime = 0.f;
    // Wait time before triggering at all, in case it was set to auto.
    float waitTime = 0.f;

    // Ground Entity link count. (To keep track if it is linked or not.)
    int32_t groundEntityLinkCount = 0;

    //------------------------------------
    // Entity Status.
    // Current health.
    int32_t health = 0;
    // Maximum health.
    int32_t maxHealth = 0;

    //------------------------------------
    // Entity GAME settings.
    // The height above the origin, this is where EYE SIGHT is at.
    int32_t viewHeight = 0;
    // Determines how to interpret, take damage like a man or like a ... ? Yeah, pick up soap.
    int32_t takeDamage = 0;
    // Actual damage it does if encountered or fucked around with.
    int32_t damage = 0;
    // Dead Flag. (Are we dead, dying or...?)
    int32_t deadFlag = 0;
    // Count (usually used for SVGBaseItem)
    int32_t count = 0;
    // Style/AreaPortal
    int32_t style = 0;

    //
    // This one resides here... for now.
    //
    //------------------------------------
    //float delay;

    //
    // Entity pointers.
    // 
    //------------------------------------
    // Current active enemy, NULL if not any.    
    SVGBaseEntity *enemyEntity = nullptr;
    // Ground entity we're standing on.
    SVGBaseEntity *groundEntity = nullptr;
    // Old enemy, NULL if not any.
    SVGBaseEntity *oldEnemyEntity = nullptr;

    // Owner pointer. (Such as, did the player fire a blaster bolt? If so, the owner is...)
    SVGBaseEntity* ownerEntity = nullptr;

    // Team Chain Pointer.
    SVGBaseEntity* teamChainEntity = nullptr;
    // Master Pointer.
    SVGBaseEntity* teamMasterEntity = nullptr;
    
public:
    //
    // Ugly, but effective callback SET methods.
    //
    // Sets the 'Think' callback function.
    template<typename function>
    inline void SetThinkCallback(function f)
    {
        thinkFunction = static_cast<ThinkCallbackPointer>(f);
    }
    inline qboolean HasThinkCallback() {
        return (thinkFunction != nullptr ? true : false);
    }

    // Sets the 'Use' callback function.
    template<typename function>
    inline void SetUseCallback(function f)
    {
        useFunction = static_cast<UseCallbackPointer>(f);
    }

    // Sets the 'Touch' callback function.
    template<typename function>
    inline void SetTouchCallback(function f)
    {
        touchFunction = static_cast<TouchCallbackPointer>(f);
    }
    inline qboolean HasTouchCallback() {
        return (touchFunction != nullptr ? true : false);
    }

    // Sets the 'Blocked' callback function.
    template<typename function>
    inline void SetBlockedCallback(function f)
    {
        blockedFunction = static_cast<BlockedCallbackPointer>(f);
    }

    // Sets the 'SetTakeDamage' callback function.
    template<typename function>
    inline void SetTakeDamageCallback(function f)
    {
        takeDamageFunction = static_cast<TakeDamageCallbackPointer>(f);
    }
    inline qboolean HasTakeDamageCallback() {
        return (takeDamageFunction != nullptr ? true : false);
    }

    // Sets the 'Die' callback function.
    template<typename function>
    inline void SetDieCallback(function f)
    {
        dieFunction = static_cast<DieCallbackPointer>(f);
    }


protected:
    //
    // Callback function pointers.
    //
    ThinkCallbackPointer        thinkFunction = nullptr;
    UseCallbackPointer          useFunction = nullptr;
    TouchCallbackPointer        touchFunction = nullptr;
    BlockedCallbackPointer      blockedFunction = nullptr;
    TakeDamageCallbackPointer   takeDamageFunction = nullptr;
    DieCallbackPointer          dieFunction = nullptr;

public:
    //
    // Callback implementations that can be set by all child entities.
    //
    void SVGBaseEntityThinkFree(void);
    // "No" thinking
    void SVGBaseEntityThinkNull() { }
};