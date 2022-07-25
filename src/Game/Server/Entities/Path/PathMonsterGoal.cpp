/*
// LICENSE HERE.

// PathMonsterGoal.cpp
*/

#include "../../ServerGameLocals.h"
#include "../../Entities.h"

// Base Entities.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseSkeletalAnimator.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBasePlayer.h"

// Base Slide Monster.
#include "../Base/SVGBaseRootMotionMonster.h"
#include "../../World/ServerGameWorld.h"
#include "PathMonsterGoal.h"

//===============
// PathMonsterGoal::ctor
//===============
PathMonsterGoal::PathMonsterGoal( Entity* entity )
	: Base( entity ) {

}

//===============
// PathMonsterGoal::Spawn
//===============
void PathMonsterGoal::Spawn() {
	static const vec3_t BboxSize = vec3_t { 8.0f, 8.0f, 8.0f };

    Base::Spawn();

    if ( GetTargetName().empty()) {
        gi.DPrintf( "path_monster_goal with no targetname at %s\n", vec3_to_cstr( GetOrigin() ) );
        return Remove();
    }

	// Basic trig setup.
    SetSolid( Solid::Trigger );
    SetMaxs( BboxSize );
    SetMins( vec3_negate( BboxSize ) );
    SetServerFlags(GetServerFlags() | EntityServerFlags::NoClient);
    LinkEntity();

	//self->noiseIndexA = gi.PrecacheSound("world/electro.wav");
	SetTouchCallback(&PathMonsterGoal::PathMonsterGoalTouch);

	// In case the entity can be "used", set it to hurt those who use it as well.
	SetUseCallback(&PathMonsterGoal::PathMonsterGoalUse);
}

//===============
// PathMonsterGoal::SpawnKey
//===============
void PathMonsterGoal::SpawnKey( const std::string& key, const std::string& value ) {
	if ( key == "monstergoaltarget" ) {
        strNextGoal = value;
    } else if ( key == "monsters" ) {
        strMonsters = value;
    } else {
        return Base::SpawnKey( key, value );
    }
}

//===============
// PathMonsterGoal::OnReachedCorner
//===============
void PathMonsterGoal::PathMonsterGoalTouch(IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf) {
	// First determine whether the entity is a legitimate monster.
	if (!other) {
		SetActivator( nullptr );
		return;
	}
	if ( other->IsSubclassOf<SVGBaseRootMotionMonster>()) {
		// Find the actual entity we want to move next to.
		ServerGameWorld *gw = GetGameWorld();
		for (auto* geGoal : GetGameWorld()->GetGameEntityRange(0, MAX_WIRED_POD_ENTITIES)
			| cef::IsValidPointer
			| cef::HasServerEntity
			| cef::HasKeyValue("targetname", strNextGoal)) {
				// Cast monster.
				SVGBaseRootMotionMonster *castOther = dynamic_cast<SVGBaseRootMotionMonster*>( other );
				if ( castOther ) { 
					castOther->SetGoalEntity(geGoal);
					castOther->SetEnemy(geGoal);	

						
					// Yeah.
					SetActivator( other );
				}

				//gi.DPrintf("Set Goal Entity for StepDummy: %s\n", geGoal->GetTargetName().c_str());
		}
	} else {
	
		SetActivator( nullptr );
	}
}
//void PathMonsterGoalEnable(IServerGameEntity* other, IServerGameEntity* activator);
void PathMonsterGoal::PathMonsterGoalUse(IServerGameEntity* other, IServerGameEntity* activator) {

}