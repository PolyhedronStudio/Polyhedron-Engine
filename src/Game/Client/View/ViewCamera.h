/***
*
*	License here.
*
*	@file
*
*	ClientGame ViewCamera Implementation.
* 
***/
#pragma once



/**
*
*    ViewCamera Functionality.
* 
**/
class ViewCamera {
public:
    /***
    *
	*
    *   View Projections: First and Third -person are supported.
	*
    * 
    ***/
    /**
    *   @brief  Sets up a firstperson view mode.
    **/
    void SetupFirstpersonViewProjection();
    /**
    *   @brief  Sets up a thirdperson view mode.
    **/
    void SetupThirdpersonViewProjection();

    /***
    *
	*
    *   Viewbob & Weapon Viewmodel.
	*
    * 
    ***/
	/**
	*	@brief	Calculates the weapon viewmodel's origin and angles and adds it for rendering.
	**/
	void AddWeaponViewModel();


	/**
	*	@brief	Calculate client view bobmove.
	**/
	void CalculateBobMoveCycle( PlayerState *previousPlayerState, PlayerState *currentPlayerState );
	/**
	*	@brief	Calculate's view offset based on bobmove.
	**/
	void CalculateViewOffset( PlayerState *previousPlayerState, PlayerState *currentPlayerState );

	/**
	*	Applies the Viewbob to weapon.
	**/
	void CalculateWeaponViewBob( PlayerState *previousPlayerState, PlayerState *currentPlayerState );
	/**
	*	@brief	Traces the weapon tip against the world to calculate and apply a viewmodel offset with.
	**/
	void TraceViewWeaponOffset();
	/**
	*	@brief	Applies a certain view model drag effect to make it look more realistic in turns.
	**/
	void CalculateViewWeaponDrag( );
	/**
	*	@brief	Changes the view weapon model if the gun index has changed, and applies current animation state.
	**/
	void UpdateViewWeaponModel( PlayerState *previousPlayerState, PlayerState *currentPlayerState );



    /***
    *
	*
    *   View Vectors: Updated every time after we've set the view camera in a game's frame.
	*
    * 
    ***/
public:
    /**
    *   @brief  Calculates the new forward, up, and right vectors of
    *           the view camera.
    **/
    void UpdateViewVectors();
    /**
    *   @brief  Calculates the new forward, up, and right vectors of
    *           the view camera based on the vec3_t argument.
    **/
    void UpdateViewVectors(const vec3_t &fromAngles);
    /**
    *   @return The last calculated Forward vector for the View Camera's view. (Based on ViewAngles).
    **/
    inline const vec3_t &GetForwardViewVector() { return viewForward; }
    /**
    *   @return The last calculated Right vector for the View Camera's view. (Based on ViewAngles).
    **/
    inline const vec3_t &GetRightViewVector() { return viewRight; }
    /**
    *   @return The last calculated Up vector for the View Camera's view. (Based on ViewAngles).
    **/
    inline const vec3_t &GetUpViewVector() { return viewUp; }



    /***
    *
	*
    *   Get/Set View origin, angles, and delta angles.
	*
    * 
    ***/
    /**
    *   @return Origin of View Camera.
    **/
    const vec3_t& GetViewOrigin() { return viewOrigin; }
    /**
    *   @brief  Sets the camera's view origin.
    **/
    void SetViewOrigin( const vec3_t& viewOrigin ) {
        this->viewOrigin = viewOrigin;
    }
    /**
    *   @return View angles of ViewCamera.
    **/
    const vec3_t& GetViewAngles() { return viewAngles; }
    /**
    *   @brief  Sets the camera's view angles.
    **/
    void SetViewAngles( const vec3_t& viewAngles ) {
        this->viewAngles = viewAngles;
    }
    /**
    *   @return View delta angles of ViewCamera.
    **/
    const vec3_t& GetViewDeltaAngles() { return viewDeltaAngles; }
    /**
    *   @brief  Sets the camera's view delta angles.
    **/
    void SetViewDeltaAngles( const vec3_t& viewAngles ) {
        this->viewDeltaAngles = viewAngles;
    }

private:
	//! For maintaining view bob client side.


private:
	//! Refresh entity for our gun model.
	r_entity_t rEntWeaponViewModel;

    //! True if in a third person game frame.
    bool isThirdperson = false;
	//! Forward View Vector, based on viewAngles.
    vec3_t viewForward = vec3_zero();
    //! Forward View Vector, based on viewAngles.
    vec3_t viewRight = vec3_zero();
    //! Forward View Vector, based on viewAngles.
    vec3_t viewUp = vec3_zero();


    //! View Origin.
    vec3_t viewOrigin   = vec3_zero();
    //! The client maintains its own idea of view angles, which are
    //! sent to the server each frame.  It is cleared to 0 upon entering each level.
    //! the server sends a delta each frame which is added to the locally
    //! tracked view angles to account for standing on rotating objects,
    //! and teleport direction changes.
    vec3_t viewAngles   = vec3_zero();
    //! View Delta Angles.
    vec3_t viewDeltaAngles = vec3_zero();
    //! Kick Angles. 
    vec3_t viewKickAngles   = vec3_zero();

    //! View Bob.
	public:
    //GameTime bobTime = GameTime::zero();
	double bobTime = 0;
    //! Bob value of the client's view camera.
    double bobValue = 0;
    // BobMoveCycle is used for view bobbing, where the player FPS view looks like he is
    // walking instead of floating around.
    struct BobMoveCycle {
        // Forward, right, and up vectors.
        vec3_t  forward = vec3_zero(), right = vec3_zero(), up = vec3_zero();
        // Speed squared over the X/Y axis.
        double XYSpeed = 0.f;
        // bobMove counter.
        double move = 0.f;
        // Cycles are caculated over bobMove, uneven cycles = right foot.
        int64_t cycle = 0;
        // Calculated as: // sin(bobfrac*M_PI)
        double fracSin = 0.f;
    } bobMoveCycle;
};