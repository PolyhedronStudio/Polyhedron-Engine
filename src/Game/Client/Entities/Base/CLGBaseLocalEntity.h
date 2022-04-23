/***
*
*	License here.
*
*	@file
*
*	Entities that are intended for client-side only, such as decorative entities like models, gibs or
*	and but not limited to environmental particle effects that do NOT depend on ANY server-side 
*	interactions are inherited from this class. 
*
*	The inherited CLGBaseLocalEntity game entity classes are not received from the server but are parsed 
*	and spawned directly, local to the client only, by the ClientGameWorld itself during load time.
* 
***/
#pragma once

// Client Game GameEntity Interface.
#include "../IClientGameEntity.h"



/**
*   CLGBaseLocalEntity
**/
class CLGBaseLocalEntity : public IClientGameEntity {
public:
    /**
    *
    *   Constructor/Destructor AND TypeInfo related.
    *
    **/
    //! Constructor/Destructor.
    CLGBaseLocalEntity(PODEntity *podEntity);
    virtual ~CLGBaseLocalEntity() = default;

    // Runtime type information
	DefineMapClass( "CLGBaseLocalEntity", CLGBaseLocalEntity, IClientGameEntity);

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
	*   @return	Human-readable string classname.
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
    virtual void SetClassname(const std::string& classname) final { this->classname = classname; };
    /**
    *   [Stub Implementation]
    *   @brief  Link entity to world for collision testing using gi.LinkEntity.
    **/
    void LinkEntity() override { 
		//if (podEntity) {
		//	podEntity->linkCount++; 
		//}
		if (podEntity) {
			clgi.LinkEntity(podEntity);
		}
	};

    /**
    *   [Stub Implementation]
    *   @brief  Unlink the entity from the world for collision testing.
    **/
    void UnlinkEntity() override {
		//if (podEntity) {
		//	podEntity->linkCount = 0;
		//}
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
    virtual void SetAbsoluteMin(const vec3_t &absMin) {
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
    virtual GameEntity* GetActivator() { return nullptr; };
    virtual void SetActivator(GameEntity* activator) {};

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
			SetMins(mins);
			SetMaxs(maxs);
			//podEntity->currentState.mins = mins;
			//podEntity->currentState.maxs = maxs;
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
    virtual gclient_s* GetClient() { return 0; };

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
    virtual const uint32_t  GetEffects() { 
		if (podEntity) {
			return podEntity->currentState.effects; 
		} else {
			return 0;
		}
	};
    virtual void            SetEffects(const uint32_t effects) {
		if (podEntity) {
			podEntity->currentState.effects = effects;
		}
	};

    /**
    *   @brief Get/Set: Enemy
    **/
    virtual GameEntity*    GetEnemy() { return 0; };
    virtual void            SetEnemy(GameEntity* enemy) {};

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
    virtual const int32_t   GetFlags() { return flags; };
    virtual void            SetFlags(const int32_t flags) { this->flags = flags; };

    /**
    *   @brief Get/Set: Animation Frame
    **/
    virtual const float     GetAnimationFrame() { return 0.f; };
    virtual void            SetAnimationFrame(const float frame) {};

    /**
    *   @brief Get/Set: Gravity
    **/
    virtual const float     GetGravity() { return gravity; };
    virtual void            SetGravity(const float gravity) { this->gravity = gravity; };

    /**
    *   @brief Get/Set: Ground Entity
    **/
    // TODO TODO TODO: Fix it so it returns the actual ground entity....
	virtual SGEntityHandle  GetGroundEntity() { return groundEntity; }; //SGEntityHandle(); };
    virtual void            SetGroundEntity(GameEntity* groundEntity) { this->groundEntity = groundEntity; };

    /**
    *   @brief Get/Set: Ground Entity Link Count
    **/
    virtual int32_t         GetGroundEntityLinkCount() { return groundEntityLinkCount; };
    virtual void            SetGroundEntityLinkCount(int32_t groundEntityLinkCount) { this->groundEntityLinkCount = groundEntityLinkCount; };

    /**
    *   @brief Get/Set: Health
    **/
    virtual const int32_t   GetHealth() { return health; };
    virtual void            SetHealth(const int32_t health) { this->health = health; };

    /**
    *   @brief Get/Set: Ideal Yaw Angle.
    **/
    virtual const float     GetIdealYawAngle() { return idealYawAngle; };
    virtual void            SetIdealYawAngle(const float idealYawAngle) { this->idealYawAngle = idealYawAngle; };

    /**
    *   @brief Is/Set: In Use.
    **/
    virtual const qboolean        IsInUse() override { //return 0; };
        if (podEntity) {
            return podEntity->inUse;
        } 

        return false;
    }
    virtual void            SetInUse(const qboolean inUse) {
        if (podEntity) {
            podEntity->inUse = inUse;
        }
	};

    /**
    *   @brief Get/Set: Kill Target.
    **/
    virtual const std::string&  GetKillTarget() { return killTargetStr; };
    virtual void                SetKillTarget(const std::string& killTarget) { killTargetStr = killTarget;};

    /**
    *   @brief Get/Set: Link Count.
    **/
    virtual const int32_t   GetLinkCount() { 
		if (podEntity) {
			return podEntity->linkCount; 
		} else {
			return 0;
		}
	};
    virtual void            SetLinkCount(const int32_t linkCount) { 
		if (podEntity) {
			podEntity->linkCount = linkCount; 
		}
	};

    /**
    *   @brief Get/Set: Mass
    **/
    virtual int32_t         GetMass() { return mass; };
    virtual void            SetMass(const int32_t mass) { this->mass = mass; };

    /**
    *   @brief Get/Set: Max Health
    **/
    virtual const int32_t   GetMaxHealth() { return maxHealth; };
    virtual void            SetMaxHealth(const int32_t maxHealth) { this->maxHealth = maxHealth; };

    /**
    *   @brief Get/Set: Bounding Box 'Maxs'
    **/
    virtual const vec3_t&   GetMaxs() { 
		if (podEntity) {
			return podEntity->currentState.maxs;
		}
		return ZeroVec3;
	};
    virtual void            SetMaxs(const vec3_t& maxs) {
		if (podEntity) {
			podEntity->currentState.maxs = maxs;
			podEntity->maxs = maxs;
		}
	};
    /**
    *   @brief Get/Set: Message
    **/
    virtual const std::string&  GetMessage() { return EmptyString; };
    virtual void                SetMessage(const std::string& message) {};

    /**
    *   @brief Get/Set: Bounding Box 'Mins'
    **/
    virtual const vec3_t&   GetMins() { 
		if (podEntity) {
			return podEntity->currentState.mins;
		}
		return ZeroVec3;
	};
    virtual void            SetMins(const vec3_t& mins) {
		if (podEntity) {
			podEntity->mins = mins;
			podEntity->currentState.mins = mins;
		}
	};
   
    /**
    *   @brief Get/Set: Model
    **/
    virtual const std::string&  GetModel() { return model; };
    virtual void                SetModel(const std::string &model) {
		// Set the actual entity model.
		if (podEntity) {
			// Set modelstr.
			this->model = model;


			// If it is an inline model, get the size information for it.
			if (model[0] == '*') {
				mmodel_t *inlineModel = clgi.BSP_InlineModel(model.c_str());

				if (inlineModel) {
					podEntity->mins = inlineModel->mins;
					podEntity->maxs = inlineModel->maxs;
				}

				// Link it for collision testing.
				LinkEntity();
			}

			// Update model index.
			SetModelIndex(clgi.R_RegisterModel(model.c_str()));
			//Com_DPrint("DEBUG MODEL: %i for %s\n", clgi.R_RegisterModel(model.c_str()), model.c_str());
			//SetModelIndex(cl->drawModels[ clgi.R_RegisterModel(model.c_str()) ]);
		}
	};

    /**
    *   @brief Get/Set: Model Index 1
    **/
    virtual const int32_t   GetModelIndex() { 
		if (podEntity) {
			return podEntity->currentState.modelIndex;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex(const int32_t index) {
		if (podEntity) {
			podEntity->currentState.modelIndex = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 2
    **/
    virtual const int32_t   GetModelIndex2() { 
		if (podEntity) {
			return podEntity->currentState.modelIndex2;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex2(const int32_t index) {
		if (podEntity) {
			podEntity->currentState.modelIndex2 = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 3
    **/
    virtual const int32_t   GetModelIndex3() { 
		if (podEntity) {
			return podEntity->currentState.modelIndex3;
		} else {
			return 0; 
		}
	};
    virtual void            SetModelIndex3(const int32_t index) {
		if (podEntity) {
			podEntity->currentState.modelIndex3 = index;
		}
	};
    /**
    *   @brief Get/Set: Model Index 4
    **/
    virtual const int32_t   GetModelIndex4() { 
		if (podEntity) {
			return podEntity->currentState.modelIndex4;
		} else {
			return 0; 
		}
	}
    virtual void            SetModelIndex4(const int32_t index) { 
		if (podEntity) {
			podEntity->currentState.modelIndex4 = index;
		}
	};

    /**
    *   @brief Get/Set: Move Type.
    **/
	virtual const int32_t   GetMoveType() { return moveType; };
    virtual void			SetMoveType(const int32_t moveType) { this->moveType = moveType; };

    /**
    *   @brief Get/Set:     NextThink Time.
    **/
    //virtual const float     GetNextThinkTime() { return 0.f; };
    //virtual void            SetNextThinkTime(const float nextThinkTime) {};

    /**
    *   @brief Get/Set:     Noise Index A
    **/
    virtual const int32_t   GetNoiseIndexA() { return noiseIndexA; };
    virtual void            SetNoiseIndexA(const int32_t noiseIndexA) { this->noiseIndexA = noiseIndexA; };

    /**
    *   @brief Get/Set:     Noise Index B
    **/
    virtual const int32_t   GetNoiseIndexB() { return noiseIndexB; };
    virtual void            SetNoiseIndexB(const int32_t noiseIndexB) { this->noiseIndexB = noiseIndexB; };

    /**
    *   @brief Get/Set:     State Number
    **/
    virtual const int32_t   GetNumber() { 
		if (podEntity) {
			return podEntity->clientEntityNumber;
		} else {
			return 0;
		}
	};
    virtual void            SetNumber(const int32_t number) {
		if (podEntity) {
			podEntity->clientEntityNumber = number;
			podEntity->currentState.number = number;
		}	
	};

    /**
    *   @brief Get/Set:     Old Enemy Entity
    **/
    virtual GameEntity*		GetOldEnemy() { return oldEnemyEntity; }
    virtual void            SetOldEnemy(IClientGameEntity* oldEnemy) { this->oldEnemyEntity = oldEnemy; };

    /**
    *   @brief Get/Set:     Old Origin
    **/
    virtual const vec3_t&   GetOldOrigin() { 
		if (podEntity) {
			return podEntity->currentState.oldOrigin;
		} else {
			return ZeroVec3;
		}
	};
    virtual void            SetOldOrigin(const vec3_t& oldOrigin) {
		if (podEntity) {
			podEntity->currentState.oldOrigin = oldOrigin;
		}
	};

    /**
    *   @brief Get/Set:     Origin
    **/
    virtual const vec3_t&   GetOrigin() { 
		if (podEntity) {
			return podEntity->currentState.origin;
		} else {
			return ZeroVec3;
		}
	};
    virtual void            SetOrigin(const vec3_t& origin) {
		if (podEntity) {
			podEntity->currentState.origin = origin;
		}
	};

    /**
    *   @brief Get/Set:     Owner Entity
    **/
    virtual GameEntity*		GetOwner() { return ownerEntity; };
    virtual void            SetOwner(IClientGameEntity* owner) { ownerEntity = owner; };

    /**
    *   @brief Get/Set:     Render Effects
    **/
    virtual const int32_t   GetRenderEffects() { 
		if (podEntity) {
			return podEntity->currentState.renderEffects;
		} else {
			return 0;
		}
	};
    virtual void            SetRenderEffects(const int32_t renderEffects) {
		if (podEntity) {
			podEntity->currentState.renderEffects = renderEffects;
		}
	};
        
    // Get the 'pathTarget' entity value.
    // Overridden by PathCorner
    // TODO: replace this ugly workaround with some component system
    virtual const char*     GetPathTarget() { return EmptyString.c_str(); };

    /**
    *   @brief Get/Set:     Server Flags
    **/
    virtual const int32_t   GetServerFlags() { return 0; };
    virtual void            SetServerFlags(const int32_t serverFlags) {};

    /**
    *   @brief Get/Set:     Skin Number
    **/
    virtual const int32_t   GetSkinNumber() { 
		if (podEntity) {
			return podEntity->currentState.skinNumber;
		} else {
			return 0;
		}
	};
    virtual void            SetSkinNumber(const int32_t skinNumber) {
		if (podEntity) {
			podEntity->currentState.skinNumber = skinNumber;
		}
	};

    /**
    *   @brief Get/Set:     Entity Size
    **/
    virtual const vec3_t&   GetSize() { 
		if (podEntity) {
			return podEntity->size;
		}
		else {
			return ZeroVec3;
		};
	};
    virtual void            SetSize(const vec3_t& size) {
		if (podEntity) {
			podEntity->size = size;
		}
	};

    /**
    *   @brief Get/Set:     Solid
    **/
    virtual const uint32_t  GetSolid() { 
		if (podEntity) {
			return podEntity->currentState.solid;
		} else {
			return 0;
		}
	};
    virtual void            SetSolid(const uint32_t solid) {
		if (podEntity) {
			podEntity->currentState.solid = solid;
		}
	};

    /**
    *   @brief Get/Set:     Sound.
    **/
    virtual const int32_t   GetSound() { 
		if (podEntity) {
			return podEntity->currentState.sound;
		} else {
			return 0;
		}
	};
    virtual void            SetSound(const int32_t sound) {
		if (podEntity) {
			podEntity->currentState.sound = sound;
		}
	};

    /**
    *   @brief Get/Set:     Spawn Flags
    **/
    virtual const int32_t   GetSpawnFlags() { return spawnFlags; };
    virtual void            SetSpawnFlags(const int32_t spawnFlags) { this->spawnFlags = spawnFlags; };

    /**
    *   @brief Get/Set:     Entity State
    **/
    virtual const EntityState&   GetState() { return podEntity->currentState; };
    virtual void                 SetState(const EntityState &state) { podEntity->currentState = state; };

    /**
    *   @brief Get/Set:     Style
    **/
    virtual const int32_t   GetStyle() { return style; };
    virtual void            SetStyle(const int32_t style) { this->style = style; };

    /**
    *   @brief Get/Set:     Take Damage
    **/
    virtual const int32_t   GetTakeDamage() { return takeDamage; };
    virtual void            SetTakeDamage(const int32_t takeDamage) { this->takeDamage = takeDamage; };
    
    /**
    *   @brief Get/Set:     Target
    **/
    virtual const std::string&   GetTarget() { return targetStr; };
    virtual void                 SetTarget(const std::string& target) { this->targetStr = target; };

    /**
    *   @brief Get/Set:     Target Name
    **/
    virtual const std::string&   GetTargetName() { return targetNameStr; };
    virtual void                 SetTargetName(const std::string& targetName) { this->targetNameStr = targetName; };

    /**
    *   @brief Get/Set:     Team
    **/
    virtual const std::string&   GetTeam() { return teamStr; };
    virtual void                 SetTeam(const std::string &team) { this->teamStr = team;};

    /**
    *   @brief Get/Set:     Team Chain
    **/
    virtual IClientGameEntity*	GetTeamChainEntity() { return this->teamChainEntity; }
    virtual void				SetTeamChainEntity(GameEntity* entity) { this->teamChainEntity = teamChainEntity; };

    /**
    *   @brief Get/Set:     Team Master
    **/
    virtual IClientGameEntity*	GetTeamMasterEntity() { return teamMasterEntity; };
    virtual void				SetTeamMasterEntity(IClientGameEntity* entity) { this->teamMasterEntity = teamMasterEntity; };

    /**
    *   @brief Get/Set:     Velocity
    **/
    virtual const vec3_t&   GetVelocity() { return velocity; };
    virtual void            SetVelocity(const vec3_t &velocity) { this->velocity = velocity; };

    /**
    *   @brief Get/Set:     View Height
    **/
    virtual const int32_t   GetViewHeight() { return viewHeight; };
    virtual void            SetViewHeight(const int32_t height) { this->viewHeight = height; };

    /**
    *   @brief Get/Set:     Wait Time
    **/
    virtual const Frametime&    GetWaitTime() { return waitTime; }
    virtual void                SetWaitTime(const Frametime &waitTime) { this->waitTime = waitTime; };

    /**
    *   @brief Get/Set:     Water Level
    **/
    virtual const int32_t   GetWaterLevel() { return waterLevel; };
    virtual void            SetWaterLevel(const int32_t waterLevel) { this->waterLevel = waterLevel; };

    /**
    *   @brief Get/Set:     Water Type
    **/
    virtual const int32_t   GetWaterType() { return waterType; }
    virtual void            SetWaterType(const int32_t waterType) { this->waterType = waterType; };

    /**
    *   @brief Get/Set:     Yaw Speed
    **/
    virtual const float     GetYawSpeed() { return yawSpeed; };
    virtual void            SetYawSpeed(const float yawSpeed) { this->yawSpeed = yawSpeed; };



private:
	/**
	*
	*
	*	Client Specific.
	*
	*
	**/
    //! Refresh Entity Object.
    r_entity_t refreshEntity = {};

    // Client Class Entities maintain their own states. (Get copied in from updates.)
    EntityState currentState = {};
    EntityState previousState = {};



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
    void DispatchUseCallback(IClientGameEntity* other, IClientGameEntity* activator);
    /**
    *   @brief  Dispatches 'Use' callback.
    *   @param  inflictor:  
    *   @param  attacker:   
    *   @param  damage:     
    *   @param  pointer:    
    **/
    void DispatchDieCallback(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point);
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  other:  
    **/
    void DispatchBlockedCallback(IClientGameEntity* other);
    /**
    *   @brief  Dispatches 'Block' callback.
    *   @param  self:   
    *   @param  other:  
    *   @param  plane:  
    *   @param  surf:   
    **/
    void DispatchTouchCallback(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    /**
    *   @brief  Dispatches 'TakeDamage' callback.
    *   @param  other:
    *   @param  kick:
    *   @param  damage:
    **/
    void DispatchTakeDamageCallback(IClientGameEntity* other, float kick, int32_t damage);


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
	virtual void PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction);

private:
	virtual void ProcessSkeletalAnimationForTime(uint64_t time);


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
    void CLGBaseLocalEntityThinkFree(void);

    /**
    *   @brief  Callback for assigning when "no thinking" behavior is wished for.
    **/
    void CLGBaseLocalEntityThinkNull() { }
};