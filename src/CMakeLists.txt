cmake_minimum_required (VERSION 3.8)

###############################################################################
##
##			SOURCE AND HEADER FILE LISTS
##
###############################################################################
###################### CLIENT GAME
SET(SRC_CLGAME
	CLGame/ClientGameExports.cpp
	CLGame/ClientGameImports.cpp

	CLGame/Effects/DynamicLights.cpp
	CLGame/Effects/LightStyles.cpp
	CLGame/Effects/MuzzleFlashEffects.cpp
	CLGame/Effects/Particles.cpp
	CLGame/Effects/Particles/BloodSplatters.cpp
	CLGame/Effects/Particles/BubbleTrailA.cpp
	CLGame/Effects/Particles/BubbleTrailB.cpp
	CLGame/Effects/Particles/DiminishingTrail.cpp
	CLGame/Effects/Particles/DirtAndSparks.cpp
	CLGame/Effects/Particles/ExplosionSparks.cpp
	CLGame/Effects/Particles/ForceWall.cpp
	CLGame/Effects/Particles/HeatBeam.cpp
	CLGame/Effects/Particles/ItemRespawn.cpp
	CLGame/Effects/Particles/LogoutEffect.cpp
	CLGame/Effects/Particles/MonsterPlasmaShell.cpp
	CLGame/Effects/Particles/SteamPuffs.cpp
	CLGame/Effects/Particles/Teleporter.cpp
	CLGame/Effects/Particles/WaterSplash.cpp

	CLGame/Entities/Base/CLGBaseEntity.cpp
	CLGame/Entities/Monsters/MonsterTestDummy.cpp
	CLGame/Entities/ClassEntityList.cpp
	CLGame/Entities/IClientGameEntity.cpp


	CLGame/Exports/Core.cpp
	CLGame/Exports/Entities.cpp
	CLGame/Exports/Media.cpp
	CLGame/Exports/Movement.cpp
	CLGame/Exports/Prediction.cpp
	CLGame/Exports/Screen.cpp
	CLGame/Exports/ServerMessage.cpp
	CLGame/Exports/View.cpp

	CLGame/HUD/ChatHUD.cpp
	CLGame/HUD/NumberHUD.cpp

	CLGame/Input/KeyBinding.cpp

	#CLGame/Physics/Physics.cpp
	#CLGame/Physics/StepMove.cpp

	CLGame/Debug.cpp
	CLGame/Entities.cpp
	CLGame/Main.cpp
	CLGame/TemporaryEntities.cpp

	CLGame/CLGame.def
)

SET(HEADERS_CLGAME
	CLGame/ClientGameExports.h
	CLGame/ClientGameImports.h

	CLGame/Effects/DynamicLights.h
	CLGame/Effects/LightStyles.h
	CLGame/Effects/MuzzleFlashEffects.h
	CLGame/Effects/ParticleEffects.h
	CLGame/Effects/Particles.h

	CLGame/Entities/Base/CLGBaseEntity.h
	CLGame/Entities/Monsters/MonsterTestDummy.h
	CLGame/Entities/ClassEntityList.h
	CLGame/Entities/IClientGameEntity.h

	CLGame/Exports/Core.h
	CLGame/Exports/Entities.h
	CLGame/Exports/Media.h
	CLGame/Exports/Movement.h
	CLGame/Exports/Prediction.h
	CLGame/Exports/Screen.h
	CLGame/Exports/ServerMessage.h
	CLGame/Exports/View.h

	CLGame/HUD/ChatHUD.h
	CLGame/HUD/NumberHUD.h

	CLGame/Input/KeyBinding.h
	
	#CLGame/Physics/StepMove.h

	CLGame/Debug.h
	CLGame/Entities.h
	CLGame/ClientGameLocal.h
	CLGame/Main.h
	CLGame/TemporaryEntities.h
)

# Server game DLL
# Throw all sources in here, but group them elsewhere
SET( SRC_SVGAME 
	SVGame/Ballistics.cpp
	SVGame/ChaseCamera.cpp
	SVGame/Commands.cpp
	SVGame/Effects.cpp
	SVGame/Entities.cpp
	SVGame/ServerGameLocals.cpp
	SVGame/ImportsWrapper.cpp
	SVGame/Main.cpp
	SVGame/Effects.cpp
	SVGame/Save.cpp
	SVGame/SVCommands.cpp
	SVGame/Utilities.cpp
	SVGame/Weapons.cpp 
	SVGame/ai/MonsterAI.cpp
	SVGame/Entities/Base/SVGBaseEntity.cpp
	SVGame/Entities/Base/SVGBaseItem.cpp
	SVGame/Entities/Base/SVGBaseItemAmmo.cpp
	SVGame/Entities/Base/SVGBaseItemWeapon.cpp
	SVGame/Entities/Base/SVGBaseTrigger.cpp
	SVGame/Entities/Base/SVGBaseMonster.cpp
	SVGame/Entities/Base/SVGBaseMover.cpp
	SVGame/Entities/Base/SVGBasePlayer.cpp
	SVGame/Entities/Base/BodyCorpse.cpp
	SVGame/Entities/Base/DebrisEntity.cpp
	SVGame/Entities/Base/GibEntity.cpp

	#SVGame/Entities/Info/InfoNotNull.cpp
	SVGame/Entities/Info/InfoNull.cpp
	SVGame/Entities/Info/InfoPlayerCoop.cpp
	SVGame/Entities/Info/InfoPlayerDeathmatch.cpp
	SVGame/Entities/Info/InfoPlayerIntermission.cpp
	SVGame/Entities/Info/InfoPlayerStart.cpp
	SVGame/Entities/Items/ItemAmmo9mm.cpp
	SVGame/Entities/Items/ItemHealthMega.cpp
	SVGame/Entities/Items/ItemWeaponKnife.cpp
	SVGame/Entities/Items/ItemWeaponBeretta.cpp
	SVGame/Entities/Items/ItemWeaponNone.cpp
	SVGame/Entities/Items/ItemWeaponSMG.cpp
	SVGame/Entities/Func/FuncAreaportal.cpp
	SVGame/Entities/Func/FuncButton.cpp
	#SVGame/Entities/Func/FuncConveyor.cpp
	SVGame/Entities/Func/FuncDoor.cpp 
	SVGame/Entities/Func/FuncDoorRotating.cpp
	SVGame/Entities/Func/FuncExplosive.cpp 
	#SVGame/Entities/Func/FuncKillbox.cpp
	#SVGame/Entities/Func/FuncObject.cpp
	SVGame/Entities/Func/FuncPlat.cpp
	SVGame/Entities/Func/FuncRotating.cpp
	SVGame/Entities/Func/FuncTimer.cpp
	SVGame/Entities/Func/FuncTrain.cpp 
	SVGame/Entities/Func/FuncWall.cpp
	#SVGame/Entities/Func/FuncWater.cpp
	#SVGame/Entities/Misc/MiscExplobox.cpp
	#SVGame/Entities/Misc/MiscGibArm.cpp 
	#SVGame/Entities/Misc/MiscGibLeg.cpp 
	#SVGame/Entities/Misc/MiscGibHead.cpp 
	SVGame/Entities/Misc/MiscExplosionBox.cpp
	SVGame/Entities/Misc/MiscServerModel.cpp
	#SVGame/Entities/Misc/MiscTeleporter.cpp
	#SVGame/Entities/Misc/MiscTeleporterDest.cpp
	SVGame/Entities/Monsters/MonsterTestDummy.cpp
	SVGame/Entities/Path/PathCorner.cpp
	#SVGame/Entities/Target/TargetBlaster.cpp
	#SVGame/Entities/Target/TargetChangelevel.cpp
	#SVGame/Entities/Target/TargetCrosslevel_target.cpp
	#SVGame/Entities/Target/TargetCrosslevel_trigger.cpp
	SVGame/Entities/Target/TargetEarthquake.cpp
	SVGame/Entities/Target/TargetExplosion.cpp
	#SVGame/Entities/Target/TargetLightramp.cpp
	#SVGame/Entities/Target/TargetSpawner.cpp
	SVGame/Entities/Target/TargetSpeaker.cpp
	#SVGame/Entities/Target/TargetSplash.cpp
	SVGame/Entities/Target/TargetTempEntity.cpp
	SVGame/Entities/Trigger/TriggerCounter.cpp
	#SVGame/Entities/Trigger/TriggerElevator.cpp
	SVGame/Entities/Trigger/TriggerGravity.cpp
	#SVGame/Entities/Trigger/TriggerKey.cpp
	#SVGame/Entities/Trigger/TriggerMonsterjump.cpp
	SVGame/Entities/Trigger/TriggerPush.cpp
	SVGame/Entities/Trigger/TriggerAlways.cpp
	SVGame/Entities/Trigger/TriggerAutoDoor.cpp
	SVGame/Entities/Trigger/TriggerAutoPlatform.cpp
	SVGame/Entities/Trigger/TriggerDelayedUse.cpp
	SVGame/Entities/Trigger/TriggerHurt.cpp
	SVGame/Entities/Trigger/TriggerMultiple.cpp
	SVGame/Entities/Trigger/TriggerOnce.cpp
	SVGame/Entities/Trigger/TriggerRelay.cpp
	SVGame/Entities/Weaponry/BlasterBolt.cpp
	SVGame/Entities/Light.cpp
	SVGame/Entities/Worldspawn.cpp
	SVGame/Gamemodes/DefaultGamemode.cpp
	SVGame/Gamemodes/CoopGamemode.cpp
	SVGame/Gamemodes/DeathmatchGamemode.cpp
	SVGame/Physics/Physics.cpp
	SVGame/Physics/StepMove.cpp
	SVGame/Player/Client.cpp
	SVGame/Player/Hud.cpp
	SVGame/Player/Trail.cpp
	SVGame/Player/Player.cpp
	SVGame/Player/View.cpp
	SVGame/Player/Weapons.cpp
	#SVGame/Weapons/Blaster.cpp
	#SVGame/Weapons/Machinegun.cpp
	#SVGame/Weapons/Shotgun.cpp
	#SVGame/Weapons/SuperShotgun.cpp
	SVGame/World/GameWorld.cpp
    SVGame/SVGame.def
    )
    
    SET ( HEADERS_SVGAME
	SVGame/Ballistics.h
	SVGame/ChaseCamera.h
	SVGame/Effects.h
	SVGame/Entities.h
	SVGame/ServerGameLocals.h
	SVGame/TypeInfo.h
	SVGame/Utilities.h
	SVGame/AI/MonsterAI.h
	SVGame/Entities/Base/SVGBaseEntity.h
	SVGame/Entities/Base/SVGBaseItem.h
	SVGame/Entities/Base/SVGBaseItemAmmo.h
	SVGame/Entities/Base/SVGBaseItemWeapon.h
	SVGame/Entities/Base/SVGBaseTrigger.h
	SVGame/Entities/Base/SVGBaseMonster.h
	SVGame/Entities/Base/SVGBaseMover.h
	SVGame/Entities/Base/SVGBasePlayer.h
	SVGame/Entities/Base/BodyCorpse.h
	SVGame/Entities/Base/DebrisEntity.h
	SVGame/Entities/Base/GibEntity.h
	SVGame/Entities/Func/FuncAreaportal.h
	SVGame/Entities/Func/FuncButton.h
	SVGame/Entities/Func/FuncDoor.h
	SVGame/Entities/Func/FuncDoorRotating.h
	SVGame/Entities/Func/FuncExplosive.h
	SVGame/Entities/Func/FuncPlat.h
	SVGame/Entities/Func/FuncRotating.h
	SVGame/Entities/Func/FuncTimer.h
	SVGame/Entities/Func/FuncTrain.h
	SVGame/Entities/Func/FuncWall.h
	#SVGame/Entities/Info/InfoNull.h - Only has a .cpp file.
	SVGame/Entities/Info/InfoPlayerCoop.h
	SVGame/Entities/Info/InfoPlayerDeathmatch.h
	SVGame/Entities/Info/InfoPlayerIntermission.h
	SVGame/Entities/Info/InfoPlayerStart.h
	
	SVGame/Entities/Items/ItemAmmo9mm.h
	
	SVGame/Entities/Items/ItemHealthMega.h
	
	SVGame/Entities/Items/ItemWeaponKnife.h
	SVGame/Entities/Items/ItemWeaponBeretta.h
	SVGame/Entities/Items/ItemWeaponSMG.h
	SVGame/Entities/Items/ItemWeaponNone.h

	SVGame/Entities/Misc/MiscExplosionBox.h
	SVGame/Entities/Misc/MiscServerModel.h
	SVGame/Entities/Monsters/MonsterTestDummy.h
	SVGame/Entities/Path/PathCorner.h
	SVGame/Entities/Target/TargetEarthquake.h
	SVGame/Entities/Target/TargetExplosion.h
	SVGame/Entities/Target/TargetSpeaker.h
	SVGame/Entities/Target/TargetTempEntity.h
	SVGame/Entities/Trigger/TriggerAlways.h
	SVGame/Entities/Trigger/TriggerAutoDoor.h
	SVGame/Entities/Trigger/TriggerAutoPlatform.h
	SVGame/Entities/Trigger/TriggerCounter.h
	SVGame/Entities/Trigger/TriggerDelayedUse.h
	SVGame/Entities/Trigger/TriggerHurt.h
	SVGame/Entities/Trigger/TriggerGravity.h
	SVGame/Entities/Trigger/TriggerMultiple.h
	SVGame/Entities/Trigger/TriggerOnce.h
	SVGame/Entities/Trigger/TriggerPush.h
	SVGame/Entities/Trigger/TriggerRelay.h
	SVGame/Entities/weaponry/BlasterBolt.h
	SVGame/Entities/IServerGameEntity.h
	SVGame/Entities/Light.h
	SVGame/Entities/Worldspawn.h
	SVGame/Gamemodes/DefaultGamemode.h
	SVGame/Gamemodes/CoopGamemode.h
	SVGame/Gamemodes/DeathmatchGamemode.h
	SVGame/Gamemodes/IGamemode.h
	SVGame/Player/Animations.h
	SVGame/Player/Client.h
	SVGame/Player/Player.h
	SVGame/Player/View.h
	SVGame/Player/Weapons.h
	SVGame/Weapons/Blaster.h
	SVGame/Weapons/Machinegun.h
	SVGame/Weapons/Shotgun.h
	SVGame/Weapons/SuperShotgun.h
	SVGame/World/GameWorld.h
    SVGame/SVGame.def
    )
    
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
	Client/input.cpp
	Client/Keys.cpp
	Client/Locations.cpp
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
	Client/Sound/OGG.cpp
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
	Client/Sound/QAL/fixed.h
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

	# These might have to go some day.
	#Common/netq3/netchan.cpp
	#Common/netq3/net.cpp
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
SET(HEADERS_COMMON
	../inc/Format/Bsp.h
	../inc/Format/Iqm.h
	../inc/Format/Md2.h
	../inc/Format/Md3.h
	../inc/Format/Pak.h
	../inc/Format/Pcx.h
	../inc/Format/Sp2.h
	../inc/Format/Wal.h
)

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

###################### Shared Game Folder.
SET(HEADERS_SHAREDGAME
	../inc/SharedGame/Entities/TypeInfo.h 
	../inc/SharedGame/Entities/ISharedGameEntity.h 
	../inc/SharedGame/Entities/SGEntityHandle.h 
	../inc/SharedGame/Entities.h 
	
	../inc/SharedGame/PMove.h 
	../inc/SharedGame/Protocol.h 
	../inc/SharedGame/SharedGame.h 
	../inc/SharedGame/SkeletalAnimation.h 
)
SET(SRC_SHAREDGAME
	../src/SharedGame/Entities/SGEntityHandle.cpp

	../src/SharedGame/PMove.cpp
	../src/SharedGame/SkeletalAnimation.cpp
)

###################### Shared Folder.
SET(HEADERS_SHARED
	# Container lib.
	../inc/Shared/Containers/IterableCArray.h

	# Interfaces
	../inc/Shared/Interfaces/IClientGameExports.h
	../inc/Shared/Interfaces/IClientGameImports.h

	# Math lib.
	../inc/Shared/Math/color.h
	../inc/Shared/Math/matrix3.h
	../inc/Shared/Math/matrix4.h
	../inc/Shared/Math/plane.h
	../inc/Shared/Math/rectangle.h
	../inc/Shared/Math/utilities.h
	../inc/Shared/Math/vector2.h
	../inc/Shared/Math/vector3.h
	../inc/Shared/Math/vector4.h
	../inc/Shared/Math/vector5.h
	
	# Other shared lib functionalities.
	../inc/Shared/CLGame.h
	../inc/Shared/CLTypes.h
	../inc/Shared/Collision.h
	../inc/Shared/Common.h
	../inc/Shared/Config.h
	../inc/Shared/Endian.h 
	../inc/Shared/SVGame.h 
	../inc/Shared/Keys.h 
	../inc/Shared/List.h 
	../inc/Shared/Math.h 
	../inc/Shared/Messaging.h 
	../inc/Shared/Platform.h
	../inc/Shared/Refresh.h 
	../inc/Shared/QString.h 
	../inc/Shared/Strings.h
	../inc/Shared/Shared.h 
	../inc/Shared/TickRate.h 
	../inc/Shared/UI.h 
)
SET(SRC_SHARED
	Shared/Math/plane.cpp
	Shared/Math/vector3.cpp

	Shared/Math.cpp
	Shared/shared.cpp
)

###################### UNIX Client Specific.
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

###################### Windows Client Specific.
SET(SRC_WINDOWS
	OS/Windows/Debug.cpp
	OS/Windows/Hunk.cpp
	OS/Windows/System.cpp
)
SET(SRC_WINDOWS_CLIENT
	OS/Windows/Wave.cpp
	OS/SDL2/Video.cpp
	OS/SDL2/Time.cpp
)
SET(HEADERS_WINDOWS
	OS/Windows/WGL.h
	OS/Windows/GLImp.h
	OS/Windows/Client.h
	OS/Windows/Threads/Threads.h
)

###################### VKPT.
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

###################### Vulkan Shaders.
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
	refresh/vkpt/shader/normalize_normal_map.comp
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
			compile_shader(SOURCE_FILE ${s} INCLUDES "-I${CMAKE_CURRENT_SOURCE_DIR}/Refresh/vkpt/fsr" OUTPUT_FILE_LIST shader_bytecode)
		endforeach()

		add_custom_target(shaders DEPENDS ${shader_bytecode})

		if(TARGET glslangValidator)
			add_dependencies(shaders glslangValidator)
		endif()
	ENDIF()
endif()

# Embed version number
ADD_DEFINITIONS(-DVERSION_MAJOR=${POLYHEDRON_VERSION_MAJOR})
ADD_DEFINITIONS(-DVERSION_MINOR=${POLYHEDRON_VERSION_MINOR})
ADD_DEFINITIONS(-DVERSION_POINT=${POLYHEDRON_VERSION_POINT})
ADD_DEFINITIONS(-DVERSION_SHA=${POLYHEDRON_VERSION_SHA})
ADD_DEFINITIONS(-DVERSION_BRANCH=${POLYHEDRON_VERSION_BRANCH})

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
	# Add shared game include directory.
	TARGET_INCLUDE_DIRECTORIES(svgame PRIVATE ${HEADERS_SHAREDGAME}/)
endif()

# Optional build target.
if (CONFIG_BUILD_CL_GAME)
	# Used for sharedgame code(such as pmove), so we can ifdef certain parts specific to client or server.	
	ADD_DEFINITIONS(-DSHAREDGAME_CLIENT=1)
	# Add library.
	ADD_LIBRARY(clgame SHARED ${SRC_CLGAME} ${HEADERS_CLGAME} ${SRC_SHARED} ${HEADERS_SHARED} ${SRC_SHAREDGAME} ${HEADERS_SHAREDGAME})
	# Add shared game include directory.
	TARGET_INCLUDE_DIRECTORIES(clgame PRIVATE ${HEADERS_SHAREDGAME}/)
endif()

IF(WIN32)
ADD_EXECUTABLE(client WIN32 
	${SRC_CLIENT} ${HEADERS_CLIENT} 
	${SRC_COMMON} ${HEADERS_COMMON} 
	${SRC_REFRESH} ${SRC_SHADERS} 
	${SRC_SHARED} ${HEADERS_SHARED}
	${SRC_WINDOWS} ${SRC_WINDOWS_CLIENT} ${HEADERS_WINDOWS}
	${SRC_SERVER} ${HEADERS_SERVER}
	OS/Resources/polyhedron.rc
)
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
#TARGET_LINK_LIBRARIES(server enet)

SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_COMMON})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_COMMON})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_CLIENT})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${SRC_SERVER})
SOURCE_GROUP( TREE ${CMAKE_SOURCE_DIR} FILES ${HEADERS_SERVER})

# Old SVGame source grouping - it just bundled *everything* into SVGame/sources and SVGame/headers, eugh -Admer
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
	TARGET_SOURCES(svgame PRIVATE OS/Resources/basepoly_sv.rc)
	TARGET_SOURCES(clgame PRIVATE OS/Resources/basepoly_cl.rc)

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
        OS/Resources/polyhedron.rc 
        OS/Resources/polyhedron.rc )
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
    OUTPUT_NAME "Polyhedron"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin"
    DEBUG_POSTFIX ""
)

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

# Client Game
if (CONFIG_BUILD_CL_GAME)
	# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
	SET_TARGET_PROPERTIES(clgame
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
		PREFIX ""
		DEBUG_POSTFIX ""
	)
endif()

# Server Game
if (CONFIG_BUILD_SV_GAME)
	# Specify both LIBRARY and RUNTIME because one works only on Windows and another works only on Linux
	SET_TARGET_PROPERTIES(svgame
		PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
		LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/bin/basepoly"
		RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/bin/basepoly"
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
