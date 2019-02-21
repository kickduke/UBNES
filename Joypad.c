int m_bStrobe;
int m_CurIndex[2];
DWORD m_PadBit[2];


void InitJoypad()
{
    m_bStrobe = FALSE;
    memset(m_CurIndex, 0, sizeof(m_CurIndex));
    memset(m_PadBit, 0, sizeof(m_PadBit));
}


void SetState(int pad, int index, BYTE val)
{
    DWORD * PadBit = &m_PadBit[pad & 0x1];
    DWORD mask = index;    
    if (val) {
        *PadBit |= mask; 
    }else {
        *PadBit &= ~mask;
    }
}


void InputBurst(BYTE burst)   
{
    if (burst & 0x1 && m_bStrobe == FALSE) {
        m_bStrobe = TRUE;
    }else if (!(burst & 0x1) && m_bStrobe) {
        m_bStrobe = FALSE;
        m_CurIndex[0] = m_CurIndex[1] = 0;
    }
}


BYTE GetValue(int padbit)
{
    BYTE ret = (BYTE)((m_PadBit[padbit] >> m_CurIndex[padbit]) & 0x1);
    m_CurIndex[padbit]++;
    if (m_CurIndex[padbit] >= 24) {
         memset(m_CurIndex, 0, sizeof(m_CurIndex));
    }
    return ret;
}
