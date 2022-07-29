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
void TargetExplosion::ExplosionUse( IServerGameEntity* other, IServerGameEntity* activator ) {
	SetActivator(activator);

	if ( GetDelayTime() == Frametime::zero() ) {
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
	gi.MSG_WriteUint8(ServerGameCommand::TempEntityEvent);//WriteByte( ServerGameCommand::TempEntityEvent );
	gi.MSG_WriteUint8(TempEntityEvent::Explosion1);//WriteByte( TempEntityEvent::Explosion1 );
	gi.MSG_WriteUint16(GetNumber());//gi.MSG_WriteVector3( GetOrigin(), false );
	gi.Multicast( GetOrigin(), Multicast::PHS );

	GetGameMode()->InflictRadiusDamage( this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

	const Frametime save = GetDelayTime();
	SetDelayTime( GameTime::zero() );
	UseTargets();
	SetDelayTime( save );
}
