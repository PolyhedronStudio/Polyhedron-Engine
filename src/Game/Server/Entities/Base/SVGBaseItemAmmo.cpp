/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
#include "../../ServerGameLocals.h"  // SVGame.
#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.
#include "../../Physics/StepMove.h" // Stepmove funcs.

// Server Game Base Entity.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseItemAmmo.h"


//! Constructor/Destructor.
SVGBaseItemAmmo::SVGBaseItemAmmo(PODEntity *svEntity, const std::string& displayString, uint32_t identifier) 
    : Base(svEntity, displayString, identifier) {

}
SVGBaseItemAmmo::~SVGBaseItemAmmo() {

}



/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseItemAmmo::Precache() {
    Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseItemAmmo::Spawn() {
    Base::Spawn();
}


/***
* 
*   Entity functions.
*
***/
