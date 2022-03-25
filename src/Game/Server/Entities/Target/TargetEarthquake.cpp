/*
// LICENSE HERE.

// TargetEarthquake.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

// World.
#include "../../World/Gameworld.h"

#include "TargetEarthquake.h"

//===============
// TargetEarthquake::ctor
//===============
TargetEarthquake::TargetEarthquake( Entity* entity )
	: Base( entity ) {

}

//===============
// TargetEarthquake::Spawn
//===============
void TargetEarthquake::Spawn() {
    if ( GetTargetName().empty()) {
        gi.DPrintf( "Untargeted target_earthquake at %s\n", vec3_to_cstr( GetOrigin() ) );
    }

    SetServerFlags( EntityServerFlags::NoClient );
    SetThinkCallback( &TargetEarthquake::QuakeThink );
    SetUseCallback( &TargetEarthquake::QuakeUse );

    SetNoiseIndexA( gi.SoundIndex( "world/quake.wav" ) );
}

//===============
// TargetEarthquake::SpawnKey
//===============
void TargetEarthquake::SpawnKey( const std::string& key, const std::string& value ) {
    if ( key == "count" ) {
        duration = std::stof( value );
    } else if ( key == "speed" ) {
        severity = std::stof( value );
    } else {
        return Base::SpawnKey( key, value );
    }
}

//===============
// TargetEarthquake::QuakeUse
//===============
void TargetEarthquake::QuakeUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
    SetActivator(activator);
    
    lastQuakeTime = 0.0f;
    timeStamp = level.time + duration;
    SetNextThinkTime( level.time + FRAMETIME );
}

//===============
// TargetEarthquake::QuakeThink
//===============
void TargetEarthquake::QuakeThink() {
    if ( lastQuakeTime < level.time ) {
        gi.PositionedSound( GetOrigin(), GetPODEntity(), SoundChannel::Auto, GetNoiseIndexA(), 1.0f, Attenuation::None, 0.0f);
        lastQuakeTime = level.time + 0.5f;
    }

    for (auto& entity : GetGameworld()->GetClassEntityRange(0, MAX_EDICTS)
         | cef::Standard | cef::HasClient | cef::HasGroundEntity ) 
    {
        vec3_t newVelocity {
            crandom() * 150.0f,
            crandom() * 150.0f,
            severity * (100.0f / entity->GetMass())
        };
        newVelocity += entity->GetVelocity();
        entity->SetGroundEntity( nullptr );
        entity->SetVelocity( newVelocity );
    }

    if ( level.time < timeStamp ) {
        SetNextThinkTime( level.time + 1 * FRAMETIME );
    }
}
