/*
// LICENSE HERE.

//
// PlayerClient.h
//
//
*/
#ifndef __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__
#define __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__

class SVGBaseEntity;

class PlayerClient : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    PlayerClient(Entity* svEntity);
    virtual ~PlayerClient();

    // Interface functions. 
    void Precache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

    //
    // Callback functions.
    //
    void PlayerClientDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

private:

};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__