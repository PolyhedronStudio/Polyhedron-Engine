/*
// LICENSE HERE.

//
// MiscExplosionBox.h
//
//
*/
#pragma once

class CLGBasePacketEntity;
class CLGBaseLocalEntity;
class CLGBaseTrigger;

class MiscExplosionBox : public CLGBaseTrigger { // public CLGBasePacketEntity { 
public:
    // Constructor/Deconstructor.
    MiscExplosionBox(PODEntity* clEntity);
    virtual ~MiscExplosionBox() = default;

    //DefineMapClass( "misc__explobox", MiscExplosionBox, CLGBasePacketEntity ); // Should be CLGBaseTrigger inherited.
	DefineMapClass( "xmisc_explobox", MiscExplosionBox, CLGBaseTrigger ); // Should be CLGBaseTrigger inherited.
    
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

	void MiscExplosionBoxInterpolateThink();
    
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
