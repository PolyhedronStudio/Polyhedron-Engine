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
    using ThinkCallBackPointer      = void(SVGBaseEntity::*)(void);
    using UseCallBackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* activator, SVGBaseEntity* caller, float value);
    using TouchCallBackPointer      = void(SVGBaseEntity::*)(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    using BlockedCallBackPointer    = void(SVGBaseEntity::*)(SVGBaseEntity* other);
    using TakeDamageCallBackPointer = void(SVGBaseEntity::*)(SVGBaseEntity* attacker, SVGBaseEntity* inflictor, int damageFlags, float damage);
    using DieCallBackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

    //
    // Constructor/Deconstructor.
    //
    SVGBaseEntity(Entity* svEntity);
    virtual ~SVGBaseEntity();


    //
    // Interface functions. 
    //
    virtual void Precache();    // Precaches data.
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
    // Return the bounding box absolute 'min' value.
    inline const vec3_t& GetAbsoluteMin() {
        return serverEntity->absMin;
    }

    // Return the bounding box absolute 'max' value.
    inline const vec3_t& GetAbsoluteMax() {
        return serverEntity->absMax;
    }

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

    // Return the bounding box 'mins' value.
    inline const vec3_t& GetMins() {
        return serverEntity->mins;
    }

    // Return the bounding box 'maxs' value.
    inline const vec3_t& GetMaxs() {
        return serverEntity->maxs;
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

    // Return the 'size' value.
    inline const vec3_t& GetSize() {
        return serverEntity->size;
    }

    // Return the 'solid' value.
    inline const uint32_t GetSolid() {
        return serverEntity->solid;
    }

    // Set the 'spawnFlags' value.
    inline const int32_t GetSpawnFlags() {
        return serverEntity->spawnFlags;
    }

    // Return the 'style' value.
    inline const int32_t GetStyle() {
        return serverEntity->style;
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

    // Set the 'spawnFlags' value.
    inline void SetSpawnFlags(const int32_t& spawnFlags) {
        serverEntity->spawnFlags = spawnFlags;
    }

    // Set the 'style' value.
    inline const uint32_t SetStyle(const int32_t &style) {
        serverEntity->style = style;
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
    inline void SetThinkCallback(function f)
    {
        thinkFunction = static_cast<ThinkCallBackPointer>(f);
    }

    // Sets the 'Use' callback function.
    template<typename function>
    inline void SetUseCallback(function f)
    {
        useFunction = static_cast<UseCallBackPointer>(f);
    }

    // Sets the 'Touch' callback function.
    template<typename function>
    inline void SetTouchCallback(function f)
    {
        touchFunction = static_cast<TouchCallBackPointer>(f);
    }

    // Sets the 'Blocked' callback function.
    template<typename function>
    inline void SetBlockedCallback(function f)
    {
        blockedFunction = static_cast<BlockedCallBackPointer>(f);
    }

    // Sets the 'SetTakeDamage' callback function.
    template<typename function>
    inline void SetTakeDamageCallback(function f)
    {
        takeDamageFunction = static_cast<TakeDamageCallBackPointer>(f);
    }

    // Sets the 'Die' callback function.
    template<typename function>
    inline void SetDieCallback(function f)
    {
        dieFunction = static_cast<DieCallBackPointer>(f);
    }


protected:
    //
    // Callback function pointers.
    //
    ThinkCallBackPointer        thinkFunction;
    UseCallBackPointer          useFunction;
    TouchCallBackPointer        touchFunction;
    BlockedCallBackPointer      blockedFunction;
    TakeDamageCallBackPointer   takeDamageFunction;
    DieCallBackPointer          dieFunction;
};

#endif // __SVGAME_ENTITIES_BASE_CBASEENTITY_H__