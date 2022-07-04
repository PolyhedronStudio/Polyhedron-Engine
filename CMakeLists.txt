cmake_minimum_required (VERSION 3.9)
cmake_policy(SET CMP0069 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
#   Polyhedron Version.
#
project(Polyhedron VERSION 0.4.0)
set(POLYHEDRON_VERSION_MAJOR 0)
set(POLYHEDRON_VERSION_MINOR 4)
set(POLYHEDRON_VERSION_POINT 0)

# CPP Version
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # From: gheshu
    # /MP: multi-process compilation
    set(CMAKE_CPP_FLAGS "${CMAKE_C_FLAGS} /MP")
    # /MD: release dynamic linkage runtime (the default, encouraged by GPL)
    set(CMAKE_CPP_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MD")
    set(CMAKE_CPP_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_CPP_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MD")
else()
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()



#
#   Git Repository Info.
#
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



#
#   Build Settings.
#
OPTION(CONFIG_BUILD_CL_GAME "Build Client Game module" ON)
OPTION(CONFIG_BUILD_SV_GAME "Build Server Game module" ON)
OPTION(CONFIG_ENABLE_RMLUI "Enable libRmlUI implementation" OFF)
OPTION(CONFIG_GL_RENDERER "Enable GL renderer" ON)
OPTION(CONFIG_VKPT_RENDERER "Enable VKPT renderer" ON)
OPTION(CONFIG_VKPT_ENABLE_DEVICE_GROUPS "VKPT: Enable device groups (multi-gpu) support" ON) 
OPTION(CONFIG_VKPT_ENABLE_IMAGE_DUMPS "Enable image dumping functionality" ON) 
OPTION(CONFIG_BUILD_GLSLANG "Build glslangValidator from source instead of using the SDK" OFF)
OPTION(CONFIG_USE_CURL "Use CURL for HTTP support" ON)

#
# CMake Configuration.
#
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries."
)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Bin CACHE PATH "Single Directory for all Binaries.")

#
# Base Build Configuration.
#
# Check whether we are building for 64 bit (We don't support 32 bit anyhow.)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( IS_64_BIT 1 )
endif ()

# Specific Windows build flags.
IF(WIN32)

ENDIF()

# Add externs folder and its CMakeLists.txt
add_subdirectory(extern)

# Add current folder to linkdirectories.
link_directories(.)


# Last but not least, add the source folder and its CMakeLists.txt
add_subdirectory(src)
