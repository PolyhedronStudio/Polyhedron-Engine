/***
*
*	License here.
*
*	@file
*
*	Client Game View Interface Implementation.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"

// Temporary Entities.
#include "Game/Client/TemporaryEntities.h"

// Effects.
#include "Game/Client/Effects/DynamicLights.h"
#include "Game/Client/Effects/LightStyles.h"
#include "Game/Client/Effects/Particles.h"

// Exports.
#include "Game/Client/Exports/Entities.h"
#include "Game/Client/Exports/View.h"


/**
*
*    Interface Functions.
* 
**/
/**
*   @brief  Called right after ClearScene.
**/
void ClientGameView::PreRenderView() {
	// Calculate client view values.
    clge->ClientUpdateOrigin();

    // Finish calculating view values.
    SetupViewCamera();

    // Set view origin and angles to that of our view camera.
    cl->refdef.vieworg      = viewCamera.GetViewOrigin();
    cl->refdef.viewAngles   = viewCamera.GetViewAngles();
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
    // Add all entities of the last received(should be the current if all is stable)
    // server frame to the current frame view.
    clge->entities->PrepareRefreshEntities();

    //
    // TODO: Not like how it is done right now haha. 
    //
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
*   @brief  Sets up the view for either first or third -person camera modes
*           and interpolates the camera smoothly between frames.
**/
void ClientGameView::SetupViewCamera() {
    PODEntity *viewEntity = &cs->entities[cl->frame.clientNumber + 1];

    // If cl_player_model isn't set to thirdperson, jump to firstperson label.
    if (cl_player_model->integer != CL_PLAYER_MODEL_THIRD_PERSON)
        goto firstpersonview;

    // If clientnumber isn't correct, jump to firstperson label.
    if (cl->frame.clientNumber == CLIENTNUM_NONE)
        goto firstpersonview;

    // If the serverframe isn't matching the view entity's frame number, engage
    // a first person view instead. 
    if (viewEntity && viewEntity->serverFrame != cl->frame.number)
        goto firstpersonview;

    // If there is no modelindex, jump to firstperson label.
    if (viewEntity && !viewEntity->currentState.modelIndex)
        goto firstpersonview;

    // Setup the thirdperson view.
    //SetupThirdpersonView();
    viewCamera.SetupThirdpersonViewProjection();

    return;

firstpersonview:
    // Setup the firstperson view.
    viewCamera.SetupFirstpersonViewProjection();
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
*   @brief   Outputs the view position to console.
**/
static void V_Viewpos_f(void)
{
    // Acquire view camera origin and angles.
    ViewCamera *viewCamera = clge->view->GetViewCamera();
    const vec3_t viewOrigin = viewCamera->GetViewOrigin();
    const vec3_t viewAngles = viewCamera->GetViewAngles();

    CLG_Print( PrintType::Regular, fmt::format( "({} {} {}) : {}\n", (int32_t)viewOrigin.x, (int32_t)viewOrigin.y, (int32_t)viewOrigin.z, (int32_t)viewAngles[vec3_t::Yaw] ));
}

/**
*   @brief   View Commands to register.
**/
static const cmdreg_t viewCommands[] = {
    { "viewpos", V_Viewpos_f },
    { NULL }
};

/**
*   @brief   ViewPosition macro.
**/
static size_t CL_ViewPos_m(char* buffer, size_t size) {
    ViewCamera *viewCamera = clge->view->GetViewCamera();
    const vec3_t viewOrigin = viewCamera->GetViewOrigin();

    return Q_scnprintf(buffer, size, "(%.1f, %.1f, %.1f)", viewOrigin.x, viewOrigin.y, viewOrigin.z);
}

/**
*   @brief   ViewDirection macro.
**/
static size_t CL_ViewDir_m(char* buffer, size_t size) {
    ViewCamera *viewCamera = clge->view->GetViewCamera();
    const vec3_t viewForward = viewCamera->GetForwardViewVector();
    return Q_scnprintf(buffer, size, "(%.3f, %.3f, %.3f)", viewForward.x, viewForward.y, viewForward.z);
}

/**
*   @brief  Called by media initialization to locally initialize 
*           the client game view data.
**/
void ClientGameView::Initialize() {
    // Set Cmds.
    clgi.Cmd_Register(viewCommands);

    // Set Cvars.
    cl_add_lights       = clgi.Cvar_Get("cl_lights", "1", 0);
    cl_show_lights      = clgi.Cvar_Get("cl_show_lights", "0", 0);

    cl_add_particles    = clgi.Cvar_Get("cl_particles", "1", 0);
    cl_add_entities     = clgi.Cvar_Get("cl_entities", "1", 0);
    cl_add_blend        = clgi.Cvar_Get("cl_blend", "1", 0);
    //cl_add_blend->changed = cl_add_blend_changed;

    cl_adjustfov        = clgi.Cvar_Get("cl_adjustfov", "1", 0);

    clgi.Cmd_AddMacro("cl_viewpos", CL_ViewPos_m);
	clgi.Cmd_AddMacro("cl_viewdir", CL_ViewDir_m);


}

/**
*   @brief  Same as Initialize, but for shutting down instead..
*/
void ClientGameView::Shutdown() {
    // Unregister view commands.
    clgi.Cmd_Unregister(viewCommands);
}

/**
*   @brief  When a free view render entity slot is available, assign this render entity to it.
**/
void ClientGameView::AddRenderEntity(r_entity_t &refreshEntity) {
    // Ensure we aren't exceeding boundary limits.
    if (num_renderEntities >= MAX_ENTITIES) {
        return;
    }

    // Copy entity over into the current scene frame list.
    renderEntities[num_renderEntities++] = refreshEntity;
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
        CLG_Print( PrintType::DeveloperWarning, "Warning: client view num_dlights >= MAX_DLIGHTS\n");
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
        CLG_Error( ErrorType::Drop, fmt::format( "{}: Bad light style {}", __func__, style ) );
        return;
    }

    lightstyle_t &lightStyle = lightstyles[style];

    // Setup lightstyle and its properties.
    lightStyle.rgb.x    = rgba.x;
    lightStyle.rgb.y    = rgba.y;
    lightStyle.rgb.z    = rgba.z;
    lightStyle.white    = rgba.w;
}
#else
void ClientGameView::AddLightStyle(int32_t style, const vec4_t &rgba) {

}
#endif