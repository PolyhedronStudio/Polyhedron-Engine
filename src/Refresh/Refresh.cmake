################################################################################################
##  Refresh Subsystems, Shader, Source and Header files.                                                           ##
################################################################################################
####
##	General refresh.
####
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
####
##	Refresh 'OpenGL' Subsystem.
####
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

####
##	Refresh 'Vulkan Path Tracer' Susbsystem.
####
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

function(GenerateShaderSourceLists)
	####
	##	Vulkan Path Tracer Shaders
	####
	list( APPEND SRC_SHADERS
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/animate_materials.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/god_rays_filter.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/god_rays.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/bloom_composite.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/bloom_blur.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/bloom_downscale.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/compositing.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/checkerboard_interleave.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_atrous.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_gradient_atrous.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_gradient_img.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_gradient_reproject.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_lf.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_taau.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/asvgf_temporal.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/instance_geometry.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/normalize_normal_map.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/tone_mapping_histogram.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/tone_mapping_curve.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/tone_mapping_apply.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/physical_sky.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/physical_sky_space.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/shadow_map.vert"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/sky_buffer_resolve.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/stretch_pic.frag"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/stretch_pic.vert"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/final_blit_lanczos.frag"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/final_blit.vert"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/fsr_easu_fp16.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/fsr_easu_fp32.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/fsr_rcas_fp16.comp"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/fsr_rcas_fp32.comp"
	)
	set(SRC_RT_SHADERS
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/primary_rays.rgen"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/direct_lighting.rgen"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/indirect_lighting.rgen"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer.rchit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer.rmiss"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_masked.rahit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_particle.rahit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_sprite.rahit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_beam.rahit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_beam.rint"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/path_tracer_explosion.rahit"
		"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vkpt/shader/reflect_refract.rgen"
	)
endfunction()
GenerateShaderSourceLists()