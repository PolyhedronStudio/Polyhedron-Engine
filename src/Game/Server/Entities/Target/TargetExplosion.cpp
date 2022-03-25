/*
// LICENSE HERE.

// TargetExplosion.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

#include "../../Gamemodes/IGamemode.h"

#include "TargetExplosion.h"

//===============
// TargetExplosion::ctor
//===============
TargetExplosion::TargetExplosion( Entity* entity )
	: Base( entity ) {

}

//===============
// TargetExplosion::Spawn
//===============
void TargetExplosion::Spawn() {
	SetUseCallback( &TargetExplosion::ExplosionUse );
	SetServerFlags( EntityServerFlags::NoClient );
}

//===============
// TargetExplosion::ExplosionUse
//===============
void TargetExplosion::ExplosionUse( SVGBaseEntity* other, SVGBaseEntity* activator ) {
	SetActivator(activator);

	if ( !GetDelayTime() ) {
		ExplosionThink();
		return;
	}

	SetThinkCallback( &TargetExplosion::ExplosionThink );
	SetNextThinkTime( level.time + GetDelayTime() );
}

//===============
// TargetExplosion::ExplosionThink
//===============
void TargetExplosion::ExplosionThink() {
	gi.MSG_WriteUint8(ServerGameCommand::TempEntity);//WriteByte( ServerGameCommand::TempEntity );
	gi.MSG_WriteUint8(TempEntityEvent::Explosion1);//WriteByte( TempEntityEvent::Explosion1 );
	gi.MSG_WriteVector3( GetOrigin(), false );
	gi.Multicast( GetOrigin(), Multicast::PHS );

	GetGamemode()->InflictRadiusDamage( this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

	float save = GetDelayTime();
	SetDelayTime( 0.0f );
	UseTargets();
	SetDelayTime( save );
}
