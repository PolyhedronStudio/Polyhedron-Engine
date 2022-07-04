/***
*
*	License here.
*
*	@file
*
*	Item Respawn particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'ItemRespawn' like particle effect.
**/
void ParticleEffects::ItemRespawn(const vec3_t& origin) {
    const int count = 64 * Particles::GetParticleNumberFactor();

    for (int32_t i = 0; i < count; i++) {
        cparticle_t *particle = Particles::GetFreeParticle();
        if (!particle) {
            return;
        }

        particle->time = cl->time;

        particle->color = 0xd4 + (rand() & 3); // green
        particle->brightness = 1.0f;

        particle->org.x = origin.x + crand() * 8;
        particle->org.y = origin.y + crand() * 8;
        particle->org.z = origin.z + crand() * 8;

        for (int32_t j = 0; j < 3; j++) {
            particle->vel[j] = crand() * 8;
        }

        particle->acceleration.x = particle->acceleration.y = 0;
        particle->acceleration.z = -ParticleEffects::ParticleGravity * 0.2;
        particle->alpha = 1.0;

        particle->alphavel = -1.0 / (1.0 + frand() * 0.3);
    }
}