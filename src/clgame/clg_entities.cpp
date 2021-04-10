// LICENSE HERE.

//
// clg_entities.c
//
//
// Takes care of entity management.
//
// Local includes (shared, & other game defines.)
#include "clg_local.h"

// The actual Implementation.
#include "IEAPI/ClientGameExports.hpp"

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
    //    if (renderfx & RF_SHELL_HALF_DAM) {
    //        // ditch the half damage shell if any of red, blue, or double are on
    //        if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_DOUBLE))
    //            renderfx &= ~RF_SHELL_HALF_DAM;
    //    }

    //    if (renderfx & RF_SHELL_DOUBLE) {
    //        // lose the yellow shell if we have a red, blue, or green shell
    //        if (renderfx & (RF_SHELL_RED | RF_SHELL_BLUE | RF_SHELL_GREEN))
    //            renderfx &= ~RF_SHELL_DOUBLE;
    //        // if we have a red shell, turn it to purple by adding blue
    //        if (renderfx & RF_SHELL_RED)
    //            renderfx |= RF_SHELL_BLUE;
    //        // if we have a blue shell (and not a red shell), turn it to cyan by adding green
    //        else if (renderfx & RF_SHELL_BLUE) {
    //            // go to green if it's on already, otherwise do cyan (flash green)
    //            if (renderfx & RF_SHELL_GREEN)
    //                renderfx &= ~RF_SHELL_BLUE;
    //            else
    //                renderfx |= RF_SHELL_GREEN;
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
    entity_state_t* s1;
    float               autorotate;
    int                 i;
    int                 pnum;
    cl_entity_t* cent;
    int                 autoanim;
    clientinfo_t* ci;
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

        // set frame
        if (effects & EF_ANIM01)
            ent.frame = autoanim & 1;
        else if (effects & EF_ANIM23)
            ent.frame = 2 + (autoanim & 1);
        else if (effects & EF_ANIM_ALL)
            ent.frame = autoanim;
        else if (effects & EF_ANIM_ALLFAST)
            ent.frame = (cl->time / 33.33f); //30 fps
      //        ent.frame = (cl->time / 50.0f); //20 fps
        else
            ent.frame = s1->frame;

        // quad and pent can do different things on client
        if (effects & EF_PENT) {
            effects &= ~EF_PENT;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_RED;
        }

        if (effects & EF_QUAD) {
            effects &= ~EF_QUAD;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_BLUE;
        }

        if (effects & EF_DOUBLE) {
            effects &= ~EF_DOUBLE;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_DOUBLE;
        }

        if (effects & EF_HALF_DAMAGE) {
            effects &= ~EF_HALF_DAMAGE;
            effects |= EF_COLOR_SHELL;
            renderfx |= RF_SHELL_HALF_DAM;
        }

        // optionally remove the glowing effect
        if (cl_noglow->integer)
            renderfx &= ~RF_GLOW;

        ent.oldframe = cent->prev.frame;
        ent.backlerp = 1.0 - cl->lerpfrac;

        if (renderfx & RF_FRAMELERP) {
            // step origin discretely, because the frames
            // do the animation properly
            VectorCopy(cent->current.origin, ent.origin);
            VectorCopy(cent->current.old_origin, ent.oldorigin);  // FIXME
        }
        else if (renderfx & RF_BEAM) {
            // interpolate start and end points for beams
            LerpVector(cent->prev.origin, cent->current.origin,
                cl->lerpfrac, ent.origin);
            LerpVector(cent->prev.old_origin, cent->current.old_origin,
                cl->lerpfrac, ent.oldorigin);
        }
        else {
            if (s1->number == cl->frame.clientNum + 1) {
                // use predicted origin
                VectorCopy(cl->playerEntityOrigin, ent.origin);
                VectorCopy(cl->playerEntityOrigin, ent.oldorigin);
            }
            else {
                // interpolate origin
                LerpVector(cent->prev.origin, cent->current.origin,
                    cl->lerpfrac, ent.origin);
                VectorCopy(ent.origin, ent.oldorigin);
            }
        }

        if ((effects & EF_GIB) && !cl_gibs->integer) {
            goto skip;
        }

        // create a new entity

        // tweak the color of beams
        if (renderfx & RF_BEAM) {
            // the four beam colors are encoded in 32 bits of skinnum (hack)
            ent.alpha = 0.30;
            ent.skinnum = (s1->skinnum >> ((rand() % 4) * 8)) & 0xff;
            ent.model = 0;
        }
        else {
            // set skin
            if (s1->modelindex == 255) {
                // use custom player skin
                ent.skinnum = 0;
                ci = &cl->clientinfo[s1->skinnum & 0xff];
                ent.skin = ci->skin;
                ent.model = ci->model;
                if (!ent.skin || !ent.model) {
                    ent.skin = cl->baseclientinfo.skin;
                    ent.model = cl->baseclientinfo.model;
                    ci = &cl->baseclientinfo;
                }
                if (renderfx & RF_USE_DISGUISE) {
                    char buffer[MAX_QPATH];

                    Q_concat(buffer, sizeof(buffer), "players/", ci->model_name, "/disguise.pcx", NULL);
                    ent.skin = clgi.R_RegisterSkin(buffer);
                }
            }
            else {
                ent.skinnum = s1->skinnum;
                ent.skin = 0;
                ent.model = cl->model_draw[s1->modelindex];
                if (ent.model == cl_mod_laser || ent.model == cl_mod_dmspot)
                    renderfx |= RF_NOSHADOW;
            }
        }

        // only used for black hole model right now, FIXME: do better
        if ((renderfx & RF_TRANSLUCENT) && !(renderfx & RF_BEAM))
            ent.alpha = 0.70;

        // render effects (fullbright, translucent, etc)
        if ((effects & EF_COLOR_SHELL))
            ent.flags = 0;  // renderfx go on color shell entity
        else
            ent.flags = renderfx;

        // calculate angles
        if (effects & EF_ROTATE) {  // some bonus items auto-rotate
            ent.angles[0] = 0;
            ent.angles[1] = autorotate;
            ent.angles[2] = 0;
        }
        else if (effects & EF_SPINNINGLIGHTS) {
            vec3_t forward;
            vec3_t start;

            ent.angles[0] = 0;
            ent.angles[1] = anglemod(cl->time / 2) + s1->angles[1];
            ent.angles[2] = 180;

            AngleVectors(ent.angles, &forward, NULL, NULL);
            VectorMA(ent.origin, 64, forward, start);
            V_AddLight(start, 100, 1, 0, 0);
        }
        else if (s1->number == cl->frame.clientNum + 1) {
            VectorCopy(cl->playerEntityAngles, ent.angles);      // use predicted angles
        }
        else { // interpolate angles
            LerpAngles(cent->prev.angles, cent->current.angles,
                cl->lerpfrac, ent.angles);

            // mimic original ref_gl "leaning" bug (uuugly!)
            if (s1->modelindex == 255 && cl_rollhack->integer) {
                ent.angles[vec3_t::Roll] = -ent.angles[vec3_t::Roll];
            }
        }

        if (s1->number == cl->frame.clientNum + 1) {
            if (effects & EF_FLAG1)
                V_AddLight(ent.origin, 225, 1.0, 0.1, 0.1);
            else if (effects & EF_FLAG2)
                V_AddLight(ent.origin, 225, 0.1, 0.1, 1.0);
            else if (effects & EF_TAGTRAIL)
                V_AddLight(ent.origin, 225, 1.0, 1.0, 0.0);
            else if (effects & EF_TRACKERTRAIL)
                V_AddLight(ent.origin, 225, -1.0, -1.0, -1.0);

            if (!cl->thirdPersonView)
            {
                if (vid_rtx->integer)
                    base_entity_flags |= RF_VIEWERMODEL;    // only draw from mirrors
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
        if (!s1->modelindex) {
            goto skip;
        }

        if (effects & EF_BFG) {
            ent.flags |= RF_TRANSLUCENT;
            ent.alpha = 0.30;
        }

        if (effects & EF_PLASMA) {
            ent.flags |= RF_TRANSLUCENT;
            ent.alpha = 0.6;
        }

        if (effects & EF_SPHERETRANS) {
            ent.flags |= RF_TRANSLUCENT;
            if (effects & EF_TRACKERTRAIL)
                ent.alpha = 0.6;
            else
                ent.alpha = 0.3;
        }

        ent.flags |= base_entity_flags;

        // in rtx mode, the base entity has the renderfx for shells
        if ((effects & EF_COLOR_SHELL) && vid_rtx->integer) {
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
        if ((effects & EF_COLOR_SHELL) && !vid_rtx->integer) {
            renderfx = adjust_shell_fx(renderfx);
            ent.flags = renderfx | RF_TRANSLUCENT | base_entity_flags;
            ent.alpha = 0.30;
            V_AddEntity(&ent);
        }

        ent.skin = 0;       // never use a custom skin on others
        ent.skinnum = 0;
        ent.flags = base_entity_flags;
        ent.alpha = 0;

        // duplicate for linked models
        if (s1->modelindex2) {
            if (s1->modelindex2 == 255) {
                // custom weapon
                ci = &cl->clientinfo[s1->skinnum & 0xff];
                i = (s1->skinnum >> 8); // 0 is default weapon model
                if (i < 0 || i > cl->numWeaponModels - 1)
                    i = 0;
                ent.model = ci->weaponmodel[i];
                if (!ent.model) {
                    if (i != 0)
                        ent.model = ci->weaponmodel[0];
                    if (!ent.model)
                        ent.model = cl->baseclientinfo.weaponmodel[0];
                }
            }
            else
                ent.model = cl->model_draw[s1->modelindex2];

            // PMM - check for the defender sphere shell .. make it translucent
            if (!Q_strcasecmp(cl->configstrings[CS_MODELS + (s1->modelindex2)], "models/items/shell/tris.md2")) {
                ent.alpha = 0.32;
                ent.flags = RF_TRANSLUCENT;
            }

            if ((effects & EF_COLOR_SHELL) && vid_rtx->integer) {
                ent.flags |= renderfx;
            }

            V_AddEntity(&ent);

            //PGM - make sure these get reset.
            ent.flags = base_entity_flags;
            ent.alpha = 0;
        }

        if (s1->modelindex3) {
            ent.model = cl->model_draw[s1->modelindex3];
            V_AddEntity(&ent);
        }

        if (s1->modelindex4) {
            ent.model = cl->model_draw[s1->modelindex4];
            V_AddEntity(&ent);
        }

        if (effects & EF_POWERSCREEN) {
            ent.model = cl_mod_powerscreen;
            ent.oldframe = 0;
            ent.frame = 0;
            ent.flags |= (RF_TRANSLUCENT | RF_SHELL_GREEN);
            ent.alpha = 0.30;
            V_AddEntity(&ent);
        }

        // add automatic particle trails
        if (effects & ~EF_ROTATE) {
            if (effects & EF_ROCKET) {
                if (!(cl_disable_particles->integer & NOPART_ROCKET_TRAIL)) {
                    CLG_RocketTrail(cent->lerp_origin, ent.origin, cent);
                }
                V_AddLight(ent.origin, 200, 0.6f, 0.4f, 0.12f);
            }
            else if (effects & EF_BLASTER) {
                if (effects & EF_TRACKER) {
                    CLG_BlasterTrail2(cent->lerp_origin, ent.origin);
                    V_AddLight(ent.origin, 200, 0.1f, 0.4f, 0.12f);
                }
                else {
                    CLG_BlasterTrail(cent->lerp_origin, ent.origin);
                    V_AddLight(ent.origin, 200, 0.6f, 0.4f, 0.12f);
                }
            }
            else if (effects & EF_HYPERBLASTER) {
                if (effects & EF_TRACKER)
                    V_AddLight(ent.origin, 200, 0.1f, 0.4f, 0.12f);
                else
                    V_AddLight(ent.origin, 200, 0.6f, 0.4f, 0.12f);
            }
            else if (effects & EF_GIB) {
                CLG_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
            }
            else if (effects & EF_GRENADE) {
                if (!(cl_disable_particles->integer & NOPART_GRENADE_TRAIL)) {
                    CLG_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
                }
            }
            else if (effects & EF_FLIES) {
                CLG_FlyEffect(cent, ent.origin);
            }
            else if (effects & EF_BFG) {
                if (effects & EF_ANIM_ALLFAST) {
                    CLG_BfgParticles(&ent);
#if USE_DLIGHTS
                    i = 100;
                }
                else {
                    static const int bfg_lightramp[6] = { 300, 400, 600, 300, 150, 75 };

                    i = s1->frame; clamp(i, 0, 5);
                    i = bfg_lightramp[i];
#endif
                }
                const vec3_t nvgreen = { 0.2716f, 0.5795f, 0.04615f };
                V_AddLightEx(ent.origin, i, nvgreen[0], nvgreen[1], nvgreen[2], 20.f);
            }
            else if (effects & EF_TRAP) {
                ent.origin[2] += 32;
                CLG_TrapParticles(&ent);
#if USE_DLIGHTS
                i = (rand() % 100) + 100;
                V_AddLight(ent.origin, i, 1, 0.8, 0.1);
#endif
            }
            else if (effects & EF_FLAG1) {
                CLG_FlagTrail(cent->lerp_origin, ent.origin, 242);
                V_AddLight(ent.origin, 225, 1, 0.1, 0.1);
            }
            else if (effects & EF_FLAG2) {
                CLG_FlagTrail(cent->lerp_origin, ent.origin, 115);
                V_AddLight(ent.origin, 225, 0.1, 0.1, 1);
            }
            else if (effects & EF_TAGTRAIL) {
                CLG_TagTrail(cent->lerp_origin, ent.origin, 220);
                V_AddLight(ent.origin, 225, 1.0, 1.0, 0.0);
            }
            else if (effects & EF_TRACKERTRAIL) {
                if (effects & EF_TRACKER) {
#if USE_DLIGHTS
                    float intensity;

                    intensity = 50 + (500 * (sinf(cl->time / 500.0) + 1.0));
                    V_AddLight(ent.origin, intensity, -1.0, -1.0, -1.0);
#endif
                }
                else {
                    CLG_Tracker_Shell(cent->lerp_origin);
                    V_AddLight(ent.origin, 155, -1.0, -1.0, -1.0);
                }
            }
            else if (effects & EF_TRACKER) {
                CLG_TrackerTrail(cent->lerp_origin, ent.origin, 0);
                V_AddLight(ent.origin, 200, -1, -1, -1);
            }
            else if (effects & EF_GREENGIB) {
                CLG_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
            }
            else if (effects & EF_IONRIPPER) { // N&C - Turned into flickering candle light
                float anim = sinf((float)ent.id + ((float)cl->time / 60.f + frand() * 3.2)) / (3.24356 - (frand() / 3.24356));

                float offset = anim * 0.0f;
                float brightness = anim * 1.2f + 1.6f;

                vec3_t origin;
                VectorCopy(ent.origin, origin);
                origin[2] += offset;

                V_AddLightEx(origin, 25.f, 1.0f * brightness, 0.52f * brightness, 0.1f * brightness, 1.0f);
            }
            else if (effects & EF_BLUEHYPERBLASTER) { // N&C - Turned into flickering flame light
                float anim = sinf((float)ent.id + ((float)cl->time / 60.f + frand() * 3.3)) / (3.14356 - (frand() / 3.14356));

                float offset = anim * 0.0f;
                float brightness = anim * 1.2f + 1.6f;

                vec3_t origin;
                VectorCopy(ent.origin, origin);
                origin[2] += offset;

                V_AddLightEx(origin, 25.f, 1.0f * brightness, 0.425f * brightness, 0.1f * brightness, 3.6f);
            }
            else if (effects & EF_PLASMA) {
                if (effects & EF_ANIM_ALLFAST) {
                    CLG_BlasterTrail(cent->lerp_origin, ent.origin);
                }
                V_AddLight(ent.origin, 130, 1, 0.5, 0.5);
            }
        }

        //Com_DPrint("[NORMAL] entity ID =%i - origin = [%f, %f, %f]\n", ent.id, ent.origin[0], ent.origin[1], ent.origin[1]);
    skip:
        VectorCopy(ent.origin, cent->lerp_origin);

        //Com_DPrint("[SKIP] entity ID =%i - origin = [%f, %f, %f]\n", ent.id, ent.origin[0], ent.origin[1], ent.origin[1]);
    }
}

static int shell_effect_hack(void)
{
    cl_entity_t* ent;
    int         flags = 0;

    if (cl->frame.clientNum == CLIENTNUM_NONE)
        return 0;

    ent = &cs->entities[cl->frame.clientNum + 1];
    if (ent->serverframe != cl->frame.number)
        return 0;

    if (!ent->current.modelindex)
        return 0;

    if (ent->current.effects & EF_PENT)
        flags |= RF_SHELL_RED;
    if (ent->current.effects & EF_QUAD)
        flags |= RF_SHELL_BLUE;
    if (ent->current.effects & EF_DOUBLE)
        flags |= RF_SHELL_DOUBLE;
    if (ent->current.effects & EF_HALF_DAMAGE)
        flags |= RF_SHELL_HALF_DAM;

    return flags;
}

/*
==============
CLG_AddViewWeapon
==============
*/
void CLG_AddViewWeapon(void)
{
    player_state_t* ps, * ops;
    r_entity_t    gun;        // view model
    int         i, shell_flags;

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
        gun.model = cl->model_draw[ps->gunindex];
    }
    if (!gun.model) {
        return;
    }

    gun.id = RESERVED_ENTITIY_GUN;

    // set up gun position
    for (i = 0; i < 3; i++) {
        gun.origin[i] = cl->refdef.vieworg[i] + ops->gunoffset[i] +
            CL_KEYLERPFRAC * (ps->gunoffset[i] - ops->gunoffset[i]);
        gun.angles[i] = cl->refdef.viewAngles[i] + LerpAngle(ops->gunangles[i],
            ps->gunangles[i], CL_KEYLERPFRAC);
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

        clgi.CM_BoxTrace(&trace, &gun_real_pos, &gun_tip, &mins, &maxs, cl->bsp->nodes, CONTENTS_MASK_SOLID);

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
        gun.frame = ps->gunframe;
        if (gun.frame == 0) {
            gun.oldframe = 0;   // just changed weapons, don't lerp from old
        }
        else {
            gun.oldframe = ops->gunframe;
            gun.backlerp = 1.0f - CL_KEYLERPFRAC;
        }
    }

    gun.flags = RF_MINLIGHT | RF_DEPTHHACK | RF_WEAPONMODEL;
    if (info_hand->integer == 1) {
        gun.flags |= RF_LEFTHAND;
    }

    if (cl_gunalpha->value != 1) {
        gun.alpha = clgi.Cvar_ClampValue(cl_gunalpha, 0.1f, 1.0f);
        gun.flags |= RF_TRANSLUCENT;
    }

    // add shell effect from player entity
    shell_flags = shell_effect_hack();

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
        gun.flags |= shell_flags | RF_TRANSLUCENT;
        V_AddEntity(&gun);
    }
}

#if USE_SMOOTH_DELTA_ANGLES
static inline float LerpShort(int a2, int a1, float frac)
{
    if (a1 - a2 > 32768)
        a1 &= 65536;
    if (a2 - a1 > 32768)
        a1 &= 65536;
    return a2 + frac * (a1 - a2);
}
#endif

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
//=============================================================================
//
// CLIENT GAME MODULE ENTITY ENTRY FUNCTIONS.
//
//=============================================================================}
//
//===============
// ClientGameExports::EntityEvent
//
// Handles specific events on an entity.
//===============
//
void ClientGameExports::EntityEvent(int number) {
    cl_entity_t *cent = &cs->entities[number];
    
    // EF_TELEPORTER acts like an event, but is not cleared each frame
    if ((cent->current.effects & EF_TELEPORTER) && CL_FRAMESYNC) {
        CLG_TeleporterParticles(cent->current.origin);
    }
        
    switch (cent->current.event) {
    case EV_ITEM_RESPAWN:
        clgi.S_StartSound(NULL, number, CHAN_WEAPON, clgi.S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
        CLG_ItemRespawnParticles(cent->current.origin);
        break;
    case EV_PLAYER_TELEPORT:
        clgi.S_StartSound(NULL, number, CHAN_WEAPON, clgi.S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
        CLG_TeleportParticles(cent->current.origin);
        break;
    case EV_FOOTSTEP:
        if (cl_footsteps->integer)
            clgi.S_StartSound(NULL, number, CHAN_BODY, cl_sfx_footsteps[rand() & 3], 1, ATTN_NORM, 0);
        break;
    case EV_FALLSHORT:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("player/land1.wav"), 1, ATTN_NORM, 0);
        break;
    case EV_FALL:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("*fall2.wav"), 1, ATTN_NORM, 0);
        break;
    case EV_FALLFAR:
        clgi.S_StartSound(NULL, number, CHAN_AUTO, clgi.S_RegisterSound("*fall1.wav"), 1, ATTN_NORM, 0);
        break;
    }
}


// Utility function for CLG_CalculateViewValues
static inline float lerp_client_fov(float ofov, float nfov, float lerp)
{
    if (clgi.IsDemoPlayback()) {
        float fov = info_fov->value;

        if (fov < 1)
            fov = 90;
        else if (fov > 160)
            fov = 160;

        if (info_uf->integer & UF_LOCALFOV)
            return fov;

        if (!(info_uf->integer & UF_PLAYERFOV)) {
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
// CLG_CalculateViewValues
//
// Sets cl->refdef view values and sound spatialization params.
// Usually called from V_AddEntities, but may be directly called from the main
// loop if rendering is disabled but sound is running.
//===============
//
void CLG_CalculateViewValues(void) {
    player_state_t* ps, * ops;
    vec3_t viewoffset;
    float lerp;

    if (!cl->frame.valid) {
        return;
    }

    // find states to interpolate between
    ps = &cl->frame.playerState;
    ops = &cl->oldframe.playerState;

    lerp = cl->lerpfrac;

    // calculate the origin
    if (!clgi.IsDemoPlayback() && cl_predict->integer && !(ps->pmove.flags & PMF_NO_PREDICTION)) {
        // use predicted values
        unsigned delta = clgi.GetRealTime() - cl->predicted_step_time;
        float backlerp = lerp - 1.0;

        VectorMA(cl->predicted_origin, backlerp, cl->prediction_error, cl->refdef.vieworg);

        // smooth out stair climbing
        // N&C: FF Precision.
        if (cl->predicted_step < (127.0f / 8.0f)) {         //if (cl->predicted_step < 127 * 0.125f) {
            delta *= 0.5;                                   //  delta <<= 1; // small steps
        }
        // N&C: FF Precision.
        if (delta < (100.0f)) {
            cl->refdef.vieworg[2] -= cl->predicted_step * ((100.0f) - delta) * 0.01f;
        }
        //if (delta < 100) {
        //  cl->refdef.vieworg[2] -= cl->predicted_step * (100 - delta) * 0.01f;
        //}
    }
    else {
        // just use interpolated values
        // N&C: FF Precision.
        cl->refdef.vieworg[0] = ops->pmove.origin[0] +
            lerp * (ps->pmove.origin[0] - ops->pmove.origin[0]);
        cl->refdef.vieworg[1] = ops->pmove.origin[1] +
            lerp * (ps->pmove.origin[1] - ops->pmove.origin[1]);
        cl->refdef.vieworg[2] = ops->pmove.origin[2] +
            lerp * (ps->pmove.origin[2] - ops->pmove.origin[2]);
    }

    // if not running a demo or on a locked frame, add the local angle movement
    if (clgi.IsDemoPlayback()) {
        LerpAngles(ops->viewAngles, ps->viewAngles, lerp, cl->refdef.viewAngles);
    }
    else if (ps->pmove.type < PM_DEAD) {
        // use predicted values
        VectorCopy(cl->predicted_angles, cl->refdef.viewAngles);
    }
    else {
        // just use interpolated values
        LerpAngles(ops->viewAngles, ps->viewAngles, lerp, cl->refdef.viewAngles);
    }

#if USE_SMOOTH_DELTA_ANGLES
    cl->delta_angles[0] = LerpShort(ops->pmove.delta_angles[0], ps->pmove.delta_angles[0], lerp);
    cl->delta_angles[1] = LerpShort(ops->pmove.delta_angles[1], ps->pmove.delta_angles[1], lerp);
    cl->delta_angles[2] = LerpShort(ops->pmove.delta_angles[2], ps->pmove.delta_angles[2], lerp);
#endif

    // don't interpolate blend color
    Vec4_Copy(ps->blend, cl->refdef.blend);

    // interpolate field of view
    cl->fov_x = lerp_client_fov(ops->fov, ps->fov, lerp);
    cl->fov_y = CLG_CalculateFOV(cl->fov_x, 4, 3);

    LerpVector(ops->viewoffset, ps->viewoffset, lerp, viewoffset);

    AngleVectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    VectorCopy(cl->refdef.vieworg, cl->playerEntityOrigin);
    VectorCopy(cl->refdef.viewAngles, cl->playerEntityAngles);

    if (cl->playerEntityAngles[vec3_t::Pitch] > 180) {
        cl->playerEntityAngles[vec3_t::Pitch] -= 360;
    }

    cl->playerEntityAngles[vec3_t::Pitch] = cl->playerEntityAngles[vec3_t::Pitch] / 3;

    VectorAdd(cl->refdef.vieworg, viewoffset, cl->refdef.vieworg);

    // Update the client's listener origin values.
    clgi.UpdateListenerOrigin();
}

void ClientGameExports::CalculateViewValues(void) {
    CLG_CalculateViewValues();
}