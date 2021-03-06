DWORD rgbQuard[64] = 
{  
	0x7F7F7F,  0x2000B0,  0x2800B8,  0x6010A0,
	0x982078,  0xB01030,  0xA03000,  0x784000,
	0x485800,  0x386800,  0x386C00,  0x306040,
	0x305080,  0x000000,  0x000000,  0x000000,
	0xBCBCBC,  0x4060F8,  0x4040FF,  0x9040F0,
	0xD840C0,  0xD84060,  0xE05000,  0xC07000,
	0x888800,  0x50A000,  0x48A810,  0x48A068,
	0x4090C0,  0x000000,  0x000000,  0x000000,
	0xFFFFFF,  0x60A0FF,  0x5080FF,  0xA070FF,
	0xF060FF,  0xFF60B0,  0xFF7830,  0xFFA000,
	0xE8D020,  0x98E800,  0x70F040,  0x70E090,
	0x60D0E0,  0x606060,  0x000000,  0x000000,
	0xFFFFFF,  0x90D0FF,  0xA0B8FF,  0xC0B0FF,
	0xE0B0FF,  0xFFB8E8,  0xFFC8B8,  0xFFD8A0,
	0xFFF090,  0xC8F080,  0xA0F0A0,  0xA0FFC8,
	0xA0FFF0,  0xA0A0A0,  0x000000,  0x000000
};
DWORD rgbSP[64] = 
{
    0x7f7f7f,  0x585858,  0x5c5c5c,  0x585858,  
    0x5c5c5c,  0x606060,  0x505050,  0x3c3c3c,  
	0x2c2c2c,  0x343434,  0x363636,  0x484848, 
	0x585858,  0x000000,  0x000000,  0x000000,  
    0xbcbcbc,  0x9c9c9c,  0x9f9f9f,  0x989898,
    0x8c8c8c,  0x8c8c8c,  0x707070,  0x606060,
    0x444444,  0x505050,  0x5c5c5c,  0x747474,
    0x808080,  0x000000,  0x000000,  0x000000,
    0xffffff,  0xafafaf,  0xa7a7a7,  0xb7b7b7,
    0xafafaf,  0xafafaf,  0x979797,  0x7f7f7f,
    0x848484,  0x747474,  0x989898,  0xa8a8a8,
    0xa0a0a0,  0x606060,  0x000000,  0x000000,
    0xffffff,  0xc7c7c7,  0xcfcfcf,  0xd7d7d7,
    0xd7d7d7,  0xdbdbdb,  0xdbdbdb,  0xcfcfcf,
    0xc7c7c7,  0xb8b8b8,  0xc8c8c8,  0xcfcfcf,  
	0xcfcfcf,  0xa0a0a0,  0x000000,  0x000000
};

//绘制像素
void DrawPixel(SDL_Surface *lscreen, DWORD ColorCode, int x, int y)
{     
    Uint8 a = ColorCode >> 16;              
	Uint8 b = (ColorCode >> 8) & 0x0FF;
	Uint8 c = ColorCode & 0x0FF;  
    Uint32 color = SDL_MapRGB(lscreen->format, a, b, c);
    //Uint32 *bufp;
    //bufp = (Uint32 *)lscreen->pixels + y*lscreen->pitch/4 + x;
    //*bufp = color;
    Uint32 *pixel = (Uint32*)lscreen->pixels;
    int pos = y * lscreen->pitch / 4 + x;
    pixel[pos] = color;
}

//加载图片
SDL_Surface *load_image(char *filename) 
{
    SDL_Surface *loadedImage = NULL;
    SDL_Surface *optimizedImage = NULL;
    loadedImage = SDL_LoadBMP(filename);
    if(loadedImage != NULL) {
        optimizedImage = SDL_DisplayFormat(loadedImage);
        SDL_FreeSurface(loadedImage);
    }
    return optimizedImage;
}

//surface粘贴
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface(source, NULL, destination, &offset);
}
