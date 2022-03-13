// License here.
// 
//
// ClientGameServerMessage implementation.
#pragma once

#include "Shared/Interfaces/IClientGameExports.h"

//---------------------------------------------------------------------
// Client Game Server Message IMPLEMENTATION.
//---------------------------------------------------------------------
class ClientGameServerMessage : public IClientGameExportServerMessage {
public:
    virtual ~ClientGameServerMessage() = default;

    /**
    *   @brief  Breaks up playerskin into name(optional), modeland skin components.
    *           If model or skin are found to be invalid, replaces them with sane defaults.
    **/
    qboolean ParsePlayerSkin(char* name, char* model, char* skin, const char* str) final;
    /**
    *   @brief  Breaks up playerskin into name(optional), modeland skin components.
    *           If model or skin are found to be invalid, replaces them with sane defaults.
    **/
    qboolean UpdateConfigString(int32_t index, const char* str) final;
    
    /**
    *   @brief  Called at the start of receiving a server message.
    **/
    void Start() final;
    /**
    *   @brief  Actually parses the server message, and handles it accordingly.
    *   @return True if the message was succesfully parsed. False in case the message was unkown, or corrupted, etc.
    **/
    qboolean ParseMessage(int32_t serverCommand) final;
    /**
    *   @brief  Handles the demo message during playback.
    *   @return True if the message was succesfully parsed. False in case the message was unkown, or corrupted, etc.
    **/
    qboolean SeekDemoMessage(int32_t demoCommand) final;
    /**
    *   @brief  Called when we're done receiving a server message.
    **/
    void End(int32_t realTime) final;



private:
    /***
    *
    *   Non Interface Parse Functions.
    *
    ***/
    /**
    *   @brief  Parses the client's inventory message.
    **/
    void ParseInventory(void);
    /**
    *   @brief  Parses the client's layout message.
    **/
    void ParseLayout(void);
    /**
    *   @brief  Parses a temp entities packet.
    **/
    void ParseTempEntitiesPacket(void);
    /**
    *   @brief  Parses a muzzleflash packet.
    **/
    void ParseMuzzleFlashPacket(int32_t mask);
    /**
    *   @brief  Parses a print message.
    **/
    void ParsePrint(void);
    /**
    *   @brief  Parses a centerprint message.
    **/
    void ParseCenterPrint(void);
};

