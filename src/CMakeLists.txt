cmake_minimum_required (VERSION 3.8)

###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
###################### CLIENT GAME
SET(SRC_CLGAME
	clgame/clg_effects.c
	clgame/clg_entities.c
	clgame/clg_main.c
	clgame/clg_media.c
	clgame/clg_newfx.c
	clgame/clg_parse.c
	clgame/clg_predict.c
	clgame/clg_screen.c
	clgame/clg_tents.c
	clgame/clg_view.c

	clgame/clgame.def
)

SET(HEADERS_CLGAME
	clgame/clg_local.h
)

###################### SERVER GAME
SET(SRC_SVGAME
	svgame/ai.c
	svgame/brushfuncs.c
	svgame/chase.c
	svgame/cmds.c
	svgame/combat.c
	svgame/functionpointers.c
	svgame/items.c
	svgame/main.c
	svgame/misc.c
	svgame/monsters.c
	svgame/physics.c
	svgame/save.c
	svgame/spawn.c
	svgame/svcmds.c
	svgame/trigger.c
	svgame/turret.c
	svgame/utils.c
	svgame/weapons.c

	svgame/entities/func/func_areaportal.c
	svgame/entities/func/func_button.c
	svgame/entities/func/func_clock.c
	svgame/entities/func/func_conveyor.c
	svgame/entities/func/func_door.c 
	svgame/entities/func/func_door_rotating.c
	svgame/entities/func/func_explosive.c 
	svgame/entities/func/func_group.c
	svgame/entities/func/func_killbox.c
	svgame/entities/func/func_object.c
	svgame/entities/func/func_plat.c
	svgame/entities/func/func_rotating.c
	svgame/entities/func/func_timer.c
	svgame/entities/func/func_train.c 
	svgame/entities/func/func_wall.c
	svgame/entities/func/func_water.c
	
	svgame/entities/misc/misc_banner.c 
	svgame/entities/misc/misc_bigviper.c 
	svgame/entities/misc/misc_blackhole.c 
	svgame/entities/misc/misc_deadsoldier.c 
	svgame/entities/misc/misc_easterchick.c 
	svgame/entities/misc/misc_easterchick2.c 
	svgame/entities/misc/misc_eastertank.c 
	svgame/entities/misc/misc_explobox.c 
	svgame/entities/misc/misc_gib_arm.c 
	svgame/entities/misc/misc_gib_leg.c 
	svgame/entities/misc/misc_gib_head.c 
	svgame/entities/misc/misc_satellite_dish.c 
	svgame/entities/misc/misc_strogg_ship.c 
	svgame/entities/misc/misc_teleporter.c
	svgame/entities/misc/misc_teleporter_dest.c
	svgame/entities/misc/misc_viper.c
	svgame/entities/misc/misc_viperbomb.c

	svgame/entities/target/target_blaster.c
	svgame/entities/target/target_changelevel.c
	svgame/entities/target/target_character.c
	svgame/entities/target/target_crosslevel_target.c
	svgame/entities/target/target_crosslevel_trigger.c
	svgame/entities/target/target_earthquake.c
	svgame/entities/target/target_explosion.c
	svgame/entities/target/target_goal.c
	svgame/entities/target/target_help.c
	svgame/entities/target/target_laser.c
	svgame/entities/target/target_lightramp.c
	svgame/entities/target/target_secret.c
	svgame/entities/target/target_spawner.c
	svgame/entities/target/target_speaker.c
	svgame/entities/target/target_splash.c
	svgame/entities/target/target_string.c
	svgame/entities/target/target_temp_entity.c
	svgame/entities/trigger/trigger_always.c
	svgame/entities/trigger/trigger_counter.c
	svgame/entities/trigger/trigger_elevator.c
	svgame/entities/trigger/trigger_gravity.c
	svgame/entities/trigger/trigger_hurt.c
	svgame/entities/trigger/trigger_key.c
	svgame/entities/trigger/trigger_monsterjump.c
	svgame/entities/trigger/trigger_multiple.c
	svgame/entities/trigger/trigger_once.c
	svgame/entities/trigger/trigger_push.c
	svgame/entities/trigger/trigger_relay.c
	
	svgame/entities/info_notnull.c
	svgame/entities/info_null.c
	svgame/entities/light.c
	svgame/entities/light_mine1.c
	svgame/entities/light_mine2.c
	svgame/entities/path_corner.c
	svgame/entities/point_combat.c
	svgame/entities/viewthing.c

	svgame/monsters/movement.c
	svgame/monsters/soldier.c

	svgame/player/client.c
	svgame/player/hud.c
	svgame/player/trail.c
	svgame/player/view.c
	svgame/player/weapons.c

	svgame/weapons/bfg10k.c
	svgame/weapons/blaster.c
	svgame/weapons/chaingun.c
	svgame/weapons/flaregun.c
	svgame/weapons/grenade.c
	svgame/weapons/grenadelauncher.c
	svgame/weapons/hyperblaster.c
	svgame/weapons/machinegun.c
	svgame/weapons/railgun.c
	svgame/weapons/rocketlauncher.c
	svgame/weapons/shotgun.c
	svgame/weapons/supershotgun.c
)

SET(HEADERS_SVGAME
	svgame/svgame.def

	svgame/g_local.h
	svgame/functionpointers.h

	svgame/entities/func/func_door.h

	svgame/monsters/soldier.h

	svgame/player/animations.h
	svgame/player/weapons.h

	svgame/weapons/bfg10k.h
	svgame/weapons/blaster.h
	svgame/weapons/chaingun.h
	svgame/weapons/flaregun.h
	svgame/weapons/grenade.h
	svgame/weapons/grenadelauncher.h
	svgame/weapons/hyperblaster.h
	svgame/weapons/machinegun.h
	svgame/weapons/railgun.h
	svgame/weapons/rocketlauncher.h
	svgame/weapons/shotgun.h
	svgame/weapons/supershotgun.h
)

###################### Client.
SET(SRC_CLIENT
	client/ascii.c
	client/console.c
	client/cin.c
	client/clgame.c
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
	client/parse.c
	client/precache.c
	client/predict.c
	client/refresh.c
	client/screen.c
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
	server/svgame.c
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
	common/prompt.c
	common/sizebuf.c
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

###################### Shared Game Folder.
SET(HEADERS_SHAREDGAME
	../inc/sharedgame/pmove.h 
)
SET(SRC_SHAREDGAME
	sharedgame/pmove.c
)

###################### Shared Folder.
SET(HEADERS_SHARED
	../inc/shared/cl_game.h
	../inc/shared/cl_types.h
	../inc/shared/config.h
	../inc/shared/game.h 
	../inc/shared/list.h 
	../inc/shared/math.h 
	../inc/shared/platform.h
	../inc/shared/shared.h 
)
SET(SRC_SHARED
	shared/m_flash.c
	shared/math.c
	shared/shared.c
)

###################### UNIX Client Specific.
SET(SRC_UNIX
	os/unix/hunk.c
	os/unix/system.c
	os/unix/tty.c
)
SET(SRC_UNIX_CLIENT
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
ADD_DEFINITIONS(-DVERSION_MAJOR=${NAC_VERSION_MAJOR})
ADD_DEFINITIONS(-DVERSION_MINOR=${NAC_VERSION_MINOR})
ADD_DEFINITIONS(-DVERSION_POINT=${NAC_VERSION_POINT})
ADD_DEFINITIONS(-DVERSION_SHA=${NAC_VERSION_SHA})
ADD_DEFINITIONS(-DVERSION_BRANCH=${NAC_VERSION_BRANCH})

ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ENDIF()

# Optional build target.
if (CONFIG_BUILD_SV_GAME)
	ADD_LIBRARY(svgame SHARED ${SRC_SVGAME} ${HEADERS_SVGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
endif()

# Optional build target.
if (CONFIG_BUILD_CL_GAME)
	ADD_LIBRARY(clgame SHARED ${SRC_CLGAME} ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
endif()

IF(WIN32)
ADD_EXECUTABLE(client WIN32 
	${SRC_CLIENT} ${HEADERS_CLIENT} 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_REFRESH} ${SRC_SHADERS} 
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_WINDOWS} ${SRC_WINDOWS_CLIENT} ${HEADERS_WINDOWS}
	${SRC_SERVER} ${HEADERS_SERVER}
	os/res/nac.rc
)
ADD_EXECUTABLE(server 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_SHARED} ${HEADERS_SHARED}
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
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_UNIX} ${SRC_UNIX_CLIENT}
	${SRC_SERVER} ${HEADERS_SERVER}
)
ADD_EXECUTABLE(server
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_UNIX}
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

SOURCE_GROUP("common\\sources" FILES ${SRC_COMMON})
SOURCE_GROUP("common\\headers" FILES ${HEADERS_COMMON})
SOURCE_GROUP("client\\sources" FILES ${SRC_CLIENT})
SOURCE_GROUP("client\\headers" FILES ${HEADERS_CLIENT})
SOURCE_GROUP("server\\sources" FILES ${SRC_SERVER})
SOURCE_GROUP("server\\headers" FILES ${HEADERS_SERVER})
SOURCE_GROUP("svgame\\sources" FILES ${SRC_SVGAME})
SOURCE_GROUP("svgame\\headers" FILES ${HEADERS_SVGAME})
SOURCE_GROUP("refresh\\sources" FILES ${SRC_REFRESH} ${SRC_VKPT})
SOURCE_GROUP("refresh\\headers" FILES ${HEADERS_VKPT})
SOURCE_GROUP("refresh\\shaders" FILES ${SRC_SHADERS})
SOURCE_GROUP("shared\\sources" FILES ${SRC_SHARED})
SOURCE_GROUP("os\\windows\\sources" FILES ${SRC_WINDOWS})
SOURCE_GROUP("os\\windows\\headers" FILES ${HEADERS_WINDOWS})
SOURCE_GROUP("os\\unix\\sources" FILES ${SRC_UNIX})
SOURCE_GROUP("os\\unix\\headers" FILES ${HEADERS_UNIX})

IF (WIN32)
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../VC/inc)
	TARGET_INCLUDE_DIRECTORIES(svgame PRIVATE ../VC/inc)
	TARGET_SOURCES(svgame PRIVATE os/res/basenac.rc)

	TARGET_LINK_LIBRARIES(client winmm ws2_32)
	TARGET_LINK_LIBRARIES(server winmm ws2_32)

	set_target_properties(client PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	set_target_properties(server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

	# macro redefinition, deprecation
	target_compile_options(client PRIVATE /wd4005 /wd4996)
	target_compile_options(server PRIVATE /wd4005 /wd4996)
	target_compile_options(svgame PRIVATE /wd4005 /wd4996)
ENDIF()

TARGET_INCLUDE_DIRECTORIES(svgame PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(clgame PRIVATE ../inc)

TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(server PRIVATE "${ZLIB_INCLUDE_DIRS}")

# WATISDEZE: Do we still need this?
# Use dynamic zlib for steam runtime
# if (CONFIG_LINUX_STEAM_RUNTIME_SUPPORT)
#     TARGET_LINK_LIBRARIES(client SDL2main SDL2-static z)
#     TARGET_LINK_LIBRARIES(server SDL2main SDL2-static z)
# else()
# TARGET_LINK_LIBRARIES(client SDL2main SDL2-static zlibstatic)
# TARGET_LINK_LIBRARIES(server SDL2main SDL2-static zlibstatic)
# endif()
TARGET_LINK_LIBRARIES(client SDL2main SDL2-static zlibstatic)
TARGET_LINK_LIBRARIES(server SDL2main SDL2-static zlibstatic)

SET_TARGET_PROPERTIES(client
    PROPERTIES
    OUTPUT_NAME "nac"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
    DEBUG_POSTFIX ""
)

SET_TARGET_PROPERTIES(server
    PROPERTIES
    OUTPUT_NAME "nacded"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}"
    DEBUG_POSTFIX ""
)

# Client Game
if (CONFIG_BUILD_CL_GAME)
	# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
	SET_TARGET_PROPERTIES(clgame
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
endif()

# Server Game
if (CONFIG_BUILD_SV_GAME)
	# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
	SET_TARGET_PROPERTIES(svgame
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
endif()

# WATISDEZE: Do we still need this?
# IF(CONFIG_LINUX_PACKAGING_SUPPORT)
#     # Put the real game binary in /usr/share so we can have a wrapper in /usr/bin
#     INSTALL(TARGETS client DESTINATION share/quake2rtx/bin COMPONENT shareware)
#     INSTALL(TARGETS server DESTINATION games COMPONENT shareware)

#     # Compress NVIDIA custom content
#     INSTALL(CODE "set(SOURCE \"${CMAKE_SOURCE_DIR}\")"
#         SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_media.cmake"
#         SCRIPT "${CMAKE_SOURCE_DIR}/setup/package_shaders.cmake"
#         COMPONENT shareware)
#     INSTALL(TARGETS svgame DESTINATION share/quake2rtx/basenac COMPONENT shareware)

#     # Package data files, including the shareware pak0 for demo levels
#     set (SHAREWARE_DATA_FILES_LIST
#         "${CMAKE_SOURCE_DIR}/basenac/q2rtx_media.pkz"
#         "${CMAKE_SOURCE_DIR}/basenac/shaders.pkz"
#         "${CMAKE_SOURCE_DIR}/basenac/blue_noise.pkz"
#         "${CMAKE_SOURCE_DIR}/basenac/shareware/pak0.pak"
#     )
#     INSTALL(FILES
#         ${SHAREWARE_DATA_FILES_LIST}
#         DESTINATION share/quake2rtx/basenac
#         COMPONENT shareware)

#     # Package shareware player model
#     INSTALL(DIRECTORY
#         "${CMAKE_SOURCE_DIR}/basenac/shareware/players"
#         DESTINATION share/quake2rtx/basenac
#         COMPONENT shareware)

#     # Package the legal and info files
#     INSTALL(FILES
#         "${CMAKE_SOURCE_DIR}/license.txt"
#         "${CMAKE_SOURCE_DIR}/notice.txt"
#         "${CMAKE_SOURCE_DIR}/readme.md"
#         "${CMAKE_SOURCE_DIR}/changelog.md"
#         DESTINATION share/quake2rtx/
#         COMPONENT shareware)

#     # Package the icon/desktop file
#     INSTALL(FILES
#         "${CMAKE_SOURCE_DIR}/setup/q2rtx.desktop"
#         DESTINATION share/applications/
#         COMPONENT shareware)
#     INSTALL(FILES
#         "${CMAKE_SOURCE_DIR}/setup/q2rtx.png"
#         DESTINATION share/pixmaps/
#         COMPONENT shareware)

#     # Install Client Wrapper
#     INSTALL(FILES
#         "${CMAKE_SOURCE_DIR}/setup/q2rtx.sh"
#         DESTINATION games/
#         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#         COMPONENT shareware
# 	RENAME q2rtx)

#     # Install PAK file finder
#     INSTALL(FILES
#         "${CMAKE_SOURCE_DIR}/setup/find-retail-paks.sh"
#         DESTINATION share/quake2rtx/bin
#         PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#         COMPONENT shareware)
# ENDIF()
