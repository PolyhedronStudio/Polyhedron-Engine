/***
*
*	License here.
*
*	@file
*
*	Client Game Entities Interface Implementation.
* 
***/
// Core.
#include "../ClientGameLocal.h"

// Base Client Game Functionality.
#include "../Debug.h"
#include "../Entities.h"
#include "../Main.h"
#include "../TemporaryEntities.h"

// Export classes.
#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "Entities.h"
#include "View.h"

// Effects.
#include "../Effects/ParticleEffects.h"

// Ents.
#include "../Entities/EntityList.h"
#include "../Entities/Base/CLGBaseEntity.h"

// Shared Game.
#include "SharedGame/SkeletalAnimation.h"

// WID: TODO: Gotta fix this one too.
extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;
extern qhandle_t cl_sfx_footsteps[4];

// Use a static entity ID on some things because the renderer relies on EntityID to match between meshes
// between the current and previous frames.
static constexpr int32_t RESERVED_ENTITIY_GUN = 1;
static constexpr int32_t RESERVED_ENTITIY_SHADERBALLS = 2;
static constexpr int32_t RESERVED_ENTITIY_COUNT = 3;


/**
*   @brief  Parses and spawns the local class entities in the BSP Entity String.
* 
*   @details    When a class isn't locally registered, it'll automatically spawn
*               a CLGBaseEntity instead which has all the default behaviors that
*               you'd expect for it to be functional.
* 
*   @return True on success.
**/
qboolean ClientGameEntities::SpawnFromBSPString(const char* bspString) {
	// Clear level state.
    //level = {};

    // Clear out our entity list in case it is holding some from a previous session.
    entityList.Clear();
    
	// Prepare our player class entity.
    //PreparePlayer();

    // Parsing state variables.
	qboolean isParsing = true; // We'll keep on parsing until this is set to false.
	qboolean parsedSuccessfully = false;// This gets set to false the immediate moment we run into parsing trouble.
	char *com_token = nullptr; // Token pointer.
	ClientEntity *clientEntity = nullptr; // Pointer to the client entity we intend to employ.
    uint32_t entityIndex = 0;         

	// Engage parsing.
	while (!!isParsing == true) {
		// Parse the opening brace.
		com_token = COM_Parse(&bspString);

        // Break out when we're done and there is no string data left to parse.
		if (!bspString) {
			break;
		}

        // If the token isn't a {, something is off.
		if (com_token[0] != '{') {
		    Com_Error(ErrorType::Drop, "SpawnFromBSPString: found %s when expecting {", com_token);
			return false;
		}

		// Pick the first entity there is, start asking for 
		if (!clientEntity) {
			clientEntity = cs->entities;
		} else {

            entityIndex++;
            clientEntity = &cs->entities[entityIndex];


            if (entityIndex > MAX_EDICTS) {
                Com_Error(ErrorType::Drop, ("SpawnFromBSPString: entityIndex > MAX_EDICTS\n"));
            }
		}

		// Now we've got the reserved server entity to use, let's parse the entity.
        ParseEntityString(&bspString, clientEntity);

		// Allocate the class entity, and call its spawn.
		if (!SpawnParsedClassEntity(clientEntity)) {
			parsedSuccessfully = false;
		}
	}

	//// Post spawn entities.
	//for (auto& classEntity : classEntities) {
	//	if (classEntity) {
	//		classEntity->PostSpawn();
	//	}
	//}

	//// Find and hook team slaves.
	//FindTeams();

	// Initialize player trail...
	// SVG_PlayerTrail_Init

	return parsedSuccessfully;
}


/**
*	@brief	Parses the BSP Entity string and places the results in the client
*			entity dictionary.
**/
qboolean ClientGameEntities::ParseEntityString(const char** data, ClientEntity* clEntity) {
    // False until proven otherwise.
    qboolean parsedSuccessfully = false;

    // Key value ptrs.
    char *key = nullptr, *value = nullptr;

    // Go through all the dictionary pairs.
    while (1) {
	    // Parse the key.
	    key = COM_Parse(data);
		
	    // If we hit a }, it means we're done parsing, break out of this loop.
	    if (key[0] == '}') {
		    break;
	    }

	    // If we are at the end of the string without a closing brace, error out.
	    if (!*data) {
		    Com_Error(ErrorType::Drop, "%s: EOF without closing brace", __func__);
		    return false;
	    }

	    // Parse the value.
	    value = COM_Parse(data);

	    // If we are at the end of the string without a closing brace, error out.
	    if (!*data) {
		    Com_Error(ErrorType::Drop, "%s: EOF without closing brace", __func__);
		    return false;
	    }

	    // Ensure we had a value.
	    if (value[0] == '}') {
		    Com_Error(ErrorType::Drop, "%s: closing brace without value for key %s", __func__, key);
		    return false;
	    }

	    // We successfully managed to parse this entity.
	    parsedSuccessfully = true;

	    // keynames with a leading underscore are used for utility comments,
	    // and are immediately discarded by quake
	    if (key[0] == '_') {
		    continue;
	    }

	    // Insert the key/value into the dictionary.
	    Com_DPrint("Parsed client entity, key='%s', value='%s'\n", key, value);
	    if (clEntity) {
            clEntity->entityDictionary.try_emplace(std::string(key),std::string(value));// = value;
        }
    }

    // If we failed to parse the entity properly, zero this one back out.
    if (!parsedSuccessfully) {
        int32_t clientEntityNumber = clEntity->clientEntityNumber;

	    *clEntity = {.clientEntityNumber = clientEntityNumber };
	    return false;
    }

    // Return the result.
    return parsedSuccessfully;
}

/**
*   @brief  Allocates the class entity determined by the classname key, and
*           then does a precache before spawning the class entity.
**/
qboolean ClientGameEntities::SpawnParsedClassEntity(ClientEntity* clEntity) {
    // Acquire dictionary.
    auto &dictionary = clEntity->entityDictionary;

    // Get client side entity number.
    int32_t stateNumber = clEntity->clientEntityNumber;

    // If it does not have a classname key we're in for trouble.
    if (!clEntity->entityDictionary.contains("classname")) {
	    // Error out.
	    Com_EPrint("%s: Can't spawn parsed server entity #%i due to a missing classname key.\n");
		
	    // Failed.
	    return false;
    }

    // Actually spawn the class entity.
    CLGBaseEntity *classEntity = clEntity->classEntity = AllocateClassEntity(clEntity, clEntity->entityDictionary["classname"]);

    // Something went wrong with allocating the class entity.
    if (!classEntity) {
	    // Be sure to free it.
        *clEntity = { .clientEntityNumber = stateNumber };//FreeClientEntity(clEntity);

	    // Failed.
	    Com_DPrint("Warning: Spawning entity(%s) failed.\n", clEntity->entityDictionary["classname"].c_str());
        return false;
    }

    // Initialise the entity with its respected keyvalue properties
    for (const auto& keyValueEntry : clEntity->entityDictionary) {
	    classEntity->SpawnKey(keyValueEntry.first, keyValueEntry.second);
    }

    // Precache and spawn the entity.
    classEntity->Precache();
    classEntity->Spawn();

    // Success.
    return true;
}

/**
*	@brief	Seeks through the type info system for a class registered under the classname string.
*			When found, it'll check whether it is allowed to be spawned as a map entity. If it is,
*			try and allocate it.
*	@return	CLGBaseEntity object in case of failure, a valid pointer to a class entity otherwise.
**/
CLGBaseEntity *ClientGameEntities::AllocateClassEntity(ClientEntity* clEntity, const std::string &classname) {
    // Start with a nice nullptr.
    CLGBaseEntity* spawnEntity = nullptr;

	// Safety check.
    if (!clEntity) {
		return nullptr;
    }

    // Get entity state number.
    int32_t stateNumber = clEntity->clientEntityNumber;

	// Warn if a slot is already occupied.
    if (entityList.GetByStateNumber(stateNumber) != nullptr) {
		// Warn.
		Com_DPrint("Warning: trying to allocate class entity '%s' the slot #%i was pre-occupied.\n", classname.c_str(), stateNumber);

		// Return nullptr.
		return nullptr;
    }

    // New type info-based spawning system, to replace endless string comparisons
    // First find it by the map name
    TypeInfo* info = TypeInfo::GetInfoByMapName(classname.c_str());
    if (info == nullptr) {
		// Then try finding it by the C++ class name
		if ((info = TypeInfo::GetInfoByName(classname.c_str())) == nullptr) {
			// Warn.
		    Com_DPrint("Warning: unknown entity '%s'\n", classname.c_str());

			// Bail out, we didn't find one.
			return nullptr;
		}
    }

    // Don't freak out if the entity cannot be allocated, but do warn us about it, it's good to know.
    // Entity classes with 'DefineDummyMapClass' won't be reported here.
    if (info->AllocateInstance != nullptr && info->IsMapSpawnable()) {
		// Allocate and return out new class entity.
		return entityList.InsertAtSlotNumber(info->AllocateInstance(clEntity), stateNumber);
    } else {
		// Check and warn about what went wrong.
		if (info->IsAbstract()) {
			Com_DPrint("Warning: tried to allocate an abstract class '%s'\n", info->classname);
		} else if (!info->IsMapSpawnable()) {
		    Com_DPrint("Warning: tried to allocate a code-only class '%s'\n", info->classname);
		}
    }

	// If we get to this point, we've triggered one warning either way.
	return nullptr;
}

//---------------------------------------------------------------------------------------

/**
*   @brief  Emplaces, or spawn anew, an entity from the entity state.
**/
qboolean ClientGameEntities::SpawnFromState(ClientEntity *clEntity, const EntityState& state) {
    // Sanity check. Even though it shouldn't have reached this point of execution if the entity was nullptr.
    if (!clEntity) {
        // Developer warning.
        Com_DPrint("Warning: ClientGameEntities::SpawnFromState called with a nullptr(clEntity)!!\n");

        return false;
    }

    // DEVELOPER
    if (clEntity->clientEntityNumber != state.number) {
        //Com_DPrint("CAUTION: clEntity->id=%i, state->number=%i\n", clEntity->id, state.number);
    }
    //// DEV

    // See whether we already have a class entity for this id.
    CLGBaseEntity *clgEntity = entityList.GetByStateNumber(state.number);
    
    // If we don't, create one.
    if (!clgEntity) {
        // Spawn one from current state.
        clgEntity = entityList.SpawnEntityFromState<CLGBaseEntity>(clEntity, state);
    } else {
        // If we did find it, compare its hashed classname, when not equal 
        // we'll reallocate the appropriate client game entity class.
        if (clgEntity->GetHashedClassname() != state.hashedClassname) {
            // Look up the actual class that does match for the hashed classname.
            // classregistry.lookupbyhash(hashedClassname).
            uint32_t hashed
            // When found, allocate it. If not found, allocate a CLGBaseEntity*
        }
    }

    // Now we've got the actual entity we're good to go.

    Com_DPrint("SpawnFromState: clEntity->id=%i, state->number=%i\n", clEntity->clientEntityNumber, state.number);

    return true;
}

/**
*   @brief Executed whenever an entity event is receieved.
**/
void ClientGameEntities::Event(int32_t number) {
    // Ensure entity number is in bounds.
    if (number < 0 || number > MAX_ENTITIES) {
        Com_WPrint("ClientGameEntities::Event caught an OOB Entity ID: %i\n", number);
        return;
    }

    // Fetch the client entity.
    ClientEntity* clientEntity = &cs->entities[number];

    // EF_TELEPORTER acts like an event, but is not cleared each frame
    if ((clientEntity->current.effects & EntityEffectType::Teleporter)) {
        ParticleEffects::Teleporter(clientEntity->current.origin);
    }

    // Switch to specific execution based on a unique Event ID.
    switch (clientEntity->current.eventID) {
        case EntityEvent::ItemRespawn:
            clgi.S_StartSound(NULL, number, SoundChannel::Weapon, clgi.S_RegisterSound("items/respawn1.wav"), 1, Attenuation::Idle, 0);
            ParticleEffects::ItemRespawn(clientEntity->current.origin);
            break;
        case EntityEvent::PlayerTeleport:
            clgi.S_StartSound(NULL, number, SoundChannel::Weapon, clgi.S_RegisterSound("misc/tele1.wav"), 1, Attenuation::Idle, 0);
            ParticleEffects::Teleporter(clientEntity->current.origin);
            break;
        case EntityEvent::Footstep:
            if (cl_footsteps->integer)
                clgi.S_StartSound(NULL, number, SoundChannel::Body, cl_sfx_footsteps[rand() & 3], 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::FallShort:
            clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("player/land1.wav"), 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::Fall:
            clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("*fall2.wav"), 1, Attenuation::Normal, 0);
            break;
        case EntityEvent::FallFar:
            clgi.S_StartSound(NULL, number, SoundChannel::Auto, clgi.S_RegisterSound("*fall1.wav"), 1, Attenuation::Normal, 0);
            break;
    }
}

/**
*   @brief  Parse the server frame for server entities to add to our client view.
*           Also applies special rendering effects to them where desired.
**/
void ClientGameEntities::AddPacketEntities() {
    // Render entity that is about to be passed to the current render frame.
    r_entity_t   renderEntity = {}; // Ensure it is clear aka set to 0.
    // State of the current entity.
    EntityState* entityState = nullptr;
    // Current processing client entity ptr.
    ClientEntity* clientEntity = nullptr;
    // Client Info.
    ClientInfo*  clientInfo = nullptr;
    // Entity specific effects. (Such as whether to rotate or not.)
    uint32_t effects = 0;
    // Entity render effects. (Shells and the like.)
    uint32_t renderEffects = 0;
    // Bonus items rotate at a fixed rate
    float autoRotate = AngleMod(cl->time * BASE_1_FRAMETIME);
    // Brush models can auto animate their frames
    int32_t autoAnimation = BASE_FRAMERATE * cl->time / BASE_FRAMETIME_1000;

    // Iterate from 0 till the amount of entities present in the current frame.
    for (int32_t pointerNumber = 0; pointerNumber < cl->frame.numEntities; pointerNumber++) {
        // C++20: Had to be placed here because of label skip.
        int32_t baseEntityFlags = 0;

        //
        // Fetch Entity.
        // 
        // Fetch the entity index.
        int32_t entityIndex = (cl->frame.firstEntity + pointerNumber) & PARSE_ENTITIES_MASK;
        // Fetch the state of the given entity index.
        entityState = &cl->entityStates[entityIndex];
        // Fetch the actual entity to process based on the entity's state index number.
        clientEntity = &cs->entities[entityState->number];
        // Setup the render entity ID for the renderer.
        renderEntity.id = clientEntity->clientEntityNumber + RESERVED_ENTITIY_COUNT;
//
//
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//          class entities test code.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Com_DPrint("entityIndex=%i, entityState->number=%i, clientEntity->id=%i, pointernr=%i\n", entityIndex, entityState->number, clientEntity->id, pointerNumber);

        // Loop through class entities, and see if their IDs still match 
        //classEntities.UpdateFrame();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 
// 
        //
        // Effects.
        // 
        // Fetch the effects of current entity.
        effects = entityState->effects;
        // Fetch the render effects of current entity.
        renderEffects = entityState->renderEffects;

        //
        // Frame Animation Effects.
        //
        if (effects & EntityEffectType::AnimCycleFrames01hz2)
            renderEntity.frame = autoAnimation & 1;
        else if (effects & EntityEffectType::AnimCycleFrames23hz2)
            renderEntity.frame = 2 + (autoAnimation & 1);
        else if (effects & EntityEffectType::AnimCycleAll2hz)
            renderEntity.frame = autoAnimation;
        else if (effects & EntityEffectType::AnimCycleAll30hz)
            renderEntity.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
	    else {
    	    //// Fetch the iqm animation index data.
         //   if (clientEntity->current.animationIndex != 0 && clientEntity->current.modelIndex != 0) {
		       // model_t* iqmData = clgi.MOD_ForHandle(clientEntity->current.modelIndex);

         //       if (iqmData) {
			      //  Com_DPrint("WOW!!!\n");
         //       }
         //   } else {
		        //renderEntity.frame = entityState->animationFrame;
		 //       renderEntity.oldframe = clientEntity->prev.animationFrame;
		//        renderEntity.backlerp = 1.0 - cl->lerpFraction;
//            }

 
            //clientEntity->current.animationFrame, 
	 //   framefrac = GS_FrameForTime(&curframe, cg.time, viewweapon->baseAnimStartTime,  // start time
		//weaponInfo->frametime[viewweapon->baseAnim],				    // current frame time?
		//weaponInfo->firstframe[viewweapon->baseAnim],				    // first frame.
		//weaponInfo->lastframe[viewweapon->baseAnim],				    // last frame.
		//weaponInfo->loopingframes[viewweapon->baseAnim],			    // looping frames.
		//true); 
        }
        

        // Optionally remove the glowing effect.
        if (cl_noglow->integer)
            renderEffects &= ~RenderEffects::Glow;

        // Setup the proper lerp and model frame to render this pass.
        // Moved into the if statement's else case up above.
        renderEntity.oldframe = clientEntity->prev.animationFrame;
        renderEntity.backlerp = 1.0 - SG_FrameForTime(&renderEntity.frame,
            cl->serverTime,                                     // Current Time.
            clientEntity->current.animationStartTime,           // Animation Start time. (TODO: This needs to changed to a stored cl->time of the moment where the animation event got through.)
            clientEntity->current.animationFramerate,           // Current frame time.
            clientEntity->current.animationStartFrame,          // Start frame.
            clientEntity->current.animationEndFrame,            // End frame.
            0,                                                  // Loop count.
            true                                                // Force loop
        );
        clientEntity->current.animationFrame = renderEntity.frame;
    //clientEntity->prev.animationFrame = clientEntity->current.animationFrame;

        //
        // Setup renderEntity origin.
        //
        if (renderEffects & RenderEffects::FrameLerp) {
            // Step origin discretely, because the model frames do the animation properly.
            renderEntity.origin = clientEntity->current.origin;
            renderEntity.oldorigin = clientEntity->current.oldOrigin;
        } else if (renderEffects & RenderEffects::Beam) {
            // Interpolate start and end points for beams
            renderEntity.origin = vec3_mix(clientEntity->prev.origin, clientEntity->current.origin, cl->lerpFraction);
            renderEntity.oldorigin = vec3_mix(clientEntity->prev.oldOrigin, clientEntity->current.oldOrigin, cl->lerpFraction);
        } else {
            if (entityState->number == cl->frame.clientNumber + 1) {
                // In case of this being our actual client entity, we use the predicted origin.
                renderEntity.origin = cl->playerEntityOrigin;
                renderEntity.oldorigin = cl->playerEntityOrigin;
            } else {
                // Ohterwise, just neatly interpolate the origin.
                renderEntity.origin = vec3_mix(clientEntity->prev.origin, clientEntity->current.origin, cl->lerpFraction);
                // Neatly copy it as the renderEntity's oldorigin.
                renderEntity.oldorigin = renderEntity.origin;
            }
        }

	    // Draw debug bounding box for client entity.
	    if (renderEffects & RenderEffects::DebugBoundingBox) {
	        CLG_DrawDebugBoundingBox(clientEntity->lerpOrigin, clientEntity->mins, clientEntity->maxs);
	    }

        // tweak the color of beams
        if (renderEffects & RenderEffects::Beam) {
            // The four beam colors are encoded in 32 bits of skinNumber (hack)
            renderEntity.alpha = 0.30;
            renderEntity.skinNumber = (entityState->skinNumber >> ((rand() % 4) * 8)) & 0xff;
            renderEntity.model = 0;
        } else {
            //
            // Set the entity model skin
            //
            if (entityState->modelIndex == 255) {
                // Use a custom player skin
                clientInfo = &cl->clientInfo[entityState->skinNumber & 255];
                renderEntity.skinNumber = 0;
                renderEntity.skin = clientInfo->skin;
                renderEntity.model = clientInfo->model;

                // Setup default base client info in case of 0.
                if (!renderEntity.skin || !renderEntity.model) {
                    renderEntity.skin = cl->baseClientInfo.skin;
                    renderEntity.model = cl->baseClientInfo.model;
                    clientInfo = &cl->baseClientInfo;
                }

                // Special Disguise render effect handling.
                if (renderEffects & RenderEffects::UseDisguise) {
                    char buffer[MAX_QPATH];

                    Q_concat(buffer, sizeof(buffer), "players/", clientInfo->model_name, "/disguise.pcx", NULL);
                    renderEntity.skin = clgi.R_RegisterSkin(buffer);
                }
            } else {
                // Default entity skin number handling behavior.
                renderEntity.skinNumber = entityState->skinNumber;
                renderEntity.skin = 0;
                renderEntity.model = cl->drawModels[entityState->modelIndex];

                // Disable shadows on lasers and dm spots.
                if (renderEntity.model == cl_mod_laser || renderEntity.model == cl_mod_dmspot)
                    renderEffects |= RF_NOSHADOW;
            }
        }

        // Only used for black hole model right now, FIXME: do better
        if ((renderEffects & RenderEffects::Translucent) && !(renderEffects & RenderEffects::Beam)) {
            renderEntity.alpha = 0.70;
        }

        // Render effects (fullbright, translucent, etc)
        if ((effects & EntityEffectType::ColorShell)) {
            renderEntity.flags = 0;  // Render effects go on color shell entity
        } else {
            renderEntity.flags = renderEffects;
        }

        //
        // Angles.
        //
        if (effects & EntityEffectType::Rotate) {
            // Autorotate for bonus item entities.
            renderEntity.angles[0] = 0;
            renderEntity.angles[1] = autoRotate;
            renderEntity.angles[2] = 0;
        } else if (entityState->number == cl->frame.clientNumber + 1) {
            // Predicted angles for client entities.
            renderEntity.angles = cl->playerEntityAngles;
        } else {
            // Otherwise, lerp angles by default.
            renderEntity.angles = vec3_mix(clientEntity->prev.angles, clientEntity->current.angles, cl->lerpFraction);

            // Mimic original ref_gl "leaning" bug (uuugly!)
            if (entityState->modelIndex == 255 && cl_rollhack->integer) {
                renderEntity.angles[vec3_t::Roll] = -renderEntity.angles[vec3_t::Roll];
            }
        }

        //
        // Entity Effects for in case the entity is the actual client.
        //
        if (entityState->number == cl->frame.clientNumber + 1) {
            if (!cl->thirdPersonView)
            {
                // Special case handling for RTX rendering. Only draw third person model from mirroring surfaces.
                if (vid_rtx->integer)
                    baseEntityFlags |= RenderEffects::ViewerModel;
                else
                    goto skip;
            }

            // Don't tilt the model - looks weird
            renderEntity.angles[0] = 0.f;

            //
            // TODO: This needs to be fixed properly for the shadow to render.
            // 
            // Offset the model back a bit to make the view point located in front of the head
            //constexpr float offset = -15.f;
            //constexpr float offset = 8.f;// 0.0f;
            //vec3_t angles = { 0.f, renderEntity.angles[1], 0.f };
            //vec3_t forward;
            //AngleVectors(angles, &forward, NULL, NULL);
            //renderEntity.origin = vec3_fmaf(renderEntity.origin, offset, forward);
            //renderEntity.oldorigin = vec3_fmaf(renderEntity.oldorigin, offset, forward);

            // Temporary fix, not quite perfect though. Add some z offset so the shadow isn't too dark under the feet.
            renderEntity.origin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
            renderEntity.oldorigin = cl->predictedState.viewOrigin + vec3_t{0.f, 0.f, 4.f};
        }

        // If set to invisible, skip
        if (!entityState->modelIndex) {
            goto skip;
        }

        // Add the baseEntityFlags to the renderEntity flags.
        renderEntity.flags |= baseEntityFlags;

        // In rtx mode, the base entity has the renderEffects for shells
        if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
            renderEffects = ApplyRenderEffects(renderEffects);
            renderEntity.flags |= renderEffects;
        }

        // Last but not least, add the entity to the refresh render list.
        clge->view->AddRenderEntity(&renderEntity);

        // Keeping it here commented to serve as an example case.
        // Add dlights for flares
        //model_t* model;
        //if (renderEntity.model && !(renderEntity.model & 0x80000000) &&
        //    (model = clgi.MOD_ForHandle(renderEntity.model)))
        //{
        //    if (model->model_class == MCLASS_FLARE)
        //    {
        //        float phase = (float)cl->time * 0.03f + (float)renderEntity.id;
        //        float anim = sinf(phase);

        //        float offset = anim * 1.5f + 5.f;
        //        float brightness = anim * 0.2f + 0.8f;

        //        vec3_t origin;
        //        VectorCopy(renderEntity.origin, origin);
        //        origin[2] += offset;

        //        V_AddLightEx(origin, 500.f, 1.6f * brightness, 1.0f * brightness, 0.2f * brightness, 5.f);
        //    }
        //}

        // For color shells we generate a separate entity for the main model.
        // (Use the settings of the already rendered model, and apply translucency to it.
        if ((effects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
            renderEffects = ApplyRenderEffects(renderEffects);
            renderEntity.flags = renderEffects | baseEntityFlags | RenderEffects::Translucent;
            renderEntity.alpha = 0.30;
            clge->view->AddRenderEntity(&renderEntity);
        }

        renderEntity.skin = 0;       // never use a custom skin on others
        renderEntity.skinNumber = 0;
        renderEntity.flags = baseEntityFlags;
        renderEntity.alpha = 0;

        //
        // ModelIndex2
        // 
        // Add an entity to the current rendering frame that has model index 2 attached to it.
        // Duplicate for linked models
        if (entityState->modelIndex2) {
            if (entityState->modelIndex2 == 255) {
                // Custom weapon
                clientInfo = &cl->clientInfo[entityState->skinNumber & 0xff];
                
                // Determine skinIndex.
                int32_t skinIndex = (entityState->skinNumber >> 8); // 0 is default weapon model
                if (skinIndex < 0 || skinIndex > cl->numWeaponModels - 1) {
                    skinIndex = 0;
                }

                // Fetch weapon model.
                renderEntity.model = clientInfo->weaponmodel[skinIndex];

                // If invalid, use defaults.
                if (!renderEntity.model) {
                    if (skinIndex != 0) {
                        renderEntity.model = clientInfo->weaponmodel[0];
                    }
                    if (!renderEntity.model) {
                        renderEntity.model = cl->baseClientInfo.weaponmodel[0];
                    }
                }
            } else {
                renderEntity.model = cl->drawModels[entityState->modelIndex2];
            }


            if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
                renderEntity.flags |= renderEffects;
            }

            clge->view->AddRenderEntity(&renderEntity);

            //PGM - make sure these get reset.
            renderEntity.flags = baseEntityFlags;
            renderEntity.alpha = 0;
        }

        //
        // ModelIndex3
        // 
        // Add an entity to the current rendering frame that has model index 3 attached to it.
        if (entityState->modelIndex3) {
            renderEntity.model = cl->drawModels[entityState->modelIndex3];
            clge->view->AddRenderEntity(&renderEntity);
        }

        //
        // ModelIndex4
        // 
        // Add an entity to the current rendering frame that has model index 4 attached to it.
        if (entityState->modelIndex4) {
            renderEntity.model = cl->drawModels[entityState->modelIndex4];
            clge->view->AddRenderEntity(&renderEntity);
        }

        //
        // Particle Trail Effects.
        // 
        // Add automatic particle trail effects where desired.
        if (effects & ~EntityEffectType::Rotate) {
            if (effects & EntityEffectType::Gib) {
                ParticleEffects::DiminishingTrail(clientEntity->lerpOrigin, renderEntity.origin, clientEntity, effects);
            } else if (effects & EntityEffectType::Torch) {
                const float anim = sinf((float)renderEntity.id + ((float)cl->time / 60.f + frand() * 3.3)) / (3.14356 - (frand() / 3.14356));
                const float offset = anim * 0.0f;
                const float brightness = anim * 1.2f + 1.6f;
                const vec3_t origin = { 
                    renderEntity.origin.x,
                    renderEntity.origin.y,
                    renderEntity.origin.z + offset 
                };

                clge->view->AddLight(origin, vec3_t{ 1.0f * brightness, 0.425f * brightness, 0.1f * brightness }, 25.f, 3.6f);
            }
        }

    skip:
        // Assign renderEntity origin to clientEntity lerp origin in the case of a skip.
        clientEntity->lerpOrigin = renderEntity.origin;
    }
}

/**
* Add the view weapon render entity to the screen. Can also be used for
* other scenarios where a depth hack is required.
**/
static uint32_t animStart = 0;
r_entity_t gunRenderEntity;
void ClientGameEntities::AddViewEntities() {
    int32_t  shellFlags = 0;

    // Hidden in bsp menu mode.
    if (info_in_bspmenu->integer) {
        return;
    }

    // No need to render the gunRenderEntity in this case.
    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED) {
        return;
    }

    // Neither in this case.
    if (info_hand->integer == 2) {
        return;
    }

    // Find states to between frames to interpolate between.
    PlayerState *currentPlayerState = &cl->frame.playerState;
    PlayerState *oldPlayerState= &cl->oldframe.playerState;

    // Gun ViewModel.
    //if (gunRenderEntity.model != cl->drawModels[currentPlayerState->gunIndex]) {
    //    gunRenderEntity.frame = 1;
    //    gunRenderEntity.oldframe = 0;
    //}
    uint32_t lastModel = gunRenderEntity.model;
    gunRenderEntity.model = (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0);//(gun_model ? gun_model : (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0));


    // This is very ugly right now, but it'll prevent the wrong frame from popping in-screen...
    if (oldPlayerState->gunAnimationStartTime != currentPlayerState->gunAnimationStartTime) {
        animStart = cl->time;
                gunRenderEntity.frame = currentPlayerState->gunAnimationStartFrame;
        gunRenderEntity.oldframe = currentPlayerState->gunAnimationEndFrame;
    }
    //if (lastModel != gunRenderEntity.model) {
    //    gunRenderEntity.frame = currentPlayerState->gunAnimationStartFrame;
    //    gunRenderEntity.oldframe = currentPlayerState->gunAnimationEndFrame;
    //}

    gunRenderEntity.id = RESERVED_ENTITIY_GUN;

    // If there is no model to render, there is no need to continue.
    if (!gunRenderEntity.model) {
        return;
    }

    // Set up gunRenderEntity position
    for (int32_t i = 0; i < 3; i++) {
        gunRenderEntity.origin[i] = cl->refdef.vieworg[i] + oldPlayerState->gunOffset[i] +
            cl->lerpFraction * (currentPlayerState->gunOffset[i] - oldPlayerState->gunOffset[i]);
        gunRenderEntity.angles = cl->refdef.viewAngles + vec3_mix_euler(oldPlayerState->gunAngles,
                                                            currentPlayerState->gunAngles, cl->lerpFraction);
    }

    // Adjust for high fov.
    if (currentPlayerState->fov > 90) {
        vec_t ofs = (90 - currentPlayerState->fov) * 0.2f;
        gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, ofs, cl->v_forward);
    }

    // Adjust the gunRenderEntity origin so that the gunRenderEntity doesn't intersect with walls
    {
        vec3_t view_dir, right_dir, up_dir;
        vec3_t gun_real_pos, gun_tip;
        constexpr float gun_length = 56.f;
        constexpr float gun_right = 10.f;
        constexpr float gun_up = -5.f;
        static vec3_t mins = { -4, -2, -12 }, maxs = { 4, 8, 12 };

        AngleVectors(cl->refdef.viewAngles, &view_dir, &right_dir, &up_dir);
        gun_real_pos = vec3_fmaf(gunRenderEntity.origin, gun_right, right_dir);
        gun_real_pos = vec3_fmaf(gun_real_pos, gun_up, up_dir);
        gun_tip = vec3_fmaf(gun_real_pos, gun_length, view_dir);

        // Execute the trace for the view model weapon.

        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        // Add mask support and perhaps a skip...
        TraceResult trace = clgi.Trace(gun_real_pos, mins, maxs, gun_tip, nullptr, BrushContentsMask::PlayerSolid); 

        // In case the trace hit anything, adjust our view model position so it doesn't stick in a wall.
        if (trace.fraction != 1.0f || trace.ent != nullptr)
        {
            gunRenderEntity.origin = vec3_fmaf(trace.endPosition, -gun_length, view_dir);
            gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, -gun_right, right_dir);
            gunRenderEntity.origin = vec3_fmaf(gunRenderEntity.origin, -gun_up, up_dir);
        }
    }

    // Do not lerp the origin at all.
    gunRenderEntity.oldorigin = gunRenderEntity.origin;

    //if (gun_frame) {
    //    gunRenderEntity.frame = gun_frame;      // Development tool
    //    gunRenderEntity.oldframe = gun_frame;   // Development tool
    //} else {
        // Setup the proper lerp and model frame to render this pass.
        // Moved into the if statement's else case up above.
        gunRenderEntity.oldframe = gunRenderEntity.frame;
        gunRenderEntity.backlerp = 1.0 - SG_FrameForTime(&gunRenderEntity.frame,
            cl->time, // Current Time.
            animStart,  // Animation Start time.
            currentPlayerState->gunAnimationFrametime,  // Current frame time.
            currentPlayerState->gunAnimationStartFrame, // Start frame.
            currentPlayerState->gunAnimationEndFrame,   // End frame.
            currentPlayerState->gunAnimationLoopCount,  // Loop count.
            currentPlayerState->gunAnimationForceLoop
        );

        // Don't allow it to go below 0, instead set it to old frame.
        if (gunRenderEntity.frame < 0) {
            gunRenderEntity.frame = gunRenderEntity.oldframe;
        }
        //gunRenderEntity.frame = tionFrame = renderEntity.frame;
        //gunRenderEntity.frame = 0;//currentPlayerState->gunAnimationFrame;
        //if (gunRenderEntity.frame == 0) {
        //    gunRenderEntity.oldframe = 0;   // just changed weapons, don't lerp from old
        //} else {
        //    gunRenderEntity.oldframe = 0;//oldPlayerState->gunAnimationFrame;
        //    gunRenderEntity.backlerp = 1.0f - cl->lerpFraction;
        //}
    //}

    // Setup basic render entity flags for our view weapon.
    gunRenderEntity.flags = RenderEffects::MinimalLight | RenderEffects::DepthHack | RenderEffects::WeaponModel;
    if (info_hand->integer == 1) {
        gunRenderEntity.flags |= RF_LEFTHAND;
    }

    // Apply translucency render effect to the render entity and clamp its alpha value if nescessary.
    if (cl_gunalpha->value != 1) {
        gunRenderEntity.alpha = clgi.Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
        gunRenderEntity.flags |= RenderEffects::Translucent;
    }

    // Apply shell effects to the same entity in rtx mode.
    if (vid_rtx->integer) {
        gunRenderEntity.flags |= shellFlags;
    }

    // Add the gun render entity to the current render frame.
    clge->view->AddRenderEntity(&gunRenderEntity);

    // Render a separate shell entity in non-rtx mode.
    if (shellFlags && !vid_rtx->integer) {
        gunRenderEntity.alpha = 0.30f * cl_gunalpha->value;
        gunRenderEntity.flags |= shellFlags | RenderEffects::Translucent;
        clge->view->AddRenderEntity(&gunRenderEntity);
    }
}

//---------------
// ClientGameEntities::ApplyRenderEffects
//
//---------------
int32_t ClientGameEntities::ApplyRenderEffects(int32_t renderEffects) {

    //
    // NOTE: The commented code below is left here as an example
    // of what to do with this function.
    //

    //    // all of the solo colors are fine.  we need to catch any of the combinations that look bad
    //    // (double & half) and turn them into the appropriate color, and make double/quad something special
    //    if (renderEffects & RenderEffects::HalfDamShell) {
    //        // ditch the half damage shell if any of red, blue, or double are on
    //        if (renderEffects & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::DoubleShell))
    //            renderEffects &= ~RenderEffects::HalfDamShell;
    //    }
    //    if (renderEffects & RenderEffects::DoubleShell) {
    //        // lose the yellow shell if we have a red, blue, or green shell
    //        if (renderEffects & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::GreenShell))
    //            renderEffects &= ~RenderEffects::DoubleShell;
    //        // if we have a red shell, turn it to purple by adding blue
    //        if (renderEffects & RenderEffects::RedShell)
    //            renderEffects |= RenderEffects::BlueShell;
    //        // if we have a blue shell (and not a red shell), turn it to cyan by adding green
    //        else if (renderEffects & RenderEffects::BlueShell) {
    //            // go to green if it's on already, otherwise do cyan (flash green)
    //            if (renderEffects & RenderEffects::GreenShell)
    //                renderEffects &= ~RenderEffects::BlueShell;
    //            else
    //                renderEffects |= RenderEffects::GreenShell;
    //        }
    //    }

    // Just return the renderEffects as they were.
    return renderEffects;
}