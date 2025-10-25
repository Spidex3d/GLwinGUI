#pragma once
// mouse line 69

// true - false
#define GLWIN_TRUE                   1
#define GLWIN_FALSE                  0

// Window definitions and constants
#define GLWIN_MAXIMIZED              0x00020008
#define GLWIN_RESIZABLE              0x00020003

// OpenGl definitions and constants
#define GLWIN_CONTEXT_VERSION_MAJOR  0x00022002
#define GLWIN_CONTEXT_VERSION_MINOR  0x00022003
#define GLWIN_OPENGL_PROFILE         0x00022008
#define GLWIN_OPENGL_CORE_PROFILE    0x00032001




//#define WM_KEYFIRST                     0x0100
//#define WM_KEYDOWN                      0x0100
//#define VK_END            0x23
//#define VK_HOME           0x24
//#define WM_KEYUP                        0x0101
// Key codes
#define GLWIN_KEYDOWN                 256 //0x0100
#define GLWIN_KEYUP                   257 //0x0101
// Keyboard input
#define GLWIN_PRESS                   1
#define GLWIN_RELEASE                 0
// Top row number keys ect.          
#define GLWIN_ESCAPE                  27 //0x1B

// Action keys Enter, Space, Arrows, etc.
#define GLWIN_RETURN                  13 //0x0D

#define GLWIN_LEFT                    37 //0x25
#define GLWIN_UP                      38 //0x26
#define GLWIN_RIGHT                   39 //0x27
#define GLWIN_DOWN                    40 //0x28

#define GLWIN_INSERT                  45 //0x2D
#define GLWIN_BACKSPACE               8  //0x08
#define GLWIN_TAB                     9  //0x09
#define GLWIN_SPACE                   32 //0x20 space
#define GLWIN_PAGE_UP                 33 //0x21
#define GLWIN_PAGE_DOWN               34 //0x22
#define GLWIN_HOME                    36 //0x24
#define GLWIN_END                     35 //0x23
#define GLWIN_TILDE                   126 //0x7E ~
#define GLWIN_DELETE                  127 //0x7f Delete

#define GLWIN_DOT					  46 //0x2E .
#define GLWIN_COMMA                   44 //0x2C ,
#define GLWIN_SEMICOLON               59 //0x3B ;
#define GLWIN_SLASH                   47 //0x2F /
#define GLWIN_BACKSLASH               92 //0x5C \

#define GLWIN_LEFT_BRACKET            91 //0x5B [
#define GLWIN_RIGHT_BRACKET           93 //0x5D ]
#define GLWIN_EQUAL                   61 //0x3D =
#define GLWIN_QMARK                   63 //0x3f ?
#define GLWIN_AT                      64 //0x40 @
#define GLWIN_COLON                   58 //0x3A :
#define GLWIN_CARET                   94 //0x5E ^
#define GLWIN_EXCLAMATION             33 //0x21 !
#define GLWIN_DOUBLE_QUOTE            34 //0x22 "
#define GLWIN_HASH                    35 //0x23 #	
#define GLWIN_DOLLAR                  36 //0x24 $
#define GLWIN_PERCENT                 37 //0x25 %
#define GLWIN_AMPERSAND               38 //0x26 &
#define GLWIN_APOSTROPHE              39 //0x27 '
#define GLWIN_LEFT_PARENTHESIS        40 //0x28 (
#define GLWIN_RIGHT_PARENTHESIS       41 //0x29 )
#define GLWIN_ASTERISK                42 //0x2A *
#define GLWIN_PLUS                    43 //0x2B +
#define GLWIN_MINUS                   45 //0x2D -
#define GLWIN_TABULATION              9  //0x09 Tab





// Alphabet keys 65 to 90	         
#define GLWIN_KEY_A                   65 //0x41
#define GLWIN_KEY_B                   66 //0x42
#define GLWIN_KEY_C                   67 //0x43
#define GLWIN_KEY_D                   68 //0x44
#define GLWIN_KEY_E                   69
#define GLWIN_KEY_F                   70
#define GLWIN_KEY_G                   71
#define GLWIN_KEY_H                   72
#define GLWIN_KEY_I                   73
#define GLWIN_KEY_J                   74
#define GLWIN_KEY_K                   75
#define GLWIN_KEY_L                   76
#define GLWIN_KEY_M                   77
#define GLWIN_KEY_N                   78
#define GLWIN_KEY_O                   79
#define GLWIN_KEY_P                   80
#define GLWIN_KEY_Q                   81
#define GLWIN_KEY_R                   82
#define GLWIN_KEY_S                   83
#define GLWIN_KEY_T                   84
#define GLWIN_KEY_U                   85
#define GLWIN_KEY_V                   86
#define GLWIN_KEY_W                   87
#define GLWIN_KEY_X                   88
#define GLWIN_KEY_Y                   89
#define GLWIN_KEY_Z                   90
// Number keys 48 to 57
#define GLWIN_KEY_0                   48 //0x30
#define GLWIN_KEY_1                   49 //0x31
#define GLWIN_KEY_2                   50 //0x32
#define GLWIN_KEY_3                   51 //0x33
#define GLWIN_KEY_4                   52 //0x34
#define GLWIN_KEY_5                   53 //0x35
#define GLWIN_KEY_6                   54 //0x36
#define GLWIN_KEY_7                   55 //0x37
#define GLWIN_KEY_8                   56 //0x38
#define GLWIN_KEY_9                   57 //0x39


// Mouse buttons				     
#define GLWIN_MOUSE_BUTTON_LEFT       0
#define GLWIN_MOUSE_BUTTON_RIGHT      1
#define GLWIN_MOUSE_BUTTON_MIDDLE     2

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// mouse button actions			     
#define GLWIN_PRESS                   1
#define GLWIN_RELEASE                 0


#define GLWIN_NO_ERROR                0

