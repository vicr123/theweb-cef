#include "renderer.h"
#include <QDebug>

// From ui/events/keycodes/keyboard_codes_posix.h.
enum KeyboardCode {
  VKEY_BACK = 0x08,
  VKEY_TAB = 0x09,
  VKEY_BACKTAB = 0x0A,
  VKEY_CLEAR = 0x0C,
  VKEY_RETURN = 0x0D,
  VKEY_SHIFT = 0x10,
  VKEY_CONTROL = 0x11,
  VKEY_MENU = 0x12,
  VKEY_PAUSE = 0x13,
  VKEY_CAPITAL = 0x14,
  VKEY_KANA = 0x15,
  VKEY_HANGUL = 0x15,
  VKEY_JUNJA = 0x17,
  VKEY_FINAL = 0x18,
  VKEY_HANJA = 0x19,
  VKEY_KANJI = 0x19,
  VKEY_ESCAPE = 0x1B,
  VKEY_CONVERT = 0x1C,
  VKEY_NONCONVERT = 0x1D,
  VKEY_ACCEPT = 0x1E,
  VKEY_MODECHANGE = 0x1F,
  VKEY_SPACE = 0x20,
  VKEY_PRIOR = 0x21,
  VKEY_NEXT = 0x22,
  VKEY_END = 0x23,
  VKEY_HOME = 0x24,
  VKEY_LEFT = 0x25,
  VKEY_UP = 0x26,
  VKEY_RIGHT = 0x27,
  VKEY_DOWN = 0x28,
  VKEY_SELECT = 0x29,
  VKEY_PRINT = 0x2A,
  VKEY_EXECUTE = 0x2B,
  VKEY_SNAPSHOT = 0x2C,
  VKEY_INSERT = 0x2D,
  VKEY_DELETE = 0x2E,
  VKEY_HELP = 0x2F,
  VKEY_0 = 0x30,
  VKEY_1 = 0x31,
  VKEY_2 = 0x32,
  VKEY_3 = 0x33,
  VKEY_4 = 0x34,
  VKEY_5 = 0x35,
  VKEY_6 = 0x36,
  VKEY_7 = 0x37,
  VKEY_8 = 0x38,
  VKEY_9 = 0x39,
  VKEY_A = 0x41,
  VKEY_B = 0x42,
  VKEY_C = 0x43,
  VKEY_D = 0x44,
  VKEY_E = 0x45,
  VKEY_F = 0x46,
  VKEY_G = 0x47,
  VKEY_H = 0x48,
  VKEY_I = 0x49,
  VKEY_J = 0x4A,
  VKEY_K = 0x4B,
  VKEY_L = 0x4C,
  VKEY_M = 0x4D,
  VKEY_N = 0x4E,
  VKEY_O = 0x4F,
  VKEY_P = 0x50,
  VKEY_Q = 0x51,
  VKEY_R = 0x52,
  VKEY_S = 0x53,
  VKEY_T = 0x54,
  VKEY_U = 0x55,
  VKEY_V = 0x56,
  VKEY_W = 0x57,
  VKEY_X = 0x58,
  VKEY_Y = 0x59,
  VKEY_Z = 0x5A,
  VKEY_LWIN = 0x5B,
  VKEY_COMMAND = VKEY_LWIN,  // Provide the Mac name for convenience.
  VKEY_RWIN = 0x5C,
  VKEY_APPS = 0x5D,
  VKEY_SLEEP = 0x5F,
  VKEY_NUMPAD0 = 0x60,
  VKEY_NUMPAD1 = 0x61,
  VKEY_NUMPAD2 = 0x62,
  VKEY_NUMPAD3 = 0x63,
  VKEY_NUMPAD4 = 0x64,
  VKEY_NUMPAD5 = 0x65,
  VKEY_NUMPAD6 = 0x66,
  VKEY_NUMPAD7 = 0x67,
  VKEY_NUMPAD8 = 0x68,
  VKEY_NUMPAD9 = 0x69,
  VKEY_MULTIPLY = 0x6A,
  VKEY_ADD = 0x6B,
  VKEY_SEPARATOR = 0x6C,
  VKEY_SUBTRACT = 0x6D,
  VKEY_DECIMAL = 0x6E,
  VKEY_DIVIDE = 0x6F,
  VKEY_F1 = 0x70,
  VKEY_F2 = 0x71,
  VKEY_F3 = 0x72,
  VKEY_F4 = 0x73,
  VKEY_F5 = 0x74,
  VKEY_F6 = 0x75,
  VKEY_F7 = 0x76,
  VKEY_F8 = 0x77,
  VKEY_F9 = 0x78,
  VKEY_F10 = 0x79,
  VKEY_F11 = 0x7A,
  VKEY_F12 = 0x7B,
  VKEY_F13 = 0x7C,
  VKEY_F14 = 0x7D,
  VKEY_F15 = 0x7E,
  VKEY_F16 = 0x7F,
  VKEY_F17 = 0x80,
  VKEY_F18 = 0x81,
  VKEY_F19 = 0x82,
  VKEY_F20 = 0x83,
  VKEY_F21 = 0x84,
  VKEY_F22 = 0x85,
  VKEY_F23 = 0x86,
  VKEY_F24 = 0x87,
  VKEY_NUMLOCK = 0x90,
  VKEY_SCROLL = 0x91,
  VKEY_LSHIFT = 0xA0,
  VKEY_RSHIFT = 0xA1,
  VKEY_LCONTROL = 0xA2,
  VKEY_RCONTROL = 0xA3,
  VKEY_LMENU = 0xA4,
  VKEY_RMENU = 0xA5,
  VKEY_BROWSER_BACK = 0xA6,
  VKEY_BROWSER_FORWARD = 0xA7,
  VKEY_BROWSER_REFRESH = 0xA8,
  VKEY_BROWSER_STOP = 0xA9,
  VKEY_BROWSER_SEARCH = 0xAA,
  VKEY_BROWSER_FAVORITES = 0xAB,
  VKEY_BROWSER_HOME = 0xAC,
  VKEY_VOLUME_MUTE = 0xAD,
  VKEY_VOLUME_DOWN = 0xAE,
  VKEY_VOLUME_UP = 0xAF,
  VKEY_MEDIA_NEXT_TRACK = 0xB0,
  VKEY_MEDIA_PREV_TRACK = 0xB1,
  VKEY_MEDIA_STOP = 0xB2,
  VKEY_MEDIA_PLAY_PAUSE = 0xB3,
  VKEY_MEDIA_LAUNCH_MAIL = 0xB4,
  VKEY_MEDIA_LAUNCH_MEDIA_SELECT = 0xB5,
  VKEY_MEDIA_LAUNCH_APP1 = 0xB6,
  VKEY_MEDIA_LAUNCH_APP2 = 0xB7,
  VKEY_OEM_1 = 0xBA,
  VKEY_OEM_PLUS = 0xBB,
  VKEY_OEM_COMMA = 0xBC,
  VKEY_OEM_MINUS = 0xBD,
  VKEY_OEM_PERIOD = 0xBE,
  VKEY_OEM_2 = 0xBF,
  VKEY_OEM_3 = 0xC0,
  VKEY_OEM_4 = 0xDB,
  VKEY_OEM_5 = 0xDC,
  VKEY_OEM_6 = 0xDD,
  VKEY_OEM_7 = 0xDE,
  VKEY_OEM_8 = 0xDF,
  VKEY_OEM_102 = 0xE2,
  VKEY_OEM_103 = 0xE3,  // GTV KEYCODE_MEDIA_REWIND
  VKEY_OEM_104 = 0xE4,  // GTV KEYCODE_MEDIA_FAST_FORWARD
  VKEY_PROCESSKEY = 0xE5,
  VKEY_PACKET = 0xE7,
  VKEY_DBE_SBCSCHAR = 0xF3,
  VKEY_DBE_DBCSCHAR = 0xF4,
  VKEY_ATTN = 0xF6,
  VKEY_CRSEL = 0xF7,
  VKEY_EXSEL = 0xF8,
  VKEY_EREOF = 0xF9,
  VKEY_PLAY = 0xFA,
  VKEY_ZOOM = 0xFB,
  VKEY_NONAME = 0xFC,
  VKEY_PA1 = 0xFD,
  VKEY_OEM_CLEAR = 0xFE,
  VKEY_UNKNOWN = 0,

  // POSIX specific VKEYs. Note that as of Windows SDK 7.1, 0x97-9F, 0xD8-DA,
  // and 0xE8 are unassigned.
  VKEY_WLAN = 0x97,
  VKEY_POWER = 0x98,
  VKEY_BRIGHTNESS_DOWN = 0xD8,
  VKEY_BRIGHTNESS_UP = 0xD9,
  VKEY_KBD_BRIGHTNESS_DOWN = 0xDA,
  VKEY_KBD_BRIGHTNESS_UP = 0xE8,

  // Windows does not have a specific key code for AltGr. We use the unused 0xE1
  // (VK_OEM_AX) code to represent AltGr, matching the behaviour of Firefox on
  // Linux.
  VKEY_ALTGR = 0xE1,
  // Windows does not have a specific key code for Compose. We use the unused
  // 0xE6 (VK_ICO_CLEAR) code to represent Compose.
  VKEY_COMPOSE = 0xE6,
};

#ifdef Q_OS_LINUX
// From ui/events/keycodes/keyboard_code_conversion_x.cc.
KeyboardCode KeyboardCodeFromNative(unsigned int keysym) {
  switch (keysym) {
    case XK_BackSpace:
      return VKEY_BACK;
    case XK_Delete:
    case XK_KP_Delete:
      return VKEY_DELETE;
    case XK_Tab:
    case XK_KP_Tab:
    case XK_ISO_Left_Tab:
      return VKEY_TAB;
    case XK_Linefeed:
    case XK_Return:
    case XK_KP_Enter:
    case XK_ISO_Enter:
      return VKEY_RETURN;
    case XK_Clear:
    case XK_KP_Begin:  // NumPad 5 without Num Lock, for crosbug.com/29169.
      return VKEY_CLEAR;
    case XK_KP_Space:
    case XK_space:
      return VKEY_SPACE;
    case XK_Home:
    case XK_KP_Home:
      return VKEY_HOME;
    case XK_End:
    case XK_KP_End:
      return VKEY_END;
    case XK_Page_Up:
    case XK_KP_Page_Up:  // aka XK_KP_Prior
      return VKEY_PRIOR;
    case XK_Page_Down:
    case XK_KP_Page_Down:  // aka XK_KP_Next
      return VKEY_NEXT;
    case XK_Left:
    case XK_KP_Left:
      return VKEY_LEFT;
    case XK_Right:
    case XK_KP_Right:
      return VKEY_RIGHT;
    case XK_Down:
    case XK_KP_Down:
      return VKEY_DOWN;
    case XK_Up:
    case XK_KP_Up:
      return VKEY_UP;
    case XK_Escape:
      return VKEY_ESCAPE;
    case XK_Kana_Lock:
    case XK_Kana_Shift:
      return VKEY_KANA;
    case XK_Hangul:
      return VKEY_HANGUL;
    case XK_Hangul_Hanja:
      return VKEY_HANJA;
    case XK_Kanji:
      return VKEY_KANJI;
    case XK_Henkan:
      return VKEY_CONVERT;
    case XK_Muhenkan:
      return VKEY_NONCONVERT;
    case XK_Zenkaku_Hankaku:
      return VKEY_DBE_DBCSCHAR;
    case XK_A:
    case XK_a:
      return VKEY_A;
    case XK_B:
    case XK_b:
      return VKEY_B;
    case XK_C:
    case XK_c:
      return VKEY_C;
    case XK_D:
    case XK_d:
      return VKEY_D;
    case XK_E:
    case XK_e:
      return VKEY_E;
    case XK_F:
    case XK_f:
      return VKEY_F;
    case XK_G:
    case XK_g:
      return VKEY_G;
    case XK_H:
    case XK_h:
      return VKEY_H;
    case XK_I:
    case XK_i:
      return VKEY_I;
    case XK_J:
    case XK_j:
      return VKEY_J;
    case XK_K:
    case XK_k:
      return VKEY_K;
    case XK_L:
    case XK_l:
      return VKEY_L;
    case XK_M:
    case XK_m:
      return VKEY_M;
    case XK_N:
    case XK_n:
      return VKEY_N;
    case XK_O:
    case XK_o:
      return VKEY_O;
    case XK_P:
    case XK_p:
      return VKEY_P;
    case XK_Q:
    case XK_q:
      return VKEY_Q;
    case XK_R:
    case XK_r:
      return VKEY_R;
    case XK_S:
    case XK_s:
      return VKEY_S;
    case XK_T:
    case XK_t:
      return VKEY_T;
    case XK_U:
    case XK_u:
      return VKEY_U;
    case XK_V:
    case XK_v:
      return VKEY_V;
    case XK_W:
    case XK_w:
      return VKEY_W;
    case XK_X:
    case XK_x:
      return VKEY_X;
    case XK_Y:
    case XK_y:
      return VKEY_Y;
    case XK_Z:
    case XK_z:
      return VKEY_Z;

    case XK_0:
    case XK_1:
    case XK_2:
    case XK_3:
    case XK_4:
    case XK_5:
    case XK_6:
    case XK_7:
    case XK_8:
    case XK_9:
      return static_cast<KeyboardCode>(VKEY_0 + (keysym - XK_0));

    case XK_parenright:
      return VKEY_0;
    case XK_exclam:
      return VKEY_1;
    case XK_at:
      return VKEY_2;
    case XK_numbersign:
      return VKEY_3;
    case XK_dollar:
      return VKEY_4;
    case XK_percent:
      return VKEY_5;
    case XK_asciicircum:
      return VKEY_6;
    case XK_ampersand:
      return VKEY_7;
    case XK_asterisk:
      return VKEY_8;
    case XK_parenleft:
      return VKEY_9;

    case XK_KP_0:
    case XK_KP_1:
    case XK_KP_2:
    case XK_KP_3:
    case XK_KP_4:
    case XK_KP_5:
    case XK_KP_6:
    case XK_KP_7:
    case XK_KP_8:
    case XK_KP_9:
      return static_cast<KeyboardCode>(VKEY_NUMPAD0 + (keysym - XK_KP_0));

    case XK_multiply:
    case XK_KP_Multiply:
      return VKEY_MULTIPLY;
    case XK_KP_Add:
      return VKEY_ADD;
    case XK_KP_Separator:
      return VKEY_SEPARATOR;
    case XK_KP_Subtract:
      return VKEY_SUBTRACT;
    case XK_KP_Decimal:
      return VKEY_DECIMAL;
    case XK_KP_Divide:
      return VKEY_DIVIDE;
    case XK_KP_Equal:
    case XK_equal:
    case XK_plus:
      return VKEY_OEM_PLUS;
    case XK_comma:
    case XK_less:
      return VKEY_OEM_COMMA;
    case XK_minus:
    case XK_underscore:
      return VKEY_OEM_MINUS;
    case XK_greater:
    case XK_period:
      return VKEY_OEM_PERIOD;
    case XK_colon:
    case XK_semicolon:
      return VKEY_OEM_1;
    case XK_question:
    case XK_slash:
      return VKEY_OEM_2;
    case XK_asciitilde:
    case XK_quoteleft:
      return VKEY_OEM_3;
    case XK_bracketleft:
    case XK_braceleft:
      return VKEY_OEM_4;
    case XK_backslash:
    case XK_bar:
      return VKEY_OEM_5;
    case XK_bracketright:
    case XK_braceright:
      return VKEY_OEM_6;
    case XK_quoteright:
    case XK_quotedbl:
      return VKEY_OEM_7;
    case XK_ISO_Level5_Shift:
      return VKEY_OEM_8;
    case XK_Shift_L:
    case XK_Shift_R:
      return VKEY_SHIFT;
    case XK_Control_L:
    case XK_Control_R:
      return VKEY_CONTROL;
    case XK_Meta_L:
    case XK_Meta_R:
    case XK_Alt_L:
    case XK_Alt_R:
      return VKEY_MENU;
    case XK_ISO_Level3_Shift:
      return VKEY_ALTGR;
    case XK_Multi_key:
      return VKEY_COMPOSE;
    case XK_Pause:
      return VKEY_PAUSE;
    case XK_Caps_Lock:
      return VKEY_CAPITAL;
    case XK_Num_Lock:
      return VKEY_NUMLOCK;
    case XK_Scroll_Lock:
      return VKEY_SCROLL;
    case XK_Select:
      return VKEY_SELECT;
    case XK_Print:
      return VKEY_PRINT;
    case XK_Execute:
      return VKEY_EXECUTE;
    case XK_Insert:
    case XK_KP_Insert:
      return VKEY_INSERT;
    case XK_Help:
      return VKEY_HELP;
    case XK_Super_L:
      return VKEY_LWIN;
    case XK_Super_R:
      return VKEY_RWIN;
    case XK_Menu:
      return VKEY_APPS;
    case XK_F1:
    case XK_F2:
    case XK_F3:
    case XK_F4:
    case XK_F5:
    case XK_F6:
    case XK_F7:
    case XK_F8:
    case XK_F9:
    case XK_F10:
    case XK_F11:
    case XK_F12:
    case XK_F13:
    case XK_F14:
    case XK_F15:
    case XK_F16:
    case XK_F17:
    case XK_F18:
    case XK_F19:
    case XK_F20:
    case XK_F21:
    case XK_F22:
    case XK_F23:
    case XK_F24:
      return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XK_F1));
    case XK_KP_F1:
    case XK_KP_F2:
    case XK_KP_F3:
    case XK_KP_F4:
      return static_cast<KeyboardCode>(VKEY_F1 + (keysym - XK_KP_F1));

    case XK_guillemotleft:
    case XK_guillemotright:
    case XK_degree:
    // In the case of canadian multilingual keyboard layout, VKEY_OEM_102 is
    // assigned to ugrave key.
    case XK_ugrave:
    case XK_Ugrave:
    case XK_brokenbar:
      return VKEY_OEM_102;  // international backslash key in 102 keyboard.

    // When evdev is in use, /usr/share/X11/xkb/symbols/inet maps F13-18 keys
    // to the special XF86XK symbols to support Microsoft Ergonomic keyboards:
    // https://bugs.freedesktop.org/show_bug.cgi?id=5783
    // In Chrome, we map these X key symbols back to F13-18 since we don't have
    // VKEYs for these XF86XK symbols.
    case XF86XK_Tools:
      return VKEY_F13;
    case XF86XK_Launch5:
      return VKEY_F14;
    case XF86XK_Launch6:
      return VKEY_F15;
    case XF86XK_Launch7:
      return VKEY_F16;
    case XF86XK_Launch8:
      return VKEY_F17;
    case XF86XK_Launch9:
      return VKEY_F18;
    case XF86XK_Refresh:
    case XF86XK_History:
    case XF86XK_OpenURL:
    case XF86XK_AddFavorite:
    case XF86XK_Go:
    case XF86XK_ZoomIn:
    case XF86XK_ZoomOut:
      // ui::AcceleratorGtk tries to convert the XF86XK_ keysyms on Chrome
      // startup. It's safe to return VKEY_UNKNOWN here since ui::AcceleratorGtk
      // also checks a Gdk keysym. http://crbug.com/109843
      return VKEY_UNKNOWN;
    // For supporting multimedia buttons on a USB keyboard.
    case XF86XK_Back:
      return VKEY_BROWSER_BACK;
    case XF86XK_Forward:
      return VKEY_BROWSER_FORWARD;
    case XF86XK_Reload:
      return VKEY_BROWSER_REFRESH;
    case XF86XK_Stop:
      return VKEY_BROWSER_STOP;
    case XF86XK_Search:
      return VKEY_BROWSER_SEARCH;
    case XF86XK_Favorites:
      return VKEY_BROWSER_FAVORITES;
    case XF86XK_HomePage:
      return VKEY_BROWSER_HOME;
    case XF86XK_AudioMute:
      return VKEY_VOLUME_MUTE;
    case XF86XK_AudioLowerVolume:
      return VKEY_VOLUME_DOWN;
    case XF86XK_AudioRaiseVolume:
      return VKEY_VOLUME_UP;
    case XF86XK_AudioNext:
      return VKEY_MEDIA_NEXT_TRACK;
    case XF86XK_AudioPrev:
      return VKEY_MEDIA_PREV_TRACK;
    case XF86XK_AudioStop:
      return VKEY_MEDIA_STOP;
    case XF86XK_AudioPlay:
      return VKEY_MEDIA_PLAY_PAUSE;
    case XF86XK_Mail:
      return VKEY_MEDIA_LAUNCH_MAIL;
    case XF86XK_LaunchA:  // F3 on an Apple keyboard.
      return VKEY_MEDIA_LAUNCH_APP1;
    case XF86XK_LaunchB:  // F4 on an Apple keyboard.
    case XF86XK_Calculator:
      return VKEY_MEDIA_LAUNCH_APP2;
    case XF86XK_WLAN:
      return VKEY_WLAN;
    case XF86XK_PowerOff:
      return VKEY_POWER;
    case XF86XK_MonBrightnessDown:
      return VKEY_BRIGHTNESS_DOWN;
    case XF86XK_MonBrightnessUp:
      return VKEY_BRIGHTNESS_UP;
    case XF86XK_KbdBrightnessDown:
      return VKEY_KBD_BRIGHTNESS_DOWN;
    case XF86XK_KbdBrightnessUp:
      return VKEY_KBD_BRIGHTNESS_UP;

      // TODO(sad): some keycodes are still missing.
  }
  return VKEY_UNKNOWN;
}
#elif defined(Q_OS_MAC)
// From ui/events/keycodes/keyboard_code_conversion_mac.mm.
KeyboardCode KeyboardCodeFromNative(unsigned short keyCode) {
  static const KeyboardCode kKeyboardCodes[] = {
    /* 0 */ VKEY_A,
    /* 1 */ VKEY_S,
    /* 2 */ VKEY_D,
    /* 3 */ VKEY_F,
    /* 4 */ VKEY_H,
    /* 5 */ VKEY_G,
    /* 6 */ VKEY_Z,
    /* 7 */ VKEY_X,
    /* 8 */ VKEY_C,
    /* 9 */ VKEY_V,
    /* 0x0A */ VKEY_OEM_3, // Section key.
    /* 0x0B */ VKEY_B,
    /* 0x0C */ VKEY_Q,
    /* 0x0D */ VKEY_W,
    /* 0x0E */ VKEY_E,
    /* 0x0F */ VKEY_R,
    /* 0x10 */ VKEY_Y,
    /* 0x11 */ VKEY_T,
    /* 0x12 */ VKEY_1,
    /* 0x13 */ VKEY_2,
    /* 0x14 */ VKEY_3,
    /* 0x15 */ VKEY_4,
    /* 0x16 */ VKEY_6,
    /* 0x17 */ VKEY_5,
    /* 0x18 */ VKEY_OEM_PLUS, // =+
    /* 0x19 */ VKEY_9,
    /* 0x1A */ VKEY_7,
    /* 0x1B */ VKEY_OEM_MINUS, // -_
    /* 0x1C */ VKEY_8,
    /* 0x1D */ VKEY_0,
    /* 0x1E */ VKEY_OEM_6, // ]}
    /* 0x1F */ VKEY_O,
    /* 0x20 */ VKEY_U,
    /* 0x21 */ VKEY_OEM_4, // {[
    /* 0x22 */ VKEY_I,
    /* 0x23 */ VKEY_P,
    /* 0x24 */ VKEY_RETURN, // Return
    /* 0x25 */ VKEY_L,
    /* 0x26 */ VKEY_J,
    /* 0x27 */ VKEY_OEM_7, // '"
    /* 0x28 */ VKEY_K,
    /* 0x29 */ VKEY_OEM_1, // ;:
    /* 0x2A */ VKEY_OEM_5, // \|
    /* 0x2B */ VKEY_OEM_COMMA, // ,<
    /* 0x2C */ VKEY_OEM_2, // /?
    /* 0x2D */ VKEY_N,
    /* 0x2E */ VKEY_M,
    /* 0x2F */ VKEY_OEM_PERIOD, // .>
    /* 0x30 */ VKEY_TAB,
    /* 0x31 */ VKEY_SPACE,
    /* 0x32 */ VKEY_OEM_3, // `~
    /* 0x33 */ VKEY_BACK, // Backspace
    /* 0x34 */ VKEY_UNKNOWN, // n/a
    /* 0x35 */ VKEY_ESCAPE,
    /* 0x36 */ VKEY_APPS, // Right Command
    /* 0x37 */ VKEY_LWIN, // Left Command
    /* 0x38 */ VKEY_SHIFT, // Left Shift
    /* 0x39 */ VKEY_CAPITAL, // Caps Lock
    /* 0x3A */ VKEY_MENU, // Left Option
    /* 0x3B */ VKEY_CONTROL, // Left Ctrl
    /* 0x3C */ VKEY_SHIFT, // Right Shift
    /* 0x3D */ VKEY_MENU, // Right Option
    /* 0x3E */ VKEY_CONTROL, // Right Ctrl
    /* 0x3F */ VKEY_UNKNOWN, // fn
    /* 0x40 */ VKEY_F17,
    /* 0x41 */ VKEY_DECIMAL, // Num Pad .
    /* 0x42 */ VKEY_UNKNOWN, // n/a
    /* 0x43 */ VKEY_MULTIPLY, // Num Pad *
    /* 0x44 */ VKEY_UNKNOWN, // n/a
    /* 0x45 */ VKEY_ADD, // Num Pad +
    /* 0x46 */ VKEY_UNKNOWN, // n/a
    /* 0x47 */ VKEY_CLEAR, // Num Pad Clear
    /* 0x48 */ VKEY_VOLUME_UP,
    /* 0x49 */ VKEY_VOLUME_DOWN,
    /* 0x4A */ VKEY_VOLUME_MUTE,
    /* 0x4B */ VKEY_DIVIDE, // Num Pad /
    /* 0x4C */ VKEY_RETURN, // Num Pad Enter
    /* 0x4D */ VKEY_UNKNOWN, // n/a
    /* 0x4E */ VKEY_SUBTRACT, // Num Pad -
    /* 0x4F */ VKEY_F18,
    /* 0x50 */ VKEY_F19,
    /* 0x51 */ VKEY_OEM_PLUS, // Num Pad =.
    /* 0x52 */ VKEY_NUMPAD0,
    /* 0x53 */ VKEY_NUMPAD1,
    /* 0x54 */ VKEY_NUMPAD2,
    /* 0x55 */ VKEY_NUMPAD3,
    /* 0x56 */ VKEY_NUMPAD4,
    /* 0x57 */ VKEY_NUMPAD5,
    /* 0x58 */ VKEY_NUMPAD6,
    /* 0x59 */ VKEY_NUMPAD7,
    /* 0x5A */ VKEY_F20,
    /* 0x5B */ VKEY_NUMPAD8,
    /* 0x5C */ VKEY_NUMPAD9,
    /* 0x5D */ VKEY_UNKNOWN, // Yen (JIS Keyboard Only)
    /* 0x5E */ VKEY_UNKNOWN, // Underscore (JIS Keyboard Only)
    /* 0x5F */ VKEY_UNKNOWN, // KeypadComma (JIS Keyboard Only)
    /* 0x60 */ VKEY_F5,
    /* 0x61 */ VKEY_F6,
    /* 0x62 */ VKEY_F7,
    /* 0x63 */ VKEY_F3,
    /* 0x64 */ VKEY_F8,
    /* 0x65 */ VKEY_F9,
    /* 0x66 */ VKEY_UNKNOWN, // Eisu (JIS Keyboard Only)
    /* 0x67 */ VKEY_F11,
    /* 0x68 */ VKEY_UNKNOWN, // Kana (JIS Keyboard Only)
    /* 0x69 */ VKEY_F13,
    /* 0x6A */ VKEY_F16,
    /* 0x6B */ VKEY_F14,
    /* 0x6C */ VKEY_UNKNOWN, // n/a
    /* 0x6D */ VKEY_F10,
    /* 0x6E */ VKEY_APPS, // Context Menu key
    /* 0x6F */ VKEY_F12,
    /* 0x70 */ VKEY_UNKNOWN, // n/a
    /* 0x71 */ VKEY_F15,
    /* 0x72 */ VKEY_INSERT, // Help
    /* 0x73 */ VKEY_HOME, // Home
    /* 0x74 */ VKEY_PRIOR, // Page Up
    /* 0x75 */ VKEY_DELETE, // Forward Delete
    /* 0x76 */ VKEY_F4,
    /* 0x77 */ VKEY_END, // End
    /* 0x78 */ VKEY_F2,
    /* 0x79 */ VKEY_NEXT, // Page Down
    /* 0x7A */ VKEY_F1,
    /* 0x7B */ VKEY_LEFT, // Left Arrow
    /* 0x7C */ VKEY_RIGHT, // Right Arrow
    /* 0x7D */ VKEY_DOWN, // Down Arrow
    /* 0x7E */ VKEY_UP, // Up Arrow
    /* 0x7F */ VKEY_UNKNOWN // n/a
  };
  if (keyCode >= 0x80)
    return VKEY_UNKNOWN;
  return kKeyboardCodes[keyCode];
}

#endif

KeyboardCode KeyboardCodeFromQtKey(int key) {
    switch (key) {
      case Qt::Key_Backspace:
        return VKEY_BACK;
      case Qt::Key_Delete:
        return VKEY_DELETE;
      case Qt::Key_Tab:
        return VKEY_TAB;
      case Qt::Key_Return:
      case Qt::Key_Enter:
        return VKEY_RETURN;
      case Qt::Key_Clear:
        return VKEY_CLEAR;
      case Qt::Key_Space:
        return VKEY_SPACE;
      case Qt::Key_Home:
        return VKEY_HOME;
      case Qt::Key_End:
        return VKEY_END;
      case Qt::Key_PageUp:
        return VKEY_PRIOR;
      case Qt::Key_PageDown:
        return VKEY_NEXT;
      case Qt::Key_Left:
        return VKEY_LEFT;
      case Qt::Key_Right:
        return VKEY_RIGHT;
      case Qt::Key_Down:
        return VKEY_DOWN;
      case Qt::Key_Up:
        return VKEY_UP;
      case Qt::Key_Escape:
        return VKEY_ESCAPE;
      case Qt::Key_Kana_Shift:
        return VKEY_KANA;
      case Qt::Key_Hangul:
        return VKEY_HANGUL;
      case Qt::Key_Hangul_Hanja:
        return VKEY_HANJA;
      case Qt::Key_Kanji:
        return VKEY_KANJI;
      case Qt::Key_Henkan:
        return VKEY_CONVERT;
      case Qt::Key_Muhenkan:
        return VKEY_NONCONVERT;
      case Qt::Key_Zenkaku_Hankaku:
        return VKEY_DBE_DBCSCHAR;
      case Qt::Key_A:
        return VKEY_A;
      case Qt::Key_B:
        return VKEY_B;
      case Qt::Key_C:
        return VKEY_C;
      case Qt::Key_D:
        return VKEY_D;
      case Qt::Key_E:
        return VKEY_E;
      case Qt::Key_F:
        return VKEY_F;
      case Qt::Key_G:
        return VKEY_G;
      case Qt::Key_H:
        return VKEY_H;
      case Qt::Key_I:
        return VKEY_I;
      case Qt::Key_J:
        return VKEY_J;
      case Qt::Key_K:
        return VKEY_K;
      case Qt::Key_L:
        return VKEY_L;
      case Qt::Key_M:
        return VKEY_M;
      case Qt::Key_N:
        return VKEY_N;
      case Qt::Key_O:
        return VKEY_O;
      case Qt::Key_P:
        return VKEY_P;
      case Qt::Key_Q:
        return VKEY_Q;
      case Qt::Key_R:
        return VKEY_R;
      case Qt::Key_S:
        return VKEY_S;
      case Qt::Key_T:
        return VKEY_T;
      case Qt::Key_U:
        return VKEY_U;
      case Qt::Key_V:
        return VKEY_V;
      case Qt::Key_W:
        return VKEY_W;
      case Qt::Key_X:
        return VKEY_X;
      case Qt::Key_Y:
        return VKEY_Y;
      case Qt::Key_Z:
        return VKEY_Z;

      case Qt::Key_ParenRight:
      case Qt::Key_0:
        return VKEY_0;
      case Qt::Key_Exclam:
      case Qt::Key_1:
        return VKEY_1;
      case Qt::Key_At:
      case Qt::Key_2:
        return VKEY_2;
      case Qt::Key_NumberSign:
      case Qt::Key_3:
        return VKEY_3;
      case Qt::Key_Dollar:
      case Qt::Key_4:
        return VKEY_4;
      case Qt::Key_Percent:
      case Qt::Key_5:
        return VKEY_5;
      case Qt::Key_AsciiCircum:
      case Qt::Key_6:
        return VKEY_6;
      case Qt::Key_Ampersand:
      case Qt::Key_7:
        return VKEY_7;
      case Qt::Key_Asterisk:
      case Qt::Key_8:
        return VKEY_8;
      case Qt::Key_ParenLeft:
      case Qt::Key_9:
        return VKEY_9;

      case Qt::Key_multiply:
        return VKEY_MULTIPLY;
      case Qt::Key_division:
        return VKEY_DIVIDE;
      case Qt::Key_Plus:
      case Qt::Key_Equal:
        return VKEY_OEM_PLUS;
      case Qt::Key_Comma:
        return VKEY_OEM_COMMA;
      case Qt::Key_Minus:
        return VKEY_OEM_MINUS;
      case Qt::Key_Period:
        return VKEY_OEM_PERIOD;
      case Qt::Key_Colon:
      case Qt::Key_Semicolon:
        return VKEY_OEM_1;
      case Qt::Key_Question:
      case Qt::Key_Slash:
        return VKEY_OEM_2;
      case Qt::Key_AsciiTilde:
      case Qt::Key_QuoteLeft:
        return VKEY_OEM_3;
      case Qt::Key_BracketLeft:
      case Qt::Key_BraceLeft:
        return VKEY_OEM_4;
      case Qt::Key_Backslash:
      case Qt::Key_Bar:
        return VKEY_OEM_5;
      case Qt::Key_BracketRight:
      case Qt::Key_BraceRight:
        return VKEY_OEM_6;
      case Qt::Key_QuoteDbl:
        return VKEY_OEM_7;
      case Qt::Key_Shift:
        return VKEY_SHIFT;
      case Qt::Key_Control:
        return VKEY_CONTROL;
      case Qt::Key_Meta:
      case Qt::Key_Alt:
        return VKEY_MENU;
      case Qt::Key_Pause:
        return VKEY_PAUSE;
      case Qt::Key_CapsLock:
        return VKEY_CAPITAL;
      case Qt::Key_NumLock:
        return VKEY_NUMLOCK;
      case Qt::Key_ScrollLock:
        return VKEY_SCROLL;
      case Qt::Key_Select:
        return VKEY_SELECT;
      case Qt::Key_Print:
        return VKEY_PRINT;
      case Qt::Key_Execute:
        return VKEY_EXECUTE;
      case Qt::Key_Insert:
        return VKEY_INSERT;
      case Qt::Key_Help:
        return VKEY_HELP;
      case Qt::Key_Super_L:
        return VKEY_LWIN;
      case Qt::Key_Super_R:
        return VKEY_RWIN;
      case Qt::Key_Menu:
        return VKEY_APPS;
      case Qt::Key_F1:
        return VKEY_F1;
      case Qt::Key_F2:
        return VKEY_F2;
      case Qt::Key_F3:
        return VKEY_F3;
      case Qt::Key_F4:
        return VKEY_F4;
      case Qt::Key_F5:
        return VKEY_F5;
      case Qt::Key_F6:
        return VKEY_F6;
      case Qt::Key_F7:
        return VKEY_F7;
      case Qt::Key_F8:
        return VKEY_F8;
      case Qt::Key_F9:
        return VKEY_F9;
      case Qt::Key_F10:
        return VKEY_F10;
      case Qt::Key_F11:
        return VKEY_F11;
      case Qt::Key_F12:
        return VKEY_F12;
      case Qt::Key_F13:
        return VKEY_F13;
      case Qt::Key_F14:
        return VKEY_F14;
      case Qt::Key_F15:
        return VKEY_F15;
      case Qt::Key_F16:
        return VKEY_F16;
      case Qt::Key_F17:
        return VKEY_F17;
      case Qt::Key_F18:
        return VKEY_F18;
      case Qt::Key_F19:
        return VKEY_F19;
      case Qt::Key_F20:
        return VKEY_F20;
      case Qt::Key_F21:
        return VKEY_F21;
      case Qt::Key_F22:
        return VKEY_F22;
      case Qt::Key_F23:
        return VKEY_F23;
      case Qt::Key_F24:
        return VKEY_F24;


      // When evdev is in use, /usr/share/X11/xkb/symbols/inet maps F13-18 keys
      // to the special XF86XK symbols to support Microsoft Ergonomic keyboards:
      // https://bugs.freedesktop.org/show_bug.cgi?id=5783
      // In Chrome, we map these X key symbols back to F13-18 since we don't have
      // VKEYs for these XF86XK symbols.
      case Qt::Key_Tools:
        return VKEY_F13;
      case Qt::Key_Launch5:
        return VKEY_F14;
      case Qt::Key_Launch6:
        return VKEY_F15;
      case Qt::Key_Launch7:
        return VKEY_F16;
      case Qt::Key_Launch8:
        return VKEY_F17;
      case Qt::Key_Launch9:
        return VKEY_F18;
      case Qt::Key_Refresh:
      case Qt::Key_History:
      case Qt::Key_OpenUrl:
      case Qt::Key_AddFavorite:
      case Qt::Key_Go:
      case Qt::Key_ZoomIn:
      case Qt::Key_ZoomOut:
        // ui::AcceleratorGtk tries to convert the XF86XK_ keysyms on Chrome
        // startup. It's safe to return VKEY_UNKNOWN here since ui::AcceleratorGtk
        // also checks a Gdk keysym. http://crbug.com/109843
        return VKEY_UNKNOWN;
      // For supporting multimedia buttons on a USB keyboard.
      case Qt::Key_Back:
        return VKEY_BROWSER_BACK;
      case Qt::Key_Forward:
        return VKEY_BROWSER_FORWARD;
      case Qt::Key_Reload:
        return VKEY_BROWSER_REFRESH;
      case Qt::Key_Stop:
        return VKEY_BROWSER_STOP;
      case Qt::Key_Search:
        return VKEY_BROWSER_SEARCH;
      case Qt::Key_Favorites:
        return VKEY_BROWSER_FAVORITES;
      case Qt::Key_HomePage:
        return VKEY_BROWSER_HOME;
      case Qt::Key_VolumeMute:
        return VKEY_VOLUME_MUTE;
      case Qt::Key_VolumeDown:
        return VKEY_VOLUME_DOWN;
      case Qt::Key_VolumeUp:
        return VKEY_VOLUME_UP;
      case Qt::Key_AudioForward:
        return VKEY_MEDIA_NEXT_TRACK;
      case Qt::Key_AudioRewind:
        return VKEY_MEDIA_PREV_TRACK;
      case Qt::Key_Play:
        return VKEY_MEDIA_PLAY_PAUSE;
      case Qt::Key_MailForward:
        return VKEY_MEDIA_LAUNCH_MAIL;
      case Qt::Key_LaunchA:  // F3 on an Apple keyboard.
        return VKEY_MEDIA_LAUNCH_APP1;
      case Qt::Key_LaunchB:  // F4 on an Apple keyboard.
      case Qt::Key_Calculator:
        return VKEY_MEDIA_LAUNCH_APP2;
      case Qt::Key_WLAN:
        return VKEY_WLAN;
      case Qt::Key_PowerOff:
        return VKEY_POWER;
      case Qt::Key_KeyboardBrightnessDown:
        return VKEY_KBD_BRIGHTNESS_DOWN;
      case Qt::Key_KeyboardBrightnessUp:
        return VKEY_KBD_BRIGHTNESS_UP;

        // TODO(sad): some keycodes are still missing.
    }
    return VKEY_UNKNOWN;
}

Renderer::Renderer(CefRefPtr<CefBrowser> browser, QWidget *parent) : QWidget(parent)
{
    this->browser = browser;
    host = browser.get()->GetHost();

    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::StrongFocus);

    image = QImage(this->size(), QImage::Format_ARGB32);
    image.fill(Qt::white);
    mainPainter = new QPainter();
}

QRect Renderer::RootScreenRect() {
    QRect screenRect;
    screenRect.setTopLeft(this->parentWidget()->mapToGlobal(this->geometry().topLeft()));
    screenRect.setSize(this->size());
    return screenRect;
}

void Renderer::paint(CefRenderHandler::PaintElementType type, QList<QRect> dirty, const void *buffer, int width, int height) {
    if (image.width() != width || image.height() != height) {
        image = QImage(width, height, QImage::Format_ARGB32);
    }

    QRegion dirtyRegion;

    mainPainter->begin(&image);
    uchar* data = (uchar*) buffer;
    for (QRect rect : dirty) {
        uchar* dirtyData = (uchar*) malloc(sizeof(uchar) * rect.width() * rect.height() * 4);

        QImage dirtySection(rect.width(), rect.height(), QImage::Format_ARGB32);
        for (int i = 0; i < rect.height(); i++) {
            memcpy(dirtySection.scanLine(i), data + ((rect.y() + i) * width * 4) + (rect.x() * 4), rect.width() * 4);
        }

        if (dirtySection.isNull()) {
            for (int i = 0; i < rect.height(); i++) {
                for (int j = 0; j < rect.width(); j++) {
                    QColor col;
                    col.setBlue(dirtyData[i * rect.width() * 4 + j * 4]);
                    col.setGreen(dirtyData[i * rect.width() * 4 + j * 4 + 1]);
                    col.setRed(dirtyData[i * rect.width() * 4 + j * 4 + 2]);

                    dirtySection.setPixelColor(j, i, col);
                }
            }
        }

        mainPainter->drawImage(rect, dirtySection);

        free(dirtyData);

        dirtyRegion = dirtyRegion.united(rect);
    }
    mainPainter->end();

    this->repaint(dirtyRegion);
}

void Renderer::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    for (QRect rect : event->region().rects()) {
        painter.drawImage(rect, image, rect);
    }

    if (paused) {
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor::fromRgb(0, 0, 0, 127));
        painter.drawRect(0, 0, this->width(), this->height());
    }

    event->accept();
}

void Renderer::resizeEvent(QResizeEvent *event) {
    host.get()->WasResized();
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    CefMouseEvent cefEvent;
    cefEvent.x = event->x();
    cefEvent.y = event->y();
    host.get()->SendMouseMoveEvent(cefEvent, false);
}

void Renderer::leaveEvent(QEvent *event) {
    CefMouseEvent cefEvent;
    host.get()->SendMouseMoveEvent(cefEvent, true);
}

void Renderer::mousePressEvent(QMouseEvent *event) {
    CefMouseEvent cefEvent;
    cefEvent.x = event->x();
    cefEvent.y = event->y();

    CefBrowserHost::MouseButtonType mouse;
    if (event->button() == Qt::LeftButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_LEFT;
    } else if (event->button() == Qt::RightButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_RIGHT;
    } else if (event->button() == Qt::MiddleButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
    }

    host.get()->SendMouseClickEvent(cefEvent, mouse, false, 1);

#ifdef Q_OS_MAC
    QWidget::mousePressEvent(event);
#endif
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    CefMouseEvent cefEvent;
    cefEvent.x = event->x();
    cefEvent.y = event->y();

    CefBrowserHost::MouseButtonType mouse;
    if (event->button() == Qt::LeftButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_LEFT;
    } else if (event->button() == Qt::RightButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_RIGHT;
    } else if (event->button() == Qt::MiddleButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
    }

    host.get()->SendMouseClickEvent(cefEvent, mouse, true, 1);

#ifdef Q_OS_MAC
    QWidget::mouseReleaseEvent(event);
#endif
}

void Renderer::mouseDoubleClickEvent(QMouseEvent *event) {
    CefMouseEvent cefEvent;
    cefEvent.x = event->x();
    cefEvent.y = event->y();

    CefBrowserHost::MouseButtonType mouse;
    if (event->button() == Qt::LeftButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_LEFT;
    } else if (event->button() == Qt::RightButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_RIGHT;
    } else if (event->button() == Qt::MiddleButton) {
        mouse = CefBrowserHost::MouseButtonType::MBT_MIDDLE;
    }

    host.get()->SendMouseClickEvent(cefEvent, mouse, false, 2);
    host.get()->SendMouseClickEvent(cefEvent, mouse, true, 2);
}

void Renderer::keyPressEvent(QKeyEvent *event) {
    CefKeyEvent::struct_type cefEvent;
    cefEvent.windows_key_code = KeyboardCodeFromQtKey(event->key());
    cefEvent.native_key_code = event->nativeVirtualKey();

    if (cefEvent.windows_key_code == VKEY_RETURN) {
        cefEvent.character = '\r';
    } else {
        QString text = event->text();
        if (!text.isEmpty()) {
            cefEvent.character = text.at(0).toLatin1();
        }
    }

    int modifiers = EVENTFLAG_NONE;

#ifdef Q_OS_MAC
    if (event->modifiers() & Qt::ControlModifier) {
        modifiers |= EVENTFLAG_COMMAND_DOWN;
    }
    if (event->modifiers() & Qt::ShiftModifier) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (event->modifiers() & Qt::MetaModifier) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
#else
    if (event->modifiers() & Qt::ControlModifier) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (event->modifiers() & Qt::ShiftModifier) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (event->modifiers() & Qt::AltModifier) {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }
#endif

    cefEvent.modifiers = modifiers;

    cefEvent.type = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
    host.get()->SendKeyEvent(cefEvent);
    cefEvent.type = cef_key_event_type_t::KEYEVENT_CHAR;
    host.get()->SendKeyEvent(cefEvent);
}

void Renderer::keyReleaseEvent(QKeyEvent *event) {
    CefKeyEvent::struct_type cefEvent;
    cefEvent.windows_key_code = KeyboardCodeFromQtKey(event->key());
    cefEvent.native_key_code = event->nativeVirtualKey();

    if (cefEvent.windows_key_code == VKEY_RETURN) {
        cefEvent.character = '\r';
    } else {
        QString text = event->text();
        if (!text.isEmpty()) {
            cefEvent.character = text.at(0).toLatin1();
        }
    }

    int modifiers = EVENTFLAG_NONE;

#ifdef Q_OS_MAC
    if (event->modifiers() & Qt::ControlModifier) {
        modifiers |= EVENTFLAG_COMMAND_DOWN;
    }
    if (event->modifiers() & Qt::ShiftModifier) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (event->modifiers() & Qt::MetaModifier) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
#else
    if (event->modifiers() & Qt::ControlModifier) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (event->modifiers() & Qt::ShiftModifier) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (event->modifiers() & Qt::AltModifier) {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }
#endif

    cefEvent.modifiers = modifiers;

    cefEvent.type = cef_key_event_type_t::KEYEVENT_KEYUP;
    host.get()->SendKeyEvent(cefEvent);
}

void Renderer::focusInEvent(QFocusEvent *event) {
    host.get()->SendFocusEvent(true);
}

void Renderer::focusOutEvent(QFocusEvent *event) {
    host.get()->SendFocusEvent(false);
}

void Renderer::wheelEvent(QWheelEvent *event) {
    CefMouseEvent cefEvent;
    cefEvent.x = event->x();
    cefEvent.y = event->y();

    host.get()->SendMouseWheelEvent(cefEvent, event->angleDelta().x(), event->angleDelta().y());
}

void Renderer::setCursor(CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo &custom_cursor_info) {
    QCursor cur;

    switch (type) {
        case CefRenderHandler::CursorType::CT_POINTER:
            cur.setShape(Qt::ArrowCursor);
            break;
        case CefRenderHandler::CursorType::CT_HAND:
            cur.setShape(Qt::PointingHandCursor);
            break;
        case CefRenderHandler::CursorType::CT_IBEAM:
            cur.setShape(Qt::IBeamCursor);
            break;
    }

    QWidget::setCursor(cur);
}

void Renderer::pause() {
    paused = true;
    this->update();
}

void Renderer::resume() {
    paused = false;
    this->update();
}
