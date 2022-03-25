/***
*
*	License here.
*
*	@file
*
*	Client Game Light Styles.
* 
***/
#pragma once

class LightStyles {
public:
    /**
    *
    *    Light Styles Management.
    * 
    **/
    static void Clear();
    static void RunFrame();
    static void Set(int32_t index, const char* style);

    /**
    *   @brief  Adds the light styles to the scene view.
    **/
    static void AddLightStylesToView();

private:
    struct StyleEntry {
        list_t  entry = {};
        int     length = 0;
        vec4_t  value = vec4_zero();
        float   map[MAX_QPATH] = {};
    };

    static StyleEntry styles[MAX_LIGHTSTYLES];
    static list_t lightList;
    static int32_t lastOfs;
};
