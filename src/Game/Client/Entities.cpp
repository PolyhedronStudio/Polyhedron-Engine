/***
*
*	License here.
*
*	@file
* 
*   Client Entity utility functions.
*
***/
#include "ClientGameLocals.h"
#include "Entities.h"

///**
//*   @brief  Gets us a pointer to the entity that is currently being viewed.
//*           This could be an other client to in case of spectator mode.
//**/
//PODEntity* CLG_GetClientViewEntity(void) {
//    // Default is of course our own client entity number.
//    int32_t index = cl->clientNumber;
//
//    // However, faith has it that a chase client ID might be set, in which case we want to switch to its number instead.
//    if (cl->frame.playerState.stats[PlayerStats::ChaseClientID]) {
//        index = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;
//    }
//
//    return &cs->entities[index + 1];
//}
//
///**
//*   @return True if the specified entity is bound to the local client's view.
//**/
//qboolean CLG_IsClientViewEntity(const PODEntity* ent) {
//    // If the entity number matches, then we're good.
//    if (ent->current.number == cl->clientNumber + 1) {
//        return true;
//    }
//
//    // If not, then we are viewing an other client entity, check whether it is in corpse mode.
//    if ((ent->current.effects & EntityEffectType::Corpse) == 0) {
//        // No corpse, and modelIndex #255 indicating that we are dealing with a client entity.
//        if (ent->current.modelIndex == 255) {
//            // If the entity number matches our client number, we're good to go.
//            if (ent->current.number == cl->clientNumber) {
//                return true;
//            } 
//
//            // Otherwise we'll fetch the number of currently being chased client and see if that matches with this entity's number instead.
//            const int16_t chase = cl->frame.playerState.stats[PlayerStats::ChaseClientID] - ConfigStrings::PlayerSkins;
//
//            // Oh boy, it matched, someone is really happy right now.
//            if (ent->current.number == chase) {
//                return true;
//            }
//        }
//    }
//
//    // All bets are off, this is no client entity which we are viewing.
//    return false;
//}