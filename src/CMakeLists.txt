cmake_minimum_required (VERSION 3.8)

###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
###################### BASENAC.
SET(SRC_BASENAC
	../kmnac/game/acebot_ai.c
	../kmnac/game/acebot_cmds.c
	../kmnac/game/acebot_compress.c
	../kmnac/game/acebot_items.c
	../kmnac/game/acebot_movement.c
	../kmnac/game/acebot_nodes.c
	../kmnac/game/acebot_spawn.c
	../kmnac/game/g_ai.c
	../kmnac/game/g_camera.c
	../kmnac/game/g_chase.c
	../kmnac/game/g_cmds.c
	../kmnac/game/g_combat.c
	../kmnac/game/g_crane.c
	../kmnac/game/g_ctf.c
	../kmnac/game/g_fog.c
	../kmnac/game/g_func.c
	../kmnac/game/g_items.c
	../kmnac/game/g_jetpack.c
	../kmnac/game/g_lights.c
	../kmnac/game/g_lock.c
	../kmnac/game/g_main.c
	../kmnac/game/g_misc.c
	../kmnac/game/g_model.c
	../kmnac/game/g_monster.c
	../kmnac/game/g_moreai.c
	../kmnac/game/g_mtrain.c
	../kmnac/game/g_patchplayermodels.c
	../kmnac/game/g_pendulum.c
	../kmnac/game/g_phys.c
	../kmnac/game/g_ptrs.c
	../kmnac/game/g_reflect.c
	../kmnac/game/g_save.c
	../kmnac/game/g_spawn.c
	../kmnac/game/g_svcmds.c
	../kmnac/game/g_target.c
	../kmnac/game/g_thing.c
	../kmnac/game/g_tracktrain.c
	../kmnac/game/g_trigger.c
	../kmnac/game/g_turret.c
	../kmnac/game/g_utils.c
	../kmnac/game/g_vehicle.c
	../kmnac/game/g_weapon.c
	../kmnac/game/km_cvar.c
	../kmnac/game/m_actor.c
	../kmnac/game/m_actor_weap.c
	../kmnac/game/m_berserk.c
	../kmnac/game/m_boss2.c
	../kmnac/game/m_boss3.c
	../kmnac/game/m_boss31.c
	../kmnac/game/m_boss32.c
	../kmnac/game/m_brain.c
	../kmnac/game/m_chick.c
#	../kmnac/game/m_flash.c
	../kmnac/game/m_flipper.c
	../kmnac/game/m_float.c
	../kmnac/game/m_flyer.c
	../kmnac/game/m_gladiator.c
	../kmnac/game/m_gunner.c
	../kmnac/game/m_hover.c
	../kmnac/game/m_infantry.c
	../kmnac/game/m_insane.c
	../kmnac/game/m_medic.c
	../kmnac/game/m_move.c
	../kmnac/game/m_mutant.c
	../kmnac/game/m_parasite.c
	../kmnac/game/m_soldier.c
	../kmnac/game/m_supertank.c
	../kmnac/game/m_tank.c
	../kmnac/game/p_chase.c
	../kmnac/game/p_client.c
	../kmnac/game/p_hud.c
	../kmnac/game/p_menu.c
	../kmnac/game/p_text.c
	../kmnac/game/p_trail.c
	../kmnac/game/p_view.c
	../kmnac/game/p_weapon.c
	../kmnac/game/q_shared.c
)

SET(HEADERS_BASENAC
	../kmnac/game/acebot.h
	../kmnac/game/game.h
	../kmnac/game/g_ctf.h
	../kmnac/game/g_func_decs.h
	../kmnac/game/g_func_list.h
	../kmnac/game/g_local.h
	../kmnac/game/g_mmove_decs.h
	../kmnac/game/g_mmove_list.h
	../kmnac/game/lazarus.def
	../kmnac/game/laz_misc.h
	../kmnac/game/km_cvar.h
	../kmnac/game/g_ptrs.h
	../kmnac/game/m_actor.h
	../kmnac/game/m_berserk.h
	../kmnac/game/q_shared.h
	../kmnac/game/resource.h
	../kmnac/game/p_text.h
	../kmnac/game/p_menu.h
	../kmnac/game/m_tank.h
	../kmnac/game/pak.h
	../kmnac/game/m_supertank.h
	../kmnac/game/m_soldier.h
	../kmnac/game/m_parasite.h
	../kmnac/game/m_player.h
	../kmnac/game/m_rider.h
	../kmnac/game/m_mutant.h
	../kmnac/game/m_medic.h
	../kmnac/game/m_insane.h
	../kmnac/game/m_boss2.h
	../kmnac/game/m_infantry.h
	../kmnac/game/m_hover.h
	../kmnac/game/m_gunner.h
	../kmnac/game/m_gladiator.h
	../kmnac/game/m_flyer.h
	../kmnac/game/m_float.h
	../kmnac/game/m_boss31.h
	../kmnac/game/m_flipper.h
	../kmnac/game/m_chick.h
	../kmnac/game/m_brain.h
	../kmnac/game/m_boss32.h
)

###################### Client.
SET(SRC_CLIENT
	client/ascii.c
	client/console.c
	client/cin.c
	client/crc.c
	client/demo.c
	client/download.c
	client/effects.c
	client/entities.c
	client/gtv.c
	client/input.c
	client/keys.c
	client/locs.c
	client/main.c
	client/newfx.c
#	client/null.c
#	client/null_gl.c
	client/parse.c
	client/precache.c
	client/predict.c
	client/refresh.c
	client/screen.c
	client/tent.c
	client/view.c
	client/ui/demos.c
	client/ui/menu.c
	client/ui/playerconfig.c
	client/ui/playermodels.c
	client/ui/script.c
	client/ui/servers.c
	client/ui/ui.c
	client/sound/dma.c
	client/sound/al.c
	client/sound/main.c
	client/sound/mem.c
	client/sound/mix.c
	client/sound/ogg.c
#	client/sound/qal/fixed.c
	client/sound/qal/dynamic.c
)
# Required as a placeholder for certain Vulkan only related features.
if (NOT CONFIG_VKPT_RENDERER AND CONFIG_GL_RENDERER)
	set(SRC_CLIENT 
		${SRC_CLIENT}
		client/null_gl.c
	)
endif()
SET(SRC_CLIENT_HTTP
	client/http.c
)
SET(HEADERS_CLIENT
	client/client.h
	client/ui/ui.h
	client/sound/sound.h
	client/sound/qal/dynamic.h
	client/sound/qal/fixed.h
)

###################### Client.
SET(SRC_SERVER
	server/commands.c
	server/entities.c
	server/game.c
	server/init.c
	server/main.c
	server/mvd.c
	server/send.c
	server/user.c
	server/world.c
	server/mvd/client.c
	server/mvd/parse.c
	server/mvd/game.c
	server/save.c
)
SET(HEADERS_SERVER
	server/server.h
)

###################### Common.
SET(SRC_COMMON
	common/bsp.c
	common/cmd.c
	common/cmodel.c
	common/common.c
	common/cvar.c
	common/error.c
	common/field.c
	common/fifo.c
	common/files.c
	common/math.c
	common/mdfour.c
	common/msg.c
	common/pmove.c
	common/prompt.c
	common/sizebuf.c
#	common/tests.c
	common/utils.c
	common/zone.c
	common/net/chan.c
	common/net/net.c
	common/x86/fpu.c
)
SET(HEADERS_COMMON
	common/net/inet_ntop.h
	common/net/inet_pton.h
	common/net/win.h
)

###################### Refresh.
SET(SRC_REFRESH
	refresh/images.c
	refresh/models.c
	refresh/stb/stb.c
)

###################### GL.
SET(SRC_GL
	refresh/gl/draw.c
	refresh/gl/hq2x.c
	refresh/gl/images.c
	refresh/gl/main.c
	refresh/gl/mesh.c
	refresh/gl/models.c
	refresh/gl/sky.c
	refresh/gl/state.c
	refresh/gl/surf.c
	refresh/gl/tess.c
	refresh/gl/world.c
	refresh/gl/qgl/dynamic.c
#	refresh/gl/qgl/fixed.c
)
SET(HEADERS_GL
	refresh/gl/arbfp.h
	refresh/gl/gl.h
)

###################### Shared Folder.
SET(HEADERS_SHARED
	../inc/shared/config.h
	../inc/shared/game.h 
	../inc/shared/list.h 
	../inc/shared/platform.h
	../inc/shared/shared.h 
)
SET(SRC_SHARED
	shared/m_flash.c
	shared/shared.c
)

###################### Linux Client Specific.
SET(SRC_LINUX
	os/unix/hunk.c
	os/unix/system.c
	os/unix/tty.c
)
SET(SRC_LINUX_CLIENT
	os/sdl2/sound.c
	os/sdl2/video.c
)

###################### Windows Client Specific.
SET(SRC_WINDOWS
	os/windows/ac.c
	os/windows/debug.c
	os/windows/hunk.c
	os/windows/system.c
)
SET(SRC_WINDOWS_CLIENT
	os/windows/wave.c
	os/sdl2/video.c
)
SET(HEADERS_WINDOWS
	os/windows/wgl.h
	os/windows/glimp.h
	os/windows/client.h
	os/windows/threads/threads.h
)

###################### VKPT.
SET(SRC_VKPT
	refresh/vkpt/asvgf.c
	refresh/vkpt/bloom.c
	refresh/vkpt/bsp_mesh.c
	refresh/vkpt/draw.c
	refresh/vkpt/freecam.c
	refresh/vkpt/main.c
	refresh/vkpt/material.c
	refresh/vkpt/matrix.c
	refresh/vkpt/mgpu.c
	refresh/vkpt/models.c
	refresh/vkpt/path_tracer.c
	refresh/vkpt/physical_sky.c
	refresh/vkpt/precomputed_sky.c
	refresh/vkpt/profiler.c
	refresh/vkpt/shadow_map.c
	refresh/vkpt/textures.c
	refresh/vkpt/tone_mapping.c
	refresh/vkpt/transparency.c
	refresh/vkpt/uniform_buffer.c
	refresh/vkpt/vertex_buffer.c
	refresh/vkpt/vk_util.c
	refresh/vkpt/buddy_allocator.c
	refresh/vkpt/device_memory_allocator.c
	refresh/vkpt/god_rays.c
)
SET(HEADERS_VKPT
	refresh/vkpt/vkpt.h
	refresh/vkpt/vk_util.h
	refresh/vkpt/buddy_allocator.h
	refresh/vkpt/device_memory_allocator.h
	refresh/vkpt/material.h
	refresh/vkpt/physical_sky.h
	refresh/vkpt/precomputed_sky.h
)

###################### Vulkan Shaders.
set(SRC_SHADERS
	refresh/vkpt/shader/animate_materials.comp
	refresh/vkpt/shader/god_rays_filter.comp
	refresh/vkpt/shader/god_rays.comp
	refresh/vkpt/shader/bloom_composite.comp
	refresh/vkpt/shader/bloom_blur.comp
	refresh/vkpt/shader/compositing.comp
	refresh/vkpt/shader/checkerboard_interleave.comp
	refresh/vkpt/shader/asvgf_atrous.comp
	refresh/vkpt/shader/asvgf_gradient_atrous.comp
	refresh/vkpt/shader/asvgf_gradient_img.comp
	refresh/vkpt/shader/asvgf_gradient_reproject.comp
	refresh/vkpt/shader/asvgf_lf.comp
	refresh/vkpt/shader/asvgf_taau.comp
	refresh/vkpt/shader/asvgf_temporal.comp
	refresh/vkpt/shader/instance_geometry.comp
	refresh/vkpt/shader/tone_mapping_histogram.comp
	refresh/vkpt/shader/tone_mapping_curve.comp
    refresh/vkpt/shader/tone_mapping_apply.comp
	refresh/vkpt/shader/physical_sky.comp
	refresh/vkpt/shader/physical_sky_space.comp
	refresh/vkpt/shader/shadow_map.vert
	refresh/vkpt/shader/sky_buffer_resolve.comp
	refresh/vkpt/shader/stretch_pic.frag
	refresh/vkpt/shader/stretch_pic.vert
	refresh/vkpt/shader/final_blit_lanczos.frag
	refresh/vkpt/shader/final_blit.vert
)
set(SRC_RT_SHADERS
	refresh/vkpt/shader/primary_rays.rgen
	refresh/vkpt/shader/direct_lighting.rgen
	refresh/vkpt/shader/indirect_lighting.rgen
	refresh/vkpt/shader/path_tracer.rchit
	refresh/vkpt/shader/path_tracer.rmiss
	refresh/vkpt/shader/path_tracer_particle.rahit
	refresh/vkpt/shader/path_tracer_sprite.rahit
	refresh/vkpt/shader/path_tracer_beam.rahit
	refresh/vkpt/shader/path_tracer_explosion.rahit
	refresh/vkpt/shader/path_tracer_shadow.rmiss
	refresh/vkpt/shader/reflect_refract.rgen
)

# Check for a shader compiler.
include(../cmake/compileShaders.cmake)

if (GLSLANG_COMPILER)
	foreach(s ${SRC_RT_SHADERS})
        get_filename_component(shader_name ${s} NAME)
		compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.nv" DEFINES "-DNV_RAY_TRACING")
		compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.khr" DEFINES "-DKHR_RAY_TRACING")
	endforeach()

	foreach(s ${SRC_SHADERS})
		compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode)
	endforeach()

	add_custom_target(shaders DEPENDS ${shader_bytecode})
else()
    message(WARNING "Could not find shader compiler: you're on your own to build the shaders")
endif()

# Embed version number
ADD_DEFINITIONS(-DVERSION_MAJOR=${Q2RTX_VERSION_MAJOR})
ADD_DEFINITIONS(-DVERSION_MINOR=${Q2RTX_VERSION_MINOR})
ADD_DEFINITIONS(-DVERSION_POINT=${Q2RTX_VERSION_POINT})
ADD_DEFINITIONS(-DVERSION_SHA=${Q2RTX_VERSION_SHA})
ADD_DEFINITIONS(-DVERSION_BRANCH=${Q2RTX_VERSION_BRANCH})

ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

ADD_LIBRARY(gamex86 SHARED ${SRC_BASENAC} ${HEADERS_BASENAC} ${SRC_SHARED})
IF(WIN32)
ADD_EXECUTABLE(client WIN32 
	${SRC_CLIENT} ${HEADERS_CLIENT} 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_REFRESH} ${SRC_SHADERS} 
	${SRC_SHARED} 
	${SRC_WINDOWS} ${SRC_WINDOWS_CLIENT} ${HEADERS_WINDOWS}
	${SRC_SERVER} ${HEADERS_SERVER}
	os/res/nac.rc
)
ADD_EXECUTABLE(server 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_SHARED} 
	${SRC_WINDOWS} ${HEADERS_WINDOWS}
	${SRC_SERVER} ${HEADERS_SERVER}
	server/ac.c
	client/null.c
	os/res/nacded.rc
)
ELSE()
ADD_EXECUTABLE(client
	${SRC_CLIENT} ${HEADERS_CLIENT} 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_REFRESH} ${SRC_SHADERS} 
	${SRC_SHARED} 
	${SRC_LINUX} ${SRC_LINUX_CLIENT}
	${SRC_SERVER} ${HEADERS_SERVER}
)
ADD_EXECUTABLE(server
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_SHARED} 
	${SRC_LINUX}
	${SRC_SERVER} ${HEADERS_SERVER}
	server/ac.c
	client/null.c
)
ENDIF()

TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_SERVER=1 USE_CLIENT=1)
TARGET_COMPILE_DEFINITIONS(server PRIVATE USE_SERVER=1 USE_CLIENT=0)

IF(CONFIG_USE_CURL)
	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_HTTP})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_CURL=1)
	TARGET_LINK_LIBRARIES(client libcurl)
ENDIF()

if (GLSLANG_COMPILER)
	add_dependencies(client shaders)
endif()

IF (CONFIG_GL_RENDERER)
	TARGET_SOURCES(client PRIVATE ${SRC_GL} ${HEADERS_GL})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE REF_GL=1 USE_REF=1)
ENDIF()

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

TARGET_LINK_LIBRARIES(client stb)
TARGET_LINK_LIBRARIES(client tinyobjloader)

SOURCE_GROUP("basenac\\sources" FILES ${SRC_BASENAC})
SOURCE_GROUP("basenac\\headers" FILES ${HEADERS_BASENAC})
SOURCE_GROUP("client\\sources" FILES ${SRC_CLIENT})
SOURCE_GROUP("client\\headers" FILES ${HEADERS_CLIENT})
SOURCE_GROUP("server\\sources" FILES ${SRC_SERVER})
SOURCE_GROUP("server\\headers" FILES ${HEADERS_SERVER})
SOURCE_GROUP("common\\sources" FILES ${SRC_COMMON})
SOURCE_GROUP("common\\headers" FILES ${HEADERS_COMMON})
SOURCE_GROUP("refresh\\sources" FILES ${SRC_REFRESH} ${SRC_VKPT})
SOURCE_GROUP("refresh\\headers" FILES ${HEADERS_VKPT})
SOURCE_GROUP("refresh\\shaders" FILES ${SRC_SHADERS})
SOURCE_GROUP("shared\\sources" FILES ${SRC_SHARED})
SOURCE_GROUP("windows\\sources" FILES ${SRC_WINDOWS})
SOURCE_GROUP("windows\\headers" FILES ${HEADERS_WINDOWS})

IF (WIN32)
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(gamex86 PRIVATE ../VC/inc)
	TARGET_SOURCES(gamex86 PRIVATE os/res/basenac.rc)

	TARGET_LINK_LIBRARIES(client winmm ws2_32)
	TARGET_LINK_LIBRARIES(server winmm ws2_32)

	set_target_properties(client PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

	# macro redefinition, deprecation
	target_compile_options(client PRIVATE /wd4005 /wd4996)
	target_compile_options(server PRIVATE /wd4005 /wd4996)
	target_compile_options(gamex86 PRIVATE /wd4005 /wd4996)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(gamex86 PRIVATE ../inc)

TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(server PRIVATE "${ZLIB_INCLUDE_DIRS}")

# Use dynamic zlib for steam runtime
if (CONFIG_LINUX_STEAM_RUNTIME_SUPPORT)
    TARGET_LINK_LIBRARIES(client SDL2main SDL2-static z)
    TARGET_LINK_LIBRARIES(server SDL2main SDL2-static z)
else()
    TARGET_LINK_LIBRARIES(client SDL2main SDL2-static zlibstatic)
    TARGET_LINK_LIBRARIES(server SDL2main SDL2-static zlibstatic)
endif()

SET_TARGET_PROPERTIES(client
    PROPERTIES
    OUTPUT_NAME "q2rtx"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
    DEBUG_POSTFIX ""
)

SET_TARGET_PROPERTIES(server
    PROPERTIES
    OUTPUT_NAME "q2rtxded"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
    DEBUG_POSTFIX ""
)

# specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux

SET_TARGET_PROPERTIES(gamex86
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/basenac"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/basenac"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/basenac"
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/basenac"
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/basenac"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/basenac"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/basenac"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/basenac"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/basenac"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/basenac"
    PREFIX ""
    DEBUG_POSTFIX ""
)

IF(IS_64_BIT)
	SET_TARGET_PROPERTIES(gamex86
		PROPERTIES
		LIBRARY_OUTPUT_NAME "gamex86_64"
		RUNTIME_OUTPUT_NAME "gamex86_64"
	)
ENDIF()

IF(CONFIG_LINUX_PACKAGING_SUPPORT)
    # Put the real game binary in /usr/share so we can have a wrapper in /usr/bin
    INSTALL(TARGETS client DESTINATION share/quake2rtx/bin COMPONENT shareware)
    INSTALL(TARGETS server DESTINATION games COMPONENT shareware)

    # Compress NVIDIA custom content
    INSTALL(CODE "set(SOURCE \"${CMAKE_SOURCE_DIR}\")"
        SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_media.cmake"
        SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_shaders.cmake"
        COMPONENT shareware)
    INSTALL(TARGETS gamex86 DESTINATION share/quake2rtx/basenac COMPONENT shareware)

    # Package data files, including the shareware pak0 for demo levels
    set (SHAREWARE_DATA_FILES_LIST
        "${CMAKE_SOURCE_DIR}/basenac/q2rtx_media.pkz"
        "${CMAKE_SOURCE_DIR}/basenac/shaders.pkz"
        "${CMAKE_SOURCE_DIR}/basenac/blue_noise.pkz"
        "${CMAKE_SOURCE_DIR}/basenac/shareware/pak0.pak"
    )
    INSTALL(FILES
        ${SHAREWARE_DATA_FILES_LIST}
        DESTINATION share/quake2rtx/basenac
        COMPONENT shareware)

    # Package shareware player model
    INSTALL(DIRECTORY
        "${CMAKE_SOURCE_DIR}/basenac/shareware/players"
        DESTINATION share/quake2rtx/basenac
        COMPONENT shareware)

    # Package the legal and info files
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/license.txt"
        "${CMAKE_SOURCE_DIR}/notice.txt"
        "${CMAKE_SOURCE_DIR}/readme.md"
        "${CMAKE_SOURCE_DIR}/changelog.md"
        DESTINATION share/quake2rtx/
        COMPONENT shareware)

    # Package the icon/desktop file
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.desktop"
        DESTINATION share/applications/
        COMPONENT shareware)
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.png"
        DESTINATION share/pixmaps/
        COMPONENT shareware)

    # Install Client Wrapper
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/q2rtx.sh"
        DESTINATION games/
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT shareware
	RENAME q2rtx)

    # Install PAK file finder
    INSTALL(FILES
        "${CMAKE_SOURCE_DIR}/setup/find-retail-paks.sh"
        DESTINATION share/quake2rtx/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT shareware)
ENDIF()
