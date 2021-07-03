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
class SVGBaseTrigger;

class MiscExplosionBox : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    MiscExplosionBox(Entity* svEntity);
    virtual ~MiscExplosionBox();

    DefineMapClass( "misc_explobox", MiscExplosionBox, SVGBaseTrigger );

    //
    // Interface functions. 
    //
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    //
    // Callback Functions.
    //
    void ExplosionBoxUse( SVGBaseEntity* caller, SVGBaseEntity* activator );
    void ExplosionBoxThink(void);
    void ExplosionBoxDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void ExplosionBoxTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    // Set when exploding, after a minor delay.
    void MiscExplosionBoxExplode(void);

private:
    // Function to spawn "debris1/tris.md2" chunks.
    void SpawnDebris1Chunk();

    // Function to spawn "debris2/tris.md2" chunks.
    void SpawnDebris2Chunk();

    // Function to spawn "debris3/tris.md2" chunks.
    void SpawnDebris3Chunk(const vec3_t& origin);
};

#endif // __SVGAME_ENTITIES_MISC_MISCEXPLOSIONBOX_H__