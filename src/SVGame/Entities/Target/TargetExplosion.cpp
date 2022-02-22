/*
// LICENSE HERE.

// TargetExplosion.cpp
*/

#include "../../ServerGameLocal.h"
#include "../../Effects.h"
#include "../../Entities.h"
#include "../../Utilities.h"

#include "../Base/SVGBaseEntity.h"

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
	gi.WriteByte( ServerGameCommands::TempEntity );
	gi.WriteByte( TempEntityEvent::Explosion1 );
	gi.WriteVector3( GetOrigin() );
	gi.Multicast( GetOrigin(), Multicast::PHS );

	SVG_InflictRadiusDamage( this, GetActivator(), GetDamage(), nullptr, GetDamage() + 40.0f, MeansOfDeath::Explosive);

	float save = GetDelayTime();
	SetDelayTime( 0.0f );
	UseTargets();
	SetDelayTime( save );
}
