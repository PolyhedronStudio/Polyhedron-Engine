/***
*
*	License here.
*
*	@file
*
*	GameWorld header.
*
*	The game world handles management of entities by using the EntityBridge handle
*	class. Use it to create, find and destroy entities safely.
*
***/
#pragma once

#ifdef SHAREDGAME_CLIENT
#include "CLGame/ClientGameLocal.h"
#else
#include "SVGame/ServerGameLocal.h"
#endif

#include "Entities/SGEntityBridge.h"
#include "Entities/SGBaseEntity.h"
#include "Entities/SGGameWorld.h"