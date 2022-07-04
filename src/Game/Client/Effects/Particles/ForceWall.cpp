/***
*
*	License here.
*
*	@file
*
*	Force Wall particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Force Wall' particle effect.
**/
void ParticleEffects::ForceWall(const vec3_t &start, const vec3_t &end, int32_t color) {
    vec3_t move = start;
    vec3_t vec = end - start;
    float len = VectorNormalize(vec);

    vec = vec3_scale(vec, 4.f);

    // FIXME: this is a really silly way to have a loop
    while (len > 0) {
        len -= 4;

        if (frand() > 0.3) {
            cparticle_t *particle = Particles::GetFreeParticle();
            if (!particle) {
                return;
            }

            particle->acceleration = vec3_zero();

            particle->time = cl->time;

            particle->alpha = 1.0;
            particle->alphavel = -1.0 / (3.0 + frand() * 0.5);
            particle->color = color;
            for (int32_t j = 0; j < 3; j++) {
                particle->org[j] = move[j] + crand() * 3;
                particle->acceleration[j] = 0;
            }
            particle->vel.x = 0;
            particle->vel.y = 0;
            particle->vel.z = -40 - (crand() * 10);
        }

        move += vec;
    }
}