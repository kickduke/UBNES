static float seconds_offset = 0.0f;

static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    float amp = 1.0f;          //振幅，统一1.0
    float PCT1 = 12.5f;        //方波1占空比
    float PCT2 = 12.5f;        //方波2占空比
    float squa1_vol = 1.0f;     //方波1音量
    float squa1_amp = 1.0f;     //方波1振幅0.0-1.0
    float float_sample_rate = outstream->sample_rate;      //48000
    float seconds_per_frame = 1.0f / float_sample_rate;
    struct SoundIoChannelArea *areas;
    int frames_left = frame_count_max;
    float radians_per_second = 0;
    float sample = 0;     //样本值，0.0~1.0
    float squa1_pitch;          //方波1频率
    float squa2_pitch;          //方波2频率
    WORD tri_wave_len = 0;
    WORD squa1_wave_len = 0;
    WORD squa2_wave_len = 0;
    BYTE squa1_duty = 0;
    BYTE squa2_duty = 0;
    //fprintf(stderr, "pitch=%f PCT=%f vol=%f\n", squa1_pitch, PCT1, squa1_vol);
    //开始写入样本数据
    while (frames_left > 0) {
        int frame_count = 4;
        soundio_outstream_begin_write(outstream, &areas, &frame_count);
        if (!frame_count)
            break;   
        for (int frame = 0; frame < frame_count; frame += 1) {
            //获取APU数据
            ////获取方波1数据
            squa1_wave_len = (0x00 | apu_reg[2]) | ((0x07 & apu_reg[3]) << 8);
            squa1_duty = (0xC0 & apu_reg[0]) >> 6;
            squa1_vol = (0x0F & apu_reg[0])/15.0f;
            if(squa1_duty == 0) {
                PCT1 = 12.5f;
            }
            if(squa1_duty== 1) {
               PCT1 = 25.0f;
            }
            if(squa1_duty == 2) {
                PCT1 = 50.0f;
            }
            if(squa1_duty == 3) {
                PCT1 = 75.0f;
            }
            if(squa1_wave_len > 0) {
                squa1_pitch = (1.789773f*1000000.0f)/(16.0*(squa1_wave_len+1));
            }else {
                squa1_pitch = 0;
            }
            radians_per_second = squa1_pitch * 8.0f;    //假设每个方波长是8
            if(fmod((seconds_offset + frame * seconds_per_frame) * radians_per_second, 8) <= PCT1/100.0f*8.0f) {
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
}
