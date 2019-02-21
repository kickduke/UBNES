typedef struct tagScreenOffset
{
	//void Reset();
	//void ResetY();
	//void SetValue(BYTE val);
	//int AtX();
	BYTE* current; 
    BYTE x;
	BYTE y;
} SCREENOFFSET;

SCREENOFFSET m_ScreenOffset = {&m_ScreenOffset.x, 0, 0};
SCREENOFFSET m_CurLineOft = {&m_CurLineOft.x, 0, 0};

void SCREENOFFSET_Reset(SCREENOFFSET* a_screenoffset)
{
    a_screenoffset->current = &a_screenoffset->x;
}

void SCREENOFFSET_ResetY(SCREENOFFSET* a_screenoffset)
{
    a_screenoffset->current = &a_screenoffset->y;
}

void SCREENOFFSET_SetValue(SCREENOFFSET* a_screenoffset, BYTE val)
{
    *a_screenoffset->current = val;
    if(a_screenoffset->current == &a_screenoffset->x) {
        a_screenoffset->current = &a_screenoffset->y;
    }else {
        a_screenoffset->current = &a_screenoffset->x;
    }
}

int SCREENOFFSET_AtX(SCREENOFFSET* a_screenoffset)
{
    return (a_screenoffset->current == &a_screenoffset->x);
}


typedef struct tagADDRESS
{
	//tagADDRESS();
	//void Reset();
	//void ResetL();
	//void SetAddress(BYTE val);
	//WORD GetAddress();
	//void Step(int add);
	//int AtLow();
	BYTE *current;
    BYTE LowAddr;
	BYTE HeightAddr;
} ADDRESS;

ADDRESS m_Address = {&m_Address.HeightAddr, 0, 0};

void ADDRESS_Reset(ADDRESS* a_address)
{
    a_address->current = &a_address->HeightAddr;
}

void ADDRESS_ResetL(ADDRESS* a_address) 
{
    a_address->current = &a_address->LowAddr;
}

void ADDRESS_SetAddress(ADDRESS* a_address, BYTE val)
{
    *a_address->current = val;
    if(a_address->current == &a_address->HeightAddr) {
        a_address->current = &a_address->LowAddr;
    }else {
        a_address->current = &a_address->HeightAddr;
    }
}

WORD ADDRESS_GetAddress(ADDRESS* a_address)
{
    return *(WORD*)&a_address->LowAddr;
}

void ADDRESS_Step(ADDRESS* a_address, int add)
{
    *(WORD*)&a_address->LowAddr = *(WORD*)&a_address->LowAddr + add;
}

int ADDRESS_AtLow(ADDRESS* a_address)
{
    return (a_address->current == &a_address->LowAddr);
}
