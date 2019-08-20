#define NMI_FLAG    0x01 
#define PPU_VBLANK_BIT      0x80
#define PPU_SPHIT_BIT       0x40
#define PPU_SP16_BIT        0x20
#define PPU_BGTBL_BIT       0x10
#define PPU_SPTBL_BIT       0x08
#define PPU_INC32_BIT       0x04
#define PPU_NAMETBL_BIT     0x03
#define PPU_SHOWCOLOR       0x00     
#define PPU_NOCOLOR         0x01     
#define PPU_LEFT8COL        0x02     
#define PPU_SPRLEFT8COL     0x04     
#define PPU_SHOWBG          0x08     
#define PPU_SHOWSPR         0x10     
#define PPU_VBLANK_FLAG     0x80
#define PPU_SPHIT_FLAG      0x40
#define PPU_SPMAX_FLAG      0x20
#define PPU_WENABLE_FLAG    0x10
#define SP_VREVERT          0x80     
#define SP_HREVERT          0x40     
#define SP_LEVEL            0x20     
#define SP_HIGHCOLOR        0X03 

BYTE SPRAM[0x100]; 
typedef struct tagSPRITE
{
    BYTE Y;
    BYTE Index;
    BYTE Attribute;
    BYTE X;
} SPRITE; 
SPRITE * Sprite = (SPRITE *)SPRAM;
BYTE* PPU_MEM_BANK[12]; 
     
BYTE INT_pending;
BYTE *m_PatternTable;
BYTE *m_NameTable[4];
BYTE m_REG[0x04];                        
BYTE BGPal[0x10];            
BYTE SPPal[0x10];            
BYTE RevertByte[256];   
BYTE m_Reg2007Temp=0;
int m_CurLineSprite;
WORD m_ByteIndex;
WORD m_CurByteIndex;


void  NMI()
{
    INT_pending |= NMI_FLAG;
}
 

void PPU6528Reset()
{
    memset(m_REG, 0, sizeof(m_REG));
    memset(SPRAM, 0, sizeof(SPRAM));
    m_ByteIndex = 0;
}


BYTE PPU6528Read(WORD addr)
{
    if (addr < 0x3000) {
        return PPU_MEM_BANK[addr >> 10][addr & 0x3FF];
    }else if (addr < 0x3F00) {
        return PPU6528Read(addr - 0x1000);
    }else if (addr < 0x4000) {
        if (addr & 0x0010) {
            return SPPal[addr & 0xF];
        }else {
            return BGPal[addr & 0xF];
        }
    }else {
        return PPU6528Read(addr & 0x3FFF);
    }
}


BYTE  ReadFromPort(WORD addr)
{
    BYTE value=0x00;
    WORD address;
    switch (addr)
    {
    case 0x2002 :
        value = m_REG[2];
        ADDRESS_Reset(&m_Address);
        SCREENOFFSET_Reset(&m_ScreenOffset); 
        m_REG[2] &= ~PPU_VBLANK_FLAG;   
        break;
    case 0x2004 :            
        value = SPRAM[m_REG[3]++];
        break;
    case 0x2007 :                       
        value = m_Reg2007Temp;                 
        address = ADDRESS_GetAddress(&m_Address);
        m_Reg2007Temp = PPU6528Read(address);
        if(address >= 0x3F00 && address < 0x4000) {
            if(address & 0x0010) {                  
                value = SPPal[address & 0xF];
            }else {
                value = BGPal[address & 0xF];
            }
        }
        ADDRESS_Step(&m_Address,m_REG[0] & PPU_INC32_BIT ? 32 : 1);
    }
    return value;
}


void PPU6528Write(WORD addr, BYTE val)
{
    if (addr < 0x3000) {
        PPU_MEM_BANK[addr >> 10][addr & 0x3FF] = val;     //mapper2的copy即使用这里
    }else if (addr < 0x3F00) {
        PPU6528Write(addr - 0x1000, val);
    }else if (addr < 0x4000) {
        if (addr & 0x0010) {
            SPPal[addr & 0xF] = val;
        }else {
            BGPal[addr & 0xF] = val;
        }
        if (!(addr & 0x000F)) {
            BGPal[0x0] = SPPal[0x10] = val;
            BGPal[0x04] = BGPal[0x08] = BGPal[0x0C] = BGPal[0x00];
            SPPal[0x14] = SPPal[0x18] = SPPal[0x1C] = SPPal[0x10];
        }
    }else {
        PPU6528Write(addr & 0x3FFF, val);
    }
}


void WriteToPort(WORD addr, BYTE val)
{
    switch (addr) {
    case 0x2000 : 
        m_ByteIndex = (m_ByteIndex & 0xF3FF) | (((WORD)val & 0x03) << 10);  
        if((val & 0x80) && !(m_REG[0] & 0x80) && (m_REG[2] & 0x80)) {
             NMI();
        }
        m_REG[0] = val;
        break;
    case 0x2001 :
        m_REG[1] = val;
        break;
    case 0x2003 :
        m_REG[3] = val;
        break;
    case 0x2004 :
        SPRAM[m_REG[3]++] = val;
        break;
    case 0x2005 :
        if (SCREENOFFSET_AtX(&m_ScreenOffset)) {
            m_ByteIndex = (m_ByteIndex & 0xFFE0) | (val >> 3);
        }else {
            m_ByteIndex = (m_ByteIndex & 0xFC1F) | ((val & 0xF8) << 2); 
        }
        SCREENOFFSET_SetValue(&m_ScreenOffset, val);
        break;
    case 0x2006 :
        if(!ADDRESS_AtLow(&m_Address)) {
            m_ByteIndex = (m_ByteIndex & 0xF3FF) | ((val & 0x0C) << 8);
        }
        ADDRESS_SetAddress(&m_Address, val);
        break;
    case 0x2007 :
        PPU6528Write(ADDRESS_GetAddress(&m_Address), val);
        ADDRESS_Step(&m_Address, m_REG[0] & PPU_INC32_BIT ? 32 : 1);
        break;
    }
}


void ScanlineStart()
{
    if (m_REG[1] & (PPU_SHOWBG | PPU_SHOWSPR)) {
        m_CurLineOft = m_ScreenOffset;
        m_CurByteIndex = m_ByteIndex;
    }
}


void RanderBottomBG(BYTE* pBit)
{
    memset(pBit, BGPal[0], 256 * 240);
}


BYTE GetScreenBGColor(int x, int y)
{
	int CurNameTable = (m_CurByteIndex >> 10) & 0x3;
	int OffsetX = (m_CurLineOft.x + x) & 0xFF;
	int TotalY = m_CurLineOft.y + y;

	WORD NameByte = PPU_MEM_BANK[CurNameTable + 8][(TotalY >> 3 << 5)  + (OffsetX >> 3)];
	BYTE * pBGPattern = PPU_MEM_BANK[(m_REG[0] & 0x10) >> 2];
	BYTE LowByte  = pBGPattern[(NameByte << 4) + (TotalY & 0x7)];
	BYTE HighByte = pBGPattern[(NameByte << 4) + 8 + (TotalY & 0x7)];
	if (!(LowByte | HighByte)) {
		return 0;
    }
	BYTE ByteBit = 7 - (OffsetX & 0x7);
	return GetBit(LowByte, ByteBit) | (GetBit(HighByte, ByteBit) << 1);
}


void  ScanHitPoint(BYTE LineNo)     
{
	int i;
	if (m_REG[2] & PPU_SPHIT_FLAG) {
		return;
    }
	int sp_h = (m_REG[0] & PPU_SP16_BIT) ? 15 : 7;		/* Sprite size */
	int dy = (int)LineNo - ((int)Sprite[0].Y + 1);
	if (dy < 0 || dy > sp_h) {
		return;
    }
	if (Sprite[0].Attribute & SP_VREVERT) {
		dy = sp_h - dy;
    }
	WORD spraddr;
	if (!(m_REG[0] & PPU_SP16_BIT)) {
		// 8x8 Sprite
		spraddr = (((WORD)m_REG[0] & PPU_SPTBL_BIT) << 9)+((WORD)Sprite[0].Index << 4);
		if (!(Sprite[0].Attribute & SP_VREVERT)) {
			spraddr += dy;
        }else {
			spraddr += 7 - dy;
        }
	}
	else {
		// 8x16 Sprite
		spraddr = (((WORD)Sprite[0].Index & 1) << 12) + (((WORD)Sprite[0].Index & 0xFE) << 4);
		if (!(Sprite[0].Attribute & SP_VREVERT)) {
			spraddr += ((dy & 8) << 1) + (dy & 7);
        }else {
			spraddr += ((~dy & 8) << 1) + (7 - (dy & 7));
        }
	}

	BYTE LowByte  = PPU_MEM_BANK[spraddr >> 10][ spraddr & 0x3FF];
	BYTE HighByte = PPU_MEM_BANK[spraddr >> 10][(spraddr & 0x3FF) + 8];

	if (Sprite[0].Attribute & SP_HREVERT) {
		LowByte  = RevertByte[LowByte];
		HighByte = RevertByte[HighByte];
	}
	
	for( i = 0; i < 8; i++) {
		if (LowByte & (1 << (7-i)) || HighByte & (1 << (7-i))) {
			if (GetScreenBGColor(Sprite[0].X + i, LineNo) & 0x03) {
				m_REG[2] |= PPU_SPHIT_FLAG;
				break;
			}
		}
	}
}


void ScanSprite(BYTE* pBit, BYTE LineNo, int bBackLevel)
{
	int i,j;
	int sp_h = (m_REG[0] & PPU_SP16_BIT) ? 15 : 7;		//精灵尺寸，7或15

	for( i = 0; i < 64; i++) {
		if (Sprite[i].Attribute & SP_LEVEL && !bBackLevel) {
			continue;
        }
		if (!(Sprite[i].Attribute & SP_LEVEL) && bBackLevel) {
			continue;
        }
		int dy = (int)LineNo - ((int)Sprite[i].Y + 1);
		if (dy != (dy & sp_h)) {
			continue;
        }
		m_CurLineSprite++;
		if (m_CurLineSprite >= 8) {
			m_REG[2] |= PPU_SPMAX_FLAG;
        }
		WORD spraddr;
		if (!(m_REG[0] & PPU_SP16_BIT)) {
		// 8x8 Sprite
			spraddr = (((WORD)m_REG[0] & PPU_SPTBL_BIT) << 9)+((WORD)Sprite[i].Index << 4);
			if (!(Sprite[i].Attribute & SP_VREVERT)) {
				spraddr += dy;
            }else {
				spraddr += 7 - dy;
            }
		}else {
		// 8x16 Sprite
			spraddr = (((WORD)Sprite[i].Index & 1) << 12) + (((WORD)Sprite[i].Index & 0xFE) << 4);
			if (!(Sprite[i].Attribute & SP_VREVERT)) {
				spraddr += ((dy & 8) << 1) + (dy & 7);
            }else {
				spraddr += ((~dy & 8) << 1) + (7 - (dy & 7));
            }
		}
		BYTE LowByte  = PPU_MEM_BANK[spraddr >> 10][ spraddr & 0x3FF];
		BYTE HighByte = PPU_MEM_BANK[spraddr >> 10][(spraddr & 0x3FF) + 8];
		if (Sprite[i].Attribute & SP_HREVERT) {
			LowByte  = RevertByte[LowByte];
			HighByte = RevertByte[HighByte];
		}
		BYTE Color;
		BYTE HighColor = (Sprite[i].Attribute & SP_HIGHCOLOR) << 2;
		for ( j = 0; j < 8; j++) {
			Color = HighColor | ((HighByte >> (7 - j) << 1) & 0x02) | ((LowByte >> (7 - j)) & 0x01);
			if (Color & 0x03 && Sprite[i].X + j <= 255) {
				pBit[LineNo* 256 + Sprite[i].X + j] = SPPal[Color];
            }
		}
	}
}


void ScanBG(BYTE* pBit, BYTE LineNo)
{
    int i,j;
	BYTE NTIndex = (m_CurByteIndex >> 10) & 0x3d;	//NameTable index
	WORD ByteIndex = ((m_CurLineOft.y + LineNo) % 240 >> 3 << 5) + (m_CurLineOft.x >> 3);
    if (LineNo + m_CurLineOft.y >= 240) {
		NTIndex ^= 0x2;
	}
	BYTE YOft = (LineNo + m_CurLineOft.y) & 0x7;
	BYTE * pBGPattern = PPU_MEM_BANK[(m_REG[0] & 0x10) >> 2];
	WORD AttrIndex = (((ByteIndex >> 7) & 0x7) << 3) + ((ByteIndex >> 2) & 0x7) + 0x3C0;

	int LoopByte = m_CurLineOft.x & 0x7 ? 33 : 32;
	for ( i = 0; i < LoopByte; i++) {
		if (i && !(ByteIndex & 0x1F)) {   //如果一行的tile号为0且i不为0
		
			AttrIndex &= 0xFFF8;
			ByteIndex -=32;
			NTIndex ^= 1;
		}
		else if (i && !(ByteIndex & 0x3)) {
			AttrIndex++;
		}
		int AttrBitOft = (ByteIndex >> 5) & 0x2;
		BYTE ByteLow  = pBGPattern[PPU_MEM_BANK[NTIndex + 8][ByteIndex] * 16 + YOft];
		BYTE ByteHigh = pBGPattern[PPU_MEM_BANK[NTIndex + 8][ByteIndex] * 16 + YOft + 8];
		if (ByteLow | ByteHigh) {
			AttrBitOft |= (ByteIndex & 0x2) >> 1;
			BYTE ByteAttr = PPU_MEM_BANK[NTIndex + 8][AttrIndex];
			BYTE AttrBit  = (BYTE)(((ByteAttr >> (AttrBitOft * 2)) & 0x3) << 2);
			for ( j = 0; j < 8; j++) {
				BYTE ColorIndex = ((ByteHigh >> (7 - j)) & 0x1) << 1;
				ColorIndex |= ((ByteLow >> (7 - j)) & 0x1);
				if (ColorIndex) {
					ColorIndex |= AttrBit;
					int xoft = i * 8 + j - (m_CurLineOft.x & 0x7);
					if (xoft >= 0 && xoft <= 255) {
						pBit[LineNo * 256 + (i * 8 + j - (m_CurLineOft.x & 0x7))] = BGPal[ColorIndex];
                    }
				}
			}
		}
		ByteIndex++;
	}
}


void ScanLine(BYTE* pBit, int LineNo)
{
    m_REG[2] &= ~PPU_SPMAX_FLAG;
    m_CurLineSprite = 0;

    if (LineNo < 240 && m_REG[1] & PPU_SHOWSPR) {
        ScanSprite(pBit, LineNo, TRUE); 
    }
    if (LineNo < 240 && m_REG[1] & PPU_SHOWBG) {
        ScanBG(pBit, LineNo);            
    }
    if (LineNo < 240 && m_REG[1] & PPU_SHOWSPR) {
        ScanSprite(pBit, LineNo, FALSE);     
        ScanHitPoint(LineNo);            
    }
}


void  VBlankStart()
{
    m_REG[2] |= PPU_VBLANK_FLAG;
    if (m_REG[0] & PPU_VBLANK_BIT) {
         NMI();
    }
}


void  VBlankEnd()
{
    m_REG[2] &= ~PPU_VBLANK_FLAG;
    m_REG[2] &= ~PPU_SPHIT_FLAG;
}
