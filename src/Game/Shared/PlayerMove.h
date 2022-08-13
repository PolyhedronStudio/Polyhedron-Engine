/***
*
*	License here.
*
*	@file
*
*	SharedGame PlayerMove header. Contains the actual configuration
*   const expressions as well.
* 
***/
#pragma once


/**
*   Player Movement configuration.
*   
*   Most settings can be easily tweaked here to fine tune movement to custom
*   desires.
**/
/**
*   Acceleration Constants.
**/
static constexpr float PM_ACCEL_AIR = 3.625f; // WID: [DO NOT REMOVE THIS COMMENT] The default for lesser air control is: 2.125f
static constexpr float PM_ACCEL_AIR_MOD_DUCKED = 0.125f;
static constexpr float PM_ACCEL_GROUND = 10.f;
static constexpr float PM_ACCEL_GROUND_SLICK = 4.375f;
static constexpr float PM_ACCEL_LADDER = 16.f;
static constexpr float PM_ACCEL_SPECTATOR = 2.5f;
static constexpr float PM_ACCEL_WATER = 2.8f;

/**
*   Bounce constant when clipping against solids.
**/
static constexpr float PM_CLIP_BOUNCE = 1.01f;

/**
*   Friction constants.
**/
static constexpr float PM_FRICT_AIR = 0.1f; // WID: [DO NOT REMOVE THIS COMMENT] The default for lesser air control is: 0.075f;
static constexpr float PM_FRICT_GROUND = 6.f;
static constexpr float PM_FRICT_GROUND_SLICK = 2.f;
static constexpr float PM_FRICT_LADDER = 5.f;
static constexpr float PM_FRICT_SPECTATOR = 2.5f;
static constexpr float PM_FRICT_WATER = 2.f;

/**
*   Water gravity constant.
**/
static constexpr float PM_GRAVITY_WATER = 0.33f;

/**
*   Distances traced when seeking ground.
**/
static constexpr float PM_GROUND_DIST = .25f;
static constexpr float PM_GROUND_DIST_TRICK = 16.f;

/**
*   Speed constants; intended velocities are clamped/clipped to these.
**/
static constexpr float PM_SPEED_AIR = 285.f; // PH: Tweaked - old value: 350
static constexpr float PM_SPEED_CURRENT = 100.f;
static constexpr float PM_SPEED_DUCK_STAND = 200.f;
static constexpr float PM_SPEED_DUCKED = 140.f;
static constexpr float PM_SPEED_FALL = -700.f;
static constexpr float PM_SPEED_FALL_FAR = -900.f;
static constexpr float PM_SPEED_JUMP = 270.f;
static constexpr float PM_SPEED_LADDER = 125.f;
static constexpr float PM_SPEED_LAND = -280.f;
static constexpr float PM_SPEED_RUN = 300.f; // This is the wished for running speed. Changing it, also impacts walking speed.
static constexpr float PM_SPEED_SPECTATOR = 500.f;
static constexpr float PM_SPEED_STOP = 100.f;
static constexpr float PM_SPEED_UP = 0.1f;
static constexpr float PM_SPEED_TRICK_JUMP = 0.f;
static constexpr float PM_SPEED_WATER = 118.f;
static constexpr float PM_SPEED_WATER_JUMP = 420.f;
static constexpr float PM_SPEED_WATER_SINK = -16.f;
static constexpr float PM_SPEED_STEP = 150.f;

/**
*   General Configuration.
**/
static constexpr float PM_SPEED_MOD_WALK = 0.48f;// The walk modifier slows all user-controlled speeds.
static constexpr float PM_SPEED_JUMP_MOD_WATER = 0.66;// Water reduces jumping ability.
static constexpr float PM_STOP_EPSILON = 0.1f; // Velocity is cleared when less than this.
static constexpr float PM_NUDGE_DIST = 1.f;  // Invalid player positions are nudged to find a valid position.
static constexpr float PM_SNAP_DISTANCE = PM_GROUND_DIST; // Valid player positions are snapped a small distance away from planes.

/**
*   Step Climbing.
**/
static constexpr float PM_STEP_HEIGHT = 18.f; // The vertical distance afforded in step climbing.
static constexpr float PM_STEP_HEIGHT_MIN = 4.f;  // The smallest step that will be interpolated by the client.
static constexpr float PM_STEP_NORMAL = 0.7f; // The minimum Z plane normal component required for standing.

/**
*	Player bounding box scaling. mins = VectorScale(PM_MINS, PM_SCALE)..
**/
constexpr float PM_SCALE = 1.f;

/**
*	Makes these accessable elsewhere.
**/
//! Actual player (bounding/octagon-)box mins.
extern const vec3_t PM_MINS;
//! Actual player (bounding/octagon-)maxs.
extern const vec3_t PM_MAXS;

/**
*	Game-specific flags for PlayerMoveState.flags.
**/
static constexpr int32_t PMF_DUCKED             = (PMF_GAME << 0);  // Player is ducked
static constexpr int32_t PMF_JUMPED             = (PMF_GAME << 1);  // Player jumped
static constexpr int32_t PMF_JUMP_HELD          = (PMF_GAME << 2);  // Player's jump key is down
static constexpr int32_t PMF_ON_GROUND          = (PMF_GAME << 3);  // Player is on ground
static constexpr int32_t PMF_ON_LADDER          = (PMF_GAME << 4);  // Player is on ladder
static constexpr int32_t PMF_UNDER_WATER        = (PMF_GAME << 5);  // Player is under water
static constexpr int32_t PMF_TIME_PUSHED        = (PMF_GAME << 6);  // Time before can seek ground
static constexpr int32_t PMF_TIME_TRICK_JUMP    = (PMF_GAME << 7);  // Time eligible for trick jump
static constexpr int32_t PMF_TIME_WATER_JUMP    = (PMF_GAME << 8);  // Time before control
static constexpr int32_t PMF_TIME_LAND          = (PMF_GAME << 9); // Time before jump eligible
static constexpr int32_t PMF_GIBLET             = (PMF_GAME << 10); // Player is a giblet
static constexpr int32_t PMF_TIME_TRICK_START   = (PMF_GAME << 11); // Time until we can initiate a trick jump

/**
*	The mask of PlayerMoveState.flags affecting PlayerMoveState.time.
**/
static constexpr int32_t PMF_TIME_MASK = (
    PMF_TIME_PUSHED |
    PMF_TIME_TRICK_START |
    PMF_TIME_TRICK_JUMP | PMF_TIME_WATER_JUMP | PMF_TIME_LAND |
    PMF_TIME_TELEPORT
);

/**
*	The maximum number of entities any single player movement can impact.
**/
static constexpr int32_t PM_MAX_TOUCH_ENTS = 32;


/**
*   @brief  General player movement and capabilities classification.
*           One can add custom types up till index 32.
**/
struct PlayerMoveType {
    //! Default walking behavior, supports: Walking, jumping, falling, swimming, etc.
    static constexpr uint8_t Normal     = 0;
    //! Free-flying movement with acceleration and friction, no gravity.
    static constexpr uint8_t Spectator  = 1;
    //! Free-Flying like Spectator, excluding clipping to brushes. Meaning you can move through Entities and Walls.
    static constexpr uint8_t Noclip     = 2;
};

/**
*	The player movement structure provides context management between the
*	game modules and the player movement code.
*	
*	(in), (out), (in/out) mark which way a variable goes. Copied in to the
*	state befor processing, or copied back out of the state after processing.
**/
struct PlayerMove {
    // Movement command (in)
	ClientMoveCommand moveCommand = {};
    
    // Movement state (in/out)
	PlayerMoveState state = {};

    // Entities touched (out)
    SGTraceResult touchedEntityTraces[PM_MAX_TOUCH_ENTS];
    int32_t numTouchedEntities;

    // Pointer to the entity that is below the player. (out)
    //struct PODEntity* groundEntityPtr;
	int32_t groundEntityNumber = 0;

    // Clamped, and including kick and delta (out)
    vec3_t viewAngles = vec3_zero();

    // Bounding box size (out)
    vec3_t mins = vec3_zero(), maxs = vec3_zero();
    
    float step = 0; // Traversed step height. (out)

    // Water Type (lava, slime, water), and waterLevel.
    int32_t waterType = 0;  
    int32_t waterLevel = 0; // Water Level (1 - 3)

    // Callback functions for collision with the world and solid entities
    TraceResult (*q_gameabi Trace)(const vec3_t &start, const vec3_t &mins, const vec3_t &maxs, const vec3_t &end) = nullptr;
    int32_t     (*PointContents)(const vec3_t &point) = nullptr;
};

//
// PMove functions.
//
void PMove(PlayerMove* pmove);
