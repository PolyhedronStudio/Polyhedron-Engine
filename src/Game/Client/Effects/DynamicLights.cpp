/***
*
*	License here.
*
*	@file
*
*	Dynamic Light Management.
* 
***/
#include "../ClientGameLocals.h"

#include "../Main.h"
#include "../TemporaryEntities.h"

#include "../ClientGameExports.h"
#include "../Exports/View.h"

#include "DynamicLights.h"
#include "Particles.h"
#include "ParticleEffects.h"


//! Actual array storing our dynamic lights for the current game.
cdlight_t DynamicLights::lights[MAX_DLIGHTS] = {};

/**
*   @brief  Clears out the dynamic light array for this frame.
**/
void DynamicLights::Clear() {
    for (int32_t i = 0; i < MAX_DLIGHTS; i++) {
        lights[i] = {};
    }
}

/**
*   @brief  Looks for a free slot to use in the dynamic lights array.
*   @return A pointer to one of the slots in the dynamic light array on success. nullptr otherwise.
**/
cdlight_t* DynamicLights::GetDynamicLight(int32_t key) {
    // First look for an exact key match
    if (key) {
        cdlight_t *dynamicLight = lights;
        for (int32_t i = 0; i < MAX_DLIGHTS; i++, dynamicLight++) {
            if (dynamicLight->key == key) {
                *dynamicLight = {};
                dynamicLight->key = key;
                return dynamicLight;
            }
        }
    }

    // Then look for anything else
    cdlight_t *dynamicLight = lights;
    for (int32_t i = 0; dynamicLight != nullptr && i < MAX_DLIGHTS; i++, dynamicLight++) {
        if (dynamicLight && dynamicLight->die < cl->time) {
            *dynamicLight = {};
            dynamicLight->key = key;
            return dynamicLight;
        }
    }

    dynamicLight = &lights[0];
    *dynamicLight = {};
    dynamicLight->key = key;
    return dynamicLight;
}

/**
*   @brief  Run each dynamic light for a frame. (Unless sv_paused is set.)
**/
void DynamicLights::RunFrame(void) {
    if (sv_paused->integer)
    {
        // Don't update the persistent dlights when the game is paused (e.g. photo mode).
        // Use sv_paused here because cl_paused can be nonzero in network play,
        // but the game is not really paused in that case.

        return;
    }

    cdlight_t *dynamicLight = lights;
    for (int32_t i = 0; i < MAX_DLIGHTS; i++, dynamicLight++) {
        if (!dynamicLight->radius)
            continue;

        if (dynamicLight->die < cl->time) {
            dynamicLight->radius = 0;
            return;
        }

        dynamicLight->radius -= clgi.GetFrameTime() * dynamicLight->decay;
        if (dynamicLight->radius < 0) {
            dynamicLight->radius = 0;
        }

        dynamicLight->origin = vec3_fmaf(dynamicLight->origin, clgi.GetFrameTime(), dynamicLight->velocity);
    }
}

/**
*   @brief  Adds the dynamic lights to the scene view.
**/
void DynamicLights::AddDynamicLightsToView() {
    cdlight_t *dynamicLight = lights;
    for (int32_t i = 0; i < MAX_DLIGHTS; i++, dynamicLight++) {
        if (!dynamicLight->radius) {
            continue;
        }
        clge->view->AddLight(dynamicLight->origin, dynamicLight->color, dynamicLight->radius);
    }
}