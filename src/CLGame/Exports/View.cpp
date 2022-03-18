/***
*
*	License here.
*
*	@file
*
*	Client Game View Interface Implementation.
* 
***/
#include "../ClientGameLocal.h"

#include "../Entities.h"
#include "../Main.h"
#include "../TemporaryEntities.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"

#include "../Effects/DynamicLights.h"
#include "../Effects/LightStyles.h"
#include "../Effects/Particles.h"
#include "Entities.h"
#include "View.h"


/**
*
*    Interface Functions.
* 
**/
/**
*   @brief  Called right after the engine clears the scene, and begins a new one.
**/
void ClientGameView::PreRenderView() {

}

/**
*   @brief  Called whenever the engine wants to clear the scene.
**/
void ClientGameView::ClearScene() {
    num_dlights = 0;
    num_renderEntities = 0;
    num_particles = 0;
}

/**
*   @brief  Called whenever the engine wants to render a valid frame.
**/
void ClientGameView::RenderView() {
    // Calculate client view values.
    clge->ClientUpdateOrigin();

    // Finish calculating view values.
    FinalizeViewValues();

    // Add all entities of the current server frame to the renderers view.
    clge->entities->AddPacketEntities();
    CLG_AddTempEntities();

    // Add all particle effects to view.
    Particles::AddParticlesToView();
    DynamicLights::AddDynamicLightsToView();

#if USE_LIGHTSTYLES
    LightStyles::AddLightStylesToView();
#endif

    // Last but not least, pass our array over to the client.
    cl->refdef.num_entities = num_renderEntities;
    cl->refdef.entities = renderEntities;
    cl->refdef.num_particles = num_particles;
    cl->refdef.particles = particles;
    cl->refdef.num_dlights = num_dlights;
    cl->refdef.dlights = dlights;
#if USE_LIGHTSTYLES
    cl->refdef.lightstyles = lightstyles;
#endif
}

/**
*   @brief  Called right after the engine renders the scene, and prepares to
*           finish up its current frame loop iteration.
**/
void ClientGameView::PostRenderView() {
    SetLightLevel();
}



/**
*
*    Camera Setup Functions.
* 
**/
/**
*   @brief  Finalizes the view values, aka render first or third person specific view data.
**/
void ClientGameView::FinalizeViewValues() {
    // For fetching the player pointer.
    ClientEntity* player = nullptr;

    // If cl_player_model isn't set to thirdperson, jump to firstperson label.
    if (cl_player_model->integer != CL_PLAYER_MODEL_THIRD_PERSON)
        goto firstpersonview;

    // If clientnumber isn't correct, jump to firstperson label.
    if (cl->frame.clientNumber == CLIENTNUM_NONE)
        goto firstpersonview;

    // If the entity its serverframe isn't matching the client's frame number, jump to firstperson label.
    player = &cs->entities[cl->frame.clientNumber + 1];
    if (player->serverFrame != cl->frame.number)
        goto firstpersonview;

    // If there is no modelindex, jump to firstperson label.
    if (!player ->current.modelIndex)
        goto firstpersonview;

    // Setup the thirdperson view.
    SetupThirdpersonView();
    return;

firstpersonview:
    // Setup the firstperson view.
    SetupFirstpersonView();
}

/**
*   @brief  Sets up a firstperson view mode.
**/
void ClientGameView::SetupFirstpersonView() {
    // If kickangles are enabled, lerp them and add to view angles.
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

/**
*   @brief  Sets up a thirdperson view mode.
**/
void ClientGameView::SetupThirdpersonView() {
    // Const vec3_t mins and maxs for box tracing the camera with.
    static const vec3_t mins = { -4, -4, -4 }, maxs = { 4, 4, 4 };

    // In case of death, set a specific view angle that looks nice.
    if (cl->frame.playerState.stats[PlayerStats::Health] <= 0) {
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

    // TODO: Uncomment when I get back to work on thirdperson camera.
    // This is the start of having a camera that is nice third person wise.
    // 
    // Likely needs a sphere instead of box collide in order to work properly though.
    // Experimenting with a side third person view.
    //cl->refdef.vieworg = vec3_fmaf(cl->refdef.vieworg, 24, cl->v_right);
    
    // Execute a box trace to see if we collide with the world.
    trace_t trace = clgi.Trace(cl->playerEntityOrigin, vec3_zero(), vec3_zero(), cl->refdef.vieworg, nullptr, CONTENTS_MASK_PLAYERSOLID);

    if (trace.fraction != 1.0f) {
        // We've collided with the world, let's adjust our view origin.
        cl->refdef.vieworg = trace.endPosition;
    }
    
    // Subtract view origin from focus point.
    focus -= cl->refdef.vieworg;

    // Calculate the new distance to use.
    float dist = std::sqrtf(focus[0] * focus[0] + focus[1] * focus[1]);

    // Set our view angles.
    cl->refdef.viewAngles[vec3_t::Pitch] = -180.f / M_PI * std::atan2f(focus[2], dist);
    cl->refdef.viewAngles[vec3_t::Yaw] -= cl_thirdperson_angle->value;

    // Last but not least, let it be known we are in thirdperson view.
    cl->thirdPersonView = true;
}

/**
*   TODO: If this feature gets to stay-alive, it should be moved over to the server.
*   Technically this is rather impossible to do given that we have no pre-baked data anymore.
*   And even if we do, it won't align to the real time RTX data that is made use of.
**/
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



/**
*
*    View Management Functions.
* 
**/
/**
*   @brief  Called by media initialization to locally initialize 
*           the client game view data.
**/
void ClientGameView::Initialize() {
    cl_add_lights       = clgi.Cvar_Get("cl_lights", "1", 0);
    cl_show_lights      = clgi.Cvar_Get("cl_show_lights", "0", 0);

    cl_add_particles    = clgi.Cvar_Get("cl_particles", "1", 0);
    cl_add_entities     = clgi.Cvar_Get("cl_entities", "1", 0);
    cl_add_blend        = clgi.Cvar_Get("cl_blend", "1", 0);
    //cl_add_blend->changed = cl_add_blend_changed;

    cl_adjustfov        = clgi.Cvar_Get("cl_adjustfov", "1", 0);
}

/**
*   @brief  Same as Initialize, but for shutting down instead..
*/
void ClientGameView::Shutdown() {

}

/**
*   @brief  When a free view render entity slot is available, assign this render entity to it.
**/
void ClientGameView::AddRenderEntity(r_entity_t *ent) {
    // Ensure we aren't exceeding boundary limits.
    if (num_renderEntities >= MAX_ENTITIES) {
        return;
    }

    // Copy entity over into the current scene frame list.
    renderEntities[num_renderEntities++] = *ent;
}

/**
*   @brief  When a free view particle slot is available, assign it to this render particle.
*   @return Returns false when no slot was available to assigning the particle to.
**/
bool ClientGameView::AddRenderParticle(const rparticle_t &renderParticle) {
    // Ensure we aren't exceeding boundary limits.
    if (num_particles >= MAX_PARTICLES) {
        return false;
    }

    // Copy particle over into the current scene frame list.
    particles[num_particles++] = renderParticle;

    // Success.
    return true;
}


/**
*   @brief  Adds a light to the current frame view.
*   @param  radius defaults to 10.f, effectively replacing 
*           the old AddLight/AddLightEx with a single function.
**/
void ClientGameView::AddLight(const vec3_t& origin, const vec3_t &rgb, float intensity, float radius) {
    // We topped the dynamic light limit, developer print a warning and opt out.
    if (num_dlights >= MAX_DLIGHTS) {
        Com_DPrint("Warning: client view num_dlights >= MAX_DLIGHTS\n");
        return;
    }

    // Acquire a reference to the free dynamic light slot.
    rdlight_t &dynamicLight = dlights[num_dlights++];
    dynamicLight.origin     = origin;
    dynamicLight.intensity  = intensity;
    dynamicLight.color.x    = rgb.x;
    dynamicLight.color.y    = rgb.y;
    dynamicLight.color.z    = rgb.z;
	dynamicLight.radius     = radius;

    // For developer/debug reasons, add a particle to where a light is meant to be just in case
    // something is off. Helps visualizing dat sh1t3 y0 d4wg.
	if (cl_show_lights->integer && num_particles < MAX_PARTICLES)
	{
		rparticle_t &particle = particles[num_particles++];

		particle.origin = dynamicLight.origin;
		particle.radius = radius;
		particle.brightness = Maxf(rgb.x, Maxf(rgb.y, rgb.z));
		particle.color = -1;
		particle.rgba.u8[0] = (uint8_t)Maxf(0.f, Minf(255.f, rgb.x / particle.brightness * 255.f));
		particle.rgba.u8[1] = (uint8_t)Maxf(0.f, Minf(255.f, rgb.y / particle.brightness * 255.f));
		particle.rgba.u8[2] = (uint8_t)Maxf(0.f, Minf(255.f, rgb.z / particle.brightness * 255.f));
		particle.rgba.u8[3] = 255;
		particle.alpha = 1.f;
	}
}

#if USE_LIGHTSTYLES
/**
*   @brief  Adds a 'lightstyle' to the current frame view.
**/
void ClientGameView::AddLightStyle(int32_t style, const vec4_t &rgba) {
    // Check for sanity.
    if (style < 0 || style >= MAX_LIGHTSTYLES) {
        Com_Error(ErrorType::Drop, "Bad light style %i", style);
        return;
    }

    lightstyle_t &lightStyle = lightstyles[style];

    // Setup lightstyle and its properties.
    lightStyle.rgb.x    = rgba.x;
    lightStyle.rgb.y    = rgba.y;
    lightStyle.rgb.z    = rgba.z;
    lightStyle.white    = rgba.w;
}
#endif