/***
*
*	License here.
*
*	@file
*
*	Experimental file, goal: Load in specific animation data that is of interest
*	to both the client as well as the server.
* 
***/
#include "Shared/Shared.h"
#include "Shared/Formats/Iqm.h"
#include "Shared/Refresh.h"
#include "Common/Common.h"
#include "Common/Error.h"

// Taken from /src/Refresh/Models.h
//#define modelAlloc(&model->hunk, size)    Hunk_Alloc(&model->hunk, size)

static qboolean IQM_CheckRange(const iqmHeader_t* header, uint32_t offset, uint32_t count, size_t size) {
	// return true if the range specified by offset, count and size
	// doesn't fit into the file
	return (count == 0 ||
			offset > header->filesize ||
			offset + count * size > header->filesize);
}

// "multiply" 3x4 matrices, these are assumed to be the top 3 rows
// of a 4x4 matrix with the last row = (0 0 0 1)
static void Matrix34Multiply(const float* a, const float* b, float* out) {
	out[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8];
	out[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9];
	out[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10];
	out[3] = a[0] * b[3] + a[1] * b[7] + a[2] * b[11] + a[3];
	out[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8];
	out[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9];
	out[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10];
	out[7] = a[4] * b[3] + a[5] * b[7] + a[6] * b[11] + a[7];
	out[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8];
	out[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9];
	out[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10];
	out[11] = a[8] * b[3] + a[9] * b[7] + a[10] * b[11] + a[11];
}

static void JointToMatrix(const quat_t rot, const vec3_t scale, const vec3_t trans, float* mat) {
	float xx = 2.0f * rot[0] * rot[0];
	float yy = 2.0f * rot[1] * rot[1];
	float zz = 2.0f * rot[2] * rot[2];
	float xy = 2.0f * rot[0] * rot[1];
	float xz = 2.0f * rot[0] * rot[2];
	float yz = 2.0f * rot[1] * rot[2];
	float wx = 2.0f * rot[3] * rot[0];
	float wy = 2.0f * rot[3] * rot[1];
	float wz = 2.0f * rot[3] * rot[2];

	mat[0] = scale[0] * (1.0f - (yy + zz));
	mat[1] = scale[0] * (xy - wz);
	mat[2] = scale[0] * (xz + wy);
	mat[3] = trans[0];
	mat[4] = scale[1] * (xy + wz);
	mat[5] = scale[1] * (1.0f - (xx + zz));
	mat[6] = scale[1] * (yz - wx);
	mat[7] = trans[1];
	mat[8] = scale[2] * (xz - wy);
	mat[9] = scale[2] * (yz + wx);
	mat[10] = scale[2] * (1.0f - (xx + yy));
	mat[11] = trans[2];
}

static void Matrix34Invert(const float* inMat, float* outMat) {
	outMat[0] = inMat[0]; outMat[1] = inMat[4]; outMat[2] = inMat[8];
	outMat[4] = inMat[1]; outMat[5] = inMat[5]; outMat[6] = inMat[9];
	outMat[8] = inMat[2]; outMat[9] = inMat[6]; outMat[10] = inMat[10];

	float invSqrLen, * v;
	v = outMat + 0; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 4; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);
	v = outMat + 8; invSqrLen = 1.0f / DotProduct(v, v); VectorScale(v, invSqrLen, v);

	vec3_t trans;
	trans[0] = inMat[3];
	trans[1] = inMat[7];
	trans[2] = inMat[11];

	outMat[3] = -DotProduct(outMat + 0, trans);
	outMat[7] = -DotProduct(outMat + 4, trans);
	outMat[11] = -DotProduct(outMat + 8, trans);
}

static void QuatSlerp(const quat_t from, const quat_t _to, float fraction, quat_t out) {
	// cos() of angle
	float cosAngle = from[0] * _to[0] + from[1] * _to[1] + from[2] * _to[2] + from[3] * _to[3];

	// negative handling is needed for taking shortest path (required for model joints)
	quat_t to;
	if (cosAngle < 0.0f) 	{
		cosAngle = -cosAngle;
		to[0] = -_to[0];
		to[1] = -_to[1];
		to[2] = -_to[2];
		to[3] = -_to[3];
	}
	else 	{
		QuatCopy(_to, to);
	}

	float backlerp, lerp;
	if (cosAngle < 0.999999f) 	{
		// spherical lerp (slerp)
		const float angle = acosf(cosAngle);
		const float sinAngle = sinf(angle);
		backlerp = sinf((1.0f - fraction) * angle) / sinAngle;
		lerp = sinf(fraction * angle) / sinAngle;
	}
	else 	{
		// linear lerp
		backlerp = 1.0f - fraction;
		lerp = fraction;
	}

	out[0] = from[0] * backlerp + to[0] * lerp;
	out[1] = from[1] * backlerp + to[1] * lerp;
	out[2] = from[2] * backlerp + to[2] * lerp;
	out[3] = from[3] * backlerp + to[3] * lerp;
}

static vec_t QuatNormalize2(const quat_t v, quat_t out) {
	float length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];

	if (length > 0.f) 	{
		/* writing it this way allows gcc to recognize that rsqrt can be used */
		float ilength = 1 / sqrtf(length);
		/* sqrt(length) = length * (1 / sqrt(length)) */
		length *= ilength;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
		out[3] = v[3] * ilength;
	}
	else 	{
		out[0] = out[1] = out[2] = 0;
		out[3] = -1;
	}

	return length;
}

// ReSharper disable CppClangTidyClangDiagnosticCastAlign

// WID: Adding these to make this file build, without it getting too tricky.
// TODO: Obviously get IQM to just use vec3_t bounds[2]; instead.
void IQM_ClearBounds(float *mins, float *maxs) {
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void IQM_AddPointToBounds(const float* v, float* mins, float* maxs) {
	int        i;
	vec_t    val;

	for (i = 0; i < 3; i++) {
		val = v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}

/**
*	@brief	Load an IQM model and compute the joint poses for every frame.
**/
qerror_t MOD_LoadIQM_Base(model_t* model, ModelMemoryAllocateCallback modelAlloc, const void* rawdata, size_t length, const char* mod_name) {
	iqm_transform_t* transform;
	float* mat, * matInv;
	size_t joint_names;
	iqm_model_t* iqmData;
	char meshName[MAX_QPATH];
	int vertexArrayFormat[IQM_COLOR + 1];

	if (length < sizeof(iqmHeader_t)) 	{
		return Q_ERR_FILE_TOO_SMALL;
	}

	const iqmHeader_t* header = (const iqmHeader_t * )rawdata;
	if (strncmp(header->magic, IQM_MAGIC, sizeof(header->magic)) != 0) 	{
		return Q_ERR_INVALID_FORMAT;
	}

	if (header->version != IQM_VERSION) 	{
		Com_WPrintf("R_LoadIQM: %s is a unsupported IQM version (%d), only version %d is supported.\n",
					mod_name, header->version, IQM_VERSION);
		return Q_ERR_UNKNOWN_FORMAT;
	}

	if (header->filesize > length || header->filesize > 16 << 20) 	{
		return Q_ERR_FILE_TOO_SMALL;
	}

	// check ioq3 joint limit
	if (header->num_joints > IQM_MAX_JOINTS) 	{
		Com_WPrintf("R_LoadIQM: %s has more than %d joints (%d).\n",
					mod_name, IQM_MAX_JOINTS, header->num_joints);
		return Q_ERR_INVALID_FORMAT;
	}

	for (uint32_t vertexarray_idx = 0; vertexarray_idx < Q_COUNTOF(vertexArrayFormat); vertexarray_idx++) 	{
		vertexArrayFormat[vertexarray_idx] = -1;
	}

	if (header->num_meshes) 	{
		// check vertex arrays
		if (IQM_CheckRange(header, header->ofs_vertexarrays, header->num_vertexarrays, sizeof(iqmVertexArray_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}
		const iqmVertexArray_t* vertexarray = (const iqmVertexArray_t*)((const byte*)header + header->ofs_vertexarrays);
		for (uint32_t vertexarray_idx = 0; vertexarray_idx < header->num_vertexarrays; vertexarray_idx++, vertexarray++) 		{
			if (vertexarray->size <= 0 || vertexarray->size > 4) 			{
				return Q_ERR_INVALID_FORMAT;
			}

			uint32_t num_values = header->num_vertexes * vertexarray->size;

			switch (vertexarray->format) {
			case IQM_BYTE:
			case IQM_UBYTE:
				// 1-byte
				if (IQM_CheckRange(header, vertexarray->offset, num_values, sizeof(byte))) 				{
					return Q_ERR_BAD_EXTENT;
				}
				break;
			case IQM_INT:
			case IQM_UINT:
			case IQM_FLOAT:
				// 4-byte
				if (IQM_CheckRange(header, vertexarray->offset, num_values, sizeof(float))) 				{
					return Q_ERR_BAD_EXTENT;
				}
				break;
			default:
				// not supported
				return Q_ERR_INVALID_FORMAT;
			}

			if (vertexarray->type < Q_COUNTOF(vertexArrayFormat)) 			{
				vertexArrayFormat[vertexarray->type] = (int)vertexarray->format;
			}

			switch (vertexarray->type) 			{
			case IQM_POSITION:
			case IQM_NORMAL:
				if (vertexarray->format != IQM_FLOAT ||
					vertexarray->size != 3) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			case IQM_TANGENT:
				if (vertexarray->format != IQM_FLOAT ||
					vertexarray->size != 4) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			case IQM_TEXCOORD:
				if (vertexarray->format != IQM_FLOAT ||
					vertexarray->size != 2) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			case IQM_BLENDINDEXES:
				if ((vertexarray->format != IQM_INT &&
					 vertexarray->format != IQM_UBYTE) ||
					vertexarray->size != 4) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			case IQM_BLENDWEIGHTS:
				if ((vertexarray->format != IQM_FLOAT &&
					 vertexarray->format != IQM_UBYTE) ||
					vertexarray->size != 4) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			case IQM_COLOR:
				if (vertexarray->format != IQM_UBYTE ||
					vertexarray->size != 4) 				{
					return Q_ERR_INVALID_FORMAT;
				}
				break;
			default:
				break;
			}
		}

		// check for required vertex arrays
		if (vertexArrayFormat[IQM_POSITION] == -1 || vertexArrayFormat[IQM_NORMAL] == -1 || vertexArrayFormat[IQM_TEXCOORD] == -1) 		{
			Com_WPrintf("R_LoadIQM: %s is missing IQM_POSITION, IQM_NORMAL, and/or IQM_TEXCOORD array.\n", mod_name);
			return Q_ERR_INVALID_FORMAT;
		}

		if (header->num_joints) 		{
			if (vertexArrayFormat[IQM_BLENDINDEXES] == -1 || vertexArrayFormat[IQM_BLENDWEIGHTS] == -1) 			{
				Com_WPrintf("R_LoadIQM: %s is missing IQM_BLENDINDEXES and/or IQM_BLENDWEIGHTS array.\n", mod_name);
				return Q_ERR_INVALID_FORMAT;
			}
		}
		else 		{
			// ignore blend arrays if present
			vertexArrayFormat[IQM_BLENDINDEXES] = -1;
			vertexArrayFormat[IQM_BLENDWEIGHTS] = -1;
		}

		// check triangles
		if (IQM_CheckRange(header, header->ofs_triangles, header->num_triangles, sizeof(iqmTriangle_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}
		const iqmTriangle_t* triangle = (const iqmTriangle_t*)((const byte*)header + header->ofs_triangles);
		for (uint32_t triangle_idx = 0; triangle_idx < header->num_triangles; triangle_idx++, triangle++) 		{
			if (triangle->vertex[0] > header->num_vertexes ||
				triangle->vertex[1] > header->num_vertexes ||
				triangle->vertex[2] > header->num_vertexes) {
				return Q_ERR_INVALID_FORMAT;
			}
		}

		// check meshes
		if (IQM_CheckRange(header, header->ofs_meshes, header->num_meshes, sizeof(iqmMesh_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}

		const iqmMesh_t* mesh = (const iqmMesh_t*)((const byte*)header + header->ofs_meshes);
		for (uint32_t mesh_idx = 0; mesh_idx < header->num_meshes; mesh_idx++, mesh++) 		{
			if (mesh->name < header->num_text) 			{
				strncpy(meshName, (const char*)header + header->ofs_text + mesh->name, sizeof(meshName) - 1);
			} else {
				meshName[0] = '\0';
			}

			if (mesh->first_vertex >= header->num_vertexes ||
				mesh->first_vertex + mesh->num_vertexes > header->num_vertexes ||
				mesh->first_triangle >= header->num_triangles ||
				mesh->first_triangle + mesh->num_triangles > header->num_triangles ||
				mesh->name >= header->num_text ||
				mesh->material >= header->num_text) {
				return Q_ERR_INVALID_FORMAT;
			}
		}
	}

	if (header->num_poses != header->num_joints && header->num_poses != 0) 	{
		Com_WPrintf("R_LoadIQM: %s has %d poses and %d joints, must have the same number or 0 poses\n",
					mod_name, header->num_poses, header->num_joints);
		return Q_ERR_INVALID_FORMAT;
	}

	joint_names = 0;

	if (header->num_joints) 	{
		// check joints
		if (IQM_CheckRange(header, header->ofs_joints, header->num_joints, sizeof(iqmJoint_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}

		const iqmJoint_t* joint = (const iqmJoint_t*)((const byte*)header + header->ofs_joints);
		for (uint32_t joint_idx = 0; joint_idx < header->num_joints; joint_idx++, joint++) 		{
			if (joint->parent < -1 ||
				joint->parent >= (int)header->num_joints ||
				joint->name >= header->num_text) {
				return Q_ERR_INVALID_FORMAT;
			}
			joint_names += strlen((const char*)header + header->ofs_text +
								  joint->name) + 1;
		}
	}

	if (header->num_poses) 	{
		// check poses
		if (IQM_CheckRange(header, header->ofs_poses, header->num_poses, sizeof(iqmPose_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}
	}

	if (header->ofs_bounds) 	{
		// check model bounds
		if (IQM_CheckRange(header, header->ofs_bounds, header->num_frames, sizeof(iqmBounds_t))) 		{
			return Q_ERR_BAD_EXTENT;
		}
	}

	if (header->num_anims) 	{
		// check animations
		const iqmAnim_t* anim = (const iqmAnim_t*)((const byte*)header + header->ofs_anims);
		for (uint32_t anim_idx = 0; anim_idx < header->num_anims; anim_idx++, anim++) 		{
			if (anim->first_frame + anim->num_frames > header->num_frames) 			{
				return Q_ERR_INVALID_FORMAT;
			}
		}
	}

	iqmData = (iqm_model_t*)modelAlloc(&model->hunk, sizeof(iqm_model_t));
	model->iqmData = iqmData;

	// fill header
	iqmData->num_vertexes = (header->num_meshes > 0) ? header->num_vertexes : 0;
	iqmData->num_triangles = (header->num_meshes > 0) ? header->num_triangles : 0;
	iqmData->num_frames = header->num_frames;
	iqmData->num_meshes = header->num_meshes;
	iqmData->num_joints = header->num_joints;
	iqmData->num_poses = header->num_poses;

	if (header->num_meshes) 	{
		iqmData->meshes = (iqm_mesh_t*)modelAlloc(&model->hunk, header->num_meshes * sizeof(iqm_mesh_t));
		iqmData->indices = (uint32_t*)modelAlloc(&model->hunk, header->num_triangles * 3 * sizeof(int));
		iqmData->positions = (float*)modelAlloc(&model->hunk, header->num_vertexes * 3 * sizeof(float));
		iqmData->texcoords = (float*)modelAlloc(&model->hunk, header->num_vertexes * 2 * sizeof(float));
		iqmData->normals = (float*)modelAlloc(&model->hunk, header->num_vertexes * 3 * sizeof(float));

		if (vertexArrayFormat[IQM_TANGENT] != -1) 		{
			iqmData->tangents = (float*)modelAlloc(&model->hunk, header->num_vertexes * 4 * sizeof(float));
		}

		if (vertexArrayFormat[IQM_COLOR] != -1) 		{
			iqmData->colors = (byte*)modelAlloc(&model->hunk, header->num_vertexes * 4 * sizeof(byte));
		}

		if (vertexArrayFormat[IQM_BLENDINDEXES] != -1) 		{
			iqmData->blend_indices = (byte*)modelAlloc(&model->hunk, header->num_vertexes * 4 * sizeof(byte));
		}

		if (vertexArrayFormat[IQM_BLENDWEIGHTS] != -1) 		{
			iqmData->blend_weights = (float*)modelAlloc(&model->hunk, header->num_vertexes * 4 * sizeof(float));
		}
	}

	if (header->num_joints) 	{
		iqmData->jointNames = (char*)modelAlloc(&model->hunk, joint_names);
		iqmData->jointParents = (int*)modelAlloc(&model->hunk, header->num_joints * sizeof(int));
		iqmData->bindJoints = (float*)modelAlloc(&model->hunk, header->num_joints * 12 * sizeof(float)); // bind joint matricies
		iqmData->invBindJoints = (float*)modelAlloc(&model->hunk, header->num_joints * 12 * sizeof(float)); // inverse bind joint matricies
	}

	if (header->num_poses) 	{
		iqmData->poses = (iqm_transform_t*)modelAlloc(&model->hunk, header->num_poses * header->num_frames * sizeof(iqm_transform_t)); // pose transforms
	}

	if (header->ofs_bounds) {
		iqmData->bounds = (float*)modelAlloc(&model->hunk, header->num_frames * 6 * sizeof(float)); // model bounds
	}
	else if (header->num_meshes && header->num_frames == 0) {
		iqmData->bounds = (float*)modelAlloc(&model->hunk, 6 * sizeof(float)); // model bounds
	}

	if (header->num_meshes) {
		const iqmMesh_t* mesh = (const iqmMesh_t*)((const byte*)header + header->ofs_meshes);
		iqm_mesh_t* surface = iqmData->meshes;
		const char* str = (const char*)header + header->ofs_text;
		for (uint32_t mesh_idx = 0; mesh_idx < header->num_meshes; mesh_idx++, mesh++, surface++) {
			strncpy(surface->name, str + mesh->name, sizeof(surface->name) - 1);
			PH_StringLower(surface->name); // lowercase the surface name so skin compares are faster
			strncpy(surface->material, str + mesh->material, sizeof(surface->material) - 1);
			PH_StringLower(surface->material);

			surface->data = iqmData;
			surface->first_vertex = mesh->first_vertex;
			surface->num_vertexes = mesh->num_vertexes;
			surface->first_triangle = mesh->first_triangle;
			surface->num_triangles = mesh->num_triangles;
		}

		// copy triangles
		const iqmTriangle_t* triangle = (const iqmTriangle_t*)((const byte*)header + header->ofs_triangles);
		for (uint32_t i = 0; i < header->num_triangles; i++, triangle++) 		{
			iqmData->indices[3 * i + 0] = triangle->vertex[0];
			iqmData->indices[3 * i + 1] = triangle->vertex[1];
			iqmData->indices[3 * i + 2] = triangle->vertex[2];
		}

		// copy vertexarrays and indexes
		const iqmVertexArray_t* vertexarray = (const iqmVertexArray_t*)((const byte*)header + header->ofs_vertexarrays);
		for (uint32_t vertexarray_idx = 0; vertexarray_idx < header->num_vertexarrays; vertexarray_idx++, vertexarray++) 		{
			// skip disabled arrays
			if (vertexarray->type < Q_COUNTOF(vertexArrayFormat)
				&& vertexArrayFormat[vertexarray->type] == -1)
				continue;

			// total number of values
			uint32_t n = header->num_vertexes * vertexarray->size;

			switch (vertexarray->type) 			{
			case IQM_POSITION:
				memcpy(iqmData->positions,
					   (const byte*)header + vertexarray->offset,
					   n * sizeof(float));
				break;
			case IQM_NORMAL:
				memcpy(iqmData->normals,
					   (const byte*)header + vertexarray->offset,
					   n * sizeof(float));
				break;
			case IQM_TANGENT:
				memcpy(iqmData->tangents,
					   (const byte*)header + vertexarray->offset,
					   n * sizeof(float));
				break;
			case IQM_TEXCOORD:
				memcpy(iqmData->texcoords,
					   (const byte*)header + vertexarray->offset,
					   n * sizeof(float));
				break;
			case IQM_BLENDINDEXES:
				memcpy(iqmData->blend_indices, (const byte*)header + vertexarray->offset, n * sizeof(float));
				break;
			case IQM_BLENDWEIGHTS:
				if (vertexArrayFormat[IQM_BLENDWEIGHTS] == IQM_FLOAT) 				{
					memcpy(iqmData->blend_weights,
						   (const byte*)header + vertexarray->offset,
						   n * sizeof(float));
				} else {
					// convert blend weights from byte to float
					for (uint32_t vertex_idx = 0; vertex_idx < 4 * header->num_vertexes; vertex_idx++) 					{
						iqmData->blend_weights[vertex_idx] = (float)((const byte*)header + vertexarray->offset)[vertex_idx] / 255.f;
					}
				}
				break;
			case IQM_COLOR:
				memcpy(iqmData->colors, (const byte*)header + vertexarray->offset, n * sizeof(byte));
				break;
			default:
				break;
			}
		}
	}

	if (header->num_joints) 	{
		// copy joint names
		char* str = iqmData->jointNames;
		const iqmJoint_t* joint = (const iqmJoint_t*)((const byte*)header + header->ofs_joints);
		for (uint32_t joint_idx = 0; joint_idx < header->num_joints; joint_idx++, joint++) 		{
			const char* name = (const char*)header + header->ofs_text + joint->name;
			size_t len = strlen(name) + 1;
			memcpy(str, name, len);
			str += len;
		}

		// copy joint parents
		joint = (const iqmJoint_t*)((const byte*)header + header->ofs_joints);
		for (uint32_t joint_idx = 0; joint_idx < header->num_joints; joint_idx++, joint++) 		{
			iqmData->jointParents[joint_idx] = joint->parent;
		}

		// calculate bind joint matrices and their inverses
		mat = iqmData->bindJoints;
		matInv = iqmData->invBindJoints;
		joint = (const iqmJoint_t*)((const byte*)header + header->ofs_joints);
		for (uint32_t joint_idx = 0; joint_idx < header->num_joints; joint_idx++, joint++) 		{
			float baseFrame[12], invBaseFrame[12];

			quat_t rotate;
			QuatNormalize2(joint->rotate, rotate);

			JointToMatrix(rotate, joint->scale, joint->translate, baseFrame);
			Matrix34Invert(baseFrame, invBaseFrame);

			if (joint->parent >= 0) 			{
				Matrix34Multiply(iqmData->bindJoints + 12 * joint->parent, baseFrame, mat);
				mat += 12;
				Matrix34Multiply(invBaseFrame, iqmData->invBindJoints + 12 * joint->parent, matInv);
				matInv += 12;
			}
			else 			{
				memcpy(mat, baseFrame, sizeof(baseFrame));
				mat += 12;
				memcpy(matInv, invBaseFrame, sizeof(invBaseFrame));
				matInv += 12;
			}
		}
	}

	if (header->num_poses) {
		// calculate pose transforms
		transform = iqmData->poses;
		const uint16_t* framedata = (const uint16_t*)((const byte*)header + header->ofs_frames);
		for (uint32_t frame_idx = 0; frame_idx < header->num_frames; frame_idx++) {
			const iqmPose_t* pose = (const iqmPose_t*)((const byte*)header + header->ofs_poses);
			for (uint32_t pose_idx = 0; pose_idx < header->num_poses; pose_idx++, pose++, transform++) {
				vec3_t translate;
				quat_t rotate;
				vec3_t scale;

				translate[0] = pose->channeloffset[0]; if (pose->mask & 0x001) translate[0] += (float)*framedata++ * pose->channelscale[0];
				translate[1] = pose->channeloffset[1]; if (pose->mask & 0x002) translate[1] += (float)*framedata++ * pose->channelscale[1];
				translate[2] = pose->channeloffset[2]; if (pose->mask & 0x004) translate[2] += (float)*framedata++ * pose->channelscale[2];

				rotate[0] = pose->channeloffset[3]; if (pose->mask & 0x008) rotate[0] += (float)*framedata++ * pose->channelscale[3];
				rotate[1] = pose->channeloffset[4]; if (pose->mask & 0x010) rotate[1] += (float)*framedata++ * pose->channelscale[4];
				rotate[2] = pose->channeloffset[5]; if (pose->mask & 0x020) rotate[2] += (float)*framedata++ * pose->channelscale[5];
				rotate[3] = pose->channeloffset[6]; if (pose->mask & 0x040) rotate[3] += (float)*framedata++ * pose->channelscale[6];

				scale[0] = pose->channeloffset[7]; if (pose->mask & 0x080) scale[0] += (float)*framedata++ * pose->channelscale[7];
				scale[1] = pose->channeloffset[8]; if (pose->mask & 0x100) scale[1] += (float)*framedata++ * pose->channelscale[8];
				scale[2] = pose->channeloffset[9]; if (pose->mask & 0x200) scale[2] += (float)*framedata++ * pose->channelscale[9];

				VectorCopy(translate, transform->translate);
				QuatNormalize2(rotate, transform->rotate);
				VectorCopy(scale, transform->scale);
			}
		}
	}

	// copy model bounds
	if (header->ofs_bounds) {
		mat = iqmData->bounds;
		const iqmBounds_t* bounds = (const iqmBounds_t*)((const byte*)header + header->ofs_bounds);
		for (uint32_t frame_idx = 0; frame_idx < header->num_frames; frame_idx++) 		{
			mat[0] = bounds->bbmin[0];
			mat[1] = bounds->bbmin[1];
			mat[2] = bounds->bbmin[2];
			mat[3] = bounds->bbmax[0];
			mat[4] = bounds->bbmax[1];
			mat[5] = bounds->bbmax[2];

			mat += 6;
			bounds++;
		}
	} else if (header->num_meshes && header->num_frames == 0) {
		mat = iqmData->bounds;

		IQM_ClearBounds(&iqmData->bounds[0], &iqmData->bounds[3]);
		for (uint32_t vertex_idx = 0; vertex_idx < header->num_vertexes; vertex_idx++) {
			IQM_AddPointToBounds(&iqmData->positions[vertex_idx * 3], &iqmData->bounds[0], &iqmData->bounds[3]);
		}
	}

	if (header->num_anims) {
		iqmData->num_animations = header->num_anims;
		iqmData->animations = (iqm_anim_t*)modelAlloc(&model->hunk, header->num_anims * sizeof(iqm_anim_t));

		const iqmAnim_t* src = (const iqmAnim_t*)((const byte*)header + header->ofs_anims);
		iqm_anim_t* dst = iqmData->animations;
		for (uint32_t anim_idx = 0; anim_idx < header->num_anims; anim_idx++, src++, dst++) 		{
			const char* name = (const char*)header + header->ofs_text + src->name;
			strncpy(dst->name, name, sizeof(dst->name));
			dst->name[sizeof(dst->name) - 1] = 0;

			dst->first_frame = src->first_frame;
			dst->num_frames = src->num_frames;
			//dst->framerate = src->framerate;
			dst->loop = (src->flags & IQM_LOOP) != 0;
		}
	}

	return Q_ERR_SUCCESS;
}



/**
*
*
*	Animation Processing Logic.
*
*
**/
/*
=================
R_ComputeIQMTransforms

Compute matrices for this model, returns [model->num_poses] 3x4 matrices in the (pose_matrices) array
=================
*/
qboolean R_ComputeIQMTransforms(const iqm_model_t* model, const r_entity_t* entity, float* pose_matrices) {
	iqm_transform_t relativeJoints[IQM_MAX_JOINTS];

	iqm_transform_t* relativeJoint = relativeJoints;

	const int frame = model->num_frames ? entity->frame % (int)model->num_frames : 0;
	const int oldframe = model->num_frames ? entity->oldframe % (int)model->num_frames : 0;
	const float backlerp = entity->backlerp;

	// copy or lerp animation frame pose
	if (oldframe == frame) {
		const iqm_transform_t* pose = &model->poses[frame * model->num_poses];
		for (uint32_t pose_idx = 0; pose_idx < model->num_poses; pose_idx++, pose++, relativeJoint++) {
			relativeJoint->translate = pose->translate;
			VectorCopy(pose->translate, relativeJoint->translate);
			QuatCopy(pose->rotate, relativeJoint->rotate);
			VectorCopy(pose->scale, relativeJoint->scale);
		}
	} else {
		const float lerp = 1.0f - backlerp;
		const iqm_transform_t* pose = &model->poses[frame * model->num_poses];
		const iqm_transform_t* oldpose = &model->poses[oldframe * model->num_poses];
		for (uint32_t pose_idx = 0; pose_idx < model->num_poses; pose_idx++, oldpose++, pose++, relativeJoint++) {
			relativeJoint->translate[0] = oldpose->translate[0] * backlerp + pose->translate[0] * lerp;
			relativeJoint->translate[1] = oldpose->translate[1] * backlerp + pose->translate[1] * lerp;
			relativeJoint->translate[2] = oldpose->translate[2] * backlerp + pose->translate[2] * lerp;

			relativeJoint->scale[0] = oldpose->scale[0] * backlerp + pose->scale[0] * lerp;
			relativeJoint->scale[1] = oldpose->scale[1] * backlerp + pose->scale[1] * lerp;
			relativeJoint->scale[2] = oldpose->scale[2] * backlerp + pose->scale[2] * lerp;

			QuatSlerp(oldpose->rotate, pose->rotate, lerp, relativeJoint->rotate);
		}
	}

	// multiply by inverse of bind pose and parent 'pose mat' (bind pose transform matrix)
	relativeJoint = relativeJoints;
	const int* jointParent = model->jointParents;
	const float* invBindMat = model->invBindJoints;
	float* poseMat = pose_matrices;
	for (uint32_t pose_idx = 0; pose_idx < model->num_poses; pose_idx++, relativeJoint++, jointParent++, invBindMat += 12, poseMat += 12) {
		float mat1[12], mat2[12];

		JointToMatrix(relativeJoint->rotate, relativeJoint->scale, relativeJoint->translate, mat1);

		if (*jointParent >= 0) {
			Matrix34Multiply(&model->bindJoints[(*jointParent) * 12], mat1, mat2);
			Matrix34Multiply(mat2, invBindMat, mat1);
			Matrix34Multiply(&pose_matrices[(*jointParent) * 12], mat1, poseMat);
		} else {
			Matrix34Multiply(mat1, invBindMat, poseMat);
		}
	}

	return true;
}

/**
*	@brief	Compute pose transformations for the given model + data
*			relativeJoints` must have enough room for iqmModel->num_poses
*
*			NOTE: This needs some refactoring in the near future. Data flowing isn't exactly...
*			great here. 
**/
void MOD_ComputeIQMRelativeJoints(/*const iqm_model_t* model*/const model_t *model, const int32_t rootBoneAxisFlags, int32_t currentFrame, int32_t oldFrame, float lerp, float backLerp, iqm_transform_t *relativeJoints) {
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;
	// Relative Joints.
	iqm_transform_t* relativeJoint = relativeJoints;

	if (!iqmModel) {
		return;
	}

	// Current Frame.
	currentFrame = iqmModel->num_frames ? (currentFrame % (int) iqmModel->num_frames) : 0;
	// Old Frame.
	oldFrame = iqmModel->num_frames ? (oldFrame % (int) iqmModel->num_frames) : 0;

	// Copy or lerp animation currentFrame pose
	//if (oldFrame == currentFrame) {
	//	const iqm_transform_t* pose = &iqmModel->poses[currentFrame * iqmModel->num_poses];
	//	for (uint32_t poseIndex = 0; poseIndex < iqmModel->num_poses; poseIndex++, pose++, relativeJoint++) {
	//		// Do we have skeletal model data?
	//		if (skmData && poseIndex == skmData->rootJointIndex) {
	//			// Copy over the pose's translation.
	//			relativeJoint->translate = pose->translate;

	//			// See whether to cancel/zero out any axis.
	//			if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
	//				relativeJoint->translate.x = 0.0;
	//			}
	//			if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
	//				relativeJoint->translate.y = 0.0;
	//			}
	//			if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
	//				relativeJoint->translate.z = 0.0;
	//			}

	//			// Copy over the pose's scale.
	//			relativeJoint->scale = pose->scale;
	//			// Copy over the pose's rotation.
	//			QuatCopy(pose->rotate, relativeJoint->rotate);
	//			
	//			// Skip regular treatment.
	//			continue;
	//		}

	//		// Copy over the pose's translation.
	//		relativeJoint->translate = pose->translate;
	//		// Copy over the pose's scale.
	//		relativeJoint->scale = pose->scale;
	//		// Copy over the pose's rotation.
	//		QuatCopy(pose->rotate, relativeJoint->rotate);
	//	}
	//}
	// Copy or lerp animation currentFrame pose
	if (oldFrame == currentFrame) {
		const iqm_transform_t* pose = &iqmModel->poses[currentFrame * iqmModel->num_poses];
		for (uint32_t poseIndex = 0; poseIndex < iqmModel->num_poses; poseIndex++, pose++, relativeJoint++) {
			// Do we have skeletal model data?
			if (skmData && poseIndex == skmData->rootJointIndex) {
				// Copy over the pose's translation.
				relativeJoint->translate = pose->translate;

				// See whether to cancel/zero out any axis.
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
					relativeJoint->translate.x = 0.0;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
					relativeJoint->translate.y = 0.0;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
					relativeJoint->translate.z = 0.0;
				}

				// Copy over the pose's scale.
				relativeJoint->scale = pose->scale;
				// Copy over the pose's rotation.
				QuatCopy(pose->rotate, relativeJoint->rotate);
				
				// Skip regular treatment.
				continue;
			}

			// Copy over the pose's translation.
			relativeJoint->translate = pose->translate;
			// Copy over the pose's scale.
			relativeJoint->scale = pose->scale;
			// Copy over the pose's rotation.
			QuatCopy(pose->rotate, relativeJoint->rotate);
		}

	} else {
		const iqm_transform_t* pose = &iqmModel->poses[currentFrame * iqmModel->num_poses];
		const iqm_transform_t* oldpose = &iqmModel->poses[oldFrame * iqmModel->num_poses];
		for (uint32_t poseIndex = 0; poseIndex < iqmModel->num_poses; poseIndex++, oldpose++, pose++, relativeJoint++)
		{
			// Do we have skeletal model data?
			if (skmData && poseIndex == skmData->rootJointIndex) {
				
				// Cancel out if the Zero*Translation flag is set, otherwise, calculate translation for that axis.
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
					relativeJoint->translate.x = 0.0;
				} else {
					relativeJoint->translate.x = oldpose->translate.x * backLerp + pose->translate.x * lerp;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
					relativeJoint->translate.y = 0.0;
				} else {
					relativeJoint->translate.y = oldpose->translate.y * backLerp + pose->translate.y * lerp;
				}
				if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
					relativeJoint->translate.z = 0.0;
				} else {
					relativeJoint->translate.z = oldpose->translate.z * backLerp + pose->translate.z * lerp;
				}
				
				// Calculate the Joint's pose scale.				
				relativeJoint->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
				relativeJoint->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
				relativeJoint->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

				// Copy over the pose's rotation.
				//QuatCopy(pose->rotate, relativeJoint->rotate);
				// Slerp rotation.
				QuatSlerp(oldpose->rotate, pose->rotate, lerp, relativeJoint->rotate);

				// Skip regular treatment.
				continue;
			}

			// Calculate translation.
			relativeJoint->translate[0] = oldpose->translate[0] * backLerp + pose->translate[0] * lerp;
			relativeJoint->translate[1] = oldpose->translate[1] * backLerp + pose->translate[1] * lerp;
			relativeJoint->translate[2] = oldpose->translate[2] * backLerp + pose->translate[2] * lerp;

			// Scale.
			relativeJoint->scale[0] = oldpose->scale[0] * backLerp + pose->scale[0] * lerp;
			relativeJoint->scale[1] = oldpose->scale[1] * backLerp + pose->scale[1] * lerp;
			relativeJoint->scale[2] = oldpose->scale[2] * backLerp + pose->scale[2] * lerp;

			// Slerp rotation.
			QuatSlerp(oldpose->rotate, pose->rotate, lerp, relativeJoint->rotate);
		}
	}
}

// Apply Flags to Root Bone relatives for use after applying animations.
void MOD_ApplyRootBoneAxisFlags(const model_t* model, const int32_t rootBoneAxisFlags, const int32_t rootBoneNumber, iqm_transform_t* bonePoses, float fraction, float lerp, float backlerp) {
	
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;
	
	if (!iqmModel) {
		return;
	}
			
	// Do we have skeletal model data?
	if (skmData && rootBoneNumber >= 0) {
		// Get Root Bone Pose Transform.
		iqm_transform_t *rootBone = bonePoses + rootBoneNumber;

		// Cancel out if the Zero*Translation flag is set, otherwise, calculate translation for that axis.
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroXTranslation) ) {
			rootBone->translate.x = 0.0;
		}
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroYTranslation) ) {
			rootBone->translate.y = 0.0;
		}
		if ( (rootBoneAxisFlags & SkeletalAnimation::RootBoneAxisFlags::ZeroZTranslation) ) {
			rootBone->translate.z = 0.0;
		}
	}
}

/**
*	@brief	Combine 2 poses into one by performing a recursive blend starting from the given boneNode.
**/
void MOD_RecursiveBlendFromBone(const model_t *model, iqm_transform_t* inBonePoses, iqm_transform_t* outBonePoses, int32_t boneNumber, float fraction, float lerp, float backlerp) {
	// Get 
	if (boneNumber >= 0) {
		iqm_transform_t *inBone = inBonePoses + boneNumber;
		iqm_transform_t *outBone = outBonePoses + boneNumber;

		if (fraction == 1) {
			outBonePoses[boneNumber] = inBonePoses[boneNumber];
			//memcpy( &outBonePoses[boneNumber], &inBonePoses[boneNumber], sizeof(iqm_transform_t) );
		} else if (lerp == 0) {
			inBonePoses[boneNumber] = outBonePoses[boneNumber];
			//memcpy( &inBonePoses[boneNumber], &outBonePoses[boneNumber], sizeof(iqm_transform_t) );
		} else {
			// This is wrong.
			/*outBone->translate[0] = inBone->translate[0] * backlerp + outBone->translate[0] * lerp;
			outBone->translate[1] = inBone->translate[1] * backlerp + outBone->translate[1] * lerp;
			outBone->translate[2] = inBone->translate[2] * backlerp + outBone->translate[2] * lerp;

			outBone->scale[0] = inBone->scale[0] * backlerp + outBone->scale[0] * lerp;
			outBone->scale[1] = inBone->scale[1] * backlerp + outBone->scale[1] * lerp;
			outBone->scale[2] = inBone->scale[2] * backlerp + outBone->scale[2] * lerp;

			QuatSlerp(inBone->rotate, outBone->rotate, lerp, outBone->rotate);*/

			//
			// This seems to literally, combine them.
			//
			//outBone->translate = vec3_mix(outBone->translate, inBone->translate, backlerp);
			outBone->translate[0] = outBone->translate[0] * backlerp + inBone->translate[0] * lerp;
			outBone->translate[1] = outBone->translate[1] * backlerp + inBone->translate[1] * lerp;
			outBone->translate[2] = outBone->translate[2] * backlerp + inBone->translate[2] * lerp;
			//outBone->scale = vec3_mix(outBone->scale, inBone->scale, backlerp);
			outBone->scale[0] = outBone->scale[0] * backlerp + inBone->scale[0] * lerp;
			outBone->scale[1] = outBone->scale[1] * backlerp + inBone->scale[1] * lerp;
			outBone->scale[2] = outBone->scale[2] * backlerp + inBone->scale[2] * lerp;

			QuatSlerp(outBone->rotate, inBone->rotate, lerp, outBone->rotate);
		}
	}
	
	//
	// This badly needs a bone hierachy instead of this crap lmao.
	//
	if (model->iqmData) {
		auto *iqmData = model->iqmData;

		for ( int32_t jointIndex = 0; jointIndex < iqmData->num_joints; jointIndex++ ) {
			//// Get the parent joints.
			//const int32_t jointParentIndex = (jointIndex == 0 ? -1 : model->iqmData->jointParents[jointIndex]);

			//// Create our joint object.
			//SkeletalModelData::Joint jointData = {
			//	.name = parsedJointNames[jointIndex],
			//	.index = jointIndex,
			//	.parentIndex = jointParentIndex,
			//};
			//// Store our joint in our list.
			//skm->jointMap[jointData.name] = jointData;
			//if (jointData.index < 256) {
			//	skm->jointArray[jointData.index] = jointData;
			//} else {
			//	// TODO: Warn.
			//}

			//// If we are dealing with the "root" bone, store it.
			//if (jointData.name == "root") {
			//	skm->rootJointIndex = jointIndex;
			//}

			const int32_t parentIndex = model->iqmData->jointParents[jointIndex];

			if (parentIndex >= 0 && parentIndex == boneNumber) {
				MOD_RecursiveBlendFromBone( model, inBonePoses, outBonePoses, jointIndex, fraction, lerp, backlerp );
			}
			//iqmData->jointParents;
			//iqmData->bindJoints
		}
	}
	//// TODO: We need a decent bone hierachy for this, we got none of that right now so...
	//for (int32_t i = 0; i < numberOfChildBones; i++) {
	//	int32_t childBoneNumber = 0;

	//	// Blend from here.
	//	MOD_RecursiveBlendFromBone( inBonePoses, outBonePoses, childBoneNumber, lerp, backlerp );
	//}
}

/**
*	@brief	Compute local space matrices for the given pose transformations.
*			this is the "fast path" for when world space is not necessary.
**/
void MOD_ComputeIQMLocalSpaceMatricesFromRelative(/*const iqm_model_t* model*/const model_t *model, const iqm_transform_t *relativeJoints, float *poseMatrices) {
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;
	
	// multiply by inverse of bind pose and parent 'pose mat' (bind pose transform matrix)
	const iqm_transform_t *relativeJoint = relativeJoints;
	const int* jointParent = iqmModel->jointParents;
	const float* invBindMat = iqmModel->invBindJoints;
	float* poseMat = poseMatrices;
	for (uint32_t pose_idx = 0; pose_idx < iqmModel->num_poses; pose_idx++, relativeJoint++, jointParent++, invBindMat += 12, poseMat += 12) {
		float mat1[12], mat2[12];

		JointToMatrix(relativeJoint->rotate, relativeJoint->scale, relativeJoint->translate, mat1);

		if (*jointParent >= 0) {
			Matrix34Multiply(&iqmModel->bindJoints[(*jointParent) * 12], mat1, mat2);
			Matrix34Multiply(mat2, invBindMat, mat1);
			Matrix34Multiply(&poseMatrices[(*jointParent) * 12], mat1, poseMat);
		} else {
			Matrix34Multiply(mat1, invBindMat, poseMat);
		}
	}
}

/**
*	@brief	Compute world space matrices for the given pose transformations.
**/
void MOD_ComputeIQMWorldSpaceMatricesFromRelative(/*const iqm_model_t* model*/const model_t *model, const iqm_transform_t *relativeJoints, float *poseMatrices) {
	// Get IQM Data.
	const iqm_model_t *iqmModel = model->iqmData;
	// Get our Skeletal Model Data.
	SkeletalModelData *skmData = model->skeletalModelData;

	MOD_ComputeIQMLocalSpaceMatricesFromRelative(model, relativeJoints, poseMatrices);

	float *poseMat = iqmModel->bindJoints;
	float *outPose = poseMatrices;

	for (size_t i = 0; i < iqmModel->num_poses; i++, poseMat += 12, outPose += 12) {
		float inPose[12];
		memcpy(inPose, outPose, sizeof(inPose));
		Matrix34Multiply(inPose, poseMat, outPose);
	}
}