/*
// LICENSE HERE.

// TargetSpeaker.cpp
*/

#include "../../g_local.h"
#include "../../effects.h"
#include "../../entities.h"
#include "../../utils.h"

#include "../base/SVGBaseEntity.h"

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

    SetNoiseIndex( gi.SoundIndex( soundFile.c_str() ) );

    if ( attenuation == -1.0f ) {
        attenuation = 0.0f;
    }

    // Check for prestarted looping sound
    if ( GetSpawnFlags() & SF_LoopedOn ) {
        SetSound( GetNoiseIndex() );
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
void TargetSpeaker::SpeakerUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
    int channel = CHAN_VOICE;

    if ( GetSpawnFlags() & (SF_LoopedOn | SF_LoopedOn) ) {
        // Looping sound toggles
        SetSound( GetSound() ? 0 : GetNoiseIndex() );
    } else {
        // Normal sound
        if ( GetSpawnFlags() & SF_Reliable ) {
            channel |= CHAN_RELIABLE;
        }
        // Use a positioned_sound, because this entity won't normally be
        // sent to any clients because it is invisible
        gi.PositionedSound( GetOrigin(), serverEntity, channel, GetNoiseIndex(), volume, attenuation, 0.0f );
    }
}
