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
    using ThinkCallbackPointer      = void(SVGBaseEntity::*)(void);
    using UseCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* other, SVGBaseEntity* activator);
    using TouchCallbackPointer      = void(SVGBaseEntity::*)(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    using BlockedCallbackPointer    = void(SVGBaseEntity::*)(SVGBaseEntity* other);
    using TakeDamageCallbackPointer = void(SVGBaseEntity::*)(SVGBaseEntity* other, float kick, int32_t damage);
    using DieCallbackPointer        = void(SVGBaseEntity::*)(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);

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
    void Use(SVGBaseEntity* other, SVGBaseEntity* activator);
    void Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point);
    void Blocked(SVGBaseEntity* other);
    void Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf);
    void TakeDamage(SVGBaseEntity* other, float kick, int32_t damage);

    //
    // Entity Get Functions.
    //
    // Return the 'activatorPtr' entity pointer.
    SVGBaseEntity* GetActivator() {
        return activatorPtr;
    }

    // Return the bounding box absolute 'min' value.
    inline const vec3_t& GetAbsoluteMin() {
        return serverEntity->absMin;
    }

    // Return the bounding box absolute 'max' value.
    inline const vec3_t& GetAbsoluteMax() {
        return serverEntity->absMax;
    }

    // Return the 'client' pointer.
    gclient_s* GetClient() {
        return serverEntity->client;
    }

    // Return the 'damage' value.
    inline const int32_t GetDamage() {
        return serverEntity->damage;
    }

    // Return the 'enemyPtr' entity pointer.
    SVGBaseEntity* GetEnemy() {
        return enemyPtr;
    }

    // Return the 'oldEnemyPtr' entity pointer.
    SVGBaseEntity* GetOldEnemy() {
        return oldEnemyPtr;
    }

    // Return the 'flags' value.
    inline const int32_t GetFlags() {
        return serverEntity->flags;
    }

    // Return the 'health' value.
    inline const int32_t GetHealth() {
        return serverEntity->health;
    }

    // Get the 'inuse' value.
    inline qboolean GetInUse() {
        return serverEntity->inUse;
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
        return moveType;
    }

    // Return the 'nextThinkTime' value.
    inline const int32_t GetNextThinkTime() {
        return serverEntity->nextThinkTime;
    }

    // Return the 'origin' value.
    inline const vec3_t &GetOrigin() {
        return serverEntity->state.origin;
    }

    // Returns the 'serverFlags' value.
    inline const int32_t GetServerFlags() {
        return serverEntity->serverFlags;
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

    // Return the 'velocity' value.
    inline const vec3_t& GetVelocity() {
        return serverEntity->velocity;
    }


    //
    // Entity Set Functions.
    //
    // Set the 'activatorPtr' pointer.
    inline void SetActivator(SVGBaseEntity* activator) {
        this->activatorPtr = activator;
    }

    // Set the 'mins', and 'maxs' values of the entity bounding box.
    inline void SetBoundingBox(const vec3_t& mins, const vec3_t& maxs) {
        serverEntity->mins = mins;
        serverEntity->maxs = maxs;
    }

    // Set the 'damage' value.
    inline void SetDamage(const int32_t &damage) {
        serverEntity->damage = damage;
    }

    // Set the 'enemyPtr' pointer.
    inline void SetEnemy(SVGBaseEntity* enemy) {
        this->enemyPtr = enemy;
    }

    // Set the 'oldEnemyPtr' pointer.
    inline void SetOldEnemy(SVGBaseEntity* oldEnemy) {
        this->oldEnemyPtr = oldEnemy;
    }

    // Set the 'flags' value.
    inline void SetFlags(const int32_t &flags) {
        serverEntity->flags = flags;
    }

    // Set the 'health' value.
    inline void SetHealth(const int32_t &health) {
        serverEntity->health = health;
    }

    // Set the 'inuse' value.
    inline void SetInUse(const qboolean& inUse) {
        serverEntity->inUse = inUse;
    }

    // Set the 'mass' value.
    inline void SetMass(const int32_t &mass) {
        serverEntity->mass = mass;
    }

    // Set the 'modelIndex, modelIndex1, modelIndex2, modelIndex3' values.
    inline void SetModelIndex(const int32_t& index) {
        serverEntity->state.modelIndex = index;
    }
    inline void SetModelIndex2(const int32_t& index) {
        serverEntity->state.modelIndex2 = index;
    }
    inline void SetModelIndex3(const int32_t& index) {
        serverEntity->state.modelIndex3 = index;
    }
    inline void SetModelIndex4(const int32_t& index) {
        serverEntity->state.modelIndex4 = index;
    }

    // Set the 'model' value.
    inline void SetModel(const char* model) {
        // Set model.
        serverEntity->model = model;

        // Set model index.
        SetModelIndex(gi.ModelIndex(GetModel()));
    }

    // Set the 'nextThinkTime' value.
    inline void SetMoveType(const int32_t &moveType) {
        this->moveType = moveType;
    }

    // Set the 'nextThinkTime' value.
    inline void SetNextThinkTime(const float& nextThinkTime) {
        serverEntity->nextThinkTime = nextThinkTime;
    }

    // Set the 'origin' value.
    inline void SetOrigin(const vec3_t& origin) {
        serverEntity->state.origin = origin;
    }

    // Returns the 'serverFlags' value.
    inline void SetServerFlags(const int32_t &serverFlags) {
        serverEntity->serverFlags = serverFlags;
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

    // Set the 'velocity' value.
    inline void GetVelocity(const vec3_t &velocity) {
        serverEntity->velocity = velocity;
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

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    // Move Type. (MoveType:: ... )
    int32_t moveType;

    // Current active enemy, NULL if not any.    
    SVGBaseEntity *enemyPtr;

    // Old enemy, NULL if not any.
    SVGBaseEntity *oldEnemyPtr;

    // Entity that activated this entity, NULL if none.
    SVGBaseEntity *activatorPtr;

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

    // Sets the 'Use' callback function.
    template<typename function>
    inline void SetUseCallback(function f)
    {
        useFunction = static_cast<UseCallbackPointer>(f);
    }

    // Sets the 'Touch' callback function.
    template<typename function>
    inline void SetTouchCallback(function f)
    {
        touchFunction = static_cast<TouchCallbackPointer>(f);
    }

    // Sets the 'Blocked' callback function.
    template<typename function>
    inline void SetBlockedCallback(function f)
    {
        blockedFunction = static_cast<BlockedCallbackPointer>(f);
    }

    // Sets the 'SetTakeDamage' callback function.
    template<typename function>
    inline void SetTakeDamageCallback(function f)
    {
        takeDamageFunction = static_cast<TakeDamageCallbackPointer>(f);
    }
    inline qboolean HasTakeDamageCallback() {
        return (takeDamageFunction != nullptr ? true : false);
    }

    // Sets the 'Die' callback function.
    template<typename function>
    inline void SetDieCallback(function f)
    {
        dieFunction = static_cast<DieCallbackPointer>(f);
    }


protected:
    //
    // Callback function pointers.
    //
    ThinkCallbackPointer        thinkFunction;
    UseCallbackPointer          useFunction;
    TouchCallbackPointer        touchFunction;
    BlockedCallbackPointer      blockedFunction;
    TakeDamageCallbackPointer   takeDamageFunction;
    DieCallbackPointer          dieFunction;
};

#endif // __SVGAME_ENTITIES_BASE_CBASEENTITY_H__