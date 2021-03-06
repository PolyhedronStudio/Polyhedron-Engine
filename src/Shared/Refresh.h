/*
// LICENSE HERE.

//
// Shared/refresh.h
//
//
// Contains all definitions of refresh.h that are shared to
// the game projects.
//
*/

#pragma once

#include "../System/Hunk.h"

#define MAX_DLIGHTS     32
#define MAX_ENTITIES    4096     // == MAX_PACKET_ENTITIES * 2
#define MAX_PARTICLES   16384
#define MAX_LIGHTSTYLES 256

#define POWERSUIT_SCALE     4.0f
#define WEAPONSHELL_SCALE   0.5f

#define SHELL_RED_COLOR     0xF2
#define SHELL_GREEN_COLOR   0xD0
#define SHELL_BLUE_COLOR    0xF3

#define SHELL_RG_COLOR      0xDC
//#define SHELL_RB_COLOR        0x86
#define SHELL_RB_COLOR      0x68
#define SHELL_BG_COLOR      0x78

//ROGUE
#define SHELL_DOUBLE_COLOR  0xDF // 223
#define SHELL_HALF_DAM_COLOR    0x90
#define SHELL_CYAN_COLOR    0x72
//ROGUE

#define SHELL_WHITE_COLOR   0xD7

// NOTE: these flags are intentionally the same value
#define RF_LEFTHAND         0x80000000
#define RF_NOSHADOW         0x80000000

#define RF_SHELL_MASK       (RenderEffects::RedShell | RenderEffects::GreenShell | RenderEffects::BlueShell | \
                             RenderEffects::DoubleShell | RenderEffects::HalfDamShell)

#define DLIGHT_CUTOFF       64

// Used to allow for client/server using their own memory allocation.
using ModelMemoryAllocateCallback = void*(*)(memhunk_t *hunk, size_t size);

// Include IQM model format.
#include "Formats/Iqm.h"

typedef struct r_entity_s {
	/**
	*	Entity ID, Temporary Entity Type.
	**/
	//! Refresh Entity ID.
    int32_t	id			= 0;
	//! Temporary Entity Type.
	int32_t tent_type	= 0;


	/**
	*	Visual Appearance:
	**/
	//! Actual Render Flags.
    int32_t		flags	= 0;
	//! Entity Model Index.
    qhandle_t	model			= 0;
	//! In case this model is a light emitter, this configurates what lightstyle it should operate with.
	int32_t		modelLightStyle = -1;
	//! Actual handle containing the index of the skin we should be taking. (The skin number is considered a "sub skin").
    qhandle_t	skin			= 0;
	//! The Entity's skin number. Also used as RenderEffects::Beam's palette index, -1 => use rgba.
    int32_t		skinNumber		= 0;

	//! Actual entity rotation angles.
    vec3_t		angles	= vec3_zero();
	//! Optionable color overlay.
	color_t		rgba	= { .u8 = { 255, 255, 255, 255 } };
	//! The Entity's alpha, only applied if RenderEffects::Translucent is set.
    float		alpha	= 0.f;
	//! Default Render Scale.
	float		scale	= 1.f;


	/**
	*	Origin & OldOrigin:
	**/
    //! Entity Origin: Also used as RenderEffects::Beam's "from" point.
	vec3_t	origin		= vec3_zero();
	//! Old Entity Origin: Also used as RenderEffects::Beam's "to" point.
	vec3_t	oldorigin	= vec3_zero();
	

	/**
	*	Animation LERPing.
	**/
	//! Entity's main animation frame. (This is also used for regular alias models and animated sprites/textures.)
	int32_t	frame		= 0;
	//! Entity's main animation frame at the time of rendering our last frame.
    int32_t	oldframe	= 0;
	//! Main Animation Back LERP: 0.0 = current, 1.0 = old.
    float backlerp		= 0.f;


	/**
	*	Skeletal Model Animation
	**/
	//! A pointer to a temporary bone cache.
	iqm_transform_t	*currentBonePoses	= nullptr;

	//! Root Bone Axis Flags for the Main Animation.
	int32_t rootBoneAxisFlags	= 0;

} r_entity_t;

typedef struct dlight_s {
    vec3_t  origin;
#if USE_REF == REF_GL
    vec3_t  transformed;
#endif
    vec3_t  color;
    float   intensity;
	float   radius;
} rdlight_t;

typedef struct particle_s {
    vec3_t  origin;
    int     color;              // -1 => use rgba
    float   alpha;
    color_t rgba;
	float   brightness;
	float   radius;
} rparticle_t;

typedef struct lightstyle_s {
    float           white;          // highest of RGB
    vec3_t          rgb;            // 0.0 - 2.0
} lightstyle_t;

#ifdef USE_SMALL_GPU
#define MAX_DECALS 2
#else
#define MAX_DECALS 50
#endif
typedef struct decal_s {
    vec3_t pos;
    vec3_t dir;
    float spread;
    float length;
    float dummy;
} decal_t;

// passes information back from the RTX renderer to the engine for various development maros
typedef struct ref_feedback_s {
	int         viewcluster;
	int         lookatcluster;
	int         num_light_polys;
	int         resolution_scale;

	char        view_material[MAX_QPATH];
	char        view_material_override[MAX_QPATH];
    int         view_material_index;

	vec3_t      hdr_color;
    float  adapted_luminance;
} ref_feedback_t;

typedef struct refdef_s {
    int         x, y, width, height;// in virtual screen coordinates
    float       fov_x, fov_y;
    vec3_t      vieworg;
    vec3_t      viewAngles;
    vec4_t      blend;          // rgba 0-1 full screen blend
    float       time;               // time is uesed to auto animate
    int         rdflags;            // RDF_UNDERWATER, etc

    byte        *areaBits;          // if not NULL, only areas with set bits will be drawn

    lightstyle_t    *lightstyles;   // [MAX_LIGHTSTYLES]

    int         num_entities;
    r_entity_t    *entities;

    int         num_dlights;
    rdlight_t    *dlights;

    int         num_particles;
    rparticle_t  *particles;

    int         decal_beg;
    int         decal_end;
    decal_t     decal[MAX_DECALS];

	ref_feedback_t feedback;
} refdef_t;

typedef enum {
    QVF_ACCELERATED     = (1 << 0),
    QVF_GAMMARAMP       = (1 << 1),
    QVF_FULLSCREEN      = (1 << 2)
} vidFlags_t;

typedef struct {
    int         width;
    int         height;
    vidFlags_t  flags;
} refcfg_t;

extern refcfg_t r_config;

typedef struct {
    int left, right, top, bottom;
} clipRect_t;

//---------------------
// Added our own operator to the int32_t enum.
//---------------------
enum imageflags_t : int {
    IF_NONE				= 0,
    IF_PERMANENT		= (1 << 0),
    IF_TRANSPARENT		= (1 << 1),
    IF_PALETTED			= (1 << 2),
    IF_UPSCALED			= (1 << 3),
    IF_SCRAP			= (1 << 4),
    IF_TURBULENT		= (1 << 5),
    IF_REPEAT			= (1 << 6),
    IF_NEAREST			= (1 << 7),
    IF_OPAQUE			= (1 << 8),
    IF_SRGB				= (1 << 9),
    IF_FAKE_EMISSIVE	= (1 << 10),
    IF_EXACT			= (1 << 11),
    IF_NORMAL_MAP		= (1 << 12),

    // Image source indicator/requirement flags
    IF_SRC_BASE = (0x1 << 16),
    IF_SRC_GAME = (0x2 << 16),
    IF_SRC_MASK = (0x3 << 16),
};

// Shift amount for storing fake emissive synthesis threshold
#define IF_FAKE_EMISSIVE_THRESH_SHIFT  20

typedef enum {
    IT_PIC,
    IT_FONT,
    IT_SKIN,
    IT_SPRITE,
    IT_WALL,
    IT_SKY,

    IT_MAX
} imagetype_t;

typedef struct mspriteframe_s {
    int             width, height;
    int             origin_x, origin_y;
    struct image_s  *image;
} mspriteframe_t;

typedef enum
{
	MCLASS_REGULAR,
	MCLASS_EXPLOSION,
	MCLASS_SMOKE,
    MCLASS_STATIC_LIGHT,
    MCLASS_FLARE
} model_class_t;

typedef struct light_poly_s {
    float positions[9]; // 3x vec3_t
    vec3_t off_center;
    vec3_t color;
    struct pbr_material_s* material;
    int cluster;
    int style;
    float emissive_factor;
} light_poly_t;

typedef struct model_s {
    enum {
        MOD_FREE,
        MOD_ALIAS,
        MOD_SPRITE,
        MOD_EMPTY
    } type;
    char name[MAX_QPATH];
    int registration_sequence;
    memhunk_t hunk;

    // alias models
    int numframes;
    struct maliasframe_s *frames;
#if USE_REF == REF_GL || USE_REF == REF_VKPT
    int nummeshes;
    struct maliasmesh_s *meshes;
	model_class_t model_class;
#else
    int numskins;
    struct image_s *skins[MAX_ALIAS_SKINS];
    int numtris;
    struct maliastri_s *tris;
    int numsts;
    struct maliasst_s *sts;
    int numverts;
    int skinwidth;
    int skinheight;
#endif

    // sprite models
    struct mspriteframe_s *spriteframes;
	qboolean sprite_vertical;
	qboolean sprite_fxup;
	qboolean sprite_fxft;
	qboolean sprite_fxlt;

	// IQM Data.
    iqm_model_t* iqmData;

	// Skeletal Model Data.
	struct SkeletalModelData *skeletalModelData;

	//// Polyhedron IQM Data? For Game Modules?
	//struct IQMGameData {
	//	struct IQMAnimation {
	//		const std::string name;
	//		uint32_t first_frame = 0;
	//		uint32_t num_frames = 0;
	//		qboolean loop = false;
	//	};
	//	std::map<std::string, IQMAnimation> 
	//} iqmGameData;

    int num_light_polys;
    light_poly_t* light_polys;
} model_t;