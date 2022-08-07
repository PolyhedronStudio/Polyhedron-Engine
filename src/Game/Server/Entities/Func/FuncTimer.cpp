/*
// LICENSE HERE.

// FuncTimer.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"
#include "../../Physics/StepMove.h"

#include "../Base/SVGBaseEntity.h"

#include "FuncTimer.h"

//===============
// FuncTimer::ctor
//===============
FuncTimer::FuncTimer( Entity* entity ) 
	: Base( entity ) {

}

//===============
// FuncTimer::Spawn
//===============
void FuncTimer::Spawn() {
	Base::Spawn();

	if ( waitTime == Frametime::zero() ) {
		waitTime = 1s;
	}

	SetUseCallback( &FuncTimer::TimerUse );
	SetThinkCallback( &FuncTimer::TimerThink );

	if ( randomTime >= waitTime ) {
		randomTime = waitTime - FRAMETIME_S;
		gi.DPrintf( "func_timer at %s has random >= wait\n", vec3_to_cstr( GetOrigin() ) );
	}

	if ( spawnFlags & SF_StartOn ) {
		nextThinkTime = duration_cast<GameTime>(level.time + 1s + pauseTime + delayTime + waitTime + crandom() * randomTime);
		SetActivator(this);
	}

	SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);
}

//===============
// FuncTimer::SpawnKey
//===============
void FuncTimer::SpawnKey( const std::string& key, const std::string& value )
{
	if (key == "pausetime") {
		ParseKeyValue( key, value, pauseTime);
	} else if ( key == "random" ) {
		ParseKeyValue( key, value, randomTime);
	} else if ( key == "wait" ) {
		ParseKeyValue( key, value, waitTime);
	} else {
		Base::SpawnKey( key, value );
	}
}

//===============
// FuncTimer::TimerThink
//===============
void FuncTimer::TimerThink() {
	UseTargets();
	SetNextThinkTime( duration_cast<GameTime>(level.time + waitTime + crandom() * randomTime) );
}

//===============
// FuncTimer::TimerUse
//===============
void FuncTimer::TimerUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	SetActivator(activator);

	// If on, turn it off
	if ( nextThinkTime != GameTime::zero() ) {
		SetNextThinkTime( GameTime::zero() );
		return;
	}

	// Turn it on
	if ( delayTime != Frametime::zero() ) {
		SetNextThinkTime( level.time + delayTime );
	} else {
		TimerThink();
	}
}
