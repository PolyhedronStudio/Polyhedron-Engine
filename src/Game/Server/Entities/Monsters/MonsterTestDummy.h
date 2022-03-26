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
class SVGBaseMonster;


class MonsterTestDummy : public SVGBaseMonster {
public:
    //! Constructor/Deconstructor.
    MonsterTestDummy(Entity* svEntity);
    virtual ~MonsterTestDummy();

    DefineMapClass("monster_testdummy", MonsterTestDummy, SVGBaseMonster);

    /**
    *
    *   Interface functions.
    *
    **/
    void Precache() override;
    void Spawn() override;
    void Respawn() override;
    void PostSpawn() override;
    void Think() override;

    void SpawnKey(const std::string& key, const std::string& value) override;


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
    void MonsterTestDummyThink(void);
    void MonsterTestDummyStartAnimation(void);
    void MonsterTestDummyDie(IServerGameEntity* inflictor, IServerGameEntity* attacker, int damage, const vec3_t& point);
    //void MonsterTestTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf);

    // Set when exploding, after a minor delay.
    //void MonsterTestBoxExplode(void);

private:

};
