#include "PPU.c"    
    
void NMI();
void PPU6528Reset();
void WriteToPort(WORD addr, BYTE val);
BYTE PPU6528Read(WORD addr);
void PPU6528Write(WORD addr, BYTE val);
void ScanlineStart();
void RanderBottomBG(BYTE* pBit)  ;
void ScanLine(BYTE* pBit, int LineNo);
void ScanHitPoint(BYTE LineNo) ;    
void ScanSprite(BYTE* pBit, BYTE LineNo, int bBackLevel);
void ScanBG(BYTE* pBit, BYTE LineNo);
BYTE GetScreenBGColor(int x, int y);
void VBlankStart();
void VBlankEnd();
BYTE ReadFromPort(WORD addr);


