// Client Game.
#include "../ClientGameLocals.h"
#include "../Main.h"

// ChatHUD.
#include "KeyBinding.h"


/***
*
*   
* 
***/
//! Constructor.
KeyBinding::KeyBinding() {

}

/**
*   @brief  Process key up state.
**/
void KeyBinding::ProcessKeyUp() {
    // Store the precise time of this event.
    uint32_t eventTime = clgi.Com_GetEventTime();

    // Acquire command.
    const char *command = clgi.Cmd_Argv(1);
    
    // Determine whether command was typed manually at the console for continuous down.
    int32_t key = 0;

    if (command[0]) {
        key = std::atoi(command);
    } else {
        // Typed manually at console, assume for unsticking, so clear all.
        keys[0] = keys[1] = 0;
        keyStateBits = 0;
        return;
    }

    // Unset key(s).
    if (keys[0] == key) {
        keys[0] = 0;
    } else if (keys[1] == key) {
        keys[1] = 0;
    } else {
        // Kept up without corresponding down. (Menu pass through)
        return;
    }

    // Ensure no other keys are still holding it down.
    if (keys[0] || keys[1]) {
        return;
    }

    // This should not happen in sane scenarios.
    if (!(keyStateBits & ButtonState::Held)) {
        return;
    }

    // Acquire up time.
    command = clgi.Cmd_Argv(2);
    
    uint32_t upTime = std::atoi(command);
    if (!upTime) {
        // Ensure it is set for movement binds.
        milliseconds += 10; // NOTE: So far I haven't figured out, or met anyone, that knows why += 10? 
    } else if (upTime > eventTimeStamp) {
        milliseconds += upTime - eventTimeStamp;
    }

    // Now up. (Remove 'Held' state.)
    keyStateBits &= ~ButtonState::Held;
}

/**
*   @brief  Process key down state.
**/
void KeyBinding::ProcessKeyDown() {
    // Acquire command.
    const char *command = clgi.Cmd_Argv(1);
    
    // Determine whether command was typed manually at the console for continuous down.
    int32_t key = -1; // -1 = default, unless set in the if statement below.

    if (command[0]) {
        key = std::atoi(command);
    }

    // Check for repeating key(s).
    if (key == keys[0] || key == keys[1]) {
        return;
    }

    // Assign key to keys[0], or keys[1] depending on which is still free. Warn out otherwise.
    if (!keys[0]) {
        keys[0] = key;
    } else if (!keys[1]) {
        keys[1] = key;
    } else {
        Com_WPrint("Warning: Holding three keys down for a button is unsupported!\n");
        return;
    }

    // See whether its keyStateBits already was ButtonState::Held
    if (keyStateBits & ButtonState::Held) {
        return;
    }

    // Save timestamp.
    command = clgi.Cmd_Argv(2);
    eventTimeStamp = std::atoi(command);
    if (!eventTimeStamp) {
        // Prevent it from being too far off with determining (net/frame-)rates.
        eventTimeStamp = clgi.Com_GetEventTime() - 100 / CLG_FRAMEDIV;
    }

    // Addition keyStateBits.
    keyStateBits |= ButtonState::Held + ButtonState::Down;
}

/**
*   @brief  Returns the fraction of the frame that the key was down.
**/
void KeyBinding::ClearKeyState() {
    uint32_t eventTime = clgi.Com_GetEventTime();
    milliseconds = 0;
    keyStateBits &= ~ButtonState::Down; // Clear impulses.
    if (keyStateBits & ButtonState::Held) {
        eventTimeStamp = eventTime; // Still down.
    }
}

/**
*   @brief  Returns the fraction of the frame that the key was down.
**/
float KeyBinding::GetStateFraction() {
    float val;

    if (keyStateBits & ButtonState::Held) {
        // Acquire event time.
        uint32_t currentEventTime = clgi.Com_GetEventTime();

        // Still down
        if (currentEventTime > eventTimeStamp) {
            milliseconds += currentEventTime - eventTimeStamp;
        }
    }

    // special case for instant packet
    if (!cl->moveCommand.input.msec) {
        return (float)(keyStateBits & ButtonState::Held);
    }

    return Clampf((float)milliseconds / cl->moveCommand.input.msec, 0.f, 1.f);
}