/***
*
*	License here.
*
*	@file
*
*	See header for information.
*
***/
#include "../../ServerGameLocal.h"   // SVGame.
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
void SVGBaseMonster::Precache() { Base::Precache(); }

/**
*   @brief 
**/
void SVGBaseMonster::Spawn() { Base::Spawn(); }


/***
* 
*   Entity functions.
*
***/
