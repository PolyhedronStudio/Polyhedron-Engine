############################################################################
##  Polyhedron ServerGame Module(.dll/.so):                               ##
##                                                                        ##
##  When CONFIG_BUILD_GAME_SERVER is set the ServerGame Module builds as  ##
##  a shared library(.dll/.so) file taking along with itself the Shared-  ##
##  Game code and its ServerGame bindings.                                ##
############################################################################
if( CONFIG_BUILD_GAME_SERVER )
	####
	## CMake Project and version number
	####
	project(target-svgame VERSION "${POLYHEDRON_VERSION_MAJOR}.${POLYHEDRON_VERSION_MINOR}.${POLYHEDRON_VERSION_POINT}" )
	# Add SHARED library.
	if (WIN32)
		add_library( svgame SHARED ${SRC_SYSTEM_DIR}/Resources/basepoly_sv.rc )
	else()
		add_library( svgame SHARED )
	endif()

	# Setup all Server target Sources.
	target_sources( svgame PUBLIC
		#${SRC_GAME_SERVER_DIR}/svgame.def
		${SRC_GAME_SERVER} ${HEADERS_GAME_SERVER}
		${SRC_GAME_SHARED_DIR}/GameBindings/ServerBinding.cpp
		${SRC_GAME_SHARED_DIR}/GameBindings/ServerBinding.h
		${SRC_GAME_SHARED} ${HEADERS_GAME_SHARED}
	)
	target_sources( svgame PUBLIC 
		${SRC_SHARED} ${HEADERS_SHARED}
	)

	# And also on any specific headers that require it.
	set_source_files_properties(
		${SRC_GAME_SHARED_DIR}/GameBindings/ServerBinding.cpp
		${SRC_GAME_SHARED_DIR}/GameBindings/ServerBinding.h
		COMPILE_OPTIONS 
		"SHAREDGAME_UNIT_INCLUDE=1"
	)
	# Set the SHAREDGAME_UNIT_INCLUDE flags for all SharedGame source files.
	set_source_files_properties(${SRC_GAME_SHARED} COMPILE_OPTIONS "SHAREDGAME_UNIT_INCLUDE=1")
	# And also on any specific headers that require it.
	set_source_files_properties(${SRC_GAME_SHARED_DIR}/Entities/EntityFilters.h COMPILE_OPTIONS "SHAREDGAME_UNIT_INCLUDE=1")

	# Add include directories.
	target_include_directories( svgame INTERFACE 
		"${SRC_ROOT_DIR}" 
	)
	target_include_directories( svgame PRIVATE 
		"${SRC_ROOT_DIR}"
	)

	# Embed version numbers.
	target_compile_definitions( svgame PRIVATE 
		VERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR}
		VERSION_MINOR=${POLYHEDRON_VERSION_MINOR}
		VERSION_POINT=${POLYHEDRON_VERSION_POINT}
		VERSION_SHA=${POLYHEDRON_VERSION_SHA}
		VERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH}
	)

	# We always have a config.h in use.
	target_compile_definitions( svgame PRIVATE _CRT_SECURE_NO_WARNINGS )

	# Add 'public' C++20 requirement.
	target_compile_features( svgame PUBLIC "cxx_std_20" )

	# Require C++20 Support.
	set_target_properties( svgame PROPERTIES CMAKE_CXX_STANDARD_REQUIRED ON )
	#set_property( TARGET svgame PROPERTY CXX_STANDARD_REQUIRED ON )

	# We always have a config.h in use.
	target_compile_definitions( svgame PUBLIC HAVE_CONFIG_H=1 )

	# Set the flag for SharedGame to build with ServerGame compatability in mind.
	target_compile_definitions( svgame PUBLIC "SHAREDGAME_SERVERGAME=1" )


	####
	##	Hook up with /extern/ libs.
	####
	# GLM Lib Includes.
	target_include_directories( svgame PRIVATE "${GLM_INCLUDE_DIRS}" )

	# Link libraries.
	target_link_libraries( 
		svgame 
		PUBLIC 
		fmt::fmt-header-only 
		glm
	)

	####
	##	Target Properties. 
	####
	# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
	set_target_properties(svgame
		PROPERTIES
		OUTPUT_NAME "svgame"
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