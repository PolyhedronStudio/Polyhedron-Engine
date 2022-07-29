/*
===========================================================================
This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#pragma once

/**
*
*
*	The following is the actual IQM file structur.
*
*	Scroll down to find the actual in-engine representation that is used
*	to work with.
*
*
**/
#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

#define IQM_IDENT (('E'<<24)+('T'<<16)+('N'<<8)+'I')

#define IQM_MAX_JOINTS 256

typedef struct iqmheader {
    char magic[16];
    unsigned int version;
    unsigned int filesize;
    unsigned int flags;
    unsigned int num_text, ofs_text;
    unsigned int num_meshes, ofs_meshes;
    unsigned int num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned int num_triangles, ofs_triangles, ofs_adjacency;
    unsigned int num_joints, ofs_joints;
    unsigned int num_poses, ofs_poses;
    unsigned int num_anims, ofs_anims;
    unsigned int num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned int num_comment, ofs_comment;
    unsigned int num_extensions, ofs_extensions;
} iqmHeader_t;

typedef struct iqmmesh {
    unsigned int name;
    unsigned int material;
    unsigned int first_vertex, num_vertexes;
    unsigned int first_triangle, num_triangles;
} iqmMesh_t;

enum {
    IQM_POSITION = 0,
    IQM_TEXCOORD = 1,
    IQM_NORMAL = 2,
    IQM_TANGENT = 3,
    IQM_BLENDINDEXES = 4,
    IQM_BLENDWEIGHTS = 5,
    IQM_COLOR = 6,
    IQM_CUSTOM = 0x10
};

enum {
    IQM_BYTE = 0,
    IQM_UBYTE = 1,
    IQM_SHORT = 2,
    IQM_USHORT = 3,
    IQM_INT = 4,
    IQM_UINT = 5,
    IQM_HALF = 6,
    IQM_FLOAT = 7,
    IQM_DOUBLE = 8,
};

typedef struct iqmtriangle {
    unsigned int vertex[3];
} iqmTriangle_t;

typedef struct iqmjoint {
    unsigned int name;
    int parent;
    float translate[3], rotate[4], scale[3];
} iqmJoint_t;

typedef struct iqmpose {
    int parent;
    unsigned int mask;
    float channeloffset[10];
    float channelscale[10];
} iqmPose_t;

typedef struct iqmanim {
    unsigned int name;
    unsigned int first_frame, num_frames;
    float framerate;
    unsigned int flags;
} iqmAnim_t;

enum {
    IQM_LOOP = 1 << 0
};

typedef struct iqmvertexarray {
    unsigned int type;
    unsigned int flags;
    unsigned int format;
    unsigned int size;
    unsigned int offset;
} iqmVertexArray_t;

typedef struct iqmbounds {
    float bbmin[3], bbmax[3];
    float xyradius, radius;
} iqmBounds_t;



/**
*
*
*	The following is the internal in-engine data representation of the IQM
*	model data.
*
*
**/
typedef struct {
    vec3_t translate;
    quat_t rotate;
    vec3_t scale;
} iqm_transform_t;

typedef struct {
	//! Animation Name.
    char name[MAX_QPATH];
	//! First frame index of this animation.
    uint32_t first_frame;
	//! Total number of frames of this animation, first_Frame+num_frames = end.
    uint32_t num_frames;
	//! Frametime of said IQM Animation.
	float framerate;
	//! To auto-loop, or not to auto-loop, that is tzhe questzion.
    qboolean loop;
} iqm_anim_t;

// inter-quake-model
typedef struct {
    uint32_t num_vertexes;
    uint32_t num_triangles;
    uint32_t num_frames;
    uint32_t num_meshes;
    uint32_t num_joints;
    uint32_t num_poses;
    uint32_t num_animations;
    struct iqm_mesh_s* meshes;

    uint32_t* indices;

    // vertex arrays
    float* positions;
    float* texcoords;
    float* normals;
    float* tangents;
    byte* colors;
    byte* blend_indices; // byte4 per vertex
    float* blend_weights; // float4 per vertex

    char* jointNames;
    int* jointParents;
    float* bindJoints; // [num_joints * 12]
    float* invBindJoints; // [num_joints * 12]
    iqm_transform_t* poses; // [num_frames * num_poses]
    float* bounds;

    iqm_anim_t* animations;
} iqm_model_t;

// inter-quake-model mesh
typedef struct iqm_mesh_s {
    char name[MAX_QPATH];
    char material[MAX_QPATH];
    iqm_model_t* data;
    uint32_t first_vertex, num_vertexes;
    uint32_t first_triangle, num_triangles;
    uint32_t first_influence, num_influences;
} iqm_mesh_t;