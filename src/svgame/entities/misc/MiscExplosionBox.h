/*
// LICENSE HERE.

//
// MiscExplosionBox.h
//
//
*/
#ifndef __SVGAME_ENTITIES_MISC_MISCEXPLOSIONBOX_H__
#define __SVGAME_ENTITIES_MISC_MISCEXPLOSIONBOX_H__

class SVGBaseEntity;

class MiscExplosionBox : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    MiscExplosionBox(Entity* svEntity);
    virtual ~MiscExplosionBox();

    // Interface functions. 
    void PreCache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

    // Functions.
    void MiscExplosionBoxThink(void);
    void MiscExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void MiscExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    // Set when exploding, after a minor delay.
    void MiscExplosionBoxExplode(void);

private:

};

#endif // __SVGAME_ENTITIES_MISC_MISCEXPLOSIONBOX_H__