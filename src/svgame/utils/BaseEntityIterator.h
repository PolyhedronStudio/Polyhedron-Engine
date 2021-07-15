/*
// LICENSE HERE.

//
// BaseEntityIterator.h
//
// Iterator to use for iterating the g_baseEntities array. (Class based Entities aka "classEntities").
//
*/
#ifndef __SVGAME_UTILS_BASEENTITYITERATOR_H__
#define __SVGAME_UTILS_BASEENTITYITERATOR_H__

class SVGBaseEntity;

//
// BaseEntityIterator iterator class, simple, yet effective! ;-)
//
class BaseEntityIterator {
private:
	SVGBaseEntity* first;
	SVGBaseEntity* last;

public:
	BaseEntityIterator(size_t first, size_t last);

	SVGBaseEntity* begin();
	SVGBaseEntity* end();
};

// Fetch an entity range, for iteration.
BaseEntityIterator FetchBaseEntitiesInRange(size_t start = 1, size_t end = MAX_EDICTS);

#endif // __SHARED_CONTAINERS_BASEENTITYITERATOR_H__