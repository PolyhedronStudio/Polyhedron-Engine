/*
// LICENSE HERE.

//
// GibEntity.h
//
// Base entity for gibs.
//
*/
#pragma once

class CLGBaseLocalEntity;

class GibEntity : public CLGBaseLocalEntity {
public:
    /**
    *   @brief  Used by game modes to spawn server side gibs.
    **/
    static GibEntity* Create(const vec3_t &origin, const vec3_t &size, const vec3_t &velocity, const std::string& gibModel, int32_t damage, int32_t gibType);

private:
    //! Private constructor. Gibs are created using the Create function.
    GibEntity(PODEntity *svEntity);
    virtual ~GibEntity() = default;

public:
	DefineGameClass( GibEntity, CLGBaseLocalEntity);
	//DefineClass( GibEntity, IClientGameEntity);
	//DefineMapClass( "GibEntity", CLGBaseLocalEntity, IClientGameEntity);
	//DefineGameClass( GibEntity, IClientGameEntity);
    //DefineGameClass("GibEntity", GibEntity, CLGBaseLocalEntity);

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
	void GibEntityStopBleeding();
    void GibEntityDie(GameEntity* inflictor, GameEntity* attacker, int damage, const vec3_t& point);
    void GibEntityTouch(GameEntity* self, GameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

protected:

private:

};

