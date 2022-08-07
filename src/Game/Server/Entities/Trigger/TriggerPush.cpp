/*
// LICENSE HERE.

// TriggerPush.cpp
*/

//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBasePlayer.h"

#include "TriggerPush.h"

//===============
// TriggerPush::ctor
//===============
TriggerPush::TriggerPush( Entity* entity )
	: Base( entity ) {

}

//===============
// TriggerPush::Spawn
//===============
void TriggerPush::Spawn() {
	if ( !WindSound ) {
		WindSound = gi.PrecacheSound( "misc/windfly.wav" );
	}

	InitBrushTrigger();
	SetTouchCallback( &TriggerPush::PushTouch );
	LinkEntity();
}

//===============
// TriggerPush::SpawnKey
//===============
void TriggerPush::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "speed" ) {
        ParseKeyValue(key, value, pushForce);
	} else if ( key == "movedir" ) {
        ParseKeyValue( key, value, pushDirection );
	} else {
		return Base::SpawnKey( key, value );
	}
}

//===============
// TriggerPush::PushTouch
//===============
void TriggerPush::PushTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf ) {
    vec3_t newVelocity = vec3_scale( pushDirection, pushForce * 10.0f);
    
    /* TODO:
    if ( other == grenade ) {
        increase push velocity 10x
    } else*/ 
    if ( other->GetHealth() > 0 ) {
        other->SetVelocity( newVelocity );

        // Play sounds on clients
        if ( other->IsClass<SVGBasePlayer>() ) {
            SVGBasePlayer* player = static_cast<SVGBasePlayer*>(other);

            // Don't take fall damage immediately from this
            player->GetClient()->oldVelocity = other->GetVelocity();
            if ( player->GetDebounceSoundTime() < level.time ) {
                player->SetDebounceSoundTime( level.time + 1500ms);
                SVG_Sound( player, SoundChannel::Auto, WindSound, 1.0f, Attenuation::Normal, 0.0f );
            }
        }
    }

    if ( GetSpawnFlags() & SF_PushOnce ) {
        Remove();
    }
}
