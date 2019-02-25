#include <memory.h>

//宏定义
#define PRG_BLOCK_SIZE  0x4000     //Cartidge的单粒PROM固定大小16KB，颗粒数量对应Romheader[4]     
#define PAT_BLOCK_SIZE  0x2000     //Cartidge的单粒VROM固定大小8KB，颗粒数量对应Romheader[5]

//变量定义
typedef struct tagNESCONFIG      //NES系统配置数据，分为NTSC和PAL制式
{
    float BaseClock;         
    float CpuClock;          
    int TotalScanlines;      
    int ScanlineCycles;      
    int HDrawCycles;         
    int HBlankCycles;        
    int ScanlineEndCycles;   
    int FrameCycles;         
    int FrameIrqCycles;      
    int FrameRate;           
    float FramePeriod;     
} NESCONFIG, *LPNESCONFIG;

NESCONFIG NESCONFIG_NTSC =
{
    21477270.0f,         
    1789772.5f,      
    262,             
    1364,            
    1024,            
    340,             
    4,               
    1364 * 262,      
    29830,           
    60,              
    1000.0f / 60.0f  
};
NESCONFIG NESCONFIG_PAL =
{
    21281364.0f,         
    1773447.0f,      
    312,             
    1362,            
    1024,            
    338,             
    2,               
    1362 * 312,      
    35469,           
    50,              
    1000.0f / 50.0f  
};
NESCONFIG *NesCfg = &NESCONFIG_NTSC;      //默认NTSC

BYTE RomHeader[16];       //ROM文件头
BYTE RAM[0x2000];         //CPU逻辑地址0x0000-0x1FFF 其中0x0000-0x07FF为PRAM
BYTE SRAM[0x2000];        //CPU逻辑地址0x6000-0x7FFF 
BYTE *CPU_MEM_BANK[8];     
BYTE *PRGBlock = NULL;
BYTE *PatternTable = NULL; 
BYTE NameTable[0x800];       
BYTE* ScreenBit;

int bIsRunning = FALSE;
int PROM_8K_SIZE;
int PROM_16K_SIZE;
int PROM_32K_SIZE;    
int VROM_1K_SIZE;
int VROM_2K_SIZE;
int VROM_4K_SIZE;
int VROM_8K_SIZE;
int MapperNo;
BYTE VRAM[4 * 1024];

//函数定义
void NES_Init()                           
{   
    int i,j;
    for ( i = 0; i < 256; i++) {
        BYTE c = 0;
        BYTE mask = 0x80;
        for ( j = 0; j < 8; j++) {
            if (i & (1 << j)) {
                c |= (mask >> j);
            }
        }
        RevertByte[i] = c;
    }
}


void NES_ReleasePRGBlock()
{
    if (PRGBlock) {
        free(PRGBlock);
        PRGBlock = NULL;
    }
}


void NES_ReleasePatternTable()
{
    if (PatternTable) {
        free(PatternTable);
        PatternTable = NULL;
    }
}


int NES_IsRunning()
{
    return bIsRunning;
}

void CPU6502Reset();
void NES_Start()
{
    memset(RAM,       0, sizeof(RAM));
    memset(SRAM,      0, sizeof(SRAM));
    memset(NameTable, 0, sizeof(NameTable));
    CPU6502Reset();
    PPU6528Reset();
    bIsRunning = TRUE;
}


void NES_Stop()
{
	if (NES_IsRunning()) {
		NES_ReleasePRGBlock();
		NES_ReleasePatternTable();
	}
	bIsRunning = FALSE;
}


void SetPROM_8K_Bank(BYTE page, int bank)
{
    bank %= PROM_8K_SIZE;
    CPU_MEM_BANK[page] = PRGBlock + 0x2000 * bank;
}


void SetPROM_16K_Bank(BYTE page, int bank)
{
    SetPROM_8K_Bank(page + 0, bank * 2 + 0);
    SetPROM_8K_Bank(page + 1, bank * 2 + 1);
}


void SetPROM_32K_Bank(int bank)
{
	SetPROM_8K_Bank(4, bank * 4 + 0);
	SetPROM_8K_Bank(5, bank * 4 + 1);
	SetPROM_8K_Bank(6, bank * 4 + 2);
	SetPROM_8K_Bank(7, bank * 4 + 3);
}


void SetPROM002_32K_Bank(int bank0, int bank1, int bank2, int bank3)
{
	SetPROM_8K_Bank(4, bank0);
	SetPROM_8K_Bank(5, bank1);
	SetPROM_8K_Bank(6, bank2);
	SetPROM_8K_Bank(7, bank3);
}


void SetVROM_1K_Bank(BYTE page, int bank)
{
    bank %= VROM_1K_SIZE;
    PPU_MEM_BANK[page] = PatternTable + 0x0400 * bank;   //from PPU.c 指向12个1KB的bank
}


void SetVROM_8K_Bank(int bank)
{
    int i;
    for( i = 0; i < 8; i++) {
        SetVROM_1K_Bank(i, bank * 8 + i);
    }
}


void SetSRAM_1K_Bank(BYTE page, int bank)
{
    bank &= 0x1F;
    PPU_MEM_BANK[page] = SRAM + 0x0400 * bank;
}


void SetSRAM_8K_Bank(int bank)
{
    int i;
    for ( i = 0; i < 8; i++) {
        SetSRAM_1K_Bank(i, bank * 8 + i);
    }
}


void SetVRAM_1K_Bank(BYTE page, int bank)
{
    bank &= 3;          
    PPU_MEM_BANK[page] = VRAM + 0x0400 * bank;     
}


void SetNameTable_Bank(int bank0, int bank1, int bank2, int bank3)
{
    SetVRAM_1K_Bank( 8, bank0);
    SetVRAM_1K_Bank( 9, bank1);
    SetVRAM_1K_Bank(10, bank2);
    SetVRAM_1K_Bank(11, bank3);
}


int NES_LoadRom(char *FileName)
{
    FILE *fp;
    fp = fopen(FileName,"rb");
    fread(RomHeader, sizeof(BYTE), 16, fp);  
    memset(CPU_MEM_BANK, 0, sizeof(CPU_MEM_BANK));
    memset(PPU_MEM_BANK, 0, sizeof(PPU_MEM_BANK));
    memset(SRAM, 0, sizeof(SRAM));
    memset(VRAM, 0, sizeof(VRAM));
     
    if (RomHeader[4] > 0) {
        PRGBlock = (BYTE*)malloc(RomHeader[4] * PRG_BLOCK_SIZE * sizeof(BYTE));   //为PROM分配内存空间 
        fread(PRGBlock, sizeof(BYTE), RomHeader[4] * PRG_BLOCK_SIZE, fp);         //从ROM文件读入PROM数据到内存
    }
    if (RomHeader[5] > 0) {
        PatternTable = (BYTE*)malloc(RomHeader[5] * PAT_BLOCK_SIZE*sizeof(BYTE)); //为VROM分配内存空间
        fread(PatternTable, sizeof(BYTE),RomHeader[5] * PAT_BLOCK_SIZE, fp);      //从ROM文件读入VROM数据到内存
    }
    fclose(fp); 
    MapperNo = RomHeader[6] >> 4;             //Mapper编号低两位
    PROM_8K_SIZE  = RomHeader[4] * 2;         //大小为8KB的PROM颗粒数量（换算）
    PROM_16K_SIZE = RomHeader[4];             
    PROM_32K_SIZE = RomHeader[4] / 2;
    VROM_1K_SIZE = RomHeader[5] * 8;          //大小为1KB的VROM颗粒数量（换算）
    VROM_2K_SIZE = RomHeader[5] * 4;
    VROM_4K_SIZE = RomHeader[5] * 2;
    VROM_8K_SIZE = RomHeader[5];

    if(VROM_8K_SIZE) {
        SetVROM_8K_Bank(0);    //将VROM映射到PPU的逻辑地址中
    }else {
        SetSRAM_8K_Bank(0);    //类似mapper002没有VROM，将SRAM映射到PPU的逻辑地址中
    }
    if(RomHeader[6] & 0x2) {   //存在SRAM
		CPU_MEM_BANK[3] = SRAM;   //对应CPU逻辑地址的0x6000-0x7FFF
    }
    if (RomHeader[6] & 0x01) {
        SetNameTable_Bank(0, 1, 0, 1);
    }
    if(!(RomHeader[6] & 0x01)) {                        
        SetNameTable_Bank(0, 0, 1, 1);
    }
    return TRUE;
}


void ExecOnBaseCycle(int BaseCycle);
void NES_FrameExec()
{
    int i;
    
    RanderBottomBG(ScreenBit);          
    ExecOnBaseCycle(NesCfg->ScanlineCycles);   
    ScanLine(ScreenBit, 0);
    ScanlineStart();

    for (i = 1; i < 240; i++) {
         ScanLine(ScreenBit, i);
         ExecOnBaseCycle(NesCfg->ScanlineCycles);
         ScanlineStart();
    }

    ExecOnBaseCycle(NesCfg->HDrawCycles);
    ExecOnBaseCycle(NesCfg->HBlankCycles);

     
    VBlankStart();
    ExecOnBaseCycle(NesCfg->HDrawCycles);
    ExecOnBaseCycle(NesCfg->HBlankCycles);

     
    for (i = 242; i < NesCfg->TotalScanlines - 1; i++) {
         ExecOnBaseCycle(NesCfg->HDrawCycles);
         ExecOnBaseCycle(NesCfg->HBlankCycles);
    }

    VBlankEnd();
    ExecOnBaseCycle(NesCfg->HDrawCycles);
    ExecOnBaseCycle(NesCfg->HBlankCycles);
}
