/*
// LICENSE HERE.

// TargetSpeaker.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

#include "TargetSpeaker.h"

//===============
// TargetSpeaker::ctor
//===============
TargetSpeaker::TargetSpeaker( Entity* entity )
	: Base( entity ) {
}

//===============
// TargetSpeaker::Spawn
//===============
void TargetSpeaker::Spawn() {
    if ( soundFile.empty() ) {
        gi.DPrintf( "target_speaker with no sound set at %s\n", vec3_to_cstr(GetOrigin()) );
        return;
    }

    if ( soundFile.find( ".wav" ) == std::string::npos ) {
        soundFile += ".wav";
    }

    SetNoiseIndexA( gi.PrecacheSound( soundFile.c_str() ) );

    if ( attenuation == -1.0f ) {
        attenuation = 0.0f;
    }

    // Check for prestarted looping sound
    if ( GetSpawnFlags() & SF_LoopedOn ) {
        SetSound( GetNoiseIndexA() );
    }

    SetUseCallback( &TargetSpeaker::SpeakerUse );

    // Must link the entity so we get areas and clusters so
    // the server can determine who to send updates to
    LinkEntity();
}

//===============
// TargetSpeaker::SpawnKey
//===============
void TargetSpeaker::SpawnKey( const std::string& key, const std::string& value ) {
    if ( key == "noise" ) {
        soundFile = value;
    } else if ( key == "volume" ) {
        volume = std::stof( value );
    } else if ( key == "attenuation" ) {
        attenuation = std::stof( value );
    } else {
        return Base::SpawnKey( key, value );
    }
}

//===============
// TargetSpeaker::SpeakerUse
//===============
void TargetSpeaker::SpeakerUse( IServerGameEntity* other, IServerGameEntity* activator ) {
    int channel = SoundChannel::Voice;

    if ( GetSpawnFlags() & (SF_LoopedOn | SF_LoopedOn) ) {
        // Looping sound toggles
        SetSound( GetSound() ? 0 : GetNoiseIndexA() );
    } else {
        // Normal sound
        if ( GetSpawnFlags() & SF_Reliable ) {
            channel |= SoundChannel::Reliable;
        }
        // Use a positioned_sound, because this entity won't normally be
        // sent to any clients because it is invisible
        gi.PositionedSound( GetOrigin(), GetPODEntity(), channel, GetNoiseIndexA(), volume, attenuation, 0.0f);
    }
}
