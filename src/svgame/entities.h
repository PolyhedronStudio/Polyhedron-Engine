/*
// LICENSE HERE.

//
// entities.h
//
// All entity related functionality resides here. Need to allocate a class?
// Find an entity? Anything else? You've hit the right spot.
//
// A "ClassEntity", or a CE, is always a member of a "ServerEntity", aka a SE.
//
// The actual game logic implementation thus goes in ClassEntities. An SE is
// merely a binding layer between SVGame and server.
//
*/
#ifndef __SVGAME_ENTITIES_H__
#define __SVGAME_ENTITIES_H__

class SVGBaseEntity;

//
// Entity SEARCH utilities.
//
Entity* SVG_PickTarget(char* targetName);
Entity* SVG_Find(Entity* from, int32_t fieldofs, const char* match); // C++20: Added const to char*

// Find entities within a given radius.
SVGBaseEntity* SVG_FindEntitiesWithinRadius(SVGBaseEntity* from, vec3_t org, float rad, uint32_t excludeSolidFlags = Solid::Not);
// Find entities based on their field(key), and field(value).
SVGBaseEntity* SVG_FindEntityByKeyValue(const std::string& fieldKey, const std::string& fieldValue, SVGBaseEntity* lastEntity = nullptr);

//
// Server Entity handling.
//
void    SVG_InitEntity(Entity* e);
void    SVG_FreeEntity(Entity* e);

Entity* SVG_GetWorldServerEntity();
Entity* SVG_Spawn(void);

Entity* SVG_CreateTargetChangeLevel(char* map);

// Admer: quick little template function to spawn entities, until we have this code in a local game class :)
template<typename entityClass>
inline entityClass* SVG_CreateEntity(Entity* edict = nullptr, bool allocateNewEdict = true) {
    entityClass* entity = nullptr;
    // If a null entity was passed, create a new one
    if (nullptr == edict) {
        if (allocateNewEdict) {
            edict = SVG_Spawn();
        } else {
            gi.DPrintf("WARNING: tried to spawn a class entity when the edict is null\n");
            return nullptr;
        }
    }
    // Abstract classes will have AllocateInstance as nullptr, hence we gotta check for that
    if (entityClass::ClassInfo.AllocateInstance) {
        entity = static_cast<entityClass*>(entityClass::ClassInfo.AllocateInstance(edict)); // Entities that aren't in the type info system will error out here
        edict->className = entity->GetTypeInfo()->className;
        edict->classEntity = entity;
        if (nullptr == g_baseEntities[edict->state.number]) {
            g_baseEntities[edict->state.number] = entity;
        } else {
            gi.DPrintf("ERROR: edict %i is already taken\n", edict->state.number);
        }
    }
    return entity;
}

//
// ClassEntity handling.
//
SVGBaseEntity* SVG_GetWorldClassEntity();

SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& className);
void SVG_FreeClassEntity(Entity* ent);

#endif // __SVGAME_ENTITIES_H__