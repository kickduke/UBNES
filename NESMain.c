/*编译：   
 *gcc -o UBNES NESMain.c -lSDL -lSDL_ttf -lSDL_draw -lsoundio -lm
 * */
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <soundio/soundio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_draw.h>
#include "CPUINC.h"
#include "INC.h"
#include "PPU.h"
#include "Joypad.h"
#include "NES.h"
#include "Mapper.h"
#include "CPU.h"
#include "APU.h"
#include "Graph.h"
#include "SaveLoad.h"

SDL_Surface *screen = NULL; 
SDL_Surface *backpic = NULL;
SDL_Surface *cartpic = NULL;
SDL_Surface *gamelist = NULL;
SDL_Surface *reglist = NULL;
SDL_Surface *mouse = NULL;
TTF_Font *font = NULL;
TTF_Font *font_mid = NULL;
TTF_Font *font_small = NULL;
char byte_color[8] = "";   //绘制pattern_table用
char reg_bin_list[9];     //用于显示register的二进制
/*FILE *fp_apu_tri_log = NULL;      //用于记录APU三角波
FILE *fp_apu_squa1_log = NULL;    //用于记录APU方波1
FILE *fp_apu_squa2_log = NULL;    //用于记录APU方波2
FILE *fp_apu_noise_log = NULL;    //用于记录NOISE*/
int log_seq = 0;  //用于log文件计数

//函数声明
void render_static_elements();
void render_dynamic_elements();
void process_pattern_color(char* a_byte_color, BYTE a_byte_n, BYTE a_byte_n8);
char* byte_to_binary(char* a_bin_list, short val);

int main(int argc, char* argv[])
{         
    int pixelx = 0;        //临时变量用于绘制屏幕
    int pixely = 0;
    int done = 0;          //主循环控制
    //int cart = 0;          //0:主界面   1：游戏选择界面
	int romno = -1;
	float speed = 1;
	DWORD *rgb = rgbQuard;      //from Graph.c    ？？？
    char *romname = "/home/kqs/Project/UBNES/ROM/test.nes";      //ROM文件路径
    long FrameStartTime;        //用于控制帧率
    long SleepTime;             //用于控制帧率
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);          //注册终止函数，相当于程序退出时执行SDL_Quit() 
    screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
    SDL_WM_SetCaption("NES Simulator for Ubuntu", NULL);
    SDL_WM_SetIcon(load_image("/home/kqs/Project/UBNES/res/icon1.bmp"),NULL);
    //cartpic = load_image("/home/kqs/Project/UBNES/res/cart.bmp");     //游戏选择背景图片
    //backpic = load_image("/home/kqs/Project/UBNES/res/back.bmp" );    //主界面背景图片
    double FramePeriod = 1000.0 / (NesCfg->FrameRate);       //from NES.c  NesCfg指向配置信息数据，默认NTSC制式
    ScreenBit = (BYTE*)malloc(sizeof(BYTE)*256*240);         //该内存区域存放屏幕显示，分辨率256X240
    Uint32 c_white = SDL_MapRGB(screen->format,255,255,255);
    SDL_Color c_font1 = {120,220,120};
    SDL_Color c_font2 = {150,120,170};
    SDL_Color c_font3 = {210,20,70};
    //字体初始化
    TTF_Init();
    font = TTF_OpenFont("/home/kqs/Project/UBNES/res/YaHeiConsolas.ttf",18);
    font_mid = TTF_OpenFont("/home/kqs/Project/UBNES/res/YaHeiConsolas.ttf",16);
    font_small = TTF_OpenFont("/home/kqs/Project/UBNES/res/YaHeiConsolas.ttf",12);
    int mouse_x = 0;
    int mouse_y = 0;
    char mouse_pos[20] = "";
    WORD tri_wave_len = 0;
    WORD squa1_wave_len = 0;
    WORD squa2_wave_len = 0;
    BYTE noise_len_idx = 0;
    BYTE squa1_vol = 0;
    BYTE squa2_vol = 0;
    BYTE noise_vol = 0;
    BYTE squa1_duty = 0;
    BYTE squa2_duty = 0;
    /*int write_log_flag = 0;    //若为1,记录APU相关数据到日志文件
    fp_apu_tri_log = fopen("log/APU_TRI_LOG.txt", "w");
    fp_apu_squa1_log = fopen("log/APU_SQUA1_LOG.txt", "w");
    fp_apu_squa2_log = fopen("log/APU_SQUA2_LOG.txt", "w");
    fp_apu_noise_log = fopen("log/APU_NOISE_LOG.txt", "w");
    fp_callback_log = fopen("log/CALLBACK.txt", "w");*/
    //声音初始化
    struct SoundIo *soundio = soundio_create();
    soundio_connect(soundio);
    soundio_flush_events(soundio);
    int default_out_device_index = soundio_default_output_device_index(soundio);
    struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
    struct SoundIoOutStream *outstream = soundio_outstream_create(device);
    //outstream->format = SoundIoFormatFloat32NE;    
    outstream->format = SoundIoFormatS16LE;   
    fprintf(stderr, "support:%d\n", soundio_device_supports_format(device, SoundIoFormatS16LE ));
    outstream->write_callback = write_callback;
    outstream->underflow_callback = underflow_callback;
    soundio_outstream_open(outstream);
    outstream->software_latency = (double)(0.001);   
    //测试
    fprintf(stderr, "Output device: %s\n", device->name);
    fprintf(stderr, "sample_rate:%d\n", outstream->sample_rate);
    fprintf(stderr, "bytes_per_frame:%d\n", outstream->bytes_per_frame);
    fprintf(stderr, "channel_count:%d\n", outstream->layout.channel_count);
    fprintf(stderr, "latency:%f\n", outstream->software_latency);
    //
    if (outstream->layout_error) {
        fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));
    }
    soundio_outstream_start(outstream);


//游戏重新载入运行入口
Re: apply_surface(0, 0, backpic, screen);    //from Graph.c   绘制主界面背景图片
    InitJoypad();                            //from Joypad.c  手柄初始化   ？？？
    NES_Init();                              //from NES.c     初始化  ？？？
    NES_LoadRom(romname);                    //
	CreateMapper(MapperNo);                  //
    NES_Start(); 
      
    while(!done) {    
        FrameStartTime = SDL_GetTicks();
        NES_FrameExec();
        //窗口填充黑色
        SDL_FillRect(screen,&screen->clip_rect,SDL_MapRGB(screen->format,0,0,0));
        render_static_elements();
        render_dynamic_elements();
        if ( SDL_MUSTLOCK(screen) ) {
            if (SDL_LockSurface(screen) < 0 ) {
                return 0;
            } 
        } 
        for(pixely=0;pixely<240;pixely++) {
            for(pixelx=0;pixelx<255;pixelx++) {
                DrawPixel(screen,rgb[ScreenBit[pixely*256+pixelx]],3+pixelx,3+pixely); 
            }
        }
        if(mouse != NULL){
            apply_surface(1750, 0, mouse, screen);
        }
        if(SDL_MUSTLOCK(screen)) {
            SDL_UnlockSurface(screen);  
        }
        SDL_UpdateRect(screen, 0, 0, 1920,1080);    
        //SDL事件处理
        SDL_Event event;     
        while(SDL_PollEvent (&event)) {
            switch(event.type) {
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
                if(event.button.button == SDL_BUTTON_LEFT){ 
                    if(event.button.x < 300 + 200 && event.button.x > 300 && event.button.y < 10 + 18 && event.button.y > 10){
                         romname="/home/kqs/Project/UBNES/ROM/mxt.nes"; 
                         romno = 1;
                         goto Re;    
                    }else if(event.button.x < 300 + 200 && event.button.x > 300 && event.button.y < 50 + 18 && event.button.y > 50){
                         romname="/home/kqs/Project/UBNES/ROM/lsbt.nes"; 
                         romno = 2; 
                         goto Re;    
                    }else if(event.button.x < 300 + 200 && event.button.x > 300 && event.button.y < 90 + 18 && event.button.y > 90) {
                         romname="/home/kqs/Project/UBNES/ROM/mario.nes"; 
                         romno = 3;
                         goto Re;    
                    }else if(event.button.x < 300 + 200 && event.button.x > 300 && event.button.y < 130 + 18 && event.button.y > 130) {
                         romname="/home/kqs/Project/UBNES/ROM/hdl.nes";
                         romno = 4; 
                         goto Re;    
                    }else if(event.button.x < 300 + 200 && event.button.x > 300 && event.button.y < 170 + 18 && event.button.y > 170) {
                         romname="/home/kqs/Project/UBNES/ROM/slms.nes"; 
                         romno = 5;
                         goto Re;
                    }else if(event.button.x > 300 && event.button.x < 300+35 && event.button.y > 210 && event.button.y < 210+18) {
                         rgb = rgbQuard;
                    }else if(event.button.x > 350 && event.button.x < 350+35 && event.button.y > 210 && event.button.y < 210+18) { 
                         rgb = rgbSP;    
                    }else if(event.button.x > 400 && event.button.x < 400+40 && event.button.y > 210 && event.button.y < 210+18) { 
                         speed=0.01;
                    }else if(event.button.x > 450 && event.button.x < 450+40 && event.button.y > 210 && event.button.y < 210+18) {
                         speed=1.0;
                    }else if(event.button.x > 500 && event.button.x < 500+40 && event.button.y > 210 && event.button.y < 210+18) {
                         speed=5.0;
                    }
                }
                break;  
            case SDL_MOUSEMOTION:
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                sprintf(mouse_pos,"(%d,%d)",mouse_x,mouse_y);
                mouse = TTF_RenderUTF8_Solid(font,mouse_pos,c_font3);
                break;
            case SDL_QUIT:
		        NES_Stop();
                done = 1;
                break;
            default:
                break;
            }
        }
        /*/记录APU输出
        ////三角波
        if(write_log_flag == 1) {
            tri_wave_len = (0x00 | apu_reg[10]) | ((0x07 & apu_reg[11]) << 8); 
            fprintf(fp_apu_tri_log, "%d_%d:%04X:%d\n", second_cnt, frame_cnt, tri_wave_len, tri_wave_len);
            ////方波
            squa1_wave_len = (0x00 | apu_reg[2]) | ((0x07 & apu_reg[3]) << 8);
            squa1_duty = (0xC0 & apu_reg[0]) >> 6;
            squa1_vol = 0x0F & apu_reg[0];
            fprintf(fp_apu_squa1_log, "%d_%d:%04X:%d:%d:%d\n", second_cnt, frame_cnt, squa1_wave_len, squa1_wave_len, squa1_vol, squa1_duty);
            squa2_wave_len = (0x00 | apu_reg[6]) | ((0x07 & apu_reg[7]) << 8);
            squa2_duty = (0xC0 & apu_reg[4]) >> 6;
            squa2_vol = 0x0F & apu_reg[4];
            fprintf(fp_apu_squa2_log, "%d_%d:%04X:%d:%d:%d\n", second_cnt, frame_cnt, squa2_wave_len, squa2_wave_len, squa2_vol, squa2_duty);
            ////NOISE
            noise_vol = 0x0F & apu_reg[12];
            noise_len_idx = 0x0F & apu_reg[14];
            fprintf(fp_apu_noise_log, "%d_%d:%04X:%d:%d\n", second_cnt, frame_cnt, noise_len_idx, noise_len_idx, noise_vol);
            frame_cnt = frame_cnt%60 + 1;
            if(frame_cnt == 1) {
                second_cnt++;
            }
        }*/
        soundio_flush_events(soundio);
        SleepTime = (long)FramePeriod - ((long)SDL_GetTicks() - FrameStartTime);    
		if(SleepTime > 0) {
			SDL_Delay((long)SleepTime*speed); 
        }else {
		    SDL_Delay(0);
        }
    }
    SDL_FreeSurface(screen);
    SDL_FreeSurface(backpic);
    SDL_FreeSurface(cartpic);
    SDL_FreeSurface(gamelist);
    SDL_FreeSurface(reglist);
    SDL_FreeSurface(mouse);
    TTF_CloseFont(font);
    TTF_CloseFont(font_mid);
    TTF_CloseFont(font_small);
    TTF_Quit();
    SDL_Quit();
    /*fclose(fp_apu_tri_log);
    fclose(fp_apu_squa1_log);
    fclose(fp_apu_squa2_log);
    fclose(fp_callback_log);*/
    soundio_outstream_destroy(outstream);
    soundio_device_unref(device);
    soundio_destroy(soundio);
    return 0;           
}


//动态界面绘制
void render_dynamic_elements()
{
    Uint32 c_pattern0 = SDL_MapRGB(screen->format,240, 240, 240);
    Uint32 c_pattern1 = SDL_MapRGB(screen->format,254, 80, 33);
    Uint32 c_pattern2 = SDL_MapRGB(screen->format,72, 162, 82);
    Uint32 c_pattern3 = SDL_MapRGB(screen->format,251, 205, 59);
    Uint32 c_pattern  = SDL_MapRGB(screen->format,200, 0, 0);
    SDL_Color c_fontc = {59,171,104};
    int tile = 0;
    int line = 0;
    char regstring[50] = "";
    WORD pattern_address = 0;
    //printf("N=%x N+8=%x\n",PPU_MEM_BANK[0 >> 10][0 & 0x3FF],PPU_MEM_BANK[8 >> 10][8 & 0x3FF]);
    //printf("%s\n",process_pattern_color(byte_color, 19, 25));
    for(tile = 0; tile < 768; tile++) {
        for(line = 0; line < 8; line++) {
            pattern_address = tile*16 + line;
            process_pattern_color(byte_color, PPU_MEM_BANK[pattern_address >> 10][pattern_address & 0x3FF],
                                  PPU_MEM_BANK[(pattern_address + 8) >> 10][(pattern_address + 8) & 0x3FF]);
            //printf("%s\n",byte_color);
            for(int n = 0; n < 8; n++) {
                if(byte_color[n] == '0') {
                    c_pattern = c_pattern0;
                }
                if(byte_color[n] == '1') {
                    c_pattern = c_pattern1;
                }
                if(byte_color[n] == '2') {
                    c_pattern = c_pattern2;
                }
                if(byte_color[n] == '3') {
                    c_pattern = c_pattern3;
                }
                //Draw_Pixel(screen, 940 + tile*10 + n, 20 + line, c_pattern);  
                //Draw_FillRect(screen, 940 + (tile%16)*35 + n*4, 20 + (tile/16)*35 + line*4, 4, 4, c_pattern);
                Draw_FillRect(screen, 1170 + (tile%32)*18 + n*2, 20 + (tile/32)*18 + line*2, 2, 2, c_pattern);
            }            
        }
    }
    //BGPal 背景调色板
    for(int a = 0; a < 16; a++) {
        //printf("BGPal%x=%x\n", a + 0x3F00, BGPal[a]);
        Draw_FillRect(screen, 940+a%4*49, 40+22*(a/4), 45, 18 ,
        SDL_MapRGB(screen->format,(rgbQuard[BGPal[a]] & 0xFF0000) >> 16,
                   (rgbQuard[BGPal[a]] & 0x00FF00) >> 8, rgbQuard[BGPal[a]] & 0x0000FF));
    }
    //SPPal 精灵调色板
    for(int a = 0; a < 16; a++) {
        //printf("SPPal%x=%x\n", a, SPPal[a]);
        Draw_FillRect(screen, 940+a%4*49, 175+22*(a/4), 45, 18 ,
        SDL_MapRGB(screen->format,(rgbQuard[SPPal[a]] & 0xFF0000) >> 16,
                   (rgbQuard[SPPal[a]] & 0x00FF00) >> 8, rgbQuard[SPPal[a]] & 0x0000FF));
    }
    //寄存器显示
    /*$2000-$2002
    printf("$2000:%02X  %s\n", m_REG[0], byte_to_binary(reg_bin_list, m_REG[0]));
    printf("$2001:%02X  %s\n", m_REG[1], byte_to_binary(reg_bin_list, m_REG[1]));
    printf("$2002:%02X  %s\n", m_REG[2], byte_to_binary(reg_bin_list, m_REG[2]));
    //$4000-$401F
    printf("$4000:%02X  %s\n", apu_reg[0], byte_to_binary(reg_bin_list, apu_reg[0]));
    printf("$4001:%02X  %s\n", apu_reg[1], byte_to_binary(reg_bin_list, apu_reg[1]));
    printf("$4002:%02X  %s\n", apu_reg[2], byte_to_binary(reg_bin_list, apu_reg[2]));
    printf("$4003:%02X  %s\n", apu_reg[3], byte_to_binary(reg_bin_list, apu_reg[3]));
    printf("$4004:%02X  %s\n", apu_reg[4], byte_to_binary(reg_bin_list, apu_reg[4]));
    printf("$4015:%02X  %s\n", apu_reg[15], byte_to_binary(reg_bin_list, apu_reg[15])); 
    */
 
    sprintf(regstring, "$4000->0x%02X  %s", apu_reg[0], byte_to_binary(reg_bin_list, apu_reg[0]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 270, reglist, screen);
    sprintf(regstring, "$4001->0x%02X  %s", apu_reg[1], byte_to_binary(reg_bin_list, apu_reg[1]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 290, reglist, screen);
    sprintf(regstring, "$4002->0x%02X  %s", apu_reg[2], byte_to_binary(reg_bin_list, apu_reg[2]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 310, reglist, screen);
    sprintf(regstring, "$4003->0x%02X  %s", apu_reg[3], byte_to_binary(reg_bin_list, apu_reg[3]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 330, reglist, screen);
    sprintf(regstring, "$4004->0x%02X  %s", apu_reg[4], byte_to_binary(reg_bin_list, apu_reg[4]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 350, reglist, screen);
    sprintf(regstring, "$4005->0x%02X  %s", apu_reg[5], byte_to_binary(reg_bin_list, apu_reg[5]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 370, reglist, screen);
    sprintf(regstring, "$4006->0x%02X  %s", apu_reg[6], byte_to_binary(reg_bin_list, apu_reg[6]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 390, reglist, screen);
    sprintf(regstring, "$4007->0x%02X  %s", apu_reg[7], byte_to_binary(reg_bin_list, apu_reg[7]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 410, reglist, screen);
    sprintf(regstring, "$4008->0x%02X  %s", apu_reg[8], byte_to_binary(reg_bin_list, apu_reg[8]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 430, reglist, screen);
    sprintf(regstring, "$4009->0x%02X  %s", apu_reg[9], byte_to_binary(reg_bin_list, apu_reg[9]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 450, reglist, screen);
    sprintf(regstring, "$400A->0x%02X  %s", apu_reg[10], byte_to_binary(reg_bin_list, apu_reg[10]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 470, reglist, screen);
    sprintf(regstring, "$400B->0x%02X  %s", apu_reg[11], byte_to_binary(reg_bin_list, apu_reg[11]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 490, reglist, screen);
    sprintf(regstring, "$400C->0x%02X  %s", apu_reg[12], byte_to_binary(reg_bin_list, apu_reg[12]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 510, reglist, screen);
    sprintf(regstring, "$400D->0x%02X  %s", apu_reg[13], byte_to_binary(reg_bin_list, apu_reg[13]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 530, reglist, screen);
    sprintf(regstring, "$400E->0x%02X  %s", apu_reg[14], byte_to_binary(reg_bin_list, apu_reg[14]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 550, reglist, screen);
    sprintf(regstring, "$400F->0x%02X  %s", apu_reg[15], byte_to_binary(reg_bin_list, apu_reg[15]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 570, reglist, screen);
    sprintf(regstring, "$4015->0x%02X  %s", apu_reg[21], byte_to_binary(reg_bin_list, apu_reg[21]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(10, 590, reglist, screen);
    
    sprintf(regstring, "$2000->0x%02X  %s", m_REG[0], byte_to_binary(reg_bin_list, m_REG[0]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 270, reglist, screen);
    sprintf(regstring, "$2001->0x%02X  %s", m_REG[1], byte_to_binary(reg_bin_list, m_REG[1]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 290, reglist, screen);
    sprintf(regstring, "$2002->0x%02X  %s", m_REG[2], byte_to_binary(reg_bin_list, m_REG[2]));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 310, reglist, screen);
    sprintf(regstring, "A->    0x%02X  %s", A, byte_to_binary(reg_bin_list, A));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 330, reglist, screen);
    sprintf(regstring, "X->    0x%02X  %s", X, byte_to_binary(reg_bin_list, X));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 350, reglist, screen);
    sprintf(regstring, "Y->    0x%02X  %s", Y, byte_to_binary(reg_bin_list, Y));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 370, reglist, screen);
    sprintf(regstring, "S->    0x%02X  %s", S, byte_to_binary(reg_bin_list, S));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 390, reglist, screen);
    sprintf(regstring, "P->    0x%02X  %s", P, byte_to_binary(reg_bin_list, P));
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 410, reglist, screen);
    sprintf(regstring, "PC->   0x%04X", PC);
    reglist = TTF_RenderUTF8_Solid(font_mid, regstring, c_fontc);
    apply_surface(268, 430, reglist, screen);

    //将APU寄存器信息写入日志文件
    log_seq++;
    //fprintf(fp_log, "%6d--$4000:%02X  %s\n", log_seq, apu_reg[0], byte_to_binary(reg_bin_list, apu_reg[0]));
}

void process_pattern_color(char* a_byte_color, BYTE a_byte_n, BYTE a_byte_n8)
{    
    short n = 0;
    short n8 = 0;
    for(int c = 0; c < 8; c++){
        n8 = (a_byte_n8 & (1 << c)) >> c;
        n = (a_byte_n & (1 << c)) >> c;
        if(n8 == 0 && n == 0) {
            a_byte_color[7-c] = '0';
        }
        if(n8 == 0 && n == 1) {
            a_byte_color[7-c] = '1';
        }
        if(n8 == 1 && n == 0) {
            a_byte_color[7-c] = '2';
        }
        if(n8 == 1 && n == 1) {
            a_byte_color[7-c] = '3';
        }
    }
}


//静态界面绘制
void render_static_elements()
{
    SDL_Color c_fonta = {120,220,120};
    SDL_Color c_fontb = {150,120,170};
    SDL_Color c_fontc = {255,255,255};
    Uint32 c_square = SDL_MapRGB(screen->format,255,255,255);
    
    Draw_Rect(screen,0,0,264,248,c_square);
    gamelist = TTF_RenderUTF8_Solid(font,"马戏团 mapper000 24K",c_fonta);
    apply_surface(300, 10, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"绿色兵团 mapper002 128K",c_fonta);
    apply_surface(300, 50, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"超级玛丽 mapper000 40K",c_fonta);
    apply_surface(300, 90, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"魂斗罗 mapper002 128K",c_fonta);
    apply_surface(300, 130, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"沙罗曼蛇 mapper002 128K",c_fonta);
    apply_surface(300, 170, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"彩色",c_fontb);
    apply_surface(300, 210, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"黑白",c_fontb);
    apply_surface(350, 210, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"x2.0",c_fontb);
    apply_surface(400, 210, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"x1.0",c_fontb);
    apply_surface(450, 210, gamelist, screen);
    gamelist = TTF_RenderUTF8_Solid(font,"x0.5",c_fontb);
    apply_surface(500, 210, gamelist, screen);
    
    //绘制颜色表
    Draw_Rect(screen,594,18,148,248,c_square);
    gamelist = TTF_RenderUTF8_Solid(font_small,"rgbQuard",c_fontc);
    apply_surface(600, 3, gamelist, screen);
    for(int i = 0; i < 64; i++) {
        Draw_FillRect(screen, 600+i%4*35, 23+15*(i/4), 30, 12 ,
        SDL_MapRGB(screen->format,(rgbQuard[i] & 0xFF0000) >> 16, (rgbQuard[i] & 0x00FF00) >> 8, rgbQuard[i] & 0x0000FF));
    }
    Draw_Rect(screen,754,18,148,248,c_square);
    gamelist = TTF_RenderUTF8_Solid(font_small,"rgbSP",c_fontc);
    apply_surface(760, 3, gamelist, screen);
    for(int i = 0; i < 64; i++) {
        Draw_FillRect(screen, 760+i%4*35, 28+15*(i/4), 30, 12 ,
        SDL_MapRGB(screen->format,(rgbSP[i] & 0xFF0000) >> 16, (rgbSP[i] & 0x00FF00) >> 8, rgbSP[i] & 0x0000FF));
    }  

    //绘制SPPal、BGPal白框
    Draw_Rect(screen,935, 33, 202, 96, c_square);
    gamelist = TTF_RenderUTF8_Solid(font_small,"BGPal",c_fontc);
    apply_surface(945, 14, gamelist, screen);
    Draw_Rect(screen,935, 168, 202, 96, c_square);
    gamelist = TTF_RenderUTF8_Solid(font_small,"SPPal",c_fontc);
    apply_surface(945, 150, gamelist, screen);
}

//字节转二进制
char* byte_to_binary(char* a_bin_list, short val) 
{
	for(int i = 0; i < 8; i++) {
		a_bin_list[7-i] = (char)(((val & (1 << i)) >> i) + 48);
	}
    a_bin_list[8] = '\0';
    return a_bin_list;
}
