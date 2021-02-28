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
void P_ProjectSource(gclient_t* client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

void PlayerNoise(edict_t* who, vec3_t where, int type);

qboolean    Pickup_Weapon(edict_t* ent, edict_t* other);
void        ChangeWeapon(edict_t* ent);
void        NoAmmoWeaponChange(edict_t* ent);
void        Think_Weapon(edict_t* ent);
void        Use_Weapon(edict_t* ent, gitem_t* item);
void        Drop_Weapon(edict_t* ent, gitem_t* item);
void        Weapon_Generic(edict_t* ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int* pause_frames, int* fire_frames, void (*fire)(edict_t* ent));

#endif // __SVGAME_PLAYER_WEAPONS_H__