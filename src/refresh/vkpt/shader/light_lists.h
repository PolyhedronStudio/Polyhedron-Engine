/*
Copyright (C) 2018 Tobias Zirr
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

#ifndef _LIGHT_LISTS_
#define _LIGHT_LISTS_

#define MAX_BRUTEFORCE_SAMPLING 8

float 
projected_tri_area_simple(mat3 positions, vec3 p)
{
	positions[0] = positions[0] - p;
	positions[1] = positions[1] - p;
	positions[2] = positions[2] - p;
	
	positions[0] = normalize(positions[0]);
	positions[1] = normalize(positions[1]);
	positions[2] = normalize(positions[2]);

	vec3 a = cross(positions[1] - positions[0], positions[2] - positions[0]);
	return length(a);
}

float
projected_tri_area(mat3 positions, vec3 p, vec3 n, vec3 V, float phong_exp, float phong_scale, float phong_weight)
{
	positions[0] = positions[0] - p;
	positions[1] = positions[1] - p;
	positions[2] = positions[2] - p;
	
	vec3 g = cross(positions[1] - positions[0], positions[2] - positions[0]);
	if ( dot(n, positions[0]) <= 0 && dot(n, positions[1]) <= 0 && dot(n, positions[2]) <= 0 )
		return 0;
	if ( dot(g, positions[0]) >= 0 && dot(g, positions[1]) >= 0 && dot(g, positions[2]) >= 0 )
		return 0;

	vec3 L = normalize(positions * vec3(1.0 / 3.0));
	float specular = phong(n, L, V, phong_exp) * phong_scale;
	float brdf = mix(1.0, specular, phong_weight);

	positions[0] = normalize(positions[0]);
	positions[1] = normalize(positions[1]);
	positions[2] = normalize(positions[2]);

	vec3 a = cross(positions[1] - positions[0], positions[2] - positions[0]);
	float pa = max(length(a) - 1e-5, 0.);
	return pa * brdf;
}

vec3
sample_projected_triangle(vec3 p, mat3 positions, vec2 rnd, out vec3 light_normal, out float projected_area)
{
	light_normal = cross(positions[1] - positions[0], positions[2] - positions[0]);
	light_normal = normalize(light_normal);

	positions[0] = positions[0] - p;
	positions[1] = positions[1] - p;
	positions[2] = positions[2] - p;

	float o = dot(light_normal, positions[0]);

	positions[0] = normalize(positions[0]);
	positions[1] = normalize(positions[1]);
	positions[2] = normalize(positions[2]);
	
	vec3 projected_normal = cross(positions[1] - positions[0], positions[2] - positions[0]);

	vec3 direction = positions * sample_triangle(rnd);
	float dl = length(direction);

	// n (p + d * t - p[i]) == 0
	// -n (p - pi) / n d == o / n d == t
	vec3 lo = direction * (o / dot(light_normal, direction));
	
	projected_area = length(projected_normal) * 0.5;

	return p + lo;
}

uint get_light_stats_addr(uint cluster, uint light, uint side)
{
	uint addr = cluster;
	addr = addr * global_ubo.num_static_lights + light;
	addr = addr * 6 + side;
	addr = addr * 2;
	return addr;
}

void
sample_polygonal_lights(
		uint list_idx,
		vec3 p,
		vec3 n,
		vec3 gn,
		vec3 V, 
		float phong_exp, 
		float phong_scale,
		float phong_weight,
		bool is_gradient,
		out vec3 position_light,
		out vec3 light_color,
		out int light_index,
		out float projected_area,
		vec3 rng)
{
	position_light = vec3(0);
	light_index = -1;
	light_color = vec3(0);
	projected_area = 0;

	if(list_idx == ~0u)
		return;

	uint list_start = get_light_list_offsets(list_idx);
	uint list_end   = get_light_list_offsets(list_idx + 1);

	float partitions = ceil(float(list_end - list_start) / float(MAX_BRUTEFORCE_SAMPLING));
	rng.x *= partitions;
	float fpart = min(floor(rng.x), partitions-1);
	rng.x -= fpart;
	list_start += int(fpart);
	int stride = int(partitions);

	float mass = 0.;

	float light_masses[MAX_BRUTEFORCE_SAMPLING];

	#pragma unroll
	for(uint i = 0, n_idx = list_start; i < MAX_BRUTEFORCE_SAMPLING; i++, n_idx += stride) {
		if (n_idx >= list_end)
			break;
		
		uint current_idx = get_light_list_lights(n_idx);

		if(current_idx == ~0u)
		{
			light_masses[i] = 0;
			continue;
		}

		LightPolygon light = get_light_polygon(current_idx);

		float m = projected_tri_area(light.positions, p, n, V, phong_exp, phong_scale, phong_weight);

		float light_lum = luminance(light.color);

		// Apply light style scaling.
		// For gradient pixels, use the style from the previous frame here
		// in order to keep the CDF consistent and make sure that the same light is picked,
		// regardless of animations. This makes the image more stable around blinking lights,
		// especially in shadowed areas.
		light_lum *= is_gradient ? light.prev_style_scale : light.light_style_scale;	

		if(light_lum < 0 && global_ubo.environment_type == ENVIRONMENT_DYNAMIC)
		{
			// Set limits on sky luminance to avoid oversampling the sky in shadowed areas, or undersampling at dusk and dawn.
			// Note: the log -> linear conversion of the cvars happens on the CPU, in main.c
			m *= clamp(sun_color_ubo.sky_luminance, global_ubo.pt_min_log_sky_luminance, global_ubo.pt_max_log_sky_luminance);
		}
		else
			m *= abs(light_lum); // abs because sky lights have negative color

		// Apply CDF adjustment based on light shadowing statistics from one of the previous frames.
		// See comments in function `get_direct_illumination` in `path_tracer_rgen.h`
		if(global_ubo.pt_light_stats != 0 
			&& m > 0 
			&& current_idx < global_ubo.num_static_lights)
		{
			uint buffer_idx = global_ubo.current_frame_idx;
			// Regular pixels get shadowing stats from the previous frame;
			// Gradient pixels get the stats from two frames ago because they need to match
			// the light sampling from the previous frame.
			buffer_idx += is_gradient ? (NUM_LIGHT_STATS_BUFFERS - 2) : (NUM_LIGHT_STATS_BUFFERS - 1);
			buffer_idx = buffer_idx % NUM_LIGHT_STATS_BUFFERS;

			uint addr = get_light_stats_addr(list_idx, current_idx, get_primary_direction(n));

			uint num_hits = light_stats_bufers[buffer_idx].stats[addr];
			uint num_misses = light_stats_bufers[buffer_idx].stats[addr + 1];
			uint num_total = num_hits + num_misses;

			if(num_total > 0)
			{
				// Adjust the mass, but set a lower limit on the factor to avoid
				// extreme changes in the sampling.
				m *= max(float(num_hits) / float(num_total), 0.1);
			}
		}

		mass += m;
		light_masses[i] = m;
	}

	if (mass <= 0)
		return;

	rng.x *= mass;
	int current_idx = -1;
	mass *= partitions;
	float pdf = 0;

	#pragma unroll
	for(uint i = 0, n_idx = list_start; i < MAX_BRUTEFORCE_SAMPLING; i++, n_idx += stride) {
		if (n_idx >= list_end)
			break;
		pdf = light_masses[i];
		current_idx = int(n_idx);
		rng.x -= pdf;

		if (rng.x <= 0)
			break;
	}

	if(rng.x > 0)
		return;

	pdf /= mass;

	// assert: current_idx >= 0?
	if (current_idx >= 0) {
		current_idx = int(get_light_list_lights(current_idx));

		LightPolygon light = get_light_polygon(current_idx);

		vec3 light_normal;
		position_light = sample_projected_triangle(p, light.positions, rng.yz, light_normal, projected_area);

		vec3 L = normalize(position_light - p);

		if(dot(L, gn) <= 0)
			projected_area = 0;

		float LdotNL = max(0, -dot(light_normal, L));
		float spotlight = sqrt(LdotNL);

		if(light.color.r >= 0)
		{
			light_color = light.color * (projected_area * spotlight * light.light_style_scale);
			projected_area = 0; // disable MIS with indirect specular rays
		}
		else
			light_color = env_map(L, true) * projected_area * global_ubo.pt_env_scale;

		light_index = current_idx;
	}

	light_color /= pdf;
}

void
sample_spherical_lightsOld(
		vec3 p,
		vec3 n,
		vec3 gn,
		float max_solid_angle,
		out vec3 position_light,
		out vec3 light_color,
		vec3 rng)
{
	position_light = vec3(0);
	light_color = vec3(0);

	if(global_ubo.num_sphere_lights == 0)
		return;

	float random_light = rng.x * global_ubo.num_sphere_lights;
	uint light_idx = min(global_ubo.num_sphere_lights - 1, uint(random_light));

	vec4 light_center_radius = global_ubo.sphere_light_data[light_idx * 4];
	float sphere_radius = light_center_radius.w;

	light_color = global_ubo.sphere_light_data[light_idx * 4 + 1].rgb;

	vec3 c = light_center_radius.xyz - p;
	float dist = length(c);
	float rdist = 1.0 / dist;
	vec3 L = c * rdist;

	float irradiance = 2 * (1 - sqrt(max(0, 1 - square(sphere_radius * rdist))));
	irradiance = min(irradiance, max_solid_angle);
	irradiance *= float(global_ubo.num_sphere_lights); // 1 / pdf

	mat3 onb = construct_ONB_frisvad(L);
	vec3 diskpt;
	diskpt.xy = sample_disk(rng.yz);
	diskpt.z = sqrt(max(0, 1 - diskpt.x * diskpt.x - diskpt.y * diskpt.y));

	position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - L * diskpt.z) * sphere_radius;
	light_color *= irradiance;

	if(dot(position_light - p, gn) <= 0)
		light_color = vec3(0);
}


float SpotAttenuation(vec3 spotDirection, vec3 lightDirection, float umbra, float penumbra)
{
	// Spot attenuation function from Frostbite, pg 115 in RTR4
	float cosTheta = clamp(dot(spotDirection, lightDirection), 0.0, 1.0);
	float t = clamp((cosTheta - cos(umbra)) / (cos(penumbra) - cos(umbra)), 0.0, 1.0);
	return t * t;
}

float LightWindowing(float distanceToLight, float maxDistance)
{
	return pow(clamp(1.f - pow((distanceToLight / maxDistance), 4), 0.0, 1.0), 2);
}

float LightFalloff(float distanceToLight)
{
	return 1.f / pow(max(distanceToLight, 1.f), 2);
}



/*
void
sample_spherical_lights3(
	vec3 p,
	vec3 n,
	vec3 gn,
	float max_solid_angle,
	out vec3 position_light,
	out vec3 light_color,
	vec3 rng)
{
	position_light = vec3(0);
	light_color = vec3(0);

	if (global_ubo.num_sphere_lights == 0)
		return;

	float random_light = rng.x * global_ubo.num_sphere_lights;
	uint light_idx = min(global_ubo.num_sphere_lights - 1, uint(random_light));

	vec4 light_center_radius = global_ubo.sphere_light_data[light_idx * 2];
	float sphere_radius = light_center_radius.w;

	light_color = global_ubo.sphere_light_data[light_idx * 2 + 1].rgb;

	vec3 lightVector = (light_center_radius.xyz - p);
	float  lightDistance = length(lightVector);

	vec3 lightDirection = normalize(lightVector);
	float  nol = max(dot(n, lightDirection), 0.f);
	vec3 spotDirection = normalize(vec3(0, 0, 1));
	float  attenuation = SpotAttenuation(spotDirection, -lightDirection, 50.0, 40.0);
	float  falloff = LightFalloff(lightDistance);
	float  window = LightWindowing(lightDistance, 80.0);

	light_color = 800.0 * light_color * nol * attenuation * falloff * window;

	//mat3 onb = construct_ONB_frisvad(lightDirection);
	//vec3 diskpt;
	//diskpt.xy = sample_disk(rng.yz);
	//diskpt.z = sqrt(max(0, 1 - diskpt.x * diskpt.x - diskpt.y * diskpt.y));
	position_light = light_center_radius.xyz;
	//position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - lightDirection * diskpt.z) * sphere_radius;

	//if(dot(position_light - p, gn) <= 0)
	//	light_color = vec3(0);
}

void
sample_spherical_lightsx(
	vec3 p,
	vec3 n,
	vec3 gn,
	float max_solid_angle,
	out vec3 position_light,
	out vec3 light_color,
	vec3 rng)
{
	position_light = vec3(0);
	light_color = vec3(0);

	if (global_ubo.num_sphere_lights == 0)
		return;

	float random_light = rng.x * global_ubo.num_sphere_lights;
	uint light_idx = min(global_ubo.num_sphere_lights - 1, uint(random_light));

	vec4 light_center_radius = global_ubo.sphere_light_data[light_idx * 2];
	float sphere_radius = light_center_radius.w;

	light_color = global_ubo.sphere_light_data[light_idx * 2 + 1].rgb;

	vec3 lightVector = (light_center_radius.xyz - p);
	float  lightDistance = length(lightVector);

	
	float rdist = 1.0 / lightDistance;
	vec3 L = lightVector * rdist;

	float irradiance = 2 * (1 - sqrt(max(0, 1 - square(sphere_radius * rdist))));
	irradiance = min(irradiance, max_solid_angle);
	irradiance *= float(global_ubo.num_sphere_lights); // 1 / pdf

	// Compute lighting
	vec3 lightDirection = normalize(lightVector);
	float  nol = max(dot(n, lightDirection), 0);
	if (nol < 0.0) nol = dot(-n, lightDirection);
	//float  falloff = LightFalloff(lightDistance);
	//float  window = LightWindowing(lightDistance, 400);

	light_color = light_color * nol * irradiance;// *window;
	
	mat3 onb = construct_ONB_frisvad(L);
	vec3 diskpt;
	diskpt.xy = sample_disk(rng.yz);
	diskpt.z = sqrt(max(0, 1 - diskpt.x * diskpt.x - diskpt.y * diskpt.y));
	position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - L * diskpt.z) * sphere_radius;

	if(dot(position_light - p, gn) <= 0)
		light_color = vec3(0);
}

*/

// r0 = posx,y,z,radius
// r1 = colorr,g,b,?
// r2 = dirx,y,z,power
// r3 = umbra,pumbra,maxdist,type

void
sample_spherical_lights(
	vec3 p,
	vec3 n,
	vec3 gn,
	float max_solid_angle,
	out vec3 position_light,
	out vec3 light_color,
	vec3 rng)
{
	position_light = vec3(0);
	light_color = vec3(0);

	if (global_ubo.num_sphere_lights == 0)
		return;

	float random_light = rng.x * global_ubo.num_sphere_lights;
	uint light_idx = min(global_ubo.num_sphere_lights - 1, uint(random_light));

	vec4 light_center_radius = global_ubo.sphere_light_data[light_idx * 4];
	float sphere_radius = light_center_radius.w;

	light_color = global_ubo.sphere_light_data[light_idx * 4 + 1].rgb; 
	vec4 light_parms = global_ubo.sphere_light_data[light_idx * 4 + 2];
	vec4 light_parms2 = global_ubo.sphere_light_data[light_idx * 4 + 3];
	mat3 onb;
	vec3 diskpt;
	diskpt.xy = sample_disk(rng.yz);
	diskpt.z = sqrt(max(0, 1 - diskpt.x * diskpt.x - diskpt.y * diskpt.y));

	if (light_parms2.w == 0.0)
	{
		// Sphere
		vec3 c = light_center_radius.xyz - p;
		float dist = length(c);
		float rdist = 1.0 / dist;
		vec3 L = c * rdist;

		float irradiance = 2 * (1 - sqrt(max(0, 1 - square(light_parms2.z * rdist))));
		irradiance = min(irradiance, max_solid_angle);
		irradiance *= float(global_ubo.num_sphere_lights); // 1 / pdf
		light_color = light_parms.w * light_color * irradiance;
		onb = construct_ONB_frisvad(L);
		position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - L * diskpt.z) * sphere_radius;
	}
	if (light_parms2.w == 1.0)
	{
		// Spot
		vec3 lightVector = (light_center_radius.xyz - p);
		float  lightDistance = length(lightVector);
		vec3 lightDirection = normalize(lightVector);
		float rdist = 1.0 / lightDistance;
		float irradiance = 2 * (1 - sqrt(max(0, 1 - square(light_parms2.z * rdist))));
		irradiance = min(irradiance, max_solid_angle);
		irradiance *= float(global_ubo.num_sphere_lights); // 1 / pdf
		//light_color *= irradiance;

		//float  nol = dot(n, lightDirection);
		//if (nol < 0.0) nol = dot(-n, lightDirection);
		vec3 spotDirection = -normalize(light_parms.xyz);
		float  attenuation = SpotAttenuation(spotDirection, -lightDirection, light_parms2.x * (3.14159265358979323846 / 180.0), light_parms2.y * (3.14159265358979323846 / 180.0));
		float  falloff = LightFalloff(lightDistance);
		float  window = LightWindowing(lightDistance, light_parms2.z);
		light_color = light_parms.w * light_color * irradiance * attenuation * falloff * window;
		onb = construct_ONB_frisvad(lightDirection);
		position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - lightDirection * diskpt.z) * sphere_radius;
	}
	if (light_parms2.w == 2.0)
	{
		// Directional
		vec3 lightDirection = -normalize(light_parms.xyz);
		float  nol = dot(n, lightDirection);
		if (nol < 0.0) nol = dot(-n, lightDirection);
		light_color = light_parms.w * light_color * nol;
		onb = construct_ONB_frisvad(lightDirection);
		position_light = light_center_radius.xyz + (onb[0] * diskpt.x + onb[2] * diskpt.y - lightDirection * diskpt.z) * sphere_radius;
    }
	
	if(dot(position_light - p, gn) <= 0)
		light_color = vec3(0);
}


#endif /*_LIGHT_LISTS_*/

// vim: shiftwidth=4 noexpandtab tabstop=4 cindent
//float4x4 light = lights[nlight];
//float3 direction = -float3(light[0][2], light[1][2], light[2][2]);
//float visibility = LightVisibility(worldPosition, direction, 1e27f, normal, normalBias, viewBias, bvh);
//
//// Early out, the light isn't visible from the surface
//if (visibility <= 0.f) return float3(0.f, 0.f, 0.f);
//
//// Compute lighting
//float3 lightDirection = normalize(direction);
//float  nol = max(dot(normal, lightDirection), 0.f);
//
//return light[3][0] * float3(light[0][1], light[1][1], light[2][1]) * nol * visibility;
