SET(SRC_CLGAME
	ClientGameExports.cpp
	ClientGameImports.cpp

	Effects/DynamicLights.cpp
	Effects/LightStyles.cpp
	Effects/MuzzleFlashEffects.cpp
	Effects/Particles.cpp
	Effects/Particles/BloodSplatters.cpp
	Effects/Particles/BubbleTrailA.cpp
	Effects/Particles/BubbleTrailB.cpp
	Effects/Particles/DiminishingTrail.cpp
	Effects/Particles/DirtAndSparks.cpp
	Effects/Particles/ExplosionSparks.cpp
	Effects/Particles/ForceWall.cpp
	Effects/Particles/HeatBeam.cpp
	Effects/Particles/ItemRespawn.cpp
	Effects/Particles/LogoutEffect.cpp
	Effects/Particles/MonsterPlasmaShell.cpp
	Effects/Particles/SteamPuffs.cpp
	Effects/Particles/Teleporter.cpp
	Effects/Particles/WaterSplash.cpp

	Entities/Base/CLGBaseEntity.cpp
	Entities/Monsters/MonsterTestDummy.cpp
	Entities/ClassEntityList.cpp
	Entities/IClientGameEntity.cpp


	Exports/Core.cpp
	Exports/Entities.cpp
	Exports/Media.cpp
	Exports/Movement.cpp
	Exports/Prediction.cpp
	Exports/Screen.cpp
	Exports/ServerMessage.cpp
	Exports/View.cpp

	HUD/ChatHUD.cpp
	HUD/NumberHUD.cpp

	Input/KeyBinding.cpp

	#Physics/Physics.cpp
	#Physics/StepMove.cpp

	Debug.cpp
	Entities.cpp
	Main.cpp
	TemporaryEntities.cpp

	#CLGame.def
)

SET(HEADERS_CLGAME
	ClientGameExports.h
	ClientGameImports.h

	Effects/DynamicLights.h
	Effects/LightStyles.h
	Effects/MuzzleFlashEffects.h
	Effects/ParticleEffects.h
	Effects/Particles.h

	Entities/Base/CLGBaseEntity.h
	Entities/Monsters/MonsterTestDummy.h
	Entities/ClassEntityList.h
	Entities/IClientGameEntity.h

	Exports/Core.h
	Exports/Entities.h
	Exports/Media.h
	Exports/Movement.h
	Exports/Prediction.h
	Exports/Screen.h
	Exports/ServerMessage.h
	Exports/View.h

	HUD/ChatHUD.h
	HUD/NumberHUD.h

	Input/KeyBinding.h
	
	#Physics/StepMove.h

	ClientGameLocals.h
	Debug.h
	Entities.h
	Main.h
	TemporaryEntities.h
)

#
# Target: ClientGame Module.
#
ADD_LIBRARY(clgame SHARED CLGame.def ${SRC_CLGAME} ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME}) #${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
# Set definitions for SharedGame Framework.
ADD_DEFINITIONS("-DSHAREDGAME_CLIENTGAME=1")
# Include SharedGame Framework.
TARGET_INCLUDE_DIRECTORIES(clgame PRIVATE Game/Shared)
