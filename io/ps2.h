#pragma once

#define ESC_key 0x76
#define F1_key 0x05
#define F2_key 0x06
#define F3_key 0x04
#define F4_key 0x0C
#define F5_key 0x03
#define F6_key 0x0B
#define F7_key 0x83
#define F8_key 0x0A
#define F9_key 0x01
#define F10_key 0x09
#define F11_key 0x78
#define F12_key 0x07
#define Comma_key 0x41
#define Period_key 0x49
#define Tilda_key 0x0E
#define One_key 0x16
#define Two_key 0x1E
#define Three_key 0x26
#define Four_key 0x25
#define Five_key 0x2E
#define RSuper_key 0x27
#define Menus_key 0x2F
#define Nine_key 0x46
#define Zero_key 0x45
#define Minus_key 0x4E
#define Equal_key 0x55
#define Backspace_key 0x66
#define Tab_key 0x0D
#define Q_key 0x15
#define W_key 0x1D
#define E_key 0x24
#define R_key 0x2D
#define T_key 0x2C
#define Y_key 0x35
#define U_key 0x3C
#define I_key 0x43
#define O_key 0x44
#define P_key 0x4D
#define LBracket_key 0x54
#define RBracket_key 0x5B
#define Backslash_key 0x5D
#define CapsLock_key 0x58
#define A_key 0x1C
#define S_key 0x1B
#define D_key 0x23
#define F_key 0x2B
#define G_key 0x34
#define H_key 0x33
#define J_key 0x3B
#define K_key 0x42
#define L_key 0x4B
#define Semicolon_key 0x4C
#define Quote_key 0x52
#define Enter_key 0x5A
#define LShift_key 0x12
#define Z_key 0x1A
#define X_key 0x22
#define C_key 0x21
#define V_key 0x2A
#define B_key 0x32
#define N_key 0x31
#define M_key 0x3A
#define ScrLock_key 0x7E
#define Slash_key 0x4A
#define RShift_key 0x59
#define Ctrl_key 0x14
#define LSuper_key 0x1F
#define Alt_key 0x11
#define Space_key 0x29
#define Six_key 0x36
#define Seven_key 0x3D
#define Eight_key 0x3E
#define Insert_key 0x70
#define Home_key 0x6C
#define PgUp_key 0x7D
#define Del_key 0x71
#define End_key 0x69
#define PgDown_key 0x7A
#define Up_key 0x75
#define Left_key 0x6B
#define Down_key 0x72
#define Right_key 0x74
#define NumLock_key 0x77
#define KpMultiply_key 0x7C
#define KpMinus_key 0x7B
#define KpPlus_key 0x79
#define Kp5_key 0x73

void updateKeyboard();

bool isKeyHeld(char keycode);
bool isKeyPressed(char keycode);
bool isKeyReleased(char keycode);
bool isKeyDown(char keycode);