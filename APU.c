BYTE APURead(WORD addr)
{
    BYTE data = 0;
    if(addr == 0x4017) {
        data = (1 << 6);
    }
    return data;
}
