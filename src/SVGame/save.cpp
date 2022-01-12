/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "g_local.h"
#include "functionpointers.h"

//#define _DEBUG
typedef struct {
    fieldtype_t type;
#ifdef _DEBUG
    const char *name; // C++20: STRING: Added const to char*
#endif
    size_t ofs;
    size_t size;
} save_field_t;

#ifdef _DEBUG
#define _FA(type, name, size) { type, #name, _OFS(name), size }
#else
#define _FA(type, name, size) { type, _OFS(name), size }
#endif
#define _F(type, name) _FA(type, name, 1)
#define SZ(name, size) _FA(F_ZSTRING, name, size)
#define BA(name, size) _FA(F_BYTE, name, size)
#define B(name) BA(name, 1)
#define SA(name, size) _FA(F_SHORT, name, size)
#define S(name) SA(name, 1)
#define IA(name, size) _FA(F_INT, name, size)
#define I(name) IA(name, 1)
#define FA(name, size) _FA(F_FLOAT, name, size)
#define F(name) FA(name, 1)
#define L(name) _F(F_LSTRING, name)
#define V(name) _F(F_VECTOR, name)
#define T(name) _F(F_ITEM, name)
#define E(name) _F(F_EDICT, name)
#define P(name, type) _FA(F_POINTER, name, type)

static const save_field_t entityfields[] = {
#define _OFS FOFS
    V(state.origin),
    V(state.angles),
    V(state.oldOrigin),
    I(state.modelIndex),
    I(state.modelIndex2),
    I(state.modelIndex3),
    I(state.modelIndex4),
    I(state.frame),
    I(state.skinNumber),
    I(state.effects),
    I(state.renderEffects),
    I(state.solid),
    I(state.sound),
    I(state.eventID),

    // [...]

    I(serverFlags),
    V(mins),
    V(maxs),
    V(absMin),
    V(absMax),
    V(size),
    I(solid),
    I(clipMask),
    E(owner),

    //I(classEntity->GetMoveType()),
//    I(flags),

//    L(model),
    F(freeTime),

    L(message),
    L(className),
//    I(spawnFlags),

    F(timeStamp),

    L(target),
    L(targetName),
    L(killTarget),
    L(team),
    L(pathTarget),
    E(targetEntityPtr),

    F(speed),
    F(acceleration),
    F(deceleration),
    V(moveDirection),
    V(position1),
    V(position2),

//    V(velocity),
//    V(angularVelocity),
//    I(mass),
//    F(airFinishedTime),
//    F(gravity),

    E(goalEntityPtr),
    E(moveTargetPtr),
//    F(yawSpeed),
 //   F(idealYawAngle),

 //   F(nextThinkTime),
    //P(PreThink, P_prethink),
    //P(Think, P_think),
    //P(Blocked, P_blocked),
    //P(Touch, P_touch),
    //P(Use, P_use),
    //P(Pain, P_pain),
    //P(Die, P_die),

//    F(debounceTouchTime),
//    F(debouncePainTime),
//    F(debounceDamageTime),
//    F(debounceSoundTime),
//    F(lastMoveTime),

//    I(health),
//    I(maxHealth),
//    I(gibHealth),
//    I(deadFlag),
//    I(showHostile),

 //   F(powerArmorTime),

    L(map),

    //I(viewHeight),
    //I(takeDamage),
    //I(damage),
    //I(radiusDamage),
    //F(damageRadius),
    //I(sounds),
    I(count),

    E(chain),
    //E(enemy),
    //E(oldEnemyPtr),
    //E(activator),
    //E(groundEntityPtr),
    //I(groundEntityLinkCount),
    //E(teamChainPtr),
    //E(teamMasterPtr),

    E(myNoisePtr),
    E(myNoise2Ptr),

    I(noiseIndex),
    I(noiseIndex2),
    F(volume),
    F(attenuation),

//    F(wait),
//    F(delay),
    F(random),

    F(teleportTime),

//    I(waterType),
//    I(waterLevel),

    V(moveOrigin),
    V(moveAngles),

    I(lightLevel),

    I(style),
    L(customLightStyle),

    T(item),

//    V(moveInfo.startOrigin),
    //V(moveInfo.startAngles),
    //V(moveInfo.endOrigin),
    //V(moveInfo.endAngles),

    //I(moveInfo.startSoundIndex),
    //I(moveInfo.middleSoundIndex),
    //I(moveInfo.endSoundIndex),

    //F(moveInfo.acceleration),
    //F(moveInfo.speed),
    //F(moveInfo.deceleration),
    //F(moveInfo.distance),

    //F(moveInfo.wait),

    //I(moveInfo.state),
    //V(moveInfo.dir),
    //F(moveInfo.currentSpeed),
    //F(moveInfo.moveSpeed),
    //F(moveInfo.nextSpeed),
    //F(moveInfo.remainingDistance),
    //F(moveInfo.deceleratedDistance),
    //P(moveInfo.OnEndFunction, P_moveinfo_endfunc),

    {(fieldtype_t)0}
#undef _OFS
};

static const save_field_t levelfields[] = {
#define _OFS LLOFS
    I(frameNumber),
    F(time),

    SZ(levelName, MAX_QPATH),
    SZ(mapName, MAX_QPATH),
    SZ(nextMap, MAX_QPATH),

    F(intermission.time),
    L(intermission.changeMap),
    I(intermission.exitIntermission),
    V(intermission.origin),
    V(intermission.viewAngle),

    E(sightClient),

    E(sightEntity),
    I(sightEntityFrameNumber),
    E(soundEntity),
    I(soundEntityFrameNumber),
    E(sound2Entity),
    I(sound2EntityFrameNumber),

    I(pic_health),

    I(totalMonsters),
    I(killedMonsters),

    I(bodyQue),

    I(powerCubes),

    {(fieldtype_t)0}
#undef _OFS
};

static const save_field_t clientfields[] = {
#define _OFS CLOFS
    I(playerState.pmove.type),

    V(playerState.pmove.origin),
    V(playerState.pmove.velocity),
    B(playerState.pmove.flags),
    B(playerState.pmove.time),
    S(playerState.pmove.gravity),
    SA(playerState.pmove.deltaAngles, 3),

    V(playerState.pmove.viewAngles),
    V(playerState.pmove.viewOffset),
    V(playerState.kickAngles),

    V(playerState.gunAngles),
    V(playerState.gunOffset),
    I(playerState.gunIndex),
    I(playerState.gunFrame),

    FA(playerState.blend, 4),

    F(playerState.fov),

    I(playerState.rdflags),

    SA(playerState.stats, MAX_STATS),

    SZ(persistent.userinfo, MAX_INFO_STRING),
    SZ(persistent.netname, 16),
    I(persistent.hand),

    I(persistent.isConnected),

    I(persistent.health),
    I(persistent.maxHealth),
    I(persistent.savedFlags),

    I(persistent.selectedItem),
    IA(persistent.inventory, MAX_ITEMS),

    I(persistent.maxBullets),
    I(persistent.maxShells),
    I(persistent.maxRockets),
    I(persistent.maxGrenades),
    I(persistent.maxCells),
    I(persistent.maxSlugs),

    T(persistent.activeWeapon),
    T(persistent.lastWeapon),

    I(persistent.powerCubes),
    I(persistent.score),

    I(persistent.isSpectator),

    I(showScores),
    I(showInventory),
    I(showHelpIcon),

    I(ammoIndex),

    T(newWeapon),

    I(damages.armor),
    I(damages.powerArmor),
    I(damages.blood),
    I(damages.knockBack),
    V(damages.from),

    F(killerYaw),

    I(weaponState),

    V(kickAngles),
    V(kickOrigin),
    F(viewDamage.roll),
    F(viewDamage.pitch),
    F(viewDamage.time),
    F(fallTime),
    F(fallValue),
    F(damageAlpha),
    F(bonusAlpha),
    V(damageBlend),
    V(aimAngles),
    F(bobTime),
    V(oldViewAngles),
    V(oldVelocity),

    F(nextDrownTime),
    I(oldWaterLevel),

    I(machinegunShots),

    I(animation.endFrame),
    I(animation.priorityAnimation),
    I(animation.isDucking),
    I(animation.isRunning),

    // powerup timers
    I(weaponSound),

    F(pickupMessageTime),

    {(fieldtype_t)0}
#undef _OFS
};

static const save_field_t gamefields[] = {
#define _OFS GLOFS
    I(maximumClients),
    I(maxEntities),

    I(serverflags),

    I(numberOfItems),

    I(autoSaved),

    {(fieldtype_t)0}
#undef _OFS
};

//=========================================================

static void write_data(void *buf, size_t len, FILE *f)
{
    if (fwrite(buf, 1, len, f) != len) {
        gi.Error("%s: couldn't write %" PRIz " bytes", __func__, len); // CPP: String fix.
    }
}

static void write_short(FILE *f, short v)
{
    v = LittleShort(v);
    write_data(&v, sizeof(v), f);
}

static void write_int(FILE *f, int v)
{
    v = LittleLong(v);
    write_data(&v, sizeof(v), f);
}

static void write_float(FILE *f, float v)
{
    v = LittleFloat(v);
    write_data(&v, sizeof(v), f);
}

static void write_string(FILE *f, char *s)
{
    size_t len;

    if (!s) {
        write_int(f, -1);
        return;
    }

    len = strlen(s);
    write_int(f, len);
    write_data(s, len, f);
}

static void write_vector(FILE *f, vec_t *v)
{
    write_float(f, v[0]);
    write_float(f, v[1]);
    write_float(f, v[2]);
}

static void write_index(FILE *f, void *p, size_t size, void *start, int max_index)
{
    size_t diff;

    if (!p) {
        write_int(f, -1);
        return;
    }

    if (p < start || (byte *)p > (byte *)start + max_index * size) {
        gi.Error("%s: pointer out of range: %p", __func__, p);
    }

    diff = (byte *)p - (byte *)start;
    if (diff % size) {
        gi.Error("%s: misaligned pointer: %p", __func__, p);
    }
    write_int(f, (int)(diff / size));
}

static void write_pointer(FILE *f, void *p, ptr_type_t type)
{
    //const save_ptr_t *ptr;
    //int i;

    if (!p) {
        write_int(f, -1);
        return;
    }

    return;
    //for (i = 0, ptr = save_ptrs; i < num_save_ptrs; i++, ptr++) {
    //    if (ptr->type == type && ptr->ptr == p) {
    //        write_int(f, i);
    //        return;
    //    }
    //}

    gi.Error("%s: unknown pointer: %p", __func__, p);
}

static void write_field(FILE *f, const save_field_t *field, void *base)
{
    void *p = (byte *)base + field->ofs;
    int i;

    switch (field->type) {
    case F_BYTE:
        write_data(p, field->size, f);
        break;
    case F_SHORT:
        for (i = 0; i < field->size; i++) {
            write_short(f, ((short *)p)[i]);
        }
        break;
    case F_INT:
        for (i = 0; i < field->size; i++) {
            write_int(f, ((int *)p)[i]);
        }
        break;
    case F_FLOAT:
        for (i = 0; i < field->size; i++) {
            write_float(f, ((float *)p)[i]);
        }
        break;
    case F_VECTOR:
        write_vector(f, (vec_t *)p);
        break;

    case F_ZSTRING:
        write_string(f, (char *)p);
        break;
    case F_LSTRING:
        write_string(f, *(char **)p);
        break;

    case F_EDICT:
        write_index(f, *(void **)p, sizeof(Entity), g_entities, MAX_EDICTS - 1);
        break;
    case F_CLIENT:
        write_index(f, *(void **)p, sizeof(ServersClient), game.clients, game.maximumClients - 1);
        break;
    case F_ITEM:
        write_index(f, *(void **)p, sizeof(gitem_t), itemlist, game.numberOfItems - 1);
        break;

    case F_POINTER:
        // TODO: We aren't using this anymore...
        break;
    //case F_POINTER:
    //    write_pointer(f, *(void **)p, (ptr_type_t)field->size); // CPP: Cast
    //    break;

    default:
        gi.Error("%s: unknown field type", __func__);
    }
}

static void write_fields(FILE *f, const save_field_t *fields, void *base)
{
    const save_field_t *field;

    for (field = fields; field->type; field++) {
        write_field(f, field, base);
    }
}

static void read_data(void *buf, size_t len, FILE *f)
{
    if (fread(buf, 1, len, f) != len) {
        gi.Error("%s: couldn't read %" PRIz " bytes", __func__, len); // CPP: String fix.
    }
}

static int read_short(FILE *f)
{
    short v;

    read_data(&v, sizeof(v), f);
    v = LittleShort(v);

    return v;
}

static int read_int(FILE *f)
{
    int v;

    read_data(&v, sizeof(v), f);
    v = LittleLong(v);

    return v;
}

static float read_float(FILE *f)
{
    float v;

    read_data(&v, sizeof(v), f);
    v = LittleFloat(v);

    return v;
}


static char *read_string(FILE *f)
{
    int len;
    char *s;

    len = read_int(f);
    if (len == -1) {
        return NULL;
    }

    if (len < 0 || len > 65536) {
        gi.Error("%s: bad length", __func__);
    }

    s = (char*)gi.TagMalloc(len + 1, TAG_LEVEL); // CPP: Casts
    read_data(s, len, f);
    s[len] = 0;

    return s;
}

static void read_zstring(FILE *f, char *s, size_t size)
{
    int len;

    len = read_int(f);
    if (len < 0 || len >= size) {
        gi.Error("%s: bad length", __func__);
    }

    read_data(s, len, f);
    s[len] = 0;
}

static void read_vector(FILE *f, vec_t *v)
{
    v[0] = read_float(f);
    v[1] = read_float(f);
    v[2] = read_float(f);
}

static void *read_index(FILE *f, size_t size, void *start, int max_index)
{
    int index;
    byte *p;

    index = read_int(f);
    if (index == -1) {
        return NULL;
    }

    if (index < 0 || index > max_index) {
        gi.Error("%s: bad index", __func__);
    }

    p = (byte *)start + index * size;
    return p;
}

static void *read_pointer(FILE *f, ptr_type_t type)
{
    int index;
    const save_ptr_t *ptr;

    index = read_int(f);
    if (index == -1) {
        return NULL;
    }

    //if (index < 0 || index >= num_save_ptrs) {
    //    gi.Error("%s: bad index", __func__);
    //}

    //ptr = &save_ptrs[index];
    //if (ptr->type != type) {
    //    gi.Error("%s: type mismatch", __func__);
    //}

    return ptr->ptr;
}

static void read_field(FILE *f, const save_field_t *field, void *base)
{
    void *p = (byte *)base + field->ofs;
    int i;

    switch (field->type) {
    case F_BYTE:
        read_data(p, field->size, f);
        break;
    case F_SHORT:
        for (i = 0; i < field->size; i++) {
            ((short *)p)[i] = read_short(f);
        }
        break;
    case F_INT:
        for (i = 0; i < field->size; i++) {
            ((int *)p)[i] = read_int(f);
        }
        break;
    case F_FLOAT:
        for (i = 0; i < field->size; i++) {
            ((float *)p)[i] = read_float(f);
        }
        break;
    case F_VECTOR:
        read_vector(f, (vec_t *)p);
        break;

    case F_LSTRING:
        *(char **)p = read_string(f);
        break;
    case F_ZSTRING:
        read_zstring(f, (char *)p, field->size);
        break;

    case F_EDICT:
        *(Entity **)p = (Entity*)read_index(f, sizeof(Entity), g_entities, game.maxEntities - 1); // CPP: Cast
        break;
    case F_CLIENT:
        *(ServersClient **)p = (ServersClient*)read_index(f, sizeof(ServersClient), game.clients, game.maximumClients - 1); // CPP: Cast
        break;
    case F_ITEM:
        *(gitem_t **)p = (gitem_t*)read_index(f, sizeof(gitem_t), itemlist, game.numberOfItems - 1); // CPP: Cast
        break;

    // TODO: We aren't using this anymore...
    case F_POINTER:
        break;
    //case F_POINTER:
    //    *(void **)p = read_pointer(f, (ptr_type_t)field->size); // CPP: Cast
    //    break;

    default:
        gi.Error("%s: unknown field type", __func__);
    }
}

static void read_fields(FILE *f, const save_field_t *fields, void *base)
{
    const save_field_t *field;

    for (field = fields; field->type; field++) {
        read_field(f, field, base);
    }
}

//=========================================================

#define SAVE_MAGIC1     (('1'<<24)|('V'<<16)|('S'<<8)|'S')  // "SSV1"
#define SAVE_MAGIC2     (('1'<<24)|('V'<<16)|('A'<<8)|'S')  // "SAV1"
#define SAVE_VERSION    2

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void SVG_WriteGame(const char *filename, qboolean autosave)
{
    FILE    *f;
    int     i;

    if (!autosave)
        SVG_SaveClientData();

    f = fopen(filename, "wb");
    if (!f)
        gi.Error("Couldn't open %s", filename);

    write_int(f, SAVE_MAGIC1);
    write_int(f, SAVE_VERSION);

    game.autoSaved = autosave;
    write_fields(f, gamefields, &game);
    game.autoSaved = false;

    for (i = 0; i < game.maximumClients; i++) {
        write_fields(f, clientfields, &game.clients[i]);
    }

    fclose(f);
}

void SVG_ReadGame(const char *filename)
{
    FILE    *f;
    int     i;

    gi.FreeTags(TAG_GAME);

    f = fopen(filename, "rb");
    if (!f)
        gi.Error("Couldn't open %s", filename);

    i = read_int(f);
    if (i != SAVE_MAGIC1) {
        fclose(f);
        gi.Error("Not a save game");
    }

    i = read_int(f);
    if (i != SAVE_VERSION) {
        fclose(f);
        gi.Error("Savegame from an older version");
    }

    read_fields(f, gamefields, &game);

    // should agree with server's version
    if (game.maximumClients != (int)maximumClients->value) {
        fclose(f);
        gi.Error("Savegame has bad maximumClients");
    }
    if (game.maxEntities <= game.maximumClients || game.maxEntities > MAX_EDICTS) {
        fclose(f);
        gi.Error("Savegame has bad maxEntities");
    }

    globals.entities = g_entities;
    globals.maxEntities = game.maxEntities;

    game.clients = (ServersClient*)gi.TagMalloc(game.maximumClients * sizeof(game.clients[0]), TAG_GAME); // CPP: Cast
    for (i = 0; i < game.maximumClients; i++) {
        read_fields(f, clientfields, &game.clients[i]);
    }

    fclose(f);
}

//==========================================================


/*
=================
WriteLevel

=================
*/
void SVG_WriteLevel(const char *filename)
{
    int     i;
    Entity *ent;
    FILE    *f;

    f = fopen(filename, "wb");
    if (!f)
        gi.Error("Couldn't open %s", filename);

    write_int(f, SAVE_MAGIC2);
    write_int(f, SAVE_VERSION);

    // write out LevelLocals
    write_fields(f, levelfields, &level);

    // write out all the entities
    for (i = 0; i < globals.numberOfEntities; i++) {
        ent = &g_entities[i];
        if (!ent->inUse)
            continue;
        write_int(f, i);
        write_fields(f, entityfields, ent);
    }
    write_int(f, -1);

    fclose(f);
}


/*
=================
ReadLevel

SpawnEntities will allready have been called on the
level the same way it was when the level was saved.

That is necessary to get the entityBaselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void SVG_ReadLevel(const char *filename)
{
    int     entnum;
    FILE    *f;
    int     i;
    Entity *ent;

    // Free any dynamic memory allocated by loading the level
    // base state
    gi.FreeTags(TAG_LEVEL);

    f = fopen(filename, "rb");
    if (!f)
        gi.Error("Couldn't open %s", filename);

    // Ensure all entities have a clean slate in memory.
    for (int32_t i = 0; i < game.maxEntities; i++) {
        g_entities[i] = {};
    }
    //memset(g_entities, 0, game.maxEntities * sizeof(g_entities[0]));

    // Set the number of edicts to be maximumClients + 1. (They are soon to be in-use after all)
    globals.numberOfEntities = maximumClients->value + 1;

    i = read_int(f);
    if (i != SAVE_MAGIC2) {
        fclose(f);
        gi.Error("Not a save game");
    }

    i = read_int(f);
    if (i != SAVE_VERSION) {
        fclose(f);
        gi.Error("Savegame from an older version");
    }

    // load the level locals
    read_fields(f, levelfields, &level);

    // load all the entities
    while (1) {
        entnum = read_int(f);
        if (entnum == -1)
            break;
        if (entnum < 0 || entnum >= game.maxEntities) {
            gi.Error("%s: bad entity number", __func__);
        }
        if (entnum >= globals.numberOfEntities)
            globals.numberOfEntities = entnum + 1;

        ent = &g_entities[entnum];
        read_fields(f, entityfields, ent);
        ent->inUse = true;
        ent->state.number = entnum;

        // let the server rebuild world links for this ent
        memset(&ent->area, 0, sizeof(ent->area));
        gi.LinkEntity(ent);
    }

    fclose(f);

    // mark all clients as unconnected
    for (i = 0 ; i < maximumClients->value ; i++) {
        ent = &g_entities[i + 1];
        ent->client = game.clients + i;
        ent->client->persistent.isConnected = false;
    }

    // do any load time things at this point
    for (i = 0 ; i < globals.numberOfEntities ; i++) {
        ent = &g_entities[i];

        if (!ent->inUse)
            continue;

        // fire any cross-level triggers
        //if (ent->className)
        //    if (strcmp(ent->className, "target_crosslevel_target") == 0)
        //        ent->nextThinkTime = level.time + ent->delay;

        //if (ent->Think == func_clock_think || ent->Use == func_clock_use) {
        //    const char *msg = ent->message;
        //    ent->message = (const char)gi.TagMalloc(CLOCK_MESSAGE_SIZE, TAG_LEVEL); // CPP: Cast
        //    if (msg) {
        //        Q_strlcpy((char*)ent->message, msg, CLOCK_MESSAGE_SIZE); // C++20: STRING: Dangerous (const char*) to (char*)..?
        //        gi.TagFree((void*)msg);
        //    }
        //}
    }
}

