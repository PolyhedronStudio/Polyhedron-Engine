#############################################################################
##
##
##	Polyhedron Project: SharedGame Sources
##                      (Compiled along with ClientGame and ServerGame.)
##
##
#############################################################################
####
##  Sources
####
SET(PATH_SHAREDGAME ${CMAKE_CURRENT_LIST_DIR})

SET(SRC_SHAREDGAME
	${PATH_SHAREDGAME}/Entities/SGEntityHandle.cpp
	${PATH_SHAREDGAME}/Entities/SGBaseItem.cpp
	${PATH_SHAREDGAME}/Entities/EntityFilters.cpp

	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypeRootMotionMove.cpp
	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypeLinearProjectile.cpp
	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypeNoClip.cpp
	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypeNone.cpp
	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypePusher.cpp
	${PATH_SHAREDGAME}/Physics/MoveTypes/MoveTypeToss.cpp
	${PATH_SHAREDGAME}/Physics/Physics.cpp
	${PATH_SHAREDGAME}/Physics/RootMotionMove.cpp

	${PATH_SHAREDGAME}/World/IGameWorld.cpp

	${PATH_SHAREDGAME}/PlayerMove.cpp
	${PATH_SHAREDGAME}/SkeletalAnimation.cpp 
	${PATH_SHAREDGAME}/Tracing.cpp
)

###
##  Headers
####
SET(HEADERS_SHAREDGAME
	${PATH_SHAREDGAME}/Entities/EntityFilters.h
	${PATH_SHAREDGAME}/Entities/ISharedGameEntity.h 
	${PATH_SHAREDGAME}/Entities/SGEntityHandle.h  
	${PATH_SHAREDGAME}/Entities/SGBaseItem.h
	${PATH_SHAREDGAME}/Entities/TypeInfo.h 
	${PATH_SHAREDGAME}/Entities.h 

	${PATH_SHAREDGAME}/Physics/Physics.h
	${PATH_SHAREDGAME}/Physics/RootMotionMove.h

	${PATH_SHAREDGAME}/World/IGameWorld.h
	
	${PATH_SHAREDGAME}/ButtonBits.h
	${PATH_SHAREDGAME}/Entities.h
	${PATH_SHAREDGAME}/EntityEffectTypes.h
	${PATH_SHAREDGAME}/EntityFlags.h
	${PATH_SHAREDGAME}/GameModeFlags.h
	${PATH_SHAREDGAME}/ItemIDs.h
	${PATH_SHAREDGAME}/MuzzleFlashTypes.h
	${PATH_SHAREDGAME}/PlayerMove.h 
	${PATH_SHAREDGAME}/Protocol.h 
	${PATH_SHAREDGAME}/RenderDrawFlags.h
	${PATH_SHAREDGAME}/SharedGame.h 
	${PATH_SHAREDGAME}/SkeletalAnimation.h 
	${PATH_SHAREDGAME}/TempEntityEvents.h
	${PATH_SHAREDGAME}/Time.h
	${PATH_SHAREDGAME}/Tracing.h
	${PATH_SHAREDGAME}/Utilities.h
	${PATH_SHAREDGAME}/WaterLevels.h
	${PATH_SHAREDGAME}/WeaponStates.h
)

####
##	Set the SRC_SHAREDGAME_CLIENTGAME files if intended to build along with the client game module.
####
if (CONFIG_BUILD_GAME_CLIENT)
	SET( SRC_SHAREDGAME_CLIENTGAME 
		${PATH_SHAREDGAME}/GameBindings/ClientBinding.cpp
	)
	SET( HEADERS_SHAREDGAME_CLIENTGAME 
		${PATH_SHAREDGAME}/GameBindings/ClientBinding.h
	)
endif()

#
#	SharedGame target for linking to ServerGame
#
if (CONFIG_BUILD_SV_GAME)
	SET( SRC_SHAREDGAME_SERVERGAME 
		${PATH_SHAREDGAME}/GameBindings/ServerBinding.cpp
	)
	SET( HEADERS_SHAREDGAME_SERVERGAME 
		${PATH_SHAREDGAME}/GameBindings/ServerBinding.h
	)
endif()
