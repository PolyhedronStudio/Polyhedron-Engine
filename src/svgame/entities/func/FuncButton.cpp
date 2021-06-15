/*
// LICENSE HERE.

// FuncButton.cpp
*/

#include "../../g_local.h"          // SVGame.
#include "../../effects.h"          // Effects.
#include "../../utils.h"            // Util funcs.
#include "../../physics/stepmove.h" // Stepmove funcs.

#include "../base/SVGBaseEntity.h"
#include "../base/SVGBaseTrigger.h"

#include "FuncButton.h"

FuncButton::FuncButton( Entity* svEntity )
	: SVGBaseEntity( svEntity )
{

}

void FuncButton::Precache()
{
}

void FuncButton::Spawn()
{
}

void FuncButton::SpawnKey( const std::string& key, const std::string& value )
{
	// Admer: I hate this way of parsing keyvalues
	// SPAWNARGS IS BETTER MIKEYYYYYY!!!
	if ( key == "speed" )
	{
		ParseFloatKeyValue( key, value, GetServerEntity()->speed );
	}

	return SVGBaseEntity::SpawnKey( key, value );
}

void FuncButton::ButtonDone()
{
}

void FuncButton::ButtonReturn()
{
}

void FuncButton::ButtonWait()
{
}

void FuncButton::ButtonFire()
{
}

void FuncButton::ButtonUse( SVGBaseEntity* other, SVGBaseEntity* activator )
{
}

void FuncButton::ButtonTouch( SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf )
{
}

void FuncButton::ButtonDie( SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point )
{
}


