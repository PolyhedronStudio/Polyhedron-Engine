/***
*
*	License here.
*
*	@file
*
*	Monster Plasma Shell particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"
#include "../../Main.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Monster Plasma Shell' like particle effect.
**/
void ParticleEffects::MonsterPlasmaShell(const vec3_t& origin) {
    vec3_t direction = vec3_zero();

    for (int32_t i = 0; i < 40; i++) {
        cparticle_t *particle = Particles::GetFreeParticle();
        if (!particle) {
            return;
        }

        particle->acceleration = vec3_zero();
        particle->time = cl->time;
        particle->alpha = 1.0;
        particle->alphavel = ParticleEffects::InstantParticle;
        particle->color = 0xe0;
        particle->org = vec3_fmaf(origin, 10, vec3_t {
            static_cast<float>(crand()), 
            static_cast<float>(crand()), 
            static_cast<float>(crand())
        });
    }
}