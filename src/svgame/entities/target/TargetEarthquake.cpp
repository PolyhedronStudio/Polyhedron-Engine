/*
// LICENSE HERE.

// TargetEarthquake.cpp
*/

#include "../../g_local.h"
#include "../../effects.h"
#include "../../entities.h"
#include "../../utils.h"

#include "../base/SVGBaseEntity.h"

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
    if ( targetNameStr.empty() ) {
        gi.DPrintf( "Untargeted target_earthquake at %s\n", vec3_to_cstr( GetOrigin() ) );
    }

    SetServerFlags( EntityServerFlags::NoClient );
    SetThinkCallback( &TargetEarthquake::QuakeThink );
    SetUseCallback( &TargetEarthquake::QuakeUse );

    SetNoiseIndex( gi.SoundIndex( "world/quake.wav" ) );
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
    this->activator = activator;
    
    lastQuakeTime = 0.0f;
    timeStamp = level.time + duration;
    SetNextThinkTime( level.time + FRAMETIME );
}

//===============
// TargetEarthquake::QuakeThink
//===============
void TargetEarthquake::QuakeThink() {
    if ( lastQuakeTime < level.time ) {
        gi.PositionedSound( GetOrigin(), serverEntity, CHAN_AUTO, GetNoiseIndex(), 1.0f, ATTN_NONE, 0.0f );
        lastQuakeTime = level.time + 0.5f;
    }

    for ( auto * entity : g_baseEntities
         | bef::Standard | bef::HasClient | bef::HasGroundEntity ) 
    {
        if ( nullptr == entity->GetGroundEntity() ) {
            continue;
        }

        entity->SetGroundEntity( nullptr );
        vec3_t newVelocity{
            crandom() * 150.0f,
            crandom() * 150.0f,
            severity * (100.0f / entity->GetMass())
        };

        newVelocity += entity->GetVelocity();
        entity->SetVelocity( newVelocity );
    }

    if ( level.time < timeStamp ) {
        SetNextThinkTime( level.time + 1 * FRAMETIME );
    }
}
