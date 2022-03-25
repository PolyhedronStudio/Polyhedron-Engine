/***
*
*	License here.
*
*	@file
*
*	Client Game Dynamic Lights.
* 
***/
#pragma once

class DynamicLights {
public:
    /**
    *
    *    Dynamic Light Management.
    * 
    **/
    /**
    *   @brief  Clears out the dynamic light array for this frame.
    **/
    static void Clear();

    /**
    *   @brief  Looks for a free slot to use in the dynamic lights array.
    *   @return A pointer to one of the slots in the dynamic light array on success. nullptr otherwise.
    **/
    static cdlight_t *GetDynamicLight(int32_t key);

    /**
    *   @brief  Run each dynamic light for a frame. (Unless sv_paused is set.)
    **/
    static void RunFrame();

    /**
    *   @brief  Adds the dynamic lights to the scene view.
    **/
    static void AddDynamicLightsToView();

private:
    static cdlight_t lights[MAX_DLIGHTS];
};
