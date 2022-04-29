#
#	ClientGame Module.
#
if (CONFIG_BUILD_CL_GAME)

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

	Entities/Base/CLGBaseLocalEntity.cpp
	Entities/Base/CLGBasePacketEntity.cpp
	Entities/Base/CLGBasePlayer.cpp
	Entities/Base/CLGBaseTrigger.cpp
	

	Entities/Misc/MiscExplosionBox.cpp
	Entities/Misc/MiscClientExplosionBox.cpp
	Entities/Misc/MiscClientModel.cpp
	
	Entities/Monsters/MonsterTestDummy.cpp

	#Entities/GameEntityList.cpp
	Entities/IClientGameEntity.cpp
	Entities/Worldspawn.cpp

	Utilities/CLGTraceResult.cpp

	World/ClientGameWorld.cpp

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

	Physics/Physics.cpp
	Physics/StepMove.cpp

	View/ViewCamera.cpp

	ClientGameLocals.cpp
	Debug.cpp
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

	Entities/Base/CLGBaseLocalEntity.h
	Entities/Base/CLGBasePacketEntity.h
	Entities/Base/CLGBasePlayer.h
	Entities/Base/CLGBaseTrigger.h
	
	Entities/Misc/MiscExplosionBox.h
	Entities/Misc/MiscClientExplosionBox.h
	Entities/Misc/MiscClientModel.h

	Entities/Monsters/MonsterTestDummy.h

	#Entities/GameEntityList.h
	Entities/IClientGameEntity.h
	Entities/Worldspawn.h

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
	
	Physics/StepMove.h

	View/ViewCamera.h

	Utilities/CLGTraceResult.h

	World/ClientGameWorld.h

	ClientGameLocals.h
	Debug.h
	Main.h
	TemporaryEntities.h
)

#
# Target: ClientGame Module.
#
if (WIN32)
	ADD_LIBRARY(clgame SHARED CLGame.def ${SRC_CLGAME} ${HEADERS_CLGAME} ${HEADERS_SHARED} ${HEADERS_SHAREDGAME} $<TARGET_OBJECTS:clgame_sharedgame>) #${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
else()
	ADD_LIBRARY(clgame SHARED ${SRC_CLGAME} ${HEADERS_CLGAME} ${HEADERS_SHARED} ${HEADERS_SHAREDGAME} $<TARGET_OBJECTS:clgame_sharedgame>) #${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
endif()

# Set the flag for SharedGame to build with ClientGame compatability in mind.
TARGET_COMPILE_DEFINITIONS(clgame PUBLIC "cxx_std_20 -DSHAREDGAME_CLIENTGAME=1")

# Include directories.
TARGET_INCLUDE_DIRECTORIES(clgame PUBLIC PATH_SHARED)
# Include SharedGame Framework.
TARGET_INCLUDE_DIRECTORIES(clgame PUBLIC Game/Shared)

# Source Groups.
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_CLGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_CLGAME} )

# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
SET_TARGET_PROPERTIES(clgame
	PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
	LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
	LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
	LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
	RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
	RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
	PREFIX ""
	DEBUG_POSTFIX ""
)

endif()