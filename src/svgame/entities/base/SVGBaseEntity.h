/*
// LICENSE HERE.

//
// SVGBaseEntity.h
//
// Base entity class, where the fun begins. All entities are inherited from this,
// one way or the other :)
//
*/
#ifndef __SVGAME_ENTITIES_BASE_SVGBASEENTITY_H__
#define __SVGAME_ENTITIES_BASE_SVGBASEENTITY_H__

class SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    SVGBaseEntity(Entity* svEntity);
    virtual ~SVGBaseEntity();

    // Interface functions. 
    virtual void PreCache();    // Precaches data.
    virtual void Spawn();       // Spawns the entity.
    virtual void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think();       // General entity thinking routine.

    // Functions.

private:
    // The actual entity this class is a member of.
    Entity *serverEntity;
};

#endif // __SVGAME_ENTITIES_BASE_CBASEENTITY_H__