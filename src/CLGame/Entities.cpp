/***
*
*	License here.
*
*	@file
* 
*   Client Entity utility functions.
*
***/
#include "ClientGameLocal.h"
#include "Effects.h"
#include "Entities.h"
#include "View.h"

extern qhandle_t cl_mod_powerscreen;
extern qhandle_t cl_mod_laser;
extern qhandle_t cl_mod_dmspot;
extern qhandle_t cl_sfx_footsteps[4];


/**
*   @brief  Gets us a pointer to the entity that is currently being viewed.
*           This could be an other client to in case of spectator mode.
**/
ClientEntity* CLG_GetClientViewEntity(void) {
    // Fetch clientnumber by default.
    int32_t index = cl->clientNumber;

    // Fetch the chasing entity index if we are chasing.
    if (cl->frame.playerState.stats[PlayerStats::ChaseClientID]) {
        index = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;
    }

    return &cs->entities[index + 1];
}

/**
*   @return True if the specified entity is bound to the local client's view.
**/
qboolean CLG_IsClientViewEntity(const ClientEntity* ent) {
    // If the entity number matches, then we're good.
    if (ent->current.number == cl->clientNumber + 1) {
        return true;
    }

    // If not, then we are viewing an other client entity, check whether it is in corpse mode.
    if ((ent->current.effects & EntityEffectType::Corpse) == 0) {
        // In case of no model index, we still want to validate some other cases.
        if (ent->current.modelIndex == 255) {
            if (ent->current.number == cl->clientNumber) {
                return true;
            } 

            // If we came to this point, fetch the chasing client.
            const int16_t chase = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;

            if (ent->current.number == chase) {
                return true;
            }
        }
    }

    // And if we came to this point, all bets are off, this is no client entity which we are viewing.
    return false;
}