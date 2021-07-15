/*
// LICENSE HERE.

//
// EntityIterator.h
//
// Iterator to use for iterating the g_entities array. (Server Entities.)
//
*/
#ifndef __SVGAME_UTILS_ENTITYITERATOR_H__
#define __SVGAME_UTILS_ENTITYITERATOR_H__

//
// Entity iterator class, simple, yet effective! ;-)
//
class EntityIterator {
private:
	Entity *first;
	Entity *last;

public:
	EntityIterator(size_t first, size_t last);

	Entity* begin();
	Entity* end();
};

// Fetch an entity range, for iteration.
EntityIterator FetchEntitiesInRange(size_t start = 1, size_t end = MAX_EDICTS);

#endif // __SHARED_CONTAINERS_ENTITYITERATOR_H__