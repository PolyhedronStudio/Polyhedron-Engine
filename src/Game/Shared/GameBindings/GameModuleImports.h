/***
*
*	License here.
*
*	@file
* 
*   Main header to include for each SharedGame .cpp translation unit.
*
*	Takes care of including the corresponding game module its main and the
*	GameLocals header files.
*
***/
#pragma once


//! ClientGame
#ifdef SHAREDGAME_CLIENTGAME
//! Basic Main & Locals.
#include "Game/Client/ClientGameMain.h"
#include "Game/Client/ClientGameLocals.h"
//! Game World.
#include "Game/Client/World/ClientGameWorld.h"
//! Base Entities.
#include "Game/Client/Entities/Base/CLGBaseLocalEntity.h"
#include "Game/Client/Entities/Base/CLGBasePacketEntity.h"
#include "Game/Client/Entities/Base/CLGBaseMover.h"
#include "Game/Client/Entities/Base/CLGBaseTrigger.h"
#include "Game/Client/Entities/Base/CLGBasePlayer.h"
//! SharedGame Client Bindings.
#include "Game/Shared/GameBindings/ClientBinding.h"
#endif
//! ServerGame
#ifdef SHAREDGAME_SERVERGAME
#include "Game/Server/ServerGameMain.h"
#include "Game/Server/ServerGameLocals.h"
#include "Game/Server/World/ServerGameWorld.h"
#endif
