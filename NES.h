#include "NES.c"

//函数声明
void NES_Init() ;                          
void NES_Start();
void NES_Stop();
int NES_LoadRom(char *FileName);
void NES_ReleasePRGBlock();
void NES_ReleasePatternTable();
int NES_IsRunning();
void NES_FrameExec();
void SetPROM_8K_Bank(BYTE page, int bank);
void SetPROM_16K_Bank(BYTE page, int bank);
void SetPROM_32K_Bank(int bank);
void SetPROM002_32K_Bank(int bank0, int bank1, int bank2, int bank3);
void SetVROM_1K_Bank(BYTE page, int bank);
void SetVROM_8K_Bank(int bank);
void SetSRAM_1K_Bank(BYTE page, int bank);
void SetSRAM_8K_Bank(int bank);
void SetVRAM_1K_Bank(BYTE page, int bank);
void SetNameTable_Bank(int bank0, int bank1, int bank2, int bank3);
                   
