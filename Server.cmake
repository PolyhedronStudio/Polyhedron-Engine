#############################################################################
##
##
##	Polyhedron Project: Server
##
##
#############################################################################

################################################################################################
##  Polyhedron Server: Compiles into a binary together with the server for local gameplay.    ##
################################################################################################
####
## CMake Project and version number
####
project(target-PolyhedronServer VERSION "${POLYHEDRON_VERSION_MAJOR}.${POLYHEDRON_VERSION_MINOR}.${POLYHEDRON_VERSION_POINT}" )
# Add executable.
add_executable(	PolyhedronServer 
	${SRC_SYSTEM_DIR}/Resources/polyhedron_dedicated.rc
)

# Add "Warn All" flag.
#target_compile_options(PolyhedronServer -Wall)
# Use C++20.
#set_property(TARGET PolyhedronServer PROPERTY CXX_STANDARD 20)


# Setup all Server target Sources.
target_sources( PolyhedronServer PRIVATE 
	${SRC_SERVER} ${HEADERS_SERVER}
)
target_sources( PolyhedronServer PUBLIC 
	${SRC_CLIENT_DIR}/Null.cpp
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_SYSTEM} ${HEADERS_SYSTEM}
	${SRC_COMMON} ${HEADERS_COMMON}
)

# Add include directories.
target_include_directories( PolyhedronServer INTERFACE 
	"${SRC_ROOT_DIR}"
)
target_include_directories( PolyhedronServer PRIVATE 
	"${SRC_ROOT_DIR}"
)

# Embed version numbers.
target_compile_definitions( PolyhedronServer PRIVATE 
	VERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR}
	VERSION_MINOR=${POLYHEDRON_VERSION_MINOR}
	VERSION_POINT=${POLYHEDRON_VERSION_POINT}
	VERSION_SHA=${POLYHEDRON_VERSION_SHA}
	VERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH}
)

# We always have a config.h in use.
target_compile_definitions( PolyhedronServer PRIVATE _CRT_SECURE_NO_WARNINGS )

# Operating System specific options.
if( WIN32 ) 

	# Add /VC/inc include directory.
	target_include_directories( PolyhedronServer PRIVATE "${CMAKE_SOURCE_DIR}/VC/inc" )

	# Link to winmm and ws2_32
	target_link_libraries( PolyhedronServer PUBLIC winmm ws2_32 )

	# Set VC Debugger Working Directory.
	set_target_properties( PolyhedronServer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin" )
	# Set Win32 specific target compile options.
	# Disable double define warnings: wd4005	
	target_compile_options( PolyhedronServer PRIVATE /wd4005)
	# Disable [[deprecation]] like warnings: wd4996
	target_compile_options( PolyhedronServer PRIVATE /wd4996)

else()
	target_link_libraries( PolyhedronServer PUBLIC dl rt m pthread )
endif()

# Add 'public' C++20 requirement.
target_compile_features( PolyhedronServer PUBLIC "cxx_std_20" )
# Require C++20 Support.
set_property(TARGET PolyhedronServer PROPERTY CXX_STANDARD_REQUIRED ON )

# We always have a config.h in use.
# We always have a refresh module in place.
target_compile_definitions( PolyhedronServer PUBLIC "HAVE_CONFIG_H=1" )
target_compile_definitions( PolyhedronServer PUBLIC "USE_REF=0" )

# Compile without client, set USE_SERVER to 1 and USE_CLIENT to 0
target_compile_definitions( PolyhedronServer PUBLIC "USE_SERVER=1" )
target_compile_definitions( PolyhedronServer PUBLIC "USE_CLIENT=0" )


####
##	Hook up with /extern/ libs.
####
# ZLib Includes.
target_include_directories( PolyhedronServer PRIVATE "${ZLIB_INCLUDE_DIRS}" )
# STB Lib Includes
target_include_directories( PolyhedronServer PRIVATE "/extern/stb/" )
# GLM Lib Includes.
target_include_directories( PolyhedronServer PRIVATE "${GLM_INCLUDE_DIRS}" )

# Link libraries.
target_link_libraries( 
	PolyhedronServer 
	PUBLIC 
#	SDL2main 
	SDL2-static 
	zlibstatic 
	fmt::fmt-header-only 
	glm
)


####
##	Set Target properties.
####
set_target_properties( PolyhedronServer 
    PROPERTIES
    OUTPUT_NAME "PolyhedronDedicated"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
    DEBUG_POSTFIX ""
)
