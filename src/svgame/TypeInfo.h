#pragma once

#include <string>

class SVGBaseEntity;
typedef entity_s Entity;

//===============
// A static counter, used by TypeInfo to get compile-time IDs
//===============
class StaticCounter {
public:
	StaticCounter() {
		localID = GlobalID;
		GlobalID++;
	}

    // You can also use this to check the total number of classes
	inline static size_t GlobalID = 0U;

	size_t GetID() const {
		return localID;
	}

private:
	size_t localID; 
};

using EntityAllocatorFn = SVGBaseEntity* ( Entity* );

//===============
// TypeInfo, a system for getting runtime information about classes
//===============
class TypeInfo {
public:
	TypeInfo( const char* mapClassName, const char* entClassName, const char* superClassName, EntityAllocatorFn entityAllocator )
		: mapClass( mapClassName ), className( entClassName ), superName( superClassName ) {
		AllocateInstance = entityAllocator;
		prev = head;
		head = this;

		// Doesn't actually quite work here, so I wrote SetupSuperClasses
		super = GetInfoByName( superClassName );
	}

	// This will be used to allocate instances of each entity class
	// In theory, multiple map classnames can allocate one C++ class
	EntityAllocatorFn* AllocateInstance;

	// Is this entity of this specific class?
	bool IsClass( const TypeInfo& eci ) const {
		return classInfoID.GetID() == eci.classInfoID.GetID();
	}

	// Is this entity a subclass of this class?
	bool IsSubclassOf( const TypeInfo& eci ) const {
		if ( nullptr == super )
			return false;

		if ( classInfoID.GetID() == eci.classInfoID.GetID() )
			return true;

		return super->IsSubclassOf( eci );
	}

	// Get type info by map classname
	static TypeInfo* GetInfoByMapName( const char* name ) { 
		TypeInfo* current = nullptr;
		current = head;

		while ( current ) {
			if ( !strcmp( current->mapClass, name ) ) {
				return current;
            }
			current = current->prev;
		}

		return nullptr;
	}

	// Get type info by name
	static TypeInfo* GetInfoByName( const char* name ) {
		TypeInfo* current = nullptr;
		current = head;

		while ( current ) {
			if ( !strcmp( current->className, name ) ) {
				return current;
            }
			current = current->prev;
		}

		return nullptr;
	}

	// This is called during game initialisation to properly set all superclasses
	static void SetupSuperClasses() {
		TypeInfo* current = nullptr;
		current = head;

		while ( current ) {
			current->super = GetInfoByName( current->superName );
			current = current->prev;
		}
	}

	TypeInfo*       prev;
	inline static TypeInfo* head = nullptr;

	StaticCounter   classInfoID; // automatically increments itself; TODO: maybe generate a CRC32 for each classname instead?
	TypeInfo*       super;

	const char*     mapClass;
	const char*     className;
	const char*     superName;
};

// ========================================================================
// Welcome to macro abuse purgatory, a.k.a. M.A.P.
// TODO: rewrite these macros using templates? It'd be quite nicer that way
// ========================================================================

// Declares and initialises the type information for a class
// @param mapClassName - the map classname of this entity, used during entity spawning
// @param className - the internal C++ class name
#define __DeclareTypeInfo( mapClassName, className, superClass, allocatorFunction )	\
virtual TypeInfo* GetTypeInfo() {								\
	return &ClassInfo;											\
}																\
inline static TypeInfo ClassInfo = TypeInfo( (mapClassName), (className), (superClass), (allocatorFunction) );

// Top abstract class, the start of the class tree
// Instances of this cannot be allocated, as it is abstract
#define DefineTopAbstractClass( className )						\
__DeclareTypeInfo( #className "_abs", #className, "none", nullptr );

// Abstract class that inherits from another
// Instances of this cannot be allocated
// NOTE: multiple inheritance not supported
#define DefineAbstractClass( className, superClass )			\
using Base = superClass;										\
__DeclareTypeInfo( #className "_abs", #className, #superClass, nullptr );

// Declares and initialises the type information for this class, so it can be spawned in a map. 
// NOTE: multiple inheritance not supported
// @param mapClassName (string) - the map classname of this entity, used during entity spawning
// @param className (symbol) - the internal C++ class name
// @param superClass (symbol) - the class this entity class inherits from
#define DefineMapClass( mapClassName, className, superClass )	\
using Base = superClass;										\
static SVGBaseEntity* AllocateInstance( Entity* entity ) {		\
	return new className( entity );								\
}																\
__DeclareTypeInfo( mapClassName, #className, #superClass, &className::AllocateInstance );

// Declares and initialises the type information for this class
// NOTE: multiple inheritance not supported
// @param className (symbol) - the internal C++ class name
// @param superClass (symbol) - the class this entity class inherits from
#define DefineClass( className, superClass )					\
using Base = superClass;										\
static SVGBaseEntity* AllocateInstance( Entity* entity ) {		\
	return new className( entity );								\
}																\
__DeclareTypeInfo( #className "_dyn", #className, #superClass, &className::AllocateInstance );
