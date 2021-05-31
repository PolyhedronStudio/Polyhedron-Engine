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


    //
    // Interface functions. 
    //
    virtual void Precache() override;    // Precaches data.
    virtual void Spawn() override;       // Spawns the entity.
    virtual void Respawn() override;     // Respawns the entity.
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think() override;       // General entity thinking routine.

    // Return the 'delay' value.
    inline const int32_t GetDelay() {
        return serverEntity->delay;
    }

    //
    // Entity Set Functions.
    //

    // Return the 'delay' value.
    inline const int32_t SetDelay(const int32_t& delay) {
       // this->delay = delay;
        return 0;
    }

protected:

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    //int m_strGlobalState;
    //string m_strKillTarget;
    //string m_strMessage;
    //string m_strMaster;
    //int32_t m_iUseType;
    //int32_t m_iTeam;
    //int32_t m_iValue;

    //
    // Entity pointers.
    // 
    //// Entity that activated this entity, NULL if none.
    //SVGBaseEntity* activatorEntity;
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
    //
    // Ugly, but effective callback SET methods.
    //
    // Sets the 'Think' callback function.
    template<typename function>
    inline void SetThinkCallback(function f)
    {
        thinkFunction = static_cast<ThinkCallbackPointer>(f);
    }
    inline qboolean HasThinkCallback() {
        return (thinkFunction != nullptr ? true : false);
    }

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