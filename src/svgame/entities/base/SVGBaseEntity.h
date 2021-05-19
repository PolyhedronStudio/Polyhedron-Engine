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

    //
    // Callback functions.
    //
    void Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);

    //
    // Entity Get Functions.
    //
    // Return the 'damage' value.
    inline const int32_t GetDamage() {
        return serverEntity->damage;
    }

    // Return the 'health' value.
    inline const int32_t GetHealth() {
        return serverEntity->health;
    }
    // Return the 'mass' value.
    inline const int32_t GetMass() {
        return serverEntity->mass;
    }
    
    // Return the 'model' value.
    inline const char* GetModel() {
        return serverEntity->model;
    }

    // Return the 'movetype' value.
    inline const int32_t GetMoveType() {
        return serverEntity->moveType;
    }

    // Return the 'nextThinkTime' value.
    inline const int32_t GetNextThinkTime() {
        return serverEntity->nextThinkTime;
    }

    // Return the 'origin' value.
    inline const vec3_t &GetOrigin() {
        return serverEntity->state.origin;
    }

    // Return the 'solid' value.
    inline const uint32_t GetSolid() {
        return serverEntity->solid;
    }

    // Return the 'takeDamage' value.
    inline const int32_t GetTakeDamage() {
        return serverEntity->takeDamage;
    }

    //
    // Entity Set Functions.
    //
    // Set the 'mins', and 'maxs' values of the entity bounding box.
    inline void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) {
        serverEntity->mins = mins;
        serverEntity->maxs = maxs;
    }

    // Set the 'damage' value.
    inline void SetDamage(const int32_t &damage) {
        serverEntity->damage = damage;
    }

    // Set the 'health' value.
    inline void SetHealth(const int32_t &health) {
        serverEntity->health = health;
    }

    // Set the 'mass' value.
    inline void SetMass(const int32_t &mass) {
        serverEntity->mass = mass;
    }

    // Set the 'model' value.
    inline void SetModel(const char* model) {
        // Set model.
        serverEntity->model = model;

        // Set model index.
        serverEntity->state.modelIndex = gi.ModelIndex(GetModel());
    }

    // Set the 'nextThinkTime' value.
    inline void SetMoveType(const int32_t &moveType) {
        serverEntity->moveType = moveType;
    }

    // Set the 'nextThinkTime' value.
    inline void SetNextThinkTime(const float& nextThinkTime) {
        serverEntity->nextThinkTime = nextThinkTime;
    }

    // Set the 'origin' value.
    inline void SetOrigin(const vec3_t& origin) {
        serverEntity->state.origin = origin;
    }

    // Set the 'solid' value.
    inline void SetSolid(const uint32_t &solid) {
        serverEntity->solid = solid;
    }

    // Set the 'takeDamage' value.
    inline void SetTakeDamage(const int32_t& takeDamage) {
        serverEntity->takeDamage = takeDamage;
    }

    //
    // General Entity Functions.
    //
    // Link entity to world for collision testing using gi.LinkEntity.
    void LinkEntity();

    // Returns the server entity pointer.
    inline Entity* GetServerEntity() {
        return serverEntity;
    }


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