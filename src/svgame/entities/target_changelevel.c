// LICENSE HERE.

//
// svgame/entities/target_changelevel.c
//
//
// target_changelevel entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
Changes level to "map" when fired
*/
void use_target_changelevel(edict_t* self, edict_t* other, edict_t* activator)
{
    if (level.intermissiontime)
        return;     // already activated

    if (!deathmatch->value && !coop->value) {
        if (g_edicts[1].health <= 0)
            return;
    }

    // if noexit, do a ton of damage to other
    if (deathmatch->value && !((int)dmflags->value & DF_ALLOW_EXIT) && other != world) {
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 10 * other->max_health, 1000, 0, MOD_EXIT);
        return;
    }

    // if multiplayer, let everyone know who hit the exit
    if (deathmatch->value) {
        if (activator && activator->client)
            gi.bprintf(PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname);
    }

    // if going to a new unit, clear cross triggers
    if (strstr(self->map, "*"))
        game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);

    BeginIntermission(self);
}

void SP_target_changelevel(edict_t* ent)
{
    if (!ent->map) {
        gi.dprintf("target_changelevel with no map at %s\n", vtos(ent->s.origin));
        G_FreeEdict(ent);
        return;
    }

    // ugly hack because *SOMEBODY* screwed up their map
    if ((Q_stricmp(level.mapname, "fact1") == 0) && (Q_stricmp(ent->map, "fact3") == 0))
        ent->map = "fact3$secret1";

    ent->use = use_target_changelevel;
    ent->svflags = SVF_NOCLIENT;
}