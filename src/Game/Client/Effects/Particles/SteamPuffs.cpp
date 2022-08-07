/***
*
*	License here.
*
*	@file
*
*	Item Respawn particle effect implementation.
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
*   @brief  'ItemRespawn' like particle effect.
**/
void ParticleEffects::SteamPuffs(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count, int32_t magnitude) {
    vec3_t right, up;

    // Used for normal vectors, this code is a cheap move over anyway.
    vec3_t dir = direction;
    MakeNormalVectors(dir, right, up);

    for (int32_t i = 0; i < count; i++) {
        cparticle_t *particle = Particles::GetFreeParticle();
        if (!particle) {
            return;
        }

        particle->time = cl->time;
        particle->color = color + (rand() & 7);

        for (int32_t j = 0; j < 3; j++) {
            particle->org[j] = origin[j] + magnitude * 0.1 * crand();
        }
        particle->vel = vec3_scale(dir, magnitude); 
        float distance = crand() * magnitude / 3;
        particle->vel = vec3_fmaf(particle->vel, distance, right);
        distance = crand() * magnitude / 3;
        particle->vel = vec3_fmaf(particle->vel, distance, up);

        particle->acceleration.x = particle->acceleration.y = 0;
        particle->acceleration.z = -ParticleEffects::ParticleGravity / 2;
        particle->alpha = 1.0;

        particle->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}