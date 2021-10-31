/*
// LICENSE HERE.

//
// SVGBaseTrigger.h
//
// Base Trigger class, for brush and point entities. (Yes we can do this.)
//
*/
#ifndef __SVGAME_ENTITIES_BASE_SVGBASETRIGGER_H__
#define __SVGAME_ENTITIES_BASE_SVGBASETRIGGER_H__

class SVGBaseTrigger : public SVGBaseEntity {
public:
    //
    // Constructor/Deconstructor.
    //
    SVGBaseTrigger(Entity* svEntity);
    virtual ~SVGBaseTrigger();

    DefineAbstractClass( SVGBaseTrigger, SVGBaseEntity );

    //
    // Interface functions. 
    //
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.
    virtual void Respawn() override;     // Respawns the entity.
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;       // General entity thinking routine.

    virtual void SpawnKey(const std::string& key, const std::string& value)  override;

    //
    // Trigger functions.
    //
    void UseTargets(SVGBaseEntity* activator);

    //
    // Get/Set
    // 
    // Return the 'activatorEntity' entity pointer.
    SVGBaseEntity* GetActivator() {
        return activatorEntity;
    }

    //
    // Entity Set Functions.
    //
    // Set the 'activatorEntity' pointer.
    inline void SetActivator(SVGBaseEntity* activator) {
        this->activatorEntity = activator;
    }

protected:
    /* legacy trigger architecture */
    //float m_flDelay;
    //virtual void(entity, int) Trigger;
    //virtual void(entity, int, float) UseTargets;

    ///* master feature */
    //virtual int(void) GetValue;
    //virtual int(void) GetMaster;

    ///* spawn setup helpers */
    //virtual void(void) InitBrushTrigger;
    //virtual void(void) InitPointTrigger;
    virtual void InitBrushTrigger();
    virtual void InitPointTrigger();

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //


    // Kill target when triggered.
    //std::string killTargetStr;
    
    // Message when triggered.
    //std::string messageStr;

    // Master trigger entity.
    //std::string masterStr;

    // Timestamp that the trigger has been called at.
    //
    // Entity pointers.
    // 
    //// Entity that activated this entity, NULL if none.
    SVGBaseEntity* activatorEntity;
    //// Current active enemy, NULL if not any.    
    //SVGBaseEntity* enemyEntity;
    //// Ground entity we're standing on.
    //SVGBaseEntity* groundEntity;
    //// Old enemy, NULL if not any.
    //SVGBaseEntity* oldEnemyEntity;
    //// Team Chain Pointer.
    //SVGBaseEntity* teamChainEntity;
    //// Master Pointer.
    //SVGBaseEntity* teamMasterEntity;

public:


protected:
    //
    // Callback function pointers.
    //
    //ThinkCallbackPointer        thinkFunction;
    //UseCallbackPointer          useFunction;
    //TouchCallbackPointer        touchFunction;
    //BlockedCallbackPointer      blockedFunction;
    //TakeDamageCallbackPointer   takeDamageFunction;
    //DieCallbackPointer          dieFunction;
};

#endif // __SVGAME_ENTITIES_BASE_CBASEENTITY_H__