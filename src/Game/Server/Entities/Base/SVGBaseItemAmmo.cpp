/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
//! Main Headers.
#include "Game/Server/ServerGameMain.h"
//! Server Game Local headers.
#include "Game/Server/ServerGameLocals.h"

#include "../../Effects.h"          // Effects.
#include "../../Utilities.h"        // Util funcs.

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
