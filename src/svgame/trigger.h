// LICENSE HERE.

//
// svgame/weapons/blaster.h
//
//
// Contains blaster declarations.
//
#ifndef __SVGAME_TRIGER_H__
#define __SVGAME_TRIGER_H__

void InitTrigger(edict_t* self);
void multi_wait(edict_t* ent);
void multi_trigger(edict_t* ent);
void Use_Multi(edict_t* ent, edict_t* other, edict_t* activator);
void Touch_Multi(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_TRIGER_H__