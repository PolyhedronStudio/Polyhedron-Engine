/***
*
*	License here.
*
*	@file
*
*	Contains the 'KeyBinding' declarations. The KeyBinding objects are
*   used to determine whether a key was pressed, released, held down,
*   and keeps score for how many ms.
*
*   KeyBindings are bound to a console command. The console commands are
*   registered to callbacks, which in return depending on the state of 
*   the command (command, +command, -command) call upon the KeyUp and the
*   KeyDown functions of the KeyBinding.
***/
#pragma once


//---------------------------------------------------------------------
// Client Game Movement IMPLEMENTATION.
//---------------------------------------------------------------------
class KeyBinding {
public:
	//! Constructor/Destructor.
    KeyBinding();

    /**
    *   @brief  Process key up state.
    **/
    void ProcessKeyUp();

    /**
    *   @brief  Process key down state.
    **/
    void ProcessKeyDown();

    /**
    *   @brief  Clear key binding state.
    **/
    void ClearKeyState();

    /**
    *   @return Returns the fraction of the frame that the key was down.
    **/
    float GetStateFraction();

    /**
    *   @return The actual key state bits.
    **/
    inline uint8_t GetKeyStateBits() {
        return keyStateBits;
    }

    /**
    *   @brief  Sets the key state bits value.
    **/
    inline void SetKeyStateBits(uint8_t bits) {
        keyStateBits = bits;
    }

private:
    //! The keys being held for this binding. (2 is the limit.)
    uint32_t keys[2] = {};
    //! Timestamp of when the button got pressed.
    uint32_t eventTimeStamp = 0; 
    //! Down time for this frame (ms)
    uint32_t milliseconds = 0;
    //! The current key state bits of this binding.
    uint8_t keyStateBits = 0;
};

