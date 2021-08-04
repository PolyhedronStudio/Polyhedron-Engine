/*
// LICENSE HERE.

//
// MiscExplosionBox.h
//
//
*/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;

class MiscServerModel : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    MiscServerModel(Entity* svEntity);
    virtual ~MiscServerModel();

    DefineMapClass("misc_servermodel", MiscServerModel, SVGBaseTrigger);

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
    //void MiscServerModelBoxUse(SVGBaseEntity* caller, SVGBaseEntity* activator);
    void MiscServerModelThink(void);
    //void MiscServerModelDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    //void MiscServerModelTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    // Set when exploding, after a minor delay.
    //void MiscServerModelBoxExplode(void);

private:
    // The actual frame that this model its animation should start off with.
    int32_t startFrame{ 0 };

    // The actual frame that this model its animation should end at.
    int32_t endFrame{ 0 };

    // The bounding box its bottom left, this can be custom set in map editors.
    vec3_t boundingBoxMins = { -16, -16, 0 };
    vec3_t boundingBoxMaxs = { 16, 16, 40 };
};
