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

SVGBaseEntity* SVG_FindEntitiesWithinRadius(SVGBaseEntity* from, vec3_t org, float rad, uint32_t excludeSolidFlags = Solid::Not);


//
// Server Entity handling.
//
void    SVG_InitEntity(Entity* e);
void    SVG_FreeEntity(Entity* e);

Entity* SVG_GetWorldServerEntity();
Entity* SVG_Spawn(void);


//
// ClassEntity handling.
//
SVGBaseEntity* SVG_GetWorldClassEntity();

SVGBaseEntity* SVG_SpawnClassEntity(Entity* ent, const std::string& className);
void SVG_FreeClassEntity(Entity* ent);

#endif // __SVGAME_ENTITIES_H__