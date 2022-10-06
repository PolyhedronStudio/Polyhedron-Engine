/***
*
*	License here.
*
*	@file
*
*	ClientGame Entity: func_plat
*
*	Will perform client side prediction to allow for smoother looking platform
*	travels. 
***/
#pragma once

/**
*	Predeclarations.
**/
class CLGBaseMover;
class TriggerAutoPlatform;


/**
*	@brief	Entity: "func_plat"
**/
class FuncPlat : public CLGBaseMover {
public:
	virtual void Think() override;
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
    DefineMapClass( "kutfunc_plat", FuncPlat, CLGBaseMover );



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
    *   @brief  Updates the entity with the data of the newly passed EntityState object.
    **/
    virtual void UpdateFromState( const EntityState *state ) override;

	/**
	*	@brief	Gives the GameEntity a chance to Spawn itself appropriately based on state updates.
	**/
	virtual void SpawnFromState( const EntityState *state ) override;

	/**
	*
	*
	*	FuncPLat
	*
	*
	***/
	/**
	*	@brief	Receive ServerGame Events, effectively allowing a slight client-side prediction.
	**/
	virtual void OnEventID(uint32_t eventID) override;

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
	*	@brief
	**/
	void		Callback_RegularFrameThink( );
	/**
	*	@brief	'Use' callback: Will try to engage 'lower' or 'raise' movement depending on
	*			the current residing state of the platform. If a Think method is already
	*			set it'll do nothing but print a developer warning.
	**/
    void        Callback_Use( IClientGameEntity* other, IClientGameEntity* activator );
	/**
	*	@brief	'Blocked' callback:
	**/
	void        Callback_Blocked( IClientGameEntity* other );
    
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
    static void OnPlatformHitTop( IClientGameEntity* self );
    static void OnPlatformHitBottom( IClientGameEntity* self );


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
