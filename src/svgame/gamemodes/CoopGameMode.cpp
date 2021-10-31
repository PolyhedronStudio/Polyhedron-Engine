/*
// LICENSE HERE.

//
// CoopGameMode.cpp
//
//
*/
#include "../g_local.h"          // SVGame.
#include "../effects.h"     // Effects.
#include "../entities.h"    // Entities.
#include "../utils.h"       // Util funcs.

// Server Game Base Entity.
#include "../entities/base/SVGBaseEntity.h"

// Game Mode.
#include "CoopGameMode.h"

//
// Constructor/Deconstructor.
//
CoopGameMode::CoopGameMode() : DefaultGameMode() {

}
CoopGameMode::~CoopGameMode() {

}

//
// Interface functions. 
//
//

//
//===============
// CoopGameMode::CanDamage
//
// Template function serves as an example atm.
//===============
//
qboolean CoopGameMode::CanDamage(SVGBaseEntity* target, SVGBaseEntity* inflictor) {
    // Let it be to DefaultGameMode. :)
    return DefaultGameMode::CanDamage(target, inflictor);
}

//===============
// CoopGameMode::ClientUpdateObituary.
// 
//===============
void CoopGameMode::ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) {
    std::string message; // String stating what happened to whichever entity. "suicides", "was squished" etc.
    std::string messageAddition; // String stating what is additioned to it, "'s shrapnell" etc. Funny stuff.

    // If the attacker is a client, we know it was a friendly fire in this coop mode. :)
    if (attacker->GetClient())
        meansOfDeath |= MeansOfDeath::FriendlyFire;

    // Set a bool for whether we got friendly fire.
    qboolean friendlyFire = meansOfDeath & MeansOfDeath::FriendlyFire;
    // Quickly remove it from meansOfDeath again, our bool is set. This prevents it from 
    // sticking around when we process the next entity/client.
    int32_t finalMeansOfDeath = meansOfDeath & ~MeansOfDeath::FriendlyFire; // Sum of things, final means of death.

    // Determine the means of death.
    switch (finalMeansOfDeath) {
    case MeansOfDeath::Suicide:
        message = "suicides";
        break;
    case MeansOfDeath::Falling:
        message = "cratered";
        break;
    case MeansOfDeath::Crush:
        message = "was squished";
        break;
    case MeansOfDeath::Water:
        message = "sank like a rock";
        break;
    case MeansOfDeath::Slime:
        message = "melted";
        break;
    case MeansOfDeath::Lava:
        message = "does a back flip into the lava";
        break;
    case MeansOfDeath::Explosive:
    case MeansOfDeath::Barrel:
        message = "blew up";
        break;
    case MeansOfDeath::Exit:
        message = "found a way out";
        break;
    case MeansOfDeath::Splash:
    case MeansOfDeath::TriggerHurt:
        message = "was in the wrong place";
        break;
    }

    // Check if the attacker hurt himself, if so, ... n00b! :D 
    if (attacker == self) {
        switch (finalMeansOfDeath) {
        case MeansOfDeath::GrenadeSplash:
            message = "tripped on his own grenade";
            break;
        case MeansOfDeath::RocketSplash:
            message = "blew himself up";
            break;
        default:
            message = "killed himself";
            break;
        }
    }

    // Generated a message?
    if (message != "") {
        gi.BPrintf(PRINT_MEDIUM, "%s %s.\n", self->GetClient()->persistent.netname, message.c_str());
        //if (deathmatch->value)
        self->GetClient()->respawn.score--;
        self->SetEnemy(NULL);
        return;
    }

    // Set 'self' its attacker entity pointer.
    self->SetEnemy(attacker);

    // If we have an attacker, and it IS a client...
    if (attacker && attacker->GetClient()) {
        switch (finalMeansOfDeath) {
        case MeansOfDeath::Blaster:
            message = "was blasted by";
            break;
        case MeansOfDeath::Shotgun:
            message = "was gunned down by";
            break;
        case MeansOfDeath::SuperShotgun:
            message = "was blown away by";
            messageAddition = "'s super shotgun";
            break;
        case MeansOfDeath::Machinegun:
            message = "was machinegunned by";
            break;
        case MeansOfDeath::Grenade:
            message = "was popped by";
            messageAddition = "'s grenade";
            break;
        case MeansOfDeath::GrenadeSplash:
            message = "was shredded by";
            messageAddition = "'s shrapnel";
            break;
        case MeansOfDeath::Rocket:
            message = "ate";
            messageAddition = "'s rocket";
            break;
        case MeansOfDeath::RocketSplash:
            message = "almost dodged";
            messageAddition = "'s rocket";
            break;
        case MeansOfDeath::TeleFrag:
            message = "tried to invade";
            messageAddition = "'s personal space";
            break;
        }

        // In case we have a message, proceed.
        if (message != "") {
            // Print it.
            gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClient()->persistent.netname, messageAddition.c_str());

            // WID: Old piec of code // if (deathmatch->value) {
            if (friendlyFire)
                attacker->GetClient()->respawn.score--;
            else
                attacker->GetClient()->respawn.score++;
            // WID: Old piec of code // }
            return;
        }
    }

    // Check for monster deaths here.
    if (attacker->GetServerFlags() & EntityServerFlags::Monster) {
        // Fill in message here
        // aka if (attacker->classname == "monster_1337h4x0r")
        // Then we do...
        // Also we gotta adjust that ->classname thing, but this is a template, cheers :)
        //if (!message.empty()) {
        //    gi.BPrintf(PRINT_MEDIUM, "%s %s %s%s\n", self->GetClient()->persistent.netname, message.c_str(), attacker->GetClassName(), messageAddition.c_str());
        //    if (deathmatch->value) {
        //        if (friendlyFire)
        //            attacker->GetClient()->respawn.score--;
        //        else
        //            attacker->GetClient()->respawn.score++;
        //    }
        //    return;
        //}
    }

    // Inform the client died.
    gi.BPrintf(PRINT_MEDIUM, "%s died.\n", self->GetClient()->persistent.netname);

    // WID: This was an old piece of code, keeping it so people know what..// if (deathmatch->value)
    // Get the client, and change its current score.
    self->GetClient()->respawn.score--;
}