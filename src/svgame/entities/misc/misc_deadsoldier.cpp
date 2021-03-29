// LICENSE HERE.

//
// svgame/entities/misc_deadsoldier.c
//
//
// misc_deadsoldier entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, const vec3_t &point)
{
    int     n;

    if (self->health > -80)
        return;

    gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
    for (n = 0; n < 4; n++)
        ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier(edict_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEdict(ent);
        return;
    }

    ent->movetype = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.modelindex = gi.modelindex("models/deadbods/dude/tris.md2");

    // Defaults to frame 0
    if (ent->spawnflags & 2)
        ent->s.frame = 1;
    else if (ent->spawnflags & 4)
        ent->s.frame = 2;
    else if (ent->spawnflags & 8)
        ent->s.frame = 3;
    else if (ent->spawnflags & 16)
        ent->s.frame = 4;
    else if (ent->spawnflags & 32)
        ent->s.frame = 5;
    else
        ent->s.frame = 0;

    VectorSet(ent->mins, -16, -16, 0);
    VectorSet(ent->maxs, 16, 16, 16);
    ent->deadflag = DEAD_DEAD;
    ent->takedamage = DAMAGE_YES;
    ent->svflags |= SVF_MONSTER | SVF_DEADMONSTER;
    ent->die = misc_deadsoldier_die;
    ent->monsterinfo.aiflags |= AI_GOOD_GUY;

    gi.linkentity(ent);
}