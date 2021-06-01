// LICENSE HERE.

//
// svgame/entities/misc_explobox.c
//
//
// misc_explobox entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../effects.h"

//=====================================================
/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and damage (150).
*/


void barrel_think(Entity* self) {

}

void SP_misc_explobox(Entity* self)
{
    //if (deathmatch->value) {
    //    // auto-remove for deathmatch
    //    SVG_FreeEntity(self);
    //    return;
    //}

    //gi.ModelIndex("models/objects/debris1/tris.md2");
    //gi.ModelIndex("models/objects/debris2/tris.md2");
    //gi.ModelIndex("models/objects/debris3/tris.md2");

    //self->solid = Solid::BoundingBox;
    //self->moveType = MoveType::Step;

    //self->model = "models/objects/barrels/tris.md2";
    //self->state.modelIndex = gi.ModelIndex(self->model);
    //VectorSet(self->mins, -16, -16, 0);
    //VectorSet(self->maxs, 16, 16, 40);

    //if (!self->mass)
    //    self->mass = 400;
    //if (!self->health)
    //    self->health = 10;
    //if (!self->damage)
    //    self->damage = 150;

    //self->Die = barrel_delay;
    //self->takeDamage = TakeDamage::Yes;

    //self->Touch = barrel_touch;

    ////self->Think = barrel_think;
    ////self->nextThinkTime = level.time + 2 * FRAMETIME;

    //gi.LinkEntity(self);
}