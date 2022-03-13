#ifndef __SHARED_UI_H__
#define __SHARED_UI_H__

static constexpr uint32_t CHAR_WIDTH = 8;
static constexpr uint32_t CHAR_HEIGHT = 8;

static constexpr uint32_t UI_LEFT			= 0x00000001;
static constexpr uint32_t UI_RIGHT			= 0x00000002;
static constexpr uint32_t UI_CENTER			= (UI_LEFT | UI_RIGHT);
static constexpr uint32_t UI_BOTTOM			= 0x00000004;
static constexpr uint32_t UI_TOP			= 0x00000008;
static constexpr uint32_t UI_MIDDLE			= (UI_BOTTOM | UI_TOP);
static constexpr uint32_t UI_DROPSHADOW		= 0x00000010;
static constexpr uint32_t UI_ALTCOLOR		= 0x00000020;
static constexpr uint32_t UI_IGNORECOLOR	= 0x00000040;
static constexpr uint32_t UI_XORCOLOR		= 0x00000080;
static constexpr uint32_t UI_AUTOWRAP		= 0x00000100;
static constexpr uint32_t UI_MULTILINE		= 0x00000200;
static constexpr uint32_t UI_DRAWCURSOR		= 0x00000400;

#endif // __SHARED_UI_H__