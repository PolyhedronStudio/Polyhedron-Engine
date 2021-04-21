// LICENSE HERE.

//
// svgame/entities/target_changelevel.c
//
//
// target_changelevel entity implementation.
//

#include "../../g_local.h"          // Include SVGame header.
#include "../../player/client.h"    // Include Player Client header.
#include "../../utils.h"

//=====================================================
/*QUAKED target_changelevel (1 0 0) (-8 -8 -8) (8 8 8)
Changes level to "map" when fired
*/
void use_target_changelevel(entity_t* self, entity_t* other, entity_t* activator)
{
    if (level.intermissiontime)
        return;     // already activated

    if (!deathmatch->value && !coop->value) {
        if (g_edicts[1].health <= 0)
            return;
    }

    // if noexit, do a ton of damage to other
    if (deathmatch->value && !((int)dmflags->value & DeathMatchFlags::AllowExit) && other != world) {
        T_Damage(other, self, self, vec3_origin, other->s.origin, vec3_origin, 10 * other->maxHealth, 1000, 0, MOD_EXIT);
        return;
    }

    // if multiplayer, let everyone know who hit the exit
    if (deathmatch->value) {
        if (activator && activator->client)
            gi.BPrintf(PRINT_HIGH, "%s exited the level.\n", activator->client->pers.netname);
    }

    // if going to a new unit, clear cross triggers
    if (strstr(self->map, "*"))
        game.serverflags &= ~(SFL_CROSS_TRIGGER_MASK);

    HUD_BeginIntermission(self);
}

void SP_target_changelevel(entity_t* ent)
{
    if (!ent->map) {
        gi.DPrintf("target_changelevel with no map at %s\n", Vec3ToString(ent->s.origin));
        G_FreeEntity(ent);
        return;
    }

    // ugly hack because *SOMEBODY* screwed up their map
    if ((Q_stricmp(level.mapname, "fact1") == 0) && (Q_stricmp(ent->map, "fact3") == 0))
        ent->map = "fact3$secret1";

    ent->Use = use_target_changelevel;
    ent->svFlags = SVF_NOCLIENT;
}