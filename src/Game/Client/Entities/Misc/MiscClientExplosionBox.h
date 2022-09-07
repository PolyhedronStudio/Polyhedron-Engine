/*
// LICENSE HERE.

//
// MiscClientExplosionBox.h
//
//
*/
#pragma once

class CLGBasePacketEntity;
class CLGBaseLocalEntity;
class CLGBaseTrigger;

class MiscClientExplosionBox : public CLGBaseLocalEntity { // Should be: : public CLGBaseTrigger
public:
    // Constructor/Deconstructor.
    MiscClientExplosionBox(PODEntity* clEntity);
    virtual ~MiscClientExplosionBox() = default;

    //DefineMapClass( "misc_client_explobox", MiscClientExplosionBox, CLGBaseLocalEntity ); // Should be CLGBaseTrigger inherited.
	DefineMapClass( "misc_client_explobox", MiscClientExplosionBox, CLGBaseLocalEntity ); // Should be CLGBaseTrigger inherited.
    
	/**
    *	Interface functions. 
    **/
    void Precache() override;
    void Spawn() override;
    void Think() override;

    void SpawnKey(const std::string& key, const std::string& value) override;


    /**
    *	Callback Functions.
    **/
    void ExplosionBoxUse(IClientGameEntity* caller, IClientGameEntity* activator );
    void ExplosionBoxDropToFloor(void);
    void ExplosionBoxDie(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point);
    void ExplosionBoxTouch(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

    void ExplosionBoxThink(void);

	/**
	*	Set when exploding, after a minor delay.
	**/
    void MiscExplosionBoxExplode(void);


	/**
	*	@brief	Gives the entity a chance to prepare the 'RefreshEntity' for the current rendered frame.
	**/
	virtual void PrepareRefreshEntity(const int32_t refreshEntityID, EntityState *currentState, EntityState *previousState, float lerpFraction) override;

private:
    // Function to spawn "debris1/tris.md2" chunks.
    void SpawnDebris1Chunk();

    // Function to spawn "debris2/tris.md2" chunks.
    void SpawnDebris2Chunk();

    // Function to spawn "debris3/tris.md2" chunks.
    void SpawnDebris3Chunk(const vec3_t& origin);
};
