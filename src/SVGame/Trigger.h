// LICENSE HERE.

//
// svgame/weapons/blaster.h
//
//
// Contains blaster declarations.
//
#ifndef __SVGAME_TRIGER_H__
#define __SVGAME_TRIGER_H__

void InitTrigger(Entity* self);
void multi_wait(Entity* ent);
void multi_trigger(Entity* ent);
void Use_Multi(Entity* ent, Entity* other, Entity* activator);
void Touch_Multi(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_TRIGER_H__