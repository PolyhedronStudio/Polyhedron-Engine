/*
// LICENSE HERE.

//
// SVGBasePusher.h
//
// Base pusher class, for buttons, platforms, anything that moves based on acceleration
// basically.
//
*/
#ifndef __SVGAME_ENTITIES_BASE_SVGBASEPUSHER_H__
#define __SVGAME_ENTITIES_BASE_SVGBASEPUSHER_H__

class SVGBaseTrigger;

class SVGBasePusher : public SVGBaseTrigger {
public:
    //
    // Constructor/Deconstructor.
    //
    SVGBasePusher(Entity* svEntity);
    virtual ~SVGBasePusher();


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
    // Pusher functions.
    //
    

    //
    // Get/Set
    //
    // Return the 'acceleration' float value.
    const float& GetAcceleration() {
        return acceleration;
    }
    // Return the 'deceleration' float value.
    const float& GetDeceleration() {
        return deceleration;
    }
    // Return the 'speed' float value.
    const float &GetSpeed() {
        return speed;
    }
    // Return the 'endPosition' vec3_t value.
    const vec3_t& GetEndPosition() {
        return endPosition;
    }
    // Return the 'startPosition' vec3_t value.
    const vec3_t& GetStartPosition() {
        return startPosition;
    }

    //
    // Entity Set Functions.
    //
    // Set the 'acceleration' float value.
    inline void SetAcceleration(const float& acceleration) {
        this->acceleration = acceleration;
    }
    // Set the 'deceleration' float value.
    inline void SetDeceleration(const float& deceleration) {
        this->deceleration = deceleration;
    }
    // Set the 'speed' float value.
    inline void SetSpeed(const float &speed) {
        this->speed = speed;
    }
    // Set the 'endPosition' vec3_t value.
    inline void SetEndPosition(const vec3_t& endPosition) {
        this->endPosition = endPosition;
    }
    // Set the 'startPosition' vec3_t value.
    inline void SetStartPosition(const vec3_t& startPosition) {
        this->startPosition = startPosition;
    }

protected:

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    // The speed that this objects travels at.     
    float speed;
    // Acceleration speed.
    float acceleration;
    // Deceleration speed.
    float deceleration;
    // Direction of which to head down to when moving.
    vec3_t moveDirection;
    // Well, positions...
    //vec3_t position1, position2;
    // Position at where to start moving this thing from.
    vec3_t startPosition;
    // Position at where to end this thing from moving at all.
    vec3_t endPosition;

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