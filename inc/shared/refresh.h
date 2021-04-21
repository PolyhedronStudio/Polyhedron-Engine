/*
// LICENSE HERE.

//
// shared/refresh.h
//
//
// Contains all definitions of refresh.h that are shared to
// the game projects.
//
*/

#ifndef __SHARED_REFRESH_H__
#define __SHARED_REFRESH_H__

#include "system/hunk.h"

#define MAX_DLIGHTS     256
#define MAX_ENTITIES    1024     // == MAX_PACKET_ENTITIES * 2
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

// N&C: Geometry that can be pushed to the render interface.
// Added mainly for libRmlUi so we can have beautiful UIs.
//
// 
typedef struct refresh_geometry_s {
    int numverts;
    vec4_t* verts;
    vec4_t* normals;
    vec2_t* stcoords;
    vec4_t* colors;
    int numelems;
    unsigned short* elems;
} refresh_geometry_t;


typedef struct r_entity_s {
    //
    // Model.
    //
    qhandle_t           model;   // The entity model - opaque type outside refresh
    vec3_t              angles;  // The entity angles.

    //
    // Most recent data
    //
    vec3_t              origin;  // The entity origin -  also used as RenderEffects::Beam's "from"
    int                 frame;   // The entity frame - also used as RenderEffects::Beam's diameter

    //
    // Previous data for lerping
    //
    vec3_t              oldorigin;  // The old entity origin - also used as RenderEffects::Beam's "to"
    int                 oldframe;   // The old entity frame.

    //
    // Misc.
    //
    float               backlerp;   // 0.0 = current, 1.0 = old
    int                 skinnum;    // also used as RenderEffects::Beam's palette index,
                                    // -1 => use rgba

    float               alpha;      // ignore if RenderEffects::Translucent isn't set
    color_t             rgba;

    qhandle_t           skin;       // NULL for inline skin
    int                 flags;      // Flags.

    int                 id;         // Entity ID.

	int                 tent_type;  // Temporary Entity Type.

	float               scale;      // Entity Scale.
} r_entity_t;

typedef struct dlight_s {
    vec3_t  origin;
#if USE_REF == REF_GL
    vec3_t  transformed;
#endif
    vec3_t  color;
    float   intensity;
	float   radius;
	float   nacLightType;
	vec3_t  nacDirection;
	float   nacUmbra;
	float   nacPenumbra;
	float   nacLightPower;
	float   nacLightMaxDist;
	float   naclightStyle;
	char    nacTarget[MAX_QPATH];
	void    *nacTargetBind;
	void    *nacTargetLightLSBind;
	void    *nacTargetLightBind;
	char    nacTargetName[MAX_QPATH];
	void    *nacLightBind;
	int     nacLsDirection;

	int     nacusecustom;
	char    nacCustom[MAX_QPATH];
	int     nacCustomEnabled;
	int     nacCustomLoopEnabled;
	int     nacCustomToggleEnabled;
} rdlight_t;

typedef struct dlightLS_s {
	int		frame;
	int		done;
	float	hzOver1k;
	float   power;
	float   maxdist;
	char    lightstyle[MAX_QPATH];
	int     length;
	int     lastCtime;
} dlightLS_t;

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

	vec3_t      hdr_color;
} ref_feedback_t;

typedef struct refdef_s {
    int         x, y, width, height;// in virtual screen coordinates
    float       fov_x, fov_y;
    vec3_t      vieworg;
    vec3_t      viewAngles;
    vec4_t      blend;          // rgba 0-1 full screen blend
    float       time;               // time is uesed to auto animate
    int         rdflags;            // RDF_UNDERWATER, etc

    byte        *areabits;          // if not NULL, only areas with set bits will be drawn

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
    QVF_FULLSCREEN      = (1 << 2),
    QVF_VIDEOSYNC       = (1 << 3)
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

typedef enum {
    IF_NONE         = 0,
    IF_PERMANENT    = (1 << 0),
    IF_TRANSPARENT  = (1 << 1),
    IF_PALETTED     = (1 << 2),
    IF_UPSCALED     = (1 << 3),
    IF_SCRAP        = (1 << 4),
    IF_TURBULENT    = (1 << 5),
    IF_REPEAT       = (1 << 6),
    IF_NEAREST      = (1 << 7),
    IF_OPAQUE       = (1 << 8),
	IF_SRGB         = (1 << 9)
} imageflags_t;

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
} model_t;

#endif // __SHARED_REFRESH_H__