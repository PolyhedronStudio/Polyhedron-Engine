###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
# Need this.
include_directories(${CMAKE_SOURCE_DIR}/inc)

###################### Client.
SET(SRC_CLIENT
	Client/ASCII.cpp
	Client/Console.cpp
	Client/Cinematic.cpp
	Client/ClientGame.cpp
	Client/CRC.cpp
	Client/Demo.cpp
	Client/Download.cpp
	Client/Entities.cpp
	Client/Input.cpp
	Client/Keys.cpp
	Client/Main.cpp
	Client/Parse.cpp
	Client/Precache.cpp
	Client/Predict.cpp
	Client/Refresh.cpp
	Client/Screen.cpp
	Client/View.cpp
	Client/UI/Demos.cpp
	Client/UI/Menu.cpp
	Client/UI/PlayerConfig.cpp
	Client/UI/PlayerModels.cpp
	Client/UI/Script.cpp
	Client/UI/Servers.cpp
	Client/UI/UI.cpp
	Client/Sound/DirectMemoryAccess.cpp
	Client/Sound/OpenAL.cpp
	Client/Sound/Main.cpp
	Client/Sound/Memory.cpp
	Client/Sound/Mix.cpp
	Client/Sound/Ogg.cpp
#	Client/Sound/QAL/fixed.c
	Client/Sound/QAL/Dynamic.cpp
)
# Required as a placeholder for certain Vulkan only related features.
if (NOT CONFIG_VKPT_RENDERER AND CONFIG_GL_RENDERER)
	set(SRC_CLIENT 
		${SRC_CLIENT}
		Client/NullGL.cpp
	)
endif()

if (CONFIG_ENABLE_RMLUI)
SET(SRC_CLIENT_RMLUI
	Client/RmlUI/Interfaces/FileInterface.cpp
	Client/RmlUI/Interfaces/FileInterface.h
	Client/RmlUI/Interfaces/RenderInterface.cpp
	Client/RmlUI/Interfaces/RenderInterface.h
	Client/RmlUI/Interfaces/SystemInterface.cpp
	Client/RmlUI/Interfaces/SystemInterface.h
	Client/RmlUI/RmlUI.cpp
	Client/RmlUI/RmlUI.h
)
else()
SET(SRC_CLIENT_RMLUI
	Client/RmlUI/RmlUINull.cpp
	Client/RmlUI/RmlUI.h
)
endif()

SET(SRC_CLIENT_HTTP
	Client/HTTP.cpp
)
SET(HEADERS_CLIENT
	Client/Client.h
	Client/UI/UI.h
	Client/Sound/Sound.h
	Client/Sound/QAL/Dynamic.h
	Client/Sound/QAL/Fixed.h
)

###################### Server.
SET(SRC_SERVER
	Server/Commands.cpp
	Server/Entities.cpp
	Server/SVGame.cpp
	Server/Init.cpp
	Server/Main.cpp

	Server/Send.cpp
	Server/User.cpp
	Server/World.cpp

	Server/Save.cpp
)
SET(HEADERS_SERVER
	Server/Server.h
)

###################### Common.
SET(SRC_COMMON
	Common/Bsp.cpp
	Common/Cmd.cpp
	Common/CModel.cpp
	Common/Common.cpp
	Common/CVar.cpp
	Common/Error.cpp
	Common/Field.cpp
	Common/Fifo.cpp
	Common/Files.cpp
	Common/MDFour.cpp
	Common/Msg.cpp
	Common/Prompt.cpp
	Common/SizeBuffer.cpp
	Common/Utilities.cpp
	Common/Zone.cpp

	Common/Hashes/Crc32.cpp

	Common/Net/NetChan.cpp
	Common/Net/Net.cpp
)
SET(HEADERS_COMMON
	Common/Net/INetNToP.h
	Common/Net/INetPToN.h
	Common/Net/Win.h
	
	Common/Hashes/Crc32.h
)

###################### Format.
#SET(SRC_FORMAT
#)
#SET(HEADERS_COMMON
#	../inc/Format/Bsp.h
#	../inc/Format/Iqm.h
#	../inc/Format/Md2.h
#	../inc/Format/Md3.h
#	../inc/Format/Pak.h
#	../inc/Format/Pcx.h
#	../inc/Format/Sp2.h
#	../inc/Format/Wal.h
#)

###################### Refresh.
SET(SRC_REFRESH
	Refresh/images.cpp
	Refresh/models.cpp
	Refresh/model_iqm.cpp
	Refresh/stb/stb.cpp
)

###################### GL.
SET(SRC_GL
	Refresh/gl/draw.cpp
	Refresh/gl/hq2x.cpp
	Refresh/gl/images.cpp
	Refresh/gl/main.cpp
	Refresh/gl/mesh.cpp
	Refresh/gl/models.cpp
	Refresh/gl/sky.cpp
	Refresh/gl/state.cpp
	Refresh/gl/surf.cpp
	Refresh/gl/tess.cpp
	Refresh/gl/world.cpp
	Refresh/gl/qgl/dynamic.cpp
#	Refresh/gl/qgl/fixed.c
)
SET(HEADERS_GL
	Refresh/gl/arbfp.h
	Refresh/gl/gl.h
)



#
#	Shared Folder.	
#
include(../cmake/Shared.cmake)

SET(SRC_SHARED
	${PROJECT_SOURCE_DIR}/src/Shared/Math/Plane.cpp
	${PROJECT_SOURCE_DIR}/src/Shared/Math/Vector3.cpp

	${PROJECT_SOURCE_DIR}/src/Shared/Math.cpp
	${PROJECT_SOURCE_DIR}/src/Shared/Shared.cpp
)

#
#	Unix
#
SET(SRC_UNIX
	OS/Unix/Hunk.cpp
	OS/Unix/System.cpp
	OS/Unix/TTY.cpp
)
SET(SRC_UNIX_CLIENT
	OS/SDL2/Sound.cpp
	OS/SDL2/Video.cpp
	OS/SDL2/Time.cpp
)

#
#	Windows
#
SET(SRC_WINDOWS
	OS/Windows/Debug.cpp
	OS/Windows/Hunk.cpp
	OS/Windows/System.cpp

	OS/SDL2/Time.cpp
)
SET(SRC_WINDOWS_CLIENT
	#OS/Windows/Wave.cpp
	OS/SDL2/Sound.cpp
	OS/SDL2/Video.cpp
)
SET(HEADERS_WINDOWS
	OS/Windows/WGL.h
	OS/Windows/GLImp.h
	OS/Windows/WinClient.h
	OS/Windows/Threads/Threads.h
)



#
#	Vulkan Path Tracer
#
SET(SRC_VKPT
	Refresh/vkpt/asvgf.cpp
	Refresh/vkpt/bloom.cpp
	Refresh/vkpt/bsp_mesh.cpp
	Refresh/vkpt/cameras.cpp
	Refresh/vkpt/conversion.cpp
	Refresh/vkpt/draw.cpp
	Refresh/vkpt/freecam.cpp
	Refresh/vkpt/fog.cpp
	Refresh/vkpt/fsr.cpp
	Refresh/vkpt/main.cpp
	Refresh/vkpt/material.cpp
	Refresh/vkpt/matrix.cpp
	Refresh/vkpt/mgpu.cpp
	Refresh/vkpt/models.cpp
	Refresh/vkpt/path_tracer.cpp
	Refresh/vkpt/physical_sky.cpp
	Refresh/vkpt/precomputed_sky.cpp
	Refresh/vkpt/profiler.cpp
	Refresh/vkpt/shadow_map.cpp
	Refresh/vkpt/textures.cpp
	Refresh/vkpt/tone_mapping.cpp
	Refresh/vkpt/transparency.cpp
	Refresh/vkpt/uniform_buffer.cpp
	Refresh/vkpt/vertex_buffer.cpp
	Refresh/vkpt/vk_util.cpp
	Refresh/vkpt/buddy_allocator.cpp
	Refresh/vkpt/device_memory_allocator.cpp
	Refresh/vkpt/god_rays.cpp
)
SET(HEADERS_VKPT
	Refresh/vkpt/fsr/ffx_a.h
	Refresh/vkpt/fsr/ffx_fsr1.h

    Refresh/vkpt/shader/constants.h
    Refresh/vkpt/shader/global_textures.h
    Refresh/vkpt/shader/global_ubo.h
    Refresh/vkpt/shader/god_rays_shared.h
    Refresh/vkpt/shader/light_lists.h
    Refresh/vkpt/shader/path_tracer_hit_shaders.h
    Refresh/vkpt/shader/path_tracer_rgen.h
    Refresh/vkpt/shader/path_tracer.h
    Refresh/vkpt/shader/sky.h
    Refresh/vkpt/shader/vertex_buffer.h

	Refresh/vkpt/vkpt.h
	Refresh/vkpt/vk_util.h
	Refresh/vkpt/buddy_allocator.h
	Refresh/vkpt/cameras.h
	Refresh/vkpt/conversion.h
	Refresh/vkpt/device_memory_allocator.h
	Refresh/vkpt/fog.h
	Refresh/vkpt/material.h
	Refresh/vkpt/physical_sky.h
	Refresh/vkpt/precomputed_sky.h
)

#
#	Vulkan Path Tracer Shaders
#
set(SRC_SHADERS
	Refresh/vkpt/shader/animate_materials.comp
	Refresh/vkpt/shader/god_rays_filter.comp
	Refresh/vkpt/shader/god_rays.comp
	Refresh/vkpt/shader/bloom_composite.comp
	Refresh/vkpt/shader/bloom_blur.comp
	Refresh/vkpt/shader/bloom_downscale.comp
	Refresh/vkpt/shader/compositing.comp
	Refresh/vkpt/shader/checkerboard_interleave.comp
	Refresh/vkpt/shader/asvgf_atrous.comp
	Refresh/vkpt/shader/asvgf_gradient_atrous.comp
	Refresh/vkpt/shader/asvgf_gradient_img.comp
	Refresh/vkpt/shader/asvgf_gradient_reproject.comp
	Refresh/vkpt/shader/asvgf_lf.comp
	Refresh/vkpt/shader/asvgf_taau.comp
	Refresh/vkpt/shader/asvgf_temporal.comp
	Refresh/vkpt/shader/instance_geometry.comp
	Refresh/vkpt/shader/normalize_normal_map.comp
	Refresh/vkpt/shader/tone_mapping_histogram.comp
	Refresh/vkpt/shader/tone_mapping_curve.comp
    Refresh/vkpt/shader/tone_mapping_apply.comp
	Refresh/vkpt/shader/physical_sky.comp
	Refresh/vkpt/shader/physical_sky_space.comp
	Refresh/vkpt/shader/shadow_map.vert
	Refresh/vkpt/shader/sky_buffer_resolve.comp
	Refresh/vkpt/shader/stretch_pic.frag
	Refresh/vkpt/shader/stretch_pic.vert
	Refresh/vkpt/shader/final_blit_lanczos.frag
	Refresh/vkpt/shader/final_blit.vert
	Refresh/vkpt/shader/fsr_easu_fp16.comp
	Refresh/vkpt/shader/fsr_easu_fp32.comp
	Refresh/vkpt/shader/fsr_rcas_fp16.comp
	Refresh/vkpt/shader/fsr_rcas_fp32.comp
)
set(SRC_RT_SHADERS
	Refresh/vkpt/shader/primary_rays.rgen
	Refresh/vkpt/shader/direct_lighting.rgen
	Refresh/vkpt/shader/indirect_lighting.rgen
	Refresh/vkpt/shader/path_tracer.rchit
	Refresh/vkpt/shader/path_tracer.rmiss
	Refresh/vkpt/shader/path_tracer_masked.rahit
	Refresh/vkpt/shader/path_tracer_particle.rahit
	Refresh/vkpt/shader/path_tracer_sprite.rahit
	Refresh/vkpt/shader/path_tracer_beam.rahit
	Refresh/vkpt/shader/path_tracer_beam.rint
	Refresh/vkpt/shader/path_tracer_explosion.rahit
	Refresh/vkpt/shader/reflect_refract.rgen
)

#
#	Check for a shader compiler.
#
if (CONFIG_VKPT_RENDERER)
	IF(IS_64_BIT)
		include(${CMAKE_SOURCE_DIR}/cmake/compileShaders.cmake)

		foreach(s ${SRC_RT_SHADERS})
			get_filename_component(shader_name ${s} NAME)
			get_filename_component(shader_ext ${s} EXT)
			compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.pipeline")
			if (shader_ext STREQUAL ".rgen")
				compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.query" DEFINES "-DKHR_RAY_QUERY" STAGE "comp")
			endif()
		endforeach()

		foreach(s ${SRC_SHADERS})
			compile_shader(SOURCE_FILE ${s} INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/Refresh/vkpt/fsr" OUTPUT_FILE_LIST shader_bytecode)
		endforeach()

		add_custom_target(shaders DEPENDS ${shader_bytecode})

		if(TARGET glslangValidator)
			add_dependencies(shaders glslangValidator)
		endif()
	ENDIF()
endif()

#
# Embed version number
#
ADD_DEFINITIONS(-DVERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR})
ADD_DEFINITIONS(-DVERSION_MINOR=${POLYHEDRON_VERSION_MINOR})
ADD_DEFINITIONS(-DVERSION_POINT=${POLYHEDRON_VERSION_POINT})
ADD_DEFINITIONS(-DVERSION_SHA=${POLYHEDRON_VERSION_SHA})
ADD_DEFINITIONS(-DVERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH})


ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()


#
#	ServerGame Module
#
if (CONFIG_BUILD_SV_GAME)
	add_subdirectory(Game/Server)
endif()


#
#	ClientGame Module
#
if (CONFIG_BUILD_CL_GAME)
	add_subdirectory(Game/Client)
endif()


#
#	SharedGame Module
#
if (CONFIG_BUILD_CL_GAME OR CONFIG_BUILD_SV_GAME)
	add_subdirectory(Game/Shared)
endif()


#
#	Add Executable (client, server)
#
IF(WIN32)
	#
	#
	#
	ADD_EXECUTABLE(client WIN32 
		${SRC_CLIENT} ${HEADERS_CLIENT} 
		${SRC_COMMON} ${HEADERS_COMMON} 
		${SRC_REFRESH} ${SRC_SHADERS} 
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_WINDOWS} ${SRC_WINDOWS_CLIENT} ${HEADERS_WINDOWS}
		${SRC_SERVER} ${HEADERS_SERVER}
		OS/Resources/polyhedron.rc
	)

	#
	#	Win32 Server.
	#
	ADD_EXECUTABLE(server 
		${SRC_COMMON} ${HEADERS_COMMON} 
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_WINDOWS} ${HEADERS_WINDOWS}
		${SRC_SERVER} ${HEADERS_SERVER}
		client/null.cpp
		#OS/sdl2/time.cpp
		OS/Resources/polyhedron_dedicated.rc
	)
ELSE()
	#
	#	Unix Client.
	#
	ADD_EXECUTABLE(client
		${SRC_CLIENT} ${HEADERS_CLIENT} 
		${SRC_COMMON} ${HEADERS_COMMON} 
		${SRC_REFRESH} ${SRC_SHADERS} 
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_UNIX} ${SRC_UNIX_CLIENT}
		${SRC_SERVER} ${HEADERS_SERVER}
	)

	#
	#	Unix Server.
	#
	ADD_EXECUTABLE(server
		${SRC_COMMON} ${HEADERS_COMMON} 
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_UNIX}
		${SRC_SERVER} ${HEADERS_SERVER}
		Client/Null.cpp
	)
ENDIF()


#
#	Target Compile Definitions.
#
TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_SERVER=1 USE_CLIENT=1)
TARGET_COMPILE_DEFINITIONS(server PRIVATE USE_SERVER=1 USE_CLIENT=0)


#
#	CURL Build Configuration.
#
IF(CONFIG_USE_CURL)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_HTTP})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_CURL=1)
	TARGET_LINK_LIBRARIES(client libcurl)
ENDIF()



#
#	RMLUI Build Configuration
#
IF(CONFIG_ENABLE_RMLUI)
 	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_RMLUI})
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${CMAKE_SOURCE_DIR}/extern/Nac-RmlUi/Include/)
	TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_RMLUI=1 RMLUI_STATIC_LIB)
	TARGET_LINK_LIBRARIES(client RmlCore RmlDebugger RmlLua)
ELSE()
	# These are set to supply the rmlui_null.cpp file.
	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_RMLUI})
ENDIF()



#
#	GLSLang Compiler
#
if (GLSLANG_COMPILER AND CONFIG_VKPT_RENDERER)
	add_dependencies(client shaders)
endif()

#
#	GL1.x Renderer Build Configuration
#
IF (CONFIG_GL_RENDERER)
	TARGET_SOURCES(client PRIVATE ${SRC_GL} ${HEADERS_GL})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE REF_GL=1 USE_REF=1)
ENDIF()

#
#	VKPT Renderer Build Configuration
#
IF (CONFIG_VKPT_RENDERER)
	TARGET_SOURCES(client PRIVATE ${SRC_VKPT} ${HEADERS_VKPT})
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${CMAKE_SOURCE_DIR}/extern/Vulkan-Headers/include)
	LINK_DIRECTORIES(client PRIVATE ${CMAKE_SOURCE_DIR})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE REF_VKPT=1 USE_REF=1)
	IF (CONFIG_VKPT_ENABLE_DEVICE_GROUPS)
		TARGET_COMPILE_DEFINITIONS(client PRIVATE VKPT_DEVICE_GROUPS)
	ENDIF()
	if (CONFIG_VKPT_ENABLE_IMAGE_DUMPS)
		TARGET_COMPILE_DEFINITIONS(client PRIVATE VKPT_IMAGE_DUMPS)
	ENDIF()
	IF (WIN32)
	TARGET_LINK_LIBRARIES(client vulkan-1)
	ELSE ()
	TARGET_LINK_LIBRARIES(client vulkan)
	ENDIF()
ENDIF()

#
#	Client Link Libraries.
#
TARGET_LINK_LIBRARIES(client stb)
TARGET_LINK_LIBRARIES(client tinyobjloader)



#
#	SOURCE GROUPING
#
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_COMMON})
#SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_COMMON})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SERVER})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SERVER})

# Source and header groups
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_REFRESH} ${SRC_VKPT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_VKPT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHADERS})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHARED})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SHARED})
if (WIN32)
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_WINDOWS})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_WINDOWS})
else()
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_UNIX})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_UNIX})
endif()
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES client/null.cpp )



#
#	WIN32 SPECIFICS
#
IF (WIN32)
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(svgame PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(clgame PRIVATE ../VC/inc)
	TARGET_SOURCES(svgame PRIVATE ${CMAKE_SOURCE_DIR}/src/OS/Resources/basepoly_sv.rc)
	TARGET_SOURCES(clgame PRIVATE ${CMAKE_SOURCE_DIR}/src/OS/Resources/basepoly_cl.rc)

	TARGET_LINK_LIBRARIES(client winmm ws2_32)
	TARGET_LINK_LIBRARIES(server winmm ws2_32)

	set_target_properties(client PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

	# macro redefinition, deprecation
	target_compile_options(client PRIVATE /wd4005 /wd4996)
	target_compile_options(server PRIVATE /wd4005 /wd4996)

    ## Group the resource files too, so they don't end up in the ugly "Source Files" folder in VS
    SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES 
        OS/Resources/polyhedron.rc 
        OS/Resources/polyhedron.rc )
ENDIF()



#
#	Client Build Configuration.
#
TARGET_INCLUDE_DIRECTORIES(client PUBLIC ../inc)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "/extern/stb/")
TARGET_LINK_LIBRARIES(client SDL2main SDL2-static zlibstatic)
SET_TARGET_PROPERTIES(client
    PROPERTIES
    OUTPUT_NAME "Polyhedron"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
    DEBUG_POSTFIX ""
)


#
#	Server Build Configuration.
#
#include_directories()
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(server PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_LINK_LIBRARIES(server SDL2main SDL2-static zlibstatic)
SET_TARGET_PROPERTIES(server
    PROPERTIES
    OUTPUT_NAME "Polyhedron_Dedicated"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
    DEBUG_POSTFIX ""
)
