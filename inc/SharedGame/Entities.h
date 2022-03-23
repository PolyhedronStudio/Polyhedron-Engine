/***
*
*	License here.
*
*	@file
*
*	Client Game BaseEntity.
* 
***/
#pragma once



/**
*	Predeclare the actual class entity type that we intend to use depending on which game module
*	this file is being compiled along with.
**/
#ifdef CGAME_INCLUDE
// Predeclarations.
class CLGBaseEntity;
struct ClientEntity;

// Using =
using ClassEntity = CLGBaseEntity;
//! For now, equals POD Entity.
using Entity = ClientEntity;
//! POD Entity.
using PODEntity = ClientEntity;
//! Entity Dictionary.
using EntityDictionary = std::map<std::string, std::string>;
#else
// Predeclarations.
class SVGBaseEntity;
using ClassEntity = SVGBaseEntity;

//! For now, equals POD Entity.
using Entity = entity_s;
//! POD Entity.
using PODEntity = entity_s;
//! Entity Dictionary.
using EntityDictionary = std::map<std::string, std::string>;
#endif




/**
*	Includes.
**/
// Type Info System.
#include "SharedGame/Entities/TypeInfo.h"

// SharedGame Entity Interface.
#include "SharedGame/Entities/ISharedGameEntity.h"
