#
#	ServerGame Module.
#
if (CONFIG_BUILD_SV_GAME)
# Server game DLL
# Throw all sources in here, but group them elsewhere
SET(SRC_SVGAME 

	Ballistics.cpp
	ChaseCamera.cpp
	Commands.cpp
	Effects.cpp
	Entities.cpp
	ServerGameLocals.cpp
	ImportsWrapper.cpp
	Main.cpp
	Effects.cpp
	Save.cpp
	SVCommands.cpp
	Utilities.cpp
	Weapons.cpp 

	Entities/Base/SVGBaseEntity.cpp
	Entities/Base/SVGBaseItem.cpp
	Entities/Base/SVGBaseItemAmmo.cpp
	Entities/Base/SVGBaseItemWeapon.cpp
	Entities/Base/SVGBaseTrigger.cpp
	Entities/Base/SVGBaseMonster.cpp
	Entities/Base/SVGBaseMover.cpp
	Entities/Base/SVGBasePlayer.cpp
	Entities/Base/BodyCorpse.cpp
	Entities/Base/DebrisEntity.cpp
	Entities/Base/GibEntity.cpp

	#Entities/Info/InfoNotNull.cpp
	Entities/Info/InfoNull.cpp
	Entities/Info/InfoPlayerCoop.cpp
	Entities/Info/InfoPlayerDeathmatch.cpp
	Entities/Info/InfoPlayerIntermission.cpp
	Entities/Info/InfoPlayerStart.cpp

	Entities/Items/ItemAmmo9mm.cpp
	Entities/Items/ItemHealthMega.cpp
	Entities/Items/ItemWeaponKnife.cpp
	Entities/Items/ItemWeaponBeretta.cpp
	Entities/Items/ItemWeaponNone.cpp
	Entities/Items/ItemWeaponSMG.cpp

	Entities/Func/FuncAreaportal.cpp
	Entities/Func/FuncButton.cpp
	#Entities/Func/FuncConveyor.cpp
	Entities/Func/FuncDoor.cpp 
	Entities/Func/FuncDoorRotating.cpp
	Entities/Func/FuncExplosive.cpp 
	#Entities/Func/FuncKillbox.cpp
	#Entities/Func/FuncObject.cpp
	Entities/Func/FuncPlat.cpp
	Entities/Func/FuncRotating.cpp
	Entities/Func/FuncTimer.cpp
	Entities/Func/FuncTrain.cpp 
	Entities/Func/FuncWall.cpp
	#Entities/Func/FuncWater.cpp

	#Entities/Misc/MiscExplobox.cpp
	#Entities/Misc/MiscGibArm.cpp 
	#Entities/Misc/MiscGibLeg.cpp 
	#Entities/Misc/MiscGibHead.cpp 
	Entities/Misc/MiscExplosionBox.cpp
	Entities/Misc/MiscServerModel.cpp
	#Entities/Misc/MiscTeleporter.cpp
	#Entities/Misc/MiscTeleporterDest.cpp

	Entities/Monsters/MonsterTestDummy.cpp

	Entities/Path/PathCorner.cpp
	#Entities/Target/TargetBlaster.cpp
	#Entities/Target/TargetChangelevel.cpp
	#Entities/Target/TargetCrosslevel_target.cpp
	#Entities/Target/TargetCrosslevel_trigger.cpp
	Entities/Target/TargetEarthquake.cpp
	Entities/Target/TargetExplosion.cpp
	#Entities/Target/TargetLightramp.cpp
	#Entities/Target/TargetSpawner.cpp
	Entities/Target/TargetSpeaker.cpp
	#Entities/Target/TargetSplash.cpp
	Entities/Target/TargetTempEntity.cpp

	Entities/Trigger/TriggerCounter.cpp
	#Entities/Trigger/TriggerElevator.cpp
	Entities/Trigger/TriggerGravity.cpp
	#Entities/Trigger/TriggerKey.cpp
	#Entities/Trigger/TriggerMonsterjump.cpp
	Entities/Trigger/TriggerPush.cpp
	Entities/Trigger/TriggerAlways.cpp
	Entities/Trigger/TriggerAutoDoor.cpp
	Entities/Trigger/TriggerAutoPlatform.cpp
	Entities/Trigger/TriggerDelayedUse.cpp
	Entities/Trigger/TriggerHurt.cpp
	Entities/Trigger/TriggerMultiple.cpp
	Entities/Trigger/TriggerOnce.cpp
	Entities/Trigger/TriggerRelay.cpp

	Entities/Weaponry/BlasterBolt.cpp

	Entities/Light.cpp
	Entities/Worldspawn.cpp

	Gamemodes/DefaultGamemode.cpp
	Gamemodes/CoopGamemode.cpp
	Gamemodes/DeathMatchGamemode.cpp

	Physics/Physics.cpp
	Physics/StepMove.cpp

	Player/Client.cpp
	Player/Hud.cpp

	Utilities/SVGTraceResult.cpp

	World/ServerGameworld.cpp
	#SVGame.def
	
)
    
SET(HEADERS_SVGAME

	Ballistics.h
	ChaseCamera.h
	Effects.h
	Entities.h
	ServerGameLocals.h
	TypeInfo.h
	Utilities.h

	Entities/Base/SVGBaseEntity.h
	Entities/Base/SVGBaseItem.h
	Entities/Base/SVGBaseItemAmmo.h
	Entities/Base/SVGBaseItemWeapon.h
	Entities/Base/SVGBaseTrigger.h
	Entities/Base/SVGBaseMonster.h
	Entities/Base/SVGBaseMover.h
	Entities/Base/SVGBasePlayer.h
	Entities/Base/BodyCorpse.h
	Entities/Base/DebrisEntity.h
	Entities/Base/GibEntity.h

	Entities/Func/FuncAreaportal.h
	Entities/Func/FuncButton.h
	Entities/Func/FuncDoor.h
	Entities/Func/FuncDoorRotating.h
	Entities/Func/FuncExplosive.h
	Entities/Func/FuncPlat.h
	Entities/Func/FuncRotating.h
	Entities/Func/FuncTimer.h
	Entities/Func/FuncTrain.h
	Entities/Func/FuncWall.h

	#Entities/Info/InfoNull.h - Only has a .cpp file.
	Entities/Info/InfoPlayerCoop.h
	Entities/Info/InfoPlayerDeathmatch.h
	Entities/Info/InfoPlayerIntermission.h
	Entities/Info/InfoPlayerStart.h
	
	Entities/Items/ItemAmmo9mm.h
	Entities/Items/ItemHealthMega.h
	Entities/Items/ItemWeaponKnife.h
	Entities/Items/ItemWeaponBeretta.h
	Entities/Items/ItemWeaponSMG.h
	Entities/Items/ItemWeaponNone.h

	Entities/Misc/MiscExplosionBox.h
	Entities/Misc/MiscServerModel.h
	Entities/Monsters/MonsterTestDummy.h
	Entities/Path/PathCorner.h
	Entities/Target/TargetEarthquake.h
	Entities/Target/TargetExplosion.h
	Entities/Target/TargetSpeaker.h
	Entities/Target/TargetTempEntity.h
	Entities/Trigger/TriggerAlways.h
	Entities/Trigger/TriggerAutoDoor.h
	Entities/Trigger/TriggerAutoPlatform.h
	Entities/Trigger/TriggerCounter.h
	Entities/Trigger/TriggerDelayedUse.h
	Entities/Trigger/TriggerHurt.h
	Entities/Trigger/TriggerGravity.h
	Entities/Trigger/TriggerMultiple.h
	Entities/Trigger/TriggerOnce.h
	Entities/Trigger/TriggerPush.h
	Entities/Trigger/TriggerRelay.h
	Entities/Weaponry/BlasterBolt.h

	Entities/IServerGameEntity.h
	Entities/Light.h
	Entities/Worldspawn.h

	Gamemodes/DefaultGamemode.h
	Gamemodes/CoopGamemode.h
	Gamemodes/DeathMatchGamemode.h
	Gamemodes/IGamemode.h

	Player/Animations.h
	Player/Client.h

	Utilities/SVGTraceResult.h

	World/ServerGameworld.h
	#SVGame.def

)

#
# Target: ServerGame Module.
#
if (WIN32)
	ADD_LIBRARY(svgame SHARED SVGame.def ${SRC_SVGAME} ${HEADERS_SVGAME} ${HEADERS_SHARED} ${HEADERS_SHAREDGAME} $<TARGET_OBJECTS:svgame_sharedgame>)
else()
	ADD_LIBRARY(svgame SHARED ${SRC_SVGAME} ${HEADERS_SVGAME} ${HEADERS_SHARED} ${HEADERS_SHAREDGAME} $<TARGET_OBJECTS:svgame_sharedgame>)
endif()

# Set the flag for SharedGame to build with ServerGame compatability in mind.
TARGET_COMPILE_DEFINITIONS(svgame PUBLIC "cxx_std_20 -DSHAREDGAME_SERVERGAME=1")

# Include directories.
TARGET_INCLUDE_DIRECTORIES(svgame PUBLIC PATH_SHARED )
# Include SharedGame Framework.
TARGET_INCLUDE_DIRECTORIES(svgame PUBLIC Game/Shared)

# Source Groups.
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SVGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SVGAME} )

# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
SET_TARGET_PROPERTIES(svgame
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