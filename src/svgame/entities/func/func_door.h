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

extern void door_use_areaportals(Entity* self, qboolean open);
extern void door_go_down(Entity* self);

extern void door_hit_top(Entity* self);

extern void door_hit_bottom(Entity* self);
extern void door_go_down(Entity* self);

extern void door_go_up(Entity* self, Entity* activator);
extern void door_use(Entity* self, Entity* other, Entity* activator);

extern void Touch_DoorTrigger(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf);
extern void Think_CalcMoveSpeed(Entity* self);
extern void Think_SpawnDoorTrigger(Entity* ent);

extern void door_blocked(Entity* self, Entity* other);
extern void door_killed(Entity* self, Entity* inflictor, Entity* attacker, int damage, const vec3_t &point);
extern void door_touch(Entity* self, Entity* other, cplane_t* plane, csurface_t* surf);

#endif // __SVGAME_ENTITIES_FUNC_DOOR_H__