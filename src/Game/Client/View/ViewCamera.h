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
    /**
    *   @brief  Calculates and sets the view bob for current frame.
    **/
    void UpdateViewBob();

    /**
    *   @brief  Sets up a firstperson view mode.
    **/
    void SetupFirstpersonViewProjection();
    /**
    *   @brief  Sets up a thirdperson view mode.
    **/
    void SetupThirdpersonViewProjection();

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

    /***
    *
    *   Get/Set
    * 
    ***/
    /**
    *   @return Origin of View Camera.
    **/
    const vec3_t& GetViewOrigin() { return viewOrigin; }
    /**
    *   @brief  Sets the camera's view origin.
    **/
    void SetViewOrigin(const vec3_t& viewOrigin) {
        this->viewOrigin = viewOrigin;
    }
    
    /**
    *   @return View angles of ViewCamera.
    **/
    const vec3_t& GetViewAngles() { return viewAngles; }
    /**
    *   @brief  Sets the camera's view angles.
    **/
    void SetViewAngles(const vec3_t& viewAngles) {
        this->viewAngles = viewAngles;
    }

    /**
    *   @return View delta angles of ViewCamera.
    **/
    const vec3_t& GetViewDeltaAngles() { return viewDeltaAngles; }
    /**
    *   @brief  Sets the camera's view delta angles.
    **/
    void SetViewDeltaAngles(const vec3_t& viewAngles) {
        this->viewDeltaAngles = viewAngles;
    }

private:
    /**
    *   @brief  Periodically calculates the player's horizontal speed, and interpolates it
    *           over a small interval to smooth out rapid changes in velocity.
    **/
    float CalculateBobSpeedModulus(const PlayerState *playerState);


private:
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
    //! View Angles.
    vec3_t viewAngles   = vec3_zero();
    //! View Delta Angles.
    vec3_t viewDeltaAngles = vec3_zero();
    //! Kick Angles. 
    vec3_t viewKickAngles   = vec3_zero();

    //! View Bob.
    GameTime bobTime = GameTime::zero();
    //! Bob value of the client's view camera.
    float bobValue = 0;
};