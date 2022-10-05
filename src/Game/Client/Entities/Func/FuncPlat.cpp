/***
*
*	License here.
*
*	@file
* 
*   Client Side FuncPlat -> Intented to be predicted someday.
*
***/
//! Main Headers.
#include "Game/Client/ClientGameMain.h"
//! Client Game Local headers.
#include "Game/Client/ClientGameLocals.h"
//! BaseMover.
#include "Game/Client/Entities/Base/CLGBaseMover.h"
//#include "Game/Entities/Trigger/TriggerAutoDoor.h"
//#include "Game/Entities/Func/FuncAreaportal.h"
#include "Game/Client/Entities/Func/FuncPlat.h"
//! Game World.
#include "Game/Client/World/ClientGameWorld.h"



/**
*	Constructor.
**/
FuncPlat::FuncPlat( Entity* entity ) : Base( entity ) {
}

/**
*   @brief  Called when it is time to 'precache' this entity's data. (Images, Models, Sounds.)
**/
void FuncPlat::Precache() {
	// Be sure to call Precache on all Base classes.
    Base::Precache();
    //// Set up the default sounds
    //if ( GetSound() != 1 ) {
    //    moveInfo.startSoundIndex = SVG_PrecacheSound("plats/pt1_strt.wav");
    //    moveInfo.middleSoundIndex = SVG_PrecacheSound("plats/pt1_mid.wav");
    //    moveInfo.endSoundIndex = SVG_PrecacheSound("plats/pt1_end.wav");
    //}
}

void FuncPlat::Think() {
	// Think.
	Base::Think();
}

/**
*   @brief  Called when it is time to spawn this entity.
**/
void FuncPlat::Spawn() {
    // Zero out angles here for SetMoveDirection in Base::Spawn.
    SetAngles( vec3_zero() );

    // Spawn base class.
    Base::Spawn();

    // Basic properties.
    //SetMoveDirection( GetAngles(), true );
    SetMoveType( MoveType::Push );
    SetSolid( Solid::BSP );
    SetModel( GetModel() );

    //SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );
    SetBlockedCallback( &FuncPlat::Callback_Blocked );
    SetUseCallback( &FuncPlat::Callback_Use );

    // Make sure to have default KeyValues set in case they were not set
	// by the mapper.
    if ( !GetSpeed() ) {
        SetSpeed( 300.f);
    } else {
        //SetSpeed( GetSpeed() * 0.1f );
    }
	if ( GetWaitTime() == Frametime::zero() ) {
		SetWaitTime( 3s );
	}
    if ( !GetAcceleration() ) {
        SetAcceleration( 5.f );
    } else {
        SetAcceleration( GetAcceleration() * 0.5f );
    }
    if ( !GetDeceleration() ) {
        SetDeceleration( 5.f );
    } else {
        SetDeceleration( GetDeceleration() * 0.5f );
    }
    if ( !GetDamage() ) {
        SetDamage( 2 );
    }
    if ( !GetLip() ) {
        SetLip( 8 );
    }

    // Set start and end positions to origin.
    SetStartPosition( GetOrigin() );
    SetEndPosition( GetOrigin() );

	// For the 'Height' SpawnKey we got special behavior, a mapper can set it by himself to determine the
	// height this platform trajectory travel.
    if ( GetHeight() ) {
	    // Adjust endposition according to keyvalue set height.
        SetEndPosition( GetEndPosition() - vec3_t{ 0.f, 0.f, GetHeight() } );
    } else {
		// Calculate Height and EndPosition based on the actual mins, maxs and lip of this platform.
        SetEndPosition( GetEndPosition() - vec3_t{ 0.f, 0.f, ( GetMaxs().z - GetMins().z) - GetLip() } );
        SetHeight( GetEndPosition().z );
    }

	// Calculate movement speed to use.
	// Start at state up.
	if ( (spawnFlags & SF_PlatformStartRaised) ) {
		moveInfo.state = MoverState::Top;
		moveInfo.destOrigin =  GetEndPosition();
	} else {
		// Default to end(bottom) position.
		SetOrigin( GetEndPosition() );
		// Change its state.
		moveInfo.state = MoverState::Bottom;
		moveInfo.destOrigin = GetStartPosition();
	}

    // Setup move info.
    moveInfo.speed = GetSpeed();
    moveInfo.acceleration = GetAcceleration();
    moveInfo.deceleration = GetDeceleration();
    moveInfo.wait = GetWaitTime();
    moveInfo.startOrigin = GetStartPosition();
    moveInfo.startAngles = GetAngles();
    moveInfo.endOrigin = GetEndPosition();
    moveInfo.endAngles = GetAngles();

    // To simplify logic elsewhere, make non-teamed func_plats into a team of one
    if ( GetTeam().empty() ) {
        SetTeamMasterEntity( this );
    }


	//SetInUse( true );
	//SetThinkCallback( &FuncPlat::Callback_RegularFrameThink );
	//SetNextThinkTime( level.time + FRAMERATE_MS );
	// We're done, link the entity in for collision.
    LinkEntity();
}

void FuncPlat::Callback_RegularFrameThink() {
	//SetInUse( true );
	////SetThinkCallback( &FuncPlat::Callback_RegularFrameThink );
	//SetNextThinkTime( level.extrapolatedTime );
	//// We're done, link the entity in for collision.
	//LinkEntity();	
}

/**
*	@brief	Gives the GameEntity a chance to Spawn itself appropriately based on state updates.
**/
void FuncPlat::SpawnFromState(const EntityState* state) {
	if (!state) {
		return;
	}
}

/**
*	@brief	Gives the GameEntity a chance to Spawn itself appropriately based on state updates.
**/
void FuncPlat::UpdateFromState(const EntityState* state) {
	if (!state) {
		return;
	}

}

/**
*   @brief  Spawn the distinct touch triggers if they aren't disabled, set the proper state to start with,
*			calculate the move speed and fill in the basemover's MoveInfo settings.
**/
void FuncPlat::PostSpawn() {
	// Calculate movement speed to use.
	// Start at state up.
	if ( (spawnFlags & SF_PlatformStartRaised) ) {
		moveInfo.state = MoverState::Top;
		moveInfo.destOrigin =  GetEndPosition();
	} else {
		// Default to end(bottom) position.
		SetOrigin( GetEndPosition() );
		// Change its state.
		moveInfo.state = MoverState::Bottom;
		moveInfo.destOrigin = GetStartPosition();
	}

    // Setup move info.
    moveInfo.speed = GetSpeed();
    moveInfo.acceleration = GetAcceleration();
    moveInfo.deceleration = GetDeceleration();
    moveInfo.wait = GetWaitTime();
    moveInfo.startOrigin = GetStartPosition();
    moveInfo.startAngles = GetAngles();
    moveInfo.endOrigin = GetEndPosition();
    moveInfo.endAngles = GetAngles();

    // To simplify logic elsewhere, make non-teamed func_plats into a team of one
    if ( GetTeam().empty() ) {
        SetTeamMasterEntity( this );
    }

	//SetInUse( true );
	//SetThinkCallback( &FuncPlat::Callback_RegularFrameThink );
	//SetNextThinkTime( level.time + FRAMERATE_MS );
	// We're done, link the entity in for collision.
    LinkEntity();
}

/**
*	@brief	Receive ServerGame Events, effectively allowing a slight client-side prediction.
**/
void FuncPlat::OnEventID( uint32_t eventID ) {
	// TODO: Acquire the activator entity?
	uint16_t activatorEntityNumber = 0;
	
	// Get Game World.
	ClientGameWorld *gameWorld = GetGameWorld();
	auto *playerEntity = gameWorld->GetClientGameEntity();
//		auto *playerEntity = GetGameWorld()->GetGameEntityByIndex(1);
	switch( eventID ) {
	case 1: {//: FUNC_PLAT_ENGAGE_LOWER_MOVE:
		Callback_EngageLowerMove();
		break;
	} case 2: { //: FUNC_PLAT_ENGAGE_RAISE_MOVE:
		Callback_EngageRaiseMove();
		break;
	} default:

		break;
	}
}


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
void FuncPlat::Callback_Use( IClientGameEntity* other, IClientGameEntity* activator ) {
    if (HasThinkCallback()) {
    //    CLG_Print( PrintType::DeveloperWarning, fmt::format( "func_plat(#{}): Already has a 'Think' callback set. Can't 'Use'.\n", GetNumber() ) );
        return;
    }

	// Engage LowerMove.
    Callback_EngageLowerMove();
}

/**
*	@brief	'Blocked' callback:
**/
void FuncPlat::Callback_Blocked( IClientGameEntity* other ) {
    if (!other) {
        return;
    }

	// We don't need this here.
	//// Get GameWorld.
	//ClientGameWorld *gameWorld = GetGameWorld();
	//// Get GameMode.
	//IGamemode *gameMode = gameWorld->GetGameMode();

 //   if ( !(other->GetServerFlags() & EntityServerFlags::Monster) && !(other->GetClient()) ) {
 //       // Give it a chance to go away on its own terms (like gibs)
 //       gameMode->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), 10000, 1, 0, MeansOfDeath::Crush );
 //       // If it's still there, nuke it
 //       if ( other->GetHealth() > 0 || other->GetSolid() != Solid::Not ) {
 //           SVG_BecomeExplosion1( other );
 //       }
 //   }

 //   gameMode->InflictDamage( other, this, this, vec3_zero(), other->GetOrigin(), vec3_zero(), GetDamage(), 1, 0, MeansOfDeath::Crush );

    //if (moveInfo.state == MoverState::Down) {
	   // Callback_EngageRaiseMove( );
    //} else {
	   // Callback_EngageLowerMove();
    //}
}

/**
*	@brief	'EngageRaiseMove' callback: Engages the 'raise' movement process to return to
*			its lowered state by playing the 'startSoundIndex' and calling on the 'LowerPlatform'
*			callback. The 'LowerPlatform' callback will take control and continue setting itself
*			as the 'Think' callback until it has reached a passive state at the 'startPosition'.
**/
void FuncPlat::Callback_EngageRaiseMove(  ) {
	// Only the platform 'master' plays audio, not its team slaves.
    //if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
	    // Play sound.
		//if ( moveInfo.startSoundIndex ) {
	    //    SVG_Sound( this, SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.startSoundIndex, 1, Attenuation::Static, 0.0f );
	    //}

		// Set the entity sound.
	    //SetSound( moveInfo.startSoundIndex );
    //}

	// Begin raising the platform, the callback will continue setting itself as 'think'
	// callback until it has reached a passive state position again.
    Callback_RaisePlatform();
}

/**
*	@brief	'EngageLowerMove' callback: Engages the 'lower' movement process to return to
*			its raised state by playing the 'startSoundIndex' and calling on the 'LowerPlatform'
*			callback. The 'LowerPlatform' callback will take control and continue setting itself
*			as the 'Think' callback until it has reached a passive state at the 'endPosition'.
**/
void FuncPlat::Callback_EngageLowerMove() {
	// Only the platform 'master' plays audio, not its team slaves.
    //if (!(GetFlags() & EntityFlags::TeamSlave)) {
		// Play sound if we got one set.
	    //if ( moveInfo.startSoundIndex ) {
	        //SVG_Sound(this, SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.startSoundIndex, 1, Attenuation::Static, 0.0f);
	    //}

		// Set the entity sound.
	    //SetSound(moveInfo.startSoundIndex);
    //}

	// Begin lowering the platform, the callback will continue setting itself as 'think'
	// callback until it has reached a passive state position again.
    Callback_LowerPlatform();
}

/**
*	@brief	Performs the platform 'raise' movement for the current 'think' frame.
**/
void SG_Physics_Pusher( SGEntityHandle &handle );

void FuncPlat::Callback_RaisePlatform() {
    //if (!(GetFlags() & EntityFlags::TeamSlave)) {
	   // if (moveInfo.middleSoundIndex) {
	   //     SVG_Sound(this, SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.middleSoundIndex, 1, Attenuation::Static, 0.0f);
	   // }
	   // SetSound(moveInfo.middleSoundIndex);
    //}
    moveInfo.state = MoverState::Up;
	//EnableExtrapolation();

	//EnableExtrapolation();

	SGEntityHandle handlePusher;
	handlePusher = this;
	//level.time += FRAMERATE_MS;
	//LinearMove_Calc( this, GetStartPosition(), OnPlatformHitTop);
	//level.time -= FRAMERATE_MS;
	//SG_Physics_Pusher( handlePusher );
	
	
	//level.time -= FRAMERATE_MS;
	//LinearMove_Calc( this, GetStartPosition(), OnPlatformHitTop);

	////SG_Physics_Pusher( handlePusher );
	//level.time += FRAMERATE_MS;
	//level.time += FRAMERATE_MS;
	LinearMove_Calc( this, GetStartPosition(), OnPlatformHitTop);//BrushMoveCalc( moveInfo.startOrigin, OnPlatformHitTop );
	//level.time -= FRAMERATE_MS;
	//EnableExtrapolation();
	level.time += FRAMERATE_MS;
	SG_Physics_Pusher( handlePusher );
	level.time -= FRAMERATE_MS;
	EnableExtrapolation();
	const std::string debugStr = fmt::format( "RaiseMove Event: speed({}), wait({}), destOrigin({}, {}, {}), startOrigin({}, {}, {}), endOrigin({}, {}, {})\n",
				moveInfo.speed,
				moveInfo.wait.count(),
				moveInfo.destOrigin.x, moveInfo.destOrigin.y, moveInfo.destOrigin.z,
				moveInfo.startOrigin.x, moveInfo.startOrigin.y, moveInfo.startOrigin.z,
				moveInfo.endOrigin.x, moveInfo.endOrigin.y, moveInfo.endOrigin.z);
	CLG_Print( PrintType::DeveloperWarning, debugStr );
}

/**
*	@brief	Performs the platform 'lower' movement for the current 'think' frame.
**/
void FuncPlat::Callback_LowerPlatform() {
    moveInfo.state = MoverState::Down;
	//EnableExtrapolation();

	SGEntityHandle handlePusher;
	handlePusher = this;
	//level.time += FRAMERATE_MS;
	//LinearMove_Calc( this, GetStartPosition(), OnPlatformHitTop);
	//level.time -= FRAMERATE_MS;
	//SG_Physics_Pusher( handlePusher );
	
	
	//level.time -= FRAMERATE_MS;
	//LinearMove_Calc( this, GetStartPosition(), OnPlatformHitTop);

	////SG_Physics_Pusher( handlePusher );
	//level.time += FRAMERATE_MS;
	//level.time += FRAMERATE_MS;
	LinearMove_Calc( this, GetEndPosition(), OnPlatformHitBottom);//BrushMoveCalc( moveInfo.startOrigin, OnPlatformHitTop );
	//level.time -= FRAMERATE_MS;
	//EnableExtrapolation();
	level.time += FRAMERATE_MS;
	SG_Physics_Pusher( handlePusher );
	level.time -= FRAMERATE_MS;
	const std::string debugStr = fmt::format( "LowerMove Event: speed({}), wait({}), destOrigin({}, {}, {}), startOrigin({}, {}, {}), endOrigin({}, {}, {})\n",
				moveInfo.speed,
				moveInfo.wait.count(),
				moveInfo.destOrigin.x, moveInfo.destOrigin.y, moveInfo.destOrigin.z,
				moveInfo.startOrigin.x, moveInfo.startOrigin.y, moveInfo.startOrigin.z,
				moveInfo.endOrigin.x, moveInfo.endOrigin.y, moveInfo.endOrigin.z);
	CLG_Print( PrintType::DeveloperWarning, debugStr );

}

/**
*	@brief	'ReachedRaisedPosition' callback: Will set movestate to 'Raised', play 'endSoundIndex' and set the needed
*			'Think' callback based on the SpawnFlags that were set.
**/
void FuncPlat::Callback_ReachedRaisedPosition() {
    //if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
    //    if ( moveInfo.endSoundIndex ) {
    //        SVG_Sound( this, SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.endSoundIndex, 1.0f, Attenuation::Static, 0.0f );
    //    }
    //    SetSound( 0 );
    //}

    moveInfo.state = MoverState::Top;
	// We set the origin here so it won't be slightly off (timing.)
	SetOrigin( GetStartPosition() );
	LinkEntity();
//	DisableExtrapolation();


	//// When SF_PlatformToggle is set we..
	//if ( ( GetSpawnFlags() & SF_PlatformToggle) ) {
	//	// If the bottom trigger is disabled, we want it to go back up automatically instead of wait for a trigger.
	//	if (GetSpawnFlags() & SF_Platform_DisableTopTouchTrigger) {
	//		SetThinkCallback( &FuncPlat::Callback_EngageLowerMove );
	//		SetNextThinkTime( level.time + GetWaitTime() );
	//	} else {
	//		SetThinkCallback( nullptr );
	//	}
	//} else {
	//	SetThinkCallback( &FuncPlat::Callback_EngageLowerMove );
	//	SetNextThinkTime( level.time + GetWaitTime() );
	//}
}

/**
*	@brief	'ReachedLoweredPosition' callback: Will set movestate to 'Raised', play 'endSoundIndex' and set the needed
*			'Think' callback based on the SpawnFlags that were set.
**/
void FuncPlat::Callback_ReachedLoweredPosition() {
    //if ( !(GetFlags() & EntityFlags::TeamSlave) ) {
    //    if ( moveInfo.endSoundIndex ) {
    //        SVG_Sound( this, SoundChannel::IgnorePHS + SoundChannel::Voice, moveInfo.endSoundIndex, 1.0f, Attenuation::Static, 0.0f );
    //    }
    //    SetSound( 0 );
    //}
    moveInfo.state = MoverState::Bottom;
	SetOrigin( GetEndPosition() );
	LinkEntity();
//	DisableExtrapolation();
	
	//DisableExtrapolation();

	
	//// When SF_PlatformToggle is set we..
	//if ( ( GetSpawnFlags() & SF_PlatformToggle) ) {
	//	//SetThinkCallback( &SVGBaseEntity::SVGBaseEntityThinkNull );

	//	// If the bottom trigger is disabled, we want it to go back up automatically instead of wait for a trigger.
	//	if (GetSpawnFlags() & SF_Platform_DisableBottomTouchTrigger) {
	//		SetThinkCallback( &FuncPlat::Callback_EngageRaiseMove );
	//		SetNextThinkTime( level.time + GetWaitTime() );
	//	} else {
	//		SetThinkCallback( nullptr );
	//	}
	//} else {
	//	SetThinkCallback( &CLGBasePacketEntity::CLGBasePacketEntityThinkNull );
	//}
}



/**
*
*
*	BaseMover Callbacks: TODO: Remnant of old days. Improve base mover class by
*	changing it to set and fire callbacks like we do anywhere else around. 
*
*
**/
void FuncPlat::OnPlatformHitTop( IClientGameEntity* self ) {
    if (!self->IsSubclassOf<FuncPlat>()) {
//	    CLG_Print( PrintType::DeveloperWarning, "Warning: In function %s entity #%i is not a subclass of func_plat\n", __func__, self->GetNumber());
        return;
    }
    
    // Cast.
    FuncPlat* platEntity = static_cast<FuncPlat*>(self);
	platEntity->Callback_ReachedRaisedPosition();
}
void FuncPlat::OnPlatformHitBottom(IClientGameEntity *self) {
	if (!self->IsSubclassOf<FuncPlat>()) {
	//	gi.DPrintf("Warning: In function %s entity #%i is not a subclass of func_plat\n", __func__, self->GetNumber());
		return;
	}

	// Cast.
	FuncPlat *platEntity = static_cast<FuncPlat *>(self);
	platEntity->Callback_ReachedLoweredPosition();
}

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
void FuncPlat::CalculateMoveSpeed() {
	//if ( GetFlags() & EntityFlags::TeamSlave ) {
	//    return; // Only the team master does this
	//}

	//FuncPlat* ent = nullptr;
	float min = 0.f;
	float time = 0.f;
	float newSpeed = 0.f;
	float ratio = 0.f;
	float distance = 0.f;
	FuncPlat *ent = nullptr;

	// Find the smallest distance any member of the team will be moving
	min = fabsf(moveInfo.distance);
	for (ent = dynamic_cast<FuncPlat *>(GetTeamChainEntity()); (ent != nullptr && ent->IsSubclassOf<CLGBaseMover>()); ent = dynamic_cast<FuncPlat *>(ent->GetTeamChainEntity())) {
		distance = fabsf(ent->moveInfo.distance);
		if (distance < min) {
			min = distance;
		}
	}

	time = min / GetSpeed();

	// Adjust speeds so they will all complete at the same time
	for (ent = dynamic_cast<FuncPlat *>(GetTeamChainEntity()); (ent != nullptr && ent->IsSubclassOf<CLGBaseMover>()); ent = dynamic_cast<FuncPlat *>(ent->GetTeamChainEntity())) {
		newSpeed = fabsf(ent->moveInfo.distance) / time;
		ratio = newSpeed / ent->moveInfo.speed;

		if (ent->moveInfo.acceleration == ent->moveInfo.speed) {
			ent->moveInfo.acceleration = newSpeed;
		} else {
			ent->moveInfo.acceleration *= ratio;
		}

		if (ent->moveInfo.deceleration == ent->moveInfo.speed) {
			ent->moveInfo.deceleration = newSpeed;
		} else {
			ent->moveInfo.deceleration *= ratio;
		}

		// Update moveInfo variables and class member variables
		ent->SetAcceleration(ent->moveInfo.acceleration);
		ent->SetDeceleration(ent->moveInfo.deceleration);
		ent->moveInfo.speed = newSpeed;
		ent->SetSpeed(newSpeed);
	}
}

/**
*	@brief	Spawns the invisible 'top' touch trigger box.
**/
void FuncPlat::SpawnTopTouchTrigger() {
	//// Get mins and max.
	//const vec3_t mins = GetMins();
	//const vec3_t maxs = GetMaxs();

	//// We'll need these to calculate the trigger mins/maxs with.
 //   const vec3_t startPosition = GetStartPosition();
 //   const vec3_t endPosition = GetEndPosition();
 //   const float& lip = GetLip();

	//// Start calculation of the new trigger mins/maxs.
 //   vec3_t triggerMins = mins + vec3_t{ 25.0f, 25.0f, 0.f };
 //   vec3_t triggerMaxs = maxs + vec3_t{ -25.f, -25.f, 8.f };

 //   // Calculate a slightly larger box.
 //   triggerMins.x = mins.x + 25.f;
 //   triggerMins.y = mins.y + 25.f;
 //   triggerMins.z = mins.z + GetHeight();

 //   triggerMaxs.x = maxs.x - 25.f;
 //   triggerMaxs.y = maxs.y - 25.f;
 //   triggerMaxs.z = maxs.z + GetHeight() + 8;

 //   // Adjust height.
	////triggerMins.z = triggerMaxs.z - (startPosition.z - endPosition.z + lip);
	//
 //   // Ensure it is properly scaled on X and Y Axis. 
 //   if (triggerMaxs.x - triggerMins.x <= 0.f) {
 //       triggerMins.x = (mins.x + maxs.x) * 0.5f;
 //       triggerMaxs.x = triggerMins.x + 1;
 //   }
 //   if (triggerMaxs.y - triggerMins.y <= 0.f) {
 //       triggerMins.y = (mins.y + maxs.y) * 0.5f;
 //       triggerMaxs.y = triggerMins.y + 1;
 //   }

 //   // Add points to the generated bounding box for the trigger.
 //   for (IClientGameEntity* teamMember = GetTeamChainEntity(); teamMember != nullptr; teamMember = teamMember->GetTeamChainEntity()) {
	//    // Check it is a derivate of base mover, if not, break out of this loop.
	//    if (!teamMember->IsSubclassOf<CLGBaseMover>()) {
	//        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), teamMember->GetNumber());
	//        break;
	//    }

 //       AddPointToBounds(teamMember->GetAbsoluteMin(), triggerMins, triggerMaxs);
	//    AddPointToBounds(teamMember->GetAbsoluteMax(), triggerMins, triggerMaxs);
 //   }
 //   
 //   // At last, create platform trigger entity.
 //   TriggerAutoPlatform *trigger = TriggerAutoPlatform::Create( this, triggerMins, triggerMaxs );
}

/**
*	@brief	Spawns the invisible 'bottom' touch trigger box.
**/
void FuncPlat::SpawnBottomTouchTrigger() {
	//// Get mins and max.
	//const vec3_t mins = GetMins();
	//const vec3_t maxs = GetMaxs();

	//// We'll need these to calculate the trigger mins/maxs with.
 //   const vec3_t startPosition = GetStartPosition();
 //   const vec3_t endPosition = GetEndPosition();
 //   const float& lip = GetLip();

	//// Start calculation of the new trigger mins/maxs.
 //   vec3_t triggerMins = mins + vec3_t{ 25.0f, 25.0f, 0.f };
 //   vec3_t triggerMaxs = maxs + vec3_t{ -25.f, -25.f, 8.f };

	//gi.DPrintf("%s", "SpawnBottomTouchTrigger BRUH\n");
 //   // Calculate a slightly larger box.
 //   triggerMins.x = mins.x + 25.f;
 //   triggerMins.y = mins.y + 25.f;
 //   triggerMins.z = mins.z;

 //   triggerMaxs.x = maxs.x - 25.f;
 //   triggerMaxs.y = maxs.y - 25.f;
 //   triggerMaxs.z = maxs.z + 8;

 //   // Adjust height.
	//triggerMins.z = triggerMaxs.z - (startPosition.z - endPosition.z) + lip;
	//triggerMaxs.z = triggerMins.z + 8;

 //   // Ensure it is properly scaled on X and Y Axis. 
 //   if (triggerMaxs.x - triggerMins.x <= 0.f) {
 //       triggerMins.x = (mins.x + maxs.x) * 0.5f;
 //       triggerMaxs.x = triggerMins.x + 1;
 //   }
 //   if (triggerMaxs.y - triggerMins.y <= 0.f) {
 //       triggerMins.y = (mins.y + maxs.y) * 0.5f;
 //       triggerMaxs.y = triggerMins.y + 1;
 //   }

 //   // Add points to the generated bounding box for the trigger.
 //   for (IClientGameEntity* teamMember = GetTeamChainEntity(); teamMember != nullptr; teamMember = teamMember->GetTeamChainEntity()) {
	//    // Check it is a derivate of base mover, if not, break out of this loop.
	//    if (!teamMember->IsSubclassOf<CLGBaseMover>()) {
	//        gi.DPrintf("Warning: In function %s entity #%i has a non basemover enitity in its teamchain(#%i)\n", __func__, GetNumber(), teamMember->GetNumber());
	//        break;
	//    }

 //       AddPointToBounds(teamMember->GetAbsoluteMin(), triggerMins, triggerMaxs);
	//    AddPointToBounds(teamMember->GetAbsoluteMax(), triggerMins, triggerMaxs);
 //   }
 //   
 //   // At last, create platform trigger entity.
 //   TriggerAutoPlatform *trigger = TriggerAutoPlatform::Create( this, triggerMins, triggerMaxs );
}

//===============
// Light::SpawnKey
//===============
void FuncPlat::SpawnKey(const std::string& key, const std::string& value) {
    // Height value.
    if (key == "height") {
        // Parsed int.
        float parsedFloat = 0.f;

        // Parse.
        ParseKeyValue(key, value, parsedFloat);

        // Assign.
        SetHeight(parsedFloat);
    }
    // Parent class spawnkey.
    else {
        Base::SpawnKey(key, value);
    }
}