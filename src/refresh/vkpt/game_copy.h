typedef struct edict_s edict_t;

typedef struct link_s
{
	struct link_s	*prev, *next;
} link_t;

typedef enum
{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,		// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
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
	ENTITY_FUNC_CLOCK,
	ENTITY_FUNC_WALL,
	ENTITY_FUNC_OBJECT,
	ENTITY_FUNC_TIMER,
	ENTITY_FUNC_EXPLOSIVE,
	ENTITY_FUNC_KILLBOX,
	ENTITY_TARGET_ACTOR,
	ENTITY_TARGET_ANIMATION,
	ENTITY_TARGET_BLASTER,
	ENTITY_TARGET_CHANGELEVEL,
	ENTITY_TARGET_CHARACTER,
	ENTITY_TARGET_CROSSLEVEL_TARGET,
	ENTITY_TARGET_CROSSLEVEL_TRIGGER,
	ENTITY_TARGET_EARTHQUAKE,
	ENTITY_TARGET_EXPLOSION,
	ENTITY_TARGET_GOAL,
	ENTITY_TARGET_HELP,
	ENTITY_TARGET_LASER,
	ENTITY_TARGET_LIGHTRAMP,
	ENTITY_TARGET_SECRET,
	ENTITY_TARGET_SPAWNER,
	ENTITY_TARGET_SPEAKER,
	ENTITY_TARGET_SPLASH,
	ENTITY_TARGET_STRING,
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
	ENTITY_VIEWTHING,
	ENTITY_WORLDSPAWN,
	ENTITY_LIGHT,
	ENTITY_LIGHT_MINE1,
	ENTITY_LIGHT_MINE2,
	ENTITY_INFO_NOTNULL,
	ENTITY_PATH_CORNER,
	ENTITY_POINT_COMBAT,
	ENTITY_MISC_EXPLOBOX,
	ENTITY_MISC_BANNER,
	ENTITY_MISC_SATELLITE_DISH,
	ENTITY_MISC_ACTOR,
	ENTITY_MISC_GIB_ARM,
	ENTITY_MISC_GIB_LEG,
	ENTITY_MISC_GIB_HEAD,
	ENTITY_MISC_INSANE,
	ENTITY_MISC_DEADSOLDIER,
	ENTITY_MISC_VIPER,
	ENTITY_MISC_VIPER_BOMB,
	ENTITY_MISC_BIGVIPER,
	ENTITY_MISC_STROGG_SHIP,
	ENTITY_MISC_TELEPORTER,
	ENTITY_MISC_TELEPORTER_DEST,
	ENTITY_MISC_BLACKHOLE,
	ENTITY_MISC_EASTERTANK,
	ENTITY_MISC_EASTERCHICK,
	ENTITY_MISC_EASTERCHICK2,
	ENTITY_MONSTER_BERSERK,
	ENTITY_MONSTER_GLADIATOR,
	ENTITY_MONSTER_GUNNER,
	ENTITY_MONSTER_INFANTRY,
	ENTITY_MONSTER_SOLDIER_LIGHT,
	ENTITY_MONSTER_SOLDIER,
	ENTITY_MONSTER_SOLDIER_SS,
	ENTITY_MONSTER_TANK,
	ENTITY_MONSTER_MEDIC,
	ENTITY_MONSTER_FLIPPER,
	ENTITY_MONSTER_CHICK,
	ENTITY_MONSTER_PARASITE,
	ENTITY_MONSTER_FLYER,
	ENTITY_MONSTER_BRAIN,
	ENTITY_MONSTER_FLOATER,
	ENTITY_MONSTER_HOVER,
	ENTITY_MONSTER_MUTANT,
	ENTITY_MONSTER_SUPERTANK,
	ENTITY_MONSTER_BOSS2,
	ENTITY_MONSTER_BOSS3_STAND,
	ENTITY_MONSTER_JORG,
	ENTITY_MONSTER_COMMANDER_BODY,
	ENTITY_TURRET_BREACH,
	ENTITY_TURRET_BASE,
	ENTITY_TURRET_DRIVER,
	ENTITY_CRANE_BEAM,
	ENTITY_CRANE_HOIST,
	ENTITY_CRANE_HOOK,
	ENTITY_CRANE_CONTROL,
	ENTITY_CRANE_RESET,
	ENTITY_FUNC_BOBBINGWATER,
	ENTITY_FUNC_DOOR_SWINGING,
	ENTITY_FUNC_FORCE_WALL,
	ENTITY_FUNC_MONITOR,
	ENTITY_FUNC_PENDULUM,
	ENTITY_FUNC_PIVOT,
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
	ENTITY_MONSTER_MAKRON,
	ENTITY_PATH_TRACK,
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
	ENTITY_GRENADE,
	ENTITY_HANDGRENADE,
	ENTITY_ROCKET,
	ENTITY_CHASECAM,
	ENTITY_CAMPLAYER,
	ENTITY_PLAYER_NOISE,
	ENTITY_TARGET_GRFOG,
	ENTITY_TRIGGER_GRFOG,
	ENTITY_TRIGGER_REVERB_PRESET,
	ENTITY_TRIGGER_REVERB,
	ENTITY_TRIGGER_GODRAYS,
	ENTITY_TRIGGER_SUN


} entity_id;

typedef struct gitem_s
{
	const char		*classname;	// spawning name
	qboolean(*pickup)(struct edict_s *ent, struct edict_s *other);
	void(*use)(struct edict_s *ent, struct gitem_s *item);
	void(*drop)(struct edict_s *ent, struct gitem_s *item);
	void(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_skinnum; //Knightmare- added skinnum here so items can share models
	int			world_model_flags;
	char		*view_model;

	// client side info
	const char		*icon;
	const char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

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
	void(*aifunc)(edict_t *self, float dist);
	float	dist;
	void(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	mmove_t		*savemove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void(*stand)(edict_t *self);
	void(*idle)(edict_t *self);
	void(*search)(edict_t *self);
	void(*walk)(edict_t *self);
	void(*run)(edict_t *self);
	void(*dodge)(edict_t *self, edict_t *other, float eta);
	void(*attack)(edict_t *self);
	void(*melee)(edict_t *self);
	void(*sight)(edict_t *self, edict_t *other);
	qboolean(*checkattack)(edict_t *self);
	void(*jump)(edict_t *self);

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

	qboolean(*blocked)(edict_t *self, float dist);
	float		last_hint_time;		// last time the monster checked for hintpaths.
	edict_t		*goal_hint;			// which hint_path we're trying to get to
	int			medicTries;
	edict_t		*badMedic1, *badMedic2;	// these medics have declared this monster "unhealable"
	edict_t		*healer;	// this is who is healing this monster
	void(*duck)(edict_t *self, float eta);
	void(*unduck)(edict_t *self);
	void(*sidestep)(edict_t *self);
	//  while abort_duck would be nice, only monsters which duck but don't sidestep would use it .. only the brain
	//  not really worth it.  sidestep is an implied abort_duck
//	void		(*abort_duck)(edict_t *self);
	float		base_height;
	float		next_duck_time;
	float		duck_wait_time;
	edict_t		*last_player_enemy;
	// blindfire stuff .. the boolean says whether the monster will do it, and blind_fire_time is the timing
	// (set in the monster) of the next shot
	qboolean	blindfire;		// will the monster blindfire?
	float		blind_fire_delay;
	vec3_t		blind_fire_target;
	// used by the spawners to not spawn too much and keep track of #s of monsters spawned
	int			monster_slots;
	int			monster_used;
	edict_t		*commander;
	// powerup timers, used by widow, our friend
	float		quad_framenum;
	float		invincible_framenum;
	float		double_framenum;
	edict_t		*leader;
	edict_t		*old_leader;
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
	void(*endfunc)(edict_t *);
	qboolean	is_blocked;
} moveinfo_t;

struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	qboolean	inUse;
	int			linkCount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf

	int			numClusters;		// if -1, use headNode instead
	int			clusterNumbers[MAX_ENT_CLUSTERS];
	int			headNode;			// unused if numClusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	entity_id	class_id;			// Lazarus: Added in lieu of doing string comparisons
									// on classnames.

	int			movetype;
	int			oldmovetype;	// Knightmare added
	int			flags;

	const char		*model;
	float		freetime;			// sv.time when the object was freed

	//
	// only used locally in game, not by server
	//
	const char		*message;
	const char        *key_message;   // Lazarus: used from tremor_trigger_key
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	vec3_t		old_velocity, relative_velocity, relative_avelocity; // Knightmare added

	int			mass;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	char		*common_name;

	// Lazarus: for rotating brush models:
	float		pitch_speed;
	float		roll_speed;
	float		ideal_pitch;
	float		ideal_roll;
	float		roll;

	float		nextthink;
	void(*prethink) (edict_t *ent);
	void(*think)(edict_t *self);
	void(*postthink) (edict_t *ent); //Knightmare added
	void(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	void(*play)(edict_t *self, edict_t *activator);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		gravity_debounce_time;		// used by item_ movement commands to prevent
											// monsters from dropping to floor
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	// Lazarus: health2 and mass2 are passed from jorg to makron health and mass
	int			health2;
	int			mass2;

	float		powerarmor_time;

	const char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldEnemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;
	// Lazarus: laser timing
	float		starttime;
	float		endtime;

	float		teleport_time;

	int			watertype;
	int			waterlevel;
	int			old_watertype;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	float		goal_frame;

	// various Lazarus additions follow:

	edict_t		*turret;		// player-controlled turret
	edict_t		*child;			// "real" infantry guy, child of remote turret_driver
	vec_t		base_radius;	// Lazarus: used to project "viewpoint" of TRACK turret
								// out past base

	//ed - for the sprite/model spawner
	char		*usermodel;
	int			startframe;
	int			framenumbers;
	int			solidstate;
	// Lazarus: changed from rendereffect to renderfx and effects, and now uses
	//          real constants which can be combined.
//	int			rendereffect;
	int			renderfx;
	int         effects;
	vec3_t		bleft;
	vec3_t		tright;

	// tpp
	int			chasedist1;
	int			chasedist2;
	edict_t		*crosshair;
	// end tpp

	// item identification
	char		*datafile;

	// func_pushable
	vec3_t      oldVelocity;    // Added for TREMOR to figure falling damage
	vec3_t      offset;         // Added for TREMOR - offset from func_pushable to pusher
	float       density;
	float		bob;            // bobbing in water amplitude
	float		duration;
	int			bobframe;
	int			bounce_me;		// 0 for no bounce, 1 to bounce, 2 if velocity should not be clipped
								// this is solely used by func_pushable for now
	vec3_t      origin_offset;  // used to help locate brush models w/o origin brush
	vec3_t		org_mins, org_maxs;
	vec3_t		org_angles;
	int			org_movetype;
	int			axis;

	// crane
	qboolean    busy;
	qboolean	attracted;
	int         crane_increment;
	int         crane_dir;
	edict_t     *crane_control;
	edict_t     *crane_onboard_control;
	edict_t     *crane_beam;
	edict_t     *crane_hoist;
	edict_t     *crane_hook;
	edict_t     *crane_cargo;
	edict_t		*crane_cable;
	edict_t     *crane_light;
	vec_t       crane_bonk;

	edict_t     *speaker;       // moving speaker that eliminates the need
								// for origin brushes with brush models
	edict_t     *vehicle;       // generic drivable vehicle
	char		*idle_noise;
	float		radius;
	vec3_t		org_size;		// Initial size of the vehicle bounding box,

	vec3_t		fog_color;
	int			fog_model;
	float		fog_near;
	float		fog_far;
	float		fog_density;
	int			fog_index;
	int			fogclip;		// only used by worldspawn to indicate whether gl_clear
								// should be forced to a good value for fog obscuration
								// of HOM

	edict_t		*movewith_next;
	char		*movewith;
	edict_t		*movewith_ent;
	vec3_t		movewith_offset;
	vec3_t		parent_attach_angles;
	qboolean	do_not_rotate;

	// monster AI
	char		*dmgteam;

	// turret
	char		*destroytarget;
	char		*viewmessage;
	char		*followtarget;

	// spycam
	edict_t		*viewer;

	// monster power armor
	int			powerarmor;

	// MOVETYPE_PUSH rider angles
	int			turn_rider;

	// selected brush models will move their origin to 
	// the origin of this entity:
	char		*move_to;

	// newtargetname used ONLY by target_change and target_bmodel_spawner.
	char		*newtargetname;

	// source of target_clone's model
	char		*source;

	char		*musictrack;	// Knightmare- for specifying OGG or CD track

	// if true, brush models will move directly to Move_Done
	// at Move_Final rather than slowing down.
	qboolean	smooth_movement;

	int			in_mud;

	int			actor_sound_index[NUM_ACTOR_SOUNDS];
	int			actor_gunframe;
	int			actor_current_weapon;		// Index into weapon[]
	int			actor_weapon[2];
	int			actor_model_index[2];
	float		actor_crouch_time;
	qboolean	actor_id_model;
	vec3_t		muzzle;						// Offset from origin to gun muzzle
	vec3_t		muzzle2;					// Offset to left weapon (must have SF | 128)

	vec3_t		color;						// target_fade
	float		alpha;
	float		fadein;
	float		holdtime;
	float		fadeout;

	int			owner_id;					// These are used ONLY for ents that
	int			id;							// change maps via trigger_transition
	int			last_attacked_framenum;		// Used to turn off chicken mode

	// tracktrain
	char		*target2;
	edict_t		*prevpath;

	// spline train
	edict_t		*from;
	edict_t		*to;

	edict_t		*next_grenade;				// Used to build a list of active grenades
	edict_t		*prev_grenade;

	// FMOD
	int			*stream;	// Actually a FSOUND_STREAM * or FMUSIC_MODULE *
	int			channel;

	// gib type - specifies folder where gib models are found.
	int			gib_type;
	int			blood_type;

	int			moreflags;

	// actor muzzle flash
	edict_t		*flash;

	// Psychospaz reflections
	edict_t		*reflection[6];

	int			plat2flags;
	vec3_t		gravityVector;
	edict_t		*bad_area;
	edict_t		*hint_chain;
	edict_t		*monster_hint_chain;
	edict_t		*target_hint_chain;
	int			hint_chain_id;

	// ACEBOT_ADD
	qboolean is_bot;
	qboolean is_jumping;

	// For movement
	vec3_t move_vector;
	float next_move_time;
	float wander_timeout;
	float suicide_timeout;

	// For node code
	int current_node; // current node
	int goal_node; // current goal node
	int next_node; // the node that will take us one step closer to our goal
	int node_timeout;
	int last_node;
	int tries;
	int state;
	// ACEBOT_END
	char        *fog;
	char        *grFogOnOff, *grFogTintRed, *grFogTintGreen, *grFogTintBlue, *grFogTintPower, *grFogDensityRoot, *grFogPushBackDist;
	int         grFogDelay;
	vec3_t      grFogColor;
	int			grFogMode;
	int			reverbpreset;
	char        *reverb;
	int         TriggerDelay;
	char        *reverbString;
	char        *flDiffusion;
	char        *flGain;
	char        *flGainHF;
	char        *flDecayTime;
	char        *flDecayHFRatio;
	char        *flReflectionsGain;
	char        *flReflectionsDelay;
	char        *flLateReverbGain;
	char        *flLateReverbDelay;
	char        *flAirAbsorptionGainHF;
	char        *flRoomRolloffFactor;
	int     	iDecayHFLimit;
	char        *gr_intensity;
	char        *gr_eccentricity;
	int     	gr_enable;
	int     	gr_delay;

	char        *godrays;
	char        *sunColorRed;
	char        *sunColorGreen;
	char        *sunColorBlue;
	char        *sunAngle;
	char        *sunAnimate;
	char        *sunAzimuth;
	char        *sunElevation;
	char        *sunBrightness;
	int			sunPreset;
	char        *sun;
	int     	sunDelay;
	char        *nacname;

	int			ltMode;
	float		ltAttachOffsetX;
	float		ltAttachOffsetY;
	float		ltAttachOffsetZ;
	char        *nacCustom;
	int			nacCustomEnabled;
	int			nacCustomLoopEnabled;
	int			nacCustomToggleEnabled;
	int			nacHz;
	int         changeLightLS;
	int         changeLight;
	float		nacdirectionX;
	float		nacdirectionY;
	float		nacdirectionZ;
	float		nacumbraangle;
	float		nacpenumbraangle;
	float		naclightpow;
	float		naclightmax;
	int			naclighttype;
};
