/***
*
*	License here.
*
*	@file
*
*	Water Splash particle effect implementation.
* 
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! ClientGame Local headers.
#include "Game/Client/ClientGameLocals.h"
//! ClientGame World
#include "Game/Client/World/ClientGameWorld.h"
//! Actual particle system.
#include "Game/Client/Effects/Particles.h"
//! Actual effects.
#include "Game/Client/Effects/ParticleEffects.h"


/**
*   @brief  'Water Splash' like particle effect.
**/
void ParticleEffects::WaterSplash(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count) {
    vec3_t oy = vec3_t{0.f, 1.f, 0.f};
    
    if (fabs(vec3_dot(oy, direction)) > 0.95f) {
        oy = vec3_t{1.0f, 0.0f, 0.0f};
    }

    vec3_t ox = vec3_cross(oy, direction);

    count *= Particles::GetParticleNumberFactor() * 4;

    const float water_horizontal_spread = 0.35f;
    const float water_vertical_spread = 2.0f;
    const float water_base_velocity = 95.0f;
    const float water_rand_velocity = 150.0f;

    for (int i = 0; i < count; i++) {
        cparticle_t* splashParticle = Particles::GetFreeParticle();
        if (!splashParticle) {
            return;
        }

        splashParticle->time = cl->time;

        splashParticle->color = color + (rand() & 7);
        splashParticle->brightness = 1.0f;

        splashParticle->org = origin;
        splashParticle->org = vec3_fmaf(splashParticle->org, water_horizontal_spread * crand(), ox);
        splashParticle->org = vec3_fmaf(splashParticle->org, water_horizontal_spread * crand(), oy);
        splashParticle->org = vec3_fmaf(splashParticle->org, water_vertical_spread * frand() + 1.0f, direction);

        vec3_t velocity = vec3_normalize(splashParticle->org - origin);
        splashParticle->vel = vec3_scale(velocity, water_base_velocity + frand() * water_rand_velocity);

        splashParticle->acceleration[0] = splashParticle->acceleration[1] = 0;
        splashParticle->acceleration[2] = -ParticleEffects::ParticleGravity;
        splashParticle->alpha = 1.0;

        splashParticle->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}