/*
// LICENSE HERE.

//
// Light.h
//
// Light entity definition.
//
*/
#ifndef __SVGAME_ENTITIES_LIGHT_H__
#define __SVGAME_ENTITIES_LIGHT_H__

class SVGBaseEntity;

class Light : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    Light(Entity* svEntity);
    virtual ~Light();

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
    void LightUse(SVGBaseEntity* activator, SVGBaseEntity* caller, float value);

private:

};

#endif // __SVGAME_ENTITIES_MISC_PLAYERCLIENT_H__