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
#include "g_local.h"          // Include SVGame header.
#include "entities.h"
#include "entities/base/SVGBaseEntity.h"

//
//===============
// SVG_CenterPrint
//
// Wraps up gi.CenterPrintf for SVGBaseEntity, and nice std::string hurray.
//===============
//
void SVG_CenterPrint(SVGBaseEntity* ent, const std::string& str) {
    if (!ent)
        return;

    gi.CenterPrintf(ent->GetServerEntity(), "%s", str.c_str());
}

//
//===============
// SVG_CenterPrint
//
// Wraps up gi.Sound for SVGBaseEntity.
//===============
//
void SVG_Sound(SVGBaseEntity* ent, int32_t channel, int32_t soundIndex, float volume, float attenuation, float timeOffset) {
    if (!ent)
        return;

    gi.Sound(ent->GetServerEntity(), channel, soundIndex, volume, attenuation, timeOffset);
}


//
//===============
// SVG_BoxEntities
//
// Returns an std::vector containing the found boxed entities. Will not exceed listCount.
//===============
//
std::vector<SVGBaseEntity*> SVG_BoxEntities(const vec3_t& mins, const vec3_t& maxs, int32_t listCount, int32_t areaType) {
    Entity* boxedServerEntities[MAX_EDICTS];
    std::vector<SVGBaseEntity*> boxedBaseEntities;

    // Ensure the listCount can't exceed the max edicts.
    if (listCount > MAX_EDICTS) {
        listCount = MAX_EDICTS;
    }

    // Box the entities.
    int32_t numEntities = gi.BoxEntities(mins, maxs, boxedServerEntities, MAX_EDICTS, AREA_SOLID);

    // Go through the boxed entities list, and store there classEntities (SVGBaseEntity aka baseEntities).
    for (int32_t i = 0; i < numEntities; i++) {
        if (g_baseEntities[boxedServerEntities[i]->state.number] != nullptr)
            boxedBaseEntities.push_back(g_baseEntities[boxedServerEntities[i]->state.number]);
    }

    // Return our boxed base entities vector.
    return boxedBaseEntities;
}

//
//===============
// SVG_Trace
//
// The defacto trace function to use, for SVGBaseEntity and its derived family & friends.
//===============
//
SVGTrace SVG_Trace(const vec3_t& start, const vec3_t& mins, const vec3_t& maxs, const vec3_t& end, SVGBaseEntity* passent, const int32_t& contentMask) {
    // Fetch server entity in case one was passed to us.
    Entity* serverPassEntity = (passent ? passent->GetServerEntity() : NULL);

    // Execute server trace.
    trace_t trace = gi.Trace(start, mins, maxs, end, serverPassEntity, contentMask);

    // Convert results to Server Game Trace.
    SVGTrace svgTrace;
    svgTrace.allSolid = trace.allSolid;
    svgTrace.contents = trace.contents;
    svgTrace.endPosition = trace.endPosition;
    svgTrace.fraction = trace.fraction;
    svgTrace.offsets[0] = trace.offsets[0];
    svgTrace.offsets[1] = trace.offsets[1];
    svgTrace.offsets[2] = trace.offsets[2];
    svgTrace.offsets[3] = trace.offsets[3];
    svgTrace.offsets[4] = trace.offsets[4];
    svgTrace.offsets[5] = trace.offsets[5];
    svgTrace.offsets[6] = trace.offsets[6];
    svgTrace.offsets[7] = trace.offsets[7];
    svgTrace.plane = trace.plane;
    svgTrace.startSolid = trace.startSolid;
    svgTrace.surface = trace.surface;

    // Special.
    if (trace.ent) {
        uint32_t index = trace.ent->state.number;

        if (g_baseEntities[index] != NULL) {
            svgTrace.ent = g_baseEntities[index];
        } else {
            svgTrace.ent = g_entities[0].classEntity;
        }
    } else {
        svgTrace.ent = g_entities[0].classEntity;
    }

    return svgTrace;
}

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