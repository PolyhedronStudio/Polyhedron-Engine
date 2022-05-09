/***
*
*	License here.
*
*	@file
*
*	Base Class for Brush Entities that want to be able to move around. Comes with
*	basic support functions.
*
***/
#pragma once


// Client Game GameEntity Interface.
#include "../IClientGameEntity.h"

// Forward declare.
class CLGBasePacketEntity;
class CLGBaseTrigger;

// Callbacks used at the end of a pushmove action.
using PushMoveEndFunction = void(GameEntity*);

struct MoverState
{
    static constexpr uint32_t Top = 0U;
    static constexpr uint32_t Bottom = 1U;
    static constexpr uint32_t Up = 2U;
    static constexpr uint32_t Down = 3U;
};

//-------------------
// Contains data for keeping track of velocity based moving entities.
// (In other words, entities that aren't a: Client or AI Player.
//-------------------
struct PushMoveInfo {
    // Fixed data calculated at the spawn of a  basemover.
    vec3_t startOrigin = vec3_zero();
    vec3_t startAngles = vec3_zero();
    vec3_t endOrigin = vec3_zero();
    vec3_t endAngles = vec3_zero();

    // Mover sound indices.
    int32_t startSoundIndex = 0;
    int32_t middleSoundIndex = 0;
    int32_t endSoundIndex = 0;

    // Mover configuration.
    float acceleration = 0.f;
    float speed = 0.f;
    float deceleration = 0.f;
    float distance = 0.f;
    Frametime wait = Frametime::zero();

    // State data
    int32_t state = 0;
    vec3_t dir = vec3_zero();
    float currentSpeed = 0.f;
    float moveSpeed = 0.f;
    float nextSpeed = 0.f;
    float remainingDistance = 0.f;
    float deceleratedDistance = 0.f;

    // Callback function to use when the move has ended.
    PushMoveEndFunction* OnEndFunction = nullptr;
};

class CLGBaseMover : public CLGBaseTrigger {
public:
    //
    // Constructor/Deconstructor.
    //
    CLGBaseMover(PODEntity *svEntity);
    virtual ~CLGBaseMover() = default;

    DefineAbstractClass( CLGBaseMover, CLGBaseTrigger );

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
    virtual void SetMoveDirection(const vec3_t& angles, const qboolean resetAngles = false);

    //
    // Get/Set
    //
    // Return the 'acceleration' float value.
    virtual inline float GetAcceleration() override {
        return acceleration;
    }
    // Return the 'deceleration' float value.
    virtual inline float GetDeceleration() override {
        return deceleration;
    }
    // Return the 'speed' float value.
    virtual inline float GetSpeed() override {
        return speed;
    }
    // Return the 'endPosition' vec3_t value.
    virtual const inline vec3_t& GetEndPosition() override {
        return endPosition;
    }
    // Return the 'startPosition' vec3_t value.
    virtual const inline vec3_t& GetStartPosition() override {
        return startPosition;
    }
    // Gets the lip
    virtual const inline float& GetLip() {
        return lip;
    }

    //
    // Get/Set.
    //
    inline PushMoveInfo* GetPushMoveInfo() {
        return &moveInfo;
    }

    // Set the 'acceleration' float value.
    virtual inline void SetAcceleration(const float& acceleration) {
        this->acceleration = acceleration;
    }
    // Set the 'deceleration' float value.
    virtual inline void SetDeceleration(const float& deceleration) {
        this->deceleration = deceleration;
    }
    // Set the 'endPosition' vec3_t value.
    virtual inline void SetEndPosition(const vec3_t& endPosition) {
        this->endPosition = endPosition;
    }
    // Set the 'speed' float value.
    virtual inline void SetSpeed(const float& speed) {
        this->speed = speed;
    }
    // Set the 'startPosition' vec3_t value.
    virtual inline void SetStartPosition(const vec3_t& startPosition) {
        this->startPosition = startPosition;
    }
    // Sets the lip
    virtual inline void SetLip(const float& lip) {
        this->lip = lip;
    }

protected:
    // Calculates and returns the destination point
    // ASSUMES: startPosition and moveDirection are set properly
    vec3_t      CalculateEndPosition();

    // Swaps startPosition and endPosition, using the origin as an intermediary
    void        SwapPositions();

    // Brush movement methods
    void		BrushMoveDone();
    void		BrushMoveFinal();
    void		BrushMoveBegin();
    void		BrushMoveCalc( const vec3_t& destination, PushMoveEndFunction* function );
    // Same but for angular movement
    void        BrushAngleMoveDone();
    void        BrushAngleMoveFinal();
    void        BrushAngleMoveBegin();
    void        BrushAngleMoveCalc( PushMoveEndFunction* function );
    // Accelerative movement
    void        BrushAccelerateCalc();
    void        BrushAccelerate();
    void        BrushAccelerateThink();

    float       CalculateAccelerationDistance( float targetSpeed, float accelerationRate );

protected:

    //
    // Other base entity members. (These were old fields in edict_T back in the day.)
    //
    // The speed that this objects travels at.     
    float speed = 0.f;
    // Acceleration speed.
    float acceleration = 0.f;
    // Deceleration speed.
    float deceleration = 0.f;
    // Direction of which to head down to when moving.
    vec3_t moveDirection = vec3_zero();
    // Well, positions...
    //vec3_t position1, position2;
    // Position at where to start moving this thing from.
    vec3_t startPosition = vec3_zero();
    // Position at where to end this thing from moving at all.
    vec3_t endPosition = vec3_zero();
    // BaseMover moveInfo.
    PushMoveInfo moveInfo;
    // How far away to stop, from the destination
    float		lip{ 0.0f };
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