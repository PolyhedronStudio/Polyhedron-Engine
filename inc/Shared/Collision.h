// License here.
#ifndef __SHARED__COLLISION_H__
#define __SHARED__COLLISION_H__

/**
*   Brush Content Flags.
**/
// Lower bits are stronger, and will eat weaker brushes completely
static constexpr int32_t CONTENTS_SOLID         = 1; //! An eye is never valid in a solid
static constexpr int32_t CONTENTS_WINDOW        = 2; //! Translucent, but not watery
static constexpr int32_t CONTENTS_AUX           = 4;
static constexpr int32_t CONTENTS_LAVA          = 8;
static constexpr int32_t CONTENTS_SLIME         = 16;
static constexpr int32_t CONTENTS_WATER         = 32;
static constexpr int32_t CONTENTS_MIST          = 64;
static constexpr int32_t LAST_VISIBLE_CONTENTS  = 64;

// Remaining contents are non-visible, and don't eat brushes
static constexpr int32_t CONTENTS_AREAPORTAL    = 0x8000;
static constexpr int32_t CONTENTS_PLAYERCLIP    = 0x10000;
static constexpr int32_t CONTENTS_MONSTERCLIP   = 0x20000;

static constexpr int32_t CONTENTS_ORIGIN        = 0x1000000;   // Removed before bsping an entity
static constexpr int32_t CONTENTS_MONSTER       = 0x2000000;   // Should never be on a brush, only in game
static constexpr int32_t CONTENTS_DEADMONSTER   = 0x4000000;
static constexpr int32_t CONTENTS_DETAIL        = 0x8000000;   // Brushes to be added after vis leafs
static constexpr int32_t CONTENTS_TRANSLUCENT   = 0x10000000;  // Auto set if any surface has trans
static constexpr int32_t CONTENTS_LADDER        = 0x20000000;

// Currents can be added to any other contents, and may be mixed
static constexpr int32_t CONTENTS_CURRENT_0     = 0x40000;
static constexpr int32_t CONTENTS_CURRENT_90    = 0x80000;
static constexpr int32_t CONTENTS_CURRENT_180   = 0x100000;
static constexpr int32_t CONTENTS_CURRENT_270   = 0x200000;
static constexpr int32_t CONTENTS_CURRENT_UP    = 0x400000;
static constexpr int32_t CONTENTS_CURRENT_DOWN  = 0x800000;

/**
*   Brush Content Masks.
**/
static constexpr int32_t CONTENTS_MASK_ALL          = (-1);
static constexpr int32_t CONTENTS_MASK_SOLID        = (CONTENTS_SOLID | CONTENTS_WINDOW);
static constexpr int32_t CONTENTS_MASK_PLAYERSOLID  = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER);
static constexpr int32_t CONTENTS_MASK_DEADSOLID    = (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW);
static constexpr int32_t CONTENTS_MASK_MONSTERSOLID = (CONTENTS_SOLID | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER);
static constexpr int32_t CONTENTS_MASK_LIQUID       = (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME);
static constexpr int32_t CONTENTS_MASK_OPAQUE       = (CONTENTS_SOLID | CONTENTS_SLIME | CONTENTS_LAVA);
static constexpr int32_t CONTENTS_MASK_SHOT         = (CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEADMONSTER);
static constexpr int32_t CONTENTS_MASK_CURRENT      = (CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN);

/**
*   Surface Flags.
**/
struct SurfaceFlags {
    static constexpr int32_t Light          = 0x1;  //! Value will hold the light strength
    static constexpr int32_t Slick          = 0x2;  //! Effects game physics
    static constexpr int32_t Sky            = 0x4;  //! Don't draw, but add to skybox
    static constexpr int32_t Warp           = 0x8;  //! Turbulent water warp
    static constexpr int32_t Transparent33  = 0x10; //! 33% Transparency.
    static constexpr int32_t Transparent66  = 0x20; //! 66% Transparency.
    static constexpr int32_t Flowing        = 0x40; //! Scroll towards angle
    static constexpr int32_t NoDraw         = 0x80; //! Don't bother referencing the texture
    static constexpr int32_t AlphaTest= 0x02000000; //! TODO: Original said: used by kmquake2. For Polyhedron, do we want or need this at all?
};

/**
*   gi.BoxEntities() can return a list of either solid or trigger entities
*   FIXME: eliminate AREA_ distinction?
**/
static constexpr int32_t AREA_SOLID     = 1;
static constexpr int32_t AREA_TRIGGERS  = 2;


/**
*   Surface Collision data.
**/
struct csurface_t {
    //! The actual material name used for this surface.
    char name[16] = {};
    //! The surface flags.
    int32_t flags = 0;
    //! The 'value' of the surface that can be set inside the map editor.
    int32_t value = 0;
};

/**
*   Traces are discrete movements through world space, clipped to the
*   BSP planes they intersect. This is the basis for all collision detection
*   within Polyhedron.
**/
struct trace_t {
    //! If true, the trace startedand ended within the same solid.
    qboolean allSolid = false;
    //! If true, the trace started within a solid, but exited it.
    qboolean startSolid = false;
    //! The fraction of the desired distance traveled(0.0 - 1.0).If
    //! 1.0, no plane was impacted.
    float fraction = 0.f;

    //! The destination position.
    vec3_t endPosition = vec3_zero();

    //! The impacted plane, or empty. Note that a copy of the plane is
    //! returned, rather than a pointer. This is because the plane may belong to
    //! an inline BSP model or the box hull of a solid entity, in which case it must
    //! be transformed by the entity's current position.
    cplane_t plane = {};
    //! The impacted surface, or `NULL`.
    csurface_t* surface = nullptr;
    //! The contents mask of the impacted brush, or 0.
    int32_t contents = 0;

    // The impacted entity, or `NULL`.
    struct entity_s* ent = nullptr;   // Not set by CM_*() functions

    // PH: Custom added.
    vec3_t offsets[8] = {
        vec3_zero(),
        vec3_zero(),
        vec3_zero(),
        vec3_zero(),
        vec3_zero(),
        vec3_zero(),
        vec3_zero(),
        vec3_zero()
    };	// [signBits][x] = either size[0][x] or size[1][x]
};

#endif // __SHARED__COLLISION_H__
