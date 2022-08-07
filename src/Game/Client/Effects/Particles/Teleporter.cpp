/***
*
*	License here.
*
*	@file
*
*	Teleporter  particle effect implementation.
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
*   @brief  'Teleporter' like particle effect.
**/
void ParticleEffects::Teleporter(const vec3_t& origin) {
    const int count = 8 * Particles::GetParticleNumberFactor();

    for (int32_t i = 0; i < count; i++) {
        cparticle_t *particle = Particles::GetFreeParticle();
        if (!particle ) {
            return;
        }

        particle ->time = cl->time;

        particle ->color = 0xdb;
        particle ->brightness = 1.0f;

        for (int32_t j = 0; j < 2; j++) {
            particle ->org[j] = origin[j] - 16 + (rand() & 31);
            particle ->vel[j] = crand() * 14;
        }

        particle ->org.z = origin.z - 8 + (rand() & 7);
        particle ->vel.z = 80 + (rand() & 7);

        particle ->acceleration.x = particle ->acceleration.y = 0;
        particle ->acceleration.z = -ParticleEffects::ParticleGravity;
        particle ->alpha = 1.0;

        particle ->alphavel = -0.5;
    }
}