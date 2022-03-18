/***
*
*	License here.
*
*	@file
*
*	Dirt and Sparks particle effect implementation.
* 
***/
#include "../../ClientGameLocal.h"
#include "../../Main.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Dirt' and 'Sparks' like particle effect.
**/
void ParticleEffects::DirtAndSparks(const vec3_t& origin, const vec3_t& direction, int32_t color, int32_t count) {
    vec3_t oy = {0.f, 1.f, 0.f};
    if (fabs(vec3_dot(oy, direction)) > 0.95f) {
        oy = vec3_t{1.0f, 0.0f, 0.0f};
    }

    vec3_t ox = vec3_cross(oy, direction);
    
    count *= Particles::GetParticleNumberFactor();
    const int32_t spark_count = count / 10;

    const float dirt_horizontal_spread = 2.0f;
    const float dirt_vertical_spread = 1.0f;
    const float dirt_base_velocity = 40.0f;
    const float dirt_rand_velocity = 70.0f;

    const float spark_horizontal_spread = 1.0f;
    const float spark_vertical_spread = 1.0f;
    const float spark_base_velocity = 50.0f;
    const float spark_rand_velocity = 130.0f;

    for (int i = 0; i < count; i++) {
        cparticle_t* dirtParticle = Particles::GetFreeParticle();
        if (!dirtParticle) {
            return;
        }

        dirtParticle->time = cl->time;

        dirtParticle->color = color + (rand() & 7);
        dirtParticle->brightness = 0.5f;

        dirtParticle->org = origin;
        dirtParticle->org = vec3_fmaf(dirtParticle->org, dirt_horizontal_spread * crand(), ox);
        dirtParticle->org = vec3_fmaf(dirtParticle->org, dirt_horizontal_spread * crand(), oy);
        dirtParticle->org = vec3_fmaf(dirtParticle->org, dirt_vertical_spread * frand() + 1.0f, direction);

        dirtParticle->vel = vec3_scale(vec3_normalize(dirtParticle->org - origin), dirt_base_velocity + frand() * dirt_rand_velocity);

        dirtParticle->acceleration[0] = dirtParticle->acceleration[1] = 0;
        dirtParticle->acceleration[2] = -ParticleEffects::ParticleGravity;
        dirtParticle->alpha = 1.0;

        dirtParticle->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }

    for (int i = 0; i < spark_count; i++) {
        cparticle_t* sparkParticle = Particles::GetFreeParticle();
        if (!sparkParticle) {
            return;
        }

        sparkParticle->time = cl->time;

        sparkParticle->color = 0xe0 + (rand() & 7);
        sparkParticle->brightness = Particles::GetParticleEmissiveFactor();

        sparkParticle->org = origin;
        sparkParticle->org = vec3_fmaf(sparkParticle->org, spark_horizontal_spread * crand(), ox);
        sparkParticle->org = vec3_fmaf(sparkParticle->org, spark_horizontal_spread * crand(), oy);
        sparkParticle->org = vec3_fmaf(sparkParticle->org, spark_vertical_spread * frand() + 1.0f, direction);

        sparkParticle->vel = vec3_scale(vec3_normalize(sparkParticle->org - origin), spark_base_velocity + powf(frand(), 2.0f) * spark_rand_velocity);

        sparkParticle->acceleration[0] = sparkParticle->acceleration[1] = 0;
        sparkParticle->acceleration[2] = -ParticleEffects::ParticleGravity;
        sparkParticle->alpha = 1.0;

        sparkParticle->alphavel = -2.0 / (0.5 + frand() * 0.3);
    }
}