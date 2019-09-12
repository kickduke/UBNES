#include "APU.c"

static void write_callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max);
static void underflow_callback(struct SoundIoOutStream *outstream);