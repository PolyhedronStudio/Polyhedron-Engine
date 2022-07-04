/*
// LICENSE HERE.

//
// InfoNotNull.h
//
//
*/
#pragma once

class SVGBaseEntity;

class InfoNotNull : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    InfoNotNull(PODEntity *svEntity);
    virtual ~InfoNotNull();

    DefineMapClass( "info_notnull", InfoNotNull, SVGBaseEntity );

    // Interface functions. 
    virtual void Precache() final;    // Precaches data.
    virtual void Spawn() final;       // Spawns the entity.
    virtual void PostSpawn() final;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() final;       // General entity thinking routine.

    virtual void SpawnKey(const std::string& key, const std::string& value) final;

private:

};
