/***
*
*	License here.
*
*	@file
*
*	Client Game Particle List.
* 
***/
#pragma once


//---------------------------------------------------------------------
// Client Game View IMPLEMENTATION.
//---------------------------------------------------------------------
class Particles {
public:
    friend class ClientGameView;

	//! Destructor.
    virtual ~Particles() = default;

    /**
    *
    *    View Management Functions.
    * 
    **/
    /**
    *   @brief  Prepare the angular direction normals.
    **/
    static void Initialize();

    /**
    *   @brief  Nothing for now, placeholder stub.
    */
    static void Shutdown();

    /**
    *   @brief  Clear/Reset the particles for a new frame.
    **/
    static void Clear();

    /**
    *   @brief  Looks for a free particle slot in the current frame.
    *   @return Returns a valid pointer to the slot on success. nullptr in case of failure.
    **/
    static cparticle_t *GetFreeParticle();

    /**
    *   @brief  Adds the 
    **/
    static void AddParticlesToView();

    /**
    *   @brief  Returns particle num factor cvar value.
    **/
    inline static float GetParticleNumberFactor() {
        return cl_particle_num_factor->value;
    }

    /**
    *   @brief  Returns particle emissive factor cvar value.
    **/
    inline static float GetParticleEmissiveFactor() {
        return cvar_pt_particle_emissive->value;
    }

private:
    /**
    *   CVars.
    **/
    static cvar_t *cvar_pt_particle_emissive;
    static cvar_t *cl_particle_num_factor;

    //! Max number of angular velocity normals.
    static constexpr int32_t MaxAngularVelocities = 162;
    //! Precalculated angular velocities.
    static vec3_t angularVelocities[MaxAngularVelocities];

    //! Pointer to active particles.
    static cparticle_t *activeParticles;
    //! Pointer to free particles.
    static cparticle_t *freeParticles;

    //! Actual particle storage array.
    static cparticle_t  particles[MAX_PARTICLES];
    static constexpr int32_t cl_numparticles = MAX_PARTICLES;
};
