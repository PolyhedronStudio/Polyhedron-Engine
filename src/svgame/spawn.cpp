/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2019, NVIDIA CORPORATION. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "g_local.h"          // Include SVGame header.
#include "player/client.h"    // Include Player Client header.

typedef struct {
    const char    *name; // C++20: STRING: Added const
    void (*spawn)(entity_t *ent);
} spawn_func_t;

typedef struct {
    const char    *name; // C++20: STRING: Added const
    size_t  ofs;
    fieldtype_t type;
} spawn_field_t;

void SP_item_health(entity_t *self);
void SP_item_health_small(entity_t *self);
void SP_item_health_large(entity_t *self);
void SP_item_health_mega(entity_t *self);

void SP_info_player_start(entity_t *ent);
void SP_info_player_deathmatch(entity_t *ent);
void SP_info_player_coop(entity_t *ent);
void SP_info_player_intermission(entity_t *ent);

void SP_func_plat(entity_t *ent);
void SP_func_rotating(entity_t *ent);
void SP_func_button(entity_t *ent);
void SP_func_door(entity_t *ent);

void SP_func_door_rotating(entity_t *ent);
void SP_func_water(entity_t *ent);
void SP_func_train(entity_t *ent);
void SP_func_conveyor(entity_t *self);
void SP_func_wall(entity_t *self);
void SP_func_object(entity_t *self);
void SP_func_explosive(entity_t *self);
void SP_func_timer(entity_t *self);
void SP_func_areaportal(entity_t *ent);
void SP_func_killbox(entity_t *ent);

void SP_trigger_always(entity_t *ent);
void SP_trigger_once(entity_t *ent);
void SP_trigger_multiple(entity_t *ent);
void SP_trigger_relay(entity_t *ent);
void SP_trigger_push(entity_t *ent);
void SP_trigger_hurt(entity_t *ent);
void SP_trigger_key(entity_t *ent);
void SP_trigger_counter(entity_t *ent);
void SP_trigger_elevator(entity_t *ent);
void SP_trigger_gravity(entity_t *ent);
void SP_trigger_monsterjump(entity_t *ent);

void SP_target_temp_entity(entity_t *ent);
void SP_target_speaker(entity_t *ent);
void SP_target_explosion(entity_t *ent);
void SP_target_changelevel(entity_t *ent);
void SP_target_splash(entity_t *ent);
void SP_target_spawner(entity_t *ent);
void SP_target_blaster(entity_t *ent);
void SP_target_crosslevel_trigger(entity_t *ent);
void SP_target_crosslevel_target(entity_t *ent);

void SP_target_lightramp(entity_t *self);
void SP_target_earthquake(entity_t *ent);

void SP_worldspawn(entity_t *ent);

void SP_light(entity_t *self);
void SP_info_null(entity_t *self);
void SP_info_notnull(entity_t *self);

void SP_misc_gib_arm(entity_t *self);
void SP_misc_gib_leg(entity_t *self);
void SP_misc_gib_head(entity_t *self);

void SP_misc_teleporter(entity_t *self);
void SP_misc_teleporter_dest(entity_t *self);

void SP_monster_soldier_light(entity_t *self);
void SP_monster_soldier(entity_t *self);
void SP_monster_soldier_ss(entity_t *self);

void SP_misc_explobox(entity_t* self);

static const spawn_func_t spawn_funcs[] = {
    {"item_health", SP_item_health},
    {"item_health_small", SP_item_health_small},
    {"item_health_large", SP_item_health_large},
    {"item_health_mega", SP_item_health_mega},

    {"info_player_start", SP_info_player_start},
    {"info_player_deathmatch", SP_info_player_deathmatch},
    {"info_player_coop", SP_info_player_coop},
    {"info_player_intermission", SP_info_player_intermission},

    {"func_plat", SP_func_plat},
    {"func_button", SP_func_button},
    {"func_door", SP_func_door},

    {"func_door_rotating", SP_func_door_rotating},
    {"func_rotating", SP_func_rotating},
    {"func_train", SP_func_train},
    {"func_water", SP_func_water},
    {"func_conveyor", SP_func_conveyor},
    {"func_areaportal", SP_func_areaportal},
    {"func_wall", SP_func_wall},
    {"func_object", SP_func_object},
    {"func_timer", SP_func_timer},
    {"func_explosive", SP_func_explosive},
    {"func_killbox", SP_func_killbox},

    {"trigger_always", SP_trigger_always},
    {"trigger_once", SP_trigger_once},
    {"trigger_multiple", SP_trigger_multiple},
    {"trigger_relay", SP_trigger_relay},
    {"trigger_push", SP_trigger_push},
    {"trigger_hurt", SP_trigger_hurt},
    {"trigger_key", SP_trigger_key},
    {"trigger_counter", SP_trigger_counter},
    {"trigger_elevator", SP_trigger_elevator},
    {"trigger_gravity", SP_trigger_gravity},
    {"trigger_monsterjump", SP_trigger_monsterjump},

    {"target_temp_entity", SP_target_temp_entity},
    {"target_speaker", SP_target_speaker},
    {"target_explosion", SP_target_explosion},
    {"target_changelevel", SP_target_changelevel},
    {"target_splash", SP_target_splash},
    {"target_spawner", SP_target_spawner},
    {"target_blaster", SP_target_blaster},
    {"target_crosslevel_trigger", SP_target_crosslevel_trigger},
    {"target_crosslevel_target", SP_target_crosslevel_target},

    {"target_lightramp", SP_target_lightramp},
    {"target_earthquake", SP_target_earthquake},

    {"worldspawn", SP_worldspawn},

    {"light", SP_light},
    {"info_null", SP_info_null},
    {"func_group", SP_info_null},
    {"info_notnull", SP_info_notnull},

    {"misc_gib_arm", SP_misc_gib_arm},
    {"misc_gib_leg", SP_misc_gib_leg},
    {"misc_gib_head", SP_misc_gib_head},

    {"misc_teleporter", SP_misc_teleporter},
    {"misc_teleporter_dest", SP_misc_teleporter_dest},
    {"misc_explobox", SP_misc_explobox},

    {"monster_soldier_light", SP_monster_soldier_light},
    {"monster_soldier", SP_monster_soldier},
    {"monster_soldier_ss", SP_monster_soldier_ss},

    {NULL, NULL}
};

static const spawn_field_t spawn_fields[] = {
    {"classname", FOFS(classname), F_LSTRING},
    {"model", FOFS(model), F_LSTRING},
    {"spawnFlags", FOFS(spawnFlags), F_INT},
    {"speed", FOFS(speed), F_FLOAT},
    {"accel", FOFS(accel), F_FLOAT},
    {"decel", FOFS(decel), F_FLOAT},
    {"target", FOFS(target), F_LSTRING},
    {"targetName", FOFS(targetName), F_LSTRING},
    {"pathTarget", FOFS(pathTarget), F_LSTRING},
    {"deathTarget", FOFS(deathTarget), F_LSTRING},
    {"killTarget", FOFS(killTarget), F_LSTRING},
    {"combatTarget", FOFS(combatTarget), F_LSTRING},
    {"message", FOFS(message), F_LSTRING},
    {"team", FOFS(team), F_LSTRING},
    {"wait", FOFS(wait), F_FLOAT},
    {"delay", FOFS(delay), F_FLOAT},
    {"random", FOFS(random), F_FLOAT},
    {"moveOrigin", FOFS(moveOrigin), F_VECTOR},
    {"moveAngles", FOFS(moveAngles), F_VECTOR},
    {"style", FOFS(style), F_INT},
    {"customLightStyle", FOFS(customLightStyle), F_LSTRING},
    {"count", FOFS(count), F_INT},
    {"health", FOFS(health), F_INT},
    {"sounds", FOFS(sounds), F_INT},
    {"light", 0, F_IGNORE},
    {"dmg", FOFS(dmg), F_INT},
    {"mass", FOFS(mass), F_INT},
    {"volume", FOFS(volume), F_FLOAT},
    {"attenuation", FOFS(attenuation), F_FLOAT},
    {"map", FOFS(map), F_LSTRING},
    {"origin", FOFS(state.origin), F_VECTOR},
    {"angles", FOFS(state.angles), F_VECTOR},
    {"angle", FOFS(state.angles), F_ANGLEHACK},

    {NULL}
};

// temp spawn vars -- only valid when the spawn function is called
static const spawn_field_t temp_fields[] = {
    {"lip", STOFS(lip), F_INT},
    {"distance", STOFS(distance), F_INT},
    {"height", STOFS(height), F_INT},
    {"noise", STOFS(noise), F_LSTRING},
    {"pausetime", STOFS(pausetime), F_FLOAT},
    {"item", STOFS(item), F_LSTRING},

    {"gravity", STOFS(gravity), F_LSTRING},
    {"sky", STOFS(sky), F_LSTRING},
    {"skyrotate", STOFS(skyrotate), F_FLOAT},
    {"skyaxis", STOFS(skyaxis), F_VECTOR},
    {"minyaw", STOFS(minyaw), F_FLOAT},
    {"maxyaw", STOFS(maxyaw), F_FLOAT},
    {"minpitch", STOFS(minpitch), F_FLOAT},
    {"maxpitch", STOFS(maxpitch), F_FLOAT},
    {"nextmap", STOFS(nextmap), F_LSTRING},

    {NULL}
};


/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn(entity_t *ent)
{
    const spawn_func_t *s;
    gitem_t *item;
    int     i;

    if (!ent->classname) {
        gi.DPrintf("ED_CallSpawn: NULL classname\n");
        return;
    }

    // check item spawn functions
    for (i = 0, item = itemlist ; i < game.num_items ; i++, item++) {
        if (!item->classname)
            continue;
        if (!strcmp(item->classname, ent->classname)) {
            // found it
            SpawnItem(ent, item);
            return;
        }
    }

    // check normal spawn functions
    for (s = spawn_funcs ; s->name ; s++) {
        if (!strcmp(s->name, ent->classname)) {
            // found it
            s->spawn(ent);
            return;
        }
    }
    gi.DPrintf("%s doesn't have a spawn function\n", ent->classname);
}

/*
=============
ED_NewString
=============
*/
static char *ED_NewString(const char *string)
{
    char    *newb, *new_p;
    int     i, l;

    l = strlen(string) + 1;

    newb = (char*)gi.TagMalloc(l, TAG_LEVEL); // CPP: Cast

    new_p = newb;

    for (i = 0 ; i < l ; i++) {
        if (string[i] == '\\' && i < l - 1) {
            i++;
            if (string[i] == 'n')
                *new_p++ = '\n';
            else
                *new_p++ = '\\';
        } else
            *new_p++ = string[i];
    }

    return newb;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
static qboolean ED_ParseField(const spawn_field_t *fields, const char *key, const char *value, byte *b)
{
    const spawn_field_t *f;
    float   v;
    vec3_t  vec;

    for (f = fields ; f->name ; f++) {
        if (!Q_stricmp(f->name, key)) {
            // found it
            switch (f->type) {
            case F_LSTRING:
                *(char **)(b + f->ofs) = ED_NewString(value);
                break;
            case F_VECTOR:
                if (sscanf(value, "%f %f %f", &vec[0], &vec[1], &vec[2]) != 3) {
                    gi.DPrintf("%s: couldn't parse '%s'\n", __func__, key);
                    VectorClear(vec);
                }
                ((float *)(b + f->ofs))[0] = vec[0];
                ((float *)(b + f->ofs))[1] = vec[1];
                ((float *)(b + f->ofs))[2] = vec[2];
                break;
            case F_INT:
                *(int *)(b + f->ofs) = atoi(value);
                break;
            case F_FLOAT:
                *(float *)(b + f->ofs) = atof(value);
                break;
            case F_ANGLEHACK:
                v = atof(value);
                ((float *)(b + f->ofs))[0] = 0;
                ((float *)(b + f->ofs))[1] = v;
                ((float *)(b + f->ofs))[2] = 0;
                break;
            case F_IGNORE:
                break;
            default:
                break;
            }
            return true;
        }
    }
    return false;
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
void ED_ParseEntity(const char **data, entity_t *ent)
{
    qboolean    init;
    char        *key, *value;

    init = false;
    memset(&st, 0, sizeof(st));

// go through all the dictionary pairs
    while (1) {
        // parse key
        key = COM_Parse(data);
        if (key[0] == '}')
            break;
        if (!*data)
            gi.Error("%s: EOF without closing brace", __func__);

        // parse value
        value = COM_Parse(data);
        if (!*data)
            gi.Error("%s: EOF without closing brace", __func__);

        if (value[0] == '}')
            gi.Error("%s: closing brace without data", __func__);

        init = true;

        // keynames with a leading underscore are used for utility comments,
        // and are immediately discarded by quake
        if (key[0] == '_')
            continue;

        if (!ED_ParseField(spawn_fields, key, value, (byte *)ent)) {
            if (!ED_ParseField(temp_fields, key, value, (byte *)&st)) {
                gi.DPrintf("%s: %s is not a field\n", __func__, key);
            }
        }
    }

    if (!init)
        memset(ent, 0, sizeof(*ent));
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the EntityFlags::TeamSlave flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams(void)
{
    entity_t *e, *e2, *chain;
    int     i, j;
    int     c, c2;

    c = 0;
    c2 = 0;
    for (i = 1, e = g_edicts + i ; i < globals.num_edicts ; i++, e++) {
        if (!e->inUse)
            continue;
        if (!e->team)
            continue;
        if (e->flags & EntityFlags::TeamSlave)
            continue;
        chain = e;
        e->teamMasterPtr = e;
        c++;
        c2++;
        for (j = i + 1, e2 = e + 1 ; j < globals.num_edicts ; j++, e2++) {
            if (!e2->inUse)
                continue;
            if (!e2->team)
                continue;
            if (e2->flags & EntityFlags::TeamSlave)
                continue;
            if (!strcmp(e->team, e2->team)) {
                c2++;
                chain->teamChainPtr = e2;
                e2->teamMasterPtr = e;
                chain = e2;
                e2->flags |= EntityFlags::TeamSlave;
            }
        }
    }

    gi.DPrintf("%i teams with %i entities\n", c, c2);
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void SpawnEntities(const char *mapname, const char *entities, const char *spawnpoint)
{
    entity_t     *ent;
    int         inhibit;
    char        *com_token;
    int         i;
    float       skill_level;

    skill_level = floor(skill->value);
    if (skill_level < 0)
        skill_level = 0;
    if (skill_level > 3)
        skill_level = 3;
    if (skill->value != skill_level)
        gi.cvar_forceset("skill", va("%f", skill_level));

    SaveClientData();

    gi.FreeTags(TAG_LEVEL);

    memset(&level, 0, sizeof(level));
    // WatIs: C++-ify: Note that this may be a problem maker.
    //for (int i = 0; i < game.maxentities; i++) {
    //    g_edicts[i] = entity_t();
    //}
    memset(g_edicts, 0, game.maxentities * sizeof(g_edicts[0])); // WatIs: C++-ify: Note that this may be a problem maker.

    strncpy(level.mapname, mapname, sizeof(level.mapname) - 1);
    strncpy(game.spawnpoint, spawnpoint, sizeof(game.spawnpoint) - 1);

    // set client fields on player ents
    for (i = 0 ; i < game.maxClients ; i++)
        g_edicts[i + 1].client = game.clients + i;

    ent = NULL;
    inhibit = 0;

// parse ents
    while (1) {
        // parse the opening brace
        com_token = COM_Parse(&entities);
        if (!entities)
            break;
        if (com_token[0] != '{')
            gi.Error("ED_LoadFromFile: found %s when expecting {", com_token);

        if (!ent)
            ent = g_edicts;
        else
            ent = G_Spawn();
        ED_ParseEntity(&entities, ent);

        // yet another map hack
        if (!Q_stricmp(level.mapname, "command") && !Q_stricmp(ent->classname, "trigger_once") && !Q_stricmp(ent->model, "*27"))
            ent->spawnFlags &= ~EntitySpawnFlags::NotHard;

        // remove things (except the world) from different skill levels or deathmatch
        if (ent != g_edicts) {
			if (nomonsters->value && (strstr(ent->classname, "monster") || strstr(ent->classname, "misc_deadsoldier") || strstr(ent->classname, "misc_insane"))) {
				G_FreeEntity(ent);
				inhibit++;
				continue;
			}
            if (deathmatch->value) {
                if (ent->spawnFlags & EntitySpawnFlags::NotDeathMatch) {
                    G_FreeEntity(ent);
                    inhibit++;
                    continue;
                }
            } else {
                if ( /* ((coop->value) && (ent->spawnFlags & EntitySpawnFlags::NotCoop)) || */
                    ((skill->value == 0) && (ent->spawnFlags & EntitySpawnFlags::NotEasy)) ||
                    ((skill->value == 1) && (ent->spawnFlags & EntitySpawnFlags::NotMedium)) ||
                    (((skill->value == 2) || (skill->value == 3)) && (ent->spawnFlags & EntitySpawnFlags::NotHard))
                ) {
                    G_FreeEntity(ent);
                    inhibit++;
                    continue;
                }
            }

            ent->spawnFlags &= ~(EntitySpawnFlags::NotEasy | EntitySpawnFlags::NotMedium | EntitySpawnFlags::NotHard | EntitySpawnFlags::NotCoop | EntitySpawnFlags::NotDeathMatch);
        }

        ED_CallSpawn(ent);
    }

    gi.DPrintf("%i entities inhibited\n", inhibit);

#ifdef DEBUG
    i = 1;
    ent = EDICT_NUM(i);
    while (i < globals.pool.num_edicts) {
        if (ent->inUse != 0 || ent->inUse != 1)
            Com_DPrintf("Invalid entity %d\n", i);
        i++, ent++;
    }
#endif

    G_FindTeams();

    PlayerTrail_Init();
}


//===================================================================

#if 0
// cursor positioning
xl <value>
xr <value>
yb <value>
yt <value>
xv <value>
yv <value>

// drawing
statpic <name>
pic <stat>
num <fieldwidth> <stat>
string <stat>

// control
if <stat>
ifeq <stat> <value>
ifbit <stat> <value>
endif

#endif

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

// spectator
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


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"   environment map name
"skyaxis"   vector axis for rotating sky
"skyrotate" speed of rotation in degrees/second
"sounds"    music cd track number
"gravity"   800 is default gravity
"message"   text to print at user logon
*/
void SP_worldspawn(entity_t *ent)
{
    ent->moveType = MoveType::Push;
    ent->solid = Solid::BSP;
    ent->inUse = true;          // since the world doesn't use G_Spawn()
    ent->state.modelIndex = 1;      // world model is always index 1

    //---------------

    // reserve some spots for dead player bodies for coop / deathmatch
    level.body_que = 0;
    for (int i = 0; i < BODY_QUEUE_SIZE; i++) {
        entity_t* ent = G_Spawn();
        ent->classname = "bodyque";
    }

    // set configstrings for items
    SetItemNames();

    if (st.nextmap)
        strcpy(level.nextmap, st.nextmap);

    // make some data visible to the server

    if (ent->message && ent->message[0]) {
        gi.configstring(ConfigStrings::Name, ent->message);
        strncpy(level.level_name, ent->message, sizeof(level.level_name));
    } else
        strncpy(level.level_name, level.mapname, sizeof(level.level_name));

    if (st.sky && st.sky[0])
        gi.configstring(ConfigStrings::Sky, st.sky);
    else
        gi.configstring(ConfigStrings::Sky, "unit1_");

    gi.configstring(ConfigStrings::SkyRotate, va("%f", st.skyrotate));

    gi.configstring(ConfigStrings::SkyAxis, va("%f %f %f",
                                   st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]));

    gi.configstring(ConfigStrings::CdTrack, va("%i", ent->sounds));

    gi.configstring(ConfigStrings::MaxClients, va("%i", (int)(maxClients->value)));

    // status bar program
    if (deathmatch->value)
        gi.configstring(ConfigStrings::StatusBar, dm_statusbar);
    else
        gi.configstring(ConfigStrings::StatusBar, single_statusbar);

    //---------------


    // help icon for statusbar
    gi.ImageIndex("i_help");
    level.pic_health = gi.ImageIndex("i_health");
    gi.ImageIndex("help");
    gi.ImageIndex("field_3");

    if (!st.gravity)
        gi.cvar_set("sv_gravity", "750");
    else
        gi.cvar_set("sv_gravity", st.gravity);

    snd_fry = gi.SoundIndex("player/fry.wav");  // standing in lava / slime

    PrecacheItem(FindItem("Blaster"));

    gi.SoundIndex("player/lava1.wav");
    gi.SoundIndex("player/lava2.wav");

    gi.SoundIndex("misc/pc_up.wav");
    gi.SoundIndex("misc/talk1.wav");

    gi.SoundIndex("misc/udeath.wav");

    // gibs
    gi.SoundIndex("items/respawn1.wav");

    // sexed sounds
    gi.SoundIndex("*death1.wav");
    gi.SoundIndex("*death2.wav");
    gi.SoundIndex("*death3.wav");
    gi.SoundIndex("*death4.wav");
    gi.SoundIndex("*fall1.wav");
    gi.SoundIndex("*fall2.wav");
    gi.SoundIndex("*gurp1.wav");        // drowning damage
    gi.SoundIndex("*gurp2.wav");
    gi.SoundIndex("*jump1.wav");        // player jump
    gi.SoundIndex("*pain25_1.wav");
    gi.SoundIndex("*pain25_2.wav");
    gi.SoundIndex("*pain50_1.wav");
    gi.SoundIndex("*pain50_2.wav");
    gi.SoundIndex("*pain75_1.wav");
    gi.SoundIndex("*pain75_2.wav");
    gi.SoundIndex("*pain100_1.wav");
    gi.SoundIndex("*pain100_2.wav");

    // sexed models
    // THIS ORDER MUST MATCH THE DEFINES IN g_local.h
    // you can add more, max 15
    gi.ModelIndex("#w_blaster.md2");
    gi.ModelIndex("#w_shotgun.md2");
    gi.ModelIndex("#w_sshotgun.md2");
    gi.ModelIndex("#w_machinegun.md2");
    gi.ModelIndex("#w_chaingun.md2");
    gi.ModelIndex("#a_grenades.md2");
    gi.ModelIndex("#w_glauncher.md2");
    gi.ModelIndex("#w_rlauncher.md2");
    gi.ModelIndex("#w_hyperblaster.md2");
    gi.ModelIndex("#w_railgun.md2");
    gi.ModelIndex("#w_bfg.md2");

    //-------------------

    gi.SoundIndex("player/gasp1.wav");      // gasping for air
    gi.SoundIndex("player/gasp2.wav");      // head breaking surface, not gasping

    gi.SoundIndex("player/watr_in.wav");    // feet hitting water
    gi.SoundIndex("player/watr_out.wav");   // feet leaving water

    gi.SoundIndex("player/watr_un.wav");    // head going underwater

    gi.SoundIndex("player/u_breath1.wav");
    gi.SoundIndex("player/u_breath2.wav");

    gi.SoundIndex("items/pkup.wav");        // bonus item pickup
    gi.SoundIndex("world/land.wav");        // landing thud
    gi.SoundIndex("misc/h2ohit1.wav");      // landing splash

    gi.SoundIndex("items/damage.wav");
    gi.SoundIndex("items/protect.wav");
    gi.SoundIndex("items/protect4.wav");
    gi.SoundIndex("weapons/noammo.wav");

    gi.SoundIndex("infantry/inflies1.wav");

    sm_meat_index = gi.ModelIndex("models/objects/gibs/sm_meat/tris.md2");
    gi.ModelIndex("models/objects/gibs/arm/tris.md2");
    gi.ModelIndex("models/objects/gibs/bone/tris.md2");
    gi.ModelIndex("models/objects/gibs/bone2/tris.md2");
    gi.ModelIndex("models/objects/gibs/chest/tris.md2");
    gi.ModelIndex("models/objects/gibs/skull/tris.md2");
    gi.ModelIndex("models/objects/gibs/head2/tris.md2");

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

    // 0 normal
    gi.configstring(ConfigStrings::Lights+ 0, "m");

    // 1 FLICKER (first variety)
    gi.configstring(ConfigStrings::Lights+ 1, "mmnmmommommnonmmonqnmmo");

    // 2 SLOW STRONG PULSE
    gi.configstring(ConfigStrings::Lights+ 2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

    // 3 CANDLE (first variety)
    gi.configstring(ConfigStrings::Lights+ 3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

    // 4 FAST STROBE
    gi.configstring(ConfigStrings::Lights+ 4, "mamamamamama");

    // 5 GENTLE PULSE 1
    gi.configstring(ConfigStrings::Lights+ 5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

    // 6 FLICKER (second variety)
    gi.configstring(ConfigStrings::Lights+ 6, "nmonqnmomnmomomno");

    // 7 CANDLE (second variety)
    gi.configstring(ConfigStrings::Lights+ 7, "mmmaaaabcdefgmmmmaaaammmaamm");

    // 8 CANDLE (third variety)
    gi.configstring(ConfigStrings::Lights+ 8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

    // 9 SLOW STROBE (fourth variety)
    gi.configstring(ConfigStrings::Lights+ 9, "aaaaaaaazzzzzzzz");

    // 10 FLUORESCENT FLICKER
    gi.configstring(ConfigStrings::Lights+ 10, "mmamammmmammamamaaamammma");

    // 11 SLOW PULSE NOT FADE TO BLACK
    gi.configstring(ConfigStrings::Lights+ 11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

    // styles 32-62 are assigned by the light program for switchable lights

    // 63 testing
    gi.configstring(ConfigStrings::Lights+ 63, "a");
}

