/***
*
*	License here.
*
*	@file
*
*	Entities that are received by the server-side frame packets such as anything that moves and/or
*	does not have its NOCLIENTS flag set are inherited from CLGBasePacketEntity. These entities come
*	and go on a per frame basis. 
*
*	The client side soul-mate of these entities is generally just an instance of the CLGBasePacketEntity 
*	itself. If there is any need to adjust the way how it is represented on-screen in more advanced 
*	manners than a SetEffects/SetRenderEffects, and/or to try and predict an entities movement and its
*	actions, then one should inherit from this class to provide its client-side counterpart. 
* 
***/
#pragma once

// Client Game GameEntity Interface.
#include "../IClientGameEntity.h"



/**
*   CLGBasePacketEntity
**/
class CLGBasePacketEntity : public IClientGameEntity {
public:
    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    CLGBasePacketEntity(PODEntity *podEntity);
    virtual ~CLGBasePacketEntity() = default;

    // Runtime type information
	DefineGameClass( CLGBasePacketEntity, IClientGameEntity);
	//DefineClass( CLGBasePacketEntity, IClientGameEntity);
	//DefineMapClass( "CLGBasePacketEntity", CLGBaseLocalEntity, IClientGameEntity);
	//DefineGameClass( CLGBasePacketEntity, IClientGameEntity);

    //// Checks if this entity class is exactly the given class
    //// @param entityClass: an entity class which must inherint from SVGBaseEntity
    //template<typename entityClass>
    //bool IsClass() const { // every entity has a ClassInfo, thanks to the DefineXYZ macro
    //    return GetTypeInfo()->IsClass( entityClass::ClassInfo );
    //}

    //// Checks if this entity class is a subclass of another, or is the same class
    //// @param entityClass: an entity class which must inherint from SVGBaseEntity
    //template<typename entityClass>
    //bool IsSubclassOf() const {
    //    return GetTypeInfo()->IsSubclassOf( entityClass::ClassInfo );
    //}

    

    //! Used for returning vectors from a const vec3_t & reference.
    static vec3_t ZeroVec3;
    //! Used for returning strings from a const std::string & reference.
    static std::string EmptyString;

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
    *           Used only in SVG_FreeEntity and SVG_CreateGameEntity.
    **/
    inline void SetPODEntity(PODEntity* podEntity) override {
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
    virtual void SetClassname(const std::string& classname) final { this->classname = classname; };

    /**
    *   [Stub Implementation]
    *   @brief  Link entity to world for collision testing using gi.LinkEntity.
    **/
    void LinkEntity() override { 
		if (podEntity) {
			clgi.LinkEntity(podEntity);
		}
	};
    /**
    *   [Stub Implementation]
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    void UnlinkEntity() override {
		if (podEntity) {
			clgi.UnlinkEntity(podEntity);
		}
	};
    /**
    *   [Stub Implementation]
    *   @brief  Marks the entity to be removed in the next server frame. This is preferred to SVG_FreeEntity, 
    *           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
    *           on us during the current server game frame we're processing.
    **/
    void Remove() override;



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
    void UseTargets( GameEntity* activatorOverride = nullptr ) override {};



    /***
    *
    * 
    *   ClientGame Base Entity Set/Get:
    *
    * 
    ***/
    /**
    *   @brief Get/Set:     Client Entity Flags
    **/
	virtual const int32_t   GetClientFlags() {
		if (podEntity) {
			return podEntity->clientFlags;
		} else {
			return 0;
		}
	};
	virtual void            SetClientFlags(const int32_t clientFlags) {
		if (podEntity) {
			podEntity->clientFlags = clientFlags;
		}
	};

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
    virtual vec3_t GetAbsoluteCenter() override {
        return vec3_scale(GetAbsoluteMax() + GetAbsoluteMin(), 0.5f);
    };

    /**
    *   @brief Get/Set: BoundingBox Mins
    **/
    virtual const vec3_t& GetAbsoluteMin() override { 
        if (podEntity) {
            return podEntity->absMin;
        } else {
            return ZeroVec3;
        }
    };
    virtual void SetAbsoluteMin(const vec3_t &absMin) override {
        if (podEntity) {
            podEntity->absMin = absMin;
        }
    };

    /**
    *   @brief Get/Set: BoundingBox Maxs
    **/
    virtual const vec3_t& GetAbsoluteMax() override { 
        if (podEntity) {
            return podEntity->absMax;
        } else {
            return ZeroVec3;
        }
    };
    virtual void SetAbsoluteMax(const vec3_t &absMax) override {
        if (podEntity) {
            podEntity->absMax = absMax;
        }
    };

    /**
    *   @brief Get/Set: Activator
    **/
    virtual GameEntity* GetActivator() override { return nullptr; };
    virtual void SetActivator(GameEntity* activator) override {};

    /**
    *   @brief Get/Set: Angles
    **/
    virtual const vec3_t& GetAngles() override { 
        if (podEntity) {
            return podEntity->currentState.angles;
        } else {
            return ZeroVec3;
        }
    };
    virtual void SetAngles(const vec3_t& angles) override { 
        if (podEntity) {
            podEntity->currentState.angles = angles;
        }
    };
    /**
    *   @brief Get/Set: Angular Velocity
    **/
    virtual const vec3_t& GetAngularVelocity() override { 
        return angularVelocity;
    };
    virtual void SetAngularVelocity(const vec3_t& angularVelocity) override {
        // This minght need to be networked in the future?
        this->angularVelocity = angularVelocity;
    };

    /**
    *   @return The local center(model-space) of the entity's Bounding Box.
    **/
    virtual inline vec3_t GetCenter() override { return vec3_scale( GetMaxs() + GetMins(), 0.5f ); }

    /**
    *   @brief Set: Mins and Maxs determining the entity's Bounding Box
    **/
    virtual inline void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) override {
		if (podEntity) {
			podEntity->currentState.mins = mins;
			podEntity->currentState.maxs = maxs;
		}
    }

    /**
    *   @brief Get/Set: Classname
    **/
    //virtual const std::string   GetClassname() { return EmptyString; };
    //virtual void                SetClassname(const std::string &classname) {};

    /**
    *   @brief Get: Entity Client
    **/
    virtual gclient_s* GetClient() override { return 0; };

    /**
    *   @brief Get/Set: Clip Mask
    **/
    virtual inline const int32_t    GetClipMask() override { 
		if (podEntity) {
			return podEntity->clipMask; 
		} else {
			return 0;
		}
	};
    virtual inline void             SetClipMask(const int32_t clipMask) override { 
		if (podEntity) {
			podEntity->clipMask = clipMask; 
		}
	};

    /**
    *   @brief Get/Set: Count
    **/
    virtual const int32_t   GetCount() override { return 0; };
    virtual void            SetCount(const int32_t count) override {};

    /**
    *   @brief Get/Set: Damage
    **/
    virtual const int32_t   GetDamage() override { return 0; };
    virtual void            SetDamage(const int32_t damage) override {};

    /**
    *   @brief Get/Set: Dead Flag
    **/
    virtual const int32_t   GetDeadFlag() override { return 0; };
    virtual void            SetDeadFlag(const int32_t deadFlag) override {};

    /**
    *   @brief Get/Set: Delay Time
    **/
    virtual const Frametime&    GetDelayTime() override { return delayTime; };
    virtual void                SetDelayTime(const Frametime &delayTime) override { this->delayTime = delayTime; };

    /**
    *   @brief Get/Set: Effects
    **/
    virtual const uint32_t  GetEffects() override { 
		if (podEntity) {
			return podEntity->currentState.effects; 
		} else {
			return 0;
		}
	};
    virtual void            SetEffects(const uint32_t effects) override {
		if (podEntity) {
			podEntity->currentState.effects = effects;
		}
	};

    /**
    *   @brief Get/Set: Enemy
    **/
    virtual GameEntity*    GetEnemy() override { return 0; };
    virtual void            SetEnemy(GameEntity* enemy) override {};

    /**
    *   @brief Get: Entity Dictionary.
    **/
    virtual SpawnKeyValues &GetEntityDictionary() override { return podEntity->spawnKeyValues; };

    /**
    *   @brief Get/Set: Event ID
    **/
    virtual const uint8_t   GetEventID() override { 
		if (podEntity) { 
			return podEntity->currentState.eventID; 
		} else {
			return 0;
		}
	};
    virtual void            SetEventID(const uint8_t eventID) override {
		if (podEntity) {
			podEntity->currentState.eventID = eventID;
		}
	};

    /**
    *   @brief Get/Set: Flags
    **/
    virtual const int32_t   GetFlags() override { return 0; };
    virtual void            SetFlags(const int32_t flags) override { this->flags = flags; };

    /**
    *   @brief Get/Set: Animation Frame
    **/
    virtual const float     GetAnimationFrame() override { return 0.f; };
    virtual void            SetAnimationFrame(const float frame) override {};

    /**
    *   @brief Get/Set: Gravity
    **/
    virtual const float     GetGravity() override { return gravity; };
    virtual void            SetGravity(const float gravity) override { this->gravity = gravity; };

    /**
    *   @brief Get/Set: Ground Entity
    **/
    inline SGEntityHandle   &GetGroundEntityHandle() override { return groundEntityHandle; }
	inline PODEntity		*GetGroundPODEntity() override { return groundEntityHandle.Get(); }
	inline void             SetGroundEntity(GameEntity* groundEntity) { groundEntityHandle = (groundEntity ? groundEntity->GetPODEntity() : nullptr); } //this->groundEntity = groundEntity; }


    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    virtual int32_t         GetGroundEntityLinkCount() override { return groundEntityLinkCount; };
    virtual void            SetGroundEntityLinkCount(int32_t groundEntityLinkCount) override { this->groundEntityLinkCount = groundEntityLinkCount; };

    /**
    *   @brief Get/Set: Health
    **/
    virtual const int32_t   GetHealth() override { return health; };
    virtual void            SetHealth(const int32_t health) override { this->health = health; };

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    virtual const float     GetIdealYawAngle() override { return idealYawAngle; };
    virtual void            SetIdealYawAngle(const float idealYawAngle) override { this->idealYawAngle = idealYawAngle; };

    /**
    *   @brief Is/Set: In Use.
    **/
    virtual const qboolean        IsInUse() override { //return 0; };
        if (podEntity) {
            return cl->frame.number == podEntity->serverFrame;
        } 

        return false;
    }
    virtual void            SetInUse(const qboolean inUse) override {
		if (podEntity) {
			podEntity->inUse = inUse;
		}
	};

    /**
    *   @brief Get/Set: Kill Target.
    **/
    virtual const std::string&  GetKillTarget() override { return killTargetStr; };
    virtual void                SetKillTarget(const std::string& killTarget) override { killTargetStr = killTarget;};

    /**
    *   @brief Get/Set: Link Count.
    **/
    virtual const int32_t   GetLinkCount() override { 
		if (podEntity) {
			return podEntity->linkCount; 
		} else {
			return 0;
		}
	};
    virtual void            SetLinkCount(const int32_t linkCount) override { 
		if (podEntity) {
			podEntity->linkCount = linkCount; 
		}
	};

    /**
    *   @brief Get/Set: Mass
    **/
    virtual int32_t         GetMass() override { return mass; };
    virtual void            SetMass(const int32_t mass) override { this->mass = mass; };

    /**
    *   @brief Get/Set: Max Health
    **/
    virtual const int32_t   GetMaxHealth() override { return maxHealth; };
    virtual void            SetMaxHealth(const int32_t maxHealth) override { this->maxHealth = maxHealth; };

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    virtual const vec3_t&   GetMaxs() override { 
		if (podEntity) {
			return podEntity->currentState.maxs;
		}
		return ZeroVec3;
	};
    virtual void            SetMaxs(const vec3_t& maxs) override {
		if (podEntity) {
			podEntity->currentState.maxs = maxs;
		}
	};
    /**
    *   @brief Get/Set: Message
    **/
    virtual const std::string&  GetMessage() override { return EmptyString; };
    virtual void                SetMessage(const std::string& message) override {};

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    virtual const vec3_t&   GetMins() override { 
		if (podEntity) {
			return podEntity->currentState.mins;
		}
		return ZeroVec3;
	};
    virtual void            SetMins(const vec3_t& mins) override {
		if (podEntity) {
			podEntity->currentState.mins = mins;
		}
	};
   
    /**
    *   @brief Get/Set: Model
    **/
    virtual const std::string&  GetModel() override { return model; };
    virtual void                SetModel(const std::string &model) override {
			// Set modelstr.
			this->model = model;

			// If it is an inline model, get the size information for it.
			if (model[0] == '*') {
				mmodel_t *inlineModel = clgi.BSP_InlineModel(model.c_str());

				if (inlineModel) {
					podEntity->mins = inlineModel->mins;
					podEntity->maxs = inlineModel->maxs;
					
					// Link it for collision testing.
					LinkEntity();
				}
			}

			// Update model index.
			SetModelIndex(clgi.R_RegisterModel(model.c_str()));
	};

    /**
    *   @brief Get/Set: Model Index 1
    **/
    virtual const int32_t   GetModelIndex() override { 
		if (podEntity) {
			return podEntity->currentState.modelIndex;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex(const int32_t index) override {
		if (podEntity) {
			podEntity->currentState.modelIndex = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 2
    **/
    virtual const int32_t   GetModelIndex2() override { 
		if (podEntity) {
			return podEntity->currentState.modelIndex2;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex2(const int32_t index) override {
		if (podEntity) {
			podEntity->currentState.modelIndex2 = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 3
    **/
    virtual const int32_t   GetModelIndex3() override { 
		if (podEntity) {
			return podEntity->currentState.modelIndex3;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex3(const int32_t index) override {
		if (podEntity) {
			podEntity->currentState.modelIndex3 = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 4
    **/
    virtual const int32_t   GetModelIndex4() override { 
		if (podEntity) {
			return podEntity->currentState.modelIndex4;
		} else {
			return 0; 
		}
	}
    virtual void            SetModelIndex4(const int32_t index) override { 
		if (podEntity) {
			podEntity->currentState.modelIndex4 = index;
		}
	};

    /**
    *   @brief Get/Set: Move Type.
    **/
	virtual const int32_t   GetMoveType() override { return moveType; };
    virtual void			SetMoveType(const int32_t moveType) override { this->moveType = moveType; };

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    //virtual const float     GetNextThinkTime() { return 0.f; };
    //virtual void            SetNextThinkTime(const float nextThinkTime) {};

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    virtual const int32_t   GetNoiseIndexA() override { return noiseIndexA; };
    virtual void            SetNoiseIndexA(const int32_t noiseIndexA) override { this->noiseIndexA = noiseIndexA; };

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    virtual const int32_t   GetNoiseIndexB() override { return noiseIndexB; };
    virtual void            SetNoiseIndexB(const int32_t noiseIndexB) override { this->noiseIndexB = noiseIndexB; };

    /**
    *   @brief Get/Set:     State Number
    **/
    virtual const int32_t   GetNumber() override { 
		if (podEntity) {
			return podEntity->currentState.number;
		} else {
			return 0;
		}
	};
    virtual void            SetNumber(const int32_t number) override {
		if (podEntity) {
			podEntity->currentState.number = number;
		}	
	};

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    virtual GameEntity*    GetOldEnemy() override { return oldEnemyEntity; }
    virtual void            SetOldEnemy(IClientGameEntity* oldEnemy) override { this->oldEnemyEntity = oldEnemy; };

    /**
    *   @brief Get/Set:     Old Origin
    **/
    virtual const vec3_t&   GetOldOrigin() override { 
		if (podEntity) {
			return podEntity->currentState.oldOrigin;
		} else {
			return ZeroVec3;
		}
	};
    virtual void            SetOldOrigin(const vec3_t& oldOrigin) override {
		if (podEntity) {
			podEntity->currentState.oldOrigin = oldOrigin;
		}
	};

    /**
    *   @brief Get/Set:     Origin
    **/
    virtual const vec3_t&   GetOrigin() override { 
		if (podEntity) {
			return podEntity->currentState.origin;
		} else {
			return ZeroVec3;
		}
	};
    virtual void            SetOrigin(const vec3_t& origin) override {
		if (podEntity) {
			podEntity->currentState.origin = origin;
		}
	};

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    virtual GameEntity*    GetOwner() override { return ownerEntity; };
    virtual void            SetOwner(IClientGameEntity* owner) override { ownerEntity = owner; };

    /**
    *   @brief Get/Set:     Render Effects
    **/
    virtual const int32_t   GetRenderEffects() override { 
		if (podEntity) {
			return podEntity->currentState.renderEffects;
		} else {
			return 0;
		}
	};
    virtual void            SetRenderEffects(const int32_t renderEffects) override {
		if (podEntity) {
			podEntity->currentState.renderEffects = renderEffects;
		}
	};
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    virtual const char*     GetPathTarget() override { return EmptyString.c_str(); };

    /**
    *   @brief Get/Set:     Server Flags
    **/
    virtual const int32_t   GetServerFlags() override { return 0; };
    virtual void            SetServerFlags(const int32_t serverFlags) override {};

    /**
    *   @brief Get/Set:     Skin Number
    **/
    virtual const int32_t   GetSkinNumber() override { 
		if (podEntity) {
			return podEntity->currentState.skinNumber;
		} else {
			return 0;
		}
	};
    virtual void            SetSkinNumber(const int32_t skinNumber) override {
		if (podEntity) {
			podEntity->currentState.skinNumber = skinNumber;
		}
	};

    /**
    *   @brief Get/Set:     Entity Size
    **/
    virtual const vec3_t&   GetSize() override { 
		if (podEntity) {
			return podEntity->size;	
		} else {
			return ZeroVec3; 
		}
	};
    virtual void            SetSize(const vec3_t& size) override {
		if (podEntity) {
			podEntity->size = size;	
		}
	};

    /**
    *   @brief Get/Set:     Solid
    **/
    virtual const uint32_t  GetSolid() override { 
		if (podEntity) {
			return podEntity->currentState.solid;
		} else {
			return 0;
		}
	};
    virtual void            SetSolid(const uint32_t solid) override {
		if (podEntity) {
			podEntity->currentState.solid = solid;
		}
	};

    /**
    *   @brief Get/Set:     Sound.
    **/
    virtual const int32_t   GetSound() override { 
		if (podEntity) {
			return podEntity->currentState.sound;
		} else {
			return 0;
		}
	};
    virtual void            SetSound(const int32_t sound) override {
		if (podEntity) {
			podEntity->currentState.sound = sound;
		}
	};

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    virtual const int32_t   GetSpawnFlags() override { return spawnFlags; };
    virtual void            SetSpawnFlags(const int32_t spawnFlags) override { this->spawnFlags = spawnFlags; };

    /**
    *   @brief Get/Set:     Entity State
    **/
    virtual const EntityState&   GetState() override { return podEntity->currentState; };
    virtual void                 SetState(const EntityState &state) override { podEntity->currentState = state; };

    /**
    *   @brief Get/Set:     Style
    **/
    virtual const int32_t   GetStyle() override { return style; };
    virtual void            SetStyle(const int32_t style) override { this->style = style; };

    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const int32_t   GetTakeDamage() override { return takeDamage; };
    virtual void            SetTakeDamage(const int32_t takeDamage) override { this->takeDamage = takeDamage; };
    
    /**
    *   @brief Get/Set:     Target
    **/
    virtual const std::string&   GetTarget() override { return targetStr; };
    virtual void                 SetTarget(const std::string& target) override{ this->targetStr = target; };

    /**
    *   @brief Get/Set:     Target Name
    **/
    virtual const std::string&   GetTargetName() override { return targetNameStr; };
    virtual void                 SetTargetName(const std::string& targetName) override { this->targetNameStr = targetName; };

    /**
    *   @brief Get/Set:     Team
    **/
    virtual const std::string&   GetTeam() override { return teamStr; };
    virtual void                 SetTeam(const std::string &team) override{ this->teamStr = team;};

    /**
    *   @brief Get/Set:     Team Chain
    **/
    virtual IClientGameEntity*	GetTeamChainEntity() override { return this->teamChainEntity; }
    virtual void				SetTeamChainEntity(GameEntity* entity) override{ this->teamChainEntity = teamChainEntity; };

    /**
    *   @brief Get/Set:     Team Master
    **/
    virtual IClientGameEntity*	GetTeamMasterEntity() override { return teamMasterEntity; };
    virtual void				SetTeamMasterEntity(IClientGameEntity* entity) override { this->teamMasterEntity = teamMasterEntity; };

    /**
    *   @brief Get/Set:     Velocity
    **/
    virtual const vec3_t&   GetVelocity() override { return velocity; };
    virtual void            SetVelocity(const vec3_t &velocity) override{ this->velocity = velocity; };

    /**
    *   @brief Get/Set:     View Height
    **/
    virtual const int32_t   GetViewHeight() override { return viewHeight; };
    virtual void            SetViewHeight(const int32_t height) override { this->viewHeight = height; };

    /**
    *   @brief Get/Set:     Wait Time
    **/
    virtual const Frametime&    GetWaitTime() override { return waitTime; }
    virtual void                SetWaitTime(const Frametime &waitTime) override { this->waitTime = waitTime; };

    /**
    *   @brief Get/Set:     Water Level
    **/
    virtual const int32_t   GetWaterLevel() override { return waterLevel; };
    virtual void            SetWaterLevel(const int32_t waterLevel) override { this->waterLevel = waterLevel; };

    /**
    *   @brief Get/Set:     Water Type
    **/
    virtual const int32_t   GetWaterType() override { return waterType; }
    virtual void            SetWaterType(const int32_t waterType) override { this->waterType = waterType; };

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    virtual const float     GetYawSpeed() override { return yawSpeed; };
    virtual void            SetYawSpeed(const float yawSpeed) override { this->yawSpeed = yawSpeed; };


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
    virtual inline float GetAcceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float GetDeceleration() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetEndPosition() { return ZeroVec3; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline float    GetSpeed() { return 0.f; }
    /**
    *   @brief  Placeholder, implemented by SVGBaseMover, and derivates of that class.
    **/
    virtual inline const vec3_t& GetStartPosition() { return ZeroVec3; }



    /***
    *
    * 
    *   Refresh Related Functions.
    *
    * 
    ***/
	/**
	*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
	**/
	virtual void PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) override;

private:
	virtual void ProcessSkeletalAnimationForTime(uint64_t time) ;

protected:
	/**
	*
	*
	*	Client Specific.
	*
	*
	**/
    //! Refresh Entity Object.
    r_entity_t refreshEntity = {};



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
    virtual void DispatchUseCallback(IClientGameEntity* other, IClientGameEntity* activator) override;
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    virtual void DispatchDieCallback(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point) override;
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  other:  
    **/
    virtual void DispatchBlockedCallback(IClientGameEntity* other) override;
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  self:   
    *   @param  other:  
    *   @param  plane:  
    *   @param  surf:   
    **/
    virtual void DispatchTouchCallback(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) override;
    /**
    *   @brief  Dispatches 'TakeDamage' callback.
    *   @param  other:
    *   @param  kick:
    *   @param  damage:
    **/
    virtual void DispatchTakeDamageCallback(IClientGameEntity* other, float kick, int32_t damage) override;



protected:
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
    std::string classname = "";		// This is only set when the entity has been spawned from the BSP entity string.
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
    //Entity* goalEntityPtr = nullptr;
    //! Move Target Entity.
    //Entity* moveTargetPtr = nullptr;
    //! The entity that activated this
    IClientGameEntity* activatorEntityPtr = nullptr;


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
    IClientGameEntity* enemyEntity      = nullptr;
    //! Ground entity we're standing on.
    IClientGameEntity* groundEntity     = nullptr;
	SGEntityHandle groundEntityHandle;
    //! Old enemy.
    IClientGameEntity* oldEnemyEntity   = nullptr;
    //! Owner. (Such as, did the player fire a blaster bolt? If so, the owner is...)
    IClientGameEntity* ownerEntity      = nullptr;
    //! Team Chain.
    IClientGameEntity* teamChainEntity  = nullptr;
    //! Team Master.
    IClientGameEntity* teamMasterEntity = nullptr;



public:
    /**
    * 
	(
    *   Entity Utility callbacks that can be set as a nextThink function.
    * 
	*
    **/
    /**
    *   @brief  Callback method to use for freeing this entity. It calls upon Remove()
    **/
    void CLGBasePacketEntityThinkFree(void);

    /**
    *   @brief  Callback for assigning when "no thinking" behavior is wished for.
    **/
    void CLGBasePacketEntityThinkNull() { }
};