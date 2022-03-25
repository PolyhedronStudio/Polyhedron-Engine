// Something with license here, fucking tired of adding that, we all know its GPL shit.
#pragma once

class ClientGameScreen;

/**
*   @brief  Small utility class for the HUD's stat display functionalities.
*           It supports displaying a single number, as well as two with a '/'
*           acting as a divisor. (Useful for primary ammo display for example.)
*
*           Although it is likely very clear, here is an example:
*           numberA = Clip Ammo
*           numberB = Total count of primary ammo type.
*           Would display as:
*           36 / 144
* 
*           Another option is being able to set a unique color for both numbers,
*           and the divisor 'slash'.
* 
*           NOTE: It renders from right to left, ie, if the X axis is 1280, it'll
*           render from 1280 - summedSizeOfDigits. (Thus a number below 1280)
**/
class NumberHUD {
public:
    //! Constructor/Destructor.
    NumberHUD(ClientGameScreen *clgScreen);
    ~NumberHUD() = default;

    /**
    *   @brief  Draws the ChatHUD at given position.
    **/
    int32_t Draw(const vec2_t &position);

    /**
    *   @brief  Sets the primary number that is always displayed.
    **/
    void SetPrimaryNumber(int32_t value);

    /**
    *   @brief  Sets the secondary number that is optionally displayed.
    **/
    void SetSecondaryNumber(int32_t value);

    /**
    *   @brief  Sets a color for the designated number, or their divisor.
    **/
    void SetColor(const color_t &color, uint32_t element);

    /**
    *   @brief  Sets the display scale for the numbers, subjective to HUD Scale.
    **/
    inline void SetScale(float scale) { this->scale = scale; }
    /**
    *   @brief  Returns the scale set for this HUD element.
    **/
    inline float GetScale() { return scale; }

private:
    /**
    *   @brief  Draws the numerical digits of 'value'.

    **/
    int32_t DrawStringDigits(const vec2_t &position, int32_t value);

    /**
    *   @brief  Draws the number divisor '/'.
    *   @return Number of total pixels covered from right to left after applying scale.
    **/
    int32_t DrawNumberDivisor(const vec2_t &position);

public:
    //! Default color. ('Polyhedron' Green)
    static constexpr color_t DefaultColor = { .u32 = MakeColor(23, 233, 180, 255) };
    //! Red color. ('Polyhedron' Red)
    static constexpr color_t RedColor = { .u32 = MakeColor(233, 23, 62, 255) };

    //! Used for referring to a specific element in set/get functions.
    static constexpr uint32_t PrimaryNumber     = 0; //! Refers to primary(main) display number.
    static constexpr uint32_t SecondaryNumber   = 1; //! Refers to the optional secondary number.
    static constexpr uint32_t Divisor           = 2; //! Refers to the divisor.
    static constexpr uint32_t TotalElements     = 3; //! Used to size element config arrays.

private:
    //! Stores the colors for said elements, defaults to typical Polyhedron Green.
    color_t displayColors[TotalElements] = { DefaultColor, DefaultColor, DefaultColor };

    //! Numerical values are stored as a string.
    int32_t numericals[TotalElements - 1] = {};

    //! Actual value to scale elements width.
    float scale = 0.5f; //! 0.5f by default, hud scale takes care of larger resolutions.

    //! Pointer to client game screen object.
    ClientGameScreen *screen;
};
