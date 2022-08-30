/***
*
*	License here.
*
*	@file
*
*	ServerGame Entity: func_plat
*
*	A simple platform that can accelerate from idle to desired move speed and back.
*	It creates two touch triggers at post spawn: 'top', and 'bottom'. These can be
*	each distinctively be disabled by setting a SF_Disable***TouchTrigger spawnflag.
*	
*	By default the platform starts in its 'lowered' state and can be set to start
*	in its 'raised' state by setting the SF_PlatformStartRaised spawnflag.
* 
*	When the platform has reached a passive state, 'lowered' or 'raised, it'll take
*	the 'wait' SpawnKey as its duration for remaining in place before returning back
*	to its original spawn state. To disable this behavior and wait for it to be re-
*	triggered again you can set the SF_PlatformToggle flag.
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
	//! Instead of waiting before moving to its opposite state(top or bottom), it requires a triggering of any sorts instead.
    static constexpr int32_t SF_PlatformToggle		= 1 << 2;
	//! The platform's default position is set to its 'bottom' state position. This flag
	//! revers that and uses the 'top' position instead.
	static constexpr int32_t SF_PlatformStartRaised	= 1 << 3;
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
	*
	*
	*	FuncPLat
	*
	*
	***/
	/**
	*	@return	The height that was either set by automatic calculation or SpawnKey.
	**/
    inline const float& GetHeight() {
        return this->height;
    }
	/**
	*	@brief	Sets the height for use in calculating the movement trajectum.
	**/
	inline void SetHeight(const float& height) {
        this->height = height;
    }



protected:
	/**
	*
	*
	*	Callbacks
	*
	*
	**/
	/**
	*	@brief	'Use' callback: Will try to engage 'lower' or 'raise' movement depending on
	*			the current residing state of the platform. If a Think method is already
	*			set it'll do nothing but print a developer warning.
	**/
    void        Callback_Use( IServerGameEntity* other, IServerGameEntity* activator );
	/**
	*	@brief	'Blocked' callback:
	**/
	void        Callback_Blocked( IServerGameEntity* other );
    
	/**
	*	@brief	'EngageRaiseMove' callback: Engages the 'raise' movement process to return to
	*			its lowered state by playing the 'startSoundIndex' and calling on the 'LowerPlatform'
	*			callback. The 'LowerPlatform' callback will take control and continue setting itself
	*			as the 'Think' callback until it has reached a passive state at the 'startPosition'.
	**/
	void        Callback_EngageRaiseMove();
	/**
	*	@brief	'EngageLowerMove' callback: Engages the 'lower' movement process to return to
	*			its raised state by playing the 'startSoundIndex' and calling on the 'LowerPlatform'
	*			callback. The 'LowerPlatform' callback will take control and continue setting itself
	*			as the 'Think' callback until it has reached a passive state at the 'endPosition'.
	**/
	void        Callback_EngageLowerMove();

	/**
	*	@brief	'RaisePlatform' callback: Performs the platform 'raise' movement for the current 'think' frame.
	**/
    virtual void Callback_RaisePlatform();
	/**
	*	@brief	'LowerPlatform' callback: Performs the platform 'lower' movement for the current 'think' frame.
	**/
	virtual void Callback_LowerPlatform();

	/**
	*	@brief	'ReachedRaisedPosition' callback: Will set movestate to 'Raised', play 'endSoundIndex' and set the needed
	*			'Think' callback based on the SpawnFlags that were set.
	**/
    void        Callback_ReachedRaisedPosition();
	/**
	*	@brief	'ReachedLoweredPosition' callback: Will set movestate to 'Raised', play 'endSoundIndex' and set the needed
	*			'Think' callback based on the SpawnFlags that were set.
	**/
	void        Callback_ReachedLoweredPosition();

	

	/**
	*
	*
	*	BaseMover Callbacks: TODO: Remnant of old days. Improve base mover class by
	*	changing it to set and fire callbacks like we do anywhere else around. 
	*
	*
	**/
    // These are leftovers from the legacy brush movement functions
    // Soon, we'll have a... better way... of doing this
    static void OnPlatformHitTop( IServerGameEntity* self );
    static void OnPlatformHitBottom( IServerGameEntity* self );


	/**
	*
	*
	*	FuncPlat
	*
	*
	**/
	/**
	*	@brief	Calculates the move speed for this platform.
	**/
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
//
// Support routines for movement (changes in origin using velocity)
//

static void Move_UpdateLinearVelocity( edict_t *ent, float dist, int speed ) {
	int duration = 0;

	if( speed ) {
		duration = (float)dist * 1000.0f / speed;
		if( !duration ) {
			duration = 1;
		}
	}

	ent->s.linearMovement = speed != 0;
	if( !ent->s.linearMovement ) {
		return;
	}

	VectorCopy( ent->moveinfo.dest, ent->s.linearMovementEnd );
	VectorCopy( ent->s.origin, ent->s.linearMovementBegin );
	ent->s.linearMovementTimeStamp = game.serverTime - game.frametime;
	ent->s.linearMovementDuration = duration;
}

static void Move_Done( edict_t *ent ) {
	VectorClear( ent->velocity );
	ent->moveinfo.endfunc( ent );
	G_CallStop( ent );

	//Move_UpdateLinearVelocity( ent, 0, 0 );
}

static void Move_Watch( edict_t *ent ) {
	int moveTime;

	moveTime = game.serverTime - ent->s.linearMovementTimeStamp;
	if( moveTime >= (int)ent->s.linearMovementDuration ) {
		ent->think = Move_Done;
		ent->nextThink = level.time + 1;
		return;
	}

	ent->think = Move_Watch;
	ent->nextThink = level.time + 1;
}

static void Move_Begin( edict_t *ent ) {
	vec3_t dir;
	float dist;

	// set up velocity vector
	VectorSubtract( ent->moveinfo.dest, ent->s.origin, dir );
	dist = VectorNormalize( dir );
	VectorScale( dir, ent->moveinfo.speed, ent->velocity );
	ent->nextThink = level.time + 1;
	ent->think = Move_Watch;
	Move_UpdateLinearVelocity( ent, dist, ent->moveinfo.speed );
}

static void Move_Calc( edict_t *ent, vec3_t dest, void ( *func )( edict_t * ) ) {
	VectorClear( ent->velocity );
	VectorCopy( dest, ent->moveinfo.dest );
	ent->moveinfo.endfunc = func;
	Move_UpdateLinearVelocity( ent, 0, 0 );

	if( level.current_entity == ( ( ent->flags & FL_TEAMSLAVE ) ? ent->teammaster : ent ) ) {
		Move_Begin( ent );
	} else {
		ent->nextThink = level.time + 1;
		ent->think = Move_Begin;
	}
}