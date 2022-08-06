#############################################################################
##
##
##	Polyhedron Project: Client
##
##
#############################################################################

################################################################################################
##  Polyhedron Client: Compiles into a binary together with the server for local gameplay.    ##
################################################################################################
####
## CMake Project and version number
####
project(target-PolyhedronClient VERSION "${POLYHEDRON_VERSION_MAJOR}.${POLYHEDRON_VERSION_MINOR}.${POLYHEDRON_VERSION_POINT}" )
# Add executable.
add_executable( PolyhedronClient WIN32 
	${SRC_SYSTEM_DIR}/Resources/polyhedron.rc
)

# Add "Warn All" flag.
#target_compile_options(PolyhedronClient -Wall)
# Use C++20.
#set_property(TARGET PolyhedronClient PROPERTY CXX_STANDARD 20)

# Setup all Client target Sources.
target_sources( PolyhedronClient PRIVATE 
	${SRC_CLIENT} ${HEADERS_CLIENT}
)
target_sources( PolyhedronClient PUBLIC 
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_SYSTEM} ${SRC_SYSTEM_CLIENT} ${HEADERS_SYSTEM}
	${SRC_COMMON} ${HEADERS_COMMON}
	${SRC_REFRESH} ${HEADERS_REFRESH}
	${SRC_SERVER} ${HEADERS_SERVER}
)

# Add in GL Refresh Module if enabled.
if( CONFIG_BUILD_GL_RENDERER )

	target_sources( PolyhedronClient PUBLIC 
		${SRC_REFRESH_GL} ${HEADERS_REFRESH_GL}
	)

endif()
# Add in VKPT Refresh Module if enabled.
if( CONFIG_BUILD_VKPT_RENDERER )

	target_sources( PolyhedronClient PUBLIC 
		${SRC_REFRESH_VKPT} ${HEADERS_REFRESH_VKPT}
	)

endif()

####
##  If Vulkan Path Tracing is enabled, look for the SpirV compiler and compile all glsl shaders.
####
if (CONFIG_BUILD_VKPT_RENDERER)
	include(cmake/compileShaders.cmake)
	foreach(s ${SRC_RT_SHADERS})
		get_filename_component(shader_name ${s} NAME)
		get_filename_component(shader_ext ${s} EXT)
		compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.pipeline")
		if (shader_ext STREQUAL ".rgen")
			compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.query" DEFINES "-DKHR_RAY_QUERY" STAGE "comp")
		endif()
	endforeach()

	foreach(s ${SRC_SHADERS})
		compile_shader(SOURCE_FILE ${s} INCLUDES "-I${SRC_ROOT_DIR}/Refresh/vkpt/fsr" OUTPUT_FILE_LIST shader_bytecode)
	endforeach()

	add_custom_target(shaders DEPENDS ${shader_bytecode})

	if(TARGET glslangValidator)
		add_dependencies(shaders glslangValidator)
	endif()
endif()

####
##	Error in case there is no refresh modules enabled.
####
if ( NOT CONFIG_BUILD_VKPT_RENDERER AND NOT CONFIG_BUILD_GL_RENDERER )
	message( FATAL_ERROR "Trying to build the Client without either GL or VKPT renderer enabled." )
endif ()


#target_sources( PolyhedronClient PRIVATE 

	#${SRC_SERVER} ${HEADERS_SERVER}
#)
# Add include directories.
#target_include_directories( PolyhedronClient INTERFACE 
#	"${SRC_CLIENT_DIR}"
#	"${SRC_SERVER_DIR}"
#	"${SRC_COMMON_DIR}"
#	"${SRC_REFRESH_DIR}"
#	"${SRC_SHARED_DIR}"
#	"${SRC_SYSTEM_DIR}"
#)
#target_include_directories( PolyhedronClient PRIVATE 
#	"${SRC_CLIENT_DIR}"
#	"${SRC_SERVER_DIR}"
#	"${SRC_COMMON_DIR}"
#	"${SRC_REFRESH_DIR}"
#	"${SRC_SHARED_DIR}"
#	"${SRC_SYSTEM_DIR}"
#)

# Embed version numbers.
target_compile_definitions( PolyhedronClient PRIVATE 
	VERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR}
	VERSION_MINOR=${POLYHEDRON_VERSION_MINOR}
	VERSION_POINT=${POLYHEDRON_VERSION_POINT}
	VERSION_SHA=${POLYHEDRON_VERSION_SHA}
	VERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH}
)

# We always have a config.h in use.
target_compile_definitions( PolyhedronClient PRIVATE _CRT_SECURE_NO_WARNINGS )

# Operating System specific options.
if( WIN32 ) 

	# Add /VC/inc include directory.
	target_include_directories( PolyhedronClient PRIVATE "${CMAKE_SOURCE_DIR}/VC/inc" )
	
	# Link to winmm and ws2_32
	target_link_libraries( PolyhedronClient PUBLIC winmm ws2_32 )
	
	# Set VC Debugger Working Directory.
	set_target_properties( PolyhedronClient PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin" )

	# Set Win32 specific target compile options.
	# Disable double define warnings: wd4005	
	target_compile_options( PolyhedronClient PRIVATE /wd4005)
	# Disable [[deprecation]] like warnings: wd4996
	target_compile_options( PolyhedronClient PRIVATE /wd4996)

else()
	
endif()

# Add 'public' C++20 requirement.
target_compile_features( PolyhedronClient PRIVATE "cxx_std_20" )
# Require C++20 Support.
set_property(TARGET PolyhedronClient PROPERTY CXX_STANDARD_REQUIRED ON)

# We always have a config.h in use.
# We always have a refresh module in place.
target_compile_definitions( PolyhedronClient PUBLIC "HAVE_CONFIG_H=1" )
target_compile_definitions( PolyhedronClient PUBLIC "USE_REF=1" )
# Compile for both, Client and Server in one support.
target_compile_definitions( PolyhedronClient PRIVATE "USE_CLIENT=1" )
target_compile_definitions( PolyhedronClient PRIVATE "USE_SERVER=1" )


# Ensure the shader target gets compiled first.
if ( GLSLANG_COMPILER AND CONFIG_BUILD_VKPT_RENDERER )

	add_dependencies( PolyhedronClient shaders )

endif()

# CURL Support Build Configuration.
if( CONFIG_USE_CURL )

	target_compile_definitions( PolyhedronClient PUBLIC "USE_CURL=1" )

	target_compile_definitions( PolyhedronClient PUBLIC "CURL_STATICLIB" )

	target_link_libraries( PolyhedronClient PUBLIC libcurl )

endif( )

# RMLUI Build Configuration.
if( CONFIG_ENABLE_RMLUI )

	target_include_directories( PolyhedronClient PRIVATE ${CMAKE_SOURCE_DIR}/extern/Nac-RmlUi/Include/ )
	target_compile_definitions( PolyhedronClient PUBLIC "USE_RMLUI=1" )

	target_compile_definitions( PolyhedronClient PUBLIC "RMLUI_STATIC_LIB" )

	target_link_libraries( PolyhedronClient PUBLIC RmlCore RmlDebugger RmlLua )

endif ( )

# GL 1.x renderer Build Configuration.
if( CONFIG_BUILD_GL_RENDERER )

	target_compile_definitions( PolyhedronClient PUBLIC "REF_GL=1" )

endif()

# VKPT Renderer Build Configuration
if( CONFIG_BUILD_VKPT_RENDERER )

	#TARGET_SOURCES(client PRIVATE ${SRC_VKPT} ${HEADERS_VKPT})
	target_include_directories( PolyhedronClient PRIVATE ${CMAKE_SOURCE_DIR}/extern/Vulkan-Headers/include )
	target_link_directories( PolyhedronClient PRIVATE ${CMAKE_SOURCE_DIR} )
	target_compile_definitions( PolyhedronClient PUBLIC "REF_VKPT=1" )
	
	if( CONFIG_VKPT_ENABLE_DEVICE_GROUPS )
		target_compile_definitions( PolyhedronClient PRIVATE "VKPT_DEVICE_GROUPS=1" )
	endif()
	if( CONFIG_VKPT_ENABLE_IMAGE_DUMPS )
		target_compile_definitions( PolyhedronClient PRIVATE "VKPT_IMAGE_DUMPS=1" )
	endif()
	if( WIN32 )
		target_link_libraries( PolyhedronClient PRIVATE vulkan-1 )
	else()
		target_link_libraries( PolyhedronClient PRIVATE vulkan )
	endif()

endif()

# Add include directories.
target_include_directories( PolyhedronClient INTERFACE 
	"${SRC_ROOT_DIR}"
)
target_include_directories( PolyhedronClient PRIVATE 
	"${SRC_ROOT_DIR}"
)
#target_include_directories( PolyhedronClient INTERFACE
#	"${SRC_ROOT_DIR}"
#)
#target_include_directories( PolyhedronClient PRIVATE 
#	"${SRC_ROOT_DIR}"
#)

####
##	Hook up with /extern/ libs.
####
# ZLib Includes.
target_include_directories( PolyhedronClient PRIVATE "${ZLIB_INCLUDE_DIRS}")
# STB Lib Includes
target_include_directories( PolyhedronClient PRIVATE "${CMAKE_SOURCE_DIR}/extern/stb/")
# GLM Lib Includes.
target_include_directories( PolyhedronClient PRIVATE "${GLM_INCLUDE_DIRS}")

# Link libraries.
target_link_libraries( 
	PolyhedronClient 
	PRIVATE 
	SDL2main 
	SDL2-static 
	zlibstatic 
	stb 
	tinyobjloader 
	fmt::fmt-header-only 
	glm
)


####
##	Set Target properties.
####
set_target_properties( PolyhedronClient 
    PROPERTIES
    OUTPUT_NAME "PolyhedronClient"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
    DEBUG_POSTFIX ""
)
