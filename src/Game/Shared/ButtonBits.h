/***
*
*	License here.
*
*	@file
*
*	SharedGame ButtonBit Fkags. These flags are used for letting the server know
*	about our user input actions, and send to the server (~multiple-) on a per
*	frame basis.
*
*	Adding any custom player action starts here. Currently there is room
*	for 2 more commands. If more room is needed, please inquire on github by 
*	posting an issue. It's perfectly do-able but seems like unwanted stress for
*	the oh so limited bandwidth.
* 
***/
#pragma once


/**
*   @brief  These are used for game logic. They are set in clg_input.cpp. One is free
*           to use up the remaining slots for their own custom needs.
**/
struct ButtonBits {
    //! Button bit for when a player is primary firing.
    static constexpr uint8_t PrimaryFire    = (1 << 0);
    //! Button bit for when a player is secondary firing.
    static constexpr uint8_t SecondaryFire  = (1 << 1);
    //! Button bit for when a player is reloading its weapon.
    static constexpr uint8_t Reload         = (1 << 4);
    //! Button bit for when a player is using an entity.
    //! TODO: This is currently still unimplemented. Should replace "auto touch" buttons etc.
    static constexpr uint8_t Use            = (1 << 2);
    //! Button bit that is set when a player is moving.
    static constexpr uint8_t Walk           = (1 << 3);
    //! Unused 0.
    static constexpr uint8_t Unused0        = (1 << 5);
    //! Unused 1.
    static constexpr uint8_t Unused1        = (1 << 6);
    //! Set when any button is pressed.
    static constexpr uint8_t Any = (1 << 7);
};