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
#include "../../ClientGameLocals.h"

// Base Client Game Functionality.
#include "../Debug.h"
#include "../Entities.h"
#include "../TemporaryEntities.h"

// Export classes.
#include "../Exports/Entities.h"
#include "../Exports/View.h"

// Effects.
#include "../Effects/ParticleEffects.h"

// Base Entity.
#include "CLGBaseLocalEntity.h"



//! Used for returning vectors from a const vec3_t & reference.
vec3_t CLGBaseLocalEntity::ZeroVec3 = vec3_zero();

//! Used for returning strings from a const std::string & reference.
std::string CLGBaseLocalEntity::EmptyString = "";

/**
*
*   Constructor/Destructor AND TypeInfo related.
*
**/
//! Constructor/Destructor.
CLGBaseLocalEntity::CLGBaseLocalEntity(PODEntity* podEntity) : Base() {//}, podEntity(clEntity) {
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
void CLGBaseLocalEntity::Precache() {

}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void CLGBaseLocalEntity::Spawn() {

}
/**
*   @brief  Called when it is time to respawn this entity.
**/
void CLGBaseLocalEntity::Respawn() {

}

/**
*   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
**/
void CLGBaseLocalEntity::PostSpawn() {

}

/**
*   @brief  General entity thinking routine.
**/
void CLGBaseLocalEntity::Think() {
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
void CLGBaseLocalEntity::SpawnKey(const std::string& key, const std::string& value) {
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
	}
	// Spawnflags.
	else if (key == "spawnflags") {
		// Parse damage.
		int32_t parsedSpawnFlags = 0;
		ParseKeyValue(key, value, parsedSpawnFlags);

		// Set SpawnFlags.
		SetSpawnFlags(parsedSpawnFlags);
	} else {
	    Com_DPrint(std::string("Warning: Entity[#" + std::to_string(GetNumber()) + ":" + GetClassname() + "] has unknown Key/Value['" + key + "','" + value + "']\n").c_str());
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
*   @brief  Updates the entity with the data of the newly passed EntityState object.
**/
void CLGBaseLocalEntity::UpdateFromState(const EntityState& state) {
    //previousState = currentState;
    //currentState = state;
}

/**
*   @returen True if the entity is still in the current frame.
**/
//const qboolean CLGBaseLocalEntity::IsInUse() {
//    if (podEntity) {
//        return cl->frame.number == podEntity->serverFrame;
//    } else {
//        false;
//    }
//}

/**
*   @brief  Stub.
**/
const std::string CLGBaseLocalEntity::GetClassname() {
    // Returns this classname, the base entity.
    return GetTypeInfo()->classname;
}

/**
*   @return An uint32_t containing the hashed classname string.
**/
uint32_t CLGBaseLocalEntity::GetHashedClassname() {
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
void CLGBaseLocalEntity::OnDeallocate() {

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
void CLGBaseLocalEntity::DispatchUseCallback(IClientGameEntity* other, IClientGameEntity* activator) {
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
void CLGBaseLocalEntity::DispatchDieCallback(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point) {
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
void CLGBaseLocalEntity::DispatchBlockedCallback(IClientGameEntity* other) {
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
void CLGBaseLocalEntity::DispatchTouchCallback(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
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
void CLGBaseLocalEntity::DispatchTakeDamageCallback(IClientGameEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
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
*           on us during the current server game frame we're processing.
**/
void CLGBaseLocalEntity::Remove()
{
	podEntity->clientFlags |= EntityServerFlags::Remove;
}

/**
*   @brief  Callback method to use for freeing this entity. It calls upon Remove()
**/
void CLGBaseLocalEntity::CLGLocalClientEntityThinkFree(void) {
	//CLG_FreeEntity(serverEntity);
	Remove();
}


void CLGBaseLocalEntity::ProcessSkeletalAnimationForTime(uint64_t time) {
	// Acquire state references.
	EntityState &currentState = podEntity->currentState;
	EntityState &previousState = podEntity->previousState;

	// Process the animation.
	refreshEntity.oldframe = previousState.animationFrame;
    refreshEntity.backlerp = 1.0 - SG_FrameForTime(&refreshEntity.frame,
        GameTime(time),                                     // Current Time.
        GameTime(currentState.animationStartTime),           // Animation Start time. (TODO: This needs to changed to a stored cl->time of the moment where the animation event got through.)
        currentState.animationFramerate,           // Current frame time.
        currentState.animationStartFrame,          // Start frame.
        currentState.animationEndFrame,            // End frame.
        0,                                                  // Loop count.
        true                                                // Force loop
    );
    currentState.animationFrame = refreshEntity.frame;
}
/**
*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
**/
void CLGBaseLocalEntity::PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) {
	extern qhandle_t cl_mod_laser;
	extern qhandle_t cl_mod_dmspot;

	// If we don't have a PODEntity then we can't do anything.
	if (!podEntity) {
		Com_DPrint("Warning: PrepareRefreshEntity has no valid podEntity pointer for refreshEntityID(#%i)!\n", refreshEntityID);
		return;
	}
	refreshEntity = {};
	// C++20: Had to be placed here because of label skip.
    int32_t baseEntityFlags = 0;
	// Client Info.
	ClientInfo*  clientInfo = nullptr;
	// Setup refresh ID.
    refreshEntity.id = refreshEntityID;


    //
    // Effects.
    // 
    // Entity Effects. (These may influence the final results of the already set Render Effects).
    const uint32_t effects = currentState->effects;
    // Entity Render Effects set by game logic.
    int32_t renderEffects = currentState->renderEffects;
	// Bonus items rotate at a fixed rate
	float autoRotate = AngleMod(cl->time * BASE_1_FRAMETIME);
	// Brush models can auto animate their frames
	int32_t autoAnimation = BASE_FRAMERATE * cl->time / BASE_FRAMETIME_1000;


    //
    // Frame Animation Effects.
    //
    if (effects & EntityEffectType::AnimCycleFrames01hz2) {
        refreshEntity.frame = autoAnimation & 1;
	} else if (effects & EntityEffectType::AnimCycleFrames23hz2) {
        refreshEntity.frame = 2 + (autoAnimation & 1);
	} else if (effects & EntityEffectType::AnimCycleAll2hz) {
        refreshEntity.frame = autoAnimation;
	} else if (effects & EntityEffectType::AnimCycleAll30hz) {
        refreshEntity.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
	} else {
    	//// Fetch the iqm animation index data.
        //   if (currentState->animationIndex != 0 && currentState->modelIndex != 0) {
		    // model_t* iqmData = clgi.MOD_ForHandle(currentState->modelIndex);

        //       if (iqmData) {
			    //  Com_DPrint("WOW!!!\n");
        //       }
        //   } else {
		    //refreshEntity.frame = currentState->animationFrame;
		//       refreshEntity.oldframe = previousState->animationFrame;
	//        refreshEntity.backlerp = 1.0 - lerpFraction;
//            }

 
        //currentState->animationFrame, 
	//   framefrac = GS_FrameForTime(&curframe, cg.time, viewweapon->baseAnimStartTime,  // start time
	//weaponInfo->frametime[viewweapon->baseAnim],				    // current frame time?
	//weaponInfo->firstframe[viewweapon->baseAnim],				    // first frame.
	//weaponInfo->lastframe[viewweapon->baseAnim],				    // last frame.
	//weaponInfo->loopingframes[viewweapon->baseAnim],			    // looping frames.
	//true); 
    }
        

    // Optionally remove the glowing effect.
	if (cl_noglow->integer) {
        renderEffects &= ~RenderEffects::Glow;
	}

	//
	//	Skeletal Animation Progressing.
	//
    // Setup the proper lerp and model frame to render this pass.
    // Moved into the if statement's else case up above.
    ProcessSkeletalAnimationForTime(cl->serverTime);
	//refreshEntity.oldframe = previousState->animationFrame;
    //refreshEntity.backlerp = 1.0 - SG_FrameForTime(&refreshEntity.frame,
    //    GameTime(cl->serverTime),                                     // Current Time.
    //    GameTime(currentState->animationStartTime),           // Animation Start time. (TODO: This needs to changed to a stored cl->time of the moment where the animation event got through.)
    //    currentState->animationFramerate,           // Current frame time.
    //    currentState->animationStartFrame,          // Start frame.
    //    currentState->animationEndFrame,            // End frame.
    //    0,                                                  // Loop count.
    //    true                                                // Force loop
    //);
    //currentState->animationFrame = refreshEntity.frame;
		

    //
    // Setup RefreshEntity origin.
    //
    if (renderEffects & RenderEffects::FrameLerp) {
        // Step origin discretely, because the model frames do the animation properly.
        refreshEntity.origin = currentState->origin;
        refreshEntity.oldorigin = currentState->oldOrigin;
    } else if (renderEffects & RenderEffects::Beam) {
        // Interpolate start and end points for beams
        refreshEntity.origin = vec3_mix(previousState->origin, currentState->origin, lerpFraction);
        refreshEntity.oldorigin = vec3_mix(previousState->oldOrigin, currentState->oldOrigin, lerpFraction);
    } else {
        if (currentState->number == cl->frame.clientNumber + 1) {
            // In case of this being our actual client entity, we use the predicted origin.
            refreshEntity.origin = cl->playerEntityOrigin;
            refreshEntity.oldorigin = cl->playerEntityOrigin;
        } else {
            // Ohterwise, just neatly interpolate the origin.
            refreshEntity.origin = vec3_mix(previousState->origin, currentState->origin, lerpFraction);
            // Neatly copy it as the refreshEntity's oldorigin.
            refreshEntity.oldorigin = refreshEntity.origin;
        }
    }

	// Draw debug bounding box for client entity.
	if (renderEffects & RenderEffects::DebugBoundingBox) {
	    CLG_DrawDebugBoundingBox(podEntity->lerpOrigin, podEntity->mins, podEntity->maxs);
	}

    // tweak the color of beams
    if (renderEffects & RenderEffects::Beam) {
        // The four beam colors are encoded in 32 bits of skinNumber (hack)
        refreshEntity.alpha = 0.30;
        refreshEntity.skinNumber = (currentState->skinNumber >> ((rand() % 4) * 8)) & 0xff;
        refreshEntity.model = 0;
    } else {
        //
        // Set the entity model skin
        //
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
            if (renderEffects & RenderEffects::UseDisguise) {
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
            if (refreshEntity.model == cl_mod_laser || refreshEntity.model == cl_mod_dmspot)
                renderEffects |= RF_NOSHADOW;
        }
    }

    // Only used for black hole model right now, FIXME: do better
    if ((renderEffects & RenderEffects::Translucent) && !(renderEffects & RenderEffects::Beam)) {
        refreshEntity.alpha = 0.70;
    }

    // Render effects (fullbright, translucent, etc)
    if ((effects & EntityEffectType::ColorShell)) {
        refreshEntity.flags = 0;  // Render effects go on color shell entity
    } else {
        refreshEntity.flags = renderEffects;
    }

    //
    // Angles.
    //
    if (effects & EntityEffectType::Rotate) {
        // Autorotate for bonus item entities.
        refreshEntity.angles[0] = 0;
        refreshEntity.angles[1] = autoRotate;
        refreshEntity.angles[2] = 0;
    } else if (currentState->number == cl->frame.clientNumber + 1) {
        // Predicted angles for client entities.
        refreshEntity.angles = cl->playerEntityAngles;
    } else {
        // Otherwise, lerp angles by default.
        refreshEntity.angles = vec3_mix(previousState->angles, currentState->angles, lerpFraction);

        // Mimic original ref_gl "leaning" bug (uuugly!)
        if (currentState->modelIndex == 255 && cl_rollhack->integer) {
            refreshEntity.angles[vec3_t::Roll] = -refreshEntity.angles[vec3_t::Roll];
        }
    }

    //
    // Entity Effects for in case the entity is the actual client.
    //
    if (currentState->number == cl->frame.clientNumber + 1) {
        if (!cl->thirdPersonView)
        {
            // Special case handling for RTX rendering. Only draw third person model from mirroring surfaces.
            if (vid_rtx->integer) {
                baseEntityFlags |= RenderEffects::ViewerModel;
			} else {
				// Assign refreshEntity origin to clientEntity lerp origin in the case of a skip.
				podEntity->lerpOrigin = refreshEntity.origin;
				return;
			}
        }

        // Don't tilt the model - looks weird
        refreshEntity.angles[0] = 0.f;

        //
        // TODO: This needs to be fixed properly for the shadow to render.
        // 
        // Offset the model back a bit to make the view point located in front of the head
        //constexpr float offset = -15.f;
        //constexpr float offset = 8.f;// 0.0f;
        //vec3_t angles = { 0.f, refreshEntity.angles[1], 0.f };
        //vec3_t forward;
        //AngleVectors(angles, &forward, NULL, NULL);
        //refreshEntity.origin = vec3_fmaf(refreshEntity.origin, offset, forward);
        //refreshEntity.oldorigin = vec3_fmaf(refreshEntity.oldorigin, offset, forward);

        // Temporary fix, not quite perfect though. Add some z offset so the shadow isn't too dark under the feet.
        refreshEntity.origin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
        refreshEntity.oldorigin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
    }

    // If set to invisible, skip
    if (!currentState->modelIndex) {
		// Assign refreshEntity origin to clientEntity lerp origin in the case of a skip.
		podEntity->lerpOrigin = refreshEntity.origin;
		return;
    }

    // Add the baseEntityFlags to the refreshEntity flags.
    refreshEntity.flags |= baseEntityFlags;

    // In rtx mode, the base entity has the renderEffects for shells
    if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
        //renderEffects = ApplyRenderEffects(renderEffects);
        refreshEntity.flags |= renderEffects;
    }

    // Last but not least, add the entity to the refresh render list.
    clge->view->AddRenderEntity(refreshEntity);

    // Keeping it here commented to serve as an example case.
    // Add dlights for flares
    //model_t* model;
    //if (refreshEntity.model && !(refreshEntity.model & 0x80000000) &&
    //    (model = clgi.MOD_ForHandle(refreshEntity.model)))
    //{
    //    if (model->model_class == MCLASS_FLARE)
    //    {
    //        float phase = (float)cl->time * 0.03f + (float)refreshEntity.id;
    //        float anim = sinf(phase);

    //        float offset = anim * 1.5f + 5.f;
    //        float brightness = anim * 0.2f + 0.8f;

    //        vec3_t origin;
    //        VectorCopy(refreshEntity.origin, origin);
    //        origin[2] += offset;

    //        V_AddLightEx(origin, 500.f, 1.6f * brightness, 1.0f * brightness, 0.2f * brightness, 5.f);
    //    }
    //}

    // For color shells we generate a separate entity for the main model.
    // (Use the settings of the already rendered model, and apply translucency to it.
    if ((effects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
        //renderEffects = ApplyRenderEffects(renderEffects);
        refreshEntity.flags = renderEffects | baseEntityFlags | RenderEffects::Translucent;
        refreshEntity.alpha = 0.30;
        clge->view->AddRenderEntity(refreshEntity);
    }

    refreshEntity.skin = 0;       // never use a custom skin on others
    refreshEntity.skinNumber = 0;
    refreshEntity.flags = baseEntityFlags;
    refreshEntity.alpha = 0;

    //
    // ModelIndex2
    // 
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


        if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
            refreshEntity.flags |= renderEffects;
        }

        clge->view->AddRenderEntity(refreshEntity);

        //PGM - make sure these get reset.
        refreshEntity.flags = baseEntityFlags;
        refreshEntity.alpha = 0;
    }

    //
    // ModelIndex3
    // 
    // Add an entity to the current rendering frame that has model index 3 attached to it.
    if (currentState->modelIndex3) {
        refreshEntity.model = cl->drawModels[currentState->modelIndex3];
        clge->view->AddRenderEntity(refreshEntity);
    }

    //
    // ModelIndex4
    // 
    // Add an entity to the current rendering frame that has model index 4 attached to it.
    if (currentState->modelIndex4) {
        refreshEntity.model = cl->drawModels[currentState->modelIndex4];
        clge->view->AddRenderEntity(refreshEntity);
    }

    //
    // Particle Trail Effects.
    // 
    // Add automatic particle trail effects where desired.
    if (effects & ~EntityEffectType::Rotate) {
        if (effects & EntityEffectType::Gib) {
            ParticleEffects::DiminishingTrail(podEntity->lerpOrigin, refreshEntity.origin, podEntity, effects);
        } else if (effects & EntityEffectType::Torch) {
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
}
