#############################################################################
##  Shared Source and Header files.
#############################################################################
list( APPEND SRC_SHARED
	"${SRC_SHARED_DIR}/Math/Plane.cpp"
	"${SRC_SHARED_DIR}/Math/Vector3.cpp"

	"${SRC_SHARED_DIR}/Math.cpp"
	"${SRC_SHARED_DIR}/Shared.cpp"
)
list( APPEND HEADERS_SHARED
	# Formats.
	"${SRC_SHARED_DIR}/Formats/Bsp.h" 
	"${SRC_SHARED_DIR}/Formats/Iqm.h" 
	"${SRC_SHARED_DIR}/Formats/Md2.h" 
	"${SRC_SHARED_DIR}/Formats/Md3.h" 
	"${SRC_SHARED_DIR}/Formats/Pak.h" 
	"${SRC_SHARED_DIR}/Formats/Pcx.h" 
	"${SRC_SHARED_DIR}/Formats/Sp2.h" 
	"${SRC_SHARED_DIR}/Formats/Wal.h" 

	# Interfaces
	"${SRC_SHARED_DIR}/Interfaces/IClientGameExports.h"
	"${SRC_SHARED_DIR}/Interfaces/IClientGameImports.h"
	"${SRC_SHARED_DIR}/Interfaces/IServerGameExports.h"
	"${SRC_SHARED_DIR}/Interfaces/IServerGameImports.h"

	# Math lib.
	"${SRC_SHARED_DIR}/Math/Color.h"
	"${SRC_SHARED_DIR}/Math/DualQuaternion.h"
	"${SRC_SHARED_DIR}/Math/Matrix3x3.h"
	"${SRC_SHARED_DIR}/Math/Matrix4x4.h"
	"${SRC_SHARED_DIR}/Math/Plane.h"
	"${SRC_SHARED_DIR}/Math/Rectangle.h"
	"${SRC_SHARED_DIR}/Math/Quaternion.h"
	"${SRC_SHARED_DIR}/Math/Utilities.h"
	"${SRC_SHARED_DIR}/Math/Vector2.h"
	"${SRC_SHARED_DIR}/Math/Vector3.h"
	"${SRC_SHARED_DIR}/Math/Vector4.h"
	"${SRC_SHARED_DIR}/Math/Vector5.h"
	
	# Other shared lib functionalities.
	"${SRC_SHARED_DIR}/CLGame.h"
	"${SRC_SHARED_DIR}/CLTypes.h"
	"${SRC_SHARED_DIR}/CollisionModel.h"
	"${SRC_SHARED_DIR}/Common.h"
	"${SRC_SHARED_DIR}/Config.h"
	"${SRC_SHARED_DIR}/Endian.h" 
	"${SRC_SHARED_DIR}/Entities.h" 
	"${SRC_SHARED_DIR}/EntitySkeleton.h" 
	"${SRC_SHARED_DIR}/Keys.h" 
	"${SRC_SHARED_DIR}/KeyValue.h" 
	"${SRC_SHARED_DIR}/List.h" 
	"${SRC_SHARED_DIR}/Math.h" 
	"${SRC_SHARED_DIR}/Messaging.h" 
	"${SRC_SHARED_DIR}/Platform.h"
	"${SRC_SHARED_DIR}/PlayerMove.h"
	"${SRC_SHARED_DIR}/QString.h"
	"${SRC_SHARED_DIR}/Refresh.h" 
	"${SRC_SHARED_DIR}/Shared.h" 
	"${SRC_SHARED_DIR}/SkeletalModelData.h" 
	"${SRC_SHARED_DIR}/Sound.h" 
	"${SRC_SHARED_DIR}/Strings.h"
	"${SRC_SHARED_DIR}/SVGame.h"
	"${SRC_SHARED_DIR}/TickRate.h"
	"${SRC_SHARED_DIR}/UI.h"
)
#set( SRC_SHARED ${SRC_SHARED} PARENT_SCOPE )
#set( HEADERS_SHARED ${HEADERS_SHARED} PARENT_SCOPE )


#############################################################################
##  System Source and Header files.
#############################################################################
# Operating System specific sources:
if (WIN32)
	# Win32 Specific Client Sources.
	list( APPEND SRC_SYSTEM
		"${SRC_SYSTEM_DIR}/Windows/Debug.cpp"
		"${SRC_SYSTEM_DIR}/Windows/Hunk.cpp"
		"${SRC_SYSTEM_DIR}/Windows/System.cpp"
		
		"${SRC_SYSTEM_DIR}/SDL2/Time.cpp"

	#	"${SRC_SYSTEM_DIR}/Resources/polyhedron.rc"
	)
	list( APPEND SRC_SYSTEM_CLIENT 
		"${SRC_SYSTEM_DIR}/SDL2/Sound.cpp"
		"${SRC_SYSTEM_DIR}/SDL2/Video.cpp"
	)
	# Win32 Specific Client Headers
	list( APPEND HEADERS_SYSTEM
		"${SRC_SYSTEM_DIR}/Windows/WGL.h"
		"${SRC_SYSTEM_DIR}/Windows/GLImp.h"
		"${SRC_SYSTEM_DIR}/Windows/WinClient.h"
		"${SRC_SYSTEM_DIR}/Windows/Threads/Threads.h"
	)

	#set( SRC_SYSTEM ${SRC_SYSTEM} PARENT_SCOPE )
	#set( HEADERS_SYSTEM ${HEADERS_SYSTEM} PARENT_SCOPE )
else()
	# Unix Specific Client Sources.
	list( APPEND SRC_SYSTEM
		"${SRC_SYSTEM_DIR}/Unix/Hunk.cpp"
		"${SRC_SYSTEM_DIR}/Unix/System.cpp"
		"${SRC_SYSTEM_DIR}/Unix/TTY.cpp"
		
		"${SRC_SYSTEM_DIR}/SDL2/Time.cpp"
	)
	list( APPEND SRC_SYSTEM_CLIENT 
		"${SRC_SYSTEM_DIR}/SDL2/Sound.cpp"
		"${SRC_SYSTEM_DIR}/SDL2/Video.cpp"
	)
	list( APPEND HEADERS_SYSTEM )
	# Unix Specific Client Headers
	#list( APPEND HEADERS_CLIENT "someheader.h")

	#set( SRC_SYSTEM ${SRC_SYSTEM} PARENT_SCOPE )
	#set( HEADERS_SYSTEM ${HEADERS_SYSTEM} PARENT_SCOPE )
endif()


#############################################################################
##  Common Source and Header files.
#############################################################################
LIST( APPEND SRC_COMMON
	"${SRC_COMMON_DIR}/Bsp.cpp"
	"${SRC_COMMON_DIR}/Cmd.cpp"
	"${SRC_COMMON_DIR}/CollisionModel.cpp"
	"${SRC_COMMON_DIR}/Common.cpp"
	"${SRC_COMMON_DIR}/CVar.cpp"
	"${SRC_COMMON_DIR}/EntitySkeleton.cpp"
	"${SRC_COMMON_DIR}/Error.cpp"
	"${SRC_COMMON_DIR}/Field.cpp"
	"${SRC_COMMON_DIR}/Fifo.cpp"
	"${SRC_COMMON_DIR}/Files.cpp"
	"${SRC_COMMON_DIR}/Huffman.cpp"
	"${SRC_COMMON_DIR}/MDFour.cpp"
	"${SRC_COMMON_DIR}/Messaging.cpp"
	"${SRC_COMMON_DIR}/Prompt.cpp"
	"${SRC_COMMON_DIR}/SizeBuffer.cpp"
	"${SRC_COMMON_DIR}/SkeletalModelData.cpp"
	"${SRC_COMMON_DIR}/TemporaryBoneCache.cpp"
	#"${SRC_COMMON_DIR}/Tests.cpp"
	"${SRC_COMMON_DIR}/Utilities.cpp"
	"${SRC_COMMON_DIR}/Zone.cpp"

	"${SRC_COMMON_DIR}/Models/Iqm.cpp"

	"${SRC_COMMON_DIR}/Hashes/Crc32.cpp"

	"${SRC_COMMON_DIR}/Messaging/MessageReadWrite.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaClientMoveCommand.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaEntityState.cpp"
	"${SRC_COMMON_DIR}/Messaging/ParseDeltaPlayerState.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaClientMoveCommand.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaEntityState.cpp"
	"${SRC_COMMON_DIR}/Messaging/WriteDeltaPlayerState.cpp"

	"${SRC_COMMON_DIR}/Net/NetChan.cpp"
	"${SRC_COMMON_DIR}/Net/Net.cpp"
)
LIST( APPEND HEADERS_COMMON
	"${SRC_COMMON_DIR}/Bsp.h"
	"${SRC_COMMON_DIR}/Cmd.h"
	"${SRC_COMMON_DIR}/CollisionModel.h"
	"${SRC_COMMON_DIR}/Common.h"
	"${SRC_COMMON_DIR}/CVar.h"
	"${SRC_COMMON_DIR}/EntitySkeleton.h"
	"${SRC_COMMON_DIR}/Error.h"
	"${SRC_COMMON_DIR}/Field.h"
	"${SRC_COMMON_DIR}/Fifo.h"
	"${SRC_COMMON_DIR}/Files.h"
	"${SRC_COMMON_DIR}/HalfFloat.h"
	"${SRC_COMMON_DIR}/Huffman.h"
	"${SRC_COMMON_DIR}/MDFour.h"
	"${SRC_COMMON_DIR}/Messaging.h"
	"${SRC_COMMON_DIR}/PlayerMove.h"
	"${SRC_COMMON_DIR}/Prompt.h"
	"${SRC_COMMON_DIR}/Protocol.h"
	"${SRC_COMMON_DIR}/SizeBuffer.h"
	"${SRC_COMMON_DIR}/SkeletalModelData.h"
	"${SRC_COMMON_DIR}/TemporaryBoneCache.h"
	#"${SRC_COMMON_DIR}/Tests.h"
	"${SRC_COMMON_DIR}/Utilities.h"
	"${SRC_COMMON_DIR}/Zone.h"

	"${SRC_COMMON_DIR}/Net/INetNToP.h"
	"${SRC_COMMON_DIR}/Net/INetPToN.h"
	"${SRC_COMMON_DIR}/Net/Win.h"
	
	"${SRC_COMMON_DIR}/Hashes/Crc32.h"
)
#set( SRC_COMMON ${SRC_COMMON} PARENT_SCOPE )
#et( HEADERS_COMMON ${HEADERS_COMMON} PARENT_SCOPE )



#############################################################################
##  Refresh Subsystems, Shader, Source and Header files. 
#############################################################################
# General refresh.
list( APPEND SRC_REFRESH
	"${SRC_REFRESH_DIR}/images.cpp"
	"${SRC_REFRESH_DIR}/models.cpp"
	#"${SRC_REFRESH_DIR}/model_iqm.cpp"
	"${SRC_REFRESH_DIR}/stb/stb.cpp"
)
list( APPEND HEADERS_REFRESH
	"${SRC_REFRESH_DIR}/Images.h"
	"${SRC_REFRESH_DIR}/Models.h"
	"${SRC_REFRESH_DIR}/Refresh.h"
)
#set( SRC_REFRESH ${SRC_REFRESH} PARENT_SCOPE )
#set( HEADERS_REFRESH ${HEADERS_REFRESH} PARENT_SCOPE )

# Refresh 'OpenGL' Subsystem.
list( APPEND SRC_REFRESH_GL
	"${SRC_REFRESH_DIR}/gl/draw.cpp"
	"${SRC_REFRESH_DIR}/gl/hq2x.cpp"
	"${SRC_REFRESH_DIR}/gl/images.cpp"
	"${SRC_REFRESH_DIR}/gl/main.cpp"
	"${SRC_REFRESH_DIR}/gl/mesh.cpp"
	"${SRC_REFRESH_DIR}/gl/models.cpp"
	"${SRC_REFRESH_DIR}/gl/sky.cpp"
	"${SRC_REFRESH_DIR}/gl/state.cpp"
	"${SRC_REFRESH_DIR}/gl/surf.cpp"
	"${SRC_REFRESH_DIR}/gl/tess.cpp"
	"${SRC_REFRESH_DIR}/gl/world.cpp"
	"${SRC_REFRESH_DIR}/gl/qgl/dynamic.cpp"
#	"${SRC_REFRESH_DIR}/gl/qgl/fixed.c
)
list( APPEND HEADERS_REFRESH_GL
	"${SRC_REFRESH_DIR}/gl/arbfp.h"
	"${SRC_REFRESH_DIR}/gl/gl.h"
)
#set( SRC_REFRESH_GL ${SRC_REFRESH_GL} PARENT_SCOPE )
#set( HEADERS_REFRESH_GL ${HEADERS_REFRESH_GL} PARENT_SCOPE )

# Refresh 'Vulkan Path Tracer' Susbsystem.
list( APPEND SRC_REFRESH_VKPT
	"${SRC_REFRESH_DIR}/vkpt/asvgf.cpp"
	"${SRC_REFRESH_DIR}/vkpt/bloom.cpp"
	"${SRC_REFRESH_DIR}/vkpt/bsp_mesh.cpp"
	"${SRC_REFRESH_DIR}/vkpt/cameras.cpp"
	"${SRC_REFRESH_DIR}/vkpt/conversion.cpp"
	"${SRC_REFRESH_DIR}/vkpt/draw.cpp"
	"${SRC_REFRESH_DIR}/vkpt/freecam.cpp"
	"${SRC_REFRESH_DIR}/vkpt/fog.cpp"
	"${SRC_REFRESH_DIR}/vkpt/fsr.cpp"
	"${SRC_REFRESH_DIR}/vkpt/main.cpp"
	"${SRC_REFRESH_DIR}/vkpt/material.cpp"
	"${SRC_REFRESH_DIR}/vkpt/matrix.cpp"
	"${SRC_REFRESH_DIR}/vkpt/mgpu.cpp"
	"${SRC_REFRESH_DIR}/vkpt/models.cpp"
	"${SRC_REFRESH_DIR}/vkpt/path_tracer.cpp"
	"${SRC_REFRESH_DIR}/vkpt/physical_sky.cpp"
	"${SRC_REFRESH_DIR}/vkpt/precomputed_sky.cpp"
	"${SRC_REFRESH_DIR}/vkpt/profiler.cpp"
	"${SRC_REFRESH_DIR}/vkpt/shadow_map.cpp"
	"${SRC_REFRESH_DIR}/vkpt/textures.cpp"
	"${SRC_REFRESH_DIR}/vkpt/tone_mapping.cpp"
	"${SRC_REFRESH_DIR}/vkpt/transparency.cpp"
	"${SRC_REFRESH_DIR}/vkpt/uniform_buffer.cpp"
	"${SRC_REFRESH_DIR}/vkpt/vertex_buffer.cpp"
	"${SRC_REFRESH_DIR}/vkpt/vk_util.cpp"
	"${SRC_REFRESH_DIR}/vkpt/buddy_allocator.cpp"
	"${SRC_REFRESH_DIR}/vkpt/device_memory_allocator.cpp"
	"${SRC_REFRESH_DIR}/vkpt/god_rays.cpp"
)
list( APPEND HEADERS_REFRESH_VKPT
	"${SRC_REFRESH_DIR}/vkpt/fsr/ffx_a.h"
	"${SRC_REFRESH_DIR}/vkpt/fsr/ffx_fsr1.h"

	"${SRC_REFRESH_DIR}/vkpt/shader/constants.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/global_textures.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/global_ubo.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/god_rays_shared.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/light_lists.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_hit_shaders.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_rgen.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/sky.h"
	"${SRC_REFRESH_DIR}/vkpt/shader/vertex_buffer.h"

	"${SRC_REFRESH_DIR}/vkpt/vkpt.h"
	"${SRC_REFRESH_DIR}/vkpt/vk_util.h"
	"${SRC_REFRESH_DIR}/vkpt/buddy_allocator.h"
	"${SRC_REFRESH_DIR}/vkpt/cameras.h"
	"${SRC_REFRESH_DIR}/vkpt/conversion.h"
	"${SRC_REFRESH_DIR}/vkpt/device_memory_allocator.h"
	"${SRC_REFRESH_DIR}/vkpt/fog.h"
	"${SRC_REFRESH_DIR}/vkpt/material.h"
	"${SRC_REFRESH_DIR}/vkpt/physical_sky.h"
	"${SRC_REFRESH_DIR}/vkpt/precomputed_sky.h"
)
#set( SRC_REFRESH_VKPT ${SRC_REFRESH_VKPT} PARENT_SCOPE )
#set( HEADERS_REFRESH_VKPT ${HEADERS_REFRESH_VKPT} PARENT_SCOPE )

# Vulkan Path Tracer Shaders
list( APPEND SRC_SHADERS
	"${SRC_REFRESH_DIR}/vkpt/shader/animate_materials.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/god_rays_filter.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/god_rays.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/bloom_composite.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/bloom_blur.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/bloom_downscale.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/compositing.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/checkerboard_interleave.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_atrous.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_gradient_atrous.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_gradient_img.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_gradient_reproject.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_lf.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_taau.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/asvgf_temporal.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/instance_geometry.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/normalize_normal_map.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/tone_mapping_histogram.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/tone_mapping_curve.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/tone_mapping_apply.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/physical_sky.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/physical_sky_space.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/shadow_map.vert"
	"${SRC_REFRESH_DIR}/vkpt/shader/sky_buffer_resolve.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/stretch_pic.frag"
	"${SRC_REFRESH_DIR}/vkpt/shader/stretch_pic.vert"
	"${SRC_REFRESH_DIR}/vkpt/shader/final_blit_lanczos.frag"
	"${SRC_REFRESH_DIR}/vkpt/shader/final_blit.vert"
	"${SRC_REFRESH_DIR}/vkpt/shader/fsr_easu_fp16.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/fsr_easu_fp32.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/fsr_rcas_fp16.comp"
	"${SRC_REFRESH_DIR}/vkpt/shader/fsr_rcas_fp32.comp"
)
set(SRC_RT_SHADERS
	"${SRC_REFRESH_DIR}/vkpt/shader/primary_rays.rgen"
	"${SRC_REFRESH_DIR}/vkpt/shader/direct_lighting.rgen"
	"${SRC_REFRESH_DIR}/vkpt/shader/indirect_lighting.rgen"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer.rchit"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer.rmiss"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_masked.rahit"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_particle.rahit"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_sprite.rahit"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_beam.rahit"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_beam.rint"
	"${SRC_REFRESH_DIR}/vkpt/shader/path_tracer_explosion.rahit"
	"${SRC_REFRESH_DIR}/vkpt/shader/reflect_refract.rgen"
)
set( SRC_SHADERS ${SRC_SHADERS} ) #PARENT_SCOPE )
set( SRC_RT_SHADERS ${SRC_RT_SHADERS} ) # PARENT_SCOPE )



#############################################################################
##  Server Source and Header files.
#############################################################################
list( APPEND SRC_SERVER
	"${SRC_SERVER_DIR}/Commands.cpp"
	"${SRC_SERVER_DIR}/Entities.cpp"
	"${SRC_SERVER_DIR}/Init.cpp"
	"${SRC_SERVER_DIR}/Main.cpp"
	"${SRC_SERVER_DIR}/Models.cpp"
	"${SRC_SERVER_DIR}/Save.cpp"
	"${SRC_SERVER_DIR}/Send.cpp"
	"${SRC_SERVER_DIR}/SVGame.cpp"
	"${SRC_SERVER_DIR}/User.cpp"
	"${SRC_SERVER_DIR}/World.cpp"

)
list( APPEND HEADERS_SERVER
	"${SRC_SERVER_DIR}/Models.h"	
	"${SRC_SERVER_DIR}/Server.h"
)
set( SRC_SERVER ${SRC_SERVER} ) #PARENT_SCOPE )
set( HEADERS_SERVER ${HEADERS_SERVER} ) #PARENT_SCOPE )



#############################################################################
##  Client Source and Header files.
#############################################################################
list( APPEND SRC_CLIENT
	"${SRC_CLIENT_DIR}/ASCII.cpp"
	"${SRC_CLIENT_DIR}/Console.cpp"
	"${SRC_CLIENT_DIR}/Cinematic.cpp"
	"${SRC_CLIENT_DIR}/ClientGame.cpp"
	"${SRC_CLIENT_DIR}/CRC.cpp"
	"${SRC_CLIENT_DIR}/Demo.cpp"
	"${SRC_CLIENT_DIR}/Download.cpp"
	"${SRC_CLIENT_DIR}/Entities.cpp"
	"${SRC_CLIENT_DIR}/Input.cpp"
	"${SRC_CLIENT_DIR}/Keys.cpp"
	"${SRC_CLIENT_DIR}/Main.cpp"
	"${SRC_CLIENT_DIR}/Models.cpp"
	"${SRC_CLIENT_DIR}/Parse.cpp"
	"${SRC_CLIENT_DIR}/Precache.cpp"
	"${SRC_CLIENT_DIR}/Predict.cpp"
	"${SRC_CLIENT_DIR}/Refresh.cpp"
	"${SRC_CLIENT_DIR}/Screen.cpp"
	"${SRC_CLIENT_DIR}/Traces.cpp"
	"${SRC_CLIENT_DIR}/View.cpp"
	"${SRC_CLIENT_DIR}/World.cpp"

	"${SRC_CLIENT_DIR}/Entities/LocalEntities.cpp"
	"${SRC_CLIENT_DIR}/Entities/PacketEntities.cpp"

	"${SRC_CLIENT_DIR}/UI/Demos.cpp"
	"${SRC_CLIENT_DIR}/UI/Menu.cpp"
	"${SRC_CLIENT_DIR}/UI/PlayerConfig.cpp"
	"${SRC_CLIENT_DIR}/UI/PlayerModels.cpp"
	"${SRC_CLIENT_DIR}/UI/Script.cpp"
	"${SRC_CLIENT_DIR}/UI/Servers.cpp"
	"${SRC_CLIENT_DIR}/UI/UI.cpp"

	"${SRC_CLIENT_DIR}/Sound/DirectMemoryAccess.cpp"
	"${SRC_CLIENT_DIR}/Sound/OpenAL.cpp"
	"${SRC_CLIENT_DIR}/Sound/Main.cpp"
	"${SRC_CLIENT_DIR}/Sound/Memory.cpp"
	"${SRC_CLIENT_DIR}/Sound/Mix.cpp"
	"${SRC_CLIENT_DIR}/Sound/Ogg.cpp"
	
	#${CMAKE_SOURCE_DIR}/Sound/QAL/fixed.c
	"${SRC_CLIENT_DIR}/Sound/QAL/Dynamic.cpp"
)
# Client main headers.
list( APPEND HEADERS_CLIENT
	"${SRC_CLIENT_DIR}/Client.h"
	#${CMAKE_SOURCE_DIR}/ClientGame.h"
	"${SRC_CLIENT_DIR}/Entities.h"
	"${SRC_CLIENT_DIR}/GameModule.h"
	"${SRC_CLIENT_DIR}/Input.h"
	"${SRC_CLIENT_DIR}/Keys.h"
	"${SRC_CLIENT_DIR}/Models.h"
	"${SRC_CLIENT_DIR}/Traces.h"
	"${SRC_CLIENT_DIR}/UI.h"
	"${SRC_CLIENT_DIR}/Video.h"
	"${SRC_CLIENT_DIR}/World.h"

	"${SRC_CLIENT_DIR}/Entities/LocalEntities.h"
	"${SRC_CLIENT_DIR}/Entities/PacketEntities.h"

	"${SRC_CLIENT_DIR}/UI/UI.h"

	"${SRC_CLIENT_DIR}/Sound/Sound.h"
	
	"${SRC_CLIENT_DIR}/Sound/QAL/Dynamic.h"
	"${SRC_CLIENT_DIR}/Sound/QAL/Fixed.h"
)

# If Enabled: CURL Support sources.
if ( CONFIG_USE_CURL ) 
	list( 
		APPEND SRC_CLIENT
		"${SRC_CLIENT_DIR}/HTTP.cpp"
	)
endif()
# Placeholder for linkage without vkpt enabled.
if ( NOT CONFIG_BUILD_GL_RENDERER AND CONFIG_BUILD_VKPT_RENDERER)
	list( 
		APPEND SRC_CLIENT
		"${SRC_CLIENT_DIR}/NullGL.cpp"
	)
endif()
# If Enabled: RmlUI sources.
if ( CONFIG_ENABLE_RMLUI )
	list( APPEND SRC_CLIENT 
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/FileInterface.cpp"
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/FileInterface.h"
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/RenderInterface.cpp"
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/RenderInterface.h"
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/SystemInterface.cpp"
		"${SRC_CLIENT_DIR}/RmlUI/Interfaces/SystemInterface.h"

		"${SRC_CLIENT_DIR}/RmlUI/RmlUI.cpp"
		"${SRC_CLIENT_DIR}/RmlUI/RmlUI.h"
	)
else()
	list( APPEND SRC_CLIENT
		"${SRC_CLIENT_DIR}/RmlUI/RmlUINull.cpp"
		"${SRC_CLIENT_DIR}/RmlUI/RmlUI.h"
	)
endif()



#############################################################################
##  SharedGame Source and Header files.
#############################################################################
list( APPEND SRC_GAME_SHARED
	"${SRC_GAME_SHARED_DIR}/Entities/SGEntityHandle.cpp"
	"${SRC_GAME_SHARED_DIR}/Entities/SGBaseItem.cpp"
	"${SRC_GAME_SHARED_DIR}/Entities/EntityFilters.cpp"

	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypeRootMotionMove.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypeLinearProjectile.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypeNoClip.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypeNone.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypePusher.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/MoveTypes/MoveTypeToss.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/Physics.cpp"
	"${SRC_GAME_SHARED_DIR}/Physics/RootMotionMove.cpp"

	"${SRC_GAME_SHARED_DIR}/World/IGameWorld.cpp"

	"${SRC_GAME_SHARED_DIR}/PlayerMove.cpp"
	"${SRC_GAME_SHARED_DIR}/SkeletalAnimation.cpp" 
	"${SRC_GAME_SHARED_DIR}/Tracing.cpp"
)
list( APPEND HEADERS_GAME_SHARED
	"${SRC_GAME_SHARED_DIR}/Entities/EntityFilters.h"
	"${SRC_GAME_SHARED_DIR}/Entities/ISharedGameEntity.h" 
	"${SRC_GAME_SHARED_DIR}/Entities/SGEntityHandle.h"  
	"${SRC_GAME_SHARED_DIR}/Entities/SGBaseItem.h"
	"${SRC_GAME_SHARED_DIR}/Entities/TypeInfo.h" 
	"${SRC_GAME_SHARED_DIR}/Entities.h" 

	"${SRC_GAME_SHARED_DIR}/Physics/Physics.h"
	"${SRC_GAME_SHARED_DIR}/Physics/RootMotionMove.h"

	"${SRC_GAME_SHARED_DIR}/World/IGameWorld.h"
	
	"${SRC_GAME_SHARED_DIR}/ButtonBits.h"
	"${SRC_GAME_SHARED_DIR}/Entities.h"
	"${SRC_GAME_SHARED_DIR}/EntityEffectTypes.h"
	"${SRC_GAME_SHARED_DIR}/EntityFlags.h"
	"${SRC_GAME_SHARED_DIR}/GameModeFlags.h"
	"${SRC_GAME_SHARED_DIR}/ItemIDs.h"
	"${SRC_GAME_SHARED_DIR}/MuzzleFlashTypes.h"
	"${SRC_GAME_SHARED_DIR}/PlayerMove.h" 
	"${SRC_GAME_SHARED_DIR}/Protocol.h" 
	"${SRC_GAME_SHARED_DIR}/RenderDrawFlags.h"
	"${SRC_GAME_SHARED_DIR}/SharedGame.h" 
	"${SRC_GAME_SHARED_DIR}/SkeletalAnimation.h" 
	"${SRC_GAME_SHARED_DIR}/TempEntityEvents.h"
	"${SRC_GAME_SHARED_DIR}/Time.h"
	"${SRC_GAME_SHARED_DIR}/Tracing.h"
	"${SRC_GAME_SHARED_DIR}/Utilities.h"
	"${SRC_GAME_SHARED_DIR}/WaterLevels.h"
	"${SRC_GAME_SHARED_DIR}/WeaponStates.h"
)
#set( SRC_GAME_SHARED ${SRC_GAME_SHARED} PARENT_SCOPE )
#set( HEADERS_GAME_SHARED ${HEADERS_GAME_SHARED} PARENT_SCOPE )



#############################################################################
##  ClientGame Source and Header files.
#############################################################################
if( CONFIG_BUILD_GAME_CLIENT )
	# Specify Source files.
	SET(SRC_GAME_CLIENT
		"${SRC_GAME_CLIENT_DIR}/ClientGameExports.cpp"
		"${SRC_GAME_CLIENT_DIR}/ClientGameImports.cpp"
		"${SRC_GAME_CLIENT_DIR}/ClientGameLocals.cpp"
		"${SRC_GAME_CLIENT_DIR}/ClientGameMain.cpp"

		"${SRC_GAME_CLIENT_DIR}/Effects/DynamicLights.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/LightStyles.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/MuzzleFlashEffects.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/BloodSplatters.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/BubbleTrailA.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/BubbleTrailB.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/DiminishingTrail.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/DirtAndSparks.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/ExplosionSparks.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/ForceWall.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/HeatBeam.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/ItemRespawn.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/LogoutEffect.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/MonsterPlasmaShell.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/SteamPuffs.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/Teleporter.cpp"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles/WaterSplash.cpp"

		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseLocalEntity.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBasePacketEntity.cpp"

		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseMover.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBasePlayer.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseTrigger.cpp"

		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncDoor.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncDoorRotating.cpp"	
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncPlat.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncRotating.cpp"

		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscExplosionBox.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscClientExplosionBox.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscClientModel.cpp"
	
		"${SRC_GAME_CLIENT_DIR}/Entities/Monsters/MonsterTestDummy.cpp"

		#"${SRC_GAME_CLIENT_DIR}/Entities/GameEntityList.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/IClientGameEntity.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/DebrisEntity.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/GibEntity.cpp"
		"${SRC_GAME_CLIENT_DIR}/Entities/Worldspawn.cpp"

		"${SRC_GAME_CLIENT_DIR}/Utilities/CLGTraceResult.cpp"

		"${SRC_GAME_CLIENT_DIR}/World/ClientGameWorld.cpp"

		"${SRC_GAME_CLIENT_DIR}/Exports/Core.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/Entities.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/Media.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/Movement.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/Prediction.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/Screen.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/ServerMessage.cpp"
		"${SRC_GAME_CLIENT_DIR}/Exports/View.cpp"

		"${SRC_GAME_CLIENT_DIR}/HUD/ChatHUD.cpp"
		"${SRC_GAME_CLIENT_DIR}/HUD/NumberHUD.cpp"

		"${SRC_GAME_CLIENT_DIR}/Input/KeyBinding.cpp"

		"${SRC_GAME_CLIENT_DIR}/View/ViewCamera.cpp"

		"${SRC_GAME_CLIENT_DIR}/Debug.cpp"		
		"${SRC_GAME_CLIENT_DIR}/TemporaryEntities.cpp"
	)
	# Specify Header files.
	SET(HEADERS_GAME_CLIENT
		"${SRC_GAME_CLIENT_DIR}/ClientGameExports.h"
		"${SRC_GAME_CLIENT_DIR}/ClientGameImports.h"
		"${SRC_GAME_CLIENT_DIR}/ClientGameLocals.h"
		"${SRC_GAME_CLIENT_DIR}/ClientGameMain.h"

		"${SRC_GAME_CLIENT_DIR}/Effects/DynamicLights.h"
		"${SRC_GAME_CLIENT_DIR}/Effects/LightStyles.h"
		"${SRC_GAME_CLIENT_DIR}/Effects/MuzzleFlashEffects.h"
		"${SRC_GAME_CLIENT_DIR}/Effects/ParticleEffects.h"
		"${SRC_GAME_CLIENT_DIR}/Effects/Particles.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseLocalEntity.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBasePacketEntity.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseMover.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBasePlayer.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Base/CLGBaseTrigger.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncDoor.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncDoorRotating.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncPlat.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Func/FuncRotating.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscExplosionBox.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscClientExplosionBox.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Misc/MiscClientModel.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/Monsters/MonsterTestDummy.h"

		"${SRC_GAME_CLIENT_DIR}/Entities/IClientGameEntity.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/DebrisEntity.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/GibEntity.h"
		"${SRC_GAME_CLIENT_DIR}/Entities/Worldspawn.h"

		"${SRC_GAME_CLIENT_DIR}/Exports/Core.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/Entities.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/Media.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/Movement.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/Prediction.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/Screen.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/ServerMessage.h"
		"${SRC_GAME_CLIENT_DIR}/Exports/View.h"

		"${SRC_GAME_CLIENT_DIR}/HUD/ChatHUD.h"
		"${SRC_GAME_CLIENT_DIR}/HUD/NumberHUD.h"

		"${SRC_GAME_CLIENT_DIR}/Input/KeyBinding.h"
	
		"${SRC_GAME_CLIENT_DIR}/View/ViewCamera.h"

		"${SRC_GAME_CLIENT_DIR}/Utilities/CLGTraceResult.h"

		"${SRC_GAME_CLIENT_DIR}/World/ClientGameWorld.h"

		"${SRC_GAME_CLIENT_DIR}/Debug.h"		
		"${SRC_GAME_CLIENT_DIR}/TemporaryEntities.h"
	)
endif()


#############################################################################
##  ServerGame Source and Header files.
#############################################################################
if( CONFIG_BUILD_GAME_SERVER )
	# Specify Source files.
	SET(SRC_GAME_SERVER
		"${SRC_GAME_SERVER_DIR}/Ballistics.cpp"
		"${SRC_GAME_SERVER_DIR}/ChaseCamera.cpp"
		"${SRC_GAME_SERVER_DIR}/Commands.cpp"
		"${SRC_GAME_SERVER_DIR}/Effects.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities.cpp"
		"${SRC_GAME_SERVER_DIR}/ServerGameExports.cpp"
		"${SRC_GAME_SERVER_DIR}/ServerGameImports.cpp"
		"${SRC_GAME_SERVER_DIR}/ServerGameLocals.cpp"
		"${SRC_GAME_SERVER_DIR}/ServerGameMain.cpp"
		"${SRC_GAME_SERVER_DIR}/ImportsWrapper.cpp"
		"${SRC_GAME_SERVER_DIR}/Effects.cpp"
		"${SRC_GAME_SERVER_DIR}/Save.cpp"
		"${SRC_GAME_SERVER_DIR}/SVCommands.cpp"
		"${SRC_GAME_SERVER_DIR}/Utilities.cpp"
		"${SRC_GAME_SERVER_DIR}/Weapons.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseEntity.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItem.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItemAmmo.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItemWeapon.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseTrigger.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseRootMotionMonster.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseStepMonster.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseMover.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBasePlayer.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseSkeletalAnimator.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/BodyCorpse.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/DebrisEntity.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/GibEntity.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncAreaportal.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncButton.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncConveyor.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncDoor.cpp" 
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncDoorRotating.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncExplosive.cpp" 
		#"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncKillbox.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncObject.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncPlat.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncRotating.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncTimer.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncTrain.cpp" 
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncWall.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncWater.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoNotNull.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoNull.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerCoop.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerDeathmatch.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerIntermission.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerStart.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemAmmo9mm.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemHealthMega.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponKnife.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponBeretta.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponNone.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponSMG.cpp"

		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscExplobox.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscGibArm.cpp" 
		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscGibLeg.cpp" 
		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscGibHead.cpp" 
		"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscExplosionBox.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscServerModel.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscTeleporter.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscTeleporterDest.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Monsters/MonsterStepDummy.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Monsters/MonsterTestDummy.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Path/PathCorner.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Path/PathMonsterGoal.cpp"

		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetBlaster.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetChangelevel.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetCrosslevel_target.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetCrosslevel_trigger.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetEarthquake.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetExplosion.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetLightramp.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetSpawner.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetSpeaker.cpp"
		#Entities/Target/TargetSplash.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetTempEntity.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerCounter.cpp"
		#Entities/Trigger/TriggerElevator.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerGravity.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerKey.cpp"
		#"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerMonsterjump.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerPush.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAlways.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAutoDoor.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAutoPlatform.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerDelayedUse.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerHurt.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerMultiple.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerOnce.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerRelay.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Weaponry/BlasterBolt.cpp"

		"${SRC_GAME_SERVER_DIR}/Entities/Light.cpp"
		"${SRC_GAME_SERVER_DIR}/Entities/Worldspawn.cpp"

		"${SRC_GAME_SERVER_DIR}/Exports/Client.cpp"
		"${SRC_GAME_SERVER_DIR}/Exports/Core.cpp"
		"${SRC_GAME_SERVER_DIR}/Exports/Entities.cpp"
		"${SRC_GAME_SERVER_DIR}/Exports/GameState.cpp"
		"${SRC_GAME_SERVER_DIR}/Exports/LevelState.cpp"

		"${SRC_GAME_SERVER_DIR}/Gamemodes/DefaultGamemode.cpp"
		"${SRC_GAME_SERVER_DIR}/GameModes/CoopGameMode.cpp"
		"${SRC_GAME_SERVER_DIR}/Gamemodes/DeathMatchGamemode.cpp"

		"${SRC_GAME_SERVER_DIR}/Physics/Physics.cpp"
		"${SRC_GAME_SERVER_DIR}/Physics/StepMove.cpp"

		"${SRC_GAME_SERVER_DIR}/Player/Client.cpp"
		"${SRC_GAME_SERVER_DIR}/Player/Hud.cpp"

		"${SRC_GAME_SERVER_DIR}/Utilities/SVGTraceResult.cpp"

		"${SRC_GAME_SERVER_DIR}/World/ServerGameWorld.cpp"
		#"${SRC_GAME_SERVER_DIR}/SVGame.def
	)
	# Specify Header files.
	SET(HEADERS_GAME_SERVER
		"${SRC_GAME_SERVER_DIR}/Ballistics.h"
		"${SRC_GAME_SERVER_DIR}/ChaseCamera.h"
		"${SRC_GAME_SERVER_DIR}/Effects.h"
		"${SRC_GAME_SERVER_DIR}/Entities.h"
		"${SRC_GAME_SERVER_DIR}/ServerGameExports.h"
		"${SRC_GAME_SERVER_DIR}/ServerGameImports.h"
		"${SRC_GAME_SERVER_DIR}/ServerGameLocals.h"
		"${SRC_GAME_SERVER_DIR}/ServerGameMain.h"
		"${SRC_GAME_SERVER_DIR}/TypeInfo.h"
		"${SRC_GAME_SERVER_DIR}/Utilities.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseEntity.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItem.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItemAmmo.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseItemWeapon.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseTrigger.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseRootMotionMonster.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseStepMonster.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBaseMover.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/SVGBasePlayer.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/BodyCorpse.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/DebrisEntity.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Base/GibEntity.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncAreaportal.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncButton.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncDoor.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncDoorRotating.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncExplosive.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncPlat.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncRotating.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncTimer.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncTrain.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Func/FuncWall.h"

		#"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoNull.h" - Only has a .cpp file.
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoNotNull.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerCoop.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerDeathmatch.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerIntermission.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Info/InfoPlayerStart.h"
	
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemAmmo9mm.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemHealthMega.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponKnife.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponBeretta.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponSMG.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Items/ItemWeaponNone.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscExplosionBox.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Misc/MiscServerModel.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Monsters/MonsterStepDummy.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Monsters/MonsterTestDummy.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Path/PathCorner.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetEarthquake.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetExplosion.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetSpeaker.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Target/TargetTempEntity.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAlways.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAutoDoor.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerAutoPlatform.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerCounter.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerDelayedUse.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerHurt.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerGravity.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerMultiple.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerOnce.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerPush.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Trigger/TriggerRelay.h"

		"${SRC_GAME_SERVER_DIR}/Entities/Weaponry/BlasterBolt.h"

		"${SRC_GAME_SERVER_DIR}/Entities/IServerGameEntity.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Light.h"
		"${SRC_GAME_SERVER_DIR}/Entities/Worldspawn.h"

		"${SRC_GAME_SERVER_DIR}/Exports/Client.h"
		"${SRC_GAME_SERVER_DIR}/Exports/Core.h"
		"${SRC_GAME_SERVER_DIR}/Exports/Entities.h"
		"${SRC_GAME_SERVER_DIR}/Exports/GameState.h"
		"${SRC_GAME_SERVER_DIR}/Exports/LevelState.h"

		"${SRC_GAME_SERVER_DIR}/Gamemodes/DefaultGamemode.h"
		"${SRC_GAME_SERVER_DIR}/GameModes/CoopGamemode.h"
		"${SRC_GAME_SERVER_DIR}/Gamemodes/DeathMatchGamemode.h"
		"${SRC_GAME_SERVER_DIR}/Gamemodes/IGamemode.h"

		"${SRC_GAME_SERVER_DIR}/Player/Animations.h"
		"${SRC_GAME_SERVER_DIR}/Player/Client.h"

		"${SRC_GAME_SERVER_DIR}/ServerGameExports.h"

		"${SRC_GAME_SERVER_DIR}/Utilities/SVGTraceResult.h"

		"${SRC_GAME_SERVER_DIR}/World/ServerGameWorld.h"
		#"${SRC_GAME_SERVER_DIR}/SVGame.def
	)
endif()