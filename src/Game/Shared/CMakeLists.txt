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
	${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/SGEntityHandle.cpp #Entities/SGEntityHandle.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/Entities/SGEntityHandle.cpp

	${CMAKE_SOURCE_DIR}/src/Game/Shared/PMove.cpp #PMove.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/PMove.cpp
	${CMAKE_SOURCE_DIR}/src/Game/Shared/SkeletalAnimation.cpp # SkeletalAnimation.cpp # ${CMAKE_SOURCE_DIR}/src/Game/Shared/SkeletalAnimation.cpp
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
)

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