// LICENSE HERE.

//
// svgame/weapons/blaster.h
//
//
// Contains blaster declarations.
//
#ifndef __SVGAME_TRIGER_H__
#define __SVGAME_TRIGER_H__

void InitTrigger(entity_t* self);
void multi_wait(entity_t* ent);
void multi_trigger(entity_t* ent);
void Use_Multi(entity_t* ent, entity_t* other, entity_t* activator);
void Touch_Multi(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_TRIGER_H__