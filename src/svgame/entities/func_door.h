// LICENSE HERE.

//
// svgame/entities/func_door.h
//
//
// Contains all door related declarations.
//
#ifndef __SVGAME_ENTITIES_FUNC_DOOR_H__
#define __SVGAME_ENTITIES_FUNC_DOOR_H__

#define DOOR_START_OPEN     1
#define DOOR_REVERSE        2
#define DOOR_CRUSHER        4
#define DOOR_NOMONSTER      8
#define DOOR_TOGGLE         32
#define DOOR_X_AXIS         64
#define DOOR_Y_AXIS         128

extern void door_use_areaportals(edict_t* self, qboolean open);
extern void door_go_down(edict_t* self);

extern void door_hit_top(edict_t* self);

extern void door_hit_bottom(edict_t* self);
extern void door_go_down(edict_t* self);

extern void door_go_up(edict_t* self, edict_t* activator);
extern void door_use(edict_t* self, edict_t* other, edict_t* activator);

extern void Touch_DoorTrigger(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);
extern void Think_CalcMoveSpeed(edict_t* self);
extern void Think_SpawnDoorTrigger(edict_t* ent);

extern void door_blocked(edict_t* self, edict_t* other);
extern void door_killed(edict_t* self, edict_t* inflictor, edict_t* attacker, int damage, vec3_t point);
extern void door_touch(edict_t* self, edict_t* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_ENTITIES_FUNC_DOOR_H__