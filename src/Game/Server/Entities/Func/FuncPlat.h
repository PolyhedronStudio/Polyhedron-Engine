/***
*
*	License here.
*
*	@file
*
*	ClientGame BaseTrigger Entity.
* 
***/
#pragma once

/**
*	Predeclarations.
**/
class SVGBaseMover;
class TriggerAutoPlatform;


/**
*	@brief	Entity: "func_plat"
**/
class FuncPlat : public SVGBaseMover {
public:
	/**
	*	Friend Class(-es).
	**/
    friend class TriggerAutoPlatform;

	/**
	*	Constructor/Destructor.
	**/
    FuncPlat( Entity* entity );
    virtual ~FuncPlat() = default;

	/**
	*	Class Definition.
	**/
    DefineMapClass( "func_plat", FuncPlat, SVGBaseMover );



	/**
	*	Spawnflags.
	**/
    //! Old.
	//static constexpr int32_t SF_PlatLowTriggered    = 1 << 1;
	


	//! Instead of waiting before moving to its opposite state(top or bottom), it requires a triggering of any sorts instead.
    static constexpr int32_t SF_PlatformToggle		= 1 << 2;
	//! The platform's default position is set to its 'bottom' state position. This flag
	//! revers that and uses the 'top' position instead.
	static constexpr int32_t SF_PlatformStartAtTop	= 1 << 3;
	//! Disables the platform from being triggered on 'top' by Touch.
	static constexpr int32_t SF_Platform_DisableTopTouchTrigger = 1 << 4;
	//! Disables the platform from being triggered from 'bottom' by Touch.
	static constexpr int32_t SF_Platform_DisableBottomTouchTrigger = 1 << 5;

	//! If either touch trigger direction is disabled, this flag Will stop the 
	//! platform from automatically moving back after 'wait' time has passed.
	static constexpr int32_t SF_Platform_DisableAutoReverse = 1 << 6;


    /**
    *
	*
    *   Base Inheritance.
    *
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
    *   @brief  PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    **/
    virtual void PostSpawn() override;   // PostSpawning is for handling entity references, since they may not exist yet during a spawn period.
    /**
    *   @brief  Act upon the parsed key and value.
    **/
    virtual void SpawnKey(const std::string& key, const std::string& value) override; // Called for each key:value when parsing the entity dictionary.


	/**
	*	SVGBaseMover Implementations.
	**/
    // Height.
    inline const float& GetHeight() {
        return this->height;
    }
    inline void SetHeight(const float& height) {
        this->height = height;
    }



protected:
    //
    // Callbacks for FuncPlat.
    //
    void        PlatformUse( IServerGameEntity* other, IServerGameEntity* activator );
    void        PlatformBlocked( IServerGameEntity* other );
    
    void        PlatformGoUp();
    void        PlatformGoDown();

    //
    // Inner workings.
    //
    virtual void DoGoUp();
    virtual void DoGoDown();

    void        HitTop();
    void        HitBottom();

    //
    // Callbacks for moveinfo.
    // 
    // These are leftovers from the legacy brush movement functions
    // Soon, we'll have a... better way... of doing this
    static void OnPlatformHitTop( IServerGameEntity* self );
    static void OnPlatformHitBottom( IServerGameEntity* self );

    //
    // Private Utilities.
    //
    void CalculateMoveSpeed();
	/**
	*	@brief	Spawns the invisible 'top' touch trigger box.
	**/
    void SpawnTopTouchTrigger();
	/**
	*	@brief	Spawns the invisible 'bottom' touch trigger box.
	**/
	void SpawnBottomTouchTrigger();

    //
    // Member Variables.
    //
    // Waiting time until it can be triggered again.
    float debounceTouchTime = 0.0f;
    // Height distance for travelling.
    float height = 0.f;

    // Sound file to use for when showing a message.
    static constexpr const char* MessageSoundPath = "misc/talk.wav";
};










///**
//*   @brief  Executes the stepslide movement.
//**/
//static void PM_StepSlideMove(void)
//{
//    // Store pre-move parameters
//    const vec3_t org0 = pm->state.origin;
//    const vec3_t vel0 = pm->state.velocity;
//
//	/**
//	*	Attempt to move; if nothing blocks us, we're done
//	**/    
//    if ( PM_SlideMove() == 0 ) {
//		// Attempt to step down to remain on ground
//		if ((pm->state.flags & PMF_ON_GROUND) && pm->moveCommand.input.upMove <= 0) {
//			const vec3_t down = vec3_fmaf(pm->state.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
//			const TraceResult downTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);
//
//			if (PM_CheckStep(&downTrace)) {
//				PM_StepDown(&downTrace);
//			}
//		}
//		return;
//	}
//
//
//	// Trace down to ground.
//    const vec3_t down = vec3_fmaf(pm->state.origin, PM_STEP_HEIGHT + PM_GROUND_DIST, vec3_down());
//    const TraceResult downTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, down);
//
//	// Never step up when you still have up velocity.
//	// Interesting part is, what if we check for input instead?
//	// pm->moveCommand.input.upMove > 0 &&
//	if (pm->state.velocity.z <= 0 && (downTrace.fraction == 1.0 || vec3_dot(downTrace.plane.normal, vec3_up()) < 0.7)) {
//		return;
//	}
//	
//	const vec3_t org1 = pm->state.origin;
//	const vec3_t vel1 = pm->state.velocity;
//
//	// Trace if the player was a step height higher.
//    const vec3_t up = vec3_fmaf(org0, PM_STEP_HEIGHT, vec3_up());
//    const TraceResult upTrace = PM_TraceCorrectAllSolid(org0, pm->mins, pm->maxs, up);
//
//	if ( upTrace.allSolid ) {
//		PM_Debug("Can't step up here kiddo");
//		return;
//	}
//
//	/**
//	*	Try and slide from above.
//	**/
//	const float stepSize = upTrace.endPosition.z - org0.z;
//
//	pm->state.origin = upTrace.endPosition;
//	pm->state.velocity = vel0;
//
//    PM_SlideMove();
//
//
//	/**
//	*	Push down to the ground.
//	**/
//	// Q3 used stepSize(instead of PM_STEP_HEIGHT) here, but that makes jumping look like climbing on top of things instead.
//	const vec3_t downB = vec3_fmaf(pm->state.origin, stepSize, vec3_down());
//	const TraceResult downTraceB = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, downB);
//
//	// If not all solid, update origin to end trace position.
//	if ( !downTraceB.allSolid ) {
//        if (downTraceB.ent && downTraceB.plane.normal.z >= PM_STEP_NORMAL ||
//		 SG_GetEntityNumber(downTraceB.ent) != pm->groundEntityNumber || downTraceB.plane.dist != playerMoveLocals.groundTrace.plane.dist) {
//			// Quake2 trick jump secret sauce
//			if ((pm->state.flags & PMF_ON_GROUND) || vel0.z < PM_SPEED_UP) {
//				// Set player origin.
//				pm->state.origin = downTraceB.endPosition;
//
//				// Set Step Height.
//				const float stepHeight = pm->state.origin.z - org0.z;
//				if (fabsf(stepHeight) >= PM_STEP_HEIGHT_MIN) {
//					pm->step = stepHeight;
//					PM_Debug( fmt::format( "Set pm->step to {}f.", pm->step ) );
//				} else {
//					PM_Debug( "Did not set pm->step." );
//				}
//			} else {
//				pm->step = pm->state.origin.z - org0.z;
//			}
//
//			//return;
//			//pm->step = pm->state.origin.z - playerMoveLocals.previousOrigin.z;
//		}
//
//	}
//
//	// If the fraction is < 1, clip against the plane.
//	if ( downTraceB.fraction < 1.0 ) {
//		//pm->state.origin = org1;
//		PM_ClipVelocity( pm->state.velocity, downTraceB.plane.normal, PM_CLIP_BOUNCE );
//	}
//
//	// Trace back to original position, if it can, don't step.
//	const TraceResult originalPosTrace = PM_TraceCorrectAllSolid(pm->state.origin, pm->mins, pm->maxs, org0);
//
//	if ( originalPosTrace.fraction == 1.0 ) {
//		// Use the original move
//		pm->state.origin = org1;
//
//		//pm->state.velocity = vel1;
//		pm->step = pm->state.origin.z - org0.z;
//
//		PM_Debug("Bend");
//	}
//}
