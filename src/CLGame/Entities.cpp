// LICENSE HERE.

//
// clg_entities.c
//
//
// Takes care of entity management.
//
#include "ClientGameLocal.h"
#include "Effects.h"
#include "Entities.h"
#include "View.h"

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

/*
==============
CLG_AddViewWeapon
==============
*/
//void CLG_AddViewWeapon(void)
//{
//    int32_t  shell_flags = 0;
//
//    // Hidden in bsp menu mode.
//    if (info_in_bspmenu->integer) {
//        return;
//    }
//
//    // No need to render the gun in this case.
//    if (cl_player_model->integer == CL_PLAYER_MODEL_DISABLED) {
//        return;
//    }
//
//    //Neither in this case.
//    if (info_hand->integer == 2) {
//        return;
//    }
//
//    // find states to interpolate between
//    PlayerState *currentPlayerState = &cl->frame.playerState;
//    PlayerState *oldPlayerState= &cl->oldframe.playerState;
//
//    // Gun ViewModel.
//    r_entity_t gun = {
//        .model = (gun_model ? gun_model : 
//        (cl->drawModels[currentPlayerState->gunIndex] ? cl->drawModels[currentPlayerState->gunIndex] : 0)),
//        .id = RESERVED_ENTITIY_GUN,
//    };
//
//    if (!gun.model) {
//        return;
//    }
//
//    // Set up gun position
//    for (int32_t i = 0; i < 3; i++) {
//        gun.origin[i] = cl->refdef.vieworg[i] + oldPlayerState->gunOffset[i] +
//            cl->lerpFraction * (currentPlayerState->gunOffset[i] - oldPlayerState->gunOffset[i]);
//        gun.angles = cl->refdef.viewAngles + vec3_mix_euler(oldPlayerState->gunAngles,
//            currentPlayerState->gunAngles, cl->lerpFraction);
//    }
//
//    // Adjust for high fov
//    if (currentPlayerState->fov > 90) {
//        vec_t ofs = (90 - currentPlayerState->fov) * 0.2f;
//        VectorMA(gun.origin, ofs, cl->v_forward, gun.origin);
//    }
//
//    // adjust the gun origin so that the gun doesn't intersect with walls
//    {
//        vec3_t view_dir, right_dir, up_dir;
//        vec3_t gun_real_pos, gun_tip;
//        const float gun_length = 28.f;
//        const float gun_right = 10.f;
//        const float gun_up = -5.f;
//        trace_t trace;
//        static vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };
//
//        AngleVectors(cl->refdef.viewAngles, &view_dir, &right_dir, &up_dir);
//        VectorMA(gun.origin, gun_right, right_dir, gun_real_pos);
//        VectorMA(gun_real_pos, gun_up, up_dir, gun_real_pos);
//        VectorMA(gun_real_pos, gun_length, view_dir, gun_tip);
//
//        clgi.CM_BoxTrace(&trace, gun_real_pos, gun_tip, mins, maxs, cl->bsp->nodes, CONTENTS_MASK_SOLID);
//
//        if (trace.fraction != 1.0f)
//        {
//            VectorMA(trace.endPosition, -gun_length, view_dir, gun.origin);
//            VectorMA(gun.origin, -gun_right, right_dir, gun.origin);
//            VectorMA(gun.origin, -gun_up, up_dir, gun.origin);
//        }
//    }
//
//    VectorCopy(gun.origin, gun.oldorigin);      // don't lerp at all
//
//    if (gun_frame) {
//        gun.frame = gun_frame;  // development tool
//        gun.oldframe = gun_frame;   // development tool
//    }
//    else {
//        gun.frame = currentPlayerState->gunFrame;
//        if (gun.frame == 0) {
//            gun.oldframe = 0;   // just changed weapons, don't lerp from old
//        }
//        else {
//            gun.oldframe = oldPlayerState->gunFrame;
//            gun.backlerp = 1.0f - cl->lerpFraction;
//        }
//    }
//
//    gun.flags = RenderEffects::MinimalLight | RenderEffects::DepthHack | RenderEffects::WeaponModel;
//    if (info_hand->integer == 1) {
//        gun.flags |= RF_LEFTHAND;
//    }
//
//    if (cl_gunalpha->value != 1) {
//        gun.alpha = clgi.Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
//        gun.flags |= RenderEffects::Translucent;
//    }
//
//    // same entity in rtx mode
//    if (vid_rtx->integer) {
//        gun.flags |= shell_flags;
//    }
//
//    V_AddEntity(&gun);
//
//    // separate entity in non-rtx mode
//    if (shell_flags && !vid_rtx->integer) {
//        gun.alpha = 0.30f * cl_gunalpha->value;
//        gun.flags |= shell_flags | RenderEffects::Translucent;
//        V_AddEntity(&gun);
//    }
//}

//
//===============
// CLG_GetClientViewEntity
// 
// Returns the entity that is bound to the client's view.
//===============
//
ClientEntity* CLG_GetClientViewEntity(void) {
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
qboolean CLG_IsClientViewEntity(const ClientEntity* ent) {
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