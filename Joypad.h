#include "Joypad.c"

#define JOY_PAD_1       0
#define JOY_PAD_2       1
#define JOY_PAD_A       0x01
#define JOY_PAD_B       0x02
#define JOY_PAD_SELECT  0x04
#define JOY_PAD_START   0x08
#define JOY_PAD_UP      0x10
#define JOY_PAD_DOWN    0x20
#define JOY_PAD_LEFT    0x40
#define JOY_PAD_RIGHT   0x80

void InitJoypad();
void SetState(int pad, int index, BYTE val);
void InputBurst(BYTE burst) ;  
BYTE GetValue(int padbit);
