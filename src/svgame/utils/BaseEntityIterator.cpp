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
// BaseEntityIterator
//
// Inline function for iterator to base entity.
// Returns a reference to the actual base entity index.
//===============
inline SVGBaseEntity *BaseIteratorToEntity(size_t index) {
	return g_baseEntities[index];
}

//===============
// EntityIterator::EntityIterator
//
//===============
BaseEntityIterator::BaseEntityIterator(size_t first, size_t last) :
	first(BaseIteratorToEntity(first)),
	last(BaseIteratorToEntity(last + 1)) {
}

//===============
// BaseEntityIterator::begin
//
//===============
SVGBaseEntity* BaseEntityIterator::begin() {
	return first;
}

//===============
// BaseEntityIterator::end
//
//===============
SVGBaseEntity* BaseEntityIterator::end() {
	return last;
}

//===============
// FetchBaseEntitiesInRange
//
//===============
BaseEntityIterator FetchBaseEntitiesInRange(size_t start, size_t end) {
	if (end < start)
		std::swap(start, end);

	return BaseEntityIterator(start, end);
}