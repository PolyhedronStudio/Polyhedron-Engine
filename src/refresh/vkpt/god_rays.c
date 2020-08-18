/*
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "shared/shared.h"
#include "vkpt.h"
#include "vk_util.h"

static const uint32_t THREAD_GROUP_SIZE = 16;
static const uint32_t FILTER_THREAD_GROUP_SIZE = 16;

struct
{
	VkPipeline pipelines[2];
	VkPipelineLayout pipeline_layout;
	VkDescriptorSetLayout descriptor_set_layout;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSet descriptor_set;

	VkImageView shadow_image_view;
	VkSampler shadow_sampler;

	cvar_t* intensity;
	cvar_t* eccentricity;
	cvar_t* enable;

	cvar_t* fogEnable;
	cvar_t* fogTintColorRed;
	cvar_t* fogTintColorGreen;
	cvar_t* fogTintColorBlue;
	cvar_t* fogTintPower;
	cvar_t* fogDensityRoot;
	cvar_t* fogPushBackDist;

} god_rays;

static void create_image_views();
static void create_pipeline_layout();
static void create_pipelines();
static void create_descriptor_set();
static void update_descriptor_set();

extern cvar_t *physical_sky_space;

#define MAX_CSV_VALUES 32
typedef struct CSV_values_s {
	char * values[MAX_CSV_VALUES];
	int num_values;
} CSV_values_t;

static qerror_t getStringValue(CSV_values_t const * csv, int index, char * dest)
{
	if (index >= csv->num_values)
		return Q_ERR_FAILURE;
	strncpy(dest, csv->values[index], MAX_QPATH);
	return Q_ERR_SUCCESS;
}

static qerror_t getFloatValue(CSV_values_t const * csv, int index, float * dest)
{
	dest[0] = '\0';
	if (index >= csv->num_values || csv->values[index][0] == '\0')
		return Q_ERR_FAILURE;
	*dest = atof(csv->values[index]);
	return Q_ERR_SUCCESS;
}

static qerror_t getIntValue(CSV_values_t const * csv, int index, int * dest)
{
	if (index >= csv->num_values || csv->values[index][0] == '\0')
		return Q_ERR_FAILURE;
	*dest = atoi(csv->values[index]);
	return Q_ERR_SUCCESS;
}

static qerror_t getFlagValue(CSV_values_t const * csv, int index, uint32_t * flags, uint32_t mask)
{
	if (index >= csv->num_values || csv->values[index][0] == '\0')
		return Q_ERR_FAILURE;
	if (atoi(csv->values[index]))
		*flags = *flags | mask;
	return Q_ERR_SUCCESS;
}

static qerror_t parse_CSV_line(char * linebuf, CSV_values_t * csv)
{
	static char delim = ',';

	char * cptr = linebuf, c;
	int inquote = 0;

	int index = 0;
	csv->values[index] = cptr;

	while ((c = *cptr) != '\0' && index < MAX_CSV_VALUES)
	{
		if (c == '"')
		{
			if (!inquote)
				csv->values[index] = cptr + 1;
			else
				*cptr = '\0';
			inquote = !inquote;
		}
		else if (c == delim && !inquote)
		{
			*cptr = '\0';
			csv->values[++index] = cptr + 1;
		}
		cptr++;
	}
	csv->num_values = index + 1;
	return Q_ERR_SUCCESS;
}

typedef struct fog_user_item_s{
	char filename[512];
	int enabled;
	float tintRed;
	float tintGreen;
	float tintBlue;
	float tintPower;
	float densityRoot;
	float pushBackDist;

} fog_user_item_s_t;

typedef struct fog_user_table_s {
	fog_user_item_s_t fog[512];
	int numberFogItems;
} fog_user_table_s_t;

static qerror_t LoadFogByMapTable(char const * filename, fog_user_table_s_t* table)
{
	qerror_t status = Q_ERR_SUCCESS;
	
	table->numberFogItems = 0;

	byte * buffer = NULL; ssize_t buffer_size = 0;
	buffer_size = FS_LoadFile(filename, (void**)&buffer);
	if (buffer == NULL)
	{
		Com_EPrintf("cannot load fogbymap.csv table '%s'\n", filename);
		return Q_ERR_FAILURE;
	}
		
	int currentLine = 0;
	char const * ptr = (char const *)buffer;
	char linebuf[MAX_QPATH * 3];
	while (sgets(linebuf, sizeof(linebuf), &ptr))
	{
		// skip first line because it contains the column names
		if (currentLine == 0 && strncmp(linebuf, "MapName", 7) == 0)
			continue;

		fog_user_item_s_t * fogItem = &table->fog[table->numberFogItems];

		fogItem->enabled = 0;
		fogItem->tintRed = 1;
		fogItem->tintGreen = 1;
		fogItem->tintBlue = 1;
		fogItem->tintPower = 1;
		fogItem->densityRoot = 1;
		fogItem->pushBackDist = 1;
		memset(fogItem->filename, sizeof(fogItem->filename), 0);

		CSV_values_t csv = { 0 };

		if (parse_CSV_line(linebuf, &csv) == Q_ERR_SUCCESS)
		{
			if (getStringValue(&csv, 0, fogItem->filename) == Q_ERR_SUCCESS && fogItem->filename[0] != '\0')
			{
				status |= getIntValue(&csv, 1, &fogItem->enabled);
				status |= getFloatValue(&csv, 2, &fogItem->tintRed);
				status |= getFloatValue(&csv, 3, &fogItem->tintGreen);
				status |= getFloatValue(&csv, 4, &fogItem->tintBlue);
				status |= getFloatValue(&csv, 5, &fogItem->tintPower);
				status |= getFloatValue(&csv, 6, &fogItem->densityRoot);
				status |= getFloatValue(&csv, 7, &fogItem->pushBackDist);
			}
			else
				status = Q_ERR_FAILURE;
		}
		if (status == Q_ERR_SUCCESS)
		{
			++table->numberFogItems;
		}
		++currentLine;
	}
		
	FS_FreeFile(buffer);

	return status;
}

static fog_user_table_s_t fogbynametable;

VkResult vkpt_initialize_god_rays()
{
	memset(&god_rays, 0, sizeof(god_rays));

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(qvk.physical_device, &properties);

	god_rays.intensity = Cvar_Get("gr_intensity", "2.0", 0);
	god_rays.eccentricity = Cvar_Get("gr_eccentricity", "0.75", 0);
	god_rays.enable = Cvar_Get("gr_enable", "1", 0);

	god_rays.fogEnable = Cvar_Get("gr_enablefog", "1", 0);
	god_rays.fogTintColorRed = Cvar_Get("gr_fogtintr", "1.0", 0);
	god_rays.fogTintColorGreen = Cvar_Get("gr_fogtintg", "1.0", 0);
	god_rays.fogTintColorBlue = Cvar_Get("gr_fogtintb", "1.0", 0);
	god_rays.fogTintPower = Cvar_Get("gr_fogtintpow", "0.17", 0);
	god_rays.fogDensityRoot = Cvar_Get("gr_fogdensity", "0.08", 0);
	god_rays.fogPushBackDist = Cvar_Get("gr_fogpushback", "0.00011", 0);
		   	 
	// Load fog by map csv file make stucture
	fogbynametable.numberFogItems = 0;
	LoadFogByMapTable("fogbymap.csv", &fogbynametable);

	return VK_SUCCESS;
}

void SetFogByMap(const char* name)
{
	for (int i = 0; i < fogbynametable.numberFogItems; i++)
	{
		if (strcmp(name, fogbynametable.fog[i].filename) == 0)
		{
			god_rays.fogEnable = Cvar_Set("gr_enablefog", va("%d", fogbynametable.fog[i].enabled));
			god_rays.fogTintColorRed = Cvar_Set("gr_fogtintr", va("%f", fogbynametable.fog[i].tintRed));
			god_rays.fogTintColorGreen = Cvar_Set("gr_fogtintg", va("%f", fogbynametable.fog[i].tintGreen));
			god_rays.fogTintColorBlue =	Cvar_Set("gr_fogtintb", va("%f", fogbynametable.fog[i].tintBlue));
			god_rays.fogTintPower =	Cvar_Set("gr_fogtintpow", va("%f", fogbynametable.fog[i].tintPower));
			god_rays.fogDensityRoot = Cvar_Set("gr_fogdensity", va("%f", fogbynametable.fog[i].densityRoot));
			god_rays.fogPushBackDist = Cvar_Set("gr_fogpushback", va("%f", fogbynametable.fog[i].pushBackDist));
						
			break;
		}
	}
}

VkResult vkpt_destroy_god_rays()
{
	vkDestroySampler(qvk.device, god_rays.shadow_sampler, NULL);
	vkDestroyDescriptorPool(qvk.device, god_rays.descriptor_pool, NULL);

	return VK_SUCCESS;
}

VkResult vkpt_god_rays_create_pipelines()
{
	create_pipeline_layout();
	create_pipelines();
	create_descriptor_set();
	
	// this is a noop outside a shader reload
	update_descriptor_set();

	return VK_SUCCESS;
}

VkResult vkpt_god_rays_destroy_pipelines()
{
	for (size_t i = 0; i < LENGTH(god_rays.pipelines); i++) {
		if (god_rays.pipelines[i]) {
			vkDestroyPipeline(qvk.device, god_rays.pipelines[i], NULL);
			god_rays.pipelines[i] = NULL;
		}
	}

	if (god_rays.pipeline_layout) {
		vkDestroyPipelineLayout(qvk.device, god_rays.pipeline_layout, NULL);
		god_rays.pipeline_layout = NULL;
	}
	
	if (god_rays.descriptor_set_layout) {
		vkDestroyDescriptorSetLayout(qvk.device, god_rays.descriptor_set_layout, NULL);
		god_rays.descriptor_set_layout = NULL;
	}

	return VK_SUCCESS;
}

VkResult
vkpt_god_rays_update_images()
{
	create_image_views();
	update_descriptor_set();
	return VK_SUCCESS;
}

VkResult
vkpt_god_rays_noop()
{
	return VK_SUCCESS;
}

#define BARRIER_COMPUTE(cmd_buf, img) \
	do { \
		VkImageSubresourceRange subresource_range = { \
			.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT, \
			.baseMipLevel   = 0, \
			.levelCount     = 1, \
			.baseArrayLayer = 0, \
			.layerCount     = 1 \
		}; \
		IMAGE_BARRIER(cmd_buf, \
				.image            = img, \
				.subresourceRange = subresource_range, \
				.srcAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.dstAccessMask    = VK_ACCESS_SHADER_WRITE_BIT, \
				.oldLayout        = VK_IMAGE_LAYOUT_GENERAL, \
				.newLayout        = VK_IMAGE_LAYOUT_GENERAL, \
		); \
	} while(0)

void vkpt_record_god_rays_trace_command_buffer(VkCommandBuffer command_buffer, int pass)
{
	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_PT_GODRAYS_THROUGHPUT_DIST]);
	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_ASVGF_COLOR]);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, god_rays.pipelines[0]);

	VkDescriptorSet desc_sets[] = {
		god_rays.descriptor_set,
		qvk.desc_set_vertex_buffer,
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
	};

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, god_rays.pipeline_layout, 0, LENGTH(desc_sets),
		desc_sets, 0, NULL);

	vkCmdPushConstants(command_buffer, god_rays.pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int), &pass);

	uint32_t group_size = THREAD_GROUP_SIZE;
	uint32_t group_num_x = (qvk.extent_render.width / (2 * qvk.device_count) + (group_size - 1)) / group_size;
	uint32_t group_num_y = (qvk.extent_render.height / 2 + (group_size - 1)) / group_size;

	vkCmdDispatch(command_buffer, group_num_x, group_num_y, 1);

	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_PT_GODRAYS_THROUGHPUT_DIST]);
	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_ASVGF_COLOR]);
}

void vkpt_record_god_rays_filter_command_buffer(VkCommandBuffer command_buffer)
{
	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_PT_TRANSPARENT]);

	const VkImageSubresourceRange subresource_range = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.levelCount = 1,
		.layerCount = 1
	};

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, god_rays.pipelines[1]);

	VkDescriptorSet desc_sets[] = {
		god_rays.descriptor_set,
		qvk.desc_set_vertex_buffer,
		qvk.desc_set_ubo,
		qvk_get_current_desc_set_textures(),
	};

	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, god_rays.pipeline_layout, 0, LENGTH(desc_sets),
		desc_sets, 0, NULL);

	int pass = 0;
	vkCmdPushConstants(command_buffer, god_rays.pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(int), &pass);

	uint32_t group_size = FILTER_THREAD_GROUP_SIZE;
	uint32_t group_num_x = (qvk.extent_render.width / qvk.device_count + (group_size - 1)) / group_size;
	uint32_t group_num_y = (qvk.extent_render.height + (group_size - 1)) / group_size;

	vkCmdDispatch(command_buffer, group_num_x, group_num_y, 1);

	BARRIER_COMPUTE(command_buffer, qvk.images[VKPT_IMG_PT_TRANSPARENT]);
}

void vkpt_god_rays_prepare_ubo(
	QVKUniformBuffer_t * ubo,
	const aabb_t* world_aabb,
	const float* proj,
	const float* view, 
	const float* shadowmap_viewproj,
	float shadowmap_depth_scale)
{
	VectorAdd(world_aabb->mins, world_aabb->maxs, ubo->world_center);
	VectorScale(ubo->world_center, 0.5f, ubo->world_center);
	VectorSubtract(world_aabb->maxs, world_aabb->mins, ubo->world_size);
	VectorScale(ubo->world_size, 0.5f, ubo->world_half_size_inv);
	ubo->world_half_size_inv[0] = 1.f / ubo->world_half_size_inv[0];
	ubo->world_half_size_inv[1] = 1.f / ubo->world_half_size_inv[1];
	ubo->world_half_size_inv[2] = 1.f / ubo->world_half_size_inv[2];
	ubo->shadow_map_depth_scale = shadowmap_depth_scale;

	ubo->god_rays_intensity = max(0.f, god_rays.intensity->value);
	ubo->god_rays_eccentricity = god_rays.eccentricity->value;

	ubo->god_rays_fogEnable = god_rays.fogEnable->value;  
	ubo->god_rays_fogTintColorRed = god_rays.fogTintColorRed->value;
	ubo->god_rays_fogTintColorGreen = god_rays.fogTintColorGreen->value;
	ubo->god_rays_fogTintColorBlue = god_rays.fogTintColorBlue->value;
	ubo->god_rays_fogTintPower = god_rays.fogTintPower->value;
	ubo->god_rays_fogDensityRoot = god_rays.fogDensityRoot->value;
	ubo->god_rays_fogPushBackDist = god_rays.fogPushBackDist->value;
	
	// Shadow parameters
	memcpy(ubo->shadow_map_VP, shadowmap_viewproj, 16 * sizeof(float));
}

static void create_image_views()
{
	god_rays.shadow_image_view = vkpt_shadow_map_get_view();

	VkSamplerReductionModeCreateInfoEXT redutcion_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT,
		.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN_EXT
	};

	const VkSamplerCreateInfo sampler_create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = &redutcion_create_info,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
	};

	_VK(vkCreateSampler(qvk.device, &sampler_create_info, NULL, &god_rays.shadow_sampler));
}

static void create_pipeline_layout()
{
	VkDescriptorSetLayoutBinding bindings[1] = { 0 };
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	
	const VkDescriptorSetLayoutCreateInfo set_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = LENGTH(bindings),
		.pBindings = bindings
	};

	_VK(vkCreateDescriptorSetLayout(qvk.device, &set_layout_create_info, NULL,
		&god_rays.descriptor_set_layout));


	VkDescriptorSetLayout desc_set_layouts[] = {
		god_rays.descriptor_set_layout,
		qvk.desc_set_layout_vertex_buffer,
		qvk.desc_set_layout_ubo,
		qvk.desc_set_layout_textures
	};

	VkPushConstantRange push_constant_range = {
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
		.offset = 0,
		.size = sizeof(int),
	};

	const VkPipelineLayoutCreateInfo layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pSetLayouts = desc_set_layouts,
		.setLayoutCount = LENGTH(desc_set_layouts),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &push_constant_range
	};

	_VK(vkCreatePipelineLayout(qvk.device, &layout_create_info, NULL,
		&god_rays.pipeline_layout));
}

static void create_pipelines()
{
	const VkPipelineShaderStageCreateInfo shader = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = qvk.shader_modules[QVK_MOD_GOD_RAYS_COMP],
		.pName = "main"
	};

	const VkPipelineShaderStageCreateInfo filter_shader = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = qvk.shader_modules[QVK_MOD_GOD_RAYS_FILTER_COMP],
		.pName = "main"
	};

	const VkComputePipelineCreateInfo pipeline_create_infos[2] = {
		{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = shader,
			.layout = god_rays.pipeline_layout
		},
		{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = filter_shader,
			.layout = god_rays.pipeline_layout
		},
	};

	_VK(vkCreateComputePipelines(qvk.device, VK_NULL_HANDLE, LENGTH(pipeline_create_infos), pipeline_create_infos,
		NULL, god_rays.pipelines));
}

static void create_descriptor_set()
{
	const VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
	};

	const VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = 1,
		.poolSizeCount = LENGTH(pool_sizes),
		.pPoolSizes = pool_sizes
	};

	_VK(vkCreateDescriptorPool(qvk.device, &pool_create_info, NULL, &god_rays.descriptor_pool));

	const VkDescriptorSetAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = god_rays.descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &god_rays.descriptor_set_layout
	};

	_VK(vkAllocateDescriptorSets(qvk.device, &allocate_info, &god_rays.descriptor_set));
}


static void update_descriptor_set()
{
	// if we end up here during init before we've called create_image_views(), punt --- we will be called again later
	if (god_rays.shadow_image_view == NULL)
		return;

	VkDescriptorImageInfo image_info = {
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		.imageView = god_rays.shadow_image_view,
		.sampler = god_rays.shadow_sampler
	};
	
	VkWriteDescriptorSet writes[1] = {
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = god_rays.descriptor_set,
			.descriptorCount = 1,
			.dstBinding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &image_info
		}
	};

	vkUpdateDescriptorSets(qvk.device, LENGTH(writes), writes, 0, NULL);
}

qboolean vkpt_god_rays_enabled(const sun_light_t* sun_light)
{
	return god_rays.enable->integer
		&& god_rays.intensity->value > 0.f
		&& sun_light->visible
		&& !physical_sky_space->integer;  // god rays look weird in space because they also appear outside of the station
}
