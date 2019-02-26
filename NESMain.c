#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <SDL/SDL.h> 
#include "CPUINC.h"
#include "INC.h"
#include "APU.h"
#include "PPU.h"
#include "Joypad.h"
#include "NES.h"
#include "Mapper.h"
#include "CPU.h"
#include "Graph.h"
#include "SaveLoad.h"

SDL_Surface *screen = NULL; 
SDL_Surface *backpic = NULL;
SDL_Surface *cartpic = NULL;

int main(int argc, char* argv[])
{         
    int pixelx = 0;        //临时变量用于绘制屏幕
    int pixely = 0;
    int done = 0;          //主循环控制
    int cart = 0;          //0:主界面   1：游戏选择界面
	int romno = -1;
	float speed = 1;
	DWORD *rgb = rgbQuard;      //from Graph.c    ？？？
    char *romname = "/home/kqs/Project/UBNES/ROM/test.nes";      //ROM文件路径
    long FrameStartTime;        //用于控制帧率
    long SleepTime;             //用于控制帧率
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);          //注册终止函数，相当于程序退出时执行SDL_Quit() 
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("NES Simulator for Ubuntu", NULL);
    SDL_WM_SetIcon(load_image("/home/kqs/Project/UBNES/res/icon1.bmp"),NULL);
    cartpic = load_image("/home/kqs/Project/UBNES/res/cart.bmp");     //游戏选择背景图片
    backpic = load_image("/home/kqs/Project/UBNES/res/back.bmp" );    //主界面背景图片
    double FramePeriod = 1000.0 / (NesCfg->FrameRate);       //from NES.c  NesCfg指向配置信息数据，默认NTSC制式
    ScreenBit = (BYTE*)malloc(sizeof(BYTE)*256*240);         //该内存区域存放屏幕显示，分辨率256X240

//游戏重新载入运行入口
Re: apply_surface(0, 0, backpic, screen);    //from Graph.c   绘制主界面背景图片
    InitJoypad();                            //from Joypad.c  手柄初始化   ？？？
    NES_Init();                              //from NES.c     初始化  ？？？
    NES_LoadRom(romname);                    //from NES.c     加载ROM文件、获取ROM信息并进行初步的CPU、PPU逻辑地址映射
	CreateMapper(MapperNo);                  //
    NES_Start(); 
      
    while(!done) {    
        FrameStartTime = SDL_GetTicks();
        NES_FrameExec();
        if ( SDL_MUSTLOCK(screen) ) {
            if (SDL_LockSurface(screen) < 0) {
                return -1;
            } 
        } 
        for(pixely=0;pixely<240;pixely++) {
            for(pixelx=0;pixelx<255;pixelx++) {
                if(cart==0) {
                    DrawPixel(screen,rgb[ScreenBit[pixely*256+pixelx]],274+pixelx,105+pixely); 
                }
            }
        }
        if(SDL_MUSTLOCK(screen)) {
            SDL_UnlockSurface(screen);  
        }
        SDL_UpdateRect(screen, 0, 0, 640,480);          
        SDL_Event event;     
        while (SDL_PollEvent (&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_w: SetState(JOY_PAD_1, JOY_PAD_UP, 1);break;
                    case SDLK_s: SetState(JOY_PAD_1, JOY_PAD_DOWN, 1);break;
                    case SDLK_a: SetState(JOY_PAD_1, JOY_PAD_LEFT, 1);break;
                    case SDLK_d: SetState(JOY_PAD_1, JOY_PAD_RIGHT, 1);break;                   
                    case SDLK_u: SetState(JOY_PAD_1, JOY_PAD_B, 1);break;
                    case SDLK_i: SetState(JOY_PAD_1, JOY_PAD_A, 1);break;
                    case SDLK_j: SetState(JOY_PAD_1, JOY_PAD_SELECT, 1);break;
                    case SDLK_k: SetState(JOY_PAD_1, JOY_PAD_START, 1);break;			               
			        case SDLK_q: Save(romno); break;
                }
                break;
            case SDL_KEYUP:
                switch(event.key.keysym.sym) {
                    case SDLK_w: SetState(JOY_PAD_1, JOY_PAD_UP, 0);break;
                    case SDLK_s: SetState(JOY_PAD_1, JOY_PAD_DOWN, 0);break;
                    case SDLK_a: SetState(JOY_PAD_1, JOY_PAD_LEFT, 0);break;
                    case SDLK_d: SetState(JOY_PAD_1, JOY_PAD_RIGHT, 0);break;                   
                    case SDLK_u: SetState(JOY_PAD_1, JOY_PAD_B, 0);break;
                    case SDLK_i: SetState(JOY_PAD_1, JOY_PAD_A, 0);break;
                    case SDLK_j: SetState(JOY_PAD_1, JOY_PAD_SELECT, 0);break;
                    case SDLK_k: SetState(JOY_PAD_1, JOY_PAD_START, 0); break;
                    case SDLK_e: Load(romno);break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button==SDL_BUTTON_LEFT) {
                    if(event.button.x<48 && event.button.y<48 && cart==0) {
                        apply_surface( 0,0, cartpic, screen ); 
                        cart=1;  
                        break; 
                    }else if(event.button.x<160 &&event.button.x>80 && event.button.y<132 && event.button.y>98 && cart==1) {
                         romname="/home/kqs/Project/UBNES/ROM/mxt.nes"; romno=1;cart=0; 
                         goto Re;    
                    }else if(event.button.x<160 &&event.button.x>80 && event.button.y<188 && event.button.y>157 && cart==1) {
                         romname="/home/kqs/Project/UBNES/ROM/boom.nes"; romno=2;cart=0; 
                         goto Re;    
                    }else if(event.button.x<160 &&event.button.x>80 && event.button.y<243 && event.button.y>211 && cart==1) {
                         romname="/home/kqs/Project/UBNES/ROM/mario.nes"; romno=3;cart=0; 
                         goto Re;    
                    }else if(event.button.x<160 &&event.button.x>80 && event.button.y<295 && event.button.y>263 && cart==1) {
                         romname="/home/kqs/Project/UBNES/ROM/hdl.nes";romno=4; cart=0; 
                         goto Re;    
                    }else if(event.button.x<160 &&event.button.x>80 && event.button.y<347 && event.button.y>315 && cart==1) {
                         romname="/home/kqs/Project/UBNES/ROM/slms.nes"; romno=5;cart=0; 
                         goto Re;    
                    }else if(event.button.x<137 && event.button.x>105 && event.button.y>440 && cart==0) {
                         speed=2.0;
                    }else if(event.button.x<179 && event.button.x>147 && event.button.y>440 && cart==0) {
                         speed=1.0;
                    }else if(event.button.x<220 && event.button.x>188 && event.button.y>440 && cart==0) {
                         speed=0.01;
                    }else if(event.button.x<48 && event.button.y>432 && cart==0) {
                         rgb=rgbQuard;  
                         backpic=load_image("/home/kqs/Project/UBNES/res/back.bmp");apply_surface( 0,0, backpic, screen );
                    }else if(event.button.x<96 && event.button.x>48 && event.button.y>432 && cart==0) {
                         rgb=rgbSP;  
                         backpic=load_image("/home/kqs/Project/UBNES/res/backsp.bmp");apply_surface( 0,0, backpic, screen );  
                    }
                }
                break;                                                          
            case SDL_QUIT:
		        NES_Stop();
                done = 1;
                break;
            default:
                break;
            }
        }      
        SleepTime = (long)FramePeriod - ((long)SDL_GetTicks() - FrameStartTime);    
		if(SleepTime > 0) {
			SDL_Delay((long)SleepTime*speed); 
        }else {
		    SDL_Delay(0);
        }
      }
    SDL_Quit();
    return 0;           
}
