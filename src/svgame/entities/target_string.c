// LICENSE HERE.

//
// svgame/entities/target_string.c
//
//
// target_string entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use(edict_t* self, edict_t* other, edict_t* activator)
{
    edict_t* e;
    int     n, l;
    char    c;

    l = strlen(self->message);
    for (e = self->teammaster; e; e = e->teamchain) {
        if (!e->count)
            continue;
        n = e->count - 1;
        if (n > l) {
            e->s.frame = 12;
            continue;
        }

        c = self->message[n];
        if (c >= '0' && c <= '9')
            e->s.frame = c - '0';
        else if (c == '-')
            e->s.frame = 10;
        else if (c == ':')
            e->s.frame = 11;
        else
            e->s.frame = 12;
    }
}

void SP_target_string(edict_t* self)
{
    if (!self->message)
        self->message = "";
    self->use = target_string_use;
}
