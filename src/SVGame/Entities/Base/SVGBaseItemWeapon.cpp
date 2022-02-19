/*
// LICENSE HERE.

//
// SVGBaseItemWeapon.cpp
//
// Base class to create item entities from.
//
// Gives the following functionalities:
// TODO: Explain what.
//
*/
#include "../../ServerGameLocal.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "../Base/SVGBaseEntity.h"
#include "../Base/SVGBaseTrigger.h"
#include "../Base/SVGBaseItem.h"
#include "../Base/SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseItemWeapon.h"


//
// Constructor/Deconstructor.
//
SVGBaseItemWeapon::SVGBaseItemWeapon(Entity* svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}
SVGBaseItemWeapon::~SVGBaseItemWeapon() {

}



//
// Interface functions. 
//
//
//===============
// SVGBaseItemWeapon::Precache
//
//===============
//
void SVGBaseItemWeapon::Precache() {
    // Always call parent class method.
    Base::Precache();

    // Precache view and world models for the given weapon.
    SVG_PrecacheModel(GetViewModel());
    SVG_PrecacheModel(GetWorldModel());
}

//
//===============
// SVGBaseItemWeapon::Spawn
//
//===============
//
void SVGBaseItemWeapon::Spawn() {
    // Always call parent class method.
    Base::Spawn();
    
    // Set the weapon item world model.
    SetModel(GetWorldModel());

    // Set the config string for this item.
    SVG_SetConfigString(ConfigStrings::Items + itemIdentifier, displayString);
}

//
//===============
// SVGBaseItemWeapon::Respawn
//===============
//
void SVGBaseItemWeapon::Respawn() {
    Base::Respawn();
}

//
//===============
// SVGBaseItemWeapon::PostSpawn
//===============
//
void SVGBaseItemWeapon::PostSpawn() {
    // Always call parent class method.
    Base::PostSpawn();
}

//
//===============
// SVGBaseItemWeapon::Think
//===============
//
void SVGBaseItemWeapon::Think() {
    // Always call parent class method.
    Base::Think();
}




//
// Entity functions.
//
//===============
// SVGBaseItemWeapon::SetRespawn
//===============
void SVGBaseItemWeapon::SetRespawn(const float delay) {
    Base::SetRespawn(delay);
}


//
// Weapon activity engagement functions.
//
void SVGBaseItemWeapon::WeaponThink(SVGBasePlayer* player) {

}

//void SVGBaseItemWeapon::BaseItemWeaponUse(SVGBaseEntity* caller, SVGBaseEntity* activator) { 
//    gi.DPrintf("USED UR WEAPON LULZ\n");
//}