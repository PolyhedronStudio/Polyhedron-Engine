/***
*
*	License here.
*
*	@file
*
*	Bubble Trail B particle effect implementation.
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
*   @brief  'Bubble Trail B' like particle effect.
**/
void ParticleEffects::BubbleTrailB(const vec3_t &start, const vec3_t &end, int32_t distance) {
    vec3_t move = start;
    vec3_t vec = end - start;
    float len = VectorNormalize(vec);

    float dec = distance;
    VectorScale(vec, dec, vec);

    for (int32_t i = 0; i < len; i += dec) {
        cparticle_t *trailParticle = Particles::GetFreeParticle();
        if (!trailParticle) {
            return;
        }

        trailParticle->acceleration = vec3_zero();
        trailParticle->time = cl->time;

        trailParticle->alpha = 1.0;
        trailParticle->alphavel = -1.0 / (1 + frand() * 0.1);
        trailParticle->color = 4 + (rand() & 7);
        for (int32_t j = 0; j < 3; j++) {
            trailParticle->org[j] = move[j] + crand() * 2;
            trailParticle->vel[j] = crand() * 10;
        }
        trailParticle->org[2] -= 4;
        trailParticle->vel[2] += 20;

        move += vec;
    }
}