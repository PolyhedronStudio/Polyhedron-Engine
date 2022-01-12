/*
// LICENSE HERE.

//
// BodyCorpse.h
//
// Class entity to accommodate body corpses with.
// These serve for client entities when they die, and get pushed into the body queue.
//
*/
#ifndef __SVGAME_ENTITIES_BASE_BODYCORPSE_H__
#define __SVGAME_ENTITIES_BASE_BODYCORPSE_H__

class SVGBaseEntity;

class BodyCorpse : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    BodyCorpse(Entity* svEntity);
    virtual ~BodyCorpse();

    DefineClass(BodyCorpse, SVGBaseEntity);

    //
    // Interface functions. 
    //
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

    //
    // Callback functions.
    //
    //
    void BodyCorpseDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void BodyCorpseTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    //
    // Get/Set
    //


protected:

private:

};

#endif // __SVGAME_ENTITIES_BASE_BODYCORPSE_H__