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
    void CalculateViewVectors();

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

private:
    /**
    *   @brief  Periodically calculates the player's horizontal speed, and interpolates it
    *           over a small interval to smooth out rapid changes in velocity.
    **/
    float CalculateBobSpeedModulus(const PlayerState *playerState);


private:
    //! True if in a third person game frame.
    bool isThirdperson = false;

    //! View Origin.
    vec3_t viewOrigin   = vec3_zero();
    //! View Angles.
    vec3_t viewAngles   = vec3_zero();
    //! Kick Angles. 
    vec3_t kickAngles   = vec3_zero();

    // View Bob.
    GameTime bobTime = GameTime::zero();
    //! Bob value of the client's view camera.
    float bobValue = 0;
};