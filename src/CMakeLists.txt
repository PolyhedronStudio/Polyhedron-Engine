cmake_minimum_required (VERSION 3.8)

###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
###################### CLIENT GAME
SET(SRC_CLGAME
	clgame/ClientGameExports.cpp
	clgame/ClientGameImports.cpp

	clgame/exports/Core.cpp
	clgame/exports/Entities.cpp
	clgame/exports/Media.cpp
	clgame/exports/Movement.cpp
	clgame/exports/Prediction.cpp
	clgame/exports/Screen.cpp
	clgame/exports/ServerMessage.cpp
	clgame/exports/View.cpp

	clgame/clg_effects.cpp
	clgame/clg_entities.cpp
	clgame/clg_input.cpp
	clgame/clg_main.cpp
	clgame/clg_media.cpp
	clgame/clg_newfx.cpp
	clgame/clg_parse.cpp
	clgame/clg_predict.cpp
	clgame/clg_screen.cpp
	clgame/clg_tents.cpp
	clgame/clg_view.cpp

	clgame/clgame.def
)

SET(HEADERS_CLGAME
	clgame/ClientGameExports.h
	clgame/ClientGameImports.h

	clgame/exports/Core.h
	clgame/exports/Entities.h
	clgame/exports/Media.h
	clgame/exports/Movement.h
	clgame/exports/Prediction.h
	clgame/exports/Screen.h
	clgame/exports/ServerMessage.h
	clgame/exports/View.h

	clgame/clg_effects.h
	clgame/clg_entities.h
	clgame/clg_input.h
	clgame/clg_local.h
	clgame/clg_main.h
	clgame/clg_media.h
	clgame/clg_newfx.h
	clgame/clg_parse.h
	clgame/clg_predict.h
	clgame/clg_screen.h
	clgame/clg_tents.h
	clgame/clg_view.h
)

# Server game DLL
# Throw all sources in here, but group them elsewhere
SET( SRC_SVGAME 
	svgame/brushfuncs.cpp
	svgame/chasecamera.cpp
	svgame/commands.cpp
	svgame/combat.cpp
	svgame/effects.cpp
	svgame/entities.cpp
	svgame/functionpointers.cpp
	svgame/items.cpp
	svgame/ImportsWrapper.cpp
	svgame/main.cpp
	svgame/effects.cpp
	svgame/save.cpp
	svgame/spawn.cpp
	svgame/svcmds.cpp
	svgame/trigger.cpp
	svgame/utils.cpp
	svgame/weapons.cpp 
	svgame/ai/MonsterAI.cpp
	svgame/entities/base/SVGBaseEntity.cpp
	svgame/entities/base/SVGBaseTrigger.cpp
	svgame/entities/base/SVGBaseMover.cpp
	svgame/entities/base/BodyCorpse.cpp
	svgame/entities/base/DebrisEntity.cpp
	svgame/entities/base/GibEntity.cpp
	svgame/entities/base/PlayerClient.cpp
	#svgame/entities/info/InfoNotNull.cpp
	svgame/entities/info/InfoNull.cpp
	#svgame/entities/info/InfoPlayerCoop.cpp
	#svgame/entities/info/InfoPlayerDeathmatch.cpp
	#svgame/entities/info/InfoPlayerIntermission.cpp
	svgame/entities/info/InfoPlayerStart.cpp
	svgame/entities/func/FuncAreaportal.cpp
	svgame/entities/func/FuncButton.cpp
	#svgame/entities/func/FuncConveyor.cpp
	svgame/entities/func/FuncDoor.cpp 
	svgame/entities/func/FuncDoorRotating.cpp
	svgame/entities/func/FuncExplosive.cpp 
	#svgame/entities/func/FuncKillbox.cpp
	#svgame/entities/func/FuncObject.cpp
	#svgame/entities/func/FuncPlat.cpp
	svgame/entities/func/FuncRotating.cpp
	svgame/entities/func/FuncTimer.cpp
	svgame/entities/func/FuncTrain.cpp 
	svgame/entities/func/FuncWall.cpp
	#svgame/entities/func/FuncWater.cpp
	#svgame/entities/misc/MiscExplobox.cpp
	#svgame/entities/misc/MiscGibArm.cpp 
	#svgame/entities/misc/MiscGibLeg.cpp 
	#svgame/entities/misc/MiscGibHead.cpp 
	svgame/entities/misc/MiscExplosionBox.cpp
	svgame/entities/misc/MiscServerModel.cpp
	#svgame/entities/misc/MiscTeleporter.cpp
	#svgame/entities/misc/MiscTeleporterDest.cpp
	svgame/entities/path/PathCorner.cpp
	#svgame/entities/target/TargetBlaster.cpp
	#svgame/entities/target/TargetChangelevel.cpp
	#svgame/entities/target/TargetCrosslevel_target.cpp
	#svgame/entities/target/TargetCrosslevel_trigger.cpp
	svgame/entities/target/TargetEarthquake.cpp
	svgame/entities/target/TargetExplosion.cpp
	#svgame/entities/target/TargetLightramp.cpp
	#svgame/entities/target/TargetSpawner.cpp
	svgame/entities/target/TargetSpeaker.cpp
	#svgame/entities/target/TargetSplash.cpp
	svgame/entities/target/TargetTempEntity.cpp
	#svgame/entities/trigger/TriggerCounter.cpp
	#svgame/entities/trigger/TriggerElevator.cpp
	#svgame/entities/trigger/TriggerGravity.cpp
	#svgame/entities/trigger/TriggerKey.cpp
	#svgame/entities/trigger/TriggerMonsterjump.cpp
	#svgame/entities/trigger/TriggerPush.cpp
	svgame/entities/trigger/TriggerAlways.cpp
	svgame/entities/trigger/TriggerAutoDoor.cpp
	svgame/entities/trigger/TriggerDelayedUse.cpp
	svgame/entities/trigger/TriggerHurt.cpp
	svgame/entities/trigger/TriggerMultiple.cpp
	svgame/entities/trigger/TriggerOnce.cpp
	svgame/entities/trigger/TriggerRelay.cpp
	svgame/entities/weaponry/BlasterBolt.cpp
	svgame/entities/Light.cpp
	svgame/entities/WorldSpawn.cpp
	svgame/gamemodes/DefaultGameMode.cpp
	svgame/gamemodes/CoopGameMode.cpp
	svgame/gamemodes/DeathMatchGameMode.cpp
	svgame/physics/physics.cpp
	svgame/physics/stepmove.cpp
	svgame/player/client.cpp
	svgame/player/hud.cpp
	svgame/player/trail.cpp
	svgame/player/player.cpp
	svgame/player/view.cpp
	svgame/player/weapons.cpp
	svgame/weapons/blaster.cpp
	svgame/weapons/machinegun.cpp
	svgame/weapons/shotgun.cpp
	svgame/weapons/supershotgun.cpp
    svgame/svgame.def
    )
    
    SET ( HEADERS_SVGAME
	svgame/g_local.h
	svgame/brushfuncs.h
	svgame/chasecamera.h
	svgame/effects.h
	svgame/entities.h
	svgame/functionpointers.h
	svgame/trigger.h
	svgame/TypeInfo.h
	svgame/utils.h
	svgame/ai/MonsterAI.h
	svgame/entities/base/SVGBaseEntity.h
	svgame/entities/base/SVGBaseTrigger.h
	svgame/entities/base/SVGBaseMover.h
	svgame/entities/base/BodyCorpse.h
	svgame/entities/base/DebrisEntity.h
	svgame/entities/base/GibEntity.h
	svgame/entities/base/PlayerClient.h
	svgame/entities/func/FuncAreaportal.h
	svgame/entities/func/FuncButton.h
	svgame/entities/func/FuncDoor.h
	svgame/entities/func/FuncDoorRotating.h
	svgame/entities/func/FuncExplosive.h
	svgame/entities/func/FuncRotating.h
	svgame/entities/func/FuncTimer.h
	svgame/entities/func/FuncTrain.h
	svgame/entities/func/FuncWall.h
	svgame/entities/info/InfoPlayerStart.h
	svgame/entities/misc/MiscExplosionBox.h
	svgame/entities/misc/MiscServerModel.h
	svgame/entities/path/PathCorner.h
	svgame/entities/target/TargetEarthquake.h
	svgame/entities/target/TargetExplosion.h
	svgame/entities/target/TargetSpeaker.h
	svgame/entities/target/TargetTempEntity.h
	svgame/entities/trigger/TriggerAlways.h
	svgame/entities/trigger/TriggerAutoDoor.h
	svgame/entities/trigger/TriggerDelayedUse.h
	svgame/entities/trigger/TriggerHurt.h
	svgame/entities/trigger/TriggerMultiple.h
	svgame/entities/trigger/TriggerOnce.h
	svgame/entities/trigger/TriggerRelay.h
	svgame/entities/weaponry/BlasterBolt.h
	svgame/entities/Light.h
	svgame/entities/WorldSpawn.h
	svgame/gamemodes/DefaultGameMode.h
	svgame/gamemodes/CoopGameMode.h
	svgame/gamemodes/DeathMatchGameMode.h
	svgame/gamemodes/IGameMode.h
	svgame/player/animations.h
	svgame/player/client.h
	svgame/player/player.h
	svgame/player/view.h
	svgame/player/weapons.h
	svgame/weapons/blaster.h
	svgame/weapons/machinegun.h
	svgame/weapons/shotgun.h
	svgame/weapons/supershotgun.h
    svgame/svgame.def
)

###################### Client.
SET(SRC_CLIENT
	client/ascii.cpp
	client/console.cpp
	client/cin.cpp
	client/clgame.cpp
	client/crc.cpp
	client/demo.cpp
	client/download.cpp
	#client/effects.cpp
	client/entities.cpp
	#client/gtv.cpp
	client/input.cpp
	client/keys.cpp
	client/locs.cpp
	client/main.cpp
	client/parse.cpp
	client/precache.cpp
	client/predict.cpp
	client/refresh.cpp
	client/screen.cpp
	client/view.cpp
	client/ui/demos.cpp
	client/ui/menu.cpp
	client/ui/playerconfig.cpp
	client/ui/playermodels.cpp
	client/ui/script.cpp
	client/ui/servers.cpp
	client/ui/ui.cpp
	client/sound/dma.cpp
	client/sound/al.cpp
	client/sound/main.cpp
	client/sound/mem.cpp
	client/sound/mix.cpp
	client/sound/ogg.cpp
#	client/sound/qal/fixed.c
	client/sound/qal/dynamic.cpp
)
# Required as a placeholder for certain Vulkan only related features.
if (NOT CONFIG_VKPT_RENDERER AND CONFIG_GL_RENDERER)
	set(SRC_CLIENT 
		${SRC_CLIENT}
		client/null_gl.cpp
	)
endif()

if (CONFIG_ENABLE_RMLUI)
SET(SRC_CLIENT_RMLUI
	client/rmlui/interfaces/FileInterface.cpp
	client/rmlui/interfaces/FileInterface.h
	client/rmlui/interfaces/RenderInterface.cpp
	client/rmlui/interfaces/RenderInterface.h
	client/rmlui/interfaces/SystemInterface.cpp
	client/rmlui/interfaces/SystemInterface.h
	client/rmlui/rmlui.cpp
	client/rmlui/rmlui.h
)
else()
SET(SRC_CLIENT_RMLUI
	client/rmlui/rmlui_null.cpp
	client/rmlui/rmlui.h
)
endif()

SET(SRC_CLIENT_HTTP
	client/http.cpp
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
	server/commands.cpp
	server/entities.cpp
	server/svgame.cpp
	server/init.cpp
	server/main.cpp

	server/send.cpp
	server/user.cpp
	server/world.cpp

	server/save.cpp
)
SET(HEADERS_SERVER
	server/server.h
)

###################### Common.
SET(SRC_COMMON
	common/bsp.cpp
	common/cmd.cpp
	common/cmodel.cpp
	common/common.cpp
	common/cvar.cpp
	common/error.cpp
	common/field.cpp
	common/fifo.cpp
	common/files.cpp
	common/mdfour.cpp
	common/msg.cpp
	common/prompt.cpp
	common/sizebuf.cpp
	common/utils.cpp
	common/zone.cpp

	common/hashes/crc32.cpp

	common/net/netchan.cpp
	common/net/net.cpp

	# These might have to go some day.
	#common/netq3/netchan.cpp
	#common/netq3/net.cpp
)
SET(HEADERS_COMMON
	common/net/inet_ntop.h
	common/net/inet_pton.h
	common/net/win.h
	
	common/hashes/crc32.h

	common/enet/netchan.h
	common/net/net.h

	# These might have to go some day.
	common/netq3/netchan.h
	common/netq3/net.h
)

###################### Format.
#SET(SRC_FORMAT
#)
SET(HEADERS_COMMON
	../inc/format/bsp.h
	../inc/format/iqm.h
	../inc/format/md2.h
	../inc/format/md3.h
	../inc/format/pak.h
	../inc/format/pcx.h
	../inc/format/sp2.h
	../inc/format/wal.h
)

###################### Refresh.
SET(SRC_REFRESH
	refresh/images.cpp
	refresh/models.cpp
	refresh/model_iqm.cpp
	refresh/stb/stb.cpp
)

###################### GL.
SET(SRC_GL
	refresh/gl/draw.cpp
	refresh/gl/hq2x.cpp
	refresh/gl/images.cpp
	refresh/gl/main.cpp
	refresh/gl/mesh.cpp
	refresh/gl/models.cpp
	refresh/gl/sky.cpp
	refresh/gl/state.cpp
	refresh/gl/surf.cpp
	refresh/gl/tess.cpp
	refresh/gl/world.cpp
	refresh/gl/qgl/dynamic.cpp
#	refresh/gl/qgl/fixed.c
)
SET(HEADERS_GL
	refresh/gl/arbfp.h
	refresh/gl/gl.h
)

###################### Shared Game Folder.
SET(HEADERS_SHAREDGAME
	../inc/sharedgame/pmove.h 
	../inc/sharedgame/protocol.h 
	../inc/sharedgame/sharedgame.h 
)
SET(SRC_SHAREDGAME
	../src/sharedgame/pmove.cpp
)

###################### Shared Folder.
SET(HEADERS_SHARED
	# Container lib.
	../inc/shared/containers/IterableCArray.h

	# Interfaces
	../inc/shared/interfaces/IClientGameExports.h
	../inc/shared/interfaces/IClientGameImports.h

	# Math lib.
	../inc/shared/math/color.h
	../inc/shared/math/matrix3.h
	../inc/shared/math/matrix4.h
	../inc/shared/math/plane.h
	../inc/shared/math/rectangle.h
	../inc/shared/math/utilities.h
	../inc/shared/math/vector2.h
	../inc/shared/math/vector3.h
	../inc/shared/math/vector4.h
	../inc/shared/math/vector5.h
	
	# Other shared lib functionalities.
	../inc/shared/clgame.h
	../inc/shared/cltypes.h
	../inc/shared/collision.h
	../inc/shared/config.h
	../inc/shared/endian.h 
	../inc/shared/svgame.h 
	../inc/shared/keys.h 
	../inc/shared/list.h 
	../inc/shared/math.h 
	../inc/shared/messaging.h 
	../inc/shared/platform.h
	../inc/shared/refresh.h 
	../inc/shared/qstring.h 
	../inc/shared/strings.h
	../inc/shared/shared.h 
	../inc/shared/ui.h 
)
SET(SRC_SHARED
	shared/math/plane.cpp
	shared/math/vector3.cpp

	shared/math.cpp
	shared/shared.cpp
)

###################### UNIX Client Specific.
SET(SRC_UNIX
	os/unix/hunk.cpp
	os/unix/system.cpp
	os/unix/tty.cpp
)
SET(SRC_UNIX_CLIENT
	os/sdl2/sound.cpp
	os/sdl2/video.cpp
	os/sdl2/time.cpp
)

###################### Windows Client Specific.
SET(SRC_WINDOWS
	os/windows/debug.cpp
	os/windows/hunk.cpp
	os/windows/system.cpp
)
SET(SRC_WINDOWS_CLIENT
	os/windows/wave.cpp
	os/sdl2/video.cpp
	os/sdl2/time.cpp
)
SET(HEADERS_WINDOWS
	os/windows/wgl.h
	os/windows/glimp.h
	os/windows/client.h
	os/windows/threads/threads.h
)

###################### VKPT.
SET(SRC_VKPT
	refresh/vkpt/asvgf.cpp
	refresh/vkpt/bloom.cpp
	refresh/vkpt/bsp_mesh.cpp
	refresh/vkpt/conversion.cpp
	refresh/vkpt/draw.cpp
	refresh/vkpt/freecam.cpp
	refresh/vkpt/fog.cpp
	refresh/vkpt/fsr.cpp
	refresh/vkpt/main.cpp
	refresh/vkpt/material.cpp
	refresh/vkpt/matrix.cpp
	refresh/vkpt/mgpu.cpp
	refresh/vkpt/models.cpp
	refresh/vkpt/path_tracer.cpp
	refresh/vkpt/physical_sky.cpp
	refresh/vkpt/precomputed_sky.cpp
	refresh/vkpt/profiler.cpp
	refresh/vkpt/shadow_map.cpp
	refresh/vkpt/textures.cpp
	refresh/vkpt/tone_mapping.cpp
	refresh/vkpt/transparency.cpp
	refresh/vkpt/uniform_buffer.cpp
	refresh/vkpt/vertex_buffer.cpp
	refresh/vkpt/vk_util.cpp
	refresh/vkpt/buddy_allocator.cpp
	refresh/vkpt/device_memory_allocator.cpp
	refresh/vkpt/god_rays.cpp
)
SET(HEADERS_VKPT
	refresh/vkpt/fsr/ffx_a.h
	refresh/vkpt/fsr/ffx_fsr1.h

	refresh/vkpt/vkpt.h
	refresh/vkpt/vk_util.h
	refresh/vkpt/buddy_allocator.h
	refresh/vkpt/conversion.h
	refresh/vkpt/device_memory_allocator.h
	refresh/vkpt/fog.h
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
	refresh/vkpt/shader/bloom_downscale.comp
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
	refresh/vkpt/shader/fsr_easu_fp16.comp
	refresh/vkpt/shader/fsr_easu_fp32.comp
	refresh/vkpt/shader/fsr_rcas_fp16.comp
	refresh/vkpt/shader/fsr_rcas_fp32.comp
)

set(SRC_RT_SHADERS
	refresh/vkpt/shader/primary_rays.rgen
	refresh/vkpt/shader/direct_lighting.rgen
	refresh/vkpt/shader/indirect_lighting.rgen
	refresh/vkpt/shader/path_tracer.rchit
	refresh/vkpt/shader/path_tracer.rmiss
	refresh/vkpt/shader/path_tracer_masked.rahit
	refresh/vkpt/shader/path_tracer_particle.rahit
	refresh/vkpt/shader/path_tracer_sprite.rahit
	refresh/vkpt/shader/path_tracer_beam.rahit
	refresh/vkpt/shader/path_tracer_beam.rint
	refresh/vkpt/shader/path_tracer_explosion.rahit
	refresh/vkpt/shader/reflect_refract.rgen
)

# Check for a shader compiler.
if (CONFIG_VKPT_RENDERER)
	IF(IS_64_BIT)
		include(../cmake/compileShaders.cmake)

		foreach(s ${SRC_RT_SHADERS})
			get_filename_component(shader_name ${s} NAME)
			get_filename_component(shader_ext ${s} EXT)
			compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.pipeline")
			if (shader_ext STREQUAL ".rgen")
				compile_shader(SOURCE_FILE ${s} OUTPUT_FILE_LIST shader_bytecode OUTPUT_FILE_NAME "${shader_name}.query" DEFINES "-DKHR_RAY_QUERY" STAGE "comp")
			endif()
		endforeach()

		foreach(s ${SRC_SHADERS})
			compile_shader(SOURCE_FILE ${s} INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/refresh/vkpt/fsr" OUTPUT_FILE_LIST shader_bytecode)
		endforeach()

		add_custom_target(shaders DEPENDS ${shader_bytecode})

		if(TARGET glslangValidator)
			add_dependencies(shaders glslangValidator)
		endif()
	ENDIF()
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
	# Used for sharedgame code(such as pmove), so we can ifdef certain parts specific to client or server.	
	ADD_DEFINITIONS(-DSHAREDGAME_SERVER=1)
	# Add library.
	ADD_LIBRARY(svgame SHARED ${SRC_SVGAME} ${HEADERS_SVGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
endif()

# Optional build target.
if (CONFIG_BUILD_CL_GAME)
	# Used for sharedgame code(such as pmove), so we can ifdef certain parts specific to client or server.	
	ADD_DEFINITIONS(-DSHAREDGAME_CLIENT=1)
	# Add library.
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
	client/null.cpp
	#os/sdl2/time.cpp
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
	client/null.c
)
ENDIF()

TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_SERVER=1 USE_CLIENT=1)
TARGET_COMPILE_DEFINITIONS(server PRIVATE USE_SERVER=1 USE_CLIENT=0)

IF(CONFIG_USE_CURL)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H=1 -DCURL_STATICLIB)

	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_HTTP})
	TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_CURL=1)
	TARGET_LINK_LIBRARIES(client libcurl)
ENDIF()

IF(CONFIG_ENABLE_RMLUI)
 	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_RMLUI})
	TARGET_INCLUDE_DIRECTORIES(client PRIVATE ${CMAKE_SOURCE_DIR}/extern/Nac-RmlUi/Include/)
	TARGET_COMPILE_DEFINITIONS(client PRIVATE USE_RMLUI=1 RMLUI_STATIC_LIB)
	TARGET_LINK_LIBRARIES(client RmlCore RmlDebugger RmlLua)
ELSE()
	# These are set to supply the rmlui_null.cpp file.
	TARGET_SOURCES(client PRIVATE ${SRC_CLIENT_RMLUI})
ENDIF()

if (GLSLANG_COMPILER AND CONFIG_VKPT_RENDERER)
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
## Admer: This is not needed any more
#TARGET_LINK_LIBRARIES(client enet)
#TARGET_LINK_LIBRARIES(server enet)

SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_COMMON})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_COMMON})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SERVER})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SERVER})

# Old SVGame source grouping - it just bundled *everything* into svgame/sources and svgame/headers, eugh -Admer
#SOURCE_GROUP("svgame" FILES ${SRC_SVGAME})
#SOURCE_GROUP("svgame" FILES ${HEADERS_SVGAME})

# Source and header groups
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SVGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SVGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHAREDGAME} )
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SHAREDGAME} )

SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_REFRESH} ${SRC_VKPT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_VKPT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHADERS})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SHARED})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SHARED})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_WINDOWS})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_WINDOWS})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_UNIX})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_UNIX})

SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES client/null.cpp )

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

    ## Group the resource files too, so they don't end up in the ugly "Source Files" folder in VS
    SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES 
        os/res/nac.rc 
        os/res/nacded.rc )
ENDIF()

TARGET_INCLUDE_DIRECTORIES(svgame PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(clgame PRIVATE ../inc)

TARGET_INCLUDE_DIRECTORIES(client PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(client PRIVATE "${ZLIB_INCLUDE_DIRS}")
TARGET_INCLUDE_DIRECTORIES(server PRIVATE ../inc)
TARGET_INCLUDE_DIRECTORIES(server PRIVATE "${ZLIB_INCLUDE_DIRS}")

TARGET_INCLUDE_DIRECTORIES(client PRIVATE "/extern/stb/")

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
