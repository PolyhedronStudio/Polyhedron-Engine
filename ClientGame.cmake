############################################################################
##  Polyhedron ClientGame Module(.dll/.so):                               ##
##                                                                        ##
##  When CONFIG_BUILD_GAME_CLIENT is set the ClientGame Module builds as  ##
##  a shared library(.dll/.so) file taking along with itself the Shared-  ##
##  Game code and its ClientGame bindings.                                ##
############################################################################
if( CONFIG_BUILD_GAME_CLIENT )
#${SRC_SHARED} ${HEADERS_SHARED} ${SRC_CLGAME} ${HEADERS_CLGAME} )#$<TARGET_OBJECTS:clgame_sharedgame>) #${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})	
#${SRC_SHARED} ${HEADERS_SHARED} ${SRC_CLGAME} ${HEADERS_CLGAME} )#$<TARGET_OBJECTS:clgame_sharedgame>) #${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})

####
## CMake Project and version number
####
project(target-clgame VERSION "${POLYHEDRON_VERSION_MAJOR}.${POLYHEDRON_VERSION_MINOR}.${POLYHEDRON_VERSION_POINT}" )
# Add SHARED library.
if (WIN32)
	add_library( clgame SHARED ${SRC_SYSTEM_DIR}/Resources/basepoly_cl.rc )
else()
	add_library( clgame SHARED )
endif()

# Setup all Server target Sources.
target_sources( clgame PUBLIC
	#${SRC_GAME_CLIENT_DIR}/clgame.def
	${SRC_GAME_CLIENT} ${HEADERS_GAME_CLIENT}
	${SRC_GAME_SHARED_DIR}/GameBindings/ClientBinding.cpp
	${SRC_GAME_SHARED_DIR}/GameBindings/ClientBinding.h
	${SRC_GAME_SHARED} ${SRC_GAME_SHARED}
)
target_sources( clgame PUBLIC 
	${SRC_SHARED} ${HEADERS_SHARED}
)

# Add include directories.
target_include_directories( clgame INTERFACE 
	"${SRC_ROOT_DIR}"
)
target_include_directories( clgame PRIVATE 
	"${SRC_ROOT_DIR}"
)

# Embed version numbers.
target_compile_definitions( clgame PRIVATE 
	VERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR}
	VERSION_MINOR=${POLYHEDRON_VERSION_MINOR}
	VERSION_POINT=${POLYHEDRON_VERSION_POINT}
	VERSION_SHA=${POLYHEDRON_VERSION_SHA}
	VERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH}
)

# We always have a config.h in use.
target_compile_definitions( clgame PRIVATE _CRT_SECURE_NO_WARNINGS )

# Add 'public' C++20 requirement.
target_compile_features( clgame PUBLIC "cxx_std_20" )
# Require C++20 Support.
set_property( TARGET clgame PROPERTY CXX_STANDARD_REQUIRED ON )

# We always have a config.h in use.
target_compile_definitions( clgame PUBLIC HAVE_CONFIG_H=1 )

# Set the flag for SharedGame to build with ClientGame compatability in mind.
target_compile_definitions( clgame PUBLIC "SHAREDGAME_CLIENTGAME=1" )


####
##	Hook up with /extern/ libs.
####
# ZLib Includes.
#target_include_directories( PolyhedronServer PRIVATE "${ZLIB_INCLUDE_DIRS}" )
# STB Lib Includes
#target_include_directories( PolyhedronServer PRIVATE "/extern/stb/" )
# GLM Lib Includes.
target_include_directories( clgame PRIVATE "${GLM_INCLUDE_DIRS}" )

# Link libraries.
target_link_libraries( 
	clgame 
	PUBLIC 
	fmt::fmt-header-only 
	glm
)
####
##	Target Properties. 
####
MESSAGE( NOTICE "CLGAME SRC DIR = ${CMAKE_SOURCE_DIR}")
# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
set_target_properties(clgame
	PROPERTIES
	OUTPUT_NAME "clgame"
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