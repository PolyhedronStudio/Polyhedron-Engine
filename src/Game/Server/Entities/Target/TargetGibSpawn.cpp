/***
*
*	License here.
*
*	@file
*
*	Target Gib Spawn Entity.
* 
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

//! Inherited Base classes.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "Game/Server/Entities/Misc/MiscSphereBall.h"
//! Gamemode Interface.
#include "../../Gamemodes/IGamemode.h"

//! GameWorld.
#include "../../World/ServerGameWorld.h"

//! Target Gib Spawn
#include "TargetGibSpawn.h"



/**
*	Constructor.
**/
TargetGibSpawn::TargetGibSpawn( Entity* entity ) : Base( entity ) {
}

/**
*   @brief  
**/
void TargetGibSpawn::Spawn() {
    // Spawn base class.
    Base::Spawn();
	SetServerFlags( EntityServerFlags::NoClient );
    SetUseCallback( &TargetGibSpawn::Callback_Use );
	LinkEntity();
}

/**
*   @brief  
**/
void TargetGibSpawn::PostSpawn() {
	Base::PostSpawn();

}

/**
*	@brief	Additional spawnkeys.
**/
void TargetGibSpawn::SpawnKey(const std::string &key, const std::string &value) {
	if ( key == "count" ) {
		// Parse damage.
		int32_t parsedCount = 0;
		ParseKeyValue( key, value, parsedCount );

		// Set Damage.
		SetCount( parsedCount );
	} else {
		Base::SpawnKey( key, value );
	}
}

/**
*	Callbacks.
**/
void TargetGibSpawn::Callback_Use( IServerGameEntity* other, IServerGameEntity* activator ) {
    // First we make sure to find our targetted entity. (It might be gone).
	GameEntity *targettedEntity = nullptr;
    for (auto& targetEntity : GetGameWorld()->GetGameEntityRange<0, MAX_WIRED_POD_ENTITIES>() | 
        cef::IsValidPointer | cef::HasServerEntity | cef::InUse | cef::HasKeyValue("targetname", targetStr ) ) {
	    targettedEntity = targetEntity;
    }

	if (targettedEntity) {
	// Original code.
	// Create a gib entity.
	
	//	ServerGameWorld *gameWorld = GetGameWorld();
	//	gameWorld->ThrowGib( targettedEntity, "models/objects/gibs/sm_meat/tris.md2", GetCount(), GetDamage(), GibType::Organic);
	
		
	// Temp for testing spheres.
    MiscSphereBall *gibEntity = GetGameWorld()->CreateGameEntity< MiscSphereBall >( nullptr );
	gibEntity->Spawn();
	gibEntity->SetOrigin( other->GetOrigin() + gibEntity->GetSize() );
	gibEntity->LinkEntity();
    // Generate angular velocity.
    gibEntity->SetAngularVelocity({
		10.f + (Randomf() * 90.f), 
		10.f + (Randomf() * 90.f), 
		10.f + (Randomf() * 11.f)
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f)
	});
    // Generate angular velocity.
    gibEntity->SetVelocity({
		10.f + (Randomf() * 90.f), 
		10.f + (Randomf() * 90.f), 
		10.f + (Randomf() * 100.f)
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f), 
		//50.f + (Randomf() * 150.f)
	});
		//gi.MSG_WriteUint8( ServerGameCommand::TempEntityEvent );//WriteByte( ServerGameCommand::TempEntityEvent );
		//gi.MSG_WriteUint8( TempEntityEvent::BodyGib );
		//gi.MSG_WriteUint16( targettedEntity->GetNumber() );
		//gi.MSG_WriteUint8( GetCount() );
		//i.DPrintf( "%s count: %i damage %i\n", "Spawning dem gibs still fails butz I foundz tzhe entity target", GetCount(), GetDamage() );
		//gi.MSG_WriteVector3( GetOrigin(), false );//WriteVector3( GetOrigin() );
		//gi.Multicast( GetOrigin(), Multicast::PVS );
	}
}