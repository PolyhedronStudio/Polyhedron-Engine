/*
// LICENSE HERE.

//
// IGameMode.h
//
// GameMode interface class. Need a custom new gamemode? Implement this interface,
// and you did yourself a pleasure. :)
//
*/
#ifndef __SVGAME_GAMEMODES_IGAMEMODE_H__
#define __SVGAME_GAMEMODES_IGAMEMODE_H__

class SVGBaseEntity;
class PlayerClient;

using BaseEntityVector = std::vector<SVGBaseEntity*>;

class IGameMode {
public:
    //
    // Constructor/Deconstructor.
    //
    IGameMode() {};
    virtual ~IGameMode() {};
    
    //
    // Map related, also known as the "current game".
    // 
    // Gets called at the moment the level exits, this gives the gamemode one last
    // shot to finish off any last wishes before it gets destroyed.
    virtual void OnLevelExit() = 0;


    //
    // Client related callbacks.
    // 
    // Game modes have the ability to implement their own, this can be of use
    // for specific game modes. They like to have control over this.
    //
    // Called when a client connects. This does not get called between
    // load games, of course. A client is still connected to the current
    // game session in that case.
    virtual qboolean ClientConnect(Entity* serverEntity, char *userinfo) = 0;
    // Called when a client has finished connecting, and is ready
    // to be placed into the game.This will happen every map load.
    virtual void ClientBegin(Entity* serverEntity) = 0;
    // This will be called once for all clients at the start of each server 
    // frame. Before running any other entities in the world.
    virtual void ClientBeginServerFrame(Entity* serverEntity) = 0;
    // Called for each player at the end of the server frame and right 
    // after spawning.
    virtual void ClientEndServerFrame(Entity *serverEntity) = 0;
    // Called when a client disconnects. This does not get called between
    // load games.
    virtual void ClientDisconnect(PlayerClient* ent) = 0;
    //called whenever the player updates a userinfo variable.

    // The game can override any of the settings in place
    // (forcing skins or names, etc) before copying it off.
    virtual void ClientUserinfoChanged(Entity* ent, char *userinfo) = 0;
    // Called in order to process "obituary" updates, aka with what weapon did this client
    // or did other clients, kill any other client/entity.
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) = 0;
    

    //
    // Client related faciliated functions.
    //
    // This is only called when the game first initializes in single player,
    // but is called after each death and level change in deathmatch
    virtual void InitializeClientPersistentData(ServersClient* client) = 0;
    // This is only called when the game first initializes in single player,
    // but is called after each death and level change in deathmatch
    virtual void InitializeClientRespawnData(ServersClient *client) = 0;

    // Choose any info_player_start or its derivates, it'll do a subclassof check, so the only valid classnames are
    // those who have inherited from info_player_start. (info_player_deathmatch, etc).
    virtual void SelectClientSpawnPoint(Entity* ent, vec3_t& origin, vec3_t& angles, const std::string &classname) = 0;
    // Called when a player connects to a single and multiplayer. 
    // In the case of a SP mode death, the loadmenu pops up and selecting a load game
    // will restart the server.
    // In thecase of a MP mode death however, after a small intermission time, it'll
    // call this function again to respawn our player.
    virtual void PutClientInServer(Entity* ent) = 0;
    // Respawns a client (if that is what the game mode wants).
    virtual void RespawnClient(PlayerClient* ent) = 0;

     // Some information that should be persistant, like health,
    // is still stored in the edict structure, so it needs to
    // be mirrored out to the client structure before all the
    // edicts are wiped.
    virtual void SaveClientEntityData(void) = 0;
    // Fetch client data that was stored between previous entity wipe session.
    virtual void FetchClientEntityData(Entity* ent) = 0;


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
    virtual BaseEntityVector FindBaseEnitiesWithinRadius(const vec3_t &origin, float radius, uint32_t excludeSolidFlags) = 0;


    //
    // Combat GameMode actions.
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
    // When implementing this interface, it is suggested to just take DefaultGameMode,
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
    int32_t meansOfDeath;
};

#endif // __SVGAME_GAMEMODES_IGAMEMODE_H__