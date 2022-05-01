###############################################################################
#
#			SharedGame CMakeLists
#
###############################################################################
###########
# Sources #
###########
SET(SRC_SHAREDGAME
	Entities/SGEntityHandle.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/SGEntityHandle.cpp

#	Physics/MoveTypes/MoveTypeLinearProjectile.cpp
#	Physics/MoveTypes/MoveTypeNone.cpp
#	Physics/MoveTypes/MoveTypePusher.cpp
#	Physics/MoveTypes/MoveTypeToss.cpp
#	Physics/MoveTypes/MoveTypeTossSlide.cpp
	Physics/Physics.cpp

	World/IGameWorld.cpp

	PlayerMove.cpp
	SkeletalAnimation.cpp # SkeletalAnimation.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/SkeletalAnimation.cpp
)

###########
# Headers #
###########
SET(HEADERS_SHAREDGAME
	Entities/ISharedGameEntity.h # ${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/ISharedGameEntity.h 
	Entities/SGEntityHandle.h  #${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/SGEntityHandle.h 
	Entities/TypeInfo.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/TypeInfo.h 
	Entities.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities.h 

	Physics/Physics.h

	World/IGameWorld.h
	
	PlayerMove.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/PMove.h 
	Protocol.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/Protocol.h 
	SkeletalAnimation.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/SkeletalAnimation.h 
	SharedGame.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/SharedGame.h 
	Time.h
)

#
#	SharedGame target for linking to ClientGame
#
if (CONFIG_BUILD_CL_GAME)
	ADD_LIBRARY(clgame_sharedgame OBJECT ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
	TARGET_COMPILE_DEFINITIONS(clgame_sharedgame PUBLIC "cxx_std_20 -DSHAREDGAME_CLIENTGAME=1")
	TARGET_COMPILE_OPTIONS(clgame_sharedgame PUBLIC "-fPIC")
	TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PUBLIC PATH_SHARED)

	# Win32 Specifics.
	if (WIN32)
		TARGET_COMPILE_OPTIONS(clgame_sharedgame PUBLIC /wd4005 /wd4996)
		TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PUBLIC ../../../VC/inc)
	endif()	
endif()

#
#	SharedGame target for linking to ServerGame
#
if (CONFIG_BUILD_SV_GAME)
	ADD_LIBRARY(svgame_sharedgame OBJECT ${HEADERS_SVGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
	TARGET_COMPILE_DEFINITIONS(svgame_sharedgame PUBLIC "cxx_std_20 -DSHAREDGAME_SERVERGAME=1")
	TARGET_COMPILE_OPTIONS(svgame_sharedgame PUBLIC "-fPIC")
	TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PUBLIC PATH_SHARED)
	
	# Win32 Specifics.
	if (WIN32)
		TARGET_COMPILE_OPTIONS(svgame_sharedgame PUBLIC /wd4005 /wd4996)
		TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PUBLIC ../../../VC/inc)
	endif()
	
endif()

#
# Source Groups
#
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHAREDGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SHAREDGAME} )
