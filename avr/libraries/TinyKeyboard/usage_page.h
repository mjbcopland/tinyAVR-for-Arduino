#ifndef __usage_page_h__
#define __usage_page_h__

/*
  Keyboard usage values, see usb.org's HID-usage-tables document, chapter
  10 Keyboard/Keypad Page for more codes.
  http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
*/

namespace Keycodes {
  enum KeyboardKey {
    Keyboard_A = 4,
    Keyboard_B,
    Keyboard_C,
    Keyboard_D,
    Keyboard_E,
    Keyboard_F,
    Keyboard_G,
    Keyboard_H,
    Keyboard_I,
    Keyboard_J,
    Keyboard_K,
    Keyboard_L,
    Keyboard_M,
    Keyboard_N,
    Keyboard_O,
    Keyboard_P,
    Keyboard_Q,
    Keyboard_R,
    Keyboard_S,
    Keyboard_T,
    Keyboard_U,
    Keyboard_V,
    Keyboard_W,
    Keyboard_X,
    Keyboard_Y,
    Keyboard_Z,
    Keyboard_1,  // and !
    Keyboard_2,  // and @
    Keyboard_3,  // and #
    Keyboard_4,  // and $
    Keyboard_5,  // and %
    Keyboard_6,  // and ^
    Keyboard_7,  // and &
    Keyboard_8,  // and *
    Keyboard_9,  // and (
    Keyboard_0,  // and )
    Keyboard_Return,
    Keyboard_Escape,
    Keyboard_Backspace,
    Keyboard_Tab,
    Keyboard_Spacebar,
    Keyboard_Hyphen,        // and _
    Keyboard_Equals,        // and +
    Keyboard_OpenBracket,   // and {
    Keyboard_CloseBracket,  // and }
    Keyboard_Backslash,     // and |
    Keyboard_NonUSHash,     // and ~
    Keyboard_Semicolon,     // and :
    Keyboard_Quote,         // and "
    Keyboard_GraveAccent,   // and Tilde
    Keyboard_Comma,         // and <
    Keyboard_Period,        // and >
    Keyboard_Slash,         // and ?
    Keyboard_CapsLock,
    Keyboard_F1,
    Keyboard_F2,
    Keyboard_F3,
    Keyboard_F4,
    Keyboard_F5,
    Keyboard_F6,
    Keyboard_F7,
    Keyboard_F8,
    Keyboard_F9,
    Keyboard_F10,
    Keyboard_F11,
    Keyboard_F12,
    Keyboard_PrintScreen,
    Keyboard_ScrollLock,
    Keyboard_Pause,
    Keyboard_Insert,
    Keyboard_Home,
    Keyboard_PageUp,
    Keyboard_Delete,
    Keyboard_End,
    Keyboard_PageDown,
    Keyboard_RightArrow,
    Keyboard_LeftArrow,
    Keyboard_DownArrow,
    Keyboard_UpArrow
  };

  enum KeypadKey {
    Keypad_NumLock = 83,
    Keypad_Slash,
    Keypad_Asterisk,
    Keypad_Hyphen,
    Keypad_Plus,
    Keypad_Enter,
    Keypad_1,
    Keypad_2,
    Keypad_3,
    Keypad_4,
    Keypad_5,
    Keypad_6,
    Keypad_7,
    Keypad_8,
    Keypad_9,
    Keypad_0,
    Keypad_Period
  };

  enum VolumeKey {
    Volume_Mute = 127,
    Volume_Up,
    Volume_Down
  };
}

#endif