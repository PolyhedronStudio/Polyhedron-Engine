/***
*
*	License here.
*
*	@file
*
*	Contains all shared common Collision Model declarations.
* 
***/
#pragma once



/**
*   Brush Content Flags.
**/
struct BrushContents {
    // Lower bits are stronger, and will eat weaker brushes completely
    static constexpr int32_t Solid   = 1; //! An eye is never valid in a solid
    static constexpr int32_t Window  = 2; //! Translucent, but not watery
    static constexpr int32_t Aux     = 4;
    static constexpr int32_t Lava    = 8;
    static constexpr int32_t Slime   = 16;
    static constexpr int32_t Water   = 32;
    static constexpr int32_t Mist    = 64;
    static constexpr int32_t LastVisibleContents    = 64;

    // Remaining contents are non-visible, and don't eat brushes
    static constexpr int32_t AreaPortal     = 0x8000;
    static constexpr int32_t PlayerClip     = 0x10000;
    static constexpr int32_t MonsterClip    = 0x20000;

    static constexpr int32_t Origin         = 0x1000000;   //! Removed before bsping an entity
    static constexpr int32_t Monster        = 0x2000000;   //! Should never be on a brush, only in game
    static constexpr int32_t DeadMonster    = 0x4000000;
    static constexpr int32_t Detail         = 0x8000000;   //! Brushes to be added after vis leafs
    static constexpr int32_t Translucent    = 0x10000000;  //! Auto set if any surface has trans
    static constexpr int32_t Ladder         = 0x20000000;

    // Currents can be added to any other contents, and may be mixed
    static constexpr int32_t Current0       = 0x40000;
    static constexpr int32_t Current90      = 0x80000;
    static constexpr int32_t Current180     = 0x100000;
    static constexpr int32_t Current270     = 0x200000;
    static constexpr int32_t CurrentUp      = 0x400000;
    static constexpr int32_t CurrentDown    = 0x800000;
};

/**
*   Brush Content Masks.
**/
struct BrushContentsMask {
    static constexpr int32_t All          = (-1);
    static constexpr int32_t Solid        = (BrushContents::Solid | BrushContents::Window);
    static constexpr int32_t PlayerSolid  = (BrushContents::Solid | BrushContents::PlayerClip | BrushContents::Window | BrushContents::Monster);
    static constexpr int32_t DeadSolid    = (BrushContents::Solid | BrushContents::PlayerClip | BrushContents::Window);
    static constexpr int32_t MonsterSolid = (BrushContents::Solid | BrushContents::MonsterClip | BrushContents::Window | BrushContents::Monster);
    static constexpr int32_t Liquid       = (BrushContents::Water | BrushContents::Lava | BrushContents::Slime);
    static constexpr int32_t Opaque       = (BrushContents::Solid | BrushContents::Slime | BrushContents::Lava);
    static constexpr int32_t Shot         = (BrushContents::Solid | BrushContents::Monster | BrushContents::Window | BrushContents::DeadMonster);
    static constexpr int32_t Current      = (BrushContents::Current0 | BrushContents::Current90 | BrushContents::Current180 | BrushContents::Current270 | BrushContents::CurrentUp | BrushContents::CurrentDown);
};

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
struct AreaEntities {
	// These are for server entities, and for those received from server by the client.
    static constexpr int32_t Solid			= 1;
    static constexpr int32_t Triggers		= 2;
	// These are for local entities, whether that be server or client related.
    static constexpr int32_t LocalSolid		= 3;
	static constexpr int32_t LocalTriggers	= 4;
};


/**
*   Surface Collision data.
**/
struct CollisionSurface {
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
struct TraceResult {
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
    CollisionPlane plane = {};
    //! The impacted surface, or `NULL`.
    CollisionSurface* surface = nullptr;
    //! The contents mask of the impacted brush, or 0.
    int32_t contents = 0;

    // The impacted entity, or `NULL`.
    struct PODEntity* ent = nullptr;   // Not set by CM_*() functions

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