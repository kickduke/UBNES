FILE *SL=NULL;

void Save(int saveno)
{	
	char *savefilename;
	switch(saveno) {
		case 1:  savefilename="/home/kqs/Project/UBNES/save/mxt.sav"; break;
		case 2:  savefilename="/home/kqs/Project/UBNES/save/boom.sav"; break;
		case 3:  savefilename="/home/kqs/Project/UBNES/save/mario.sav"; break;
		case 4:  savefilename="/home/kqs/Project/UBNES/save/hdl.sav"; break;
		case 5:  savefilename="/home/kqs/Project/UBNES/save/slms.sav"; break;
	}
	SL=fopen(savefilename,"rb+");
	fwrite("SAV",sizeof(BYTE),3,SL);   
	fwrite(&A ,sizeof(BYTE) ,1 , SL);
	fwrite( &X,sizeof(BYTE) ,1 , SL);
	fwrite( &Y,sizeof(BYTE) ,1 , SL);
	fwrite( &PC,sizeof(BYTE) , 2, SL);
	fwrite(&S ,sizeof(BYTE) , 1, SL);
	fwrite(&P ,sizeof(BYTE) , 1, SL);
	fwrite(&INT_pending ,sizeof(BYTE) , 1, SL);
	fwrite(&m_BaseCycle ,sizeof(BYTE) , 8, SL);
	fwrite( &m_EmuCycle,sizeof(BYTE) , 8, SL);
	fwrite(&m_DMACycle ,sizeof(BYTE) , 4, SL);
	if(SCREENOFFSET_AtX(&m_ScreenOffset)) {
	    fwrite("1", sizeof(BYTE), 1, SL);
    }else {
        fwrite("0", sizeof(BYTE), 1, SL);
    }
	fwrite(&(m_ScreenOffset.x) ,sizeof(BYTE), 1, SL);
	fwrite(&(m_ScreenOffset.y),sizeof(BYTE), 1, SL);
	if(ADDRESS_AtLow(&m_Address)) {
        fwrite("1", sizeof(BYTE), 1, SL);
    }else {
        fwrite("0" ,sizeof(BYTE), 1, SL);
    }
	fwrite(&(m_Address.LowAddr) ,sizeof(BYTE) , 1, SL);
	fwrite(&(m_Address.HeightAddr),sizeof(BYTE) , 1, SL);
	fwrite(m_REG,sizeof(BYTE) , 4, SL);
	fwrite(RAM,sizeof(BYTE) , 0x2000, SL);
	fwrite(SRAM,sizeof(BYTE) , 0x2000, SL);
	fwrite(VRAM ,sizeof(BYTE) ,0x1000 , SL);
	fwrite(BGPal ,sizeof(BYTE) ,0x10 , SL);
	fwrite(SPPal ,sizeof(BYTE) ,0x10 , SL);
	fwrite(SPRAM,sizeof(BYTE) , 0x100, SL);
	fclose(SL);
}


void Load(int loadno)
{    
	char *loadfilename;
	char c[3]={0};
	int m,n;
	switch(loadno) {
		case 1:  loadfilename="/home/kqs/Project/UBNES/save/mxt.sav"; break;
		case 2:  loadfilename="/home/kqs/Project/UBNES/save/boom.sav"; break;
		case 3:  loadfilename="/home/kqs/Project/UBNES/save/mario.sav"; break;
		case 4:  loadfilename="/home/kqs/Project/UBNES/save/hdl.sav"; break;
		case 5:  loadfilename="/home/kqs/Project/UBNES/save/slms.sav"; break;
	}
	SL=fopen(loadfilename,"rb");
	fread(c,sizeof(BYTE),3,SL);  
	fread(&A ,sizeof(BYTE) ,1 , SL);
	fread( &X,sizeof(BYTE) ,1 , SL);
	fread( &Y,sizeof(BYTE) ,1 , SL);
	fread( &PC,sizeof(BYTE) , 2, SL);
	fread(&S ,sizeof(BYTE) , 1, SL);
	fread(&P ,sizeof(BYTE) , 1, SL);
	fread(&INT_pending ,sizeof(BYTE) , 1, SL);
	fread(&m_BaseCycle ,sizeof(BYTE) , 8, SL);
	fread( &m_EmuCycle,sizeof(BYTE) , 8, SL);
	fread(&m_DMACycle ,sizeof(BYTE) , 4, SL);
	fread( &m,sizeof(BYTE) ,1 , SL);
	if(m==48) {
        SCREENOFFSET_ResetY(&m_ScreenOffset);
    }else {
        SCREENOFFSET_Reset(&m_ScreenOffset);
    }
	fread(&n ,sizeof(BYTE) , 1, SL);
	(m_ScreenOffset.x)=n;
	fread(&n,sizeof(BYTE) , 1, SL);
	(m_ScreenOffset.y)=n;
	fread( &m,sizeof(BYTE) , 1, SL);
	if(m==48) {
        ADDRESS_Reset(&m_Address);
    }else {
        ADDRESS_ResetL(&m_Address);
    }
	fread(&n ,sizeof(BYTE) , 1, SL);
	(m_Address.LowAddr)=n;
	fread( &n,sizeof(BYTE) , 1, SL);
	(m_Address.HeightAddr)=n;
	fread( m_REG,sizeof(BYTE) , 4, SL);
	fread( RAM,sizeof(BYTE) , 0x2000, SL);
	fread( SRAM,sizeof(BYTE) , 0x2000, SL);
	fread(VRAM ,sizeof(BYTE) ,0x1000 , SL);
	fread(BGPal ,sizeof(BYTE) ,0x10 , SL);
	fread(SPPal ,sizeof(BYTE) ,0x10 , SL);
	fread( SPRAM,sizeof(BYTE) , 0x100, SL); 
	fclose(SL);
}
