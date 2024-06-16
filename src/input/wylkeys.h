/**
 ** wylkeys.h ---- Keyboard Translation Table
 **
 ** Copyright (C) 2024 Mariano Alvarez Fernandez
 ** [e-mail: malfer@telefonica.net]
 **
 ** This file is part of the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#ifndef _WYLKEYS_H_
#define _WYLKEYS_H_

typedef struct {
  unsigned short key;
  unsigned int keywyl;
} KeyEntry;

static KeyEntry _KTAlt[] = { // state == Alt
  { GrKey_Alt_0                , KEY_0 },
  { GrKey_Alt_1                , KEY_1 },
  { GrKey_Alt_2                , KEY_2 },
  { GrKey_Alt_3                , KEY_3 },
  { GrKey_Alt_4                , KEY_4 },
  { GrKey_Alt_5                , KEY_5 },
  { GrKey_Alt_6                , KEY_6 },
  { GrKey_Alt_7                , KEY_7 },
  { GrKey_Alt_8                , KEY_8 },
  { GrKey_Alt_9                , KEY_9 },
  { GrKey_Alt_A                , KEY_A },
  { GrKey_Alt_B                , KEY_B },
  { GrKey_Alt_C                , KEY_C },
  { GrKey_Alt_D                , KEY_D },
  { GrKey_Alt_E                , KEY_E },
  { GrKey_Alt_F                , KEY_F },
  { GrKey_Alt_G                , KEY_G },
  { GrKey_Alt_H                , KEY_H },
  { GrKey_Alt_I                , KEY_I },
  { GrKey_Alt_J                , KEY_J },
  { GrKey_Alt_K                , KEY_K },
  { GrKey_Alt_L                , KEY_L },
  { GrKey_Alt_M                , KEY_M },
  { GrKey_Alt_N                , KEY_N },
  { GrKey_Alt_O                , KEY_O },
  { GrKey_Alt_P                , KEY_P },
  { GrKey_Alt_Q                , KEY_Q },
  { GrKey_Alt_R                , KEY_R },
  { GrKey_Alt_S                , KEY_S },
  { GrKey_Alt_T                , KEY_T },
  { GrKey_Alt_U                , KEY_U },
  { GrKey_Alt_V                , KEY_V },
  { GrKey_Alt_W                , KEY_W },
  { GrKey_Alt_X                , KEY_X },
  { GrKey_Alt_Y                , KEY_Y },
  { GrKey_Alt_Z                , KEY_Z },
  { GrKey_Alt_Escape           , KEY_ESC },
  { GrKey_Alt_Return           , KEY_ENTER },
  { GrKey_Alt_Tab              , KEY_TAB },
  { GrKey_Alt_Backspace        , KEY_BACKSPACE },
  { GrKey_Alt_Delete           , KEY_DELETE },
  { GrKey_Alt_Down             , KEY_DOWN },
  { GrKey_Alt_End              , KEY_END },
  { GrKey_Alt_F1               , KEY_F1 },
  { GrKey_Alt_F2               , KEY_F2 },
  { GrKey_Alt_F3               , KEY_F3 },
  { GrKey_Alt_F4               , KEY_F4 },
  { GrKey_Alt_F5               , KEY_F5 },
  { GrKey_Alt_F6               , KEY_F6 },
  { GrKey_Alt_F7               , KEY_F7 },
  { GrKey_Alt_F8               , KEY_F8 },
  { GrKey_Alt_F9               , KEY_F9 },
  { GrKey_Alt_F10              , KEY_F10 },
  { GrKey_Alt_F11              , KEY_F11 },
  { GrKey_Alt_F12              , KEY_F12 },
  { GrKey_Alt_Home             , KEY_HOME },
  { GrKey_Alt_Insert           , KEY_INSERT },
  { GrKey_Alt_Left             , KEY_LEFT },
  { GrKey_Alt_PageDown         , KEY_PAGEDOWN },
  { GrKey_Alt_PageUp           , KEY_PAGEUP },
  { GrKey_Alt_Right            , KEY_RIGHT },
  { GrKey_Alt_Up               , KEY_UP },
};


static KeyEntry _KTShift[] = { // state == Shift
  { GrKey_BackTab              , KEY_TAB },
  { GrKey_Shift_Down           , KEY_DOWN },
  { GrKey_Shift_End            , KEY_END },
  { GrKey_Shift_F1             , KEY_F1 },
  { GrKey_Shift_F2             , KEY_F2 },
  { GrKey_Shift_F3             , KEY_F3 },
  { GrKey_Shift_F4             , KEY_F4 },
  { GrKey_Shift_F5             , KEY_F5 },
  { GrKey_Shift_F6             , KEY_F6 },
  { GrKey_Shift_F7             , KEY_F7 },
  { GrKey_Shift_F8             , KEY_F8 },
  { GrKey_Shift_F9             , KEY_F9 },
  { GrKey_Shift_F10            , KEY_F10 },
  { GrKey_Shift_F11            , KEY_F11 },
  { GrKey_Shift_F12            , KEY_F12 },
  { GrKey_Shift_Home           , KEY_HOME },
  { GrKey_Shift_Insert         , KEY_INSERT },
  { GrKey_Shift_Left           , KEY_LEFT },
  { GrKey_Shift_PageDown       , KEY_PAGEDOWN },
  { GrKey_Shift_PageUp         , KEY_PAGEUP },
  { GrKey_Shift_Right          , KEY_RIGHT },
  { GrKey_Shift_Up             , KEY_UP },
};


static KeyEntry _KTControl[] = { // state == Control
  { GrKey_Control_Delete       , KEY_DELETE },
  { GrKey_Control_Down         , KEY_DOWN },
  { GrKey_Control_End          , KEY_END },
  { GrKey_Control_F1           , KEY_F1 },
  { GrKey_Control_F2           , KEY_F2 },
  { GrKey_Control_F3           , KEY_F3 },
  { GrKey_Control_F4           , KEY_F4 },
  { GrKey_Control_F5           , KEY_F5 },
  { GrKey_Control_F6           , KEY_F6 },
  { GrKey_Control_F7           , KEY_F7 },
  { GrKey_Control_F8           , KEY_F8 },
  { GrKey_Control_F9           , KEY_F9 },
  { GrKey_Control_F10          , KEY_F10 },
  { GrKey_Control_F11          , KEY_F11 },
  { GrKey_Control_F12          , KEY_F12 },
  { GrKey_Control_Home         , KEY_HOME },
  { GrKey_Control_Insert       , KEY_INSERT },
  { GrKey_Control_Left         , KEY_LEFT },
  { GrKey_Control_PageDown     , KEY_PAGEDOWN },
  { GrKey_Control_PageUp       , KEY_PAGEUP },
  { GrKey_Control_Right        , KEY_RIGHT },
  { GrKey_Control_Up           , KEY_UP },
};

static KeyEntry _KTVoid[] = { // state == None
  { GrKey_Delete               , KEY_DELETE },
  { GrKey_Down                 , KEY_DOWN },
  { GrKey_End                  , KEY_END },
  { GrKey_F1                   , KEY_F1 },
  { GrKey_F2                   , KEY_F2 },
  { GrKey_F3                   , KEY_F3 },
  { GrKey_F4                   , KEY_F4 },
  { GrKey_F5                   , KEY_F5 },
  { GrKey_F6                   , KEY_F6 },
  { GrKey_F7                   , KEY_F7 },
  { GrKey_F8                   , KEY_F8 },
  { GrKey_F9                   , KEY_F9 },
  { GrKey_F10                  , KEY_F10 },
  { GrKey_F11                  , KEY_F11 },
  { GrKey_F12                  , KEY_F12 },
  { GrKey_Home                 , KEY_HOME },
  { GrKey_Insert               , KEY_INSERT },
  { GrKey_Left                 , KEY_LEFT },
  { GrKey_PageDown             , KEY_PAGEDOWN },
  { GrKey_PageUp               , KEY_PAGEUP },
  { GrKey_Right                , KEY_RIGHT },
  { GrKey_Up                   , KEY_UP },
};

typedef struct {
  unsigned int oldk;
  unsigned int newk;
} RemapEntry;

static RemapEntry _KPRemap[] = { // Remap keypad keys to normal
  { KEY_KPDOT                  , KEY_DELETE },
  { KEY_KP2                    , KEY_DOWN },
  { KEY_KP1                    , KEY_END },
  { KEY_KP7                    , KEY_HOME },
  { KEY_KP0                    , KEY_INSERT },
  { KEY_KP4                    , KEY_LEFT },
  { KEY_KP3                    , KEY_PAGEDOWN },
  { KEY_KP9                    , KEY_PAGEUP },
  { KEY_KP6                    , KEY_RIGHT },
  { KEY_KP8                    , KEY_UP },
};

#endif
