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
*	Includes.
**/
// Type Info System.
#include "Entities/TypeInfo.h"

// Shared Entity Handle.
#include "Entities/SGEntityHandle.h"

// Shared Entity Interface.
#include "Entities/ISharedGameEntity.h"

//! This is the actual GameWorld POD array with a size based on which GameModule we are building for.
using PODGameWorldArray = PODEntity[MAX_POD_ENTITIES];

//! std::span for PODEntity* objects.
using PODEntitySpan = std::span<PODEntity>;
//! std::vector for PODEntity* objects.
using PODEntityVector = std::vector<PODEntity*>;
//! std::span for GameEntity* derived objects.
using GameEntitySpan = std::span<GameEntity*>;
//! std::vector for GameEntity* derived objects.
using GameEntityVector = std::vector<GameEntity*>;
