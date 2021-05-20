/*
// LICENSE HERE.

//
// WorldSpawn.cpp
//
//
*/
#include "../g_local.h"              // SVGame.

#include "base/SVGBaseEntity.h"
#include "WorldSpawn.h"

static const char single_statusbar[] =
"yb -24 "

// health
"xv 0 "
"hnum "
"xv 50 "
"pic 0 "

// ammo
"if 2 "
"   xv  100 "
"   anum "
"   xv  150 "
"   pic 2 "
"endif "

// armor
"if 4 "
"   xv  200 "
"   rnum "
"   xv  250 "
"   pic 4 "
"endif "

// selected item
"if 6 "
"   xv  296 "
"   pic 6 "
"endif "

"yb -50 "

// picked up item
"if 7 "
"   xv  0 "
"   pic 7 "
"   xv  26 "
"   yb  -42 "
"   stat_string 8 "
"   yb  -50 "
"endif "

// timer
"if 9 "
"   xv  262 "
"   num 2   10 "
"   xv  296 "
"   pic 9 "
"endif "

//  help / weapon icon
"if 11 "
"   xv  148 "
"   pic 11 "
"endif "
;

static const char dm_statusbar[] =
"yb -24 "

// health
"xv 0 "
"hnum "
"xv 50 "
"pic 0 "

// ammo
"if 2 "
"   xv  100 "
"   anum "
"   xv  150 "
"   pic 2 "
"endif "

// armor
"if 4 "
"   xv  200 "
"   rnum "
"   xv  250 "
"   pic 4 "
"endif "

// selected item
"if 6 "
"   xv  296 "
"   pic 6 "
"endif "

"yb -50 "

// picked up item
"if 7 "
"   xv  0 "
"   pic 7 "
"   xv  26 "
"   yb  -42 "
"   stat_string 8 "
"   yb  -50 "
"endif "

// timer
"if 9 "
"   xv  246 "
"   num 2   10 "
"   xv  296 "
"   pic 9 "
"endif "

//  help / weapon icon
"if 11 "
"   xv  148 "
"   pic 11 "
"endif "

//  frags
"xr -50 "
"yt 2 "
"num 3 14 "

// isSpectator
"if 17 "
"xv 0 "
"yb -58 "
"string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
"xv 0 "
"yb -68 "
"string \"Chasing\" "
"xv 64 "
"stat_string 16 "
"endif "
;

// Constructor/Deconstructor.
WorldSpawn::WorldSpawn(Entity* svEntity) : SVGBaseEntity(svEntity) {

}
WorldSpawn::~WorldSpawn() {

}

// Interface functions. 
void WorldSpawn::Precache() {
    // Parent class precache.
    SVGBaseEntity::Precache();

    // help icon for statusbar
    SVG_PrecacheImage("i_help");
    level.pic_health = SVG_PrecacheImage("i_health");
    SVG_PrecacheImage("help");
    SVG_PrecacheImage("field_3");

    if (!st.gravity)
        gi.cvar_set("sv_gravity", "750");
    else
        gi.cvar_set("sv_gravity", st.gravity);

    snd_fry = SVG_PrecacheSound("player/fry.wav");  // standing in lava / slime

    SVG_PrecacheItem(SVG_FindItemByPickupName("Blaster"));
    gi.DPrintf("WorldSpawn::Spawn();\n");

    SVG_PrecacheSound("player/lava1.wav");
    SVG_PrecacheSound("player/lava2.wav");

    SVG_PrecacheSound("misc/pc_up.wav");
    SVG_PrecacheSound("misc/talk1.wav");

    SVG_PrecacheSound("misc/udeath.wav");

    // gibs
    SVG_PrecacheSound("items/respawn1.wav");

    // sexed sounds
    SVG_PrecacheSound("*death1.wav");
    SVG_PrecacheSound("*death2.wav");
    SVG_PrecacheSound("*death3.wav");
    SVG_PrecacheSound("*death4.wav");
    SVG_PrecacheSound("*fall1.wav");
    SVG_PrecacheSound("*fall2.wav");
    SVG_PrecacheSound("*gurp1.wav");        // drowning damage
    SVG_PrecacheSound("*gurp2.wav");
    SVG_PrecacheSound("*jump1.wav");        // player jump
    SVG_PrecacheSound("*pain25_1.wav");
    SVG_PrecacheSound("*pain25_2.wav");
    SVG_PrecacheSound("*pain50_1.wav");
    SVG_PrecacheSound("*pain50_2.wav");
    SVG_PrecacheSound("*pain75_1.wav");
    SVG_PrecacheSound("*pain75_2.wav");
    SVG_PrecacheSound("*pain100_1.wav");
    SVG_PrecacheSound("*pain100_2.wav");

    // sexed models
    // THIS ORDER MUST MATCH THE DEFINES IN g_local.h
    // you can add more, max 15
    SVG_PrecacheModel("#w_blaster.md2");
    SVG_PrecacheModel("#w_shotgun.md2");
    SVG_PrecacheModel("#w_sshotgun.md2");
    SVG_PrecacheModel("#w_machinegun.md2");
    SVG_PrecacheModel("#w_chaingun.md2");
    SVG_PrecacheModel("#a_grenades.md2");
    SVG_PrecacheModel("#w_glauncher.md2");
    SVG_PrecacheModel("#w_rlauncher.md2");
    SVG_PrecacheModel("#w_hyperblaster.md2");
    SVG_PrecacheModel("#w_railgun.md2");
    SVG_PrecacheModel("#w_bfg.md2");

    //-------------------

    SVG_PrecacheSound("player/gasp1.wav");      // gasping for air
    SVG_PrecacheSound("player/gasp2.wav");      // head breaking surface, not gasping

    SVG_PrecacheSound("player/watr_in.wav");    // feet hitting water
    SVG_PrecacheSound("player/watr_out.wav");   // feet leaving water

    SVG_PrecacheSound("player/watr_un.wav");    // head going underwater

    SVG_PrecacheSound("player/u_breath1.wav");
    SVG_PrecacheSound("player/u_breath2.wav");

    SVG_PrecacheSound("items/pkup.wav");        // bonus item pickup
    SVG_PrecacheSound("world/land.wav");        // landing thud
    SVG_PrecacheSound("misc/h2ohit1.wav");      // landing splash

    SVG_PrecacheSound("items/damage.wav");
    SVG_PrecacheSound("items/protect.wav");
    SVG_PrecacheSound("items/protect4.wav");
    SVG_PrecacheSound("weapons/noammo.wav");

    SVG_PrecacheSound("infantry/inflies1.wav");

    sm_meat_index = SVG_PrecacheModel("models/objects/gibs/sm_meat/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/arm/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/bone/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/bone2/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/chest/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/skull/tris.md2");
    SVG_PrecacheModel("models/objects/gibs/head2/tris.md2");
}
void WorldSpawn::Spawn() {
    // Parent class spawn.
    SVGBaseEntity::Spawn();

    // Set WorldSpawn specific properties.
    SetMoveType(MoveType::Push);
    SetSolid(Solid::BSP);
    SetInUse(true);          // since the world doesn't use SVG_Spawn()
    SetModelIndex(1);      // world model is always index 1

    //---------------

    // Reserve some spots for dead player bodies for coop / deathmatch
    level.bodyQue = 0;
    for (int i = 0; i < BODY_QUEUE_SIZE; i++) {
        Entity* ent = SVG_Spawn();
        ent->className = "bodyque";
    }

    // set configstrings for items
    SVG_SetItemNames();

    if (st.nextMap)
        strcpy(level.nextMap, st.nextMap);

    // make some data visible to the server
    if (GetServerEntity()->message && GetServerEntity()->message[0]) {
        gi.configstring(ConfigStrings::Name, GetServerEntity()->message);
        strncpy(level.levelName, GetServerEntity()->message, sizeof(level.levelName));
    }
    else
        strncpy(level.levelName, level.mapName, sizeof(level.levelName));

    if (st.sky && st.sky[0])
        gi.configstring(ConfigStrings::Sky, st.sky);
    else
        gi.configstring(ConfigStrings::Sky, "unit1_");

    gi.configstring(ConfigStrings::SkyRotate, va("%f", st.skyrotate));

    gi.configstring(ConfigStrings::SkyAxis, va("%f %f %f",
        st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));

    gi.configstring(ConfigStrings::CdTrack, va("%i", GetServerEntity()->sounds));

    gi.configstring(ConfigStrings::MaxClients, va("%i", (int)(maxClients->value)));

    // status bar program
    if (deathmatch->value)
        gi.configstring(ConfigStrings::StatusBar, dm_statusbar);
    else
        gi.configstring(ConfigStrings::StatusBar, single_statusbar);

    //---------------

    //
    // Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
    //

        // 0 normal
    gi.configstring(ConfigStrings::Lights + 0, "m");

    // 1 FLICKER (first variety)
    gi.configstring(ConfigStrings::Lights + 1, "mmnmmommommnonmmonqnmmo");

    // 2 SLOW STRONG PULSE
    gi.configstring(ConfigStrings::Lights + 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

    // 3 CANDLE (first variety)
    gi.configstring(ConfigStrings::Lights + 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

    // 4 FAST STROBE
    gi.configstring(ConfigStrings::Lights + 4, "mamamamamama");

    // 5 GENTLE PULSE 1
    gi.configstring(ConfigStrings::Lights + 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

    // 6 FLICKER (second variety)
    gi.configstring(ConfigStrings::Lights + 6, "nmonqnmomnmomomno");

    // 7 CANDLE (second variety)
    gi.configstring(ConfigStrings::Lights + 7, "mmmaaaabcdefgmmmmaaaammmaamm");

    // 8 CANDLE (third variety)
    gi.configstring(ConfigStrings::Lights + 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

    // 9 SLOW STROBE (fourth variety)
    gi.configstring(ConfigStrings::Lights + 9, "aaaaaaaazzzzzzzz");

    // 10 FLUORESCENT FLICKER
    gi.configstring(ConfigStrings::Lights + 10, "mmamammmmammamamaaamammma");

    // 11 SLOW PULSE NOT FADE TO BLACK
    gi.configstring(ConfigStrings::Lights + 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

    // styles 32-62 are assigned by the light program for switchable lights

    // 63 testing
    gi.configstring(ConfigStrings::Lights + 63, "a");

    // Set think function.
    //SetThinkCallback()
}
void WorldSpawn::PostSpawn() {
    // Parent class PostSpawn.
    SVGBaseEntity::PostSpawn();
}
void WorldSpawn::Think() {
    // Parent class think.
    SVGBaseEntity::Think();
}

void WorldSpawn::WorldSpawnThink(void) {
    
}