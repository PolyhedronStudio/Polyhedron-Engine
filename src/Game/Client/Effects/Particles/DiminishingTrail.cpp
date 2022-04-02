/***
*
*	License here.
*
*	@file
*
*	Diminishing Trail particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Diminishing Trail' like particle effect.
**/
void ParticleEffects::DiminishingTrail(const vec3_t &start, const vec3_t &end, ClientEntity *oldTrailEntity, int32_t flags) {
    float originScale   = 0.f;
    float velocityScale = 0.f;

    vec3_t move = start;
    vec3_t vec = end - start;
    float len = VectorNormalize(vec);

    float dec = 0.5f;
    vec = vec3_scale(vec, dec);

    if (oldTrailEntity->trailCount > 900) {
        originScale     = 4;
        velocityScale   = 15;
    } else if (oldTrailEntity->trailCount > 800) {
        originScale     = 2;
        velocityScale   = 10;
    } else {
        originScale     = 1;
        velocityScale   = 5;
    }

    while (len > 0) {
        len -= dec;

        // drop less particles as it flies
        if ((rand() & 1023) < oldTrailEntity->trailCount) {
            cparticle_t *particle = Particles::GetFreeParticle();
            if (!particle) {
                return;
            }

            particle->acceleration = vec3_zero();

            particle->time = cl->time;

            if (flags & EntityEffectType::Gib) {
                particle->alpha = 1.0;
                particle->alphavel = -1.0 / (1 + frand() * 0.4);

                particle->color = 0xe8 + (rand() & 7);
                particle->brightness = 1.0f;

                for (int32_t j = 0; j < 3; j++) {
                    particle->org[j] = move[j] + crand() * originScale;
                    particle->vel[j] = crand() * velocityScale;
                    particle->acceleration[j] = 0;
                }
                particle->vel[2] -= ParticleEffects::ParticleGravity;
            } else {
                particle->alpha = 1.0;
                particle->alphavel = -1.0 / (1 + frand() * 0.2);

                particle->color = 4 + (rand() & 7);
                particle->brightness = 1.0f;

                for (int32_t j = 0; j < 3; j++) {
                    particle->org[j] = move[j] + crand() * originScale;
                    particle->vel[j] = crand() * velocityScale;
                }
                particle->acceleration[2] = 20;
            }
        }

        oldTrailEntity->trailCount -= 5;
        if (oldTrailEntity->trailCount < 100) {
            oldTrailEntity->trailCount = 100;
        }
        move += vec;
    }
}