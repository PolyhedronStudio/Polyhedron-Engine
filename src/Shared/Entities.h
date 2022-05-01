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
using SpawnKeyValues = std::map<std::string, std::string>;


/**
*   @brief  PODEntity is a POD Entity data structure shared across the client as well as the server.
*           
*   @details    Only a specific few set of members is strictly to either of the two and enclosed using
*				some ifdef. (Inheritance would be neater but there's still some C magic going on in
*				the server, best not do that.)
**/
struct PODEntity {
    //! Actual entity state member, a POD type that contains all data that is actually networked.
	EntityState currentState = {};
	EntityState previousState = {}; //! CL: Previous state.

    //! NULL if not a player. The server expects the first part of gclient_s to
    //! be a PlayerState but the rest of it is opaque
    struct gclient_s *client = nullptr;

	//! When true it means this entity is not being processed for networking.
	qboolean isLocal = false;
    //! When an entity is not in use it isn't being processed for a game's frame.
    //! If it also has a freeTime value assigned to it then it has been freed and its slot is
    //! queued for future reusal.
    qboolean inUse = false;


	/**
	*	World related info, used for optimizing tracing and proper VIS.
	**/
	//! Linked to a division node or leaf
	list_t area = {};

    //! If numClusters is -1, use headNode instead.
    int32_t numClusters = 0;
    int32_t clusterNumbers[MAX_ENT_CLUSTERS] = {};

    //! Only use this instead of numClusters if numClusters == -1
    int32_t headNode = 0;
    int32_t areaNumber = 0;
    int32_t areaNumber2 = 0;


	/**
	*	Collision Handling Properties.
	**/
	//! The type of 'Solid' this entity contains.
    uint32_t solid = 0;
    //! Clipping mask this entity belongs to.
    int32_t clipMask = 0;
	//! Keeps track of whether this entity is linked, or unlinked.
    int32_t linkCount = 0;
	//! An entity's server state flags.
    int32_t serverFlags = 0;

	//! Min and max bounding box.
    vec3_t mins = vec3_zero(), maxs = vec3_zero();
    //! Absolute world transform bounding box.
    vec3_t absMin = vec3_zero(), absMax = vec3_zero(), size = vec3_zero();
    
	/**
	*	Data that the gamemodules set and a client/server may need to know of.
	**/
	//! Pointer to the owning entity (if any.)
    PODEntity *owner = nullptr;

	//! Actual game entity implementation pointer.
    ISharedGameEntity* gameEntity = nullptr;

    //! Dictionary containing the initial key:value entity properties.
    SpawnKeyValues spawnKeyValues;

    //! Actual sv.time when this entity was freed.
    GameTime freeTime = GameTime::zero();

    // Move this to clientInfo?
    int32_t lightLevel = 0;

	/**
	*	Client/ClientGame only fields.
	**/
	//! The frame number that this entity was received at.
	//! Needs to be identical to the current frame number, or else this entity isn't in this frame anymore.
	int32_t serverFrame = 0;

	//! An entity's client state flags.
    int32_t clientFlags = 0;

    //! For diminishing grenade trails
    int32_t trailCount = 0;
    //! for trails (variable hz)
    vec3_t lerpOrigin = vec3_zero();

    //! This is the actual server entity number.
    int32_t serverEntityNumber = 0;
    //! This is a unique client entity id, determined by an incremental static counter.
    int32_t clientEntityNumber = 0;
};


/**
*   Client Take Damage.
**/
struct ClientTakeDamage {
    //! Will NOT take damage if hit.
    static constexpr int32_t No     = 0;  
    //! WILL take damage if hit
    static constexpr int32_t Yes    = 1;
    //! When auto targeting is enabled, it'll recognizes this
    static constexpr int32_t Aim    = 2; 
};

// edict->clientFlags
struct EntityClientFlags {
    //static constexpr uint32_t NoClient      = 0x00000001;   // Don't send entity to clients, even if it has effects.
    static constexpr uint32_t DeadMonster   = 0x00000002;   // Treat as BrushContents::DeadMonster for collision.
    static constexpr uint32_t Monster       = 0x00000004;   // Treat as BrushContents::Monster for collision.
    static constexpr uint32_t Remove        = 0x00000008;   // Delete the entity next tick.
};

/**
*   @brief  Contains types of 'solid' 
*           to use up the remaining slots for their own custom needs.
**/
struct Solid {
    static constexpr uint32_t Not           = 0;    //! No interaction with other objects.
    static constexpr uint32_t Trigger       = 1;    //! Only touch when inside, after moving.
    static constexpr uint32_t BoundingBox   = 2;    //! Touch on edge.
    static constexpr uint32_t OctagonBox    = 3;    //! Touch on edge, although it has 20, not 10.
    static constexpr uint32_t BSP           = 4;    //! Bsp clip, touch on edge.
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
