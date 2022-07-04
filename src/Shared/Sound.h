#pragma once

/**
*   @details    Sound channels
*               Channel 0 never willingly overrides
*               Other channels (1-7) allways override a playing sound on that channel
**/
struct SoundChannel {
    static constexpr int32_t Auto   = 0;
    static constexpr int32_t Weapon = 1;
    static constexpr int32_t Voice  = 2;
    static constexpr int32_t Item   = 3;
    static constexpr int32_t Body   = 4;

    // Sound Channel Modifier Flags.
    //! Send to all clients, not just ones in PHS (ATTN 0 will also do this).
    static constexpr int32_t IgnorePHS  = 8;
    //! Send by reliable message, not datagram.
    static constexpr int32_t Reliable   = 16;
};

/**
*   @brief  Sound attenuation values
**/
struct Attenuation {
    //! Full volume the entire level.
    static constexpr int32_t None   = 0;
    static constexpr int32_t Normal = 1;
    static constexpr int32_t Idle   = 2;
    //! Diminish very rapidly with distance.
    static constexpr int32_t Static = 3;
};