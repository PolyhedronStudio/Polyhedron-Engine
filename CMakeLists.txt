################################################################################################
##  Root project folder CMake file.                                                           ##
##                                                                                            ##
##	The root CMakeLists.txt to rule them all.                                                 ##  
################################################################################################
####
##	CMake Minimum Requirement
##  64 bit check
##  Set cmake policies.
##  Turn on CMAKE_EXPORT_COMPILE_COMMANDS
####
# Set the minimum requirement to CMake 3.9
cmake_minimum_required ( VERSION 3.9 )

# Disable Interprocedurla Optimization compilation. (Was a default before CMake 3.9)
cmake_policy( SET CMP0069 NEW )

# Set version number.
set( POLYHEDRON_VERSION_MAJOR 0)
set( POLYHEDRON_VERSION_MINOR 4)
set( POLYHEDRON_VERSION_POINT 5)

# Set C/C++ Standards:
if( WIN32 )
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
else()
	#set(CMAKE_CXX_STANDARD 20)
	#set(CMAKE_CXX_STANDARD_REQUIRED ON)
	#set(CMAKE_CXX_EXTENSIONS OFF)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

# Make sure we are building for 64 bit systems.
if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( IS_64_BIT 1 )
else()
	message( FATAL_ERROR "CMAKE_SIZEOF_VOID_P does not EQUAL 8. Polyhedron requires a build configuration with 64 bit support." )
endif ()

# Enable exporting all compile commands.
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
# Enable Verbose Building.
set( CMAKE_VERBOSE_MAKEFILE ON )

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build(Default: Release), options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()


####
##	Append module path with <project-root>/cmake for Spir-V VKPT shader compiling.
####
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )


####
##   Repository Branch & Version Number. 
####
# get short-hash
execute_process(
	COMMAND git rev-parse --short HEAD
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	OUTPUT_VARIABLE POLYHEDRON_VERSION_SHA
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# get branch name
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE POLYHEDRON_VERSION_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)


####
##   Build Options and Configuration.
####
# Options, description already does the commenting job.
option( CONFIG_BUILD_GAME_CLIENT "Build ClientGame module" ON )
option( CONFIG_BUILD_GAME_SERVER "Build ServerGame module" ON )
option( CONFIG_BUILD_GL_RENDERER "Enable GL renderer" ON )
option( CONFIG_BUILD_VKPT_RENDERER "Enable VKPT renderer" ON )
option( CONFIG_BUILD_GLSLANG "Build glslangValidator from source instead of using the SDK" OFF )
option( CONFIG_VKPT_ENABLE_DEVICE_GROUPS "VKPT: Enable device groups (multi-gpu) support" ON ) 
option( CONFIG_VKPT_ENABLE_IMAGE_DUMPS "Enable image dumping functionality" ON ) 
option( CONFIG_ENABLE_RMLUI "Enable libRmlUI implementation" OFF )
option( CONFIG_USE_CURL "Use CURL for HTTP support" ON )

# Enable USE_FOLDERS CMake feature, it will generate a proper folder hierachy for Visual Studio users.
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

# Set CMake Library output directory to <project-root>/bin
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   "${PROJECT_BINARY_DIR}/Bin"
   CACHE PATH
   "Single Directory for all Libraries."
)
# Set CMake Binary output directory to <project-root>/bin
set(
	CMAKE_RUNTIME_OUTPUT_DIRECTORY 
	${PROJECT_BINARY_DIR}/Bin 
	CACHE PATH 
	"Single Directory for all Binaries."
)
# Add current folder to linkdirectories.
if (WIN32)
	# TODO: Presumably for vulkan-1.lib? Hmmm.
	link_directories(.)
endif()


####
##	Start adding our project subdirectories.
####
# Add the extern folder as a subdirectory in order for its CMakeLists.txt to add all extern libraries.
add_subdirectory(extern)
# Add our source folder CMakeLists.txt to continue the job.
#add_subdirectory(src)


####
##  Configure the source paths using PARENT_SCOPE method.
####
set( SRC_PROJECT_DIR ${CMAKE_SOURCE_DIR}../ )
set( SRC_PROJECT_DIR ${SRC_PROJECT_DIR} ) #PARENT_SCOPE )

# Root ( Containing all source subfolders ):
set( SRC_ROOT_DIR ${CMAKE_SOURCE_DIR}/src )
set( SRC_ROOT_DIR ${SRC_ROOT_DIR} ) #PARENT_SCOPE )

# Client Code:
set( SRC_CLIENT_DIR ${SRC_ROOT_DIR}/Client )
set( SRC_CLIENT_DIR ${SRC_CLIENT_DIR} ) #PARENT_SCOPE )

# Server Code:
set( SRC_SERVER_DIR ${SRC_ROOT_DIR}/Server )
set( SRC_SERVER_DIR ${SRC_SERVER_DIR} ) #PARENT_SCOPE )

# Common Code (Shared between Client and Server, containing common engine logic):
set( SRC_COMMON_DIR ${SRC_ROOT_DIR}/Common )
set( SRC_COMMON_DIR ${SRC_COMMON_DIR} ) #PARENT_SCOPE )

# Refresh Code (General refresh API, and GL/VKPT implementations.)
set( SRC_REFRESH_DIR ${SRC_ROOT_DIR}/Refresh )
set( SRC_REFRESH_DIR ${SRC_REFRESH_DIR} ) #PARENT_SCOPE )

# Shared Code (Shared between Client, Server and all Game Modules, containing shared logic):
set( SRC_SHARED_DIR ${SRC_ROOT_DIR}/Shared )
set( SRC_SHARED_DIR ${SRC_SHARED_DIR} ) #PARENT_SCOPE )

# System Code (Operating system specific logic):
set( SRC_SYSTEM_DIR ${SRC_ROOT_DIR}/System )
set( SRC_SYSTEM_DIR ${SRC_SYSTEM_DIR} ) #PARENT_SCOPE )

# Game, ClientGame, ServerGame, SharedGame (Containing all game logics.)
set( SRC_GAME_DIR ${SRC_ROOT_DIR}/Game )
set( SRC_GAME_DIR ${SRC_GAME_DIR} ) #PARENT_SCOPE )
set( SRC_GAME_SHARED_DIR ${SRC_GAME_DIR}/Shared )
set( SRC_GAME_SHARED_DIR ${SRC_GAME_SHARED_DIR} ) #PARENT_SCOPE )
set( SRC_GAME_CLIENT_DIR ${SRC_GAME_DIR}/Client )
set( SRC_GAME_CLIENT_DIR ${SRC_GAME_CLIENT_DIR} ) #PARENT_SCOPE )
set( SRC_GAME_SERVER_DIR ${SRC_GAME_DIR}/Server )
set( SRC_GAME_SERVER_DIR ${SRC_GAME_SERVER_DIR} ) #PARENT_SCOPE )

####
##	Include Source/Header lists for all projects.
####
include(SourceList.cmake)

####
##	Client/Server build project script.
####
include(Client.cmake)
include(Server.cmake)

####
##	(Client/Server)-Game build project scripts.
####
include(ClientGame.cmake)
#include(ServerGame.cmake)
