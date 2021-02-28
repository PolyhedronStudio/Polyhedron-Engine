// LICENSE HERE.

//
// svgame/entities/target_character.c
//
//
// target_character entity implementation.
//

// Include local game header.
#include "../g_local.h"

//=====================================================
/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character(edict_t* self)
{
    self->movetype = MOVETYPE_PUSH;
    gi.setmodel(self, self->model);
    self->solid = SOLID_BSP;
    self->s.frame = 12;
    gi.linkentity(self);
    return;
}