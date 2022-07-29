/***
*
*	License here.
*
*	@file
*
*	ServerGame BaseMover Entity.
* 
***/
#pragma once



// Predeclarations.
class SVGBaseTrigger;


/**
*	Callbacks used at the end of a pushmove action.
**/
using PushMoveEndFunction = void(IServerGameEntity*);

struct MoverState
{
	//! The mover's state when it has finished moving up, it is at the "top".
	static constexpr uint32_t Top = 0U;
	//! The mover's state when it has finished moving down, it is at the "bottom".
	static constexpr uint32_t Bottom = 1U;
	//! The mover's state when it is moving up.
    static constexpr uint32_t Up = 2U;
	//! The mover's state when it is moving down.
    static constexpr uint32_t Down = 3U;
};

/**
*	@brief	Contains data for keeping track of velocity based moving entities.
*			(In other words, entities that aren't a: Client or AI Player.
**/
struct PushMoveInfo {
    // Fixed data calculated at the spawn of a basemover.
    vec3_t startOrigin = vec3_zero();
    vec3_t startAngles = vec3_zero();
    vec3_t endOrigin = vec3_zero();
    vec3_t endAngles = vec3_zero();

    // Mover sound indices.
    int32_t startSoundIndex = 0;
    int32_t middleSoundIndex = 0;
    int32_t endSoundIndex = 0;

    // Mover configuration.
    double acceleration = 0.f;
    double speed = 0.f;
    double deceleration = 0.f;
    double distance = 0.f;
    Frametime wait = Frametime::zero();

    // State data
    int32_t state = 0;
    vec3_t dir = vec3_zero();
    double currentSpeed = 0.f;
    double moveSpeed = 0.f;
    double nextSpeed = 0.f;
    double remainingDistance = 0.f;
    double deceleratedDistance = 0.f;

    // Callback function to use when the move has ended.
    PushMoveEndFunction* OnEndFunction = nullptr;
};

/**
*	@brief	The base mover object is a triggerable object which consists out of a Solid::BSP solid type,
*			and a MoveType::Push or MoveType::Stop.	It serves as a means to enable brushes to move, 
*			accelerating to speed, and decelerating down to a halt. 
*
*			In case of a MoveType::Stop it'll stop and have its blocked callback triggered where as in
*			case of a MoveType::Push it'll start pushing the blocking object around. All SVGBaseMover
*			brush entities themselves do not collide interact with each other.
**/
class SVGBaseMover : public SVGBaseTrigger {
public:
    // Constructor/Deconstructor.
    SVGBaseMover(PODEntity *svEntity);
    virtual ~SVGBaseMover() = default;

	// Inherit from SVGBaseTrigger.
    DefineAbstractClass( SVGBaseMover, SVGBaseTrigger );

	/**
	*	@brief	Additional spawnkeys for mover types.
	**/
    virtual void SpawnKey(const std::string& key, const std::string& value)  override;

    /**
	*	@brief
	**/
    virtual void SetMoveDirection(const vec3_t& angles, const bool resetAngles = false);

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
};