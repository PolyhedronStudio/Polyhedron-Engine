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


/**
*	SHAREDGAME_CLIENTGAME
**/
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


/**
*	SHAREDGAME_SERVERGAME
**/
#ifdef SHAREDGAME_SERVERGAME
//! Basic Main & Locals.
#include "Game/Server/ServerGameMain.h"
#include "Game/Server/ServerGameLocals.h"

//! Game World.
#include "Game/Server/World/ServerGameWorld.h"

//! Base Entities.
#include "Game/Server/Entities/Base/SVGBaseEntity.h"
#include "Game/Server/Entities/Base/SVGBaseMover.h"
#include "Game/Server/Entities/Base/SVGBaseTrigger.h"
#include "Game/Server/Entities/Base/SVGBaseItem.h"
#include "Game/Server/Entities/Base/SVGBaseItemAmmo.h"
#include "Game/Server/Entities/Base/SVGBaseItemWeapon.h"
#include "Game/Server/Entities/Base/SVGBaseSkeletalAnimator.h"
#include "Game/Server/Entities/Base/SVGBaseRootMotionMonster.h"
//#include "Game/Server/Entities/Base/SVGBaseStepMonster.h"
#include "Game/Server/Entities/Base/SVGBasePlayer.h"

//! SharedGame Client Bindings.
#include "Game/Shared/GameBindings/ServerBinding.h"

#endif
