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
    // Set/Get
    // 
    // Set the 'endFrame' value.
    inline void SetEndFrame(const uint32_t endFrame) {
        this->endFrame = endFrame;
    }

    // Set the 'startFrame' value.
    inline void SetStartFrame(const uint32_t startFrame) {
        this->startFrame = startFrame;
    }

    // Get the 'endFrame' value.
    inline const uint32_t GetEndFrame() {
        return this->endFrame;
    }

    // Get the 'noisePath' value.
    inline const std::string& GetNoisePath() {
        return this->noisePath;
    }

    // Get the 'startFrame' value.
    inline const uint32_t GetStartFrame() {
        return this->startFrame;
    }

    // Get the 'boundingboxMaxs' value.
    inline const vec3_t& GetBoundingBoxMaxs() {
        return this->boundingBoxMaxs;
    }

    // Get the 'boundingboxMins' value.
    inline const vec3_t& GetBoundingBoxMins() {
        return this->boundingBoxMins;
    }

    //
    // Callback Functions.
    //
    //void MiscServerModelBoxUse(SVGBaseEntity* caller, SVGBaseEntity* activator);
    void MiscServerModelThink(void);
    void MiscServerModelDie(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    //void MiscServerModelTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    // Set when exploding, after a minor delay.
    //void MiscServerModelBoxExplode(void);

private:
    // The noise path that got parsed and is in use.
    std::string noisePath{ "" };
    uint32_t precachedNoiseIndex{ 0 };

    // The actual frame that this model its animation should start off with.
    int32_t startFrame{ 0 };

    // The actual frame that this model its animation should end at.
    int32_t endFrame{ 0 };

    // The bounding box its bottom left, this can be custom set in map editors.
    vec3_t boundingBoxMins = { -16, -16, 0 };
    vec3_t boundingBoxMaxs = { 16, 16, 40 };
};
