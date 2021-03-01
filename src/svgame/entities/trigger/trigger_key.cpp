// LICENSE HERE.

//
// svgame/entities/trigger_key.c
//
//
// trigger_key entity implementation.
//

// Include local game header.
#include "../../g_local.h"
#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void trigger_key_use(edict_t* self, edict_t* other, edict_t* activator)
{
    int         index;

    if (!self->item)
        return;
    if (!activator->client)
        return;

    index = ITEM_INDEX(self->item);
    if (!activator->client->pers.inventory[index]) {
        if (level.time < self->touch_debounce_time)
            return;
        self->touch_debounce_time = level.time + 5.0;
        gi.centerprintf(activator, "You need the %s", self->item->pickup_name);
        gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/keytry.wav"), 1, ATTN_NORM, 0);
        return;
    }

    gi.sound(activator, CHAN_AUTO, gi.soundindex("misc/keyuse.wav"), 1, ATTN_NORM, 0);
    if (coop->value) {
        int     player;
        edict_t* ent;

        if (strcmp(self->item->classname, "key_power_cube") == 0) {
            int cube;

            for (cube = 0; cube < 8; cube++)
                if (activator->client->pers.power_cubes & (1 << cube))
                    break;
            for (player = 1; player <= game.maxclients; player++) {
                ent = &g_edicts[player];
                if (!ent->inuse)
                    continue;
                if (!ent->client)
                    continue;
                if (ent->client->pers.power_cubes & (1 << cube)) {
                    ent->client->pers.inventory[index]--;
                    ent->client->pers.power_cubes &= ~(1 << cube);
                }
            }
        }
        else {
            for (player = 1; player <= game.maxclients; player++) {
                ent = &g_edicts[player];
                if (!ent->inuse)
                    continue;
                if (!ent->client)
                    continue;
                ent->client->pers.inventory[index] = 0;
            }
        }
    }
    else {
        activator->client->pers.inventory[index]--;
    }

    G_UseTargets(self, activator);

    self->use = NULL;
}

void SP_trigger_key(edict_t* self)
{
    if (!st.item) {
        gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
        return;
    }
    self->item = FindItemByClassname(st.item);

    if (!self->item) {
        gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
        return;
    }

    if (!self->target) {
        gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
        return;
    }

    gi.soundindex("misc/keytry.wav");
    gi.soundindex("misc/keyuse.wav");

    self->use = trigger_key_use;
}