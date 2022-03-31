###############################################################################
#
#			SharedGame CMakeLists
#
###############################################################################
#		if(TARGET clgame)
#			# Remove client include in case CMake still thinks it has to be defined somehow.
#			REMOVE_DEFINITIONS(-DSHAREDGAME_SERVERGAME)
#			# Set SharedGame compilation settings for: ServerGame compilation.
#			ADD_COMPILE_DEFINITIONS(-DSHAREDGAME_CLIENTGAME)
#		elseif(TARGET svgame)
#			# Remove client include in case CMake still thinks it has to be defined somehow.
#			REMOVE_DEFINITIONS(-DSHAREDGAME_CLIENTGAME)
#			# Set SharedGame compilation settings for: ServerGame compilation.
#			ADD_COMPILE_DEFINITIONS(-DSHAREDGAME_SERVERGAME)
#		endif()

###########
# Sources #
###########
SET(SRC_SHAREDGAME
	Entities/SGEntityHandle.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/SGEntityHandle.cpp

	PMove.cpp
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
	
	PMove.h #${CMAKE_SOURCE_DIR}/src/Game/Shared/PMove.h 
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
	TARGET_COMPILE_DEFINITIONS(clgame_sharedgame PUBLIC "-DSHAREDGAME_CLIENTGAME=1")
	TARGET_COMPILE_OPTIONS(clgame_sharedgame PUBLIC /wd4005 /wd4996)

	TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PRIVATE ../../../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PRIVATE ../../../inc)
endif()

#
#	SharedGame target for linking to ServerGame
#
if (CONFIG_BUILD_SV_GAME)
	ADD_LIBRARY(svgame_sharedgame OBJECT ${HEADERS_SVGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
	TARGET_COMPILE_DEFINITIONS(svgame_sharedgame PUBLIC "-DSHAREDGAME_SERVERGAME=1")
	TARGET_COMPILE_OPTIONS(svgame_sharedgame PUBLIC /wd4005 /wd4996)

	TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PRIVATE ../../../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PRIVATE ../../../inc)
endif()


#add_library(svgame_sharedgame OBJECT ${SRC_SVGAME} ${HEADERS_SVGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
#target_compile_definitions(svgame_sharedgame PUBLIC SHAREDGAME_SERVERGAME=1 SHAREDGAME_CLIENTGAME=0)
#TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../../inc)
#TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../../VC/inc)
#target_include_directories(svgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../)
#target_compile_options(svgame_sharedgame PUBLIC /wd4005 /wd4996)

#add_library(clgame_sharedgame OBJECT ${SRC_CLGAME} ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
#target_compile_definitions(clgame_sharedgame PUBLIC SHAREDGAME_SERVERGAME=0 SHAREDGAME_CLIENTGAME=1)
#TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../../inc)
#TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../VC/inc)
#target_include_directories(clgame_sharedgame PUBLIC ${CMAKE_SOURCE_DIR}/../)
#target_compile_options(clgame_sharedgame PUBLIC /wd4005 /wd4996)