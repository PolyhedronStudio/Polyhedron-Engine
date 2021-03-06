/*
// LICENSE HERE.

//
// GibEntity.h
//
// Base entity for gibs.
//
*/
#pragma once

class SVGBaseEntity;

class GibEntity : public SVGBaseEntity {
public:
    /**
    *   @brief  Used by game modes to spawn server side gibs.
    **/
    static GibEntity* Create(GameEntity* gibber, const std::string& gibModel, int32_t damage, int32_t gibType);

private:
    //! Private constructor. Gibs are created using the Create function.
    GibEntity(PODEntity *svEntity);
    virtual ~GibEntity() = default;

public:
    DefineGameClass(GibEntity, SVGBaseEntity);

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
    // WID: These need more restructuring etc, rethinking. Doing a KISS rewrite atm ;-)
    // 
    // Can be used to clip the gib velocity.
    void ClipGibVelocity(vec3_t& velocity);

    //
    // Callback functions.
    //
    //
    void GibEntityThink();
    void GibEntityDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
    void GibEntityTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

protected:

private:

};

