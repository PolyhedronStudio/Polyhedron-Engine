// LICENSE HERE.

//
// svgame/entities/func_train.c
//
//
// func_train entity implementation.
//

#include "../../g_local.h"      // Include SVGame funcs.
#include "../../utils.h"        // Include Util funcs.
#include "../../brushfuncs.h"   // Include Brush funcs.
#include "../../effects.h"

#include "func_door.h"          // Include func_door entity header.

//=====================================================
#define TRAIN_START_ON      1
#define TRAIN_TOGGLE        2
#define TRAIN_BLOCK_STOPS   4

/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed   default 100
dmg     default 2
noise   looping sound to play when the train is in motion

*/
void train_next(entity_t* self);

void train_blocked(entity_t* self, entity_t* other)
{
    if (!(other->serverFlags & EntityServerFlags::Monster) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, 100000, 1, 0, MOD_CRUSH);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    if (level.time < self->debounceTouchTime)
        return;

    if (!self->dmg)
        return;
    self->debounceTouchTime = level.time + 0.5;
    T_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void train_wait(entity_t* self)
{
    if (self->targetEntityPtr->pathTarget) {
        char* savetarget;
        entity_t* ent;

        ent = self->targetEntityPtr;
        savetarget = ent->target;
        ent->target = ent->pathTarget;
        UTIL_UseTargets(ent, self->activator);
        ent->target = savetarget;

        // make sure we didn't get killed by a killTarget
        if (!self->inUse)
            return;
    }

    if (self->moveInfo.wait) {
        if (self->moveInfo.wait > 0) {
            self->nextThink = level.time + self->moveInfo.wait;
            self->Think = train_next;
        }
        else if (self->spawnFlags & TRAIN_TOGGLE) { // && wait < 0
            train_next(self);
            self->spawnFlags &= ~TRAIN_START_ON;
            VectorClear(self->velocity);
            self->nextThink = 0;
        }

        if (!(self->flags & EntityFlags::TeamSlave)) {
            if (self->moveInfo.sound_end)
                gi.Sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveInfo.sound_end, 1, ATTN_STATIC, 0);
            self->state.sound = 0;
        }
    }
    else {
        train_next(self);
    }

}

void train_next(entity_t* self)
{
    entity_t* ent;
    vec3_t      dest;
    qboolean    first;

    first = true;
again:
    if (!self->target) {
        //      gi.DPrintf ("train_next: no next target\n");
        return;
    }

    ent = G_PickTarget(self->target);
    if (!ent) {
        gi.DPrintf("train_next: bad target %s\n", self->target);
        return;
    }

    self->target = ent->target;

    // check for a teleport path_corner
    if (ent->spawnFlags & 1) {
        if (!first) {
            gi.DPrintf("connected teleport path_corners, see %s at %s\n", ent->classname, vec3_to_str(ent->state.origin));
            return;
        }
        first = false;
        VectorSubtract(ent->state.origin, self->mins, self->state.origin);
        VectorCopy(self->state.origin, self->state.oldOrigin);
        self->state.event = EntityEvent::OtherTeleport;
        gi.LinkEntity(self);
        goto again;
    }

    self->moveInfo.wait = ent->wait;
    self->targetEntityPtr = ent;

    if (!(self->flags & EntityFlags::TeamSlave)) {
        if (self->moveInfo.sound_start)
            gi.Sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveInfo.sound_start, 1, ATTN_STATIC, 0);
        self->state.sound = self->moveInfo.sound_middle;
    }

    VectorSubtract(ent->state.origin, self->mins, dest);
    self->moveInfo.state = STATE_TOP;
    VectorCopy(self->state.origin, self->moveInfo.start_origin);
    VectorCopy(dest, self->moveInfo.end_origin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnFlags |= TRAIN_START_ON;
}

void train_resume(entity_t* self)
{
    entity_t* ent;
    vec3_t  dest;

    ent = self->targetEntityPtr;

    VectorSubtract(ent->state.origin, self->mins, dest);
    self->moveInfo.state = STATE_TOP;
    VectorCopy(self->state.origin, self->moveInfo.start_origin);
    VectorCopy(dest, self->moveInfo.end_origin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnFlags |= TRAIN_START_ON;
}

void func_train_find(entity_t* self)
{
    entity_t* ent;

    if (!self->target) {
        gi.DPrintf("train_find: no target\n");
        return;
    }
    ent = G_PickTarget(self->target);
    if (!ent) {
        gi.DPrintf("train_find: target %s not found\n", self->target);
        return;
    }
    self->target = ent->target;

    VectorSubtract(ent->state.origin, self->mins, self->state.origin);
    gi.LinkEntity(self);

    // if not triggered, start immediately
    if (!self->targetName)
        self->spawnFlags |= TRAIN_START_ON;

    if (self->spawnFlags & TRAIN_START_ON) {
        self->nextThink = level.time + FRAMETIME;
        self->Think = train_next;
        self->activator = self;
    }
}

void train_use(entity_t* self, entity_t* other, entity_t* activator)
{
    self->activator = activator;

    if (self->spawnFlags & TRAIN_START_ON) {
        if (!(self->spawnFlags & TRAIN_TOGGLE))
            return;
        self->spawnFlags &= ~TRAIN_START_ON;
        VectorClear(self->velocity);
        self->nextThink = 0;
    }
    else {
        if (self->targetEntityPtr)
            train_resume(self);
        else
            train_next(self);
    }
}

void SP_func_train(entity_t* self)
{
    self->moveType = MoveType::Push;

    VectorClear(self->state.angles);
    self->Blocked = train_blocked;
    if (self->spawnFlags & TRAIN_BLOCK_STOPS)
        self->dmg = 0;
    else {
        if (!self->dmg)
            self->dmg = 100;
    }
    self->solid = Solid::BSP;
    gi.SetModel(self, self->model);

    if (st.noise)
        self->moveInfo.sound_middle = gi.SoundIndex(st.noise);

    if (!self->speed)
        self->speed = 100;

    self->moveInfo.speed = self->speed;
    self->moveInfo.accel = self->moveInfo.decel = self->moveInfo.speed;

    self->Use = train_use;

    gi.LinkEntity(self);

    if (self->target) {
        // start trains on the second frame, to make sure their targets have had
        // a chance to spawn
        self->nextThink = level.time + FRAMETIME;
        self->Think = func_train_find;
    }
    else {
        gi.DPrintf("func_train without a target at %s\n", Vec3ToString(self->absMin));
    }
}