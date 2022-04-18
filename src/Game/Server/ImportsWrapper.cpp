/*
// LICENSE HERE.

//
// ImportsWrapper.cpp
//
// Any import function that desires the use of SVGBaseEntity instead of
// a specific ServerEntity, resides in here.
//
// This is to wrap it up nicely.
//
*/
#include "ServerGameLocals.h"          // Include SVGame header.

// Entities.
#include "Entities.h"
#include "Entities/Base/SVGBasePlayer.h"

// Gameworld.
#include "World/ServerGameworld.h"

// Wraps up gi.CPrintf for SVGBaseEntities.
void SVG_CPrint(IServerGameEntity* ent, int32_t printlevel, const std::string& str) {
    if (!ent)
    	return;

    gi.CPrintf(ent->GetPODEntity(), printlevel, "%s", str.c_str());
}

// Wraps up gi.DPrintf
void SVG_DPrint(const std::string& str) {
    gi.DPrintf("%s", str.c_str());
}

//
//===============
// SVG_CenterPrint
//
// Wraps up gi.CenterPrintf for SVGBaseEntity, and nice std::string hurray.
//===============
//
void SVG_CenterPrint(IServerGameEntity* ent, const std::string& str) {
    if (!ent)
        return;

    gi.CenterPrintf(ent->GetPODEntity(), "%s", str.c_str());
}

//
//===============
// SVG_Sound
//
// Wraps up gi.Sound for SVGBaseEntity.
//===============
//
void SVG_Sound(IServerGameEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset) {
    if (!ent)
        return;

    gi.Sound(ent->GetPODEntity(), channel, soundIndex, volume, attenuation, timeOffset);
}


//
//===============
// SVG_BoxEntities
//
// Returns an std::vector containing the found boxed entities. Will not exceed listCount.
//===============
//
std::vector<IServerGameEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType) {
    // Boxed server entities set by gi.BoxEntities.
    Entity* boxedServerEntities[MAX_EDICTS];

    // Vector of the boxed class entities to return.
    GameEntityVector boxedClassEntities;

    // Acquire pointer to the class entities array.
    GameEntityVector gameEntities = game.world->GetGameEntities();

    // Ensure the listCount can't exceed the max edicts.
    if (listCount > MAX_EDICTS) {
        listCount = MAX_EDICTS;
    }

    // Box the entities.
    int32_t numEntities = gi.BoxEntities(mins, maxs, boxedServerEntities, listCount, areaType);

    // Go through the boxed entities list, and store there classEntities (SVGBaseEntity aka baseEntities).
    for (int32_t i = 0; i < numEntities; i++) {
        if (gameEntities[boxedServerEntities[i]->currentState.number] != nullptr) {
            boxedClassEntities.push_back(gameEntities[boxedServerEntities[i]->currentState.number]);
        }
    }

    // Return our boxed base entities vector.
    return boxedClassEntities;
}

////
////===============
//// SVG_Trace
////
//// The defacto trace function to use, for SVGBaseEntity and its derived family & friends.
////===============
////
//SVGTraceResult SVG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, IServerGameEntity* passent, const int32_t& contentMask) {
//    // Acquire server and game entity array pointers.
//    Entity* serverEntities = game.world->GetPODEntities();
//    GameEntityVector gameEntities = game.world->GetGameEntities();
//
//    // Fetch server entity in case one was passed to us.
//    Entity* serverPassEntity = (passent ? passent->GetPODEntity() : NULL);
//
//    // Execute server trace.
//    TraceResult trace = gi.Trace(start, mins, maxs, end, serverPassEntity, contentMask);
//
//    // Convert results to Server Game Trace.
//    SVGTraceResult svgTrace;
//    svgTrace.allSolid = trace.allSolid;
//    svgTrace.contents = trace.contents;
//    svgTrace.endPosition = trace.endPosition;
//    svgTrace.fraction = trace.fraction;
//    svgTrace.offsets[0] = trace.offsets[0];
//    svgTrace.offsets[1] = trace.offsets[1];
//    svgTrace.offsets[2] = trace.offsets[2];
//    svgTrace.offsets[3] = trace.offsets[3];
//    svgTrace.offsets[4] = trace.offsets[4];
//    svgTrace.offsets[5] = trace.offsets[5];
//    svgTrace.offsets[6] = trace.offsets[6];
//    svgTrace.offsets[7] = trace.offsets[7];
//    svgTrace.plane = trace.plane;
//    svgTrace.startSolid = trace.startSolid;
//    svgTrace.surface = trace.surface;
//
//    // Special.
//    if (trace.ent) {
//        uint32_t index = trace.ent->currentState.number;
//
//        if (gameEntities[index] != NULL) {
//            svgTrace.ent = gameEntities[index];
//        } else {
//	        svgTrace.ent = gameEntities[0];
//        }
//    } else {
//        svgTrace.ent = gameEntities[0];
//    }
//
//    return svgTrace;
//}

//
//===============
// SVG_SetConfigString
//
// Sets the config string at the given index number.
//===============
//
void SVG_SetConfigString(const int32_t& configStringIndex, const std::string& configString) {
    gi.configstring(configStringIndex, configString.c_str());
}

//
//===============
// SVG_PrecacheModel
//
// Precaches the model and returns the model index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheModel(const std::string& filename) {
    return gi.ModelIndex(filename.c_str());
}

//
//===============
// SVG_PrecacheImage
//
// Precaches the image and returns the image index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheImage(const std::string& filename) {
    return gi.ImageIndex(filename.c_str());
}

//
//===============
// SVG_PrecacheSound
//
// Precaches the sound and returns the sound index qhandle_t.
//===============
//
qhandle_t SVG_PrecacheSound(const std::string& filename) {
    return gi.SoundIndex(filename.c_str());
}