/*
// LICENSE HERE.

//
// clg_types.h
//
// Contains shared structures between the client and the game module.
//
*/
#ifndef __SHARED_CL_TYPES_H__
#define __SHARED_CL_TYPES_H__

//
//=============================================================================
//
// SHARED ENGINE/CLIENT DEFINITIONS.
//
// These should never be tampered with, unless you intend to recompile the
// whole engine afterwards, with the risk of possibly breaking any 
// compatibility with current mods.
//=============================================================================
//
//
// CLIENT STRUCTURES - GAME MODULE NEEDS TO KNOW ABOUT.
//

//
// Explosion particle entity effect structure.
//
typedef struct {
	enum {
		ex_free,
		ex_explosion,
		ex_misc,
		ex_flash,
		ex_mflash,
		ex_poly,
		ex_poly2,
		ex_light,
		ex_blaster,
		ex_flare
	} type;

	entity_t    ent;
	int         frames;
	float       light;
	vec3_t      lightcolor;
	float       start;
	int         baseframe;
	int         frametime; /* in milliseconds */
} explosion_t;

// Maximum amount of explosions.
#define MAX_EXPLOSIONS  32


//
// Local client entity structure, temporary entities.
//
typedef struct centity_s {
    entity_state_t    current;
    entity_state_t    prev;            // will always be valid, but might just be a copy of current

    vec3_t          mins, maxs;

    int             serverframe;        // if not current, this ent isn't in the frame

    int             trailcount;         // for diminishing grenade trails
    vec3_t          lerp_origin;        // for trails (variable hz)

#if USE_FPS
    int             prev_frame;
    int             anim_start;

    int             event_frame;
#endif

    int             fly_stoptime;

    int             id;
} centity_t;

//
// Temporarl Entity parameters.
// Used for parsing EFFECTS in the client.
//
typedef struct {
    int type;
    vec3_t pos1;
    vec3_t pos2;
    vec3_t offset;
    vec3_t dir;
    int count;
    int color;
    int entity1;
    int entity2;
    int time;
} tent_params_t;

//
// MuzzleFlash parameters.
// Used for parsing MUZZLEFLASHE messages in the client.
//
typedef struct {
    int entity;
    int weapon;
    int silenced;
} mz_params_t;

//
// Sound parameters.
// Used for parsing SOUND messages in the client.
//
typedef struct {
    int     flags;
    int     index;
    int     entity;
    int     channel;
    vec3_t  pos;
    float   volume;
    float   attenuation;
    float   timeofs;
} snd_params_t;

//
// Client Sustain structure.
//
typedef struct cl_sustain_s {
    int     id;
    int     type;
    int     endtime;
    int     nextthink;
    int     thinkinterval;
    vec3_t  org;
    vec3_t  dir;
    int     color;
    int     count;
    int     magnitude;
    void    (*think)(struct cl_sustain_s *self);
} cl_sustain_t;

//
// Client Particle Structure.
//
#define PARTICLE_GRAVITY        120
#define BLASTER_PARTICLE_COLOR  0xe0
#define INSTANT_PARTICLE    -10000.0

typedef struct cparticle_s {
    struct cparticle_s    *next;

    float   time;

    vec3_t  org;
    vec3_t  vel;
    vec3_t  accel;
    int     color;      // -1 => use rgba
    float   alpha;
    float   alphavel;
    color_t rgba;
	float   brightness;
} cparticle_t;

//
// Client DLight structure.
//
#if USE_DLIGHTS
typedef struct cdlight_s {
    int     key;        // so entities can reuse same entry
    vec3_t  color;
    vec3_t  origin;
    float   radius;
    float   die;        // stop lighting after this time
    float   decay;      // drop this each second
	vec3_t  velosity;     // move this far each second
    //float   minlight;   // don't add when contributing less
} cdlight_t;
#endif

//
// The view structure contains the view data per frame.
//
// These are pointers to the actual variables in the client.
//
typedef struct cl_view_s {
	// The entities to render for the current frame.
	entity_t    *entities;      // Will always point to a entity_t[MAX_ENTITIES] array.
	int         *num_entities;

    // The dlights to render for the current frame.
    #if USE_DLIGHTS
    dlight_t    *dlights;   // Will always point to a dlight_t[MAX_DLIGHTS] array.
    int         *num_dlights;
    #endif

    // The particles to render for the current frame.
    particle_t  *particles;   // Will always point to a particle_t[MAX_PARTICLES] array.
    int         *num_particles;

    // The lightstyles for the current frame.
    #if USE_LIGHTSTYLES
    lightstyle_t    *lightstyles;   // Will always point to a lightstyle_t[MAX_LIGHTSTYLES] array.
    #endif
} cl_view_t;


//
// Maximum amount of weapon models allowed.
//
#define MAX_CLIENTWEAPONMODELS        20        // PGM -- upped from 16 to fit the chainfist vwep

//
// Contains all the info about a client we need to know.
//
typedef struct clientinfo_s {
    char name[MAX_QPATH];           // The client name.
    char model_name[MAX_QPATH];     // The model name.
    char skin_name[MAX_QPATH];      // The skin name.

    qhandle_t skin;                 // The skin handle.
    qhandle_t icon;                 // The icon handle. 
    qhandle_t model;                // Model handle.

    qhandle_t weaponmodel[MAX_CLIENTWEAPONMODELS];  // The weapon model handles.
} clientinfo_t;

//
// Used for storing client input commands.
//
typedef struct {
    unsigned    sent;           // time sent, for calculating pings
    unsigned    rcvd;           // time rcvd, for calculating pings
    unsigned    cmdNumber;      // current cmdNumber for this frame
} client_history_t;

//
// The server frame structure contains information about the frame
// being sent from the server.
//
typedef struct {
    qboolean        valid;      // False if delta parsing failed.

    int             number;     // Sequential identifier, used for delta.
    int             delta;      // Delta between frames.

    byte            areabits[MAX_MAP_AREA_BYTES];   // Area bits of this frame.
    int             areabytes;                      // Area bytes.

    player_state_t  ps;         // The player state.
    int             clientNum;  // The client number.

    int             numEntities;    // The number of entities in the frame.
    int             firstEntity;    // The first entity in the frame.
} server_frame_t;

//
// Contains the client load states, clg_local.h can expand upon it with custom
// states. They can send a text name for the loading state to show in display.
//
typedef enum {
    LOAD_NONE,
    LOAD_MAP,
    LOAD_MODELS,
    LOAD_IMAGES,
    LOAD_CLIENTS,
    LOAD_SOUNDS
} load_state_t;

//
// The client structure is cleared at each level load, and is exposed to
// * the client game module to provide access to media and other client state.
//
typedef struct client_state_s {
    int         timeoutcount;

    unsigned    lastTransmitTime;
    unsigned    lastTransmitCmdNumber;
    unsigned    lastTransmitCmdNumberReal;
    qboolean    sendPacketNow;

    usercmd_t    cmd;
    usercmd_t    cmds[CMD_BACKUP];    // each mesage will send several old cmds
    unsigned     cmdNumber;
    short        predicted_origins[CMD_BACKUP][3];    // for debug comparing against server
    client_history_t    history[CMD_BACKUP];
    int         initialSeq;

    float       predicted_step;                // for stair up smoothing
    unsigned    predicted_step_time;
    unsigned    predicted_step_frame;

    vec3_t      predicted_origin;    // generated by CL_PredictMovement
    vec3_t      predicted_angles;
    vec3_t      predicted_velocity;
    vec3_t      prediction_error;

    // rebuilt each valid frame
    centity_t       *solidEntities[MAX_PACKET_ENTITIES];
    int             numSolidEntities;

    entity_state_t  baselines[MAX_EDICTS];

    entity_state_t  entityStates[MAX_PARSE_ENTITIES];
    int             numEntityStates;

    msgEsFlags_t    esFlags;

    server_frame_t  frames[UPDATE_BACKUP];
    unsigned        frameflags;

    server_frame_t  frame;                // received from server
    server_frame_t  oldframe;
    int             servertime;
    int             serverdelta;

#if USE_FPS
    server_frame_t  keyframe;
    server_frame_t  oldkeyframe;
    int             keyservertime;
#endif

    byte            dcs[CS_BITMAP_BYTES];

    // the client maintains its own idea of view angles, which are
    // sent to the server each frame.  It is cleared to 0 upon entering each level.
    // the server sends a delta each frame which is added to the locally
    // tracked view angles to account for standing on rotating objects,
    // and teleport direction changes
    vec3_t      viewangles;

    // interpolated movement vector used for local prediction,
    // never sent to server, rebuilt each client frame
    vec3_t      localmove;

    // accumulated mouse forward/side movement, added to both
    // localmove and pending cmd, cleared each time cmd is finalized
    vec2_t      mousemove;

#if USE_SMOOTH_DELTA_ANGLES
    short       delta_angles[3]; // interpolated
#endif

    int         time;           // this is the time value that the client
                                // is rendering at.  always <= cl.servertime
    float       lerpfrac;       // between oldframe and frame

#if USE_FPS
    int         keytime;
    float       keylerpfrac;
#endif

    //
    // Client Game View Variables.
    //
    cl_view_t   view;

    //
    // Client Rendering Variables.
    //
    refdef_t    refdef;
    float       fov_x;      // interpolated
    float       fov_y;      // derived from fov_x assuming 4/3 aspect ratio
    int         lightlevel;

    vec3_t      v_forward, v_right, v_up;    // set when refdef.angles is set

    qboolean    thirdPersonView;

    // predicted values, used for smooth player entity movement in thirdperson view
    vec3_t      playerEntityOrigin;
    vec3_t      playerEntityAngles;

    //
    // transient data from server
    //
    char        layout[MAX_NET_STRING];     // general 2D overlay
    int         inventory[MAX_ITEMS];

    //
    // server state information
    //
    int         serverstate;    // ss_* constants
    int         servercount;    // server identification for prespawns
    char        gamedir[MAX_QPATH];
    int         clientNum;            // never changed during gameplay, set by serverdata packet
    int         maxclients;
    pmoveParams_t pmp;

#if USE_FPS
    int         frametime;      // variable server frame time
    float       frametime_inv;  // 1/frametime
    int         framediv;       // BASE_FRAMETIME/frametime
#endif

    char        baseconfigstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
    char        configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
    char        mapname[MAX_QPATH]; // short format - q2dm1, etc

#if USE_AUTOREPLY
    unsigned    reply_time;
    unsigned    reply_delta;
#endif

    //
    // locally derived information from server state
    //
    bsp_t        *bsp;                  // Pointer to the actual BSP.

    qhandle_t model_draw[MAX_MODELS];   // Handles for loaded draw models (MD2, MD3, ...).
    mmodel_t *model_clip[MAX_MODELS];   // mmodel_t ptr handles for loaded clip models (Brush models).

    qhandle_t sound_precache[MAX_SOUNDS];   // Handles to the loaded sounds.
    qhandle_t image_precache[MAX_IMAGES];   // Handles to the loaded images.

    clientinfo_t    clientinfo[MAX_CLIENTS];    // Client info for all clients.
    clientinfo_t    baseclientinfo;             // Local, Player, Client Info.

    char    weaponModels[MAX_CLIENTWEAPONMODELS][MAX_QPATH]; // Weapon Models string paths.
    int     numWeaponModels;    // Number of weapon models.
} client_state_t;

typedef struct client_test_s {
    // Tests..
    char        baseconfigstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
    char        configstrings[MAX_CONFIGSTRINGS][MAX_QPATH];
} client_test_t;

#endif // __SHARED_CL_TYPES_H__