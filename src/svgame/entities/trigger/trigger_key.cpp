// LICENSE HERE.

//
// svgame/entities/trigger_key.c
//
//
// trigger_key entity implementation.
//

#include "../../g_local.h"      // SVGame funcs.
#include "../../utils.h"        // Util funcs.

#include "../../trigger.h"

//=====================================================
/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void trigger_key_use(entity_t* self, entity_t* other, entity_t* activator)
{
    int         index;

    if (!self->item)
        return;
    if (!activator->client)
        return;

    index = ITEM_INDEX(self->item);
    if (!activator->client->persistent.inventory[index]) {
        if (level.time < self->debounceTouchTime)
            return;
        self->debounceTouchTime = level.time + 5.0;
        gi.CenterPrintf(activator, "You need the %s", self->item->pickupName);
        gi.Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/keytry.wav"), 1, ATTN_NORM, 0);
        return;
    }

    gi.Sound(activator, CHAN_AUTO, gi.SoundIndex("misc/keyuse.wav"), 1, ATTN_NORM, 0);
    if (coop->value) {
        int     player;
        entity_t* ent;

        if (strcmp(self->item->classname, "key_power_cube") == 0) {
            int cube;

            for (cube = 0; cube < 8; cube++)
                if (activator->client->persistent.power_cubes & (1 << cube))
                    break;
            for (player = 1; player <= game.maxClients; player++) {
                ent = &g_edicts[player];
                if (!ent->inUse)
                    continue;
                if (!ent->client)
                    continue;
                if (ent->client->persistent.power_cubes & (1 << cube)) {
                    ent->client->persistent.inventory[index]--;
                    ent->client->persistent.power_cubes &= ~(1 << cube);
                }
            }
        }
        else {
            for (player = 1; player <= game.maxClients; player++) {
                ent = &g_edicts[player];
                if (!ent->inUse)
                    continue;
                if (!ent->client)
                    continue;
                ent->client->persistent.inventory[index] = 0;
            }
        }
    }
    else {
        activator->client->persistent.inventory[index]--;
    }

    UTIL_UseTargets(self, activator);

    self->Use = NULL;
}

void SP_trigger_key(entity_t* self)
{
    if (!st.item) {
        gi.DPrintf("no key item for trigger_key at %s\n", Vec3ToString(self->state.origin));
        return;
    }
    self->item = FindItemByClassname(st.item);

    if (!self->item) {
        gi.DPrintf("item %s not found for trigger_key at %s\n", st.item, Vec3ToString(self->state.origin));
        return;
    }

    if (!self->target) {
        gi.DPrintf("%s at %s has no target\n", self->classname, Vec3ToString(self->state.origin));
        return;
    }

    gi.SoundIndex("misc/keytry.wav");
    gi.SoundIndex("misc/keyuse.wav");

    self->Use = trigger_key_use;
}