#include "../ClientGameLocal.h"

#include "../Effects.h"
#include "../Entities.h"
#include "../Input.h"
#include "../Main.h"
#include "../Media.h"
#include "../Parse.h"
#include "../Predict.h"
#include "../Screen.h"
#include "../TemporaryEntities.h"
#include "../View.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "View.h"

//---------------
// ClientGameView::PreRenderView
//
//---------------
void ClientGameView::PreRenderView() {

}

//---------------
// ClientGameView::ClearScene
//
//---------------
void ClientGameView::ClearScene() {
    view.num_dlights = 0;
    view.num_entities = 0;
    view.num_particles = 0;
}

//---------------
// ClientGameView::RenderView
//
//---------------
void ClientGameView::RenderView() {
    // Calculate client view values.
    clge->ClientUpdateOrigin();

    // Finish calculating view values.
    FinalizeViewValues();

    // Add all entities of the current server frame to the renderers view.
    clge->entities->AddPacketEntities();
    CLG_AddTempEntities();
    CLG_AddParticles();
    CLG_AddDLights();
#if USE_LIGHTSTYLES
    CLG_AddLightStyles();
#endif

    // Last but not least, pass our array over to the client.
    cl->refdef.num_entities = view.num_entities;
    cl->refdef.entities = view.entities;
    cl->refdef.num_particles = view.num_particles;
    cl->refdef.particles = view.particles;
    cl->refdef.num_dlights = view.num_dlights;
    cl->refdef.dlights = view.dlights;
#if USE_LIGHTSTYLES
    cl->refdef.lightstyles = view.lightstyles;
#endif
}

//---------------
// ClientGameView::PostRenderView
//
//---------------
void ClientGameView::PostRenderView() {
    SetLightLevel();
}

//---------------
// ClientGameView::FinalizeViewValues
//
//---------------
void ClientGameView::FinalizeViewValues() {
    // For fetching the clientEntity pointer.
    ClientEntity* clientEntity = nullptr;

    // If cl_player_model isn't set to thirdperson, jump to firstperson label.
    if (cl_player_model->integer != CL_PLAYER_MODEL_THIRD_PERSON)
        goto firstpersonview;

    // If clientnumber isn't correct, jump to firstperson label.
    if (cl->frame.clientNumber == CLIENTNUM_NONE)
        goto firstpersonview;

    // If the entity its serverframe isn't matching the client's frame number, jump to firstperson label.
    clientEntity = &cs->entities[cl->frame.clientNumber + 1];
    if (clientEntity->serverFrame != cl->frame.number)
        goto firstpersonview;

    // If there is no modelindex, jump to firstperson label.
    if (!clientEntity ->current.modelIndex)
        goto firstpersonview;

    // Setup the thirdperson view.
    SetupThirdpersonView();
    return;

firstpersonview:
    // Setup the firstperson view.
    SetupFirstpersonView();
}

//---------------
// ClientGameView::SetupFirstpersonView
//
//---------------
void ClientGameView::SetupFirstpersonView() {
    // Lerp and add the kick angles if enabled.
    if (cl_kickangles->integer) {
        PlayerState *playerState = &cl->frame.playerState;
        PlayerState *oldPlayerState = &cl->oldframe.playerState;

        // Lerp first.
        double lerp = cl->lerpFraction;
        vec3_t kickAngles = vec3_mix_euler(oldPlayerState->kickAngles, playerState->kickAngles, cl->lerpFraction);

        // Add afterwards.
        cl->refdef.viewAngles += kickAngles;
    }

    // Add the first person view entities.
    clge->entities->AddViewEntities();

    // Let the client state be known we aren't in thirdperson mode.
    cl->thirdPersonView = false;
}

//---------------
// ClientGameView::SetupThirdpersonView
//
//---------------
void ClientGameView::SetupThirdpersonView() {
    // Const vec3_t mins and maxs for box tracing the camera with.
    static const vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

    // In case of death, set a specific view angle that looks nice.
    if (cl->frame.playerState.stats[STAT_HEALTH] <= 0) {
        cl->refdef.viewAngles[vec3_t::Roll] = 0;
        cl->refdef.viewAngles[vec3_t::Pitch] = 10;
    }

    // Calculate focus point.
    vec3_t focus = vec3_fmaf(cl->refdef.vieworg, 512, cl->v_forward);
    //VectorMA(cl->refdef.vieworg, 512, cl->v_forward, focus);

    // Add an additional unit to the z value.
    cl->refdef.vieworg[2] += 8;
    cl->refdef.viewAngles[vec3_t::Pitch] *= 0.5f;
    AngleVectors(cl->refdef.viewAngles, &cl->v_forward, &cl->v_right, &cl->v_up);

    // Calculate view origin to use based on thirdperson range and angle.
    float angle = Radians(cl_thirdperson_angle->value);
    float range = cl_thirdperson_range->value;
    float fscale = std::cosf(angle);
    float rscale = std::sinf(angle);
    cl->refdef.vieworg = vec3_fmaf(cl->refdef.vieworg, -range * fscale, cl->v_forward);
    cl->refdef.vieworg = vec3_fmaf(cl->refdef.vieworg, -range * rscale, cl->v_right);
    //VectorMA(cl->refdef.vieworg, -range * fscale, cl->v_forward, cl->refdef.vieworg);
    //VectorMA(cl->refdef.vieworg, -range * rscale, cl->v_right, cl->refdef.vieworg);
    
    // Execute a box trace to see if we collide with the world.
    CLGTrace trace = CLG_Trace(cl->playerEntityOrigin,
                     mins, maxs, cl->refdef.vieworg, nullptr, CONTENTS_MASK_PLAYERSOLID);
    if (trace.fraction != 1.0f) {
        // We've collided with the world, let's adjust our view origin.
        cl->refdef.vieworg = trace.endPosition;
        // VectorCopy(trace.endPosition, cl->refdef.vieworg);
    }
    
    // Subtract view origin from focus point.
    focus -= cl->refdef.vieworg;
    //VectorSubtract(focus, cl->refdef.vieworg, focus);

    // Calculate the new distance to use.
    float dist = std::sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    // Set our view angles.
    cl->refdef.viewAngles[vec3_t::Pitch] = -180.f / M_PI * std::atan2f(focus[2], dist);
    cl->refdef.viewAngles[vec3_t::Yaw] -= cl_thirdperson_angle->value;

    // Last but not least, let it be known we are in thirdperson view.
    cl->thirdPersonView = true;
}

//---------------
// ClientGameView::SetLightLevel
//
// TODO: If this feature gets to stay-alive, it should be moved over to the server.
// Technically this is rather impossible to do given that we have no pre-baked data anymore.
// And even if we do, it won't align to the real time RTX data that is made use of.
//---------------
void ClientGameView::SetLightLevel() {   
    // The obvious part is that in RTX mode, the code below won't work.
    // Why? Because there is no way to fetch the actual light point that
    // the client is located at.
    
    // Save off light value for server to look at (BIG HACK!)
    vec3_t shadelight;
    clgi.R_LightPoint(cl->refdef.vieworg, shadelight);

    // Pick the greatest component, which should be the same
    // as the mono value returned by software
    if (shadelight[0] > shadelight[1]) {
        if (shadelight[0] > shadelight[2]) {
            cl->lightLevel = 150.0f * shadelight[0];
        } else {
            cl->lightLevel = 150.0f * shadelight[2];
        }
    } else {
        if (shadelight[1] > shadelight[2]) {
            cl->lightLevel = 150.0f * shadelight[1];
        } else {
            cl->lightLevel = 150.0f * shadelight[2];
        }
    }
}