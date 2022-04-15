/*
// LICENSE HERE.

//
// Worldspawn.h
//
// Worldspawn entity definition.
//
*/
#pragma once

class SVGBaseEntity;

class Worldspawn : public SVGBaseEntity {
public:
    //! Constructor/Deconstructor.
    Worldspawn(PODEntity *svEntity);
    virtual ~Worldspawn() = default;

    //! Register worldspawn class as a map entity.
    DefineMapClass( "worldspawn", Worldspawn, SVGBaseEntity );

    //! Interface functions. 
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value) override;

    //! Callback functions.
    void WorldspawnThink(void);

    //! Default gravity constant.
    static constexpr int32_t DEFAULT_GRAVITY = 875;
private:
    //! Parsed gravity from key/values. (Named globalGravity to prevent collision with baseentity gravity var.)
    int32_t globalGravity = 0;
};
