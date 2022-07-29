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

	# Add CLGame SharedGame sources.
	add_library(clgame_sharedgame OBJECT 
		GameBindings/ClientBinding.cpp
		GameBindings/ClientBinding.h

		${HEADERS_CLGAME} 
		${SRC_SHARED} 
		${HEADERS_SHARED} 
		${SRC_SHAREDGAME} 
		${HEADERS_SHAREDGAME}
	)

	# C++ STD 20, and DSHAREDGAME_CLIENTGAME = 1
	target_compile_definitions(clgame_sharedgame PUBLIC "cxx_std_20 -DSHAREDGAME_CLIENTGAME=1")
	# When not using an MSVC compiler, assume G++/CLang and pass the "position independent code" flag
	# which is a requirement for these compilers.
	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(clgame_sharedgame PUBLIC "-fPIC")
	endif()	
	# Ensure it has access to the include /src/Shared path.
	target_include_directories(clgame_sharedgame PUBLIC ${PATH_SHARED})

	# Lib: FMT
	target_link_libraries(clgame_sharedgame PUBLIC fmt::fmt-header-only)
	# Lib: GLM
	target_include_directories(clgame_sharedgame PRIVATE ${GLM_INCLUDE_DIRS})
	target_link_libraries(clgame_sharedgame PUBLIC glm)

	# Win32 Specifics.
	if (WIN32)
		target_compile_options(clgame_sharedgame PUBLIC /wd4005 /wd4996)
		target_include_directories(clgame_sharedgame PUBLIC ../../../VC/inc)
	endif()	
endif()

#
#	SharedGame target for linking to ServerGame
#
if (CONFIG_BUILD_SV_GAME)
	# Add SVGame SharedGame sources.
	add_library(svgame_sharedgame OBJECT 
		GameBindings/ServerBinding.cpp
		GameBindings/ServerBinding.h

		${HEADERS_SVGAME} 
		${SRC_SHARED} 
		${HEADERS_SHARED} 
		${SRC_SHAREDGAME} 
		${HEADERS_SHAREDGAME}
	)

	# C++ STD 20, and SHAREDGAME_SERVERGAME = 1
	target_compile_definitions(svgame_sharedgame PUBLIC "cxx_std_20 -DSHAREDGAME_SERVERGAME=1")
	# When not using an MSVC compiler, assume G++/CLang and pass the "position independent code" flag
	# which is a requirement for these compilers.
	if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		target_compile_options(svgame_sharedgame PUBLIC "-fPIC")
	endif()	
	# Ensure it has access to the include /src/Shared path.
	target_include_directories(svgame_sharedgame PUBLIC ${PATH_SHARED})
	
	# Lib: FMT
	target_link_libraries(svgame_sharedgame PUBLIC fmt::fmt-header-only)
	# Lib: GLM
	target_include_directories(svgame_sharedgame PRIVATE ${GLM_INCLUDE_DIRS})
	target_link_libraries(svgame_sharedgame PUBLIC glm)

	# Win32 Specifics.
	if (WIN32)
		target_compile_options(svgame_sharedgame PUBLIC /wd4005 /wd4996)
		target_include_directories(svgame_sharedgame PUBLIC ../../../VC/inc)
	endif()
endif()

#
# Source Groups
#
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHAREDGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SHAREDGAME} )
