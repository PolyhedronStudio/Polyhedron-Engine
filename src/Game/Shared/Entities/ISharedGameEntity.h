/***
*
*	License here.
*
*	@file
*
*	SharedGame GameEntity Interface.
* 
***/
#pragma once

// Required.
#include "../../../Shared/Shared.h"
#include "TypeInfo.h"

/**
*   ISharedGameEntity
**/
class ISharedGameEntity {
public:
    // Runtime type information
    DefineTopAbstractClass( ISharedGameEntity );

    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    virtual ~ISharedGameEntity() = default;
    

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
    *   SharedGame GameEntity Interface Functions.
    * 
    *   These MUST BE IMPLEMENTED by any eventual "Base" entity derived from this interface.
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
	*	@brief	This function can be overrided, it should always call upon its Base::SpawnKey function
	*			in order to make sure that spawnkeys from the inherited base class get parsed
	*			and set accordingly as well.
	**/
    virtual void SpawnKey(const std::string& key, const std::string& value) = 0; // Called for each key:value when parsing the entity dictionary.



    /**
    *
    *   The following functions can be implemented by both client and server game modules.
    *
    *   Depending on the scenario a game module implements these functions either as a
    *   full blown implementation, or a "stub" method. This ensures that various bits of
    *   an entity's code can be shared to both game modules.
    **/
    /**
    *   @brief  Link entity to world for collision testing using gi.LinkEntity.
    **/
    virtual void LinkEntity() = 0;
    /**
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    virtual void UnlinkEntity() = 0;
    
    /**
    *   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
    *           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
    *           on us during the current server game frame we're processing.
    **/
    virtual void Remove() = 0;

    /**
    *   @return Pointer to the client/server side POD Entity.
    **/
    virtual PODEntity* GetPODEntity() = 0;
    /**
    *   @brief  Sets the pointer ot the client/server side POD Entity.
    *           Used only in SVG_FreeEntity and SVG_CreateGameEntity.
    **/
    virtual void SetPODEntity(PODEntity* podEntity) = 0;



    /**
    *
    *
    *   Dispatch Callback Functionalities.
    *
    *
    **/
    // template<typename function> inline void SetThinkCallback(function f) = 0;
    virtual const qboolean HasThinkCallback() = 0;

    // template<typename function> inline void SetUseCallback(function f) = 0;
    virtual void DispatchUseCallback(GameEntity* other, GameEntity* activator) = 0;
    virtual const qboolean HasUseCallback() = 0;

    // template<typename function> inline void SetDieCallback(function f) = 0;
    virtual void DispatchDieCallback(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) = 0;
    virtual const qboolean HasDieCallback() = 0;

    // template<typename function> inline void SetBlockedCallback(function f) = 0;
    virtual void DispatchBlockedCallback(GameEntity* other) = 0;
    virtual const qboolean HasBlockedCallback() = 0;

    // template<typename function> inline void SetTouchCallback(function f) = 0;
    virtual void DispatchTouchCallback(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf) = 0;
    virtual const qboolean HasTouchCallback() = 0;

    // template<typename function> inline void SetDamageCallback(function f) = 0;
    virtual void DispatchTakeDamageCallback(GameEntity* other, float kick, int32_t damage) = 0;
    virtual const qboolean HasTakeDamageCallback() = 0;

    // template<typename function> inline void SetStopCallback(function f) = 0;
    virtual void DispatchStopCallback() = 0;
    virtual const qboolean HasStopCallback() = 0;



public:
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
    virtual void UseTargets( GameEntity* activatorOverride = nullptr ) = 0;



    /**
    *
    *
    *   Set/Get:
    *
    *
    **/
    /**
    *   @returns The local center(world-space) of the entity's Bounding Box.
    **/
    virtual vec3_t          GetAbsoluteCenter() = 0;

    /**
    *   @brief Get/Set: BoundingBox Mins
    **/
    virtual const vec3_t&   GetAbsoluteMin() = 0;
    virtual void            SetAbsoluteMin(const vec3_t &absMin) = 0;

    /**
    *   @brief Get/Set: BoundingBox Maxs
    **/
    virtual const vec3_t&   GetAbsoluteMax() = 0;
    virtual void            SetAbsoluteMax(const vec3_t &absMax) = 0;

    /**
    *   @brief Get/Set: Activator
    **/
    virtual GameEntity*    GetActivator() = 0;
    virtual void            SetActivator(GameEntity* activator) = 0;

    /**
    *   @brief Get/Set: Angles
    **/
    virtual const vec3_t&   GetAngles() = 0;
    virtual void            SetAngles(const vec3_t& angles) = 0;

    /**
    *   @brief Get/Set: Angular Velocity
    **/
    virtual const vec3_t&   GetAngularVelocity() = 0;
    virtual void            SetAngularVelocity(const vec3_t& angularVelocity) = 0;

    /**
    *   @return The local center(model-space) of the entity's Bounding Box.
    **/
    virtual vec3_t          GetCenter() = 0;

    /**
    *   @brief Set: Mins and Maxs determining the entity's Bounding Box
    **/
    virtual void            SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) = 0;

    /**
    *   @brief Get/Set: Classname
    **/
    virtual const std::string   GetClassname() = 0;
    virtual void                SetClassname(const std::string &classname) = 0;

    /**
    *   @brief Get: Entity Client
    **/
    virtual gclient_s* GetClient() = 0;

    /**
    *   @brief Get/Set: Clip Mask
    **/
    virtual const int32_t   GetClipMask() = 0;
    virtual void            SetClipMask(const int32_t clipMask) = 0;

    /**
    *   @brief Get/Set: Count
    **/
    virtual const int32_t   GetCount() = 0;
    virtual void            SetCount(const int32_t count) = 0;

    /**
    *   @brief Get/Set: Damage
    **/
    virtual const int32_t   GetDamage() = 0;
    virtual void            SetDamage(const int32_t damage) = 0;

    /**
    *   @brief Get/Set: Dead Flag
    **/
    virtual const int32_t   GetDeadFlag() = 0;
    virtual void            SetDeadFlag(const int32_t deadFlag) = 0;

    /**
    *   @brief Get/Set: Delay Time
    **/
    virtual const Frametime&    GetDelayTime() = 0;
    virtual void                SetDelayTime(const Frametime& delayTime) = 0;

    /**
    *   @brief Get/Set: Effects
    **/
    virtual const uint32_t  GetEffects() = 0;
    virtual void            SetEffects(const uint32_t effects) = 0;

    /**
    *   @brief Get/Set: Enemy
    **/
    virtual GameEntity*    GetEnemy() = 0;
    virtual void            SetEnemy(GameEntity* enemy) = 0;

    /**
    *   @brief Get: Entity Dictionary.
    **/
    virtual SpawnKeyValues &GetEntityDictionary() = 0;

    /**
    *   @brief Get/Set: Event ID
    **/
    virtual const uint8_t   GetEventID() = 0;
    virtual void            SetEventID(const uint8_t eventID) = 0;

    /**
    *   @brief Get/Set: Flags
    **/
    virtual const int32_t   GetFlags() = 0;
    virtual void            SetFlags(const int32_t flags) = 0;

    /**
    *   @brief Get/Set: Animation Frame
    **/
    virtual const float     GetAnimationFrame() = 0;
    virtual void            SetAnimationFrame(const float frame) = 0;

    /**
    *   @brief Get/Set: Gravity
    **/
    virtual const float     GetGravity() = 0;
    virtual void            SetGravity(const float gravity) = 0;

    /**
    *   @brief Get/Set: Ground Entity
    **/
    //virtual SGEntityHandle  GetGroundEntityHandle() = 0;
    //virtual void            SetGroundEntity(GameEntity* groundEntity) = 0;
    // TODO TODO TODO: Fix it so it returns the actual ground entity....
    virtual SGEntityHandle   &GetGroundEntityHandle() = 0;
	virtual PODEntity		 *GetGroundPODEntity() = 0;
	virtual void             SetGroundEntity(const SGEntityHandle &ehGroundEntity) = 0;


    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    virtual int32_t         GetGroundEntityLinkCount() = 0;
    virtual void            SetGroundEntityLinkCount(int32_t groundEntityLinkCount) = 0;

    /**
    *   @brief Get/Set: Health
    **/
    virtual const int32_t   GetHealth() = 0;
    virtual void            SetHealth(const int32_t health) = 0;

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    virtual const float     GetIdealYawAngle() = 0;
    virtual void            SetIdealYawAngle(const float idealYawAngle) = 0;

    /**
    *   @brief Is/Set: In Use.
    **/
    virtual const qboolean  IsInUse() = 0;
    virtual void            SetInUse(const qboolean inUse) = 0;

    /**
    *   @brief Get/Set: Kill Target.
    **/
    virtual const std::string&  GetKillTarget() = 0;
    virtual void                SetKillTarget(const std::string& killTarget) = 0;

    /**
    *   @brief Get/Set: Link Count.
    **/
    virtual const int32_t   GetLinkCount() = 0;
    virtual void            SetLinkCount(const int32_t linkCount) = 0;

    /**
    *   @brief Get/Set: Mass
    **/
    virtual int32_t         GetMass() = 0;
    virtual void            SetMass(const int32_t mass) = 0;

    /**
    *   @brief Get/Set: Max Health
    **/
    virtual const int32_t   GetMaxHealth() = 0;
    virtual void            SetMaxHealth(const int32_t maxHealth) = 0;

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    virtual const vec3_t&   GetMaxs() = 0;
    virtual void            SetMaxs(const vec3_t& maxs) = 0;

    /**
    *   @brief Get/Set: Message
    **/
    virtual const std::string&  GetMessage() = 0;
    virtual void                SetMessage(const std::string& message) = 0;

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    virtual const vec3_t&   GetMins() = 0;
    virtual void            SetMins(const vec3_t& mins) = 0;
   
    /**
    *   @brief Get/Set: Model
    **/
    virtual const std::string&  GetModel() = 0;
    virtual void                SetModel(const std::string &model) = 0;

    /**
    *   @brief Get/Set: Model Index 1
    **/
    virtual const int32_t   GetModelIndex() = 0;
    virtual void            SetModelIndex(const int32_t index) = 0;
    /**
    *   @brief Get/Set: Model Index 2
    **/
    virtual const int32_t   GetModelIndex2() = 0;
    virtual void            SetModelIndex2(const int32_t index) = 0;
    /**
    *   @brief Get/Set: Model Index 3
    **/
    virtual const int32_t   GetModelIndex3() = 0;
    virtual void            SetModelIndex3(const int32_t index) = 0;
    /**
    *   @brief Get/Set: Model Index 4
    **/
    virtual const int32_t   GetModelIndex4() = 0;
    virtual void            SetModelIndex4(const int32_t index) = 0;

    /**
    *   @brief Get/Set: Move Type.
    **/
    virtual const int32_t   GetMoveType() = 0;
    virtual void            SetMoveType(const int32_t moveType) = 0;

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    virtual const GameTime& GetNextThinkTime() = 0;
    virtual void            SetNextThinkTime(const Frametime& nextThinkTime) = 0;

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    virtual const int32_t   GetNoiseIndexA() = 0;
    virtual void            SetNoiseIndexA(const int32_t noiseIndexA) = 0;

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    virtual const int32_t   GetNoiseIndexB() = 0;
    virtual void            SetNoiseIndexB(const int32_t noiseIndexB) = 0;

    /**
    *   @brief Get/Set:     State Number
    **/
    virtual const int32_t   GetNumber() = 0;
    virtual void            SetNumber(const int32_t number) = 0;

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    virtual GameEntity*    GetOldEnemy() = 0;
    virtual void            SetOldEnemy(GameEntity* oldEnemy) = 0;

    /**
    *   @brief Get/Set:     Old Origin
    **/
    virtual const vec3_t&   GetOldOrigin() = 0;
    virtual void            SetOldOrigin(const vec3_t& oldOrigin) = 0;

    /**
    *   @brief Get/Set:     Origin
    **/
    virtual const vec3_t&   GetOrigin() = 0;
    virtual void            SetOrigin(const vec3_t& origin) = 0;

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    virtual GameEntity*    GetOwner() = 0;
    virtual void            SetOwner(GameEntity* owner) = 0;

    /**
    *   @brief Get/Set:     Render Effects
    **/
    virtual const int32_t   GetRenderEffects() = 0;
    virtual void            SetRenderEffects(const int32_t renderEffects) = 0;
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    virtual const char*     GetPathTarget() = 0;

    /**
    *   @brief Get/Set:     Server Flags
    **/
    virtual const int32_t   GetServerFlags() = 0;
    virtual void            SetServerFlags(const int32_t serverFlags) = 0;

    /**
    *   @brief Get/Set:     Skin Number
    **/
    virtual const int32_t   GetSkinNumber() = 0;
    virtual void            SetSkinNumber(const int32_t skinNumber) = 0;

    /**
    *   @brief Get/Set:     Entity Size
    **/
    virtual const vec3_t&   GetSize() = 0;
    virtual void            SetSize(const vec3_t& size) = 0;

    /**
    *   @brief Get/Set:     Solid
    **/
    virtual const uint32_t  GetSolid() = 0;
    virtual void            SetSolid(const uint32_t solid) = 0;

    /**
    *   @brief Get/Set:     Sound.
    **/
    virtual const int32_t   GetSound() = 0;
    virtual void            SetSound(const int32_t sound) = 0;

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    virtual const int32_t   GetSpawnFlags() = 0;
    virtual void            SetSpawnFlags(const int32_t spawnFlags) = 0;

    /**
    *   @brief Get/Set:     Entity State
    **/
    virtual const EntityState&   GetState() = 0;
    virtual void                 SetState(const EntityState &state) = 0;

    /**
    *   @brief Get/Set:     Style
    **/
    virtual const int32_t   GetStyle() = 0;
    virtual void            SetStyle(const int32_t style) = 0;

    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const int32_t   GetTakeDamage() = 0;
    virtual void            SetTakeDamage(const int32_t takeDamage) = 0;
    
    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const std::string&   GetTarget() = 0;
    virtual void                 SetTarget(const std::string& target) = 0;

    /**
    *   @brief Get/Set:     Target Name
    **/
    virtual const std::string&   GetTargetName() = 0;
    virtual void                 SetTargetName(const std::string& targetName) = 0;

    /**
    *   @brief Get/Set:     Team
    **/
    virtual const std::string&   GetTeam() = 0;
    virtual void                 SetTeam(const std::string &team) = 0;

    /**
    *   @brief Get/Set:     Team Chain
    **/
    virtual GameEntity*    GetTeamChainEntity() = 0;
    virtual void            SetTeamChainEntity(GameEntity* entity) = 0;

    /**
    *   @brief Get/Set:     Team Master
    **/
    virtual GameEntity*    GetTeamMasterEntity() = 0;
    virtual void            SetTeamMasterEntity(GameEntity* entity) = 0;

    /**
    *   @brief Get/Set:     Velocity
    **/
    virtual const vec3_t&   GetVelocity() = 0;
    virtual void            SetVelocity(const vec3_t &velocity) = 0;

    /**
    *   @brief Get/Set:     View Height
    **/
    virtual const int32_t   GetViewHeight() = 0;
    virtual void            SetViewHeight(const int32_t height) = 0;

    /**
    *   @brief Get/Set:     Wait Time
    **/
    virtual const Frametime&    GetWaitTime() = 0;
    virtual void                SetWaitTime(const Frametime& waitTime) = 0;

    /**
    *   @brief Get/Set:     Water Level
    **/
    virtual const int32_t   GetWaterLevel() = 0;
    virtual void            SetWaterLevel(const int32_t waterLevel) = 0;

    /**
    *   @brief Get/Set:     Water Type
    **/
    virtual const int32_t   GetWaterType() = 0;
    virtual void            SetWaterType(const int32_t waterType) = 0;

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    virtual const float     GetYawSpeed() = 0;
    virtual void            SetYawSpeed(const float yawSpeed) = 0;


protected:
	/**
	*
	*
	*	Specialized Template Entity Dictionary Parsing Utilities.
	*
	*   @return True on success, false on failure.
	*
	**/
	/**
	*	@brief	Parses the key/value for the float type. (0.1, 0.004899)
	**/
	template<typename T> auto ParseKeyValue(const std::string &key, const std::string &value, T &outValue) {
		// Warning print...
#if defined(SHAREDGAME_SERVERGAME)
		Com_DPrintf("Warning: svEntity(#%i): Called ParseKeyValue called for type: '%s' on key: '%s', but its type needs implementation..\n", podEntity->currentState.number, typeid(T).name(), key.c_str());
#elif defined(SHAREDGAME_CLIENTGAME)
		Com_DPrintf("Warning: clEntity(#%i): Called ParseKeyValue called for type: '%s' on key: '%s', but its type needs implementation..\n", podEntity->clientEntityNumber, typeid(T).name(), key.c_str());
#else
		//Com_Print("Warning: Called specialized template function ParseKeyValue that wasn't compiled for client/server -game modules.\n");
#endif
	};



protected:
	//! Pointer to the entity which owns this game entity.
    PODEntity *podEntity = nullptr;
};


template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string &key, const std::string &value, float &floatNumber) {
    floatNumber = std::stof(value);
    return true;
};
/**
*	@brief	Parses the key/value for the signed numbers, -/+ 
**/
template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string &key, const std::string &value, int32_t &signedNumber) {
    signedNumber = std::stoi(value);
    return true;
};
/**
*	@brief	Parses the key/value for the unsigned numbers(0 and up) type.
**/
template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string &key, const std::string &value, uint32_t &unsignedNumber) {
    unsignedNumber = std::stof(value);
    return true;
};
/**
*	@brief	Parses the key/value for the string type.
**/
template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string &key, const std::string &value, std::string &string) {
    string = value;
    return true;
};
/**
*	@brief	Parses the key/value for the Frametime type. (floating point time numbers. 0.1, 1.2 etc.)
**/
template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string &key, const std::string &value, Frametime &frametime) {
    frametime = Frametime(std::stof(value));
    return true;
};
/**
*	@brief	Parses the key/value for the vec3_t type.
**/
template<> inline auto ISharedGameEntity::ParseKeyValue(const std::string& key, const std::string& value, vec3_t& vector3) {
    // Stores vector fields fetched from string. (Might be corrupted, so we're parsing this nicely.)
    std::vector<std::string> vectorFields;

    // We split it based on the space delimiter. Empties are okay, how can they be empty then? Good question...
    STR_Split(vectorFields, value, " ");

    // Zero out our vector.
    vector3 = vec3_zero();
    int32_t i = 0;
    for (auto& str : vectorFields) {
        vector3[i] = std::stof(str);
        i++;

        if (i > 2)
            break;
    }

    return true;
}
