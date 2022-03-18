/***
*
*	License here.
*
*	@file
*
*	Heat Beam particle effect implementation.
* 
***/
#include "../../ClientGameLocal.h"
#include "../../Main.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

/**
*   @brief  'Heat Beam' like particle effect.
**/
void ParticleEffects::HeatBeam(const vec3_t &start, const vec3_t &forward) {
    vec3_t      move;
    vec3_t      vec;
    float       len;
    int         j;
    cparticle_t* p;
    int         i;
    float       c, s;
    vec3_t      dir;
    float       ltime;
    float       step = 32.0, rstep;
    float       start_pt;
    float       rot;
    float       variance;
    vec3_t      end;

    VectorMA(start, 4096, forward, end);

    VectorCopy(start, move);
    VectorSubtract(end, start, vec);
    len = VectorNormalize(vec);

    ltime = (float)cl->time / 1000.0;
    start_pt = fmod(ltime * 96.0, step);
    VectorMA(move, start_pt, vec, move);

    VectorScale(vec, step, vec);

    rstep = M_PI / 10.0;
    for (i = start_pt; i < len; i += step) {
        if (i > step * 5) // don't bother after the 5th ring
            break;

        for (rot = 0; rot < M_PI * 2; rot += rstep) {
            p = Particles::GetFreeParticle();
            if (!p)
                return;

            p->time = cl->time;
            VectorClear(p->acceleration);
            variance = 0.5;
            c = std::cosf(rot) * variance;
            s = std::sinf(rot) * variance;

            // trim it so it looks like it's starting at the origin
            if (i < 10) {
                VectorScale(cl->v_right, c * (i / 10.0), dir);
                VectorMA(dir, s * (i / 10.0), cl->v_up, dir);
            }
            else {
                VectorScale(cl->v_right, c, dir);
                VectorMA(dir, s, cl->v_up, dir);
            }

            p->alpha = 0.5;
            p->alphavel = -1000.0;
            p->color = 223 - (rand() & 7);
            for (j = 0; j < 3; j++) {
                p->org[j] = move[j] + dir[j] * 3;
                p->vel[j] = 0;
            }
        }

        VectorAdd(move, vec, move);
    }
}