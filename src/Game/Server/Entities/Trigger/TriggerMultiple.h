/*
// LICENSE HERE.

//
// TriggerMultiple.h
//
// This trigger will always fire.  It is activated by the world.
//
*/
#ifndef __SVGAME_ENTITIES_TRIGGER_TRIGGERMULTIPLE_H__
#define __SVGAME_ENTITIES_TRIGGER_TRIGGERMULTIPLE_H__

class TriggerMultiple : public SVGBaseTrigger {
public:
    //
    // Constructor/Deconstructor.
    //
    TriggerMultiple(PODEntity *svEntity);
    virtual ~TriggerMultiple() = default;

    DefineMapClass( "trigger_multiple", TriggerMultiple, SVGBaseTrigger );

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
    void TriggerMultipleThinkWait(void);
    void TriggerMultipleTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);
    void TriggerMultipleEnable(IServerGameEntity* other, IServerGameEntity* activator);
    void TriggerMultipleUse(IServerGameEntity* other, IServerGameEntity* activator);

    //
    // Get/Set
    // 

protected:
    //
    // Trigger function.
    //
    void Trigger(IServerGameEntity* activator);

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    // The time this entity has last been hurting anyone else. It is used for the slow damage flag.
    //float lastHurtTime;

    //
    // Entity pointers.
    // 

public:

};

#endif // __SVGAME_ENTITIES_TRIGGER_TRIGGERMULTIPLE_H__