/***
*
*	License here.
*
*	@file
*
*	Heat Beam particle effect implementation.
* 
***/
#include "../../ClientGameLocals.h"

#include "../Particles.h"
#include "../ParticleEffects.h"

#include "../../Exports/View.h"

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

    ViewCamera *viewCamera = clge->view->GetViewCamera();
    end = vec3_fmaf(start, 4096, forward);

    move = start;
    vec = end - start;
    vec3_normalize_length(vec, len);

    ltime = (float)cl->time / 1000.0;
    start_pt = fmod(ltime * 96.0, step);
    move = vec3_fmaf(move, start_pt, vec);

    vec = vec3_scale(vec, step);

    rstep = M_PI / 10.0;
    for (i = start_pt; i < len; i += step) {
        if (i > step * 5) // don't bother after the 5th ring
            break;

        for (rot = 0; rot < M_PI * 2; rot += rstep) {
            p = Particles::GetFreeParticle();
            if (!p)
                return;

            p->time = cl->time;
            p->acceleration = vec3_zero();
            variance = 0.5;
            c = cosf(rot) * variance;
            s = sinf(rot) * variance;

            // trim it so it looks like it's starting at the origin
            if (i < 10) {
                dir = vec3_scale(viewCamera->GetRightViewVector(), c * (i / 10.0));
                dir = vec3_fmaf(dir, s * (i / 10.0), viewCamera->GetUpViewVector());
            }
            else {
                dir = vec3_scale(viewCamera->GetRightViewVector(), c);
                dir = vec3_fmaf(dir, s, viewCamera->GetUpViewVector());
            }

            p->alpha = 0.5;
            p->alphavel = -1000.0;
            p->color = 223 - (rand() & 7);
            for (j = 0; j < 3; j++) {
                p->org[j] = move[j] + dir[j] * 3;
                p->vel[j] = 0;
            }
        }

        move += vec;
    }
}