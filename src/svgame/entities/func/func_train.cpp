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
damage     default 2
noise   looping sound to play when the train is in motion

*/
void train_next(Entity* self);

void train_blocked(Entity* self, Entity* other)
{
    if (!(other->serverFlags & EntityServerFlags::Monster) && (!other->client)) {
        // give it a chance to go away on it's own terms (like gibs)
        SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, 100000, 1, 0, MeansOfDeath::Crush);
        // if it's still there, nuke it
        if (other)
            BecomeExplosion1(other);
        return;
    }

    if (level.time < self->debounceTouchTime)
        return;

    if (!self->damage)
        return;
    self->debounceTouchTime = level.time + 0.5;
    SVG_Damage(other, self, self, vec3_origin, other->state.origin, vec3_origin, self->damage, 1, 0, MeansOfDeath::Crush);
}

void train_wait(Entity* self)
{
    if (self->targetEntityPtr->pathTarget) {
        char* savetarget;
        Entity* ent;

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
            self->nextThinkTime = level.time + self->moveInfo.wait;
            self->Think = train_next;
        }
        else if (self->spawnFlags & TRAIN_TOGGLE) { // && wait < 0
            train_next(self);
            self->spawnFlags &= ~TRAIN_START_ON;
            VectorClear(self->velocity);
            self->nextThinkTime = 0;
        }

        if (!(self->flags & EntityFlags::TeamSlave)) {
            if (self->moveInfo.endSoundIndex)
                gi.Sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveInfo.endSoundIndex, 1, ATTN_STATIC, 0);
            self->state.sound = 0;
        }
    }
    else {
        train_next(self);
    }

}

void train_next(Entity* self)
{
    Entity* ent;
    vec3_t      dest;
    qboolean    first;

    first = true;
again:
    if (!self->target) {
        //      gi.DPrintf ("train_next: no next target\n");
        return;
    }

    ent = SVG_PickTarget(self->target);
    if (!ent) {
        gi.DPrintf("train_next: bad target %s\n", self->target);
        return;
    }

    self->target = ent->target;

    // check for a teleport path_corner
    if (ent->spawnFlags & 1) {
        if (!first) {
            gi.DPrintf("connected teleport path_corners, see %s at %s\n", ent->className, vec3_to_str(ent->state.origin));
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
        if (self->moveInfo.startSoundIndex)
            gi.Sound(self, CHAN_NO_PHS_ADD + CHAN_VOICE, self->moveInfo.startSoundIndex, 1, ATTN_STATIC, 0);
        self->state.sound = self->moveInfo.middleSoundIndex;
    }

    VectorSubtract(ent->state.origin, self->mins, dest);
    self->moveInfo.state = STATE_TOP;
    VectorCopy(self->state.origin, self->moveInfo.startOrigin);
    VectorCopy(dest, self->moveInfo.endOrigin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnFlags |= TRAIN_START_ON;
}

void train_resume(Entity* self)
{
    Entity* ent;
    vec3_t  dest;

    ent = self->targetEntityPtr;

    VectorSubtract(ent->state.origin, self->mins, dest);
    self->moveInfo.state = STATE_TOP;
    VectorCopy(self->state.origin, self->moveInfo.startOrigin);
    VectorCopy(dest, self->moveInfo.endOrigin);
    Brush_Move_Calc(self, dest, train_wait);
    self->spawnFlags |= TRAIN_START_ON;
}

void func_train_find(Entity* self)
{
    Entity* ent;

    if (!self->target) {
        gi.DPrintf("train_find: no target\n");
        return;
    }
    ent = SVG_PickTarget(self->target);
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
        self->nextThinkTime = level.time + FRAMETIME;
        self->Think = train_next;
        self->activator = self;
    }
}

void train_use(Entity* self, Entity* other, Entity* activator)
{
    self->activator = activator;

    if (self->spawnFlags & TRAIN_START_ON) {
        if (!(self->spawnFlags & TRAIN_TOGGLE))
            return;
        self->spawnFlags &= ~TRAIN_START_ON;
        VectorClear(self->velocity);
        self->nextThinkTime = 0;
    }
    else {
        if (self->targetEntityPtr)
            train_resume(self);
        else
            train_next(self);
    }
}

void SP_func_train(Entity* self)
{
    self->moveType = MoveType::Push;

    VectorClear(self->state.angles);
    self->Blocked = train_blocked;
    if (self->spawnFlags & TRAIN_BLOCK_STOPS)
        self->damage = 0;
    else {
        if (!self->damage)
            self->damage = 100;
    }
    self->solid = Solid::BSP;
    gi.SetModel(self, self->model);

    if (st.noise)
        self->moveInfo.middleSoundIndex = gi.SoundIndex(st.noise);

    if (!self->speed)
        self->speed = 100;

    self->moveInfo.speed = self->speed;
    self->moveInfo.acceleration = self->moveInfo.deceleration = self->moveInfo.speed;

    self->Use = train_use;

    gi.LinkEntity(self);

    if (self->target) {
        // start trains on the second frame, to make sure their targets have had
        // a chance to spawn
        self->nextThinkTime = level.time + FRAMETIME;
        self->Think = func_train_find;
    }
    else {
        gi.DPrintf("func_train without a target at %s\n", Vec3ToString(self->absMin));
    }
}