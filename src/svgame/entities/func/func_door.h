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

extern void door_use_areaportals(entity_t* self, qboolean open);
extern void door_go_down(entity_t* self);

extern void door_hit_top(entity_t* self);

extern void door_hit_bottom(entity_t* self);
extern void door_go_down(entity_t* self);

extern void door_go_up(entity_t* self, entity_t* activator);
extern void door_use(entity_t* self, entity_t* other, entity_t* activator);

extern void Touch_DoorTrigger(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf);
extern void Think_CalcMoveSpeed(entity_t* self);
extern void Think_SpawnDoorTrigger(entity_t* ent);

extern void door_blocked(entity_t* self, entity_t* other);
extern void door_killed(entity_t* self, entity_t* inflictor, entity_t* attacker, int damage, const vec3_t &point);
extern void door_touch(entity_t* self, entity_t* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_ENTITIES_FUNC_DOOR_H__