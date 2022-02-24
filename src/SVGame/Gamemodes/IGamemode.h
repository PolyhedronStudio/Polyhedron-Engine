/*
// LICENSE HERE.

//
// IGamemode.h
//
// Gamemode interface class. Need a custom new gamemode? Implement this interface,
// and you did yourself a pleasure. :)
//
*/
#pragma once

// It makes sense to include TypeInfo in SVGBaseEntity.h, 
// because this class absolutely requires it
#include "../TypeInfo.h"

class SVGBaseEntity;
class SVGBasePlayer;

using ClassEntityVector = std::vector<SVGBaseEntity*>;

class IGamemode {
public:
    //
    // Constructor/Deconstructor.
    //
    IGamemode() {};
    virtual ~IGamemode() = default;


    // Gamemode specific class checking. Best practice is to try and write code
    // that does not depend on checking a game mode class type too much.
    //
    // Instead try to facilitate the game mode itself instead where possible.
    /**
    *   @brief  Checks if this gamemode class is exactly the given class.
    *   @param  gamemodeClass A gamemode class which must inherint from IGamemode.
    *   @return True if the game mode class is the same class type or a derivate of gamemodeClass.
    **/
    template<typename gamemodeClass>
    bool IsClass() const {
	    return typeid(*this) == typeid(gamemodeClass);
    }
    
    /**
    *   @brief  Checks if this gamemode class is a subclass of another, or is the same class
    *   @param  gamemodeClass A gamemode class which must inherint from IGamemode.
    *   @return True if the game mode class is the same, or a derivate of gamemodeClass.
    **/
    template<typename gamemodeClass>
    bool IsSubclassOf() const {
	    return dynamic_cast<gamemodeClass>(*this) != nullptr;
    }


    //////
    // Server related.
    //////
    virtual qboolean CanSaveGame(qboolean isDedicatedServer) = 0;

    //////
    // Map related.
    //////
    /**
    *   @brief  Gets called at the moment the level exits, this gives the gamemode one last
    *           shot to finish off any last wishes before it gets destroyed.
    **/
    virtual void OnLevelExit() = 0;


    /**
    *   @brief  Called when a client connects. This does not get called between
    *           load games. In case of a loadgame, A client is still connected 
    *           to the current game session.
    **/
    virtual qboolean ClientConnect(Entity* serverEntity, char *userinfo) = 0;

    /**
    *   @brief  Called when a client has finished connecting, and is ready to be 
    *           placed into the game. This will happen every map load.
    **/
    virtual void ClientBegin(Entity* serverEntity) = 0;

    /**
    *   @brief  This will be called once for all clients at the start of each server 
    *           frame. Before running any other entities in the world.
    **/
    virtual void ClientBeginServerFrame(SVGBasePlayer* player, ServerClient *client) = 0;

    /**
    *   @brief  Called for each player at the end of the server frame and right 
    *           after spawning.
    **/
    virtual void ClientEndServerFrame(SVGBasePlayer* player, ServerClient* client) = 0;

    /**
    *   @brief  Called when a client disconnects.This does not get called between
    *           load games.
    **/
    virtual void ClientDisconnect(SVGBasePlayer* player, ServerClient* client) = 0;

    /**
    *   @brief  Called whenever the player updates a userinfo variable. The game can override any of 
    *           the settings in place (forcing skins or names, etc) before copying it off.
    **/
    virtual void ClientUserinfoChanged(Entity* ent, char *userinfo) = 0;

    /**
    *   @brief  Called in order to process "obituary" updates, aka with what weapon did this client
    *           or did other clients, kill any other client/entity.
    **/
    virtual void ClientUpdateObituary(SVGBaseEntity* player, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) = 0;

    /**
    *   @brief  Called when a client dies, usually used to clear their inventory.
    **/
    virtual void ClientDeath(SVGBasePlayer *player) = 0;


    /**
    *   @brief  Called only once in case of a single player game.
    *           Otherwise it is called after each death and level change.
    **/
    virtual void InitializePlayerPersistentData(ServerClient* client) = 0;

    /**
    *   @brief  Called only once in case of a single player game.
    *           Otherwise it is called after each death and level change.
    **/
    virtual void InitializePlayerRespawnData(ServerClient* client) = 0;


    /**
    *   @brief  Choose any info_player_start or its derivates, it'll do a subclassof check, 
    *           so the only valid classnames are those who have inherited from info_player_start. 
    *           Tyoe InfoPlayerStart, map classname info_player_deathmatch, etc.
    **/
    virtual void SelectPlayerSpawnPoint(SVGBasePlayer* player, vec3_t& origin, vec3_t& angles) = 0;

    
    // Called when a player connects to a game (whether it be single or multi -player).
    // For a SP mode death, the loadmenu pops up and the player gets to select a load game (If there are none, there is always the autosaved one.)
    // For a MP mode death, the game waits for intermission time to pass before it'll call this function again to respawn our player.
    virtual void PlacePlayerInGame(SVGBasePlayer* player) = 0;

    // Respawns a client (if that is what the game mode wants).
    virtual void RespawnClient(SVGBasePlayer* player) = 0;

    // Respawns all clients if the game mode allows so. (See RespawnClient)
    virtual void RespawnAllClients() = 0;

    /**
    *   @brief Stores player entity data in the client's persistent structure..
    * 
    *   @details    When switching a gamemap, information that should be persistant, like health,
    *               is still stored in the entity. This method mirrors it out to the client structure
    *               before all entities are wiped.
    **/
    virtual void StorePlayerPersistentData(void) = 0;

    /**
    *   @brief Restores player persistent data from the client struct by assigning it to the player entity.
    **/
    virtual void RestorePlayerPersistentData(SVGBaseEntity* player, ServerClient* client) = 0;


    //
    // Combat GameRules checks.
    //
    // Assigns the teamname to 'teamName', returns false/true if they are
    // on the same specific team.
    virtual qboolean GetEntityTeamName(SVGBaseEntity* ent, std::string &teamName) = 0;
    // Returns true if these two entities are on a same team.
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) = 0;
    // Returns true if the target entity can be damaged by the inflictor enemy.
    virtual qboolean CanDamage(SVGBaseEntity * target, SVGBaseEntity * inflictor) = 0;
    // Returns the entities found within a radius. Great for game mode fun times,
    // and that is why it resides here. Allows for customization.
    virtual ClassEntityVector FindBaseEnitiesWithinRadius(const vec3_t &origin, float radius, uint32_t excludeSolidFlags) = 0;


    //
    // Combat Gamemode actions.
    //
    // Called when an entity is killed, or at least, about to be.
    // Determine how to deal with it, usually resides in a callback to Die.
    virtual void EntityKilled(SVGBaseEntity* target, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int32_t damage, vec3_t point) = 0;

    // Inflicts actual damage on the targeted entity.
    // 
    // Old Documentation stated:
    // target      Entity that is being damaged
    // inflictor   Entity that is causing the damage
    // attacker    Entity that caused the inflictor to damage targ
    // 
    // example: target = monster, inflictor = rocket, attacker = player
    //
    // dmgDir      Direction of which the attack came from.
    // point       Point at which the damage is being inflicted
    // normal      Normal vector from that point
    // damage      Amount of damage being inflicted
    // knockBack   Force to be applied against targ as a result of the damage
    //
    // damageFlags These flags are used to control how SVG_InflictDamage works
    // DamageFlags::IndirectFromRadius    Damage was indirect(from a nearby explosion)
    // DamageFlags::NoArmorProtection     Armor does not protect from this damage
    // DamageFlags::EnergyBasedWeapon     Damage is from an energy based weapon
    // DamageFlags::NoKnockBack           Do not affect velocity, just view angles
    // DamageFlags::Bullet                Damage is from a bullet(used for ricochets)
    // DamageFlags::IgnoreProtection      Kills godmode, armor, everything
    virtual void InflictDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor, SVGBaseEntity* attacker, const vec3_t& dmgDir, const vec3_t& point, const vec3_t& normal, int32_t damage, int32_t knockBack, int32_t damageFlags, int32_t mod) = 0;
    // Similar to InflictDamage, but damages all entities within the given radius.
    // Of course, only if they apply to the same rules as InflictDamage accepts them.
    virtual void InflictRadiusDamage(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, float damage, SVGBaseEntity* ignore, float radius, int32_t mod) = 0;
    // Sets the current means of death for whichever client/entity is being processed.
    virtual void SetCurrentMeansOfDeath(int32_t meansOfDeath) = 0;
    // Retrieves the current means of death for whichever client/entity is being processed.
    virtual const int32_t& GetCurrentMeansOfDeath() = 0;


    //
    // Random Gameplay Utility Functions. s(Spawning gibs, checking velocity damage etc.)
    //
    // Spawns a temporary entity for a client, this is best suited to be in game mode.
    // Allows for all modes to customize that when wished for.
    //
    // When implementing this interface, it is suggested to just take DefaultGamemode,
    // or base yours off of that anyhow.
    // Used for spawning "Temporary Entities", on the client, that for example do particles.
    // In this case, it gets called when an entity gets damaged.
    virtual void SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) = 0;
    // Calculates the velocity for the damage given. (Used in effects, such as gibs.)
    virtual vec3_t CalculateDamageVelocity(int32_t damage) = 0;
    // Copies the model of the dead client, into a separate entity that plays dead.
    // It is a first in first out kinda list.
    virtual void SpawnClientCorpse(SVGBaseEntity* ent) = 0;


    // This function is for setting a "means of death", aka blaster or what not.
    // The thing is, it has to be able to be overrided so hey, here we go :)
    // Can't have a global like in the old code ;-)
    //
    // TODO: WID: This stuff should move to a Game/World class.


protected:
    // Means of Death, for the current client that is being processed this frame.
    int32_t meansOfDeath = 0;
};
