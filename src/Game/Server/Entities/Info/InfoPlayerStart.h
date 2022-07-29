/*
// LICENSE HERE.

//
// InfoPlayerStart.h
//
//
*/
#pragma once

class SVGBaseEntity;

class InfoPlayerStart : public SVGBaseEntity {
public:
    // Constructor/Deconstructor.
    InfoPlayerStart(PODEntity *svEntity);
    virtual ~InfoPlayerStart() = default;

    DefineMapClass( "info_player_start", InfoPlayerStart, SVGBaseEntity );

    // Interface functions. 
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

private:

};
