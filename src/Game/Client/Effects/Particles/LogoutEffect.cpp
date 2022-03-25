/***
*
*	License here.
*
*	@file
*
*	Logout particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"
#include "../../Main.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Logout' like particle effect.
**/
void ParticleEffects::Logout(const vec3_t& origin, int32_t type) {
    for (int32_t i = 0; i < 500; i++) {
        cparticle_t *particle = Particles::GetFreeParticle();
        if (!particle) {
            return;
        }

        particle->time = cl->time;

        int32_t color = 0xe0 + (rand() & 7); // yellow
        if (type == MuzzleFlashType::Login) {
            color = 0xd0 + (rand() & 7); // green
        } else if (type == MuzzleFlashType::Logout) {
            color = 0x40 + (rand() & 7); // red
        }


        particle->color = color;
        particle->brightness = 1.0f;

        particle->org.x = origin.x - 16 + frand() * 32;
        particle->org.y = origin.y - 16 + frand() * 32;
        particle->org.z = origin.z - 24 + frand() * 56;

        for (int32_t j = 0; j < 3; j++) {
            particle->vel[j] = crand() * 20;
        }

        particle->acceleration.x = particle->acceleration.y = 0;
        particle->acceleration.z = -ParticleEffects::ParticleGravity;
        particle->alpha = 1.0;

        particle->alphavel = -1.0 / (1.0 + frand() * 0.3);
    }
}