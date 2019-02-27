//完成Mapper000的PROM在CPU逻辑地址的映射
void Mapper000_Reset()
{
	switch(RomHeader[4]) {
		case 1 :	// 16KB的PROM有1个，映射到CPU逻辑地址
			SetPROM_16K_Bank(4, 0);     
			SetPROM_16K_Bank(6, 0);
			break;
		case 2 :	// 16KB的PROM有2个，映射到CPU逻辑地址
			SetPROM_32K_Bank(0);
			break;
	}
}


//完成Mapper002的PROM在CPU逻辑地址的映射
void Mapper002_Reset()
{
	SetPROM002_32K_Bank(0, 1, PROM_8K_SIZE - 2, PROM_8K_SIZE - 1);
}


void Mapper002_WriteLow(WORD addr, BYTE data)
{
	if( addr >= 0x6000 && addr <= 0x7FFF ) {
		CPU_MEM_BANK[addr>>13][addr&0x1FFF] = data;
	}
}


void Mapper002_Write(WORD addr, BYTE data)
{
	SetPROM_16K_Bank(4, data);
} 


void CreateMapper(int no)
{
	switch(no) {
	    case 0:	
            Mapper000_Reset();	
            break;
	    case 2:	
            Mapper002_Reset();	
            break;
	    default :;
	}
}
