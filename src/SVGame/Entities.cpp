/*
// LICENSE HERE.

//
// MiscExplosionBox.cpp
//
//
*/
#include "ServerGameLocal.h"    // Include SVGame header.
#include "Entities.h"			// Entities header.
#include "Player/Client.h"		// Include Player Client header.



//
// SVG_SpawnClassEntity
//
//
#include "Entities/Base/SVGBaseTrigger.h"
#include "Entities/Base/SVGBaseMover.h"
#include "Entities/Base/SVGBasePlayer.h"
#include "Entities/Info/InfoPlayerStart.h"
#include "Entities/Worldspawn.h"

//=====================
// SVG_CreateTargetChangeLevel
//
// Returns the created target changelevel entity.
//=====================
Entity* SVG_CreateTargetChangeLevel(char* map) {
    Entity* ent;

    //ent = SVG_Spawn();
//    ent->classname = (char*)"target_changelevel"; // C++20: Added a cast.
    Q_snprintf(level.nextMap, sizeof(level.nextMap), "%s", map);
//    ent->map = level.nextMap;
    return ent;
}