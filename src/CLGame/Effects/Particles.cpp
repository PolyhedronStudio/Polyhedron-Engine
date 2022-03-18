/***
*
*	License here.
*
*	@file
*
*	Client Game View Interface Implementation.
* 
***/
#include "../ClientGameLocal.h"

#include "../Entities.h"
#include "../Main.h"
#include "../TemporaryEntities.h"

#include "Shared/Interfaces/IClientGameExports.h"
#include "../ClientGameExports.h"
#include "../Exports/View.h"

#include "ParticleEffects.h"
#include "Particles.h"


//! Particle emissive factor.
cvar_t *Particles::cvar_pt_particle_emissive = nullptr;

//! Particle amount scale factor.
cvar_t *Particles::cl_particle_num_factor = nullptr;

//! Precalculated angular velocities.
vec3_t Particles::angularVelocities[MaxAngularVelocities] = {};

//! Pointer to active particles.
cparticle_t *Particles::activeParticles;
//! Pointer to free particles.
cparticle_t *Particles::freeParticles;

//! Actual particle storage array.
cparticle_t  Particles::particles[MAX_PARTICLES] = {};


/**
*   @brief  Prepare the angular direction normals.
**/
void Particles::Initialize() {
    // Fetch cvars.
    cvar_pt_particle_emissive = clgi.Cvar_Get("pt_particle_emissive", "1.0", 0);
    cl_particle_num_factor = clgi.Cvar_Get("cl_particle_num_factor", "1", 0);

    // Generate a random numbered angular velocities table.
    // This is used for rotations etc, they vary each different game session. (Client, not server.)
    for (int32_t i = 0; i < NUMVERTEXNORMALS; i++) {
        angularVelocities[i] = { (rand() & 255) * 0.01f, (rand() & 255) * 0.01f, (rand() & 255) * 0.01f };
    }
}

/**
*   @brief  Nothing for now, placeholder stub.
*/
void Particles::Shutdown() {

}

/**
*   @brief  Clear/Reset the particles for a new frame.
**/
void Particles::Clear() {
    // Reset the free particles pointer. (Defaults to index 0 ofc.)
    freeParticles = &particles[0];
    // Reset active to nullptr.
    activeParticles = nullptr;

    // Reset the next particle pointers for each one of them.
    for (int32_t i = 0; i < cl_numparticles; i++) {
        particles[i].next = &particles[i + 1];
    }

    // Last one gets a nullptr signifying the end.
    particles[cl_numparticles - 1].next = nullptr;
}

/**
*   @brief  Looks for a free particle slot in the current frame.
*   @return Returns a valid pointer to the slot on success. nullptr in case of failure.
**/
cparticle_t* Particles::GetFreeParticle() {
    // No more free particles left, return nullptr.
    if (!freeParticles) {
        return nullptr;
    }

    // Assign the first free particle in town.
    cparticle_t *particle = freeParticles;

    // Increment its pointer to the next free particle.
    freeParticles = particle->next;

    // For the currently "allocated" particle we set the next pointer to the first active
    // particle one. 
    particle->next = activeParticles;

    // Update the active particle pointer to match our current fresh particle.
    activeParticles = particle;

    // Last but not least, return it.
    return particle;
}

void Particles::AddParticlesToView(void)
{
    cparticle_t* particle, * nextParticle;
    float           alpha;
    float           time = 0, time2;
    int             color;
    cparticle_t* active, * tail;
    rparticle_t renderParticle;

    active = NULL;
    tail = NULL;

    for (particle = activeParticles; particle; particle = nextParticle) {
        nextParticle = particle->next;

        if (particle->alphavel != ParticleEffects::InstantParticle) {
            time = (cl->time - particle->time) * 0.001;
            alpha = particle->alpha + time * particle->alphavel;

            if (alpha <= 0) {
                // Faded out.
                particle->next = freeParticles;
                freeParticles = particle;
                continue;
            }
        }
        else {
            alpha = particle->alpha;
        }

        particle->next = nullptr;
        if (!tail)
            active = tail = particle;
        else {
            tail->next = particle;
            tail = particle;
        }

        if (alpha > 1.0)
            alpha = 1;
        color = particle->color;

        time2 = time * time;

        renderParticle.origin = particle->org + particle->vel * time + particle->acceleration * time2;

        if (color == -1) {
            renderParticle.rgba.u8[0] = particle->rgba.u8[0];
            renderParticle.rgba.u8[1] = particle->rgba.u8[1];
            renderParticle.rgba.u8[2] = particle->rgba.u8[2];
            renderParticle.rgba.u8[3] = particle->rgba.u8[3] * alpha;
        }

        renderParticle.color = color;
        renderParticle.brightness = particle->brightness;
        renderParticle.alpha = alpha;
        renderParticle.radius = 0.f;

        if (particle->alphavel == ParticleEffects::InstantParticle) {
            particle->alphavel = 0.0;
            particle->alpha = 0.0;
        }

        // Break out the first second we even notice there's no more slots left.
        if (!clge->view->AddRenderParticle(renderParticle)) {
            break;
        }
    }

    activeParticles = active;
}