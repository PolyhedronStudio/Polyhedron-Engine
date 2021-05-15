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
    void PreCache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

private:

};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__