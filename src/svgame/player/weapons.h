// LICENSE HERE.

//
// weapons.h
//
//
// Contains basic weapons functionalities.
//
#ifndef __SVGAME_PLAYER_WEAPONS_H__
#define __SVGAME_PLAYER_WEAPONS_H__

// These are extern, used in the weapons/* code files.
extern qboolean is_quad;
extern byte     is_silenced;

// Player project source.
vec3_t P_ProjectSource(GameClient* client, const vec3_t &point, const vec3_t& distance, const vec3_t& forward, const vec3_t& right);

void PlayerNoise(Entity* who, vec3_t where, int type);

qboolean    Pickup_Weapon(Entity* ent, Entity* other);
void        ChangeWeapon(Entity* ent);
void        NoAmmoWeaponChange(Entity* ent);
void        Think_Weapon(Entity* ent);
void        Use_Weapon(Entity* ent, gitem_t* item);
void        Drop_Weapon(Entity* ent, gitem_t* item);
void        Weapon_Generic(Entity* ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int* pause_frames, int* fire_frames, void (*fire)(Entity* ent));

#endif // __SVGAME_PLAYER_WEAPONS_H__