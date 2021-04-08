// LICENSE HERE.

//
// svgame/entities/target_string.c
//
//
// target_string entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use(entity_t* self, entity_t* other, entity_t* activator)
{
    entity_t* e;
    int     n, l;
    char    c;

    l = strlen(self->message);
    for (e = self->teamMasterPtr; e; e = e->teamChainPtr) {
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

void SP_target_string(entity_t* self)
{
    if (!self->message)
        self->message = "";
    self->Use = target_string_use;
}
