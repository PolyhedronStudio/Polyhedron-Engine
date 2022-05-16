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
#include "SVGBaseSkeletalAnimator.h"
#include "SVGBaseItem.h"
#include "SVGBasePlayer.h"

// Base Item Weapon.
#include "SVGBaseMonster.h"


//! Constructor/Destructor.
SVGBaseMonster::SVGBaseMonster(PODEntity *svEntity) : Base(svEntity) { }
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
void SVGBaseMonster::Spawn() { 
	Base::Spawn(); 
}



/**
*   @brief 
**/
void SVGBaseMonster::PostSpawn() { 
	Base::Spawn(); 
}

/**
*   @brief 
**/
void SVGBaseMonster::Respawn() { 
	Base::Respawn(); 
}

/**
*   @brief Override think method to add in animation processing.
**/
void SVGBaseMonster::Think() { 
	// Base think.
	Base::Think();
}

/**
*   @brief 
**/
void SVGBaseMonster::SpawnKey(const std::string& key, const std::string& value) { 
	Base::SpawnKey(key, value); 
}


/***
* 
*   Entity functions.
*
***/
