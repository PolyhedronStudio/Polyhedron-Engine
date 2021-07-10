/*
// LICENSE HERE.

//
// GibEntity.h
//
// Base entity for gibs.
//
*/
#ifndef __SVGAME_ENTITIES_BASE_GIBENTITY_H__
#define __SVGAME_ENTITIES_BASE_GIBENTITY_H__

class SVGBaseEntity;

class GibEntity : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    GibEntity(Entity* svEntity);
    virtual ~GibEntity();

    DefineClass(GibEntity, SVGBaseEntity);

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
    // GibEntity functions.
    // 
    void CalculateVelocity(SVGBaseEntity *other, const int32_t& damage);

    //
    // Callback functions.
    //
    //
    void GibEntityThink();
    void GibEntityDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void GibEntityTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

protected:

private:
    void ClipGibVelocity(vec3_t &velocity);
};

#endif // __SVGAME_ENTITIES_BASE_GIBENTITY_H__