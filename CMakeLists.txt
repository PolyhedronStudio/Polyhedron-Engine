cmake_minimum_required (VERSION 3.9)
cmake_policy(SET CMP0069 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(Polyhedron)
set(POLYHEDRON_VERSION_MAJOR 0)
set(POLYHEDRON_VERSION_MINOR 4)
set(POLYHEDRON_VERSION_POINT 0)

# CPP Version
if(WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

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


OPTION(CONFIG_BUILD_CL_GAME "Build Client Game module" ON)
OPTION(CONFIG_BUILD_SV_GAME "Build Server Game module" ON)
OPTION(CONFIG_ENABLE_RMLUI "Enable libRmlUI implementation" OFF)
OPTION(CONFIG_GL_RENDERER "Enable GL renderer" ON)
OPTION(CONFIG_VKPT_RENDERER "Enable VKPT renderer" ON)
OPTION(CONFIG_VKPT_ENABLE_DEVICE_GROUPS "Enable device groups (multi-gpu) support" ON) 
OPTION(CONFIG_VKPT_ENABLE_IMAGE_DUMPS "Enable image dumping functionality" ON) 
OPTION(CONFIG_BUILD_GLSLANG "Build glslangValidator from source instead of using the SDK" OFF)
OPTION(CONFIG_USE_CURL "Use CURL for HTTP support" ON)

# WATISDEZE: Do we still need these?
# OPTION(CONFIG_LINUX_PACKAGING_SUPPORT "Enable Linux Packaging support" OFF)
# OPTION(CONFIG_LINUX_STEAM_RUNTIME_SUPPORT "Enable Linux Steam Runtime support" OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------- Setup output Directories -------------------------
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

# --------- Setup the Executable output Directory -------------
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Bin CACHE PATH "Single Directory for all Executables.")
   
# --------- Dependencies -------------------------------------
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set( IS_64_BIT 1 )
endif ()

IF(WIN32)
    # From: gheshu
    # /MP: multi-process compilation
    set(CMAKE_CPP_FLAGS "${CMAKE_C_FLAGS} /MP")
    # /MD: release dynamic linkage runtime (the default, encouraged by GPL)
    set(CMAKE_CPP_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /MD")
    set(CMAKE_CPP_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_CPP_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MD")
ENDIF()

add_subdirectory(extern)

link_directories(.)

# WATISDEZE: Do we still need this?
# set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)

add_subdirectory(src)

# IF(CONFIG_LINUX_PACKAGING_SUPPORT)
#     # https://cmake.org/cmake/help/v3.8/module/CPackDeb.html
#     set(CPACK_GENERATOR "DEB")
#     set(CPACK_PACKAGE_NAME "quake2rtx")
#     set(CPACK_PACKAGE_VENDOR "NVIDIA Corporation")
#     set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "RTX Remaster of Quake 2")
#     set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")
#     set(CPACK_PACKAGE_VERSION_MAJOR ${POLYHEDRON_VERSION_MAJOR})
#     set(CPACK_PACKAGE_VERSION_MINOR ${POLYHEDRON_VERSION_MINOR})
#     set(CPACK_PACKAGE_VERSION_PATCH ${POLYHEDRON_VERSION_POINT})
#     set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#     set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
#     set(CPACK_DEBIAN_PACKAGE_SECTION "games")

#     set(CPACK_PACKAGE_CONTACT "http://nvidia.com/object/support.html")

#     set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.nvidia.com/en-us/geforce/news/quake-ii-rtx-ray-tracing-vulkan-vkray-geforce-rtx/")

#     set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopenal1, libvulkan1")

#     set(CPACK_DEB_PACKAGE_COMPONENT ON)
#     set(CPACK_DEB_COMPONENT_INSTALL ON)
#     set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

#     # Mark as shareware to be clear this is demo levels + RTX
#     set(CPACK_COMPONENTS_ALL shareware)

#     include(CPack)
# ENDIF()
