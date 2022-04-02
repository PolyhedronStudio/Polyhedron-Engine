/***
*
*	License here.
*
*	@file
*
*	Blood Splatter particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"


/**
*   @brief  'Blood Splatters' like particle effect.
**/
void ParticleEffects::BloodSplatters(const vec3_t &origin, const vec3_t &direction, int32_t color, int32_t count) {
    // TODO: Currently unsupported, them decals :-)
    // Add decal:
    decal_t dec = {
      {origin[0],origin[1],origin[2]}, // .pos = 
      {direction[0],direction[1],direction[2]}, // .direction = 
      0.25f, // .spread = 
      350  // .length = 
    };
    //decal_t dec = {
    //  .pos = {origin[0],origin[1],origin[2]},
    //  .direction = {direction[0],direction[1],direction[2]},
    //  .spread = 0.25f,
    //  .length = 350 };
    clgi.R_AddDecal(&dec);

    float a[3] = { direction[1], -direction[2], direction[0] };
    float b[3] = { -direction[2], direction[0], direction[1] };

    count *= Particles::GetParticleNumberFactor();

    for (int32_t i = 0; i < count; i++) {
        cparticle_t *splatterParticle = Particles::GetFreeParticle();

        if (!splatterParticle) {
            return;
        }

        splatterParticle->time = cl->time;

        splatterParticle->color = color + (rand() & 7);
        splatterParticle->brightness = 0.5f;

        float d = (rand() & 31) * 10.0f;
        for (int32_t j = 0; j < 3; j++) {
            splatterParticle->org[j] = origin[j] + ((int)(rand() & 7) - 4) + d * (direction[j]
                + a[j] * 0.5f * ((rand() & 31) / 64.0f - .5f)
                + b[j] * 0.5f * ((rand() & 31) / 64.0f - .5f));

            splatterParticle->vel[j] = 10.0f * direction[j] + crand() * 20;
        }

        // Fake gravity
        splatterParticle->org[2] -= d * d * .001f;

        splatterParticle->acceleration[0] = splatterParticle->acceleration[1] = 0;
        splatterParticle->acceleration[2] = -ParticleEffects::ParticleGravity;
        splatterParticle->alpha = 0.5;

        splatterParticle->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}