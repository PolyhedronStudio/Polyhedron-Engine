/*
// LICENSE HERE.

// TriggerCounter.cpp
*/

// Core.
#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

// Entities.
#include "../Base/SVGBaseTrigger.h"

#include "TriggerCounter.h"

//===============
// TriggerCounter::ctor
//===============
TriggerCounter::TriggerCounter( Entity* entity )
	: Base( entity ) {

}

//===============
// TriggerCounter::Spawn
//===============
void TriggerCounter::Spawn()
{
	SetWaitTime( -1.0s );
	SetUseCallback( &TriggerCounter::CounterUse );
}

//===============
// TriggerCounter::SpawnKey
//===============
void TriggerCounter::SpawnKey( const std::string& key, const std::string& value )
{
	if ( key == "count" ) {
		count = std::stoi( value );
	}
	else {
		return Base::SpawnKey( key, value );
	}
}

//===============
// TriggerCounter::CounterUse
//===============
void TriggerCounter::CounterUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	if ( !count ) {
		return;
	}

	count--;

	if ( count ) {
		if ( !(GetSpawnFlags() & SF_NoMessage) ) {
			gi.CenterPrintf( activator->GetPODEntity(), "%u more to go...", count );
			gi.Sound( activator->GetPODEntity(), SoundChannel::Auto, gi.SoundIndex( "misc/talk1.wav" ), 1.0f, Attenuation::Normal, 0.0f );
		}
		return;
	}

	if ( !(GetSpawnFlags() & SF_NoMessage) ) {
		gi.CenterPrintf( activator->GetPODEntity(), "Sequence completed!" );
		gi.Sound( activator->GetPODEntity(), SoundChannel::Auto, gi.SoundIndex( "misc/talk1.wav" ), 1.0f, Attenuation::Normal, 0.0f );
	}

	SetActivator(activator);

	// Mike made a funny decision to put using targets *only* into SVGBaseTrigger
	// which doesn't really make sense when you think about it, so now we have
	// SVGBaseEntity::UseTargets and SVGBaseTrigger::UseTargets
	SVGBaseEntity::UseTargets();
}
