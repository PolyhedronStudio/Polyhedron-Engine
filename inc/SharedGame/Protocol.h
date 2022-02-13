/*
// LICENSE HERE.

//
// SharedGame/protocol.h
//
// The server game module gets a chance to define its own commands to send to
// the client here.
//
*/
#ifndef __SHAREDGAME_PROTOCOL_H__
#define __SHAREDGAME_PROTOCOL_H__

/**
*   @brief  Server Game Command are a way for the server to tell a client what to do.
*           Spawn a muzzleflash effect, or a temp entity effect, or update a client's
*           layout string.
*
*           Due to protocol limitations at the time of writing, the index starts at 22
*           and the limit is 32 extra custom types.
**/
struct ServerGameCommands {
    //! First index is 22, all other slots are reserved for the server itself.
    static constexpr int32_t MuzzleFlash = 22;
    static constexpr int32_t MuzzleFlash2 = 23;
    static constexpr int32_t TempEntity = 24;
    static constexpr int32_t Layout = 25;
    static constexpr int32_t Inventory = 26;

    //! Be sure to increase limit in case you modify this array.
    static constexpr int32_t TotalNumberOfCommands = 27;
};

/**
*   @brief  Client Game Commands are a way for the client to tell the server what to do.
*           Currently it is not in utilized but can be used if needed.
*
*           Due to protocol limitations at the time of writing, the index starts at 13
*           and the limit is 32 extra custom types.
**/

struct ClientGameCommands {
    //! First index is 13, all other slots are reserved for the client itself. Feel free to rename this one and make it your own.
    static constexpr int32_t FirstCommand = 13;

    //! Be sure to increase limit in case you modify this array.
    static constexpr int32_t TotalNumberOfCommands = 14;
};

#endif