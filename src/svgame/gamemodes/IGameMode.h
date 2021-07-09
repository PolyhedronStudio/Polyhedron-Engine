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

class IGameMode {
public:
    // Constructor/Deconstructor.
    IGameMode() {};
    virtual ~IGameMode() {};

    //
    // Map related, also known as the "current game".
    // 
    // Gets called at the moment the level exits, this gives the gamemode one last
    // shot to finish off any last wishes before it gets destroyed.
    virtual void OnLevelExit() = 0;


    //
    // Client related.
    //
    // Determines whether a client is allowed to connect at all.
    // Returns false in case a client is allowed to connect.
    virtual qboolean ClientCanConnect(Entity* serverEntity, char* userInfo) = 0;
    // Called when a client connects. This does not get called between
    // load games, of course.
    virtual void ClientConnect(Entity* serverEntity) = 0;
    // Called when a client has finished connecting, and is ready
    // to be placed into the game.This will happen every level load.
    virtual void ClientBegin(Entity* serverEntity) = 0;
    // This will be called once for all clients at the start of each server 
    // frame. Before running any other entities in the world.
    virtual void ClientBeginServerFrame(PlayerClient* ent) = 0;
    // Called when a client disconnects. This does not get called between
    // load games.
    virtual void ClientDisconnect(PlayerClient* ent) = 0;
    // Called in order to process "obituary" updates, aka with what weapon did this client
    // or did other clients, kill any other client/entity.
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) = 0;
    
    //
    // Combat Game Rule checks.
    //
    // Returns true if these two entities are on a same team.
    virtual qboolean OnSameTeam(SVGBaseEntity* ent1, SVGBaseEntity* ent2) = 0;
    // Returns true if the target entity can be damaged by the inflictor enemy.
    virtual qboolean CanDamage(SVGBaseEntity * target, SVGBaseEntity * inflictor) = 0;


    //
    // Specific random gameplay related functionality. 
    // (Spawning gibs, checking velocity damage etc.)
    //
    // Spawns a temporary entity for a client, this is best suited to be in game mode.
    // Allows for all modes to customize that when wished for.
    //
    // When implementing this interface, it is suggested to just take DefaultGameMode,
    // or base yours off of that anyhow.
    virtual void SpawnTempDamageEntity(int32_t type, const vec3_t& origin, const vec3_t& normal, int32_t damage) = 0;
    // Calculates the velocity for the damage given. (Used in effects, such as gibs.)
    virtual vec3_t CalculateDamageVelocity(int32_t damage) = 0;
    // This function is for setting a "means of death", aka blaster or what not.
    // The thing is, it has to be able to be overrided so hey, here we go :)
    // Can't have a global like in the old code ;-)
    //
    // TODO: WID: This stuff should move to a Game/World class.
    virtual void SetCurrentMeansOfDeath(int32_t meansOfDeath) = 0;
    virtual const int32_t& GetCurrentMeansOfDeath() = 0;

protected:
    // Means of Death, for the current client that is being processed this frame.
    int32_t meansOfDeath;
};

#endif // __SVGAME_GAMEMODES_IGAMEMODE_H__