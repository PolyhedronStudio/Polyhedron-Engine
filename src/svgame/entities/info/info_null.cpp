// LICENSE HERE.

//
// svgame/entities/info_null.c
//
//
// info_null entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null(entity_t* self)
{
    G_FreeEntity(self);
}
