// LICENSE HERE.

//
// clg_effects.c
//
//
// Contains code for all special effects, simple steam leaking particles to
// awesome big banging explosions!
//
#include "ClientGameLocal.h"

#include "Effects.h"
#include "Main.h"
#include "NewEffects.h"
#include "TemporaryEntities.h"
#include "View.h"

static void CLG_LogoutEffect(vec3_t org, int type);

static vec3_t avelocities[NUMVERTEXNORMALS];

static cparticle_t* active_particles, * free_particles;

static cparticle_t  particles[MAX_PARTICLES];
static const int    cl_numparticles = MAX_PARTICLES;

static void CLG_ClearParticles(void);

static cdlight_t       clg_dlights[MAX_DLIGHTS];
static void CLG_ClearDLights(void);

uint32_t d_8to24table[256];

cvar_t* cvar_pt_particle_emissive = NULL;
static cvar_t* cl_particle_num_factor = NULL;

//
//===============
// CLG_EffectsInit
// 
// Initializes the effects.
// ===============
//
void CLG_EffectsInit(void)
{
    int i, j; 

    // Fetch cvars.
    cvar_pt_particle_emissive = clgi.Cvar_Get("pt_particle_emissive", "10.0", 0);
    cl_particle_num_factor = clgi.Cvar_Get("cl_particle_num_factor", "1", 0);

    // Generate a random numbered angular velocities table.
    // This is used for rotations etc, so they vary each time
    // around the user is playing.
    for (i = 0; i < NUMVERTEXNORMALS; i++)
        for (j = 0; j < 3; j++)
            avelocities[i][j] = (rand() & 255) * 0.01f;
}

//
//===============
// CLG_ClearEffects
// 
// Clears effects.
// ===============
//
void CLG_ClearEffects(void)
{
    CLG_ClearParticles();
    CLG_ClearDLights();
}


//
//=============================================================================
//
//	LIGHTSTYLE MANAGEMENT
//
//=============================================================================
//

#if USE_LIGHTSTYLES

struct ClientLightstyle {
    list_t  entry;
    int     length;
    vec4_t  value;
    float   map[MAX_QPATH];
};

static ClientLightstyle    cl_lightstyles[MAX_LIGHTSTYLES];
static LIST_DECL(cl_lightlist);
static int          cl_lastofs;

void CLG_ClearLightStyles(void)
{
    int     i;
    ClientLightstyle* ls;

    for (i = 0, ls = cl_lightstyles; i < MAX_LIGHTSTYLES; i++, ls++) {
        List_Init(&ls->entry);
        ls->length = 0;
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = 1;
    }

    List_Init(&cl_lightlist);
    cl_lastofs = -1;
}

/*
================
CLG_RunLightStyles
================
*/
void CLG_RunLightStyles(void)
{
    int     ofs;
    ClientLightstyle* ls;

    ofs = cl->time / 50;
    if (ofs == cl_lastofs)
        return;
    cl_lastofs = ofs;

    LIST_FOR_EACH(ClientLightstyle, ls, &cl_lightlist, entry) {
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = ls->map[ofs % ls->length];
    }
}

void CLG_SetLightStyle(int index, const char* s)
{
    int     i;
    ClientLightstyle* ls;

    ls = &cl_lightstyles[index];
    ls->length = strlen(s);
    if (ls->length > MAX_QPATH) {
        Com_Error(ERR_DROP, "%s: oversize style", __func__);
    }

    for (i = 0; i < ls->length; i++) {
        ls->map[i] = (1.0f / 26.f) * (float)((float)s[i] - (float)'a'); //(float)(s[i] - 'a') / (float)('m' - 'a'); //0.2f;  //(1.0f / 26.f)* (float)((float)s[i] - (float)'a');//;(float)(s[i] - 'a') / (float)('z' - 'a');
    }

    if (ls->entry.prev) {
        List_Delete(&ls->entry);
    }

    if (ls->length > 1) {
        List_Append(&cl_lightlist, &ls->entry);
        return;
    }

    if (ls->length == 1) {
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = ls->map[0];
        return;
    }

    ls->value[0] =
        ls->value[1] =
        ls->value[2] =
        ls->value[3] = 1;
}

/*
================
CLG_AddLightStyles
================
*/
void CLG_AddLightStyles(void)
{
    int     i;
    ClientLightstyle* ls;

    for (i = 0, ls = cl_lightstyles; i < MAX_LIGHTSTYLES; i++, ls++)
        V_AddLightStyle(i, ls->value);
}

#endif


//
//=============================================================================
//
//	DLIGHT EFFECT HANDLING.
//
//=============================================================================
//

static cdlight_t       cl_dlights[MAX_DLIGHTS];

static void CLG_ClearDLights(void)
{
    memset(cl_dlights, 0, sizeof(cl_dlights));
}

//
//===============
// CLG_AllocDLight
// 
// Returns a pointer to an allocated dlight, in case there is room.
// ===============
//
cdlight_t* CLG_AllocDLight(int key)
{
    int     i;
    cdlight_t* dl;

    // first look for an exact key match
    if (key) {
        dl = cl_dlights;
        for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
            if (dl->key == key) {
                memset(dl, 0, sizeof(*dl));
                dl->key = key;
                return dl;
            }
        }
    }

    // then look for anything else
    dl = cl_dlights;
    for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
        if (dl->die < cl->time) {
            memset(dl, 0, sizeof(*dl));
            dl->key = key;
            return dl;
        }
    }

    dl = &cl_dlights[0];
    memset(dl, 0, sizeof(*dl));
    dl->key = key;
    return dl;
}

//
//===============
// CLG_RunDLights
// 
// Runs each dlight for the current frame.
// ===============
//
void CLG_RunDLights(void)
{
    int         i;
    cdlight_t* dl;

    if (sv_paused->integer)
    {
        // Don't update the persistent dlights when the game is paused (e.g. photo mode).
        // Use sv_paused here because cl_paused can be nonzero in network play,
        // but the game is not really paused in that case.

        return;
    }

    dl = cl_dlights;
    for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
        if (!dl->radius)
            continue;

        if (dl->die < cl->time) {
            dl->radius = 0;
            return;
        }

        dl->radius -= clgi.GetFrameTime() * dl->decay;
        if (dl->radius < 0)
            dl->radius = 0;

        dl->origin = vec3_fmaf(dl->origin, clgi.GetFrameTime(), dl->velocity);
    }
}

//
//===============
// CLG_AddDLights
// 
// Add current frame scene dlights to the view being passed to the refresh.
// ===============
//
void CLG_AddDLights(void)
{
    int         i;
    cdlight_t* dl;

    dl = cl_dlights;
    for (i = 0; i < MAX_DLIGHTS; i++, dl++) {
        if (!dl->radius)
            continue;
        V_AddLight(dl->origin, dl->radius,
            dl->color[0], dl->color[1], dl->color[2]);
    }
}



//
//=============================================================================
//
//	MUZZLE FLASHES.
//
//=============================================================================
//
//
//===============
// CLG_MuzzleFlash
// 
// Handle the MuzzleFlash messages for the client player.
//===============
//
void CLG_MuzzleFlash() {
    vec3_t      fv, rv;
    cdlight_t* dl;
    ClientEntity* pl;
    float       volume;
    char        soundname[MAX_QPATH];

    //#ifdef _DEBUG
    //    if (developer->integer)
    //        CL_CheckEntityPresent(mzParameters.entity, "muzzleflash");
    //#endif

    pl = &cs->entities[mzParameters.entity];

    dl = CLG_AllocDLight(mzParameters.entity);
    dl->origin = pl->current.origin, dl->origin;
    AngleVectors(pl->current.angles, &fv, &rv, NULL);
    dl->origin = vec3_fmaf(dl->origin, 18, fv);
    dl->origin = vec3_fmaf(dl->origin, 16, rv);
    if (mzParameters.silenced)
        dl->radius = 100 + (rand() & 31);
    else
        dl->radius = 200 + (rand() & 31);
    //dl->minlight = 32;
    dl->die = cl->time; // + 0.1;

    if (mzParameters.silenced)
        volume = 0.2;
    else
        volume = 1;

    switch (mzParameters.weapon) {
    case MuzzleFlashType::Blaster:
        dl->color = vec3_t{1, 1, 0};
        //clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/blastf1a.wav"), volume, Attenuation::Normal, 0);
	    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/v_mark23/fire0.wav"), volume, Attenuation::Normal, 0);
        break;
    case MuzzleFlashType::MachineGun:
        dl->color = vec3_t{1, 1, 0};
        Q_snprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound(soundname), volume, Attenuation::Normal, 0);
        break;
    case MuzzleFlashType::Shotgun:
        dl->color = vec3_t{1, 1, 0};
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/shotgf1b.wav"), volume, Attenuation::Normal, 0);
        //  S_StartSound(NULL, mzParameters.entity, SoundChannel::Auto,   S_RegisterSound("weapons/shotgr1b.wav"), volume, Attenuation::Normal, 0.1);
        break;
    case MuzzleFlashType::SuperShotgun:
        dl->color = vec3_t{1, 1, 0};
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/sshotf1b.wav"), volume, Attenuation::Normal, 0);
        break;
    case MuzzleFlashType::Login:
        dl->color = vec3_t{0, 1, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/grenlf1a.wav"), 1, Attenuation::Normal, 0);
        CLG_LogoutEffect(pl->current.origin, mzParameters.weapon);
        break;
    case MuzzleFlashType::Logout:
        dl->color = vec3_t{1, 0, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/grenlf1a.wav"), 1, Attenuation::Normal, 0);
        CLG_LogoutEffect(pl->current.origin, mzParameters.weapon);
        break;
    case MuzzleFlashType::Respawn:
        dl->color = vec3_t{1, 1, 0};
        dl->die = cl->time + 1.0f;
        clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("weapons/grenlf1a.wav"), 1, Attenuation::Normal, 0);
        CLG_LogoutEffect(pl->current.origin, mzParameters.weapon);
        break;
    }

    if (vid_rtx->integer)
    {
        // don't add muzzle flashes in RTX mode
        dl->radius = 0.f;
    }
}

//
//===============
// CLG_MuzzleFlash2
// 
// Handle the MuzzleFlash messages for the monster entities.
//===============
//
void CLG_MuzzleFlash2() {
    //ClientEntity* ent;
    //vec3_t      origin;
    //const vec_t* ofs;
    //cdlight_t* dl;
    //vec3_t      forward, right;

    //// locate the origin
    //ent = &cs->entities[mzParameters.entity];
    //AngleVectors(ent->current.angles, &forward, &right, NULL);
    //ofs = vec3_t { 10.6f * 1.2f, 7.7f * 1.2f, 7.8f * 1.2f };
    //origin[0] = ent->current.origin[0] + forward[0] * ofs[0] + right[0] * ofs[1];
    //origin[1] = ent->current.origin[1] + forward[1] * ofs[0] + right[1] * ofs[1];
    //origin[2] = ent->current.origin[2] + forward[2] * ofs[0] + right[2] * ofs[1] + ofs[2];

    //dl = CLG_AllocDLight(mzParameters.entity);
    //dl->origin = origin, dl->origin;
    //dl->radius = 200 + (rand() & 31);
    ////dl->minlight = 32;
    //dl->die = cl->time;  // + 0.1;

    //switch (mzParameters.weapon) {
    //case MZ2_SOLDIER_MACHINEGUN_1:
    //case MZ2_SOLDIER_MACHINEGUN_2:
    //case MZ2_SOLDIER_MACHINEGUN_3:
    //case MZ2_SOLDIER_MACHINEGUN_4:
    //case MZ2_SOLDIER_MACHINEGUN_5:
    //case MZ2_SOLDIER_MACHINEGUN_6:
    //case MZ2_SOLDIER_MACHINEGUN_7:
    //case MZ2_SOLDIER_MACHINEGUN_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    CLG_ParticleEffect(origin, forward, 0, 40);
    //    CLG_SmokeAndFlash(origin);
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck3.wav"), 1, Attenuation::Normal, 0);
    //    break;

    //case MZ2_SOLDIER_BLASTER_1:
    //case MZ2_SOLDIER_BLASTER_2:
    //case MZ2_SOLDIER_BLASTER_3:
    //case MZ2_SOLDIER_BLASTER_4:
    //case MZ2_SOLDIER_BLASTER_5:
    //case MZ2_SOLDIER_BLASTER_6:
    //case MZ2_SOLDIER_BLASTER_7:
    //case MZ2_SOLDIER_BLASTER_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck2.wav"), 1, Attenuation::Normal, 0);
    //    break;

    //case MZ2_SOLDIER_SHOTGUN_1:
    //case MZ2_SOLDIER_SHOTGUN_2:
    //case MZ2_SOLDIER_SHOTGUN_3:
    //case MZ2_SOLDIER_SHOTGUN_4:
    //case MZ2_SOLDIER_SHOTGUN_5:
    //case MZ2_SOLDIER_SHOTGUN_6:
    //case MZ2_SOLDIER_SHOTGUN_7:
    //case MZ2_SOLDIER_SHOTGUN_8:
    //    dl->color = vec3_t{1, 1, 0};
    //    CLG_SmokeAndFlash(origin);
    //    clgi.S_StartSound(NULL, mzParameters.entity, SoundChannel::Weapon, clgi.S_RegisterSound("soldier/solatck1.wav"), 1, Attenuation::Normal, 0);
    //    break;
    //}
}

//
//=============================================================================
//
//	PARTICLE EFFECTS
//
//=============================================================================
//

//
//===============
// CLG_ClearParticles
// 
// Clears particless.
// ===============
//
static void CLG_ClearParticles(void)
{
    int     i;

    free_particles = &particles[0];
    active_particles = NULL;

    for (i = 0; i < cl_numparticles; i++)
        particles[i].next = &particles[i + 1];
    particles[cl_numparticles - 1].next = NULL;
}

//
//===============
// CLG_AllocParticle
// 
// Allocate a new particle, if there is room.s
//===============
//
cparticle_t* CLG_AllocParticle(void)
{
    cparticle_t* p;

    if (!free_particles)
        return NULL;
    p = free_particles;
    free_particles = p->next;
    p->next = active_particles;
    active_particles = p;

    return p;
}

//
//===============
// CLG_ParticleEffect
// 
// Wall Impact Puffs
//===============
//
void CLG_ParticleEffect(vec3_t org, vec3_t dir, int color, int count)
{
    vec3_t oy = {0.f, 1.f, 0.f};
    if (fabs(vec3_dot(oy, dir)) > 0.95f)
        oy = vec3_t{1.0f, 0.0f, 0.0f};

    vec3_t ox = vec3_cross(oy, dir);
    
    count *= cl_particle_num_factor->value;
    const int spark_count = count / 10;

    const float dirt_horizontal_spread = 2.0f;
    const float dirt_vertical_spread = 1.0f;
    const float dirt_base_velocity = 40.0f;
    const float dirt_rand_velocity = 70.0f;

    const float spark_horizontal_spread = 1.0f;
    const float spark_vertical_spread = 1.0f;
    const float spark_base_velocity = 50.0f;
    const float spark_rand_velocity = 130.0f;

    for (int i = 0; i < count; i++) {
        cparticle_t* p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = color + (rand() & 7);
        p->brightness = 0.5f;

        vec3_t origin = org;
        origin = vec3_fmaf(origin, dirt_horizontal_spread * crand(), ox);
        origin = vec3_fmaf(origin, dirt_horizontal_spread * crand(), oy);
        origin = vec3_fmaf(origin, dirt_vertical_spread * frand() + 1.0f, dir);
        p->org = origin;

        vec3_t velocity;
        velocity = origin - org;
        VectorNormalize(velocity);
        p->vel = vec3_scale(velocity, dirt_base_velocity + frand() * dirt_rand_velocity);

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }

    for (int i = 0; i < spark_count; i++) {
        cparticle_t* p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xe0 + (rand() & 7);
        p->brightness = cvar_pt_particle_emissive->value;

        vec3_t origin = org;
        origin = vec3_fmaf(origin, spark_horizontal_spread * crand(), ox);
        origin = vec3_fmaf(origin, spark_horizontal_spread * crand(), oy);
        origin = vec3_fmaf(origin, spark_vertical_spread * frand() + 1.0f, dir);
        p->org = origin;

        vec3_t velocity;
        velocity = origin - org;
        VectorNormalize(velocity);
        p->vel = vec3_scale(velocity, spark_base_velocity + powf(frand(), 2.0f) * spark_rand_velocity);

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -2.0 / (0.5 + frand() * 0.3);
    }
}

void CLG_ParticleEffectWaterSplash(vec3_t org, vec3_t dir, int color, int count)
{
    vec3_t oy = vec3_t{0.f, 1.f, 0.f};
    if (fabs(vec3_dot(oy, dir)) > 0.95f)
        oy = vec3_t{1.0f, 0.0f, 0.0f};

    vec3_t ox = vec3_cross(oy, dir);

    count *= cl_particle_num_factor->value * 4;

    const float water_horizontal_spread = 0.35f;
    const float water_vertical_spread = 2.0f;
    const float water_base_velocity = 95.0f;
    const float water_rand_velocity = 150.0f;

    for (int i = 0; i < count; i++) {
        cparticle_t* p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = color + (rand() & 7);
        p->brightness = 1.0f;

        vec3_t origin = org;
        origin = vec3_fmaf(origin, water_horizontal_spread * crand(), ox);
        origin = vec3_fmaf(origin, water_horizontal_spread * crand(), oy);
        origin = vec3_fmaf(origin, water_vertical_spread * frand() + 1.0f, dir);
        p->org = origin;

        vec3_t velocity;
        velocity = origin - org;
        VectorNormalize(velocity);
        p->vel = vec3_scale(velocity, water_base_velocity + frand() * water_rand_velocity);

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}

void CLG_BloodParticleEffect(vec3_t org, vec3_t dir, int color, int count)
{
    int         i, j;
    cparticle_t* p;
    float       d;

    // add decal:
    // CPP: struct init.
    decal_t dec = {
      {org[0],org[1],org[2]}, // .pos = 
      {dir[0],dir[1],dir[2]}, // .dir = 
      0.25f, // .spread = 
      350  // .length = 
    };
    //decal_t dec = {
    //  .pos = {org[0],org[1],org[2]},
    //  .dir = {dir[0],dir[1],dir[2]},
    //  .spread = 0.25f,
    //  .length = 350 };
    clgi.R_AddDecal(&dec);

    float a[3] = { dir[1], -dir[2], dir[0] };
    float b[3] = { -dir[2], dir[0], dir[1] };

    count *= cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = color + (rand() & 7);
        p->brightness = 0.5f;

        d = (rand() & 31) * 10.0f;
        for (j = 0; j < 3; j++) {
            p->org[j] = org[j] + ((int)(rand() & 7) - 4) + d * (dir[j]
                + a[j] * 0.5f * ((rand() & 31) / 64.0f - .5f)
                + b[j] * 0.5f * ((rand() & 31) / 64.0f - .5f));

            p->vel[j] = 10.0f * dir[j] + crand() * 20;
        }
        // fake gravity
        p->org[2] -= d * d * .001f;

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 0.5;

        p->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}


/*
===============
CLG_ParticleEffect2
===============
*/
void CLG_ParticleEffect2(vec3_t org, vec3_t dir, int color, int count)
{
    int         i, j;
    cparticle_t* p;
    float       d;

    count *= cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = color;
        p->brightness = 1.0f;

        d = rand() & 7;
        for (j = 0; j < 3; j++) {
            p->org[j] = org[j] + ((int)(rand() & 7) - 4) + d * dir[j];
            p->vel[j] = crand() * 20;
        }

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}


/*
===============
CLG_TeleporterParticles
===============
*/
void CLG_TeleporterParticles(vec3_t org)
{
    int         i, j;
    cparticle_t* p;

    const int count = 8 * cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xdb;
        p->brightness = 1.0f;

        for (j = 0; j < 2; j++) {
            p->org[j] = org[j] - 16 + (rand() & 31);
            p->vel[j] = crand() * 14;
        }

        p->org[2] = org[2] - 8 + (rand() & 7);
        p->vel[2] = 80 + (rand() & 7);

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -0.5;
    }
}


/*
===============
CLG_LogoutEffect

===============
*/
static void CLG_LogoutEffect(vec3_t org, int type)
{
    int         i, j;
    cparticle_t* p;

    for (i = 0; i < 500; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        int color;
        if (type == MuzzleFlashType::Login)
            color = 0xd0 + (rand() & 7); // green
        else if (type == MuzzleFlashType::Logout)
            color = 0x40 + (rand() & 7); // red
        else
            color = 0xe0 + (rand() & 7); // yellow

        p->color = color;
        p->brightness = 1.0f;

        p->org[0] = org[0] - 16 + frand() * 32;
        p->org[1] = org[1] - 16 + frand() * 32;
        p->org[2] = org[2] - 24 + frand() * 56;

        for (j = 0; j < 3; j++)
            p->vel[j] = crand() * 20;

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (1.0 + frand() * 0.3);
    }
}


/*
===============
CLG_ItemRespawnParticles

===============
*/
void CLG_ItemRespawnParticles(vec3_t org)
{
    int         i, j;
    cparticle_t* p;

    const int count = 64 * cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xd4 + (rand() & 3); // green
        p->brightness = 1.0f;

        p->org[0] = org[0] + crand() * 8;
        p->org[1] = org[1] + crand() * 8;
        p->org[2] = org[2] + crand() * 8;

        for (j = 0; j < 3; j++)
            p->vel[j] = crand() * 8;

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY * 0.2;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (1.0 + frand() * 0.3);
    }
}


/*
===============
CLG_ExplosionParticles
===============
*/
void CLG_ExplosionParticles(vec3_t org)
{
    int         i, j;
    cparticle_t* p;

    const int count = 256 * cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xe0 + (rand() & 7);
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = org[j] + ((int)(rand() % 32) - 16);
            p->vel[j] = (int)(rand() % 384) - 192;
        }

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -0.8 / (0.5 + frand() * 0.3);
    }
}

/*
===============
CLG_BigTeleportParticles
===============
*/
void CLG_BigTeleportParticles(vec3_t org)
{
    static const byte   colortable[4] = { 2 * 8, 13 * 8, 21 * 8, 18 * 8 };
    int         i;
    cparticle_t* p;
    float       angle, dist;

    for (i = 0; i < 4096; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = colortable[rand() & 3];
        p->brightness = 1.0f;

        angle = M_PI * 2 * (rand() & 1023) / 1023.0;
        dist = rand() & 31;
        p->org[0] = org[0] + std::cosf(angle) * dist;
        p->vel[0] = std::cosf(angle) * (70 + (rand() & 63));
        p->acceleration[0] = -std::cosf(angle) * 100;

        p->org[1] = org[1] + std::sinf(angle) * dist;
        p->vel[1] = std::sinf(angle) * (70 + (rand() & 63));
        p->acceleration[1] = -std::sinf(angle) * 100;

        p->org[2] = org[2] + 8 + (rand() % 90);
        p->vel[2] = -100 + (rand() & 31);
        p->acceleration[2] = PARTICLE_GRAVITY * 4;
        p->alpha = 1.0;

        p->alphavel = -0.3 / (0.5 + frand() * 0.3);
    }
}


/*
===============
CLG_BlasterParticles

Wall impact puffs
===============
*/
void CLG_BlasterParticles(vec3_t org, vec3_t dir)
{
    int         i, j;
    cparticle_t* p;
    float       d;

    const int count = 400 * cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xe0 + (rand() & 7);
        p->brightness = cvar_pt_particle_emissive->value;

        d = rand() & 15;
        for (j = 0; j < 3; j++) {
            p->org[j] = org[j] + ((int)(rand() & 7) - 4) + d * dir[j];
            p->vel[j] = dir[j] * 30 + crand() * 40;
        }

        p->acceleration[0] = p->acceleration[1] = 0;
        p->acceleration[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -1.0 / (0.5 + frand() * 0.3);
    }
}


/*
===============
CLG_BlasterTrail

===============
*/
void CLG_BlasterTrail(vec3_t start, vec3_t end)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    int         dec;
    VectorCopy(start, move);
    VectorSubtract(end, start, vec);
    len = VectorNormalize(vec);

    dec = 5;
    VectorScale(vec, 5, vec);

    // FIXME: this is a really silly way to have a loop
    while (len > 0) {
        len -= dec;

        p = CLG_AllocParticle();
        if (!p)
            return;
        p->acceleration = vec3_zero();

        p->time = cl->time;

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.3 + frand() * 0.2);

        p->color = 0xe0;
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand();
            p->vel[j] = crand() * 5;
            p->acceleration[j] = 0;
        }

        move += vec;
    }
}

/*
===============
CLG_DiminishingTrail

===============
*/
void CLG_DiminishingTrail(vec3_t start, vec3_t end, ClientEntity* old, int flags)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    float       dec;
    float       orgscale;
    float       velscale;

    move = start;
    vec = end - start;
    len = VectorNormalize(vec);

    dec = 0.5;
    vec = vec3_scale(vec, dec);

    if (old->trailcount > 900) {
        orgscale = 4;
        velscale = 15;
    }
    else if (old->trailcount > 800) {
        orgscale = 2;
        velscale = 10;
    }
    else {
        orgscale = 1;
        velscale = 5;
    }

    while (len > 0) {
        len -= dec;

        // drop less particles as it flies
        if ((rand() & 1023) < old->trailcount) {
            p = CLG_AllocParticle();
            if (!p)
                return;
            VectorClear(p->acceleration);

            p->time = cl->time;

            if (flags & EntityEffectType::Gib) {
                p->alpha = 1.0;
                p->alphavel = -1.0 / (1 + frand() * 0.4);

                p->color = 0xe8 + (rand() & 7);
                p->brightness = 1.0f;

                for (j = 0; j < 3; j++) {
                    p->org[j] = move[j] + crand() * orgscale;
                    p->vel[j] = crand() * velscale;
                    p->acceleration[j] = 0;
                }
                p->vel[2] -= PARTICLE_GRAVITY;
            } else {
                p->alpha = 1.0;
                p->alphavel = -1.0 / (1 + frand() * 0.2);

                p->color = 4 + (rand() & 7);
                p->brightness = 1.0f;

                for (j = 0; j < 3; j++) {
                    p->org[j] = move[j] + crand() * orgscale;
                    p->vel[j] = crand() * velscale;
                }
                p->acceleration[2] = 20;
            }
        }

        old->trailcount -= 5;
        if (old->trailcount < 100)
            old->trailcount = 100;
        move += vec;
    }
}

/*
===============
CLG_BubbleTrail

===============
*/
void CLG_BubbleTrail(vec3_t start, vec3_t end)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         i, j;
    cparticle_t* p;
    float       dec;

    move = start;
    vec = end - start;
    len = VectorNormalize(vec);

    dec = 32;
    vec = vec3_scale(vec, dec);

    for (i = 0; i < len; i += dec) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->acceleration = vec3_zero();
        p->time = cl->time;

        p->alpha = 1.0;
        p->alphavel = -1.0 / (1 + frand() * 0.2);

        p->color = 4 + (rand() & 7);
        p->brightness = 1.0f;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand() * 2;
            p->vel[j] = crand() * 5;
        }
        p->vel[2] += 6;

        move += vec;
    }
}


/*
===============
CLG_TeleportParticles

===============
*/
void CLG_TeleportParticles(vec3_t org)
{
    int         i, j, k;
    cparticle_t* p;
    float       vel;
    vec3_t      dir;

    for (i = -16; i <= 16; i += 4)
        for (j = -16; j <= 16; j += 4)
            for (k = -16; k <= 32; k += 4) {
                p = CLG_AllocParticle();
                if (!p)
                    return;

                p->time = cl->time;

                p->color = 7 + (rand() & 7);
                p->brightness = 1.0f;

                p->alpha = 1.0;
                p->alphavel = -1.0 / (0.3 + (rand() & 7) * 0.02);

                p->org[0] = org[0] + i + (rand() & 3);
                p->org[1] = org[1] + j + (rand() & 3);
                p->org[2] = org[2] + k + (rand() & 3);

                dir[0] = j * 8;
                dir[1] = i * 8;
                dir[2] = k * 8;

                VectorNormalize(dir);
                vel = 50 + (rand() & 63);
                p->vel = vec3_scale(dir, vel);

                p->acceleration[0] = p->acceleration[1] = 0;
                p->acceleration[2] = -PARTICLE_GRAVITY;
            }
}

/*
===============
CLG_AddParticles
===============
*/
void CLG_AddParticles(void)
{
    cparticle_t* p, * next;
    float           alpha;
    float           time = 0, time2;
    int             color;
    cparticle_t* active, * tail;
    rparticle_t* part;

    active = NULL;
    tail = NULL;

    for (p = active_particles; p; p = next) {
        next = p->next;

        if (p->alphavel != INSTANT_PARTICLE) {
            time = (cl->time - p->time) * 0.001;
            alpha = p->alpha + time * p->alphavel;
            if (alpha <= 0) {
                // faded out
                p->next = free_particles;
                free_particles = p;
                continue;
            }
        }
        else {
            alpha = p->alpha;
        }

        if (view.num_particles >= MAX_PARTICLES)
            break;
        part = &view.particles[view.num_particles++];

        p->next = NULL;
        if (!tail)
            active = tail = p;
        else {
            tail->next = p;
            tail = p;
        }

        if (alpha > 1.0)
            alpha = 1;
        color = p->color;

        time2 = time * time;

        part->origin[0] = p->org[0] + p->vel[0] * time + p->acceleration[0] * time2;
        part->origin[1] = p->org[1] + p->vel[1] * time + p->acceleration[1] * time2;
        part->origin[2] = p->org[2] + p->vel[2] * time + p->acceleration[2] * time2;

        if (color == -1) {
            part->rgba.u8[0] = p->rgba.u8[0];
            part->rgba.u8[1] = p->rgba.u8[1];
            part->rgba.u8[2] = p->rgba.u8[2];
            part->rgba.u8[3] = p->rgba.u8[3] * alpha;
        }

        part->color = color;
        part->brightness = p->brightness;
        part->alpha = alpha;
        part->radius = 0.f;

        if (p->alphavel == INSTANT_PARTICLE) {
            p->alphavel = 0.0;
            p->alpha = 0.0;
        }
    }

    active_particles = active;
}