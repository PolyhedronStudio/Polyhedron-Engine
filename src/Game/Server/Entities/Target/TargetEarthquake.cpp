/*
// LICENSE HERE.

// TargetEarthquake.cpp
*/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

// World.
#include "../../World/ServerGameWorld.h"

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

    SetNoiseIndexA( gi.PrecacheSound( "world/quake.wav" ) );
}

//===============
// TargetEarthquake::SpawnKey
//===============
void TargetEarthquake::SpawnKey( const std::string& key, const std::string& value ) {
    if ( key == "count" ) {
        ParseKeyValue(key, value, duration);
    } else if ( key == "speed" ) {
        severity = std::stof( value );
    } else {
        return Base::SpawnKey( key, value );
    }
}

//===============
// TargetEarthquake::QuakeUse
//===============
void TargetEarthquake::QuakeUse( IServerGameEntity* other, IServerGameEntity* activator ) {
    SetActivator(activator);
    
    lastQuakeTime = GameTime::zero();
    timeStamp = duration_cast<GameTime>(level.time + duration);
    SetNextThinkTime( level.time + FRAMETIME_S );
}

//===============
// TargetEarthquake::QuakeThink
//===============
void TargetEarthquake::QuakeThink() {
    if ( lastQuakeTime < level.time ) {
        gi.PositionedSound( GetOrigin(), GetPODEntity(), SoundChannel::Auto, GetNoiseIndexA(), 1.0f, Attenuation::None, 0.0f);
        lastQuakeTime = duration_cast<GameTime>(level.time + 0.5s);
    }

    for (auto& entity : GetGameWorld()->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES)
         | cef::Standard | cef::HasClient | cef::HasGroundEntity ) 
    {
        vec3_t newVelocity {
            crandom() * 150.0f,
            crandom() * 150.0f,
            severity * (100.0f / entity->GetMass())
        };
        newVelocity += entity->GetVelocity();
        entity->SetGroundEntity( SGEntityHandle( nullptr, -1 ) );
        entity->SetVelocity( newVelocity );
    }

    if ( level.time < timeStamp ) {
        SetNextThinkTime( level.time + 1 * FRAMETIME_S );
    }
}
