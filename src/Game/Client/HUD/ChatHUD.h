// Something with license here, fucking tired of adding that, we all know its GPL shit.
#pragma once

class ClientGameScreen;

/**
*   @brief  Small utility class for the HUD's chat display functionality.
**/
class ChatHUD {
public:
    //! Constructor/Destructor.
    ChatHUD(ClientGameScreen *clgScreen);
    ~ChatHUD() = default;

    /**
    *   @brief  Clears the ChatHUD.
    **/
    void Clear();

    /**
    *   @brief  Draws the ChatHUD at given position.
    **/
    void Draw();

    /**
    *   @brief  Adds a new line of text to the queue.
    **/
    void AddText(const std::string &str);

private:
    //! Configures the maximum amount of characters allowed to display per line.
    static constexpr uint32_t MaxCharsPerLine = 150;
    static constexpr uint32_t MaxLines = 32;
    static constexpr uint32_t ChatLineMask = MaxLines - 1;
    
    //! Private struct used for the chatline queue.
    struct ChatLine {
        std::string text = "";
        uint32_t timeStamp = 0;
    } chatLines[MaxLines];

    //! Current chat headline.
    uint32_t chatHeadline = 0;

    //! Chatline Queue.
    //ChatLine chatLines[MaxLines];

    //! Pointer to client game screen object.
    ClientGameScreen *screen;
};
