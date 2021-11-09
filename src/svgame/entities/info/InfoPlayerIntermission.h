/*
// LICENSE HERE.

//
// InfoPlayerIntermission.h
//
//
*/
#pragma once

class SVGBaseEntity;
class InfoPlayerStart;

class InfoPlayerIntermission : public InfoPlayerStart {
public:
    // Constructor/Deconstructor.
    InfoPlayerIntermission(Entity* svEntity);
    virtual ~InfoPlayerIntermission();

    DefineMapClass( "info_player_intermission", InfoPlayerIntermission, InfoPlayerStart );

    // Interface functions. 
    void Precache();    // Precaches data.
    void Spawn();       // Spawns the entity.
    void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think();       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

private:

};
