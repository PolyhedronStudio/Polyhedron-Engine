// LICENSE HERE.

//
// clg_effects.c
//
//
// Contains code for all special effects, simple steam leaking particles to
// awesome big banging explosions!
//
#include "clg_local.h"

static void CLG_LogoutEffect(vec3_t org, int type);

static vec3_t avelocities[NUMVERTEXNORMALS];

static cparticle_t* active_particles, * free_particles;

static cparticle_t  particles[MAX_PARTICLES];
static const int    cl_numparticles = MAX_PARTICLES;

static void CLG_ClearParticles(void);
#if USE_DLIGHTS
static cdlight_t       clg_dlights[MAX_DLIGHTS];
static void CLG_ClearDLights(void);
#endif
uint32_t d_8to24table[256];

cvar_t* cvar_pt_particle_emissive = NULL;
static cvar_t* cl_particle_num_factor = NULL;

//
//===============
// CLG_EffectsInit
// 
// Clears effects.
// ===============
//
void CLG_EffectsInit(void)
{
    int i, j;

    // Fetch cvars.
    cvar_pt_particle_emissive   = clgi.Cvar_Get("cvar_pt_particle_emissive", "", 0);
    cl_particle_num_factor      = clgi.Cvar_Get("cl_particle_num_factor", "", 0);

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
#if USE_DLIGHTS
    CLG_ClearDLights();
#endif
}


//
//=============================================================================
//
//	LIGHTSTYLE MANAGEMENT
//
//=============================================================================
//

#if USE_LIGHTSTYLES

typedef struct clightstyle_s {
    list_t  entry;
    int     length;
    vec4_t  value;
    float   map[MAX_QPATH];
} clightstyle_t;

static clightstyle_t    cl_lightstyles[MAX_LIGHTSTYLES];
static LIST_DECL(cl_lightlist);
static int          cl_lastofs;

void CLG_ClearLightStyles(void)
{
    int     i;
    clightstyle_t* ls;

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
    clightstyle_t* ls;

    ofs = cl->time / 100;
    if (ofs == cl_lastofs)
        return;
    cl_lastofs = ofs;

    LIST_FOR_EACH(clightstyle_t, ls, &cl_lightlist, entry) {
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = ls->map[ofs % ls->length];
    }
}

void CLG_SetLightStyle(int index, const char* s)
{
    int     i;
    clightstyle_t* ls;

    ls = &cl_lightstyles[index];
    ls->length = strlen(s);
    if (ls->length > MAX_QPATH) {
        Com_Error(ERR_DROP, "%s: oversize style", __func__);
    }

    for (i = 0; i < ls->length; i++) {
        ls->map[i] = (float)(s[i] - 'a') / (float)('m' - 'a');
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
    clightstyle_t* ls;

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

#if USE_DLIGHTS
static cdlight_t       cl_dlights[MAX_DLIGHTS];

static void CLG_ClearDLights(void)
{
    memset(cl_dlights, 0, sizeof(cl_dlights));
}

//
//===============
// CLG_RunDLights
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

        VectorMA(dl->origin, clgi.GetFrameTime(), dl->velosity, dl->origin);
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

#endif


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

}

//
//===============
// CLG_MuzzleFlash2
// 
// Handle the MuzzleFlash messages for the monster entities.
//===============
//
void CLG_MuzzleFlash2() {

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
    vec3_t oy;
    VectorSet(oy, 0.0f, 1.0f, 0.0f);
    if (fabs(DotProduct(oy, dir)) > 0.95f)
        VectorSet(oy, 1.0f, 0.0f, 0.0f);

    vec3_t ox;
    CrossProduct(oy, dir, ox);

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

        vec3_t origin;
        VectorCopy(org, origin);
        VectorMA(origin, dirt_horizontal_spread * crand(), ox, origin);
        VectorMA(origin, dirt_horizontal_spread * crand(), oy, origin);
        VectorMA(origin, dirt_vertical_spread * frand() + 1.0f, dir, origin);
        VectorCopy(origin, p->org);

        vec3_t velocity;
        VectorSubtract(origin, org, velocity);
        VectorNormalize(velocity);
        VectorScale(velocity, dirt_base_velocity + frand() * dirt_rand_velocity, p->vel);

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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

        vec3_t origin;
        VectorCopy(org, origin);
        VectorMA(origin, spark_horizontal_spread * crand(), ox, origin);
        VectorMA(origin, spark_horizontal_spread * crand(), oy, origin);
        VectorMA(origin, spark_vertical_spread * frand() + 1.0f, dir, origin);
        VectorCopy(origin, p->org);

        vec3_t velocity;
        VectorSubtract(origin, org, velocity);
        VectorNormalize(velocity);
        VectorScale(velocity, spark_base_velocity + powf(frand(), 2.0f) * spark_rand_velocity, p->vel);

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -2.0 / (0.5 + frand() * 0.3);
    }
}

void CLG_ParticleEffectWaterSplash(vec3_t org, vec3_t dir, int color, int count)
{
    vec3_t oy;
    VectorSet(oy, 0.0f, 1.0f, 0.0f);
    if (fabs(DotProduct(oy, dir)) > 0.95f)
        VectorSet(oy, 1.0f, 0.0f, 0.0f);

    vec3_t ox;
    CrossProduct(oy, dir, ox);

    count *= cl_particle_num_factor->value;

    const float water_horizontal_spread = 0.25f;
    const float water_vertical_spread = 1.0f;
    const float water_base_velocity = 80.0f;
    const float water_rand_velocity = 150.0f;

    for (int i = 0; i < count; i++) {
        cparticle_t* p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = color + (rand() & 7);
        p->brightness = 1.0f;

        vec3_t origin;
        VectorCopy(org, origin);
        VectorMA(origin, water_horizontal_spread * crand(), ox, origin);
        VectorMA(origin, water_horizontal_spread * crand(), oy, origin);
        VectorMA(origin, water_vertical_spread * frand() + 1.0f, dir, origin);
        VectorCopy(origin, p->org);

        vec3_t velocity;
        VectorSubtract(origin, org, velocity);
        VectorNormalize(velocity);
        VectorScale(velocity, water_base_velocity + frand() * water_rand_velocity, p->vel);

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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
    decal_t dec = {
      .pos = {org[0],org[1],org[2]},
      .dir = {dir[0],dir[1],dir[2]},
      .spread = 0.25f,
      .length = 350 };
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
            p->org[j] = org[j] + ((rand() & 7) - 4) + d * (dir[j]
                + a[j] * 0.5f * ((rand() & 31) / 32.0f - .5f)
                + b[j] * 0.5f * ((rand() & 31) / 32.0f - .5f));

            p->vel[j] = 10.0f * dir[j] + crand() * 20;
        }
        // fake gravity
        p->org[2] -= d * d * .001f;

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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
            p->org[j] = org[j] + ((rand() & 7) - 4) + d * dir[j];
            p->vel[j] = crand() * 20;
        }

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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
        if (type == MZ_LOGIN)
            color = 0xd0 + (rand() & 7); // green
        else if (type == MZ_LOGOUT)
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

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY * 0.2;
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
            p->org[j] = org[j] + ((rand() % 32) - 16);
            p->vel[j] = (rand() % 384) - 192;
        }

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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
        p->org[0] = org[0] + cos(angle) * dist;
        p->vel[0] = cos(angle) * (70 + (rand() & 63));
        p->accel[0] = -cos(angle) * 100;

        p->org[1] = org[1] + sin(angle) * dist;
        p->vel[1] = sin(angle) * (70 + (rand() & 63));
        p->accel[1] = -sin(angle) * 100;

        p->org[2] = org[2] + 8 + (rand() % 90);
        p->vel[2] = -100 + (rand() & 31);
        p->accel[2] = PARTICLE_GRAVITY * 4;
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
            p->org[j] = org[j] + ((rand() & 7) - 4) + d * dir[j];
            p->vel[j] = dir[j] * 30 + crand() * 40;
        }

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
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
        VectorClear(p->accel);

        p->time = cl->time;

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.3 + frand() * 0.2);

        p->color = 0xe0;
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand();
            p->vel[j] = crand() * 5;
            p->accel[j] = 0;
        }

        VectorAdd(move, vec, move);
    }
}

/*
===============
CLG_QuadTrail

===============
*/
void CLG_QuadTrail(vec3_t start, vec3_t end)
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

    while (len > 0) {
        len -= dec;

        p = CLG_AllocParticle();
        if (!p)
            return;
        VectorClear(p->accel);

        p->time = cl->time;

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.8 + frand() * 0.2);

        p->color = 115;
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand() * 16;
            p->vel[j] = crand() * 5;
            p->accel[j] = 0;
        }

        VectorAdd(move, vec, move);
    }
}

/*
===============
CLG_FlagTrail

===============
*/
void CLG_FlagTrail(vec3_t start, vec3_t end, int color)
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

    while (len > 0) {
        len -= dec;

        p = CLG_AllocParticle();
        if (!p)
            return;
        VectorClear(p->accel);

        p->time = cl->time;

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.8 + frand() * 0.2);

        p->color = color;
        p->brightness = 1.0f;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand() * 16;
            p->vel[j] = crand() * 5;
            p->accel[j] = 0;
        }

        VectorAdd(move, vec, move);
    }
}

/*
===============
CLG_DiminishingTrail

===============
*/
void CLG_DiminishingTrail(vec3_t start, vec3_t end, centity_t* old, int flags)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    float       dec;
    float       orgscale;
    float       velscale;

    VectorCopy(start, move);
    VectorSubtract(end, start, vec);
    len = VectorNormalize(vec);

    dec = 0.5;
    VectorScale(vec, dec, vec);

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
            VectorClear(p->accel);

            p->time = cl->time;

            if (flags & EF_GIB) {
                p->alpha = 1.0;
                p->alphavel = -1.0 / (1 + frand() * 0.4);

                p->color = 0xe8 + (rand() & 7);
                p->brightness = 1.0f;

                for (j = 0; j < 3; j++) {
                    p->org[j] = move[j] + crand() * orgscale;
                    p->vel[j] = crand() * velscale;
                    p->accel[j] = 0;
                }
                p->vel[2] -= PARTICLE_GRAVITY;
            }
            else if (flags & EF_GREENGIB) {
                p->alpha = 1.0;
                p->alphavel = -1.0 / (1 + frand() * 0.4);

                p->color = 0xdb + (rand() & 7);
                p->brightness = 1.0f;

                for (j = 0; j < 3; j++) {
                    p->org[j] = move[j] + crand() * orgscale;
                    p->vel[j] = crand() * velscale;
                    p->accel[j] = 0;
                }
                p->vel[2] -= PARTICLE_GRAVITY;
            }
            else {
                p->alpha = 1.0;
                p->alphavel = -1.0 / (1 + frand() * 0.2);

                p->color = 4 + (rand() & 7);
                p->brightness = 1.0f;

                for (j = 0; j < 3; j++) {
                    p->org[j] = move[j] + crand() * orgscale;
                    p->vel[j] = crand() * velscale;
                }
                p->accel[2] = 20;
            }
        }

        old->trailcount -= 5;
        if (old->trailcount < 100)
            old->trailcount = 100;
        VectorAdd(move, vec, move);
    }
}

/*
===============
CLG_RocketTrail

===============
*/
void CLG_RocketTrail(vec3_t start, vec3_t end, centity_t* old)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    float       dec;

    // smoke
    CLG_DiminishingTrail(start, end, old, EF_ROCKET);

    // fire
    VectorCopy(start, move);
    VectorSubtract(end, start, vec);
    len = VectorNormalize(vec);

    dec = 1;
    VectorScale(vec, dec, vec);

    while (len > 0) {
        len -= dec;

        if ((rand() & 7) == 0) {
            p = CLG_AllocParticle();
            if (!p)
                return;

            VectorClear(p->accel);
            p->time = cl->time;

            p->alpha = 1.0;
            p->alphavel = -1.0 / (1 + frand() * 0.2);

            p->color = 0xdc + (rand() & 3);
            p->brightness = cvar_pt_particle_emissive->value;

            for (j = 0; j < 3; j++) {
                p->org[j] = move[j] + crand() * 5;
                p->vel[j] = crand() * 20;
            }
            p->accel[2] = -PARTICLE_GRAVITY;
        }
        VectorAdd(move, vec, move);
    }
}

/*
===============
CLG_RailTrail

===============
*/
void CLG_OldRailTrail(void)
{
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    float       dec;
    vec3_t      right, up;
    int         i;
    float       d, c, s;
    vec3_t      dir;
    byte        clr = 0x74;

    VectorCopy(teParameters.pos1, move);
    VectorSubtract(teParameters.pos2, teParameters.pos1, vec);
    len = VectorNormalize(vec);

    MakeNormalVectors(vec, right, up);

    for (i = 0; i < len; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;
        VectorClear(p->accel);

        d = i * 0.1;
        c = cos(d);
        s = sin(d);

        VectorScale(right, c, dir);
        VectorMA(dir, s, up, dir);

        p->alpha = 1.0;
        p->alphavel = -1.0 / (1 + frand() * 0.2);

        p->color = clr + (rand() & 7);
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + dir[j] * 3;
            p->vel[j] = dir[j] * 6;
        }

        VectorAdd(move, vec, move);
    }

    dec = 0.75;
    VectorScale(vec, dec, vec);
    VectorCopy(teParameters.pos1, move);

    while (len > 0) {
        len -= dec;

        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;
        VectorClear(p->accel);

        p->alpha = 1.0;
        p->alphavel = -1.0 / (0.6 + frand() * 0.2);

        p->color = rand() & 15;
        p->brightness = 1.0f;

        for (j = 0; j < 3; j++) {
            p->org[j] = move[j] + crand() * 3;
            p->vel[j] = crand() * 3;
            p->accel[j] = 0;
        }

        VectorAdd(move, vec, move);
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

    VectorCopy(start, move);
    VectorSubtract(end, start, vec);
    len = VectorNormalize(vec);

    dec = 32;
    VectorScale(vec, dec, vec);

    for (i = 0; i < len; i += dec) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        VectorClear(p->accel);
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

        VectorAdd(move, vec, move);
    }
}


/*
===============
CLG_FlyParticles
===============
*/

#define BEAMLENGTH  16

static void CLG_FlyParticles(vec3_t origin, int count)
{
    int         i;
    cparticle_t* p;
    float       angle;
    float       sp, sy, cp, cy;
    vec3_t      forward;
    float       dist = 64;
    float       ltime;

    if (count > NUMVERTEXNORMALS)
        count = NUMVERTEXNORMALS;

    ltime = (float)cl->time / 1000.0;
    for (i = 0; i < count; i += 2) {
        angle = ltime * avelocities[i][0];
        sy = sin(angle);
        cy = cos(angle);
        angle = ltime * avelocities[i][1];
        sp = sin(angle);
        cp = cos(angle);

        forward[0] = cp * cy;
        forward[1] = cp * sy;
        forward[2] = -sp;

        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        dist = sin(ltime + i) * 64;
        p->org[0] = origin[0] + bytedirs[i][0] * dist + forward[0] * BEAMLENGTH;
        p->org[1] = origin[1] + bytedirs[i][1] * dist + forward[1] * BEAMLENGTH;
        p->org[2] = origin[2] + bytedirs[i][2] * dist + forward[2] * BEAMLENGTH;

        VectorClear(p->vel);
        VectorClear(p->accel);

        p->color = 0;
        p->brightness = 1.0f;

        p->alpha = 1;
        p->alphavel = INSTANT_PARTICLE;
    }
}

void CLG_FlyEffect(centity_t* ent, vec3_t origin)
{
    int     n;
    int     count;
    int     starttime;

    if (ent->fly_stoptime < cl->time) {
        starttime = cl->time;
        ent->fly_stoptime = cl->time + 60000;
    }
    else {
        starttime = ent->fly_stoptime - 60000;
    }

    n = cl->time - starttime;
    if (n < 20000)
        count = n * 162 / 20000.0;
    else {
        n = ent->fly_stoptime - cl->time;
        if (n < 20000)
            count = n * 162 / 20000.0;
        else
            count = 162;
    }

    CLG_FlyParticles(origin, count);
}

/*
===============
CLG_BfgParticles
===============
*/
void CLG_BfgParticles(entity_t* ent)
{
    int         i;
    cparticle_t* p;
    float       angle;
    float       sp, sy, cp, cy;
    vec3_t      forward;
    float       dist = 64;
    vec3_t      v;
    float       ltime;

    const int count = NUMVERTEXNORMALS * cl_particle_num_factor->value;

    ltime = (float)cl->time / 1000.0;
    for (i = 0; i < count; i++) {
        angle = ltime * avelocities[i][0];
        sy = sin(angle);
        cy = cos(angle);
        angle = ltime * avelocities[i][1];
        sp = sin(angle);
        cp = cos(angle);

        forward[0] = cp * cy;
        forward[1] = cp * sy;
        forward[2] = -sp;

        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        dist = sin(ltime + i) * 64;
        p->org[0] = ent->origin[0] + bytedirs[i][0] * dist + forward[0] * BEAMLENGTH;
        p->org[1] = ent->origin[1] + bytedirs[i][1] * dist + forward[1] * BEAMLENGTH;
        p->org[2] = ent->origin[2] + bytedirs[i][2] * dist + forward[2] * BEAMLENGTH;

        VectorClear(p->vel);
        VectorClear(p->accel);

        VectorSubtract(p->org, ent->origin, v);
        dist = VectorLength(v) / 90.0;

        p->color = floor(0xd0 + dist * 7);
        p->brightness = cvar_pt_particle_emissive->value;

        p->alpha = 1.0 - dist;
        p->alphavel = INSTANT_PARTICLE;
    }
}


/*
===============
CLG_BFGExplosionParticles
===============
*/
//FIXME combined with CLG_ExplosionParticles
void CLG_BFGExplosionParticles(vec3_t org)
{
    int         i, j;
    cparticle_t* p;

    const int count = 256 * cl_particle_num_factor->value;

    for (i = 0; i < count; i++) {
        p = CLG_AllocParticle();
        if (!p)
            return;

        p->time = cl->time;

        p->color = 0xd0 + (rand() & 7);
        p->brightness = cvar_pt_particle_emissive->value;

        for (j = 0; j < 3; j++) {
            p->org[j] = org[j] + ((rand() % 32) - 16);
            p->vel[j] = (rand() % 384) - 192;
        }

        p->accel[0] = p->accel[1] = 0;
        p->accel[2] = -PARTICLE_GRAVITY;
        p->alpha = 1.0;

        p->alphavel = -0.8 / (0.5 + frand() * 0.3);
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
                VectorScale(dir, vel, p->vel);

                p->accel[0] = p->accel[1] = 0;
                p->accel[2] = -PARTICLE_GRAVITY;
            }
}

extern int          r_numparticles;
extern particle_t   r_particles[MAX_PARTICLES];

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
    particle_t* part;

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

        if (r_numparticles >= MAX_PARTICLES)
            break;
        part = &r_particles[r_numparticles++];

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

        part->origin[0] = p->org[0] + p->vel[0] * time + p->accel[0] * time2;
        part->origin[1] = p->org[1] + p->vel[1] * time + p->accel[1] * time2;
        part->origin[2] = p->org[2] + p->vel[2] * time + p->accel[2] * time2;

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