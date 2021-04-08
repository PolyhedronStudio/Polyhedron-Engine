// LICENSE HERE.

//
// svgame/entities/func_clock.c
//
//
// func_clock entity implementation.
//

// Include local game header.
#include "../../g_local.h"

//=====================================================
/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathTarget"
If START_OFF, this entity must be used before it starts

"style"     0 "xx"
            1 "xx:xx"
            2 "xx:xx:xx"
*/

static void func_clock_reset(entity_t* self)
{
    //self->activator = NULL;
    //if (self->spawnFlags & 1) {
    //    self->health = 0;
    //    self->wait = self->count;
    //}
    //else if (self->spawnFlags & 2) {
    //    self->health = self->count;
    //    self->wait = 0;
    //}
}

static void func_clock_format_countdown(entity_t* self)
{
    //if (self->style == 0) {
    //    Q_snprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
    //    return;
    //}

    //if (self->style == 1) {
    //    Q_snprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
    //    if (self->message[3] == ' ')
    //        self->message[3] = '0';
    //    return;
    //}

    //if (self->style == 2) {
    //    Q_snprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
    //    if (self->message[3] == ' ')
    //        self->message[3] = '0';
    //    if (self->message[6] == ' ')
    //        self->message[6] = '0';
    //    return;
    //}
}

void func_clock_think(entity_t* self)
{
    //if (!self->enemy) {
    //    self->enemy = G_Find(NULL, FOFS(targetName), self->target);
    //    if (!self->enemy)
    //        return;
    //}

    //if (self->spawnFlags & 1) {
    //    func_clock_format_countdown(self);
    //    self->health++;
    //}
    //else if (self->spawnFlags & 2) {
    //    func_clock_format_countdown(self);
    //    self->health--;
    //}
    //else {
    //    struct tm* ltime;
    //    time_t      gmtime;

    //    gmtime = time(NULL);
    //    ltime = localtime(&gmtime);
    //    if (ltime)
    //        Q_snprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
    //    else
    //        strcpy(self->message, "00:00:00");
    //    if (self->message[3] == ' ')
    //        self->message[3] = '0';
    //    if (self->message[6] == ' ')
    //        self->message[6] = '0';
    //}

    //self->enemy->message = self->message;
    //self->enemy->Use(self->enemy, self, self);

    //if (((self->spawnFlags & 1) && (self->health > self->wait)) ||
    //    ((self->spawnFlags & 2) && (self->health < self->wait))) {
    //    if (self->pathTarget) {
    //        char* savetarget;
    //        char* savemessage;

    //        savetarget = self->target;
    //        savemessage = self->message;
    //        self->target = self->pathTarget;
    //        self->message = NULL;
    //        UTIL_UseTargets(self, self->activator);
    //        self->target = savetarget;
    //        self->message = savemessage;
    //    }

    //    if (!(self->spawnFlags & 8))
    //        return;

    //    func_clock_reset(self);

    //    if (self->spawnFlags & 4)
    //        return;
    //}

    //self->nextThink = level.time + 1;
}

void func_clock_use(entity_t* self, entity_t* other, entity_t* activator)
{
    //if (!(self->spawnFlags & 8))
    //    self->Use = NULL;
    //if (self->activator)
    //    return;
    //self->activator = activator;
    //self->Think(self);
}

void SP_func_clock(entity_t* self)
{
    //if (!self->target) {
    //    gi.DPrintf("%s with no target at %s\n", self->classname, Vec3ToString(self->s.origin));
    //    G_FreeEntity(self);
    //    return;
    //}

    //if ((self->spawnFlags & 2) && (!self->count)) {
    //    gi.DPrintf("%s with no count at %s\n", self->classname, Vec3ToString(self->s.origin));
    //    G_FreeEntity(self);
    //    return;
    //}

    //if ((self->spawnFlags & 1) && (!self->count))
    //    self->count = 60 * 60;;

    //func_clock_reset(self);

    //self->message = (char*)gi.TagMalloc(CLOCK_MESSAGE_SIZE, TAG_LEVEL); // CPP: Cast

    //self->Think = func_clock_think;

    //if (self->spawnFlags & 4)
    //    self->Use = func_clock_use;
    //else
    //    self->nextThink = level.time + 1;
}