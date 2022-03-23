/*
// LICENSE HERE.

// FuncTimer.cpp
*/

#include "../../ServerGameLocals.h"
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

	if ( !waitTime ) {
		waitTime = 1.0f;
	}

	SetUseCallback( &FuncTimer::TimerUse );
	SetThinkCallback( &FuncTimer::TimerThink );

	if ( randomTime >= waitTime ) {
		randomTime = waitTime - FRAMETIME;
		gi.DPrintf( "func_timer at %s has random >= wait\n", vec3_to_cstr( GetOrigin() ) );
	}

	if ( spawnFlags & SF_StartOn ) {
		nextThinkTime = level.time + 1.0f + pauseTime + delayTime + waitTime + crandom() * randomTime;
		SetActivator(this);
	}

	serverEntity->serverFlags = EntityServerFlags::NoClient;
}

//===============
// FuncTimer::SpawnKey
//===============
void FuncTimer::SpawnKey( const std::string& key, const std::string& value )
{
	if (key == "pausetime") {
		ParseFloatKeyValue( key, value, pauseTime );
	} else if ( key == "random" ) {
		ParseFloatKeyValue( key, value, randomTime );
	} else if ( key == "wait" ) {
		ParseFloatKeyValue( key, value, waitTime );
	} else {
		Base::SpawnKey( key, value );
	}
}

//===============
// FuncTimer::TimerThink
//===============
void FuncTimer::TimerThink() {
	UseTargets();
	SetNextThinkTime( level.time + waitTime + crandom() * randomTime );
}

//===============
// FuncTimer::TimerUse
//===============
void FuncTimer::TimerUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	SetActivator(activator);

	// If on, turn it off
	if ( nextThinkTime ) {
		SetNextThinkTime( 0.0f );
		return;
	}

	// Turn it on
	if ( delayTime ) {
		SetNextThinkTime( level.time + delayTime );
	} else {
		TimerThink();
	}
}
