/***
*
*	License here.
*
*	@file
*
*	Light Style Management.
* 
***/
#include "../ClientGameLocals.h"

#include "../TemporaryEntities.h"

#include "../Exports/View.h"

#include "LightStyles.h"

#define USE_LIGHTSTYLES 1
#if USE_LIGHTSTYLES

LightStyles::StyleEntry LightStyles::styles[MAX_LIGHTSTYLES];
LIST_DECL(LightStyles::lightList);
int32_t LightStyles::lastOfs;

void LightStyles::Clear(void)
{
    int     i;
    StyleEntry* ls;

    for (i = 0, ls = styles; i < MAX_LIGHTSTYLES; i++, ls++) {
        List_Init(&ls->entry);
        ls->length = 0;
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = 1;
    }

    List_Init(&lightList);
    lastOfs = -1;
}

/*
================
CLG_RunLightStyles
================
*/
void LightStyles::RunFrame(void)
{
    int32_t ofs = cl->time / 50;
    if (ofs == lastOfs) {
        return;
    }
    lastOfs = ofs;

    StyleEntry *ls;
    LIST_FOR_EACH(StyleEntry, ls, &lightList, entry) {
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = ls->map[ofs % ls->length];
    }
}

void LightStyles::Set(int32_t index, const char* style)
{
    int     i;
    StyleEntry* ls;

    ls = &styles[index];
    ls->length = strlen(style);
    if (ls->length > MAX_QPATH) {
        Com_Error(ErrorType::Drop, "%s: oversize style", __func__);
    }

    for (i = 0; i < ls->length; i++) {
        ls->map[i] = (1.0f / 26.f) * (float)((float)style[i] - (float)'a'); //(float)(s[i] - 'a') / (float)('m' - 'a'); //0.2f;  //(1.0f / 26.f)* (float)((float)s[i] - (float)'a');//;(float)(s[i] - 'a') / (float)('z' - 'a');
    }

    if (ls->entry.prev) {
        List_Delete(&ls->entry);
    }

    if (ls->length > 1) {
        List_Append(&lightList, &ls->entry);
        return;
    }

    if (ls->length == 1) {
        ls->value[0] =
            ls->value[1] =
            ls->value[2] =
            ls->value[3] = ls->map[0];
        return;
    }

    ls->value[0] =
        ls->value[1] =
        ls->value[2] =
        ls->value[3] = 1;
}

/**
*   @brief  Adds the light styles to the scene view.
**/
void LightStyles::AddLightStylesToView() {
    int32_t     i = 0;
    StyleEntry* ls = nullptr;

    for (i = 0, ls = styles; i < MAX_LIGHTSTYLES; i++, ls++) {
        clge->view->AddLightStyle(i, ls->value);
    }
}

#endif