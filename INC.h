#include "INC.c"

#define GetBit(val, i)		(((val) & (1 << i)) >> i)

void SCREENOFFSET_Reset(SCREENOFFSET* a_screenoffset);
void SCREENOFFSET_ResetY(SCREENOFFSET* a_screenoffset);
void SCREENOFFSET_SetValue(SCREENOFFSET* a_screenoffset, BYTE val);
int SCREENOFFSET_AtX(SCREENOFFSET* a_screenoffset);

void ADDRESS_Reset(ADDRESS* a_address);
void ADDRESS_ResetL(ADDRESS* a_address);
void ADDRESS_SetAddress(ADDRESS* a_address, BYTE val);
WORD ADDRESS_GetAddress(ADDRESS* a_address);
void ADDRESS_Step(ADDRESS* a_address, int add);
int ADDRESS_AtLow(ADDRESS* a_address);
