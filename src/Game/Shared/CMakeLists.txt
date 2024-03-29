###############################################################################
#
#			SharedGame CMakeLists
#
###############################################################################
###########
# Sources #
###########
SET(SRC_SHAREDGAME
	Entities/SGEntityHandle.cpp
	Entities/SGBaseItem.cpp

	Physics/MoveTypes/MoveTypeRootMotionMove.cpp
	Physics/MoveTypes/MoveTypeLinearProjectile.cpp
	Physics/MoveTypes/MoveTypeNoClip.cpp
	Physics/MoveTypes/MoveTypeNone.cpp
	Physics/MoveTypes/MoveTypePusher.cpp
	Physics/MoveTypes/MoveTypeToss.cpp
	Physics/Physics.cpp
	Physics/RootMotionMove.cpp

	World/IGameWorld.cpp

	PlayerMove.cpp
	SkeletalAnimation.cpp 
	Tracing.cpp
)

###########
# Headers #
###########
SET(HEADERS_SHAREDGAME
	Entities/ISharedGameEntity.h 
	Entities/SGEntityHandle.h  
	Entities/SGBaseItem.h
	Entities/TypeInfo.h 
	Entities.h 

	Physics/Physics.h
	Physics/RootMotionMove.h

	World/IGameWorld.h
	
	PlayerMove.h 
	Protocol.h 
	SkeletalAnimation.h 
	SharedGame.h 
	Time.h
	Tracing.h
)

#
#	SharedGame target for linking to ClientGame
#
if (CONFIG_BUILD_CL_GAME)
	ADD_LIBRARY(clgame_sharedgame OBJECT ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
	TARGET_COMPILE_DEFINITIONS(clgame_sharedgame PUBLIC "cxx_std_20 -DSHAREDGAME_CLIENTGAME=1")
	TARGET_COMPILE_OPTIONS(clgame_sharedgame PUBLIC "-fPIC")
	TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PUBLIC ${PATH_SHARED})
	TARGET_INCLUDE_DIRECTORIES(clgame_sharedgame PRIVATE ${GLM_INCLUDE_DIRS})

	# Link in FMT Header only library.
	target_link_libraries(clgame_sharedgame PUBLIC fmt::fmt-header-only)
	# Link in GLM headers.
	target_link_libraries(clgame_sharedgame PUBLIC glm)

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
	TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PUBLIC ${PATH_SHARED})
	TARGET_INCLUDE_DIRECTORIES(svgame_sharedgame PRIVATE ${GLM_INCLUDE_DIRS})
	
	# Link in FMT Header only library.
	target_link_libraries(svgame_sharedgame PUBLIC fmt::fmt-header-only)
	# Link in GLM headers.
	target_link_libraries(svgame_sharedgame PUBLIC glm)

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
