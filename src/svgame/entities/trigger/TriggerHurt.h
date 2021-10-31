/*
// LICENSE HERE.

//
// TriggerHurt.h
//
// Trigger Hurt brush entity.
//
*/
#ifndef __SVGAME_ENTITIES_TRIGGER_TRIGGERHURT_H__
#define __SVGAME_ENTITIES_TRIGGER_TRIGGERHURT_H__

class TriggerHurt : public SVGBaseTrigger {
public:
    //
    // Constructor/Deconstructor.
    //
    TriggerHurt(Entity* svEntity);
    virtual ~TriggerHurt();

    DefineMapClass( "trigger_hurt", TriggerHurt, SVGBaseTrigger );

    //
    // Interface functions. 
    //
    void Precache() override;    // Precaches data.
    void Spawn() override;       // Spawns the entity.
    void Respawn() override;     // Respawns the entity.
    void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    void Think() override;       // General entity thinking routine.

    void SpawnKey(const std::string& key, const std::string& value)  override;

    //
    // Trigger functions.
    //

    // Callback functions.
    void TriggerHurtTouch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    void TriggerHurtUse(SVGBaseEntity* other, SVGBaseEntity* activator);

    //
    // Get/Set
    // 

protected:
    //
    // Other base entity members. (These were old fields in edict_t back in the day.)
    //
    // The time this entity has last been hurting anyone else. It is used for the slow damage flag.
    float lastHurtTime;

    //
    // Entity pointers.
    // 

public:

};

#endif // __SVGAME_ENTITIES_TRIGGER_TRIGGERHURT_H__