/*
// LICENSE HERE.

//
// MiscCapsuleMesh.h
//
//
*/
#ifndef __SVGAME_ENTITIES_MISC_MISCCAPSULEMESH_H__
#define __SVGAME_ENTITIES_MISC_MISCCAPSULEMESH_H__

class SVGBaseEntity;
class SVGBaseTrigger;

class MiscCapsuleMesh : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    MiscCapsuleMesh(PODEntity *svEntity);
    virtual ~MiscCapsuleMesh() = default;

    DefineMapClass( "misc_capsulemesh", MiscCapsuleMesh, SVGBaseTrigger );

    //
    // Interface functions. 
    //
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value) override;

    //
    // Callback Functions.
    //
    void CapsuleMeshThink(void);
	void CapsuleMeshUse( IServerGameEntity* caller, IServerGameEntity* activator );
    void CapsuleMeshDropToFloor(void);
    void CapsuleMeshDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
    void CapsuleMeshTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
	void CapsuleMeshStop();
	void CapsuleMeshTakeDamage(IServerGameEntity* other, float kick, int32_t damage, const vec3_t &damageDirection );

    // Set when exploding, after a minor delay.
    void MiscCapsuleMeshExplode(void);

private:
    // Function to spawn "debris1/tris.md2" chunks.
    void SpawnDebris1Chunk();

    // Function to spawn "debris2/tris.md2" chunks.
    void SpawnDebris2Chunk();

    // Function to spawn "debris3/tris.md2" chunks.
    void SpawnDebris3Chunk(const vec3_t& origin);
};

#endif // __SVGAME_ENTITIES_MISC_MISCSPHEREBALL_H__