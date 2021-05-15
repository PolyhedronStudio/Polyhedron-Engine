/*
// LICENSE HERE.

//
// SVGBaseEntity.h
//
// Base entity class, where the fun begins. All entities are inherited from this,
// one way or the other :)
//
*/
#ifndef __SVGAME_ENTITIES_BASE_SVGBASEENTITY_H__
#define __SVGAME_ENTITIES_BASE_SVGBASEENTITY_H__

class SVGBaseEntity {
public:
    //
    // Function pointers for actual callbacks.
    //
    typedef void (SVGBaseEntity::* ThinkCallBackPointer)(void);
    typedef void (SVGBaseEntity::* UseCallBackPointer)(SVGBaseEntity* activator, SVGBaseEntity* caller, float value);
    typedef void (SVGBaseEntity::* TouchCallBackPointer)(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    typedef void (SVGBaseEntity::* BlockedCallBackPointer)(SVGBaseEntity* other);
    typedef void (SVGBaseEntity::* TakeDamageCallBackPointer)(SVGBaseEntity* attacker, SVGBaseEntity* inflictor, int damageFlags, float damage);
    typedef void (SVGBaseEntity::* DieCallBackPointer)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

    //
    // Constructor/Deconstructor.
    //
    SVGBaseEntity(Entity* svEntity);
    virtual ~SVGBaseEntity();

    //
    // Interface functions. 
    //
    virtual void PreCache();    // Precaches data.
    virtual void Spawn();       // Spawns the entity.
    virtual void PostSpawn();   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    virtual void Think();       // General entity thinking routine.

    void Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    //
    // Functions.
    //
    Entity* GetServerEntity();

private:
    // The actual entity this class is a member of.
    Entity *serverEntity;

public:
    //
    // Ugly, but effective callback SET methods.
    //
    // Sets the 'Think' callback function.
    template<typename function>
    inline void SetThink(function f)
    {
        thinkFunction = static_cast<ThinkCallBackPointer>(f);
    }

    // Sets the 'Touch' callback function.
    template<typename function>
    inline void SetTouch(function f)
    {

        touchFunction = static_cast<TouchCallBackPointer>(f);
    }

    // Sets the 'Die' callback function.
    template<typename function>
    inline void SetDie(function f)
    {

        dieFunction = static_cast<DieCallBackPointer>(f);
    }


protected:
    //
    // Callback function pointers.
    //
    void						(SVGBaseEntity::* thinkFunction)(void);
    void						(SVGBaseEntity::* useFunction)(SVGBaseEntity* activator, SVGBaseEntity* caller, float value);
    void						(SVGBaseEntity::* touchFunction)(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    void						(SVGBaseEntity::* blockedFunction)(SVGBaseEntity* other);
    void						(SVGBaseEntity::* takeDamageFunction)(SVGBaseEntity* attacker, SVGBaseEntity* inflictor, int damageFlags, float damage);
    void						(SVGBaseEntity::* dieFunction)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
};

#endif // __SVGAME_ENTITIES_BASE_CBASEENTITY_H__