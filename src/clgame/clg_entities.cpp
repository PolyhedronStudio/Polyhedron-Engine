// LICENSE HERE.

//
// clg_entities.c
//
//
// Takes care of entity management.
//
#include "clg_local.h"
#include "clg_effects.h"
#include "clg_entities.h"
#include "clg_view.h"

extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;
extern qhandle_t cl_sfx_footsteps[4];

//
//==========================================================================
//
// ENTITY EVENTS
//
//==========================================================================
//



//
//==========================================================================
//
// INTERPOLATE BETWEEN FRAMES TO GET RENDERING PARMS
//
//==========================================================================
//

// Use a static entity ID on some things because the renderer relies on eid to match between meshes
// on the current and previous frames.
#define RESERVED_ENTITIY_GUN 1
#define RESERVED_ENTITIY_SHADERBALLS 2
#define RESERVED_ENTITIY_COUNT 3

static int adjust_shell_fx(int renderfx)
{
    //// PMM - at this point, all of the shells have been handled
    //// if we're in the rogue pack, set up the custom mixing, otherwise just
    //// keep going
    //if (!strcmp(fs_game->string, "rogue")) {
    //    // all of the solo colors are fine.  we need to catch any of the combinations that look bad
    //    // (double & half) and turn them into the appropriate color, and make double/quad something special
    //    if (renderfx & RenderEffects::HalfDamShell) {
    //        // ditch the half damage shell if any of red, blue, or double are on
    //        if (renderfx & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::DoubleShell))
    //            renderfx &= ~RenderEffects::HalfDamShell;
    //    }

    //    if (renderfx & RenderEffects::DoubleShell) {
    //        // lose the yellow shell if we have a red, blue, or green shell
    //        if (renderfx & (RenderEffects::RedShell | RenderEffects::BlueShell | RenderEffects::GreenShell))
    //            renderfx &= ~RenderEffects::DoubleShell;
    //        // if we have a red shell, turn it to purple by adding blue
    //        if (renderfx & RenderEffects::RedShell)
    //            renderfx |= RenderEffects::BlueShell;
    //        // if we have a blue shell (and not a red shell), turn it to cyan by adding green
    //        else if (renderfx & RenderEffects::BlueShell) {
    //            // go to green if it's on already, otherwise do cyan (flash green)
    //            if (renderfx & RenderEffects::GreenShell)
    //                renderfx &= ~RenderEffects::BlueShell;
    //            else
    //                renderfx |= RenderEffects::GreenShell;
    //        }
    //    }
    //}

    return renderfx;
}

//
//===============
// CLG_AddPacketEntities
// 
// Parse the server frames, and add the server entities to our client view.
// Also apply special effects to them where desired.
//===============
//
void CLG_AddPacketEntities(void)
{
    r_entity_t            ent;
    EntityState* s1;
    float               autorotate;
    int                 i;
    int                 pnum;
    cl_entity_t* cent;
    int                 autoanim;
    ClientInfo* ci;
    unsigned int        effects, renderfx;

    // bonus items rotate at a fixed rate
    autorotate = anglemod(cl->time * 0.1f);

    // brush models can auto animate their frames
    autoanim = 20 * cl->time / 1000;

    memset(&ent, 0, sizeof(ent));

    for (pnum = 0; pnum < cl->frame.numEntities; pnum++) {
        // C++20: Had to be placed here because of label skip.
        int base_entity_flags = 0;

        i = (cl->frame.firstEntity + pnum) & PARSE_ENTITIES_MASK;
        s1 = &cl->entityStates[i];

        cent = &cs->entities[s1->number];
        ent.id = cent->id + RESERVED_ENTITIY_COUNT;

        effects = s1->effects;
        renderfx = s1->renderfx;

        //
        // Frame Animation Effects.
        //
        if (effects & EntityEffectType::AnimCycleFrames01hz2)
            ent.frame = autoanim & 1;
        else if (effects & EntityEffectType::AnimCycleFrames23hz2)
            ent.frame = 2 + (autoanim & 1);
        else if (effects & EntityEffectType::AnimCycleAll2hz)
            ent.frame = autoanim;
        else if (effects & EntityEffectType::AnimCycleAll30hz)
            ent.frame = (cl->time / 33.33f); // 30 fps ( /50 would be 20 fps, etc. )
        else
            ent.frame = s1->frame;

        // optionally remove the glowing effect
        if (cl_noglow->integer)
            renderfx &= ~RenderEffects::Glow;

        ent.oldframe = cent->prev.frame;
        ent.backlerp = 1.0 - cl->lerpFraction;

        if (renderfx & RenderEffects::FrameLerp) {
            // step origin discretely, because the frames
            // do the animation properly
            VectorCopy(cent->current.origin, ent.origin);
            VectorCopy(cent->current.oldOrigin, ent.oldorigin);  // FIXME
        }
        else if (renderfx & RenderEffects::Beam) {
            // interpolate start and end points for beams
            LerpVector(cent->prev.origin, cent->current.origin,
                cl->lerpFraction, ent.origin);
            LerpVector(cent->prev.oldOrigin, cent->current.oldOrigin,
                cl->lerpFraction, ent.oldorigin);
        }
        else {
            if (s1->number == cl->frame.clientNumber + 1) {
                // use predicted origin
                VectorCopy(cl->playerEntityOrigin, ent.origin);
                VectorCopy(cl->playerEntityOrigin, ent.oldorigin);
            }
            else {
                // interpolate origin
                LerpVector(cent->prev.origin, cent->current.origin,
                    cl->lerpFraction, ent.origin);
                VectorCopy(ent.origin, ent.oldorigin);
            }
        }

        // create a new entity

        // tweak the color of beams
        if (renderfx & RenderEffects::Beam) {
            // the four beam colors are encoded in 32 bits of skinNumber (hack)
            ent.alpha = 0.30;
            ent.skinNumber = (s1->skinNumber >> ((rand() % 4) * 8)) & 0xff;
            ent.model = 0;
        } else {
            // set skin
            if (s1->modelIndex == 255) {
                // use custom player skin
                ent.skinNumber = 0;
                ci = &cl->clientInfo[s1->skinNumber & 0xff];
                ent.skin = ci->skin;
                ent.model = ci->model;
                if (!ent.skin || !ent.model) {
                    ent.skin = cl->baseClientInfo.skin;
                    ent.model = cl->baseClientInfo.model;
                    ci = &cl->baseClientInfo;
                }
                if (renderfx & RenderEffects::UseDisguise) {
                    char buffer[MAX_QPATH];

                    Q_concat(buffer, sizeof(buffer), "players/", ci->model_name, "/disguise.pcx", NULL);
                    ent.skin = clgi.R_RegisterSkin(buffer);
                }
            }
            else {
                ent.skinNumber = s1->skinNumber;
                ent.skin = 0;
                ent.model = cl->drawModels[s1->modelIndex];
                if (ent.model == cl_mod_laser || ent.model == cl_mod_dmspot)
                    renderfx |= RF_NOSHADOW;
            }
        }

        // only used for black hole model right now, FIXME: do better
        if ((renderfx & RenderEffects::Translucent) && !(renderfx & RenderEffects::Beam))
            ent.alpha = 0.70;

        // render effects (fullbright, translucent, etc)
        if ((effects & EntityEffectType::ColorShell))
            ent.flags = 0;  // renderfx go on color shell entity
        else
            ent.flags = renderfx;

        // calculate angles
        if (effects & EntityEffectType::Rotate) {  // some bonus items auto-rotate
            ent.angles[0] = 0;
            ent.angles[1] = autorotate;
            ent.angles[2] = 0;
        } else if (s1->number == cl->frame.clientNumber + 1) {
            VectorCopy(cl->playerEntityAngles, ent.angles);      // use predicted angles
        } else { // interpolate angles
            LerpAngles(cent->prev.angles, cent->current.angles,
                cl->lerpFraction, ent.angles);

            // mimic original ref_gl "leaning" bug (uuugly!)
            if (s1->modelIndex == 255 && cl_rollhack->integer) {
                ent.angles[vec3_t::Roll] = -ent.angles[vec3_t::Roll];
            }
        }

        // Entity Effects for in case the entity is the actual client.
        if (s1->number == cl->frame.clientNumber + 1) {
            if (!cl->thirdPersonView)
            {
                if (vid_rtx->integer)
                    base_entity_flags |= RenderEffects::ViewerModel;    // only draw from mirrors
                else
                    goto skip;
            }

            // don't tilt the model - looks weird
            ent.angles[0] = 0.f;

            // offset the model back a bit to make the view point located in front of the head
            vec3_t angles = { 0.f, ent.angles[1], 0.f };
            vec3_t forward;
            AngleVectors(angles, &forward, NULL, NULL);

            float offset = -15.f;
            VectorMA(ent.origin, offset, forward, ent.origin);
            VectorMA(ent.oldorigin, offset, forward, ent.oldorigin);
        }

        // if set to invisible, skip
        if (!s1->modelIndex) {
            goto skip;
        }

        ent.flags |= base_entity_flags;

        // in rtx mode, the base entity has the renderfx for shells
        if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
            renderfx = adjust_shell_fx(renderfx);
            ent.flags |= renderfx;
        }

        // add to refresh list
        V_AddEntity(&ent);

        // add dlights for flares
        model_t* model;
        if (ent.model && !(ent.model & 0x80000000) &&
            (model = clgi.MOD_ForHandle(ent.model)))
        {
            if (model->model_class == MCLASS_FLARE)
            {
                float phase = (float)cl->time * 0.03f + (float)ent.id;
                float anim = sinf(phase);

                float offset = anim * 1.5f + 5.f;
                float brightness = anim * 0.2f + 0.8f;

                vec3_t origin;
                VectorCopy(ent.origin, origin);
                origin[2] += offset;

                V_AddLightEx(origin, 500.f, 1.6f * brightness, 1.0f * brightness, 0.2f * brightness, 5.f);
            }
        }

        // color shells generate a separate entity for the main model
        if ((effects & EntityEffectType::ColorShell) && !vid_rtx->integer) {
            renderfx = adjust_shell_fx(renderfx);
            ent.flags = renderfx | RenderEffects::Translucent | base_entity_flags;
            ent.alpha = 0.30;
            V_AddEntity(&ent);
        }

        ent.skin = 0;       // never use a custom skin on others
        ent.skinNumber = 0;
        ent.flags = base_entity_flags;
        ent.alpha = 0;

        // Add an entity to the current rendering frame that has model index 2 attached to it.
        // Duplicate for linked models
        if (s1->modelIndex2) {
            if (s1->modelIndex2 == 255) {
                // custom weapon
                ci = &cl->clientInfo[s1->skinNumber & 0xff];
                i = (s1->skinNumber >> 8); // 0 is default weapon model
                if (i < 0 || i > cl->numWeaponModels - 1)
                    i = 0;
                ent.model = ci->weaponmodel[i];
                if (!ent.model) {
                    if (i != 0)
                        ent.model = ci->weaponmodel[0];
                    if (!ent.model)
                        ent.model = cl->baseClientInfo.weaponmodel[0];
                }
            }
            else
                ent.model = cl->drawModels[s1->modelIndex2];

            // PMM - check for the defender sphere shell .. make it translucent
            if (!Q_strcasecmp(cl->configstrings[ConfigStrings::Models+ (s1->modelIndex2)], "models/items/shell/tris.md2")) {
                ent.alpha = 0.32;
                ent.flags = RenderEffects::Translucent;
            }

            if ((effects & EntityEffectType::ColorShell) && vid_rtx->integer) {
                ent.flags |= renderfx;
            }

            V_AddEntity(&ent);

            //PGM - make sure these get reset.
            ent.flags = base_entity_flags;
            ent.alpha = 0;
        }

        // Add an entity to the current rendering frame that has model index 3 attached to it.
        if (s1->modelIndex3) {
            ent.model = cl->drawModels[s1->modelIndex3];
            V_AddEntity(&ent);
        }

        // Add an entity to the current rendering frame that has model index 4 attached to it.
        if (s1->modelIndex4) {
            ent.model = cl->drawModels[s1->modelIndex4];
            V_AddEntity(&ent);
        }


        // Add automatic particle trail effects where desired.
        if (effects & ~EntityEffectType::Rotate) {
            if (effects & EntityEffectType::Blaster) {
                CLG_BlasterTrail(cent->lerpOrigin, ent.origin);
                V_AddLight(ent.origin, 200, 0.6f, 0.4f, 0.12f);
            } else if (effects & EntityEffectType::Gib) {
                CLG_DiminishingTrail(cent->lerpOrigin, ent.origin, cent, effects);
            }
        }

        //Com_DPrint("[NORMAL] entity ID =%i - origin = [%f, %f, %f]\n", ent.id, ent.origin[0], ent.origin[1], ent.origin[1]);
    skip:
        VectorCopy(ent.origin, cent->lerpOrigin);

        //Com_DPrint("[SKIP] entity ID =%i - origin = [%f, %f, %f]\n", ent.id, ent.origin[0], ent.origin[1], ent.origin[1]);
    }
}

/*
==============
CLG_AddViewWeapon
==============
*/
void CLG_AddViewWeapon(void)
{
    PlayerState* ps, * ops;
    r_entity_t    gun;        // view model
    int         i, shell_flags = 0;

    // allow the gun to be completely removed
    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED) {
        return;
    }

    if (info_hand->integer == 2) {
        return;
    }

    // find states to interpolate between
    ps = CL_KEYPS;
    ops = CL_OLDKEYPS;

    memset(&gun, 0, sizeof(gun));

    if (gun_model) {
        gun.model = gun_model;  // development tool
    }
    else {
        gun.model = cl->drawModels[ps->gunIndex];
    }
    if (!gun.model) {
        return;
    }

    gun.id = RESERVED_ENTITIY_GUN;

    // set up gun position
    for (i = 0; i < 3; i++) {
        gun.origin[i] = cl->refdef.vieworg[i] + ops->gunOffset[i] +
            CL_KEYLERPFRAC * (ps->gunOffset[i] - ops->gunOffset[i]);
        gun.angles[i] = cl->refdef.viewAngles[i] + LerpAngle(ops->gunAngles[i],
            ps->gunAngles[i], CL_KEYLERPFRAC);
    }

    // adjust for high fov
    if (ps->fov > 90) {
        vec_t ofs = (90 - ps->fov) * 0.2f;
        VectorMA(gun.origin, ofs, cl->v_forward, gun.origin);
    }

    // adjust the gun origin so that the gun doesn't intersect with walls
    {
        vec3_t view_dir, right_dir, up_dir;
        vec3_t gun_real_pos, gun_tip;
        const float gun_length = 28.f;
        const float gun_right = 10.f;
        const float gun_up = -5.f;
        trace_t trace;
        static vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

        AngleVectors(cl->refdef.viewAngles, &view_dir, &right_dir, &up_dir);
        VectorMA(gun.origin, gun_right, right_dir, gun_real_pos);
        VectorMA(gun_real_pos, gun_up, up_dir, gun_real_pos);
        VectorMA(gun_real_pos, gun_length, view_dir, gun_tip);

        clgi.CM_BoxTrace(&trace, gun_real_pos, gun_tip, mins, maxs, cl->bsp->nodes, CONTENTS_MASK_SOLID);

        if (trace.fraction != 1.0f)
        {
            VectorMA(trace.endPosition, -gun_length, view_dir, gun.origin);
            VectorMA(gun.origin, -gun_right, right_dir, gun.origin);
            VectorMA(gun.origin, -gun_up, up_dir, gun.origin);
        }
    }

    VectorCopy(gun.origin, gun.oldorigin);      // don't lerp at all

    if (gun_frame) {
        gun.frame = gun_frame;  // development tool
        gun.oldframe = gun_frame;   // development tool
    }
    else {
        gun.frame = ps->gunFrame;
        if (gun.frame == 0) {
            gun.oldframe = 0;   // just changed weapons, don't lerp from old
        }
        else {
            gun.oldframe = ops->gunFrame;
            gun.backlerp = 1.0f - CL_KEYLERPFRAC;
        }
    }

    gun.flags = RenderEffects::MinimalLight | RenderEffects::DepthHack | RenderEffects::WeaponModel;
    if (info_hand->integer == 1) {
        gun.flags |= RF_LEFTHAND;
    }

    if (cl_gunalpha->value != 1) {
        gun.alpha = clgi.Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
        gun.flags |= RenderEffects::Translucent;
    }

    // same entity in rtx mode
    if (vid_rtx->integer) {
        gun.flags |= shell_flags;
    }

    model_t* model = clgi.MOD_ForHandle(gun.model);
    if (model && strstr(model->name, "v_flareg"))
        gun.scale = 0.3f;

    V_AddEntity(&gun);

    // separate entity in non-rtx mode
    if (shell_flags && !vid_rtx->integer) {
        gun.alpha = 0.30f * cl_gunalpha->value;
        gun.flags |= shell_flags | RenderEffects::Translucent;
        V_AddEntity(&gun);
    }
}

//
//===============
// CLG_AddEntities
// 
// Emits all entities, particles, and lights, from server AND client to the refresh.
// If you desire any custom things to be rendered, add them here.
// (Think about a custom particle system, or what have ya ? : ))
//===============
//
void CLG_AddEntities(void)
{

}

//
//===============
// CLG_GetClientViewEntity
// 
// Returns the entity that is bound to the client's view.
//===============
//
cl_entity_t* CLG_GetClientViewEntity(void) {
    // Fetch clientnumber by default.
    int32_t index = cl->clientNumber;

    // Fetch the chasing entity index if we are chasing.
    if (cl->frame.playerState.stats[STAT_CHASE]) {
        index = cl->frame.playerState.stats[STAT_CHASE] - ConfigStrings::PlayerSkins;
    }

    return &cs->entities[index + 1];
}

//
//===============
// CLG_IsClientViewEntity
// 
// Returns true if the specified entity is bound to the local client's view.
//===============
//
qboolean CLG_IsClientViewEntity(const cl_entity_t* ent) {
    // If the entity number matches, then we're good.
    if (ent->current.number == cl->clientNumber + 1) {
        return true;
    }

    // If not, then we are viewing an other client entity, check whether it is in corpse mode.
    if ((ent->current.effects & EntityEffectType::Corpse) == 0) {
        // In case of no model index, we still want to validate some other cases.
        if (ent->current.modelIndex == 255) {
            if (ent->current.number == cl->clientNumber) {
                return true;
            } 

            // If we came to this point, fetch the chasing client.
            const int16_t chase = cl->frame.playerState.stats[STAT_CHASE] - ConfigStrings::PlayerSkins;

            if (ent->current.number == chase) {
                return true;
            }
        }
    }

    // And if we came to this point, all bets are off, this is no client entity which we are viewing.
    return false;
}

//
//=============================================================================
//
// CLIENT MODULE ENTITY ENTRY FUNCTIONS.
//
//=============================================================================}
//
//===============
// CLG_EntityEvent
//
// Handles specific events on an entity.
//===============
//
void CLG_EntityEvent(int number) {
    cl_entity_t *cent = &cs->entities[number];
    
    // EF_TELEPORTER acts like an event, but is not cleared each frame
    if ((cent->current.effects & EntityEffectType::Teleporter) && CL_FRAMESYNC) {
        CLG_TeleporterParticles(cent->current.origin);
    }
        
    switch (cent->current.event) {
    case EntityEvent::ItemRespawn:
        clgi.S_StartSound(NULL, number, CHAN_WEAPON, clgi.S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
        CLG_ItemRespawnParticles(cent->current.origin);
        break;
    case EntityEvent::PlayerTeleport:
        clgi.S_StartSound(NULL, number, CHAN_WEAPON, clgi.S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
        CLG_TeleportParticles(cent->current.origin);
        break;
    case EntityEvent::Footstep:
        if (cl_footsteps->integer)
            clgi.S_StartSound(NULL, number, CHAN_BODY, cl_sfx_footsteps[rand() & 3], 1, ATTN_NORM, 0);
        break;
    case EntityEvent::FallShort:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("player/land1.wav"), 1, ATTN_NORM, 0);
        break;
    case EntityEvent::Fall:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("*fall2.wav"), 1, ATTN_NORM, 0);
        break;
    case EntityEvent::FallFar:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("*fall1.wav"), 1, ATTN_NORM, 0);
        break;
    }
}


// Utility function for CLG_UpdateOrigin
static inline float lerp_client_fov(float ofov, float nfov, float lerp)
{
    if (clgi.IsDemoPlayback()) {
        float fov = info_fov->value;

        if (fov < 1)
            fov = 90;
        else if (fov > 160)
            fov = 160;

        if (info_uf->integer & UserFields::LocalFieldOfView)
            return fov;

        if (!(info_uf->integer & UserFields::PlayerFieldOfView)) {
            if (ofov >= 90)
                ofov = fov;
            if (nfov >= 90)
                nfov = fov;
        }
    }

    return ofov + lerp * (nfov - ofov);
}

//
//===============
// CLG_UpdateOrigin
//
// Sets cl->refdef view values and sound spatialization params.
// Usually called from V_AddEntities, but may be directly called from the main
// loop if rendering is disabled but sound is running.
//===============
//
void CLG_UpdateOrigin(void)
{
    PlayerState *currentPlayerState = NULL;
    PlayerState *previousPlayerState = NULL;

    float lerpFraction = cl->lerpFraction;

    // Only do this if we had a valid frame.
    if (!cl->frame.valid) {
        return;
    }

    // Find states to interpolate between
    currentPlayerState = &cl->frame.playerState;
    previousPlayerState = &cl->oldframe.playerState;

    //
    // Origin
    //
    if (!clgi.IsDemoPlayback() 
        && cl_predict->integer 
        && !(currentPlayerState->pmove.flags & PMF_NO_PREDICTION)) {
        // Add view offset to view org.
        ClientPredictedState* predictedState = &cl->predictedState;
        cl->refdef.vieworg = predictedState->viewOrigin + predictedState->viewOffset;

        const vec3_t error = vec3_scale(predictedState->error, 1.f - lerpFraction);
        cl->refdef.vieworg += error;

        cl->refdef.vieworg.z -= predictedState->stepOffset;
    } else {
        // Just use interpolated values
        // Adjust origins to keep stepOffset in mind.
        vec3_t oldOrigin = previousPlayerState->pmove.origin + previousPlayerState->pmove.viewOffset;
        oldOrigin.z -= cl->predictedState.stepOffset;
        vec3_t newOrigin = currentPlayerState->pmove.origin + currentPlayerState->pmove.viewOffset;
        newOrigin.z -= cl->predictedState.stepOffset;

        // Calculate final origin.
        cl->refdef.vieworg = vec3_mix(oldOrigin, newOrigin, lerpFraction);
    }

    //
    // View Angles.
    //
    // if not running a demo or on a locked frame, add the local angle movement
    if (clgi.IsDemoPlayback()) {
        LerpAngles(previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction, cl->refdef.viewAngles);
    }
    else if (currentPlayerState->pmove.type < EnginePlayerMoveType::Dead) {
        // use predicted values
        cl->refdef.viewAngles = cl->predictedState.viewAngles;
    }
    else {
        // just use interpolated values
        LerpAngles(previousPlayerState->pmove.viewAngles, currentPlayerState->pmove.viewAngles, lerpFraction, cl->refdef.viewAngles);
    }

#if USE_SMOOTH_DELTA_ANGLES
    cl->deltaAngles[0] = LerpAngle(previousPlayerState->pmove.deltaAngles[0], currentPlayerState->pmove.deltaAngles[0], lerpFraction);
    cl->deltaAngles[1] = LerpAngle(previousPlayerState->pmove.deltaAngles[1], currentPlayerState->pmove.deltaAngles[1], lerpFraction);
    cl->deltaAngles[2] = LerpAngle(previousPlayerState->pmove.deltaAngles[2], currentPlayerState->pmove.deltaAngles[2], lerpFraction);
#endif

    // don't interpolate blend color
    Vec4_Copy(currentPlayerState->blend, cl->refdef.blend);

    // Interpolate field of view
    cl->fov_x = lerp_client_fov(previousPlayerState->fov, currentPlayerState->fov, lerpFraction);
    cl->fov_y = CLG_CalculateFOV(cl->fov_x, 4, 3);

    // Calculate new client forward, right, and up vectors.
    vec3_vectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    // Setup player entity origin and angles accordingly to update the client's listener origins with.
    cl->playerEntityOrigin = cl->refdef.vieworg;
    cl->playerEntityAngles = cl->refdef.viewAngles;

    if (cl->playerEntityAngles[vec3_t::Pitch] > 180) {
        cl->playerEntityAngles[vec3_t::Pitch] -= 360;
    }

    cl->playerEntityAngles[vec3_t::Pitch] = cl->playerEntityAngles[vec3_t::Pitch] / 3;


    // Update the client's listener origin values.
    clgi.UpdateListenerOrigin();
}