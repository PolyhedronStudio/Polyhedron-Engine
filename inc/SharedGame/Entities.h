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
#ifdef SGINCLUDE_CLIENTGAME
// Start of ClientGame.



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



// End of ClientGame.
#else // Presume it being SGINCLUDE_SERVERGAME
// Start of ServerGame.



// Predeclarations.
class SVGBaseEntity;
struct entity_s;
using ClassEntity = SVGBaseEntity;

//! For now, equals POD Entity.
using Entity = entity_s;
//! POD Entity.
using PODEntity = entity_s;
//! Entity Dictionary.
using EntityDictionary = std::map<std::string, std::string>;



// End of ServerGame.
#endif



/**
*	Includes.
**/
// Type Info System.
#include "SharedGame/Entities/TypeInfo.h"

// Shared Entity Handle.
#include "SharedGame/Entities/SGEntityHandle.h"

// Shared Entity Interface.
#include "SharedGame/Entities/ISharedGameEntity.h"