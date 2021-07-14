/*
// LICENSE HERE.

//
// EntityIterator.cpp
//
//
*/
#include "../g_local.h"
#include "../entities.h"

//===============
// IteratorToEntity
//
// Inline function for iterator to entity.
// Returns a reference to the actual entity index.
//===============
inline Entity& IteratorToEntity(size_t index) {
	return g_entities[index];
}

//===============
// EntityIterator::EntityIterator
//
//===============
EntityIterator::EntityIterator(size_t first, size_t last) :
	first(&IteratorToEntity(first)),
	last(&IteratorToEntity(last + 1)) {
}

//===============
// EntityIterator::begin
//
//===============
Entity* EntityIterator::begin() {
	return first;
}

//===============
// EntityIterator::end
//
//===============
Entity* EntityIterator::end() {
	return last;
}

//===============
// InfoPlayerStart::PostSpawn
//
//===============
EntityIterator FetchEntitiesInRange(size_t start, size_t end ) {
	if (end < start)
		std::swap(start, end);

	return EntityIterator(start, end);
}