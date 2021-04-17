
#
# ZLIB
#

if (NOT CONFIG_LINUX_STEAM_RUNTIME_SUPPORT)
    add_subdirectory(zlib)
    target_include_directories(zlibstatic PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/zlib> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/zlib>)

    set_target_properties(zlib PROPERTIES FOLDER extern)
    set_target_properties(zlibstatic PROPERTIES FOLDER extern)
    set_target_properties(minigzip PROPERTIES FOLDER extern)
    set_target_properties(example PROPERTIES FOLDER extern)
endif()

#
# SDL 2
#

option(SDL_SHARED"" OFF)
option(SDL_STATIC"" ON)
option(SDL_SHARED_ENABLED_BY_DEFAULT OFF)

if (CONFIG_LINUX_STEAM_RUNTIME_SUPPORT)
	option(INPUT_TSLIB OFF)
endif()

add_subdirectory(NaC-SDL)

set_target_properties(SDL2main PROPERTIES FOLDER extern)
set_target_properties(SDL2-static PROPERTIES FOLDER extern)
set_target_properties(uninstall PROPERTIES FOLDER extern)


#
# CURL - On Windows it'll search for the extern/win64 library and header files.
#

if(CONFIG_USE_CURL)
    option(BUILD_CURL_EXE "" OFF)
    option(BUILD_SHARED_LIBS "" OFF)
    option(CURL_STATICLIB "" ON)
    option(BUILD_TESTING "" OFF)
    option(ENABLE_MANUAL "" OFF)
    option(ENABLE_DOCS "" OFF)
    option(HTTP_ONLY "" ON)
    option(CURL_ZLIB "" OFF)
    option(CMAKE_USE_OPENSSL "" OFF)
    set(CURL_CA_PATH "none" CACHE PATH "")

    set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Override CMAKE_DEBUG_POSTFIX, which curl sets to '-d'")
    
    set(ZLIB_DIR "{CMAKE_CURRENT_SOURCE_DIR}/zlib")

    add_subdirectory(NaC-curl)

    set_target_properties(libcurl PROPERTIES FOLDER extern)
endif()

#
# STB
#
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

#
# TinyObjLoader
#
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nac-tinyobjloader-c)

#
# OpenAL
#
if (UNIX)
	FIND_PACKAGE(OpenAL)
	if (NOT OPENAL_INCLUDE_DIR)
		message(FATAL_ERROR "Can't find OpenAL. Please install the libopenal-dev package.")
	endif()
endif()

#
# libRmlUI
#
if (CONFIG_ENABLE_RMLUI)
    # Shared libs.
    option(BUILD_SHARED_LIBS "" ON)

    # Lua bindings.
    option(BUILD_LUA_BINDINGS "" ON)

    set(CURL_CA_PATH "none" CACHE PATH "")

    add_subdirectory(Nac-RmlUi)

    set_target_properties(RmlCore PROPERTIES FOLDER extern)
    set_target_properties(RmlDebugger PROPERTIES FOLDER extern)
    set_target_properties(RmlLua PROPERTIES FOLDER extern)
endif()