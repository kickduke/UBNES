static float seconds_offset = 0.0f;
FILE *fp_callback_log = NULL;     //callback函数记录
static int frame_cnt = 1;
static int second_cnt = 0;
static int rowcnt = 0;

/*static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    double amp = 1.0;          //振幅 
    float PCT1 = 12.5f;        //方波1占空比
    float PCT2 = 12.5f;        //方波2占空比
    float squa1_vol = 1.0f;     //方波1音量
    float float_sample_rate = outstream->sample_rate;      //48000
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    float radians_per_second = 0;
    double sample = 0;     //样本值，0.0~1.0
    float squa1_pitch;          //方波1频率
    float squa2_pitch;          //方波2频率
    WORD tri_wave_len = 0;
    WORD squa1_wave_len = 0;
    WORD squa2_wave_len = 0;
    BYTE squa1_duty = 0;
    BYTE squa2_duty = 0;
    //squa1_pitch = 440.0f;
    //开始写入样本数据
    while (frames_left > 0) {
        int frame_count = 4;
        //int frame_count = frames_left;
            //获取APU数据
            ////获取方波1数据
            squa1_wave_len = (0x00 | apu_reg[2]) | ((0x07 & apu_reg[3]) << 8);
            squa1_duty = (0xC0 & apu_reg[0]) >> 6;
            //squa1_vol = (0x0F & apu_reg[0])/15.0f;
            squa1_vol = (0x0F & apu_reg[0]);
            if(squa1_duty == 0) {
                PCT1 = 12.5f;
            }
            if(squa1_duty == 1) {
               PCT1 = 25.0f;
            }
            if(squa1_duty == 2) {
                PCT1 = 50.0f;
            }
            if(squa1_duty == 3) {
                PCT1 = 75.0f;
            }
            if(squa1_wave_len > 0) {
                squa1_pitch = (1.789773f*1000000.0f)/(16.0f*(squa1_wave_len+1));
            }else {
                squa1_pitch = 0;
            }
        soundio_outstream_begin_write(outstream, &areas, &frame_count);
        if (!frame_count)
            break;   
        for (int frame = 0; frame < frame_count; frame += 1) {
            radians_per_second = squa1_pitch * 8.0f;    //假设每个方波长是8
            if(fmod((seconds_offset + frame * seconds_per_frame) * radians_per_second, 8.0f) <= PCT1/100.0f*8.0f) {
                sample = squa1_vol * amp;
            } else {
                sample = 0;
            }
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                float *ptr = (float*)(areas[channel].ptr + areas[channel].step * frame);   //step=bytes_per_frame
                *ptr = sample;
            }

        }
        seconds_offset += seconds_per_frame * frame_count;
        soundio_outstream_end_write(outstream);
        frames_left -= frame_count;
    }
}*/


static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    Uint16 amp = INT16_MAX;          //振幅 
    float PCT1 = 12.5f;        //方波1占空比
    float PCT2 = 12.5f;        //方波2占空比
    float squa1_vol = 1.0f;     //方波1音量
    float squa2_vol = 1.0f;     //方波2音量
    float tri_vol = 1.0f;       //三角波音量
    float noise_vol = 1.0f;     //噪音音量
    float float_sample_rate = outstream->sample_rate;      //48000
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    float radians1_per_second = 0;  //方波1每秒波长
    float radians2_per_second = 0;  //方波2每秒波长
    float radians3_per_second = 0;  //三角波每秒波长
    float radians4_per_second = 0;  //噪声每秒波长
    double sample1 = 0;     //方波1样本值 
    double sample2 = 0;     //方波2样本值 
    double sample3 = 0;     //三角波样本值 
    double sample4 = 0;     //噪声样本值 
    double sample_mix = 0;  
    float squa1_pitch;          //方波1频率
    float squa2_pitch;          //方波2频率
    float tri_pitch;            //三角波频率
    float noise_pitch;          //噪音频率
    WORD length_list[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068};
    WORD squa1_wave_len = 0;
    WORD squa2_wave_len = 0;
    WORD tri_wave_len = 0;
    WORD noise_wave_len = 0;
    BYTE noise_len_idx = 0;
    BYTE squa1_duty = 0;
    BYTE squa2_duty = 0;
    //开始写入样本数据
    while (frames_left > 0) {
        int frame_count = 4;
        //获取APU数据
        ////获取方波1数据
        squa1_wave_len = (0x00 | apu_reg[2]) | ((0x07 & apu_reg[3]) << 8);
        squa1_duty = (0xC0 & apu_reg[0]) >> 6;
        squa1_vol = (0x0F & apu_reg[0])/15.0f;
        if(squa1_duty == 0) {
            PCT1 = 12.5f;
        }
        if(squa1_duty == 1) {
           PCT1 = 25.0f;
        }
        if(squa1_duty == 2) {
            PCT1 = 50.0f;
        }
        if(squa1_duty == 3) {
            PCT1 = 75.0f;
        }
        if(squa1_wave_len > 0) {
            squa1_pitch = (1.789773f*1000000.0f)/(16.0f*(squa1_wave_len+1));
        }else {
            squa1_pitch = 0;
        }
        ////获取方波2数据
        squa2_wave_len = (0x00 | apu_reg[6]) | ((0x07 & apu_reg[7]) << 8);
        squa2_duty = (0xC0 & apu_reg[4]) >> 6;
        squa2_vol = (0x0F & apu_reg[4])/15.0f;
        if(squa2_duty == 0) {
            PCT2 = 12.5f;
        }
        if(squa2_duty == 1) {
           PCT2 = 25.0f;
        }
        if(squa2_duty == 2) {
            PCT2 = 50.0f;
        }
        if(squa2_duty == 3) {
            PCT2 = 75.0f;
        }
        if(squa2_wave_len > 0) {
            squa2_pitch = (1.789773f*1000000.0f)/(16.0f*(squa2_wave_len+1));
        }else {
            squa2_pitch = 0;
        }
        ////获取三角波数据
        tri_wave_len = (0x00 | apu_reg[10]) | ((0x07 & apu_reg[11]) << 8); 
        tri_vol = 1.0f;  
        if(tri_wave_len > 0) {
            tri_pitch = (1.789773f*1000000.0f)/(16.0f*(tri_wave_len+1));
        }else {
            tri_pitch = 0;
        }  
        //获取噪声数据
        noise_vol = (0x0F & apu_reg[12])/15.0f;
        noise_len_idx = 0x0F & apu_reg[14];
        if(length_list[noise_len_idx] > 0) {
            noise_pitch = (1.789773f*1000000.0f)/(16.0f*(length_list[noise_len_idx]+1));
        }else {
            noise_pitch = 0;
        }  
        soundio_outstream_begin_write(outstream, &areas, &frame_count);
        if (!frame_count)
            break;   
        for (int frame = 0; frame < frame_count; frame += 1) {
            //方波1sample
            radians1_per_second = squa1_pitch * 8.0f;    //假设每个方波长是8
            if(fmod((seconds_offset + frame * seconds_per_frame) * radians1_per_second, 8.0f) <= PCT1/100.0f*8.0f) {
                sample1 = squa1_vol * amp;
            } else {
                sample1 = 0;
            }
            //方波2sample
            radians2_per_second = squa2_pitch * 8.0f;    //假设每个方波长是8
            if(fmod((seconds_offset + frame * seconds_per_frame) * radians2_per_second, 8.0f) <= PCT2/100.0f*8.0f) {
                sample2 = squa2_vol * amp;
            } else {
                sample2 = 0;
            }
            //三角波sample
            radians3_per_second = tri_pitch * 1.0f;     //假设每个三角波长是1
            float t = (seconds_offset+frame*seconds_per_frame) * radians3_per_second;
            sample3 = tri_vol * 0.3 * amp * fabs(2.0f * (t- floor(t + 0.5f)));
            //噪音sample
            radians4_per_second = noise_pitch * 8.0f;  //假设每个噪声波长是8,为12.5%占空比
            if(fmod((seconds_offset + frame * seconds_per_frame) * radians4_per_second, 8.0f) <= 1.0f/15.0f*8.0f) {
                sample4 = noise_vol * 0.1 * amp;
            } else {
                sample4 = 0;
            }
            //写入缓冲区
            ////squa1、squa2、tri混频率
            sample_mix = sample1+sample2+sample3 - (sample1*sample2 + sample1*sample3 + sample2*sample3)/amp + sample1*sample2*sample3/(amp*amp);
            Uint16 *ptr1 = (Uint16*)(areas[0].ptr + areas[0].step * frame);   //step=bytes_per_frame
            *ptr1 = sample_mix;
            Uint16 *ptr2 = (Uint16*)(areas[1].ptr + areas[1].step * frame);   //step=bytes_per_frame
            *ptr2 = sample4;  

        }
        seconds_offset += seconds_per_frame * frame_count;
        soundio_outstream_end_write(outstream);
        frames_left -= frame_count;
    }
}


static void underflow_callback(struct SoundIoOutStream *outstream) {
    
}