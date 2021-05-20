/*
// LICENSE HERE.

//
// WorldSpawn.h
//
// WorldSpawn entity definition.
//
*/
#ifndef __SVGAME_ENTITIES_WORLDSPAWN_H__
#define __SVGAME_ENTITIES_WORLDSPAWN_H__

class SVGBaseEntity;

class WorldSpawn : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    WorldSpawn(Entity* svEntity);
    virtual ~WorldSpawn();

    //
    // Interface functions. 
    //
    void Precache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

    //
    // Callback functions.
    //
    void WorldSpawnThink(void);

private:

};

#endif // __SVGAME_ENTITIES_WORLDSPAWN_H__