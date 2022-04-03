/***
*
*	License here.
*
*	@file
*
*	Client Game View Interface Implementation.
* 
***/
#pragma once

// View Camera.
#include "../View/ViewCamera.h"


/**
*   Client Game View IMPLEMENTATION.
**/
class ClientGameView : public IClientGameExportView {
public:
	//! Destructor.
    virtual ~ClientGameView() = default;



    /**
    *
    *    Interface Functions.
    * 
    **/
    /**
    *   @brief  Called right after the engine clears the scene, and begins a new one.
    **/
    void PreRenderView() final;
    /**
    *   @brief  Called whenever the engine wants to clear the scene.
    **/
    void ClearScene() final;
    /**
    *   @brief  Called whenever the engine wants to render a valid frame.
    **/
    void RenderView() final;
    /**
    *   @brief  Called right after the engine renders the scene, and prepares to
    *           finish up its current frame loop iteration.
    **/
    void PostRenderView() final;



    /**
    *
    *    View Management Functions.
    * 
    **/
    /**
    *   @brief  Called by media initialization to locally initialize 
    *           the client game view data.
    **/
    void Initialize();

    /**
    *   @brief  Same as Initialize, but for shutting down instead..
    */
    void Shutdown();

    /**
    *   @brief  When a free view render entity slot is available, assign this render entity to it.
    **/
    void AddRenderEntity(r_entity_t* ent);

    /**
    *   @brief  When a free view particle slot is available, assign it to this render particle.
    *   @return Returns false when no slot was available to assigning the particle to.
    **/
    bool AddRenderParticle(const rparticle_t &renderParticle);

    /**
    *   @brief  Adds a light to the current frame view.
    *   @param  radius defaults to 10.f, effectively replacing 
    *           the old AddLight/AddLightEx with a single function.
    **/
    void AddLight(const vec3_t& origin, const vec3_t &rgb, float intensity, float radius = 10.f);

    /**
    *   @brief  Adds a 'lightstyle' to the current frame view.
    **/
    void AddLightStyle(int32_t style, const vec4_t &rgba);
    
    /**
    *   @return Pointer to our view camera.
    **/
    inline ViewCamera *GetViewCamera() { return &viewCamera; }


private:
    /**
    *
    *    View Camera Functionality.
    * 
    **/
    //! Client's main view camera tracking the player position and angles.
    ViewCamera viewCamera;


    /**
    *   @brief  Finalizes the view values, aka render first or third person specific view data.
    **/
    void SetupViewCamera();
    
    /**
    *   @brief  Sets up a firstperson view mode.
    **/
    void SetupFirstpersonView();

    /**
    *   @brief  Sets up a thirdperson view mode.
    **/
    void SetupThirdpersonView();

    /**
    *   TODO: If this feature gets to stay-alive, it should be moved over to the server.
    *   Technically this is rather impossible to do given that we have no pre-baked data anymore.
    *   And even if we do, it won't align to the real time RTX data that is made use of.
    **/
    void SetLightLevel();



private:
    cvar_t *cl_add_lights   = nullptr;
    cvar_t *cl_show_lights  = nullptr;
    cvar_t *cl_add_particles= nullptr;
    cvar_t *cl_add_entities = nullptr;
    cvar_t *cl_add_blend    = nullptr;
    cvar_t *cl_adjustfov    = nullptr;



private:
    //! Stores the entities.
    r_entity_t renderEntities[MAX_ENTITIES];
    int32_t num_renderEntities;

    //! Holds all the dynamic lights currently in the view frame.
    rdlight_t dlights[MAX_DLIGHTS];
    int32_t num_dlights;

    //! Holds all the particles currently in the view frame.
    rparticle_t particles[MAX_PARTICLES];
    int32_t num_particles;

    //! Holds all the explosions currently in the view frame.
    explosion_t  explosions[MAX_EXPLOSIONS];

    //! Holds all the lightstyles currently in the view frame.
#if USE_LIGHTSTYLES
    lightstyle_t lightstyles[MAX_LIGHTSTYLES];
#endif
};
