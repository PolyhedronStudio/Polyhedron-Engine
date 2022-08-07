/***
*
*	License here.
*
*	@file
*
*	Bubble Trail A particle effect implementation.
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
*   @brief  'Bubble Trail A' like particle effect.
**/
void ParticleEffects::BubbleTrailA(const vec3_t &start, const vec3_t &end) {
    vec3_t move = start;
    vec3_t vec = end - start;
    float len = VectorNormalize(vec);

    float dec = 32.f;
    vec = vec3_scale(vec, dec);

    for (int32_t i = 0; i < len; i += dec) {
        cparticle_t* trailParticle = Particles::GetFreeParticle();
        if (!trailParticle) {
            return;
        }

        trailParticle->acceleration = vec3_zero();
        trailParticle->time = cl->time;

        trailParticle->alpha = 1.0;
        trailParticle->alphavel = -1.0 / (1 + frand() * 0.2);

        trailParticle->color = 4 + (rand() & 7);
        trailParticle->brightness = 1.0f;

        for (int32_t j = 0; j < 3; j++) {
            trailParticle->org[j] = move[j] + crand() * 2;
            trailParticle->vel[j] = crand() * 5;
        }
        trailParticle->vel[2] += 6;

        move += vec;
    }
}