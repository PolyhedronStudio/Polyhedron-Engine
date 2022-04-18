/*
// LICENSE HERE.

//
// MiscExplosionBox.h
//
//
*/
#pragma once

class CLGBaseEntity;
class CLGBaseTrigger;

class MiscExplosionBox : public CLGBaseEntity { // Should be: : public CLGBaseTrigger
public:
    // Constructor/Deconstructor.
    MiscExplosionBox(PODEntity* clEntity);
    virtual ~MiscExplosionBox();

    DefineMapClass( "misc_client_explobox", MiscExplosionBox, CLGBaseEntity ); // Should be CLGBaseTrigger inherited.

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
    void ExplosionBoxUse( IClientGameEntity* caller, IClientGameEntity* activator );
    void ExplosionBoxDropToFloor(void);
    void ExplosionBoxDie(IClientGameEntity* inflictor, IClientGameEntity* attacker, int damage, const vec3_t& point);
    void ExplosionBoxTouch(IClientGameEntity* self, IClientGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

    
	/**
	*	Set when exploding, after a minor delay.
	**/
    void MiscExplosionBoxExplode(void);

private:
    // Function to spawn "debris1/tris.md2" chunks.
    void SpawnDebris1Chunk();

    // Function to spawn "debris2/tris.md2" chunks.
    void SpawnDebris2Chunk();

    // Function to spawn "debris3/tris.md2" chunks.
    void SpawnDebris3Chunk(const vec3_t& origin);
};
