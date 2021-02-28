// LICENSE HERE.

//
// svgame/entities/misc_gib_arm.c
//
//
// misc_gib_arm entity implementation.
//

// Include local game header.
#include "../g_local.h"

// Declared in misc.c
extern void gib_die(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point);

//=====================================================
/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm(edict_t* ent)
{
    gi.setmodel(ent, "models/objects/gibs/arm/tris.md2");
    ent->solid = SOLID_NOT;
    ent->s.effects |= EF_GIB;
    ent->takedamage = DAMAGE_YES;
    ent->die = gib_die;
    ent->movetype = MOVETYPE_TOSS;
    ent->svflags |= SVF_MONSTER;
    ent->deadflag = DEAD_DEAD;
    ent->avelocity[0] = random() * 200;
    ent->avelocity[1] = random() * 200;
    ent->avelocity[2] = random() * 200;
    ent->think = G_FreeEdict;
    ent->nextthink = level.time + 30;
    gi.linkentity(ent);
}
