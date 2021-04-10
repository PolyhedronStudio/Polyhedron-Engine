// LICENSE HERE.

//
// svgame/entities/func_areaportal.cpp
//
//
// func_areaportal entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================

void Use_Areaportal(entity_t * ent, entity_t * other, entity_t * activator)
{
    ent->count ^= 1;        // toggle state
//  gi.DPrintf ("portalstate: %i = %i\n", ent->style, ent->count);
    gi.SetAreaPortalState(ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal(entity_t * ent)
{
    ent->Use = Use_Areaportal;
    ent->count = 0;     // always start closed;
}