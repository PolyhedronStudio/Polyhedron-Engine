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
    void Draw(const vec2_t &position);

    /**
    *   @brief  Adds a new line of text to the queue.
    **/
    void AddText(const std::string &str);

private:
    //! Configures the maximum amount of characters allowed to display per line.
    static constexpr uint32_t MaxCharsPerLine = 150;
    static constexpr uint32_t MaxLines = 32;
    
    //! Private struct used for the chatline queue.
    struct ChatLine {
        std::string text = "";
        uint32_t timeStamp = 0;
    };

    //! Chatline Queue.
    std::queue<ChatLine> chatLineQueue;

    //! Pointer to client game screen object.
    ClientGameScreen *screen;
};



//#define MAX_CHAT_TEXT       150
//#define MAX_CHAT_LINES      32
//#define CHAT_LINE_MASK      (MAX_CHAT_LINES - 1)
//
//typedef struct {
//    char        text[MAX_CHAT_TEXT];
//    unsigned    time;
//} chatline_t;
//
//void SCR_ClearChatHUD_f(void)
//{
//    memset(scr_chatlines, 0, sizeof(scr_chatlines));
//    scr_chathead = 0;
//}
//
//void SCR_AddToChatHUD(const char* text)
//{
//    chatline_t* line;
//    char* p;
//
//    line = &scr_chatlines[scr_chathead++ & CHAT_LINE_MASK];
//    Q_strlcpy(line->text, text, sizeof(line->text));
//    line->time = clgi.GetRealTime();
//
//    p = strrchr(line->text, '\n');
//    if (p)
//        *p = 0;
//}
//
//void SCR_DrawChatHUD(void)
//{
//    //int x, y, flags, step;
//    //unsigned i, lines, time;
//    //float alpha;
//    //chatline_t* line;
//
//    //if (scr_chathud->integer == 0)
//    //    return;
//
//    //x = scr_chathud_x->integer;
//    //y = scr_chathud_y->integer;
//
//    //if (scr_chathud->integer == 2)
//    //    flags = UI_ALTCOLOR;
//    //else
//    //    flags = 0;
//
//    //if (x < 0) {
//    //    x += scr.hud_width + 1;
//    //    flags |= UI_RIGHT;
//    //}
//    //else {
//    //    flags |= UI_LEFT;
//    //}
//
//    //if (y < 0) {
//    //    y += scr.hud_height - CHAR_HEIGHT + 1;
//    //    step = -CHAR_HEIGHT;
//    //}
//    //else {
//    //    step = CHAR_HEIGHT;
//    //}
//
//    //lines = scr_chathud_lines->integer;
//    //if (lines > scr_chathead)
//    //    lines = scr_chathead;
//
//    //time = scr_chathud_time->value * 1000;
//
//    //for (i = 0; i < lines; i++) {
//    //    line = &scr_chatlines[(scr_chathead - i - 1) & CHAT_LINE_MASK];
//
//    //    if (time) {
//    //        alpha = SCR_FadeAlpha(line->time, time, 1000);
//    //        if (!alpha)
//    //            break;
//
//    //        clgi.R_SetAlpha(alpha * scr_alpha->value);
//    //        SCR_DrawString(x, y, flags, line->text);
//    //        clgi.R_SetAlpha(scr_alpha->value);
//    //    }
//    //    else {
//    //        SCR_DrawString(x, y, flags, line->text);
//    //    }
//
//    //    y += step;
//    //}
//}