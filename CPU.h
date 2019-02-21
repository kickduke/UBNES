#include "CPU.c"    

BYTE NES_Read(WORD addr);
void NES_Write(WORD addr, BYTE val);
void CPU6502Reset() ;
BYTE CPU6502Read(WORD addr);
WORD CPU6502ReadW(WORD addr);
void CPU6502Write(WORD addr, BYTE val);
BYTE *  GetRAM(WORD addr) ;   
void  ExecOnBaseCycle(int BaseCycle);
int  Exec(int CpuCycle);
BYTE NES_ReadReg(WORD addr)  ;  
void NES_WriteReg(WORD addr, BYTE val);
BYTE * NES_GetRAM(WORD addr) ;
void Mapper002_WriteLow(WORD addr, BYTE data);
void Mapper002_Write(WORD addr, BYTE data);
