/***
*
*	License here.
*
*	@file
*
*	"Test Dummy" test monster. There'll likely be more varieties of the test dummy,
*   perhaps some with different movement types, perhaps some with other experiments.
* 
*   Either way, it's one hella fun to do this so let's go!
*
***/
#pragma once

class SVGBaseEntity;
class SVGBaseTrigger;
//class SVGBaseAnimated;
class SVGBaseSkeletalAnimator;
class SVGBaseStepMonster;


class MonsterStepDummy : public SVGBaseStepMonster {
public:
    //! Constructor/Deconstructor.
    MonsterStepDummy(PODEntity *svEntity);
    virtual ~MonsterStepDummy() = default;

    DefineMapClass("monster_stepdummy", MonsterStepDummy, SVGBaseStepMonster);

    /**
    *
    *   Interface functions.
    *
    **/
    /**
    *   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
    **/
    virtual void Precache() override;    // Precaches data.
    /**
    *   @brief  Called when it is time to spawn this entity.
    **/
    virtual void Spawn() override;       // Spawns the entity.
    /**
    *   @brief  Called when it is time to respawn this entity.
    **/
    virtual void Respawn() override;     // Respawns the entity.
    /**
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  General entity thinking routine.
    **/
    virtual void Think() override;

    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.



    /**
    *
    *   TestDummy functions.
    *
    **/    
    /**
    *   @brief
    **/


    /**
    *
    *   Callback functions.
    *
    **/  
    //void MonsterTestBoxUse(SVGBaseEntity* caller, SVGBaseEntity* activator);
    void MonsterStepDummyThink(void);
    void MonsterStepDummyStartAnimation(void);
    void MonsterStepDummyDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
    //void MonsterTestTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

    // Set when exploding, after a minor delay.
    //void MonsterTestBoxExplode(void);

private:

};
