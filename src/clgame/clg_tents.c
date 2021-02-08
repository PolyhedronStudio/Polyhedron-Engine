// LICENSE HERE.

//
// clg_tents.c
//
//
// Handles the temporary entities. Explions, Beams, etc.
//
#include "clg_local.h"

//
// CVars.
//
static color_t  railcore_color;
static color_t  railspiral_color;

static cvar_t* cl_railtrail_type;
static cvar_t* cl_railtrail_time;
static cvar_t* cl_railcore_color;
static cvar_t* cl_railcore_width;
static cvar_t* cl_railspiral_color;
static cvar_t* cl_railspiral_radius;

//
// Handles to Sound Effects.
//
qhandle_t   cl_sfx_ric1;
qhandle_t   cl_sfx_ric2;
qhandle_t   cl_sfx_ric3;
qhandle_t   cl_sfx_lashit;
qhandle_t   cl_sfx_flare;
qhandle_t   cl_sfx_spark5;
qhandle_t   cl_sfx_spark6;
qhandle_t   cl_sfx_spark7;
qhandle_t   cl_sfx_railg;
qhandle_t   cl_sfx_rockexp;
qhandle_t   cl_sfx_grenexp;
qhandle_t   cl_sfx_watrexp;
qhandle_t   cl_sfx_footsteps[4];

qhandle_t   cl_sfx_lightning;
qhandle_t   cl_sfx_disrexp;

//
// Handles to Models.
//
qhandle_t   cl_mod_explode;
qhandle_t   cl_mod_smoke;
qhandle_t   cl_mod_flash;
qhandle_t   cl_mod_parasite_segment;
qhandle_t   cl_mod_grapple_cable;
qhandle_t   cl_mod_explo4;
qhandle_t   cl_mod_bfg_explo;
qhandle_t   cl_mod_powerscreen;
qhandle_t   cl_mod_laser;
qhandle_t   cl_mod_dmspot;
qhandle_t   cl_mod_explosions[4];

qhandle_t   cl_mod_lightning;
qhandle_t   cl_mod_heatbeam;
qhandle_t   cl_mod_explo4_big;

extern cvar_t* cvar_pt_particle_emissive;

//
//===============
// CLG_RegisterTEntModels
// 
// Registers all sounds used for temporary entities.
//===============
//
void CLG_RegisterTempEntitySounds(void)
{
	int     i;
	char    name[MAX_QPATH];

	// Register SFX sounds.
	cl_sfx_ric1 = clgi.S_RegisterSound("world/ric1.wav");
	cl_sfx_ric2 = clgi.S_RegisterSound("world/ric2.wav");
	cl_sfx_ric3 = clgi.S_RegisterSound("world/ric3.wav");
	cl_sfx_lashit = clgi.S_RegisterSound("weapons/lashit.wav");
	cl_sfx_flare = clgi.S_RegisterSound("weapons/flare.wav");
	cl_sfx_spark5 = clgi.S_RegisterSound("world/spark5.wav");
	cl_sfx_spark6 = clgi.S_RegisterSound("world/spark6.wav");
	cl_sfx_spark7 = clgi.S_RegisterSound("world/spark7.wav");
	cl_sfx_railg = clgi.S_RegisterSound("weapons/railgf1a.wav");
	cl_sfx_rockexp = clgi.S_RegisterSound("weapons/rocklx1a.wav");
	cl_sfx_grenexp = clgi.S_RegisterSound("weapons/grenlx1a.wav");
	cl_sfx_watrexp = clgi.S_RegisterSound("weapons/xpld_wat.wav");

	// Register Player sounds.
	clgi.S_RegisterSound("player/land1.wav");
	clgi.S_RegisterSound("player/fall2.wav");
	clgi.S_RegisterSound("player/fall1.wav");

	// Register Footstep sounds.
	for (i = 0; i < 4; i++) {
		Q_snprintf(name, sizeof(name), "player/step%i.wav", i + 1);
		cl_sfx_footsteps[i] = clgi.S_RegisterSound(name);
	}

	cl_sfx_lightning = clgi.S_RegisterSound("weapons/tesla.wav");
	cl_sfx_disrexp = clgi.S_RegisterSound("weapons/disrupthit.wav");
}

//
//===============
// CLG_RegisterTempEntityModels
// 
// Registers all models used for temporary entities.
//===============
//
void CLG_RegisterTempEntityModels(void)
{
	// Register FX models.
	cl_mod_explode = clgi.R_RegisterModel("models/objects/explode/tris.md2");
	cl_mod_smoke = clgi.R_RegisterModel("models/objects/smoke/tris.md2");
	cl_mod_flash = clgi.R_RegisterModel("models/objects/flash/tris.md2");
	cl_mod_parasite_segment = clgi.R_RegisterModel("models/monsters/parasite/segment/tris.md2");
	cl_mod_grapple_cable = clgi.R_RegisterModel("models/ctf/segment/tris.md2");
	cl_mod_explo4 = clgi.R_RegisterModel("models/objects/r_explode/tris.md2");
	cl_mod_explosions[0] = clgi.R_RegisterModel("sprites/rocket_0.sp2");
	cl_mod_explosions[1] = clgi.R_RegisterModel("sprites/rocket_1.sp2");
	cl_mod_explosions[2] = clgi.R_RegisterModel("sprites/rocket_5.sp2");
	cl_mod_explosions[3] = clgi.R_RegisterModel("sprites/rocket_6.sp2");
	cl_mod_bfg_explo = clgi.R_RegisterModel("sprites/s_bfg2.sp2");
	cl_mod_powerscreen = clgi.R_RegisterModel("models/items/armor/effect/tris.md2");
	cl_mod_laser = clgi.R_RegisterModel("models/objects/laser/tris.md2");
	cl_mod_dmspot = clgi.R_RegisterModel("models/objects/dmspot/tris.md2");

	cl_mod_lightning = clgi.R_RegisterModel("models/proj/lightning/tris.md2");
	cl_mod_heatbeam = clgi.R_RegisterModel("models/proj/beam/tris.md2");
	cl_mod_explo4_big = clgi.R_RegisterModel("models/objects/r_explode2/tris.md2");

	//
	// Configure certain models to be a vertical sprite.
	//
	model_t* model = clgi.MOD_ForHandle(cl_mod_explode);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_smoke);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_flash);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_parasite_segment);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_grapple_cable);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_explo4);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_bfg_explo);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_powerscreen);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_laser);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_dmspot);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_lightning);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_heatbeam);
	if (model)
		model->sprite_vertical = qtrue;

	model = clgi.MOD_ForHandle(cl_mod_explo4_big);
	if (model)
		model->sprite_vertical = qtrue;
}

//static void cl_railcore_color_changed(cvar_t* self)
//{
//	if (!SCR_ParseColor(self->string, &railcore_color)) {
//		Com_WPrintf("Invalid value '%s' for '%s'\n", self->string, self->name);
//		Cvar_Reset(self);
//		railcore_color.u32 = U32_RED;
//	}
//}
//
//static void cl_railspiral_color_changed(cvar_t* self)
//{
//	if (!SCR_ParseColor(self->string, &railspiral_color)) {
//		Com_WPrintf("Invalid value '%s' for '%s'\n", self->string, self->name);
//		Cvar_Reset(self);
//		railspiral_color.u32 = U32_BLUE;
//	}
//}

//
//===============
// CLG_AddTempEntities
// 
// Adds all temporal entities to the current frame scene.
//===============
//
void CLG_AddTempEntities(void)
{
	//CL_AddBeams();
	//CL_AddPlayerBeams();
	//CL_AddExplosions();
	//CL_ProcessSustain();
	//CL_AddLasers();
}

//
//===============
// CLG_ClearTempEntities
// 
// Clear the current temporary entities.
//===============
//
void CLG_ClearTempEntities(void)
{
	//CL_ClearBeams();
	//CL_ClearExplosions();
	//CL_ClearLasers();
	//CL_ClearSustains();
}

//
//===============
// CLG_InitTempEntities
// 
// Initialize temporary entity CVars.
//===============
//
void CLG_InitTempEntities(void)
{
	//cl_railtrail_type = clgi.Cvar_Get("cl_railtrail_type", "0", 0);
	//cl_railtrail_time = clgi.Cvar_Get("cl_railtrail_time", "1.0", 0);
	//cl_railcore_color = clgi.Cvar_Get("cl_railcore_color", "red", 0);
	//cl_railcore_color->changed = cl_railcore_color_changed;
	////cl_railcore_color->generator = Com_Color_g;
	//cl_railcore_color_changed(cl_railcore_color);
	//cl_railcore_width = clgi.Cvar_Get("cl_railcore_width", "2", 0);
	//cl_railspiral_color = clgi.Cvar_Get("cl_railspiral_color", "blue", 0);
	//cl_railspiral_color->changed = cl_railspiral_color_changed;
	////cl_railspiral_color->generator = Com_Color_g;
	//cl_railspiral_color_changed(cl_railspiral_color);
	//cl_railspiral_radius = clgi.Cvar_Get("cl_railspiral_radius", "3", 0);
}
