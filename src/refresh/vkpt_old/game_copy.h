typedef struct entity_s entity_t;

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;

typedef enum
{
	Solid::Not,			// no interaction with other objects
	Solid::Trigger,		// only touch when inside, after moving
	Solid::BoundingBox,			// touch on edge
	Solid::BSP			// bsp clip, touch on edge
} solid_t;
typedef enum
{
	ENTITY_DONT_USE_THIS_ONE,
	ENTITY_ITEM_HEALTH,
	ENTITY_ITEM_HEALTH_SMALL,
	ENTITY_ITEM_HEALTH_LARGE,
	ENTITY_ITEM_HEALTH_MEGA,
	ENTITY_INFO_PLAYER_START,
	ENTITY_INFO_PLAYER_DEATHMATCH,
	ENTITY_INFO_PLAYER_COOP,
	ENTITY_INFO_PLAYER_INTERMISSION,
	ENTITY_FUNC_PLAT,
	ENTITY_FUNC_BUTTON,
	ENTITY_FUNC_DOOR,
	ENTITY_FUNC_DOOR_SECRET,
	ENTITY_FUNC_DOOR_ROTATING,
	ENTITY_FUNC_ROTATING,
	ENTITY_FUNC_TRAIN,
	ENTITY_FUNC_WATER,
	ENTITY_FUNC_CONVEYOR,
	ENTITY_FUNC_AREAPORTAL,
	ENTITY_FUNC_WALL,
	ENTITY_FUNC_OBJECT,
	ENTITY_FUNC_EXPLOSIVE,
	ENTITY_FUNC_KILLBOX,
	ENTITY_TARGET_CHANGELEVEL,
	ENTITY_TARGET_CROSSLEVEL_TARGET,
	ENTITY_TARGET_CROSSLEVEL_TRIGGER,
	ENTITY_TARGET_EARTHQUAKE,
	ENTITY_TARGET_EXPLOSION,
	ENTITY_TARGET_LIGHTRAMP,
	ENTITY_TARGET_SPAWNER,
	ENTITY_TARGET_SPEAKER,
	ENTITY_TARGET_SPLASH,
	ENTITY_TARGET_TEMP_ENTITY,
	ENTITY_TRIGGER_ALWAYS,
	ENTITY_TRIGGER_COUNTER,
	ENTITY_TRIGGER_ELEVATOR,
	ENTITY_TRIGGER_GRAVITY,
	ENTITY_TRIGGER_HURT,
	ENTITY_TRIGGER_KEY,
	ENTITY_TRIGGER_ONCE,
	ENTITY_TRIGGER_MONSTERJUMP,
	ENTITY_TRIGGER_MULTIPLE,
	ENTITY_TRIGGER_PUSH,
	ENTITY_TRIGGER_RELAY,
	ENTITY_WORLDSPAWN,
	ENTITY_LIGHT,
	ENTITY_INFO_NOTNULL,
	ENTITY_PATH_CORNER,
	ENTITY_POINT_COMBAT,
	ENTITY_MISC_EXPLOBOX,
	ENTITY_MISC_BANNER,
	ENTITY_MISC_GIB_ARM,
	ENTITY_MISC_GIB_LEG,
	ENTITY_MISC_GIB_HEAD,
	ENTITY_MISC_TELEPORTER,
	ENTITY_MISC_TELEPORTER_DEST,
	ENTITY_MONSTER_SOLDIER_LIGHT,
	ENTITY_MONSTER_SOLDIER,
	ENTITY_MONSTER_SOLDIER_SS,
	ENTITY_FUNC_FORCE_WALL,
	ENTITY_FUNC_PUSHABLE,
	ENTITY_FUNC_REFLECT,
	ENTITY_FUNC_TRACKCHANGE,
	ENTITY_FUNC_TRACKTRAIN,
	ENTITY_FUNC_TRAINBUTTON,
	ENTITY_FUNC_VEHICLE,
	ENTITY_HINT_PATH,
	ENTITY_INFO_TRAIN_START,
	ENTITY_MISC_LIGHT,
	ENTITY_MODEL_SPAWN,
	ENTITY_MODEL_TRAIN,
	ENTITY_MODEL_TURRET,
	ENTITY_TARGET_ANGER,
	ENTITY_TARGET_ATTRACTOR,
	ENTITY_TARGET_CD,
	ENTITY_TARGET_CHANGE,
	ENTITY_TARGET_CLONE,
	ENTITY_TARGET_EFFECT,
	ENTITY_TARGET_FADE,
	ENTITY_TARGET_FAILURE,
	ENTITY_TARGET_FOG,
	ENTITY_TARGET_FOUNTAIN,
	ENTITY_TARGET_LIGHTSWITCH,
	ENTITY_TARGET_LOCATOR,
	ENTITY_TARGET_LOCK,
	ENTITY_TARGET_LOCK_CLUE,
	ENTITY_TARGET_LOCK_CODE,
	ENTITY_TARGET_LOCK_DIGIT,
	ENTITY_TARGET_MONITOR,
	ENTITY_TARGET_MONSTERBATTLE,
	ENTITY_TARGET_MOVEWITH,
	ENTITY_TARGET_PRECIPITATION,
	ENTITY_TARGET_ROCKS,
	ENTITY_TARGET_ROTATION,
	ENTITY_TARGET_SET_EFFECT,
	ENTITY_TARGET_SKILL,
	ENTITY_TARGET_SKY,
	ENTITY_TARGET_PLAYBACK,
	ENTITY_TARGET_TEXT,
	ENTITY_THING,
	ENTITY_TREMOR_TRIGGER_MULTIPLE,
	ENTITY_TRIGGER_BBOX,
	ENTITY_TRIGGER_DISGUISE,
	ENTITY_TRIGGER_FOG,
	ENTITY_TRIGGER_INSIDE,
	ENTITY_TRIGGER_LOOK,
	ENTITY_TRIGGER_MASS,
	ENTITY_TRIGGER_SCALES,
	ENTITY_TRIGGER_SPEAKER,
	ENTITY_TRIGGER_SWITCH,
	ENTITY_TRIGGER_TELEPORTER,
	ENTITY_TRIGGER_TRANSITION,
	ENTITY_BOLT,
	ENTITY_DEBRIS,
	ENTITY_GIB,
	ENTITY_GIBHEAD,
	ENTITY_CHASECAM,
	ENTITY_CAMPLAYER,
	ENTITY_PLAYER_NOISE,
} entity_id;

typedef struct gitem_s
{
	const char		*classname;	// spawning name
	qboolean(*pickup)(struct entity_s *ent, struct entity_s *other);
	void(*use)(struct entity_s *ent, struct gitem_s *item);
	void(*drop)(struct entity_s *ent, struct gitem_s *item);
	void(*weaponthink)(struct entity_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_skinnum; //Knightmare- added skinNumber here so items can share models
	int			world_model_flags;
	char		*view_model;

	// client side info
	const char		*icon;
	const char		*pickup_name;	// for printing on pickup
	int			countWidth;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	const char		*precaches;		// string of all models, sounds, and images this item will use
} gitem_t;

typedef struct
{
	void(*aifunc)(entity_t *self, float dist);
	float	dist;
	void(*thinkfunc)(entity_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastFrame;
	mframe_t	*frame;
	void(*endfunc)(entity_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	mmove_t		*savemove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void(*stand)(entity_t *self);
	void(*idle)(entity_t *self);
	void(*search)(entity_t *self);
	void(*walk)(entity_t *self);
	void(*run)(entity_t *self);
	void(*dodge)(entity_t *self, entity_t *other, float eta);
	void(*attack)(entity_t *self);
	void(*melee)(entity_t *self);
	void(*sight)(entity_t *self, entity_t *other);
	qboolean(*checkattack)(entity_t *self);
	void(*jump)(entity_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkCount;

	int			power_armor_type;
	int			power_armor_power;

	qboolean(*Blocked)(entity_t *self, float dist);
	float		last_hint_time;		// last time the monster checked for hintpaths.
	entity_t		*goal_hint;			// which hint_path we're trying to get to
	int			medicTries;
	entity_t		*badMedic1, *badMedic2;	// these medics have declared this monster "unhealable"
	entity_t		*healer;	// this is who is healing this monster
	void(*duck)(entity_t *self, float eta);
	void(*unduck)(entity_t *self);
	void(*sidestep)(entity_t *self);
	//  while abort_duck would be nice, only monsters which duck but don't sidestep would use it .. only the brain
	//  not really worth it.  sidestep is an implied abort_duck
//	void		(*abort_duck)(entity_t *self);
	float		base_height;
	float		next_duck_time;
	float		duck_wait_time;
	entity_t		*last_player_enemy;
	// blindfire stuff .. the boolean says whether the monster will do it, and blind_fire_time is the timing
	// (set in the monster) of the next shot
	qboolean	blindfire;		// will the monster blindfire?
	float		blind_fire_delay;
	vec3_t		blind_fire_target;
	// used by the spawners to not spawn too much and keep track of #s of monsters spawned
	int			monster_slots;
	int			monster_used;
	entity_t		*commander;
	// powerup timers, used by widow, our friend
	float		quad_framenum;
	float		invincible_framenum;
	float		double_framenum;
	entity_t		*leader;
	entity_t		*old_leader;
	//Lazarus
	float		min_range;		// Monsters stop chasing enemy at this distance
	float		max_range;		// Monsters won't notice or attack targets farther than this
	float		ideal_range[2];	// Ideal low and high range from target, weapon-specific
	float		flies;			// Probability of dead monster generating flies
	float		jumpup;
	float		jumpdn;
	float		rangetime;
	int			chicken_framenum;
	int			pathdir;		// Up/down a hint_path chain flag for medic
	float		visibility;		// Ratio of visibility (it's a fog thang)

//end Lazarus
} monsterinfo_t;



typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	int			prevstate;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	float		ratio;
	void(*endfunc)(entity_t *);
	qboolean	is_blocked;
} moveinfo_t;

