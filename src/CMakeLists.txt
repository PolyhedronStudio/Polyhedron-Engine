###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
SET(PATH_SRC ${CMAKE_CURRENT_SOURCE_DIR})




#
#	Shared Folder.	
#
SET(SRC_SHARED
	${PATH_SRC}/Shared/Math/Plane.cpp
	${PATH_SRC}/Shared/Math/Vector3.cpp

	${PATH_SRC}/Shared/Math.cpp
	${PATH_SRC}/Shared/Shared.cpp
)

SET(HEADERS_SHARED
	# Formats.
	${PATH_SRC}/Shared/Formats/Bsp.h 
	${PATH_SRC}/Shared/Formats/Iqm.h 
	${PATH_SRC}/Shared/Formats/Md2.h 
	${PATH_SRC}/Shared/Formats/Md3.h 
	${PATH_SRC}/Shared/Formats/Pak.h 
	${PATH_SRC}/Shared/Formats/Pcx.h 
	${PATH_SRC}/Shared/Formats/Sp2.h 
	${PATH_SRC}/Shared/Formats/Wal.h 

	# Interfaces
	${PATH_SRC}/Shared/Interfaces/IClientGameExports.h
	${PATH_SRC}/Shared/Interfaces/IClientGameImports.h
	${PATH_SRC}/Shared/Interfaces/IServerGameExports.h
	${PATH_SRC}/Shared/Interfaces/IServerGameImports.h

	# Math lib.
	${PATH_SRC}/Shared/Math/Color.h
	${PATH_SRC}/Shared/Math/Matrix3.h
	${PATH_SRC}/Shared/Math/Matrix4.h
	${PATH_SRC}/Shared/Math/Plane.h
	${PATH_SRC}/Shared/Math/Rectangle.h
	${PATH_SRC}/Shared/Math/Quaternion.h
	${PATH_SRC}/Shared/Math/Utilities.h
	${PATH_SRC}/Shared/Math/Vector2.h
	${PATH_SRC}/Shared/Math/Vector3.h
	${PATH_SRC}/Shared/Math/Vector4.h
	${PATH_SRC}/Shared/Math/Vector5.h
	
	# Other shared lib functionalities.
	${PATH_SRC}/Shared/CLGame.h
	${PATH_SRC}/Shared/CLTypes.h
	${PATH_SRC}/Shared/CollisionModel.h
	${PATH_SRC}/Shared/Common.h
	${PATH_SRC}/Shared/Config.h
	${PATH_SRC}/Shared/Endian.h 
	${PATH_SRC}/Shared/Entities.h 
	${PATH_SRC}/Shared/EntitySkeleton.h 
	${PATH_SRC}/Shared/Keys.h 
	${PATH_SRC}/Shared/KeyValue.h 
	${PATH_SRC}/Shared/List.h 
	${PATH_SRC}/Shared/Math.h 
	${PATH_SRC}/Shared/Messaging.h 
	${PATH_SRC}/Shared/Platform.h
	${PATH_SRC}/Shared/PlayerMove.h
	${PATH_SRC}/Shared/QString.h
	${PATH_SRC}/Shared/Refresh.h 
	${PATH_SRC}/Shared/Shared.h 
	${PATH_SRC}/Shared/SkeletalModelData.h 
	${PATH_SRC}/Shared/Sound.h 
	${PATH_SRC}/Shared/Strings.h
	${PATH_SRC}/Shared/SVGame.h 
	${PATH_SRC}/Shared/TickRate.h 
	${PATH_SRC}/Shared/UI.h 
)

SET(SRC_COMMON
	${PATH_SRC}/Common/Bsp.cpp
	${PATH_SRC}/Common/Cmd.cpp
	${PATH_SRC}/Common/CollisionModel.cpp
	${PATH_SRC}/Common/Common.cpp
	${PATH_SRC}/Common/CVar.cpp
	${PATH_SRC}/Common/EntitySkeleton.cpp
	${PATH_SRC}/Common/Error.cpp
	${PATH_SRC}/Common/Field.cpp
	${PATH_SRC}/Common/Fifo.cpp
	${PATH_SRC}/Common/Files.cpp
	${PATH_SRC}/Common/Huffman.cpp
	${PATH_SRC}/Common/MDFour.cpp
	${PATH_SRC}/Common/Messaging.cpp
	${PATH_SRC}/Common/Prompt.cpp
	${PATH_SRC}/Common/SizeBuffer.cpp
	${PATH_SRC}/Common/SkeletalModelData.cpp
	${PATH_SRC}/Common/TemporaryBoneCache.cpp
	#${PATH_SRC}/Common/Tests.cpp
	${PATH_SRC}/Common/Utilities.cpp
	${PATH_SRC}/Common/Zone.cpp

	${PATH_SRC}/Common/Models/Iqm.cpp

	${PATH_SRC}/Common/Hashes/Crc32.cpp

	${PATH_SRC}/Common/Messaging/MessageReadWrite.cpp
	${PATH_SRC}/Common/Messaging/ParseDeltaClientMoveCommand.cpp
	${PATH_SRC}/Common/Messaging/ParseDeltaEntityState.cpp
	${PATH_SRC}/Common/Messaging/ParseDeltaPlayerState.cpp
	${PATH_SRC}/Common/Messaging/WriteDeltaClientMoveCommand.cpp
	${PATH_SRC}/Common/Messaging/WriteDeltaEntityState.cpp
	${PATH_SRC}/Common/Messaging/WriteDeltaPlayerState.cpp

	${PATH_SRC}/Common/Net/NetChan.cpp
	${PATH_SRC}/Common/Net/Net.cpp
)
SET(HEADERS_COMMON
	${PATH_SRC}/Common/Bsp.h
	${PATH_SRC}/Common/Cmd.h
	${PATH_SRC}/Common/CollisionModel.h
	${PATH_SRC}/Common/Common.h
	${PATH_SRC}/Common/CVar.h
	${PATH_SRC}/Common/EntitySkeleton.h
	${PATH_SRC}/Common/Error.h
	${PATH_SRC}/Common/Field.h
	${PATH_SRC}/Common/Fifo.h
	${PATH_SRC}/Common/Files.h
	${PATH_SRC}/Common/HalfFloat.h
	${PATH_SRC}/Common/Huffman.h
	${PATH_SRC}/Common/MDFour.h
	${PATH_SRC}/Common/Messaging.h
	${PATH_SRC}/Common/PlayerMove.h
	${PATH_SRC}/Common/Prompt.h
	${PATH_SRC}/Common/Protocol.h
	${PATH_SRC}/Common/SizeBuffer.h
	${PATH_SRC}/Common/TemporaryBoneCache.h
	#${PATH_SRC}/Common/Tests.h
	${PATH_SRC}/Common/Utilities.h
	${PATH_SRC}/Common/Zone.h

	${PATH_SRC}/Common/Net/INetNToP.h
	${PATH_SRC}/Common/Net/INetPToN.h
	${PATH_SRC}/Common/Net/Win.h
	
	${PATH_SRC}/Common/Hashes/Crc32.h
)
#
#	Client Sources.
#
SET(SRC_CLIENT
	${PATH_SRC}/Client/ASCII.cpp
	${PATH_SRC}/Client/Console.cpp
	${PATH_SRC}/Client/Cinematic.cpp
	${PATH_SRC}/Client/ClientGame.cpp
	${PATH_SRC}/Client/CRC.cpp
	${PATH_SRC}/Client/Demo.cpp
	${PATH_SRC}/Client/Download.cpp
	${PATH_SRC}/Client/Entities.cpp
	${PATH_SRC}/Client/Input.cpp
	${PATH_SRC}/Client/Keys.cpp
	${PATH_SRC}/Client/Main.cpp
	${PATH_SRC}/Client/Models.cpp
	${PATH_SRC}/Client/Parse.cpp
	${PATH_SRC}/Client/Precache.cpp
	${PATH_SRC}/Client/Predict.cpp
	${PATH_SRC}/Client/Refresh.cpp
	${PATH_SRC}/Client/Screen.cpp
	${PATH_SRC}/Client/Traces.cpp
	${PATH_SRC}/Client/View.cpp
	${PATH_SRC}/Client/World.cpp

	${PATH_SRC}/Client/Entities/LocalEntities.cpp
	${PATH_SRC}/Client/Entities/PacketEntities.cpp

	${PATH_SRC}/Client/UI/Demos.cpp
	${PATH_SRC}/Client/UI/Menu.cpp
	${PATH_SRC}/Client/UI/PlayerConfig.cpp
	${PATH_SRC}/Client/UI/PlayerModels.cpp
	${PATH_SRC}/Client/UI/Script.cpp
	${PATH_SRC}/Client/UI/Servers.cpp
	${PATH_SRC}/Client/UI/UI.cpp

	${PATH_SRC}/Client/Sound/DirectMemoryAccess.cpp
	${PATH_SRC}/Client/Sound/OpenAL.cpp
	${PATH_SRC}/Client/Sound/Main.cpp
	${PATH_SRC}/Client/Sound/Memory.cpp
	${PATH_SRC}/Client/Sound/Mix.cpp
	${PATH_SRC}/Client/Sound/Ogg.cpp
	
	#${PATH_SRC}/Client/Sound/QAL/fixed.c
	${PATH_SRC}/Client/Sound/QAL/Dynamic.cpp
)
# Required as a placeholder for certain Vulkan only related features.
if (NOT CONFIG_VKPT_RENDERER AND CONFIG_GL_RENDERER)
	set(SRC_CLIENT 
		${SRC_CLIENT}
	${PATH_SRC}/Client/NullGL.cpp
	)
endif()

if (CONFIG_ENABLE_RMLUI)
	SET(SRC_CLIENT_RMLUI
		${PATH_SRC}/Client/RmlUI/Interfaces/FileInterface.cpp
		${PATH_SRC}/Client/RmlUI/Interfaces/FileInterface.h
		${PATH_SRC}/Client/RmlUI/Interfaces/RenderInterface.cpp
		${PATH_SRC}/Client/RmlUI/Interfaces/RenderInterface.h
		${PATH_SRC}/Client/RmlUI/Interfaces/SystemInterface.cpp
		${PATH_SRC}/Client/RmlUI/Interfaces/SystemInterface.h

		${PATH_SRC}/Client/RmlUI/RmlUI.cpp
		${PATH_SRC}/Client/RmlUI/RmlUI.h
	)
else()
	SET(SRC_CLIENT_RMLUI
		${PATH_SRC}/Client/RmlUI/RmlUINull.cpp
		${PATH_SRC}/Client/RmlUI/RmlUI.h
	)
endif()

SET(SRC_CLIENT_HTTP
	${PATH_SRC}/Client/HTTP.cpp
)
SET(HEADERS_CLIENT
	${PATH_SRC}/Client/Client.h
	#${PATH_SRC}/Client/ClientGame.h
	${PATH_SRC}/Client/Entities.h
	${PATH_SRC}/Client/GameModule.h
	${PATH_SRC}/Client/Input.h
	${PATH_SRC}/Client/Models.h
	${PATH_SRC}/Client/Keys.h
	${PATH_SRC}/Client/Traces.h
	${PATH_SRC}/Client/UI.h
	${PATH_SRC}/Client/Video.h
	${PATH_SRC}/Client/World.h

	${PATH_SRC}/Client/Entities/LocalEntities.h
	${PATH_SRC}/Client/Entities/PacketEntities.h

	${PATH_SRC}/Client/UI/UI.h

	${PATH_SRC}/Client/Sound/Sound.h
	
	${PATH_SRC}/Client/Sound/QAL/Dynamic.h
	${PATH_SRC}/Client/Sound/QAL/Fixed.h
)

###################### Server.
SET(SRC_SERVER
	${PATH_SRC}/Server/Commands.cpp
	${PATH_SRC}/Server/Entities.cpp
	${PATH_SRC}/Server/SVGame.cpp
	${PATH_SRC}/Server/Init.cpp
	${PATH_SRC}/Server/Main.cpp
	${PATH_SRC}/Server/Models.cpp
	${PATH_SRC}/Server/Send.cpp
	${PATH_SRC}/Server/User.cpp
	${PATH_SRC}/Server/World.cpp

	${PATH_SRC}/Server/Save.cpp
)
SET(HEADERS_SERVER
	${PATH_SRC}/Server/Server.h
)

###################### Common.


###################### Refresh.
SET(SRC_REFRESH
	${PATH_SRC}/Refresh/images.cpp
	${PATH_SRC}/Refresh/models.cpp
	#${PATH_SRC}/Refresh/model_iqm.cpp
	${PATH_SRC}/Refresh/stb/stb.cpp
)
SET(HEADERS_REFRESH
	${PATH_SRC}/Refresh/Images.h
	${PATH_SRC}/Refresh/Models.h
	${PATH_SRC}/Refresh/Refresh.h
)
###################### GL.
SET(SRC_GL
	${PATH_SRC}/Refresh/gl/draw.cpp
	${PATH_SRC}/Refresh/gl/hq2x.cpp
	${PATH_SRC}/Refresh/gl/images.cpp
	${PATH_SRC}/Refresh/gl/main.cpp
	${PATH_SRC}/Refresh/gl/mesh.cpp
	${PATH_SRC}/Refresh/gl/models.cpp
	${PATH_SRC}/Refresh/gl/sky.cpp
	${PATH_SRC}/Refresh/gl/state.cpp
	${PATH_SRC}/Refresh/gl/surf.cpp
	${PATH_SRC}/Refresh/gl/tess.cpp
	${PATH_SRC}/Refresh/gl/world.cpp
	${PATH_SRC}/Refresh/gl/qgl/dynamic.cpp
#	${PATH_SRC}/Refresh/gl/qgl/fixed.c
)
SET(HEADERS_GL
	${PATH_SRC}/Refresh/gl/arbfp.h
	${PATH_SRC}/Refresh/gl/gl.h
)


#
#	Unix
#
SET(SRC_UNIX
	System/Unix/Hunk.cpp
	System/Unix/System.cpp
	System/Unix/TTY.cpp
	System/SDL2/Time.cpp
)
SET(SRC_UNIX_CLIENT
	System/SDL2/Sound.cpp
	System/SDL2/Video.cpp
)

#
#	Windows
#
SET(SRC_WINDOWS
	System/Windows/Debug.cpp
	System/Windows/Hunk.cpp
	System/Windows/System.cpp

	System/SDL2/Time.cpp
)
SET(SRC_WINDOWS_CLIENT
	#System/Windows/Wave.cpp
	System/SDL2/Sound.cpp
	System/SDL2/Video.cpp
)
SET(HEADERS_WINDOWS
	System/Windows/WGL.h
	System/Windows/GLImp.h
	System/Windows/WinClient.h
	System/Windows/Threads/Threads.h
)



#
#	Vulkan Path Tracer
#
SET(SRC_VKPT
	${PATH_SRC}/Refresh/vkpt/asvgf.cpp
	${PATH_SRC}/Refresh/vkpt/bloom.cpp
	${PATH_SRC}/Refresh/vkpt/bsp_mesh.cpp
	${PATH_SRC}/Refresh/vkpt/cameras.cpp
	${PATH_SRC}/Refresh/vkpt/conversion.cpp
	${PATH_SRC}/Refresh/vkpt/draw.cpp
	${PATH_SRC}/Refresh/vkpt/freecam.cpp
	${PATH_SRC}/Refresh/vkpt/fog.cpp
	${PATH_SRC}/Refresh/vkpt/fsr.cpp
	${PATH_SRC}/Refresh/vkpt/main.cpp
	${PATH_SRC}/Refresh/vkpt/material.cpp
	${PATH_SRC}/Refresh/vkpt/matrix.cpp
	${PATH_SRC}/Refresh/vkpt/mgpu.cpp
	${PATH_SRC}/Refresh/vkpt/models.cpp
	${PATH_SRC}/Refresh/vkpt/path_tracer.cpp
	${PATH_SRC}/Refresh/vkpt/physical_sky.cpp
	${PATH_SRC}/Refresh/vkpt/precomputed_sky.cpp
	${PATH_SRC}/Refresh/vkpt/profiler.cpp
	${PATH_SRC}/Refresh/vkpt/shadow_map.cpp
	${PATH_SRC}/Refresh/vkpt/textures.cpp
	${PATH_SRC}/Refresh/vkpt/tone_mapping.cpp
	${PATH_SRC}/Refresh/vkpt/transparency.cpp
	${PATH_SRC}/Refresh/vkpt/uniform_buffer.cpp
	${PATH_SRC}/Refresh/vkpt/vertex_buffer.cpp
	${PATH_SRC}/Refresh/vkpt/vk_util.cpp
	${PATH_SRC}/Refresh/vkpt/buddy_allocator.cpp
	${PATH_SRC}/Refresh/vkpt/device_memory_allocator.cpp
	${PATH_SRC}/Refresh/vkpt/god_rays.cpp
)
SET(HEADERS_VKPT
	${PATH_SRC}/Refresh/vkpt/fsr/ffx_a.h
	${PATH_SRC}/Refresh/vkpt/fsr/ffx_fsr1.h

    ${PATH_SRC}/Refresh/vkpt/shader/constants.h
    ${PATH_SRC}/Refresh/vkpt/shader/global_textures.h
    ${PATH_SRC}/Refresh/vkpt/shader/global_ubo.h
    ${PATH_SRC}/Refresh/vkpt/shader/god_rays_shared.h
    ${PATH_SRC}/Refresh/vkpt/shader/light_lists.h
    ${PATH_SRC}/Refresh/vkpt/shader/path_tracer_hit_shaders.h
    ${PATH_SRC}/Refresh/vkpt/shader/path_tracer_rgen.h
    ${PATH_SRC}/Refresh/vkpt/shader/path_tracer.h
    ${PATH_SRC}/Refresh/vkpt/shader/sky.h
    ${PATH_SRC}/Refresh/vkpt/shader/vertex_buffer.h

	${PATH_SRC}/Refresh/vkpt/vkpt.h
	${PATH_SRC}/Refresh/vkpt/vk_util.h
	${PATH_SRC}/Refresh/vkpt/buddy_allocator.h
	${PATH_SRC}/Refresh/vkpt/cameras.h
	${PATH_SRC}/Refresh/vkpt/conversion.h
	${PATH_SRC}/Refresh/vkpt/device_memory_allocator.h
	${PATH_SRC}/Refresh/vkpt/fog.h
	${PATH_SRC}/Refresh/vkpt/material.h
	${PATH_SRC}/Refresh/vkpt/physical_sky.h
	${PATH_SRC}/Refresh/vkpt/precomputed_sky.h
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
		${SRC_REFRESH} ${SRC_SHADERS} ${HEADERS_REFRESH}
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_WINDOWS} ${SRC_WINDOWS_CLIENT} ${HEADERS_WINDOWS}
		${SRC_SERVER} ${HEADERS_SERVER}
		System/Resources/polyhedron.rc
	)

	#
	#	Win32 Server.
	#
	ADD_EXECUTABLE(server 
		${SRC_COMMON} ${HEADERS_COMMON} 
		${SRC_SHARED} ${HEADERS_SHARED}
		${SRC_WINDOWS} ${HEADERS_WINDOWS}
		${SRC_SERVER} ${HEADERS_SERVER}

		${PATH_SRC}/Client/null.cpp

		#System/sdl2/time.cpp
		System/Resources/polyhedron_dedicated.rc
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
		
		${PATH_SRC}/Client/Null.cpp
	)
ENDIF()


#
#	Target Compile Definitions.
#
TARGET_COMPILE_DEFINITIONS(client PRIVATE cxx_std_20 USE_SERVER=1 USE_CLIENT=1)
TARGET_COMPILE_DEFINITIONS(server PRIVATE cxx_std_20 USE_SERVER=1 USE_CLIENT=0)


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
	TARGET_SOURCES(svgame PRIVATE ${CMAKE_SOURCE_DIR}/src/System/Resources/basepoly_sv.rc)
	TARGET_SOURCES(clgame PRIVATE ${CMAKE_SOURCE_DIR}/src/System/Resources/basepoly_cl.rc)

	TARGET_LINK_LIBRARIES(client winmm ws2_32)
	TARGET_LINK_LIBRARIES(server winmm ws2_32)

	set_target_properties(client PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

	# macro redefinition, deprecation
	target_compile_options(client PRIVATE /wd4005 /wd4996)
	target_compile_options(server PRIVATE /wd4005 /wd4996)

    ## Group the resource files too, so they don't end up in the ugly "Source Files" folder in VS
    SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES 
        System/Resources/polyhedron.rc 
        System/Resources/polyhedron.rc )
ENDIF()



#
#	Client Build Configuration.
#
TARGET_INCLUDE_DIRECTORIES(client PUBLIC ../inc)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "/extern/stb/")
TARGET_LINK_LIBRARIES(client  SDL2main SDL2-static zlibstatic stb tinyobjloader fmt-header-only)
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
TARGET_LINK_LIBRARIES(server  SDL2main SDL2-static zlibstatic fmt-header-only)
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
