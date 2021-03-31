
// LICENSE HERE.

//
// svgame/entities/func/clock.h
//
//
#ifndef __SVGAME_ENTTIES_FUNC_CLOCK_H__
#define __SVGAME_ENTTIES_FUNC_CLOCK_H__

#define CLOCK_MESSAGE_SIZE  16
void func_clock_think(edict_t* self);
void func_clock_use(edict_t* self, edict_t* other, edict_t* activator);

#endif // __SVGAME_ENTTIES_FUNC_CLOCK_H__