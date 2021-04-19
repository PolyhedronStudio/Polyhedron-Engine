// LICENSE HERE.

//
// svgame/weapons/flaregun.c
//
//
// Flaregun weapon code.
//

// Include local game header.
#include "../g_local.h"

// Include player headers.
#include "../player/animations.h"
#include "../player/weapons.h"

// Include weapon header.
#include "flaregun.h"

//======================================================================

/*
 * Forward declaration for fire_flaregun(), which is defined in
 * g_weapon.c.
 */
void fire_flaregun(entity_t* self, const vec3_t &start, const vec3_t &aimdir, int damage,
    int speed, float timer, float damage_radius);
/*
 * weapon_flaregun_fire (entity_t *ent)
 *
 * Basically used to wrap the call to fire_flaregun(), this function
 * calculates all the parameters needed by fire_flaregun.  Calls
 * fire_flaregun and then subtracts 1 from the firing entity's
 * cell stash.
 */
void weapon_flaregun_fire(entity_t* ent)
{
    vec3_t offset;
    vec3_t forward, right;
    vec3_t start;

    // Setup the parameters used in the call to fire_flaregun() 
     // 
    VectorSet(offset, 8, 8, ent->viewHeight - 8);
    AngleVectors(ent->client->v_angle, &forward, &right, NULL);
    start = P_ProjectSource(ent->client, ent->s.origin, offset, forward, right);

    VectorScale(forward, -2, ent->client->kickOrigin);
    ent->client->kickAngles[0] = -1;

    // Make the flaregun actually shoot the flare 
     // 
    fire_flaregun(ent, start, forward, 0, 800, 25, 0);

    gi.WriteByte(svg_muzzleflash);
    gi.WriteShort(ent - g_edicts);
    gi.WriteByte(MuzzleFlashType::Flare | is_silenced);
    gi.Multicast(&ent->s.origin, MULTICAST_PVS);

    // Bump the gunframe 
     // 
    ent->client->playerState.gunframe++;

    PlayerNoise(ent, start, PNOISE_WEAPON);

    // Subtract one cell from our inventory 
     // 
    ent->client->pers.inventory[ent->client->ammo_index]--;
}

/*
 * Weapon_FlareGun (entity_t *ent)
 *
 * This is the function that is referenced in the itemlist structure
 * defined in g_items.c.  It is called every frame when our weapon is
 * active.  It calls Weapon_Generic() to handle per-frame weapon
 * handling (like animation and stuff).  Haven't delved too deeply
 * into Weapon_Generic()'s responsiblities... if someone has insight
 * drop me a line :)
 */
void Weapon_FlareGun(entity_t* ent)
{
    static int pause_frames[] = { 39, 45, 50, 53, 0 };
    static int fire_frames[] = { 9, 17, 0 };
    // Check the top of p_weapon.c for definition of Weapon_Generic 
    // 
    Weapon_Generic(ent, 8, 13, 49, 53,
        pause_frames,
        fire_frames,
        weapon_flaregun_fire);
}