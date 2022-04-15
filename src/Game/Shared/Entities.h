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

//! Span for PODEntity* objects.
using PODEntitySpan = std::span<PODEntity>;
//! Vector for PODEntity* objects.
using PODEntityVector = std::vector<PODEntity*>;
//! Span for GameEntity* derived objects.
using GameEntitySpan = std::span<GameEntity*>;
//! Vector for GameEntity* derived objects.
using GameEntityVector = std::vector<GameEntity*>;