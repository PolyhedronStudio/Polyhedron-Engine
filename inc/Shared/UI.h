#ifndef __SHARED_UI_H__
#define __SHARED_UI_H__

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

#define UI_LEFT             0x00000001
#define UI_RIGHT            0x00000002
#define UI_CENTER           (UI_LEFT | UI_RIGHT)
#define UI_BOTTOM           0x00000004
#define UI_TOP              0x00000008
#define UI_MIDDLE           (UI_BOTTOM | UI_TOP)
#define UI_DROPSHADOW       0x00000010
#define UI_ALTCOLOR         0x00000020
#define UI_IGNORECOLOR      0x00000040
#define UI_XORCOLOR         0x00000080
#define UI_AUTOWRAP         0x00000100
#define UI_MULTILINE        0x00000200
#define UI_DRAWCURSOR       0x00000400

#endif // __SHARED_UI_H__