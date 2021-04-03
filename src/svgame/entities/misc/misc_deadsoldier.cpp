// LICENSE HERE.

//
// svgame/entities/misc_deadsoldier.c
//
//
// misc_deadsoldier entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../effects.h"

//=====================================================
/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t &point)
{
    int     n;

    if (self->health > -80)
        return;

    gi.Sound(self, CHAN_BODY, gi.SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
    for (n = 0; n < 4; n++)
        ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
    ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier(entity_t* ent)
{
    if (deathmatch->value) {
        // auto-remove for deathmatch
        G_FreeEntity(ent);
        return;
    }

    ent->moveType = MOVETYPE_NONE;
    ent->solid = SOLID_BBOX;
    ent->s.modelindex = gi.ModelIndex("models/deadbods/dude/tris.md2");

    // Defaults to frame 0
    if (ent->spawnFlags & 2)
        ent->s.frame = 1;
    else if (ent->spawnFlags & 4)
        ent->s.frame = 2;
    else if (ent->spawnFlags & 8)
        ent->s.frame = 3;
    else if (ent->spawnFlags & 16)
        ent->s.frame = 4;
    else if (ent->spawnFlags & 32)
        ent->s.frame = 5;
    else
        ent->s.frame = 0;

    VectorSet(ent->mins, -16, -16, 0);
    VectorSet(ent->maxs, 16, 16, 16);
    ent->deadFlag = DEAD_DEAD;
    ent->takedamage = DAMAGE_YES;
    ent->svFlags |= SVF_MONSTER | SVF_DEADMONSTER;
    ent->Die = misc_deadsoldier_die;
    ent->monsterInfo.aiflags |= AI_GOOD_GUY;

    gi.LinkEntity(ent);
}