/***
*
*	License here.
*
*	@file
*
*	Shared POD(Plain Old Data) Entity Types. (Client and Server.)
* 
***/
#pragma once



// Predeclarations of game entity class interfaces. (Needed for a pointer to.)
class ISharedGameEntity;
class IServerGameEntity;
class IClientGameEntity;


//! Maximum entity clusters.
static constexpr int32_t MAX_ENT_CLUSTERS = 16;


/**
*   @brief  An std::map storing an entity's key/value dictionary.
**/
using EntityDictionary = std::map<std::string, std::string>;


/**
*   @brief  entity_s, the server side entity structure. If you know what an entity is,
*           then you know what this is.
*
*   @details    The actual SVGBaseEntity class is a member. It is where the magic happens.
*               Entities can be linked to their "classname", this will in turn make sure that
*               the proper inheritance entity is allocated.
**/
struct entity_s {
    //! Actual entity state member, a POD type that contains all data that is actually networked.
    EntityState state;

    //! NULL if not a player the server expects the first part of gclient_s to
    //! be a PlayerState but the rest of it is opaque
    struct gclient_s *client = nullptr;       

    //! An entity is in no use, in case it complies to the INUSE macro.
    qboolean inUse = false;
    //! Keeps track of whether this entity is linked, or unlinked.
    int32_t linkCount = 0;

    // FIXME: move these fields to a server private sv_entity_t
    //=================================
    list_t area; // Linked to a division node or leaf

    //! If numClusters is -1, use headNode instead.
    int32_t numClusters = 0;
    int32_t clusterNumbers[MAX_ENT_CLUSTERS] = {};

    //! Only use this instead of numClusters if numClusters == -1
    int32_t headNode = 0;
    int32_t areaNumber = 0;
    int32_t areaNumber2 = 0;
    //================================
    //! An entity's server state flags.
    int32_t serverFlags = 0;
    //! Min and max bounding box.
    vec3_t mins = vec3_zero(), maxs = vec3_zero();
    //! Absolute world transform bounding box.
    vec3_t absMin = vec3_zero(), absMax = vec3_zero(), size = vec3_zero();
    //! The type of 'Solid' this entity contains.
    uint32_t solid = 0;
    //! Clipping mask this entity belongs to.
    int32_t clipMask = 0;
    //! Pointer to the owning entity (if any.)
    entity_s *owner = nullptr;

    //---------------------------------------------------
    // Do not modify the fields above, they're shared memory wise with the server itself.
    //---------------------------------------------------
    //! Actual class entity implementation pointer.
    IServerGameEntity* classEntity = nullptr;

    //! Dictionary containing the initial key:value entity properties.
    EntityDictionary entityDictionary;

    //! Actual sv.time when this entity was freed.
    float freeTime = 0.f;

    // Move this to clientInfo?
    int32_t lightLevel = 0;
};

/**
*   @brief  Local Client entity. Acts like a POD type similar to the server entity.
**/
struct ClientEntity {
    //! The last received state of this entity.
    EntityState current = {};
    //! The previous last valid state. In worst case scenario might be a copy of current state.
    EntityState prev = {};

    //! The mins and maxs of the entity's bounding box.
    vec3_t mins = vec3_zero();
    vec3_t maxs = vec3_zero();

    //! For diminishing grenade trails
    int32_t trailcount = 0;
    //! for trails (variable hz)
    vec3_t lerpOrigin = vec3_zero();
        
    //! The frame number that this entity was received at.
    //! Needs to be identical to the current frame number, or else this entity isn't in this frame anymore.
    int32_t serverFrame = 0;

    //! This is the actual server entity number.
    int32_t serverEntityNumber = 0;
    //! This is a unique client entity id, determined by an incremental static counter.
    int32_t clientEntityNumber = 0;

    //! Pointer to the class entity object that belongs to this client entity.
    IClientGameEntity *classEntity;

    //! Key/Value entity dictionary.
    EntityDictionary entityDictionary;
};


/**
*   @details    EntityState->renderEffects
* 
*               The render effects are useful for tweaking the way how an entity is displayed.
*               It may be favored for it to only be visible in mirrors, or fullbright, name it.
*               
*               This is the place to look for in-game entity rendering effects to apply.
**/
enum RenderEffects {
    ViewerModel     = (1 << 0),     // Don't draw through eyes, only mirrors.
    WeaponModel     = (1 << 1),     // Only draw through eyes.

    MinimalLight    = (1 << 2),     // Allways have some light. (Used for viewmodels)
    FullBright      = (1 << 3),     // Always draw the model at full light intensity.

    DepthHack       = (1 << 4),     // For view weapon Z crunching.
    Translucent     = (1 << 5),     // Translucent.

    FrameLerp       = (1 << 6),     // Linear Interpolation between animation frames.
    Beam            = (1 << 7),     // Special rendering hand: origin = to, oldOrigin = from.

    CustomSkin      = (1 << 8),     // If CustomSkin is set, ent->skin is an index in precaches.images.
    Glow            = (1 << 9),     // Pulse lighting. Used for items.
    RedShell        = (1 << 10),    // Red shell color effect.
    GreenShell      = (1 << 11),    // Green shell color effect.
    BlueShell       = (1 << 12),    // Blue shell color effect.

    InfraRedVisible = (1 << 13),    // Infrared rendering.
    DoubleShell     = (1 << 14),    // Double shell rendering.
    HalfDamShell    = (1 << 15),    // Half dam shell.
    UseDisguise     = (1 << 16),    // Use disguise.

    DebugBoundingBox = (1 << 17),   // Renders a debug bounding box using particles.
};
