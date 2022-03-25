/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
#include "../../ServerGameLocals.h"   // SVGame.
#include "../../Effects.h"	     // Effects.
#include "../../Utilities.h"	     // Util funcs.
#include "../../Physics/StepMove.h"  // Stepmove funcs.

// Server Game Base Entity.
#include "SVGBaseEntity.h"
#include "SVGBaseTrigger.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseMonster.h"


//! Constructor/Destructor.
SVGBaseMonster::SVGBaseMonster(Entity* svEntity) : Base(svEntity) { }
SVGBaseMonster::~SVGBaseMonster() { }


/***
* 
*   Interface functions.
*
***/
/**
*   @brief 
**/
void SVGBaseMonster::Precache() { 
	Base::Precache();
}

/**
*   @brief 
**/
void SVGBaseMonster::Spawn() { Base::Spawn(); }

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseMonster::Think() { 
	// Base think.
	Base::Think();

	// Now go and process animations.
	IsProcessingState();
}


/***
* 
*   Entity functions.
*
***/
/**
*   @brief Takes care of server side animation processing.
*	@details	The server also does animation processing in order to determine
*				whether an animation has finished or not, this is all part of
*				managing animation states.
**/
void SVGBaseMonster::IsProcessingState() {

}