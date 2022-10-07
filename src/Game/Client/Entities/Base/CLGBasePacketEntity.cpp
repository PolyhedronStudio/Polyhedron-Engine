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
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"


// Base Client Game Functionality.
#include "Game/Client/Debug.h"
#include "Game/Client/TemporaryEntities.h"

// Export classes.
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/View.h"

// Effects.
#include "Game/Client/Effects/ParticleEffects.h"

// Base Entity.
#include "Game/Client/Entities/Base/CLGBasePacketEntity.h"

//! Here for OnEvent handling.
extern qhandle_t cl_sfx_footsteps[4];

//! Used for returning vectors from a const vec3_t & reference.
vec3_t CLGBasePacketEntity::ZeroVec3 = vec3_zero();

//! Used for returning strings from a const std::string & reference.
std::string CLGBasePacketEntity::EmptyString = "";

/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
CLGBasePacketEntity::CLGBasePacketEntity(PODEntity* podEntity) : Base() {//}, podEntity(clEntity) {
    this->podEntity = podEntity;
}



/**
*
*
*   Client Game Entity Interface Functions.
*
*
**/
/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void CLGBasePacketEntity::Precache() {

}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void CLGBasePacketEntity::Spawn() {
	// Setup the standard default NextThink method.
	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &CLGBasePacketEntity::CLGBasePacketEntityThinkStandard );
}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void CLGBasePacketEntity::Respawn() {

}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void CLGBasePacketEntity::PostSpawn() {
	
	// Setup the standard default NextThink method.
	SetNextThinkTime( level.time + FRAMERATE_MS );
	SetThinkCallback( &CLGBasePacketEntity::CLGBasePacketEntityThinkStandard );
}

/**
*   @brief  General entity thinking routine.
**/
void CLGBasePacketEntity::Think() {
	// Safety check.
    if (thinkFunction == nullptr) {
		return;
    }

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

/**
*   @brief  Act upon the parsed key and value.
**/
void CLGBasePacketEntity::SpawnKey(const std::string& key, const std::string& value) {
    // Deal with classname, set it anyway.
	if ( key == "classname" ) {
		SetClassname( value );
	}
	// Stop mapversion from causing warnings.
	else if (key == "mapversion") {
		
	}
	// Angle.
	else if (key == "angle") {
		// Parse angle.
		vec3_t hackedAngles = vec3_zero();
		ParseKeyValue(key, value, hackedAngles.y);

		// Set angle.
		SetAngles( hackedAngles );
	}
	// Angles.
	else if (key == "angles") {
		// Parse angles.
		vec3_t parsedAngles = vec3_zero();
		ParseKeyValue(key, value, parsedAngles);

		// Set origin.
		SetAngles(parsedAngles);
	}
	// Damage(dmg)
	else if (key == "dmg") {
		// Parse damage.
		int32_t parsedDamage = 0;
		ParseKeyValue(key, value, parsedDamage);

		// Set Damage.
		SetDamage(parsedDamage);
	}
	// Delay.
	else if (key == "delay") {
		// Parsed float.
		Frametime parsedTime = Frametime::zero();
		ParseKeyValue(key, value, parsedTime);

		// Assign.
		SetDelayTime(parsedTime);
	}
	// KillTarget.
	else if (key == "killtarget") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetKillTarget(parsedString);
	}
	// Mass.
	else if (key == "mass") {
	    // Parsed string.
	    int32_t parsedInteger = 0;
	    ParseKeyValue(key, value, parsedInteger);

	    // Assign.
	    SetMass(parsedInteger);
	} 
	// Message.
	else if (key == "message") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetMessage(parsedString);
	} 
	// Model.
	else if (key == "model") {
		// Parse model.
		std::string parsedModel = "";
		ParseKeyValue(key, value, parsedModel);

		// Set model.
		SetModel(parsedModel);
	}
	// Origin.
	else if (key == "origin") {
		// Parse origin.
		vec3_t parsedOrigin = vec3_zero();
		ParseKeyValue(key, value, parsedOrigin);

		// Set origin.
		SetOrigin(parsedOrigin);
	// Target.
	} else if (key == "target") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetTarget(parsedString);
	// TargetName.
	} else 	if (key == "targetname") {
		// Parsed string.
		std::string parsedString = "";
		ParseKeyValue(key, value, parsedString);

		// Assign.
		SetTargetName(parsedString);
	
	// Spawnflags.
	} else if (key == "spawnflags") {
		// Parse damage.
		int32_t parsedSpawnFlags = 0;
		ParseKeyValue(key, value, parsedSpawnFlags);

		// Set SpawnFlags.
		SetSpawnFlags(parsedSpawnFlags);
	// Style.
	//}
	//else if (key == "style") {
	//	// Parse damage.
	//	int32_t parsedStyle = 0;
	//	ParseKeyValue( key, value, parsedStyle );

	//	// Set SpawnFlags.
	//	SetStyle( parsedStyle );
	} else {
		CLG_Print(PrintType::DeveloperWarning, fmt::format("Warning: Entity[#{},{}] has unknown Key / Value['{}', '{}']\n", GetNumber(), GetClassname(), key, value) );
	}
}



/***
*
* 
*   Client Game Entity Functions.
*
* 
***/
/**
*	@TODO:	This following process has to happen elsewhere, but we keep it here for
*			prototyping at this moment.
**/
static SkeletalModelData *UpdateSkeletalModelDataFromState(EntitySkeleton *es, const EntityState* state) {
	// Zero out X axis: DEPTH. This maintains the model rendering appropriately
	// at our origin point.

	if ( !es ) {
		return nullptr;
	}
	
	// Treat elsewhere.
	if ( state->modelIndex <= 0 || state->modelIndex == 255 ) {
		return nullptr;
	}

	if (cl->drawModels[state->modelIndex] <= 0) {
		return nullptr;
	}

	// Ok, let's fetch the model we got.
	model_t *entityModel = clgi.CL_Model_GetModelByHandle( cl->drawModels[state->modelIndex] );

	if ( !entityModel ) {
		return nullptr;
	}
	
	SkeletalModelData *skm = entityModel->skeletalModelData;

	if (!skm) {
		return nullptr;
	}

	// If the ES already had a modelptr, and it is identical we're done.
	if (es->modelPtr != nullptr) {
		return es->modelPtr->skeletalModelData;
	}

	// Create our skeleton for this model.
	if ( clgi.ES_CreateFromModel( entityModel, es ) ) {

		// Ensure its set to 0 by default.
		if (!skm->actionMap.empty()) {
		for (auto& anim : skm->actionMap) {
			anim.second.rootBoneAxisFlags = 0;//SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation | SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation; //SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;  //SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation | SkeletalAnimationAction::RootBoneAxisFlags::DefaultTranslationMask;// | SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation;
		}

		// TODO: Remove this after implementing a RootAxisFlags command for 
		const int32_t ZeroAllAxis = ( SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation | SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation | SkeletalAnimationAction::RootBoneAxisFlags::ZeroZTranslation );

		skm->actionMap["Idle"].rootBoneAxisFlags	=
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation | 
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkForward"].rootBoneAxisFlags		= SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;
		skm->actionMap["WalkForwardLeft"].rootBoneAxisFlags	= 
		skm->actionMap["WalkForwardRight"].rootBoneAxisFlags =
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation |
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkLeft"].rootBoneAxisFlags = SkeletalAnimationAction::SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkRight"].rootBoneAxisFlags = SkeletalAnimationAction::SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["WalkingToDying"].rootBoneAxisFlags	= SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation | 
			SkeletalAnimationAction::RootBoneAxisFlags::ZeroYTranslation;
		skm->actionMap["RunForward"].rootBoneAxisFlags		= SkeletalAnimationAction::RootBoneAxisFlags::ZeroXTranslation;
		}
	}
	 return skm;
}
/**
*   @brief  Updates the entity with the data of the newly passed EntityState object.
**/
void CLGBasePacketEntity::UpdateFromState(const EntityState* state) {
	assert(state);
	//SetOrigin(state->origin);
	//SetAngles(state->angles);
	//SetOldOrigin(state->oldOrigin);
	//SetModelIndex(state->modelIndex);
	//SetModelIndex2(state->modelIndex2);
	//SetModelIndex3(state->modelIndex3);
	//SetModelIndex4(state->modelIndex4);
	//SetSkinNumber(state->skinNumber);
	//SetEffects(state->effects);
	//SetRenderEffects(state->renderEffects);
	//SetSolid(state->solid);
	//SetMins(state->mins);
	//SetMaxs(state->maxs);
	//SetSound(state->sound);
	//SetEventID(state->eventID);

	// This should go elsewhere, but alas prototyping atm.
//	if (GetModelIndex() != state->modelIndex) {
	
//	}
	//SetInUse( true );
	
	// Setup same think for the next frame.
	//SetNextThinkTime(level.time + FRAMETIME_S);
	//SetThinkCallback(&CLGBasePacketEntity::CLGBasePacketEntityThinkStandard);
}

/**
*	@brief	Gives the GameEntity a chance to Spawn itself appropriately based on state updates.
**/
void CLGBasePacketEntity::SpawnFromState(const EntityState* state) {
	if (!state) {
		return;
	}
	//SetOrigin(state->origin);
	//SetAngles(state->angles);
	//SetOldOrigin(state->oldOrigin);
	//SetModelIndex(state->modelIndex);
	//SetModelIndex2(state->modelIndex2);
	//SetModelIndex3(state->modelIndex3);
	//SetModelIndex4(state->modelIndex4);
	//SetSkinNumber(state->skinNumber);
	//SetEffects(state->effects);
	//SetRenderEffects(state->renderEffects);
	//SetSolid(state->solid);
	//SetMins(state->mins);
	//SetMaxs(state->maxs);
	//SetSound(state->sound);
	//SetEventID(state->eventID);

	if (!skm) {
		skm = UpdateSkeletalModelDataFromState(&entitySkeleton, state);
	}

	//SetInUse( true );

	// Setup same think for the next frame.
	//SetNextThinkTime(level.time + FRAMETIME_S);
	//SetThinkCallback(&CLGBasePacketEntity::CLGBasePacketEntityThinkStandard);
}

/**
*   @returen True if the entity is still in the current frame.
**/
//const qboolean CLGBasePacketEntity::IsInUse() {
//    if (podEntity) {
//        return cl->frame.number == podEntity->serverFrame;
//    } else {
//        false;
//    }
//}

/**
*   @brief  Stub.
**/
const std::string CLGBasePacketEntity::GetClassname() {
    // Returns this classname, the base entity.
    return GetTypeInfo()->classname;
}

/**
*   @return An uint32_t containing the hashed classname string.
**/
uint32_t CLGBasePacketEntity::GetHashedClassname() {
    return GetTypeInfo()->hashedMapClass;
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
void CLGBasePacketEntity::OnDeallocate() {

}

/**
*	@brief	Gets called in order to process the newly received EventID. (It also gets called when EventID == 0.)
**/
void CLGBasePacketEntity::OnEventID(uint32_t eventID) {
    // EF_TELEPORTER acts like an event, but is not cleared each frame
    if ((GetEffects()  & EntityEffectType::Teleporter)) {
        ParticleEffects::Teleporter(GetOrigin());
    }

    // Switch to specific execution based on a unique Event ID.
    switch (eventID) {
        case EntityEvent::ItemRespawn:
            clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Weapon, clgi.S_RegisterSound("items/respawn1.wav"), 1, Attenuation::Idle, 0);
            ParticleEffects::ItemRespawn(GetOrigin());
            break;
        case EntityEvent::PlayerTeleport:
            clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Weapon, clgi.S_RegisterSound("misc/tele1.wav"), 1, Attenuation::Idle, 0);
            ParticleEffects::Teleporter(GetOrigin());
            break;
        case EntityEvent::Footstep:
            if (cl_footsteps->integer)
                clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Body, cl_sfx_footsteps[rand() & 3], 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::FallShort:
            clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Auto, clgi.S_RegisterSound("player/land1.wav"), 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::Fall:
            clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Auto, clgi.S_RegisterSound("player/fall2.wav"), 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::FallFar:
            clgi.S_StartSound(NULL, GetNumber(), SoundChannel::Auto, clgi.S_RegisterSound("player/fall1.wav"), 1, Attenuation::Normal, 0);
            break;
    }
}

/***
*
* 
*   Dispatch Callback Functions.
*
* 
***/
/**
*   @brief  Dispatches 'Use' callback.
*   @param  other:      
*   @param  activator:  
**/
void CLGBasePacketEntity::DispatchUseCallback(GameEntity* other, GameEntity* activator) {
	// Safety check.
	if (useFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*useFunction)(other, activator);
}

/**
*   @brief  Dispatches 'Die' callback.
*   @param  inflictor:  
*   @param  attacker:   
*   @param  damage:     
*   @param  pointer:    
**/
void CLGBasePacketEntity::DispatchDieCallback(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point) {
	// Safety check.
	if (dieFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*dieFunction)(inflictor, attacker, damage, point);
}


/**
*   @brief  Dispatches 'Blocked' callback.
*   @param  other:  
**/
void CLGBasePacketEntity::DispatchBlockedCallback(GameEntity* other) {
	// Safety check.
	if (blockedFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*blockedFunction)(other);
}

/**
*   @brief  Dispatches 'Touch' callback.
*   @param  self:   
*   @param  other:  
*   @param  plane:  
*   @param  surf:   
**/
void CLGBasePacketEntity::DispatchTouchCallback(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// Safety check.
	if (touchFunction == nullptr)
		return;

	// Execute 'Touch' callback function.
	(this->*touchFunction)(self, other, plane, surf);
}

/**
*   @brief  Dispatches 'TakeDamage' callback.
*   @param  other:
*   @param  kick:
*   @param  damage:
**/
void CLGBasePacketEntity::DispatchTakeDamageCallback(GameEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
}

/**
*   @brief  Dispatches 'Stop' callback.
**/
void CLGBasePacketEntity::DispatchStopCallback() {
	// Safety check.
	if (stopFunction == nullptr)
		return;

	// Execute 'Stop' callback function.
	(this->*stopFunction)();
}



/**
* 
(
*   Entity Utility callbacks that can be set as a nextThink function.
* 
*
**/
/**
*   @brief  Marks the entity to be removed in the next client frame. This is preferred to CLG_FreeEntity, 
*           as it is safer. Prevents any handles or pointers that lead to this entity from turning invalid
*           on us during the current client game frame we're processing.
**/
void CLGBasePacketEntity::Remove()
{
	podEntity->clientFlags |= EntityClientFlags::Remove;
}

/**
*   @brief  Callback method to use for freeing this entity. It calls upon Remove()
**/
void CLGBasePacketEntity::CLGBasePacketEntityThinkFree(void) {
	//CLG_FreeEntity(serverEntity);
	Remove();
}

/**
*	@brief	Used by default in order to process entity state data such as animations.
**/
void CLGBasePacketEntity::CLGBasePacketEntityThinkStandard(void) {
	if ( IsExtrapolating() || podEntity->linearMovement.isMoving ) {
		SetNextThinkTime( level.extrapolatedTime );
	} else {
		// Setup same think for the next frame.
		SetNextThinkTime( level.time + FRAMERATE_MS );
	}
	SetThinkCallback(&CLGBasePacketEntity::CLGBasePacketEntityThinkStandard);
}



/***
*
*
*	Skeletal Animation.
*
*
***/
/**
*	@brief	Switches the animation by blending from the current animation into the next.
*	@return	True if succesfull, false otherwise.
**/
const bool CLGBasePacketEntity::SwitchAnimation(const int32_t animationIndex, const GameTime &startTime = GameTime::zero()) {
	//Com_DPrint("SwitchAnimation CALLED !! Index(#%i) startTime(#%i)\n", animationIndex, startTime.count());
	if (!skm) {
		CLG_Print( PrintType::Developer, "SwitchAnimation: No SKM Data present.\n");
		return false;
	}

	if (animationIndex < 0 || animationIndex > skm->animations.size()) {
		CLG_Print( PrintType::Developer, fmt::format("SwitchAnimation: Failed, invalid index(#{}), total animation list size({})\n", animationIndex, skm->animations.size() ) );
		return false;
	}

	// Can't switch to animation without SKM data.
	if (!skm) {
		CLG_Print( PrintType::Developer, "SwitchAnimation: No SKM data present.\n");
		return false;
	}

	if (animationIndex > skm->animations.size()) {
		CLG_Print( PrintType::Developer, fmt::format( "SwitchAnimation: animationIndex({}) out of range for animations.size({})\n",
			animationIndex,
			skm->actions.size()
		) );
		return false;
	}

	// Get state pointer.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get animation state.
	EntityAnimationState *currentAnimationState	= &currentState->currentAnimation;
	EntityAnimationState *previousAnimationState = &previousState->currentAnimation;
	//EntityAnimationState *previousAnimationState = &currentState->previousAnimation;

	//// Has the animation index changed? If so, lookup the new animation.
	//// TODO: Move to a separate function.
	//const int32_t currentAnimationIndex = currentAnimationState->animationIndex;
	const int32_t previousAnimationIndex = previousAnimationState->animationIndex;
	// Use refresh instead. It's the most actual state anyhow.
	//const int32_t previousAnimationIndex = refreshAnimation.animationIndex;

	// We change animations for our 'main timeline animation' if:
	//	- animationIndex differs from our previous animation index.
	//	AND
	//	- the start time of the new animation to switch to differs from the current animation start time.
	if ( animationIndex != previousAnimationState->animationIndex || previousAnimationState->startTime != currentAnimationState->startTime ) {
		// Get the animation.
		SkeletalAnimation *skmAnimation = GetAnimation( animationIndex );

		if ( !skmAnimation) {
			// TODO: Warn?
			return false;
		}
		SkeletalAnimationBlendAction *skmBlendAction = GetBlendAction( skmAnimation, 0 );

		// Retreive animation data matching to animationIndex.
		SkeletalAnimationAction *skmAction= GetAction( skmBlendAction->actionIndex );

		// Update our refresh animation to new values.
		refreshAnimation.animationIndex	= animationIndex;
		refreshAnimation.frame			= currentAnimationState->frame;
		refreshAnimation.startFrame		= skmAction->startFrame;
		refreshAnimation.endFrame		= skmAction->endFrame;
		refreshAnimation.forceLoop		= skmAction->forceLoop;
		refreshAnimation.frameTime		= skmAction->frametime;
		// Since the serverTime we got on the client is an estimation and starts ticking the second the client joins,
		// we have to set the startTime to whichever our level.time was at the moment this animation started switching.
		refreshAnimation.startTime		= level.time.count();
		refreshAnimation.loopCount		= skmAction->loopingFrames;

		//// Get pointer to dominating blend action.
		//SkeletalAnimationBlendAction *dominatingBlendAction = GetBlendAction( animation, 0 );
		//// Get the dominating blend action state.
		//EntitySkeletonBlendActionState *dominatingBlendActionState = GetBlendActionState( currentAnimation->animationIndex, 0 );

		//// Ensure that it is in bounds. (Should be.)
		//if ( !dominatingBlendAction || !dominatingBlendActionState ) {
		//	refreshEntity.currentBonePoses = nullptr;
		//	return;
		//}

		//// Get the dominating action.
		//SkeletalAnimationAction *dominatingAction = GetAction( dominatingBlendAction->actionIndex );
		//// Store old frame.
		//dominatingBlendActionState->oldFrame = dominatingBlendActionState->currentFrame;
		
		// Update animation states for this frame's current entity state.
		*previousAnimationState = *currentAnimationState;
	}


	return true;
}

/**
*	@brief	This class implements a basic templated functionality that will assign
*			modelindex #2,#3 and #4 to their designated bone if this was set in an animation.
*
*			Inheirted classes can override this, call the Base class method and/or implement
*			their own functionalities here. Think of: Getting/setting a bone transform, 
*			and/or rendering effects/entities at a specific bone's transform.
**/
static void Matrix34Invert(const float* inMat, float* outMat) {
	outMat[0] = inMat[0]; outMat[1] = inMat[4]; outMat[2] = inMat[8];
	outMat[4] = inMat[1]; outMat[5] = inMat[5]; outMat[6] = inMat[9];
	outMat[8] = inMat[2]; outMat[9] = inMat[6]; outMat[10] = inMat[10];

	float invSqrLen, * v;
	v = outMat + 0; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 4; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 8; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);

	vec3_t trans;
	trans[0] = inMat[3];
	trans[1] = inMat[7];
	trans[2] = inMat[11];

	outMat[3] = -DotProduct(outMat + 0, trans);
	outMat[7] = -DotProduct(outMat + 4, trans);
	outMat[11] = -DotProduct(outMat + 8, trans);
}
static inline void
mult_matrix_vector(vec4_t &v, const float *a, const vec4_t &b)
{
	int j;
	for (j = 0; j < 4; j++) {
		v[j] =
			a[0 * 4 + j] * b[0] +
			a[1 * 4 + j] * b[1] +
			a[2 * 4 + j] * b[2] +
			a[3 * 4 + j] * b[3];
	}
}

static inline vec3_t transform_point(const float* p, const float* matrix)
{
	vec4_t point = { p[0], p[1], p[2], 1.f };
	vec4_t transformed = vec4_zero();
	mult_matrix_vector(transformed, matrix, point);
	return vec3_t {transformed.x, transformed.y, transformed.z}; // vec4 -> vec3
}
static void
create_entity_matrix(mat4_t &matrix, r_entity_t* e, qboolean enable_left_hand)
{
	vec3_t axis[3];
	vec3_t origin;
	origin[0] = (1.f-e->backlerp) * e->origin[0] + e->backlerp * e->oldorigin[0];
	origin[1] = (1.f-e->backlerp) * e->origin[1] + e->backlerp * e->oldorigin[1];
	origin[2] = (1.f-e->backlerp) * e->origin[2] + e->backlerp * e->oldorigin[2];

	AnglesToAxis(e->angles, axis);

	float scale = (e->scale > 0.f) ? e->scale : 1.f;

	vec3_t scales = { scale, scale, scale };
	if ((e->flags & RF_LEFTHAND) && enable_left_hand)
	{
		scales[1] *= -1.f;
	}

	matrix[0]  = axis[0][0] * scales[0];
	matrix[4]  = axis[1][0] * scales[1];
	matrix[8]  = axis[2][0] * scales[2];
	matrix[12] = origin[0];

	matrix[1]  = axis[0][1] * scales[0];
	matrix[5]  = axis[1][1] * scales[1];
	matrix[9]  = axis[2][1] * scales[2];
	matrix[13] = origin[1];

	matrix[2]  = axis[0][2] * scales[0];
	matrix[6]  = axis[1][2] * scales[1];
	matrix[10] = axis[2][2] * scales[2];
	matrix[14] = origin[2];

	matrix[3]  = 0.0f;
	matrix[7]  = 0.0f;
	matrix[11] = 0.0f;
	matrix[15] = 1.0f;
}

static mat4_t
mult_matrix_matrix(const mat4_t &a, const mat4_t &b)
{
	mat4_t p = mat4_identity();

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			p[i * 4 + j] =
				a[0 * 4 + j] * b[i * 4 + 0] +
				a[1 * 4 + j] * b[i * 4 + 1] +
				a[2 * 4 + j] * b[i * 4 + 2] +
				a[3 * 4 + j] * b[i * 4 + 3];
		}
	}

	return p;
}

static void
inversee(const mat4_t &m, mat4_t &inv)
{
	inv[0] = m[5]  * m[10] * m[15] -
	         m[5]  * m[11] * m[14] -
	         m[9]  * m[6]  * m[15] +
	         m[9]  * m[7]  * m[14] +
	         m[13] * m[6]  * m[11] -
	         m[13] * m[7]  * m[10];
	
	inv[1] = -m[1]  * m[10] * m[15] +
	          m[1]  * m[11] * m[14] +
	          m[9]  * m[2] * m[15] -
	          m[9]  * m[3] * m[14] -
	          m[13] * m[2] * m[11] +
	          m[13] * m[3] * m[10];
	
	inv[2] = m[1]  * m[6] * m[15] -
	         m[1]  * m[7] * m[14] -
	         m[5]  * m[2] * m[15] +
	         m[5]  * m[3] * m[14] +
	         m[13] * m[2] * m[7] -
	         m[13] * m[3] * m[6];
	
	inv[3] = -m[1] * m[6] * m[11] +
	          m[1] * m[7] * m[10] +
	          m[5] * m[2] * m[11] -
	          m[5] * m[3] * m[10] -
	          m[9] * m[2] * m[7] +
	          m[9] * m[3] * m[6];
	
	inv[4] = -m[4]  * m[10] * m[15] +
	          m[4]  * m[11] * m[14] +
	          m[8]  * m[6]  * m[15] -
	          m[8]  * m[7]  * m[14] -
	          m[12] * m[6]  * m[11] +
	          m[12] * m[7]  * m[10];
	
	inv[5] = m[0]  * m[10] * m[15] -
	         m[0]  * m[11] * m[14] -
	         m[8]  * m[2] * m[15] +
	         m[8]  * m[3] * m[14] +
	         m[12] * m[2] * m[11] -
	         m[12] * m[3] * m[10];
	
	inv[6] = -m[0]  * m[6] * m[15] +
	          m[0]  * m[7] * m[14] +
	          m[4]  * m[2] * m[15] -
	          m[4]  * m[3] * m[14] -
	          m[12] * m[2] * m[7] +
	          m[12] * m[3] * m[6];
	
	inv[7] = m[0] * m[6] * m[11] -
	         m[0] * m[7] * m[10] -
	         m[4] * m[2] * m[11] +
	         m[4] * m[3] * m[10] +
	         m[8] * m[2] * m[7] -
	         m[8] * m[3] * m[6];
	
	inv[8] = m[4]  * m[9] * m[15] -
	         m[4]  * m[11] * m[13] -
	         m[8]  * m[5] * m[15] +
	         m[8]  * m[7] * m[13] +
	         m[12] * m[5] * m[11] -
	         m[12] * m[7] * m[9];
	
	inv[9] = -m[0]  * m[9] * m[15] +
	          m[0]  * m[11] * m[13] +
	          m[8]  * m[1] * m[15] -
	          m[8]  * m[3] * m[13] -
	          m[12] * m[1] * m[11] +
	          m[12] * m[3] * m[9];
	
	inv[10] = m[0]  * m[5] * m[15] -
	          m[0]  * m[7] * m[13] -
	          m[4]  * m[1] * m[15] +
	          m[4]  * m[3] * m[13] +
	          m[12] * m[1] * m[7] -
	          m[12] * m[3] * m[5];
	
	inv[11] = -m[0] * m[5] * m[11] +
	           m[0] * m[7] * m[9] +
	           m[4] * m[1] * m[11] -
	           m[4] * m[3] * m[9] -
	           m[8] * m[1] * m[7] +
	           m[8] * m[3] * m[5];
	
	inv[12] = -m[4]  * m[9] * m[14] +
	           m[4]  * m[10] * m[13] +
	           m[8]  * m[5] * m[14] -
	           m[8]  * m[6] * m[13] -
	           m[12] * m[5] * m[10] +
	           m[12] * m[6] * m[9];
	
	inv[13] = m[0]  * m[9] * m[14] -
	          m[0]  * m[10] * m[13] -
	          m[8]  * m[1] * m[14] +
	          m[8]  * m[2] * m[13] +
	          m[12] * m[1] * m[10] -
	          m[12] * m[2] * m[9];
	
	inv[14] = -m[0]  * m[5] * m[14] +
	           m[0]  * m[6] * m[13] +
	           m[4]  * m[1] * m[14] -
	           m[4]  * m[2] * m[13] -
	           m[12] * m[1] * m[6] +
	           m[12] * m[2] * m[5];
	
	inv[15] = m[0] * m[5] * m[10] -
	          m[0] * m[6] * m[9] -
	          m[4] * m[1] * m[10] +
	          m[4] * m[2] * m[9] +
	          m[8] * m[1] * m[6] -
	          m[8] * m[2] * m[5];

	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	det = 1.0f / det;

	for(int i = 0; i < 16; i++)
		inv[i] = inv[i] * det;
}

// Temporary.
static vec3_t prevAngles[2] = { vec3_zero(), vec3_zero() };
// 0 = Entity ID 13
// 1 = Entity ID 23
void CLGBasePacketEntity::PostComputeSkeletonTransforms(EntitySkeletonBonePose *bonePoses) {
	// Static buffer for stashing the local post transforms.
	static float localPoseMatrices[SKM_MAX_JOINTS * 12];

	// Get the current animation.
	// Get state references.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get Animation State references.
	EntityAnimationState *currentAnimation	= &currentState->currentAnimation;
	EntityAnimationState *previousAnimation	= &previousState->currentAnimation;

	// Model Index 3.
	//if ( currentState->modelIndex3 != 0 && entitySkeleton.modelPtr && entitySkeleton.bones.size() > 0) {
	//	if (entitySkeleton.boneMap.contains("mixamorig8:RightHand")) {
	//		// Seek right hand bone.
	//		EntitySkeletonBoneNode *rightHandeNode = entitySkeleton.boneMap["mixamorig8:RightHand"];

	//		// Get the number.
	//		if (!rightHandeNode) {
	//			return;
	//		}
	//		const int32_t poseNumber = rightHandeNode->GetEntitySkeletonBone()->index;

	//		// Get the transform.
	//		EntitySkeletonBonePose *bonePose = &bonePoses[poseNumber];

	//		// Now let's get funky, create a copy of what we had so far as to keep
	//		// and make sure ith as the same properties.
	//		r_entity_t refreshAttachmentEntity = refreshEntity;

	//		// Set the modelindex3 model for this refresh entity.
	//		refreshAttachmentEntity.model = cl->drawModels[ currentState->modelIndex3 ];

	//		// Compose the world and local pose matrices.
	//		// TODO: We oughta only do this once.
	//		clgi.ES_ComputeWorldPoseTransforms( entitySkeleton.modelPtr, bonePoses, &localPoseMatrices[0]);
	//	
	//		// Generate entity matrix to transform the actual pose with.
	//		mat4_t matEntity;
	//		refreshAttachmentEntity.origin = GetOrigin();
	//		refreshAttachmentEntity.angles = GetAngles();
	//		create_entity_matrix(matEntity, &refreshAttachmentEntity, false);

	//		/**
	//		*	Convert bone matrix to 4x4 matrix, rotate to entity angles, and get the 
	//		*	translate & rotate of our bone.
	//		**/
	//		// Get our bone's 3x4 matrix.
	//		float *matRefreshBonePose = &localPoseMatrices[poseNumber * 12];

	//		// Convert to 4x4 matrix.
	//		mat4_t matBonePose = mat4_from_mat3x4( matRefreshBonePose );
	//		mat4_t matRotatedPoseBone = mult_matrix_matrix( matEntity, matBonePose );

	//		vec3_t translate = {
	//			matRotatedPoseBone[12], matRotatedPoseBone[13], matRotatedPoseBone[14]
	//		};


	//		vec3_t rotate = {
	//			matRotatedPoseBone[4], matRotatedPoseBone[5], matRotatedPoseBone[6]
	//		};

	//		/**
	//		*	Update our origin and slerp the attachment angles.
	//		**/
	//		// Set the bone attachment's origin to the bone's translate coordinates.
	//		refreshAttachmentEntity.origin = translate;
	//		// Same for old origin.
	//		refreshAttachmentEntity.oldorigin = refreshAttachmentEntity.origin;
	//	
	//		// Get old angles, 
	//		const vec3_t oldBoneAngles = (GetNumber() == 13 ? prevAngles[0] : prevAngles[1]);
	//		// Normalize rotation and get euler coordinates.
	//		const vec3_t boneAngles = vec3_euler( rotate );
	//		// Lerp.
	//		vec3_t finalBoneAngles = vec3_mix_euler( oldBoneAngles, boneAngles, 1.0 - refreshAnimation.backLerp);
	//		// Store old angles.
	//		if (GetNumber() == 13) { prevAngles[0] = finalBoneAngles; } else { prevAngles[1] = finalBoneAngles; }

	//		// Set angles.
	//		refreshAttachmentEntity.angles = finalBoneAngles;

	//		// Add to our view.
	//		clge->view->AddRenderEntity(refreshAttachmentEntity);
	//	}
	//}

	// Model Index 4.
	if (currentState->modelIndex4) {

	}
}

/**
*	@brief	Checks to see if we've received new animation data, and if the time
*			is there to switch to the next animation before processing all the
*			current frame numbers for the current time in animation.
*
*			Processing happens either without the entity skeleton, in which case
*			it is your typical alias model incrementing a linear list of frames
*			each for frameTime ms. 
*
*			If the model has had a .skc file loaded for itself and it succesfully
*			managed to create an Entity Skeleton for this entity then it'll calculate
*			the current frame of each separate blend action in order for later
*			lerp and blend computations.
**/
void CLGBasePacketEntity::ProcessSkeletalAnimationForTime(const GameTime &time) {
	// Get state references.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get Animation State references.
	EntityAnimationState *currentAnimation	= &currentState->currentAnimation;
	EntityAnimationState *previousAnimation	= &previousState->currentAnimation;

	// Ensure that the animation is actually allowed to start.
	if (time <= GameTime::zero()) {
		return;
	}

	// Did any animation state data change?
	if (currentAnimation->startTime != previousAnimation->startTime || currentAnimation->animationIndex != previousAnimation->animationIndex) {
		SwitchAnimation(currentAnimation->animationIndex, GameTime(currentAnimation->startTime));
	}

	// See if we got skeletal model data.
	const model_t *esModel = entitySkeleton.modelPtr;

	// Without an initialized entity skeleton, we just process the refreshanimation as is, covering the entire model.
	if ( !esModel || !esModel->skeletalModelData ) {
		// Store old frame.
		refreshEntity.oldframe	= refreshAnimation.frame;

		// Process the animation, like we would do any time.
		refreshAnimation.backLerp = 1.0 - SG_FrameForTime(&refreshAnimation.frame,
			time,
			GameTime(refreshAnimation.startTime),
			refreshAnimation.frameTime,
			refreshAnimation.startFrame,
			refreshAnimation.endFrame,
			refreshAnimation.loopCount,
			refreshAnimation.forceLoop
		);

		// Store 
		refreshEntity.frame		= refreshAnimation.frame;
		refreshEntity.backlerp	= refreshAnimation.backLerp;

	// If we've got model data for our skeleton, we take the path of calculating 'frame for time' of 
	// each blend action in our current animation.
	} else {
		// Validate animation, and action indices.
		// Validate animation, and action indices.
		SkeletalModelData *skm = esModel->skeletalModelData;

		// Get our animation data.
		SkeletalAnimation *animation = GetAnimation( currentAnimation->animationIndex );
		// We NEED an animation to work with...
		if ( !animation ) {
			refreshEntity.currentBonePoses = nullptr;
			return;
		}

		// Get pointer to dominating blend action.
		SkeletalAnimationBlendAction *dominatingBlendAction = GetBlendAction( animation, 0 );
		// Get the dominating blend action state.
		EntitySkeletonBlendActionState *dominatingBlendActionState = GetBlendActionState( currentAnimation->animationIndex, 0 );

		// Ensure that it is in bounds. (Should be.)
		if ( !dominatingBlendAction || !dominatingBlendActionState ) {
			refreshEntity.currentBonePoses = nullptr;
			return;
		}

		// Get the dominating action.
		SkeletalAnimationAction *dominatingAction = GetAction( dominatingBlendAction->actionIndex );
		// Store old frame.
		dominatingBlendActionState->oldFrame = dominatingBlendActionState->currentFrame;

		// Dominating blend action operates directly on our refresh animation.
		dominatingBlendActionState->backLerp = 1.0 - SG_FrameForTime(&dominatingBlendActionState->currentFrame,
			time,
			GameTime(refreshAnimation.startTime),
			dominatingAction->frametime,
			dominatingAction->startFrame,
			dominatingAction->endFrame,
			dominatingAction->loopingFrames,
			dominatingAction->forceLoop
		);

		// While at it, make sure to apply the root bone axis flags as well.
		refreshEntity.rootBoneAxisFlags = dominatingAction->rootBoneAxisFlags;

		// Store animation data.
		refreshEntity.oldframe	= dominatingBlendActionState->oldFrame;
		refreshEntity.frame		= dominatingBlendActionState->currentFrame;
		refreshEntity.backlerp	= dominatingBlendActionState->backLerp;

		// Go over all other blend actions, and acquire their data also.
		for ( int32_t blendActionIndex = 1; blendActionIndex < animation->blendActions.size(); blendActionIndex++ ) {
			// Get pointer to the 'sub dominating' blend action.
			SkeletalAnimationBlendAction *subdominatingBlendAction = GetBlendAction( animation, blendActionIndex );

			// Only proceed if we got one.
			if ( subdominatingBlendAction ) {
				// Now get its actual action pointer information we seek.
				SkeletalAnimationAction *subdominatingAction = GetAction( subdominatingBlendAction->actionIndex );

				// One final bounds test:
				if ( subdominatingAction ) {
					// We've got the action data, time to process its frame for time and store it into
					// our distinct entity skeleton.
					//EntitySkeletonBlendActionState *baState = GetBlendActionState( currentAnimation->animationIndex, blendActionIndex );
					// We've got the action data, time to process its frame for time and store it into
					// our distinct entity skeleton.
					EntitySkeletonBlendActionState *baState = &entitySkeleton.blendActionAnimationStates[ currentAnimation->animationIndex ][ blendActionIndex ];
					if ( !baState ) {
						// TODO: Debug warning here.
						refreshEntity.currentBonePoses = nullptr;
						return;
					}

					// Be sure to store its old frame.
					baState->oldFrame = baState->currentFrame;

					// Now process the actual frame for time of this blend action state.
					baState->backLerp = 1.0 - SG_FrameForTime(&baState->currentFrame,
						time,
						GameTime(refreshAnimation.startTime),
						subdominatingAction->frametime,
						subdominatingAction->startFrame,
						subdominatingAction->endFrame,
						subdominatingAction->loopingFrames,
						subdominatingAction->forceLoop
					);
				} else {
					// TODO: Debug warning here.
					refreshEntity.currentBonePoses = nullptr;
					return;
				}
			} else {
				// TODO: Debug warning here.
				refreshEntity.currentBonePoses = nullptr;
				return;
			}
		}
	}
}

/**
*	@brief	Checks to see if we've received new animation data, and if the time
*			is there to switch to the next animation before processing all the
*			current frame numbers for the current time in animation.
*
*			Processing happens either without the entity skeleton, in which case
*			it acts as a typical alias model so we just calculate the lerped bonePose
*			for the current frame.
*
*			If the model has had a .skc file loaded for itself and it succesfully
*			managed to create an Entity Skeleton for this entity, it'll start to lerp
*			each action's bone poses. After which it traverses each blend action to
*			recursively blend from the specified bone at the given fraction. Ultimnately
*			giving us our final relative bone poses.
*
*			Local and World poses can be calculated if desired. TODO: Make it optionable
*			to skip the refresh module of doing so, and let us do it here.
*
**/
void CLGBasePacketEntity::ComputeEntitySkeletonTransforms( EntitySkeletonBonePose *tempBonePoses ) {
	// Get pointer.
	const model_t *model = entitySkeleton.modelPtr;
	if ( !model || !model->skeletalModelData ) {
		refreshEntity.currentBonePoses = nullptr;
		return;
	}
	// Get skeletal model data pointer.
	const SkeletalModelData *skm = model->skeletalModelData;

	// We got SKM, let's see what we are up against.
	// Get state references.
	EntityState *currentState	= &podEntity->currentState;
	EntityState *previousState	= &podEntity->previousState;

	// Get Animation State references.
	EntityAnimationState *currentAnimation	= &currentState->currentAnimation;

	// If we got no animations somehow, set bonepose ptr to nullptr and return.
	if (!skm->animations.size()) {
		refreshEntity.currentBonePoses = nullptr;
		return;
	}
	
	// See if we got skeletal model data.
	const model_t *esModel = entitySkeleton.modelPtr;

	// Without an initialized entity skeleton, we just process the refreshanimation as is, covering the entire model.
	if ( !esModel || !esModel->skeletalModelData ) {
		refreshEntity.currentBonePoses = nullptr;
	} else {
		// Validate animation, and action indices.
		SkeletalModelData *skm = esModel->skeletalModelData;

		// Get our animation data.
		SkeletalAnimation *animation = GetAnimation( currentAnimation->animationIndex );
		// We NEED an animation to work with...
		if ( !animation ) {
			refreshEntity.currentBonePoses = nullptr;
			return;
		}

		// Get pointer to dominating blend action.
		SkeletalAnimationBlendAction *dominatingBlendAction = GetBlendAction( animation, 0 );
		// Get the dominating blend action state.
		EntitySkeletonBlendActionState *dominatingBlendActionState = GetBlendActionState( currentAnimation->animationIndex, 0 );

		// Ensure that it is in bounds. (Should be.)
		if ( !dominatingBlendActionState ) {
			refreshEntity.currentBonePoses = nullptr;
			return;
		}

		// Allocate a cached memory block for the dominating blend action. (Our main action timeline.)
		EntitySkeletonBonePose *dominatingBlendPose = clgi.TBC_AcquireCachedMemoryBlock( model->iqmData->num_poses );
		// Lerp the blend action skeleton pose.
		clgi.ES_LerpSkeletonPoses( &entitySkeleton, 
									dominatingBlendPose,
									dominatingBlendActionState->currentFrame, 
									dominatingBlendActionState->oldFrame, 
									dominatingBlendActionState->backLerp, 
									refreshEntity.rootBoneAxisFlags
		);

		// Assign our currentbonePose pointer. It gets unset in case of any issues. (Better not render than glitch render.)
		refreshEntity.currentBonePoses = dominatingBlendPose;

		// Go over all other blend actions, and acquire their data also.
		for ( int32_t blendActionIndex = 1; blendActionIndex < animation->blendActions.size(); blendActionIndex++ ) {
			// Get pointer to the 'sub dominating' blend action.
			SkeletalAnimationBlendAction *subdominatingBlendAction = GetBlendAction( animation, blendActionIndex );

			// Only proceed if we got one.
			if ( subdominatingBlendAction ) {
				// Now get its actual action pointer information we seek.
				SkeletalAnimationAction *subdominatingAction = GetAction( subdominatingBlendAction->actionIndex );

				// One final bounds test:
				if ( subdominatingAction ) {
					// We've got the action data, time to process its frame for time and store it into
					// our distinct entity skeleton.
					EntitySkeletonBlendActionState *baState = GetBlendActionState( currentAnimation->animationIndex, blendActionIndex );
					//EntitySkeletonBlendActionState *baState = &entitySkeleton.blendActionAnimationStates[ currentAnimation->animationIndex ][ blendActionIndex ];
					// Allocate our blend action bone pose channel.
					EntitySkeletonBonePose *blendActionBonePose	= clgi.TBC_AcquireCachedMemoryBlock( model->iqmData->num_poses );
					if ( !baState || !blendActionBonePose) {
						refreshEntity.currentBonePoses = nullptr;
						return;
					}

					// Lerp the blend action skeleton pose.
					clgi.ES_LerpSkeletonPoses( &entitySkeleton, blendActionBonePose, baState->currentFrame, baState->oldFrame, baState->backLerp, refreshEntity.rootBoneAxisFlags );

					// Now, see if the node exists and blend right on top of it.
					const int32_t boneNumber = subdominatingBlendAction->boneNumber;
					const float fraction = subdominatingBlendAction->fraction;

					// Bone exists.
					if ( boneNumber < entitySkeleton.bones.size() ) {
						//auto *hipNode = entitySkeleton.boneMap["mixamorig8:Spine1"]; // Blind guess.
						auto *boneNode = entitySkeleton.bones[ boneNumber ].boneTreeNode;

						if (boneNode && boneNode->GetChildren().size() > 0) {
							// Recursive blend the Bone animations starting from joint #4, between relativeJointsB and A. (A = src, and dest.)
							clgi.ES_RecursiveBlendFromBone( blendActionBonePose, dominatingBlendPose, boneNode, baState->backLerp, fraction );
						}

						// Assign our currentbonePose pointer. It gets unset in case of any issues. (Better not render than glitch render.)
						refreshEntity.currentBonePoses = dominatingBlendPose;
					}
				} else {
					// TODO: Debug warning here.
					refreshEntity.currentBonePoses = nullptr;
					return;
				}
			} else {
				// TODO: Debug warning here.
				refreshEntity.currentBonePoses = nullptr;
				return;
			}
		}
	}
}

/***
*
*
*	Utility Functions, for easy bounds checking and sorts of tasks alike.
*	(Wraps around clgi).
*
*
***/
/**
*	@brief	Utility function to test whether an animation is existent and within range.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified action.
**/
SkeletalAnimation *CLGBasePacketEntity::GetAnimation( const std::string &name ) {
	return clgi.ES_GetAnimationByName( &entitySkeleton, name );
}
SkeletalAnimation *CLGBasePacketEntity::GetAnimation( const int32_t index ) {
	return clgi.ES_GetAnimationByIndex( &entitySkeleton, index );
}

/**
*	@brief	Utility function to easily get a pointer to an Action by name or index.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified Action.
**/
SkeletalAnimationAction *CLGBasePacketEntity::GetAction( const std::string &name ) {
	return clgi.ES_GetActionByName( &entitySkeleton, name );
}
SkeletalAnimationAction *CLGBasePacketEntity::GetAction( const int32_t index ) {
	return clgi.ES_GetActionByIndex( &entitySkeleton, index );
}

/**
*	@brief	Utility function to test whether a BlendAction is existent and within range.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified BlendAction action.
**/
SkeletalAnimationBlendAction *CLGBasePacketEntity::GetBlendAction( SkeletalAnimation *animation, const int32_t index ) {
	return clgi.ES_GetBlendAction( &entitySkeleton, animation, index );
}

/**
*	@brief	Utility function to test whether a BlendActionState is existent and within range for the specified Animation.
*	@return	(nullptr) on failure. Otherwise a pointer to the specified BlendActionState action.
**/
EntitySkeletonBlendActionState *CLGBasePacketEntity::GetBlendActionState( const int32_t animationIndex, const int32_t blendActionIndex ) {
	return clgi.ES_GetBlendActionState( &entitySkeleton, animationIndex, blendActionIndex );
}


/**
* 
(
*   Refresh Entity Setup.
* 
*
**/
/**
*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
**/
void CLGBasePacketEntity::PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) {
	extern qhandle_t cl_mod_laser;

	// If we don't have a PODEntity then we can't do anything.
	if (!podEntity) {
		CLG_Print( PrintType::DeveloperWarning, fmt::format( "Warning: PrepareRefreshEntity has no valid podEntity pointer for refreshEntityID(#{})!\n", refreshEntityID ));
		return;
	}

    // Client Info.
    ClientInfo* clientInfo = nullptr;
	// Base entity flags.
    int32_t baseEntityFlags = 0;
	// Entity specific entityEffects. (Such as whether to rotate or not.)
    uint32_t entityEffects = 0;
    // Entity render entityEffects. (Shells and the like.)
    uint32_t renderEffects= 0;

	// Are we dealing with a skeletal model here?
	const bool isSkeletalModel = ( skm && entitySkeleton.modelPtr );

    /**
    *	Setup the refreshEntity its ID:
    **/
    // Fetch the entity index.
    const int32_t entityIndex = podEntity->clientEntityNumber;
    // Setup the render entity ID for the renderer.
    refreshEntity.id = refreshEntityID;// + RESERVED_ENTITIY_COUNT;

	/**
    *	Render-)effects:
	**/
    // Fetch the entityEffects of current entity.
    entityEffects = currentState->effects;
    // Fetch the render entityEffects of current entity.
    renderEffects = currentState->renderEffects;

	/**
	*	Model Light Styles:
	**/
	refreshEntity.modelLightStyle = GetStyle();

    /**
    *	Frame Based Animation Effects:
    **/
    // For brush model texture frame animation.
    const int32_t autoAnimation = BASE_FRAMERATE * cl->time / BASE_FRAMETIME_1000;

    if (entityEffects & EntityEffectType::AnimCycleFrames01hz2) {
        refreshEntity.frame = autoAnimation & 1;
	} else if (entityEffects & EntityEffectType::AnimCycleFrames23hz2) {
        refreshEntity.frame = 2 + (autoAnimation & 1);
	} else if (entityEffects & EntityEffectType::AnimCycleAll2hz) {
        refreshEntity.frame = autoAnimation;
	} else if (entityEffects & EntityEffectType::AnimCycleAll30hz) {
        refreshEntity.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
	/**
	*	Skeletal Animation:
	**/
	} else {
		// With skeletal model data we calculate all required data her so we can intercept it on time
		// to apply custom needs. (Attaching objects to a bone, etc.)
		if ( isSkeletalModel ) {
			// Process the skeletal animation blend action frames for the current client time. (Based on animation start time.)
			ProcessSkeletalAnimationForTime( GameTime( cl->time ) ); 
			// Compute the Entity Skeleton Trasforms for Refresh Frame.
			ComputeEntitySkeletonTransforms( nullptr );
		// Otherwise make sure to set the bonePoses to nullptr since it might've been set to something in the
		// previous frame.
		} else {
			refreshEntity.currentBonePoses = nullptr;
		}
    }
        

	/**
    *	Optionally remove the glowing effect:
	**/
    if (cl_noglow->integer) {
        renderEffects&= ~RenderEffects::Glow;
	}


    /**
    *	Origin:
    **/
    // Step origin discretely, because the model frames do the animation properly.
	if ( renderEffects & RenderEffects::FrameLerp ) {
        refreshEntity.origin = podEntity->currentState.origin;
        refreshEntity.oldorigin = podEntity->currentState.oldOrigin;
	// Interpolate start and end points for beams
	} else if ( renderEffects & RenderEffects::Beam ) {
        refreshEntity.origin = vec3_mix(podEntity->previousState.origin, podEntity->currentState.origin, cl->lerpFraction);
        refreshEntity.oldorigin = vec3_mix(podEntity->previousState.oldOrigin, podEntity->currentState.oldOrigin, cl->lerpFraction);
    } else {
        if ( currentState->number == cl->frame.clientNumber + 1 ) {
            // In case of this being our actual client entity, we use the predicted origin.
            refreshEntity.origin = cl->playerEntityOrigin;
            refreshEntity.oldorigin = cl->playerEntityOrigin;
        } else {
			// Extrapolate if needed.
			PODEntity *podGroundEntity = GetGroundPODEntity();

			// Extrapolate the mover.
			if ( IsExtrapolating() || podEntity->linearMovement.isMoving ) {
				//refreshEntity.oldorigin = refreshEntity.origin;		
				//if ( cl->xerpFraction ) {
				//	refreshEntity.oldorigin = vec3_mix( previousState->oldOrigin, currentState->oldOrigin, 1.0 - cl->xerpFraction );
				//	refreshEntity.origin = vec3_mix( previousState->origin, currentState->origin, 1.0 - cl->xerpFraction );
				//} else {
				//	refreshEntity.origin = currentState->origin;
				//	refreshEntity.oldorigin = refreshEntity.origin;
				//}
				if ( cl->xerpFraction ) {
					refreshEntity.oldorigin = vec3_mix( previousState->oldOrigin, currentState->oldOrigin, cl->xerpFraction );
					refreshEntity.origin = vec3_mix( previousState->origin, currentState->origin, cl->xerpFraction );
				} else {
					refreshEntity.oldorigin = currentState->origin;
					refreshEntity.origin = currentState->origin;
				}
//				refreshEntity.oldorigin = refreshEntity.origin;
			}
			// Extrapolate for ground entity movement.
			else if ( ( podGroundEntity && podGroundEntity->linearMovement.isMoving ) ) {
					//refreshEntity.oldorigin = vec3_mix( previousState->oldOrigin, currentState->oldOrigin, 1.0 - cl->xerpFraction );
					//refreshEntity.origin = vec3_mix( previousState->origin, currentState->origin, 1.0 - cl->xerpFraction );
				if ( cl->xerpFraction ) {
					refreshEntity.oldorigin = vec3_mix( previousState->oldOrigin, currentState->oldOrigin, cl->lerpFraction  );
					refreshEntity.origin = vec3_mix( previousState->origin, currentState->origin, cl->lerpFraction  );
				} else {
					//refreshEntity.oldorigin = currentState->oldOrigin;
					refreshEntity.oldorigin = currentState->origin;
					refreshEntity.origin = currentState->origin;
				}
//				refreshEntity.oldorigin = refreshEntity.origin;
//				refreshEntity.oldorigin = refreshEntity.origin;
//				refreshEntity.origin = currentState->origin; //vec3_mix( currentState->origin, podEntity->currentState.origin, cl->xerpFraction );//gePlayer->GetClient()->playerState.pmove.origin; //vec3_mix( cl->frame.playerState.pmove.origin, gePlayer->GetClient()->playerState.pmove.origin, f);
			// Linear Interpolate for regular packet entity origin changes. ( Client is always a frame behind )
			} else {
				// Ohterwise, just neatly interpolate the origin.
				refreshEntity.origin = vec3_mix(podEntity->previousState.origin, podEntity->currentState.origin, cl->lerpFraction);
				// Neatly copy it as the refreshEntity's oldorigin.
				refreshEntity.oldorigin = refreshEntity.origin;
			}
        }
    }


	/**
	*	Particle Debug BoundingBox:
	**/
	if ( renderEffects& RenderEffects::DebugBoundingBox ) {
	    CLG_DrawDebugBoundingBox( podEntity->lerpOrigin, podEntity->mins, podEntity->maxs );
	}


	/**
    *	Beam Color Tweaking:
	**/
    if (renderEffects & RenderEffects::Beam) {
        // The four beam colors are encoded in 32 bits of skinNumber (hack)
        refreshEntity.alpha = 0.30;
        refreshEntity.skinNumber = (currentState->skinNumber >> ((rand() % 4) * 8)) & 0xff;
        refreshEntity.model = 0;
    } else {
		// Set the entity model skin
        if (currentState->modelIndex == 255) {
            // Use a custom player skin
            clientInfo = &cl->clientInfo[currentState->skinNumber & 255];
            refreshEntity.skinNumber = 0;
            refreshEntity.skin = clientInfo->skin;
            refreshEntity.model = clientInfo->model;

            // Setup default base client info in case of 0.
            if (!refreshEntity.skin || !refreshEntity.model) {
                refreshEntity.skin = cl->baseClientInfo.skin;
                refreshEntity.model = cl->baseClientInfo.model;
                clientInfo = &cl->baseClientInfo;
            }

            // Special Disguise render effect handling.
            if (renderEffects& RenderEffects::UseDisguise) {
                char buffer[MAX_QPATH];

                Q_concat(buffer, sizeof(buffer), "players/", clientInfo->model_name, "/disguise.pcx", NULL);
                refreshEntity.skin = clgi.R_RegisterSkin(buffer);
            }
        } else {
            // Default entity skin number handling behavior.
            refreshEntity.skinNumber = currentState->skinNumber;
            refreshEntity.skin = 0;
            refreshEntity.model = cl->drawModels[currentState->modelIndex];

            // Disable shadows on lasers and dm spots.
            if ( refreshEntity.model == cl_mod_laser )
                renderEffects|= RF_NOSHADOW;
        }
    }


	/**
    *	Only used for black hole model right now, FIXME: do better
	**/
    if ((renderEffects& RenderEffects::Translucent) && !(renderEffects& RenderEffects::Beam)) {
        refreshEntity.alpha = 0.70;
    }


	/**
    *	Render entityEffects (fullbright, translucent, etc)
	**/
    if ((entityEffects & EntityEffectType::ColorShell)) {
        refreshEntity.flags = 0;  // Render entityEffects go on color shell entity
    } else {
        refreshEntity.flags = renderEffects;
    }


    /**
    *	Angles.
    **/
    if (entityEffects & EntityEffectType::Rotate) {
		// Bonus items rotate at a fixed rate
		const float autoRotate = AngleMod( cl->time * BASE_1_FRAMETIME );

        // Autorotate for bonus item entities.
        refreshEntity.angles[0] = 0;
        refreshEntity.angles[1] = autoRotate;
        refreshEntity.angles[2] = 0;
    } else if (currentState->number == cl->frame.clientNumber + 1) {
        // Predicted angles for client entities.
        refreshEntity.angles = cl->playerEntityAngles;
    } else {
        // Otherwise, lerp angles by default.
        refreshEntity.angles = vec3_mix_euler( podEntity->previousState.angles, podEntity->currentState.angles, cl->lerpFraction );

        // Mimic original ref_gl "leaning" bug (uuugly!)
        if (currentState->modelIndex == 255 && cl_rollhack->integer) {
            refreshEntity.angles[vec3_t::Roll] = -refreshEntity.angles[vec3_t::Roll];
        }
    }

		
	/**
	*	Entity Skeleton - PostCompute Transforms:
	**/
	// Make sure we got "skeletal model data", an "entity skeleton", and computed bone pose transforms to work with.
	if ( isSkeletalModel && refreshEntity.currentBonePoses != nullptr ) {
		// Pass it our current generated boen poses.
		PostComputeSkeletonTransforms( refreshEntity.currentBonePoses );
	}

	/**
	*	Client Entity (Thirdperson/Reflection-)model:
	**/
    if (currentState->number == cl->frame.clientNumber + 1) {
        if (!cl->thirdPersonView)
        {
            // Special case handling for RTX rendering. Only draw third person model from mirroring surfaces.
            if (vid_rtx->integer)
                baseEntityFlags |= RenderEffects::ViewerModel;
            else
                goto skip;
        }

        // Don't tilt the model - looks weird
        refreshEntity.angles[0] = 0.f;
        // Temporary fix, not quite perfect though. Add some z offset so the shadow isn't too dark under the feet.
        refreshEntity.origin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
        refreshEntity.oldorigin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
    }

    // If set to invisible, skip
    if (!currentState->modelIndex) {
        goto skip;
    }


	/**
	*	Color Shell:
	**/
    // Add the baseEntityFlags to the refreshEntity flags.
    refreshEntity.flags |= baseEntityFlags;

    // In rtx mode, the base entity has the renderEffectsfor shells
    if ((entityEffects & EntityEffectType::ColorShell) && vid_rtx->integer) {
        refreshEntity.flags |= renderEffects;
    }

    // Last but not least, add the entity to the refresh render list.
    clge->view->AddRenderEntity(refreshEntity);

    // For color shells we generate a separate entity for the main model.
    // (Use the settings of the already rendered model, and apply translucency to it.
    if ((entityEffects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
		// Apply flags.
        refreshEntity.flags = renderEffects | baseEntityFlags | RenderEffects::Translucent;
		// Set alpha.
        refreshEntity.alpha = 0.30;
		// Add the shell refresh entity.
        clge->view->AddRenderEntity(refreshEntity);
    }

	// Reset skin, number, flags and alpha for further processing of other model indexes.
    refreshEntity.skin = 0;       // never use a custom skin on others
    refreshEntity.skinNumber = 0;
    refreshEntity.flags = baseEntityFlags;
    refreshEntity.alpha = 0;


	/**
	*	ModelIndex #2: 
	**/
    // Add an entity to the current rendering frame that has model index 2 attached to it.
    // Duplicate for linked models
    if (currentState->modelIndex2) {
        if (currentState->modelIndex2 == 255) {
            // Custom weapon
            clientInfo = &cl->clientInfo[currentState->skinNumber & 0xff];
                
            // Determine skinIndex.
            int32_t skinIndex = (currentState->skinNumber >> 8); // 0 is default weapon model
            if (skinIndex < 0 || skinIndex > cl->numWeaponModels - 1) {
                skinIndex = 0;
            }

            // Fetch weapon model.
            refreshEntity.model = clientInfo->weaponmodel[skinIndex];

            // If invalid, use defaults.
            if (!refreshEntity.model) {
                if (skinIndex != 0) {
                    refreshEntity.model = clientInfo->weaponmodel[0];
                }
                if (!refreshEntity.model) {
                    refreshEntity.model = cl->baseClientInfo.weaponmodel[0];
                }
            }
        } else {
            refreshEntity.model = cl->drawModels[currentState->modelIndex2];
        }


        if ((entityEffects & EntityEffectType::ColorShell) && vid_rtx->integer) {
            refreshEntity.flags |= renderEffects;
        }

        clge->view->AddRenderEntity(refreshEntity);

        //PGM - make sure these get reset.
        refreshEntity.flags = baseEntityFlags;
        refreshEntity.alpha = 0;
    }


	/**
	*	ModelIndex #3: 
	**/
    // Add an entity to the current rendering frame that has model index 3 attached to it.
    //if (currentState->modelIndex3) {
    //    refreshEntity.model = cl->drawModels[currentState->modelIndex3];
    //    clge->view->AddRenderEntity(refreshEntity);
    //}


	/**
	*	ModelIndex #4: 
	**/
    //// Add an entity to the current rendering frame that has model index 4 attached to it.
    //if (currentState->modelIndex4) {
    //    refreshEntity.model = cl->drawModels[currentState->modelIndex4];
    //    clge->view->AddRenderEntity(refreshEntity);
    //}


    /**
    *	Particle Trail Effects:
    **/
    // Add automatic particle trail entityEffects where desired.
    if (entityEffects & ~EntityEffectType::Rotate) {
        if (entityEffects & EntityEffectType::Gib) {
            ParticleEffects::DiminishingTrail(podEntity->lerpOrigin, refreshEntity.origin, podEntity, entityEffects);
        } else if (entityEffects & EntityEffectType::Torch) {
            const float anim = sinf((float)refreshEntity.id + ((float)cl->time / 60.f + frand() * 3.3)) / (3.14356 - (frand() / 3.14356));
            const float offset = anim * 0.0f;
            const float brightness = anim * 1.2f + 1.6f;
            const vec3_t origin = { 
                refreshEntity.origin.x,
                refreshEntity.origin.y,
                refreshEntity.origin.z + offset 
            };

            clge->view->AddLight(origin, vec3_t{ 1.0f * brightness, 0.425f * brightness, 0.1f * brightness }, 25.f, 3.6f);
        }
    }

skip:
    // Assign refreshEntity origin to podEntity lerp origin in the case of a skip.
    podEntity->lerpOrigin = refreshEntity.origin;
}