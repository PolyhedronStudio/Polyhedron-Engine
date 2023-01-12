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
*   @details	Only a specific few set of members is strictly to either of the two and enclosed using
*				some ifdef. (Inheritance would be neater but there's still some C magic going on in
*				the server, best not do that.)
**/
struct PODEntity {
	/**
	*	Entity State Data:
	*
	*	The server keeps a backup of the last(previous thus) entity state when creating a new one
	*	based on the current frame that is being processed. In return, the client stores a previous
	*	state for each newly received 'ServerCommand::Frame' message. Effectively using these to
	*	interpolate from last to current frame over frametime itself.
	**/
    //! Actual entity state member, a POD type that contains all data that is actually networked.
	EntityState currentState = {};
	EntityState previousState = {}; //! CL: Previous state.

	//! When set it is the determinant for turning this entity into a client controlled entity.
	//! If not set, the entity becomes a regular entity like any others and does not deal with any
	//! means of a "player state".
	struct gclient_s *client = nullptr;

	//! Local entities are unique to each client itself and run in their own local client physics
	//! simulations. When isLocal is true it means that the specific entity is NOT meant for being
	//! sent over the wire.
	// TODO: I suppose that this could actually just be dealt with by inspecting the entity number instead.
	qboolean isLocal = false;
	//! When an entity is 'inUse' it's open to being processed for each game frame. Do note that it
	//! still gets sent to the client unless the NoClient flag is set.
	//! If the 'freeTime' value is set this entity is ready for re-use after the required wait time has
	//! passed in order to prevent client-side entity morphing/interpolation panic.
    qboolean inUse = false;

	//! Animation State members.
	//EntityAnimationState currentAnimationState = {};
	//EntityAnimationState previousAnimationState = {};

	/**
	*	PVS Data: Members that are used for determining where we are in the PVS, useful for trace work and net code.
	**/
	//! Linked to a division node or leaf
	list_t area = { .next = nullptr, .prev = nullptr };

    //! If numClusters is -1, use headNode instead.
    int32_t numClusters = 0;
    int32_t clusterNumbers[MAX_ENT_CLUSTERS] = {};

    //! Only use this instead of numClusters if numClusters == -1
    int32_t headNode = 0;
    int32_t areaNumber = 0;
    int32_t areaNumber2 = 0;


	/**
	*	Physical Properties.
	**/
	//! The type of 'Solid' this entity contains.
    uint32_t solid = 0;
    //! Clipping mask this entity belongs to.
    int32_t clipMask = 0;
	//! Keeps track of whether this entity is linked, or unlinked.
    int32_t linkCount = 0;
	//! An entity's server state flags.
    int32_t serverFlags = 0;

	// MATRIX:
	//! Actual current entity translation matrix, calculated and set by each Link call.
	glm::mat4 translateMatrix;//glm::identity< glm::mat4 >();
	//! The inverse of translateMatrix, calculated and set by each Link call.
	glm::mat4 invTranslateMatrix;

	//! The entity's bounds in 'Model Space'.
	bbox3_t bounds = bbox3_zero();
	//! The tnity's absolute bounds in 'World Space'.
	bbox3_t absoluteBounds = bbox3_zero();
	// EOF MATRIX:

	//! Min and max bounding box.
    vec3_t mins = vec3_zero(), maxs = vec3_zero();
    //! Absolute world transform bounding box.
    vec3_t absMin = vec3_zero(), absMax = vec3_zero();
	//! The entity size described as a vec3.
	vec3_t size = vec3_zero();
    
	/**
	*	Game Physics POD:
	**/
    /**
	*	Linear Movement, used for prediction:
	*		- Currently in try-out for 'func_plat'.
	**/
	struct LinearMovement {
		//! Level timestamp of when this move should start.
		int64_t timeStamp	= 0;
		//! uint32_t movement duration.
		uint32_t duration	= 0;

		//! When extrapolating, parsed entity states will NOT set the origin.
		bool isExtrapolating	= false;
		//! True if we are at or past timeStamp and within duration.
		bool isMoving			= false;
		//! The current velocity that this entity is traveling along its linear movement path.
		vec3_t velocity		= vec3_zero();
		//! The starting origin for the linear movement path
		vec3_t beginOrigin	= vec3_zero();
		//! The end origin for the linear movement path
		vec3_t endOrigin	= vec3_zero();

		//! The starting angles for the linear movement path
		vec3_t beginAngles	= vec3_zero();
		//! The end angles for the linear movement path
		vec3_t endAngles	= vec3_zero();
	} linearMovement;

	/**
	*	Game Specific POD:
	**/
	//! Pointer to the owning entity (if any.)
    PODEntity *owner = nullptr;

	//! The actual current active game entity's hashed classname.
	uint32_t hashedClassname = 0;

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
	//! The last 'valid frame' number that this entity was received at.
	//! When not matching cl.frame.number it means that this entity is 'out of date'.
	int32_t serverFrame = 0;

	//! An entity's client state flags.
    int32_t clientFlags = 0;

	//! The current local client frame number. Used to keep score of whether an entity is 'in use' for the local
	//!	client simulation.
	int64_t clientFrame = 0;

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
*   @brief	Take Damage flags controls whether this entity should respond to ballistics or other means of getting
*			harmed at all. 
**/
struct ClientTakeDamage {
    //! Will NOT take damage if hit.
    static constexpr int32_t No     = 0;  
    //! WILL take damage if hit
    static constexpr int32_t Yes    = 1;
    //! When auto targeting is enabled, it'll recognizes this
    static constexpr int32_t Aim    = 2; 
};

/**
*	@brief	Server Flags are specifically meant for how to deal with an entity behind the scenes.
*			Prepare it for removing next frame, disable it from being sent to client, or change
*			how it should be handled when being traced and clipped against.
**/
struct EntityServerFlags {
    static constexpr uint32_t NoClient      = 0x00000001;   // Don't send entity to clients, even if it has effects.
    static constexpr uint32_t DeadMonster   = 0x00000002;   // Treat as BrushContents::DeadMonster for collision.
    static constexpr uint32_t Monster       = 0x00000004;   // Treat as BrushContents::Monster for collision.
    static constexpr uint32_t Remove        = 0x00000008;   // Delete the entity next tick.
};

/**
*	@brief	Client Entity Flags: With client-entities, comes in a similar fashion, also ClientEntityFlags.
**/
struct EntityClientFlags {
    //static constexpr uint32_t NoClient      = 0x00000001;   // Don't send entity to clients, even if it has effects.
    static constexpr uint32_t DeadMonster   = 0x00000002;   // Treat as BrushContents::DeadMonster for collision.
    static constexpr uint32_t Monster       = 0x00000004;   // Treat as BrushContents::Monster for collision.
    static constexpr uint32_t Remove        = 0x00000008;   // Delete the entity next tick.
};

/**
*   @brief  Contains types of 'solid' 
*			The commentary for each Solid described the Shape/Nature of this solid,
*			as well as when it triggers a Touch callback.
**/
struct Solid {
    static constexpr uint32_t Not           = 0;    //! No interaction with other objects.
    static constexpr uint32_t Trigger       = 1;    //! Only touch when inside, after moving.
    static constexpr uint32_t BoundingBox   = 2;    //! Touch on edge.
	static constexpr uint32_t OctagonBox    = 3;    //! Touch on edge, although it has 20, not 10.
    static constexpr uint32_t BSP           = 4;    //! Bsp clip, touch on edge.
	static constexpr uint32_t Capsule		= 5;    //! Capsule containing of a cylinder and 2 spheres. 
													//! Touch on edge.
													//! Does NOT support transformed traces. (ie can't rotate hull based on angles.)
	
	static constexpr uint32_t Sphere		= 6;    //! Simple sphere, radius is generated by 'bounds', needs to be symmetric. 
													//! Touch on edge.
													//! Does NOT support transformed traces. (ie can't rotate hull based on angles.)
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
