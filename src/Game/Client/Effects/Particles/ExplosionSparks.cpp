/***
*
*	License here.
*
*	@file
*
*	Explosion Sparks particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Explosion Sparks' like particle effect.
**/
void ParticleEffects::ExplosionSparks(const vec3_t& origin) {
    const int count = 256 * Particles::GetParticleNumberFactor();

    for (int32_t i = 0; i < count; i++) {
        cparticle_t *sparkParticle = Particles::GetFreeParticle();
        if (!sparkParticle) {
            return;
        }

        sparkParticle->time = cl->time;

        sparkParticle->color = 0xe0 + (rand() & 7);
        sparkParticle->brightness = Particles::GetParticleEmissiveFactor();

        for (int32_t j = 0; j < 3; j++) {
            sparkParticle->org[j] = origin[j] + ((int)(rand() % 32) - 16);
            sparkParticle->vel[j] = (int)(rand() % 384) - 192;
        }

        sparkParticle->acceleration[0] = sparkParticle->acceleration[1] = 0;
        sparkParticle->acceleration[2] = -ParticleEffects::ParticleGravity;
        sparkParticle->alpha = 1.0;

        sparkParticle->alphavel = -0.8 / (0.5 + frand() * 0.3);
    }
}